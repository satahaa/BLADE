#include "HTTPServer.h"
#include "NetworkUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

namespace blade {

HTTPServer::HTTPServer(const int port, std::string webRoot)
    : port_(port), webRoot_(std::move(webRoot)), useHttps_(true), running_(false)
#ifdef _WIN32
    , certStore_(nullptr), certContext_(nullptr), sslInitialized_(false)
#endif
{
#ifdef _WIN32
    std::cout << "Initializing HTTPS with self-signed certificate..." << std::endl;
    if (!initializeSSL()) {
        std::cerr << "Warning: SSL initialization failed, falling back to HTTP" << std::endl;
        useHttps_ = false;
    }
#else
    std::cerr << "Warning: HTTPS not supported on this platform, using HTTP" << std::endl;
    useHttps_ = false;
#endif
}

HTTPServer::~HTTPServer() {
    stop();
#ifdef _WIN32
    cleanupSSL();
#endif
}

bool HTTPServer::start() {
    if (running_) {
        return false;
    }
    
    running_ = true;
    serverThread_ = std::thread(&HTTPServer::run, this);
    return true;
}

void HTTPServer::stop() {
    if (running_) {
        running_ = false;
        if (serverThread_.joinable()) {
            serverThread_.join();
        }
    }
}

bool HTTPServer::isRunning() const {
    return running_;
}

void HTTPServer::run() {
    const SocketType serverSocket = NetworkUtils::createSocket();
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create HTTP server socket" << std::endl;
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::bindSocket(serverSocket, port_)) {
        std::cerr << "Failed to bind HTTP server to port " << port_ << std::endl;
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::listenSocket(serverSocket)) {
        std::cerr << "Failed to listen on HTTP server socket" << std::endl;
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    std::cout << "HTTP Server listening on port " << port_ << std::endl;
    
    while (running_) {
        std::string clientAddr;
        SocketType clientSocket = NetworkUtils::acceptConnection(serverSocket, clientAddr);
        
        if (clientSocket != INVALID_SOCKET) {
            handleRequest(clientSocket);
            NetworkUtils::closeSocket(clientSocket);
        }
    }
    
    NetworkUtils::closeSocket(serverSocket);
}

// Called from run() method which executes in a separate thread
void HTTPServer::handleRequest(const SocketType clientSocket) const {
    char buffer[4096];
    const int bytesRead = NetworkUtils::receiveData(clientSocket, buffer, sizeof(buffer) - 1);
    
    if (bytesRead <= 0) {
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    // Parse HTTP request
    const size_t methodEnd = request.find(' ');
    const size_t pathEnd = request.find(' ', methodEnd + 1);
    
    if (methodEnd == std::string::npos || pathEnd == std::string::npos) {
        return;
    }
    
    std::string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    
    // Default to index.html
    if (path == "/") {
        path = "/index.html";
    }
    
    // Load file
    const std::string filePath = webRoot_ + path;
    const std::string content = loadFile(filePath);
    
    std::string response;
    if (!content.empty()) {
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + getContentType(path) + "\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += content;
    } else {
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += "<html><body><h1>404 Not Found</h1></body></html>";
    }
    
    (void)NetworkUtils::sendData(clientSocket, response);
}

// Helper function - called from handleRequest()
std::string HTTPServer::getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".json") != std::string::npos) return "application/json";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (path.find(".gif") != std::string::npos) return "image/gif";
    return "text/plain";
}

// Helper function - called from handleRequest() to load web files
std::string HTTPServer::loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#ifdef _WIN32
bool HTTPServer::initializeSSL() {
    // Create self-signed certificate for local HTTPS
    if (!createSelfSignedCert()) {
        return false;
    }

    // Initialize Schannel credentials
    SCHANNEL_CRED schannelCred = {};
    schannelCred.dwVersion = SCHANNEL_CRED_VERSION;
    schannelCred.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION;
    schannelCred.cCreds = 1;
    schannelCred.paCred = &certContext_;

    TimeStamp tsExpiry;
    SECURITY_STATUS status = AcquireCredentialsHandle(
        nullptr,
        UNISP_NAME_A,
        SECPKG_CRED_INBOUND,
        nullptr,
        &schannelCred,
        nullptr,
        nullptr,
        &credHandle_,
        &tsExpiry
    );

    if (status != SEC_E_OK) {
        std::cerr << "Failed to acquire credentials handle: " << status << std::endl;
        return false;
    }

    sslInitialized_ = true;
    std::cout << "SSL/TLS initialized successfully with self-signed certificate" << std::endl;
    std::cout << "Note: Browsers will show security warnings for self-signed certificates" << std::endl;
    return true;
}

void HTTPServer::cleanupSSL() {
    if (sslInitialized_) {
        FreeCredentialsHandle(&credHandle_);
        sslInitialized_ = false;
    }

    if (certContext_) {
        CertFreeCertificateContext(certContext_);
        certContext_ = nullptr;
    }

    if (certStore_) {
        CertCloseStore(certStore_, 0);
        certStore_ = nullptr;
    }
}

bool HTTPServer::createSelfSignedCert() {
    // Open certificate store
    certStore_ = CertOpenStore(
        CERT_STORE_PROV_SYSTEM,
        0,
        0,
        CERT_SYSTEM_STORE_CURRENT_USER,
        L"MY"
    );

    if (!certStore_) {
        std::cerr << "Failed to open certificate store" << std::endl;
        return false;
    }

    // Try to find existing BLADE certificate
    certContext_ = CertFindCertificateInStore(
        certStore_,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0,
        CERT_FIND_SUBJECT_STR,
        L"BLADE",
        nullptr
    );

    if (certContext_) {
        std::cout << "Using existing self-signed certificate" << std::endl;
        return true;
    }

    // Create new self-signed certificate
    HCRYPTPROV hCryptProv = 0;
    if (!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        std::cerr << "Failed to acquire cryptographic context" << std::endl;
        return false;
    }

    // Create certificate name
    CERT_NAME_BLOB subjectNameBlob = {};
    const char* subjectName = "CN=BLADE Local Server";

    if (!CertStrToNameA(
        X509_ASN_ENCODING,
        subjectName,
        CERT_X500_NAME_STR,
        nullptr,
        nullptr,
        &subjectNameBlob.cbData,
        nullptr)) {
        CryptReleaseContext(hCryptProv, 0);
        return false;
    }

    subjectNameBlob.pbData = new BYTE[subjectNameBlob.cbData];
    CertStrToNameA(
        X509_ASN_ENCODING,
        subjectName,
        CERT_X500_NAME_STR,
        nullptr,
        subjectNameBlob.pbData,
        &subjectNameBlob.cbData,
        nullptr
    );

    // Create self-signed certificate
    SYSTEMTIME st;
    GetSystemTime(&st);
    SYSTEMTIME stExpire = st;
    stExpire.wYear += 1; // Valid for 1 year

    CRYPT_KEY_PROV_INFO keyProvInfo = {};
    keyProvInfo.pwszContainerName = const_cast<LPWSTR>(L"BLADE_Key_Container");
    keyProvInfo.pwszProvName = nullptr;
    keyProvInfo.dwProvType = PROV_RSA_FULL;
    keyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET;
    keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;

    certContext_ = CertCreateSelfSignCertificate(
        hCryptProv,
        &subjectNameBlob,
        0,
        &keyProvInfo,
        nullptr,
        &st,
        &stExpire,
        nullptr
    );

    delete[] subjectNameBlob.pbData;
    CryptReleaseContext(hCryptProv, 0);

    if (!certContext_) {
        std::cerr << "Failed to create self-signed certificate" << std::endl;
        return false;
    }

    // Add certificate to store
    if (!CertAddCertificateContextToStore(
        certStore_,
        certContext_,
        CERT_STORE_ADD_REPLACE_EXISTING,
        nullptr)) {
        std::cerr << "Failed to add certificate to store" << std::endl;
        return false;
    }

    std::cout << "Created new self-signed certificate for BLADE" << std::endl;
    return true;
}
#endif

} // namespace blade
