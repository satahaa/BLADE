#include "HTTPServer.h"
#include "Server.h"
#include "NetworkUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

namespace blade {

HTTPServer::HTTPServer(const int port, std::string webRoot, Server* server, const bool useAuth, std::string username, std::string password)
    : port_(port), webRoot_(std::move(webRoot)), running_(false), server_(server),
      useAuth_(useAuth), username_(std::move(username)), password_(std::move(password))
{
    std::cout << "[DEBUG] HTTPServer constructor - useAuth_: " << (useAuth_ ? "true" : "false")
              << ", username_: '" << username_ << "', password_: '" << password_ << "'" << std::endl;
    std::cout << "HTTP Server initialized on port " << port_ << std::endl;
}

HTTPServer::~HTTPServer() {
    stop();
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
    
    // Silently accept and handle HTTP requests (no logging of every request)
    while (running_) {
        std::string clientAddr;

        if (SocketType clientSocket = NetworkUtils::acceptConnection(serverSocket, clientAddr); clientSocket != INVALID_SOCKET) {
            // Track this client connection if it's not localhost, and we have a server reference
            if (server_ && clientAddr != "127.0.0.1" && clientAddr != "::1" && clientAddr.find("127.") != 0) {
                // Get server's own IP to filter it out
                const std::string serverIP = NetworkUtils::getLocalIPAddress();
                if (clientAddr != serverIP) {
                    // Add this IP to the server's connected devices
                    // The Server will handle adding it to its connectedIPs_ set
                    server_->trackHTTPConnection(clientAddr);
                }
            }

            // Handle each request in a separate thread (detached) for non-blocking behavior
            std::thread([this, clientSocket]() {
                handleRequest(clientSocket);
                NetworkUtils::closeSocket(clientSocket);
            }).detach();
        }
    }
    
    NetworkUtils::closeSocket(serverSocket);
}

// Called from run() method which executes in a separate thread
void HTTPServer::handleRequest(const SocketType clientSocket) const {
    // Set 5 second timeout for read/write operations to prevent hanging
    setSocketTimeout(clientSocket, 5);

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
    
    // Remove query parameters from path (e.g., /api/auth-config?t=123 -> /api/auth-config)
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }

    // Handle auth config endpoint
    if (path == "/api/auth-config") {
        const std::string config = getAuthConfig();
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(config.length()) + "\r\n";
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Pragma: no-cache\r\n";
        response += "Expires: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += config;
        (void)NetworkUtils::sendData(clientSocket, response);
        return;
    }

    // Handle connected devices endpoint
    if (path == "/api/connected-devices") {
        const std::string devices = getConnectedDevicesJson();
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(devices.length()) + "\r\n";
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Pragma: no-cache\r\n";
        response += "Expires: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += devices;
        (void)NetworkUtils::sendData(clientSocket, response);
        return;
    }

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
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Pragma: no-cache\r\n";
        response += "Expires: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += content;
    } else {
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += "<html><body><h1>404 Not Found</h1></body></html>";
    }
    
    (void)NetworkUtils::sendData(clientSocket, response);
}

// Set socket timeout for read/write operations
void HTTPServer::setSocketTimeout(const SocketType socket, const int seconds) {
#ifdef _WIN32
    const DWORD timeout = seconds * 1000;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

// Helper function - called from handleRequest()
std::string HTTPServer::getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".json") != std::string::npos) return "application/json";
    if (path.find(".svg") != std::string::npos) return "image/svg+xml";
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

    // Check file size before reading
    file.seekg(0, std::ios::end);
    const std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Limit file size to 10MB to prevent memory issues
    if (fileSize > 10 * 1024 * 1024) {
        std::cerr << "File too large: " << path << " (" << fileSize << " bytes)" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


std::string HTTPServer::getAuthConfig() const {
    std::string json = "{";
    json += "\"authEnabled\":" + std::string(useAuth_ ? "true" : "false");
    if (useAuth_) {
        json += R"(,"username":")" + username_ + "\"";
        json += R"(,"password":")" + password_ + "\"";
    }
    json += "}";

    // Debug logging
    std::cout << "[DEBUG] Auth config: " << json << std::endl;

    return json;
}

std::string HTTPServer::getConnectedDevicesJson() const {
    std::string json = "{\"devices\":[";

    if (server_) {
        auto devices = server_->getConnectedDevices();
        for (size_t i = 0; i < devices.size(); ++i) {
            json += "\"" + devices[i] + "\"";
            if (i < devices.size() - 1) {
                json += ",";
            }
        }
    }

    json += "]}";
    return json;
}

} // namespace blade
