#include "HTTPServer.h"

#include <cstring>

#include "Server.h"
#include "NetworkUtils.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <utility>

namespace blade {

HTTPServer::HTTPServer(const int port, std::string webRoot, Server* server, const bool useAuth, std::string password)
    : port_(port), webRoot_(std::move(webRoot)), running_(false), server_(server),
      useAuth_(useAuth), password_(std::move(password))
{
    Logger::getInstance().debug("HTTPServer constructor - useAuth_: " + std::string(useAuth_ ? "true" : "false") +
                                 + "', password_: '" + password_ + "'");
    Logger::getInstance().info("HTTP Server initialized on port " + std::to_string(port_));
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
    Logger::getInstance().info("HTTP Server stopped");
}

bool HTTPServer::isRunning() const {
    return running_;
}

void HTTPServer::run() {
    const SocketType serverSocket = NetworkUtils::createSocket();
    if (serverSocket == INVALID_SOCKET) {
        Logger::getInstance().error("Failed to create HTTP server socket");
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::bindSocket(serverSocket, port_)) {
        Logger::getInstance().error("Failed to bind HTTP server to port " + std::to_string(port_));
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::listenSocket(serverSocket)) {
        Logger::getInstance().error("Failed to listen on HTTP server socket");
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    // Silently accept and handle HTTP requests (no logging of every request)
    while (running_) {
        std::string clientAddr;
        // Use non-blocking accept with timeout
        SocketType clientSocket = NetworkUtils::acceptConnectionWithTimeout(serverSocket, clientAddr, 1000); // 1s timeout
        if (!running_) break;
        if (clientSocket != INVALID_SOCKET) {
            // Track this client connection if it's not localhost, and we have a server reference
            if (server_ && clientAddr != "127.0.0.1" && clientAddr != "::1" && clientAddr.find("127.") != 0) {
                // Get server's own IP to filter it out
                const std::string serverIP = NetworkUtils::getLocalIPAddress();
                if (clientAddr != serverIP) {
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
    setSocketTimeout(clientSocket, 5);

    auto recvSome = [&](std::vector<uint8_t>& dst) -> int {
        uint8_t tmp[4096];
        int n = NetworkUtils::receiveData(clientSocket, reinterpret_cast<char*>(tmp), sizeof(tmp));
        if (n > 0) dst.insert(dst.end(), tmp, tmp + n);
        return n;
    };

    // 1) Read until we have full headers: "\r\n\r\n"
    std::vector<uint8_t> raw;
    raw.reserve(8192);

    const auto findSeq = [](const std::vector<uint8_t>& hay, const std::string& needle, const size_t start = 0) -> size_t {
        if (needle.empty()) return std::string::npos;
        const auto* n = reinterpret_cast<const uint8_t*>(needle.data());
        for (size_t i = start; i + needle.size() <= hay.size(); ++i) {
            if (std::memcmp(hay.data() + i, n, needle.size()) == 0) return i;
        }
        return std::string::npos;
    };

    while (true) {
        if (raw.size() > 1024 * 1024) return; // sanity limit on headers+early body
        if (findSeq(raw, "\r\n\r\n") != std::string::npos) break;
        int n = recvSome(raw);
        if (n <= 0) return;
    }

    size_t headerEnd = findSeq(raw, "\r\n\r\n");
    size_t bodyStart = headerEnd + 4;

    // Convert ONLY headers to string (safe: headers are text)
    std::string headerStr(reinterpret_cast<const char*>(raw.data()), headerEnd);

    // 2) Parse request line (method/path)
    size_t lineEnd = headerStr.find("\r\n");
    if (lineEnd == std::string::npos) return;
    std::string requestLine = headerStr.substr(0, lineEnd);

    size_t methodEnd = requestLine.find(' ');
    size_t pathEnd   = requestLine.find(' ', methodEnd + 1);
    if (methodEnd == std::string::npos || pathEnd == std::string::npos) return;

    std::string method = requestLine.substr(0, methodEnd);
    std::string path   = requestLine.substr(methodEnd + 1, pathEnd - methodEnd - 1);

    if (size_t queryPos = path.find('?'); queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }

    // Helper: get a header value (case-insensitive match for typical headers)
    auto getHeaderValue = [&](const std::string& name) -> std::string {
        std::string needle = "\r\n" + name + ":";
        size_t p = headerStr.find(needle);
        if (p == std::string::npos) {
            // also allow header at start (first line already removed)
            needle = name + ":";
            p = headerStr.find(needle);
            if (p != 0) return "";
        } else {
            p += 2; // skip leading \r\n
        }
        p += name.size() + 1; // skip "Name:"
        while (p < headerStr.size() && headerStr[p] == ' ') ++p;
        size_t e = headerStr.find("\r\n", p);
        if (e == std::string::npos) e = headerStr.size();
        return headerStr.substr(p, e - p);
    };

    // --- File upload handling ---
    if (path == "/api/upload" && method == "POST") {
        // 3) Read full body using Content-Length
        std::string cl = getHeaderValue("Content-Length");
        if (cl.empty()) return;

        size_t contentLength = 0;
        try { contentLength = std::stoull(cl); }
        catch (...) { return; }

        // Ensure we have exactly Content-Length bytes after headers
        while (raw.size() - bodyStart < contentLength) {
            if (int n = recvSome(raw); n <= 0)
                break;
        }
        if (raw.size() - bodyStart < contentLength) return;

        // Copy body bytes
        std::vector body(raw.begin() + bodyStart, raw.begin() + bodyStart + contentLength);

        // 4) Parse multipart boundary from Content-Type
        std::string ct = getHeaderValue("Content-Type");
        size_t bpos = ct.find("boundary=");
        if (bpos == std::string::npos) return;
        std::string boundaryToken = ct.substr(bpos + 9);
        // Trim optional quotes and spaces
        while (!boundaryToken.empty() && (boundaryToken.front() == ' ')) boundaryToken.erase(boundaryToken.begin());
        if (!boundaryToken.empty() && boundaryToken.front() == '"') {
            boundaryToken.erase(boundaryToken.begin());
            if (size_t q = boundaryToken.find('"'); q != std::string::npos)
                boundaryToken.resize(q);
        } else {
            if (size_t sc = boundaryToken.find(';'); sc != std::string::npos)
                boundaryToken.resize(sc);
        }

        std::string boundary = "--" + boundaryToken;
        std::string boundaryNext = "\r\n" + boundary;

        size_t partPos = 0;
        bool anyOk = false;
        while ((partPos = findSeq(body, boundary, partPos)) != std::string::npos) {
            partPos += boundary.size();
            // Skip optional leading \r\n
            if (partPos + 2 <= body.size() && body[partPos] == '\r' && body[partPos + 1] == '\n') {
                partPos += 2;
            }
            // Check for end marker
            if (partPos + 2 <= body.size() && body[partPos] == '-' && body[partPos + 1] == '-') {
                break; // End of multipart
            }

            // Find headers end
            size_t headersEnd = findSeq(body, "\r\n\r\n", partPos);
            if (headersEnd == std::string::npos) break;

            std::string partHeaderStr(reinterpret_cast<const char*>(&body[partPos]), headersEnd - partPos);

            // Extract filename
            std::string filename;
            if (size_t fn = partHeaderStr.find("filename="); fn != std::string::npos) {
                fn += 9;
                if (fn < partHeaderStr.size() && partHeaderStr[fn] == '"') {
                    ++fn;
                    if (size_t endq = partHeaderStr.find('"', fn); endq != std::string::npos)
                        filename = partHeaderStr.substr(fn, endq - fn);
                } else {
                    size_t end = partHeaderStr.find(';', fn);
                    if (end == std::string::npos) end = partHeaderStr.find("\r\n", fn);
                    filename = partHeaderStr.substr(fn, end - fn);
                }
            }
            if (filename.empty()) filename = "upload.bin";

            // File data
            size_t dataStart = headersEnd + 4;
            size_t nextBoundary = findSeq(body, boundaryNext, dataStart);
            if (nextBoundary == std::string::npos) break;
            size_t dataEnd = nextBoundary;

            if (std::vector fileData(body.begin() + dataStart, body.begin() + dataEnd);
                server_ && server_->handleUpload(filename, fileData)) {
                anyOk = true;
            }

            partPos = nextBoundary;
        }

        std::string resp = anyOk
            ? "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK"
            : "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 5\r\nConnection: close\r\n\r\nERROR";
        (void)NetworkUtils::sendData(clientSocket, resp);
        return;
    }
    // --- End file upload handling ---

    // Handle heartbeat endpoint
    if (path == "/api/heartbeat") {
        // Get client IP from the request (you'll need to extract this from the socket)
        // For now, we'll use a placeholder
        std::string clientIP = "unknown";

        if (server_) {
            const std::string heartbeatResponse = server_->handleHeartbeat(clientIP);
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(heartbeatResponse.length()) + "\r\n";
            response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
            response += "Pragma: no-cache\r\n";
            response += "Expires: 0\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += heartbeatResponse;
            (void)NetworkUtils::sendData(clientSocket, response);
        }
        return;
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
    if (path.find(".ico") != std::string::npos) return "image/x-icon";
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
        Logger::getInstance().error("File too large: " + path + " (" + std::to_string(fileSize) + " bytes)");
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
        json += R"(,"password":")" + password_ + "\"";
    }
    json += "}";

    Logger::getInstance().debug("Auth config: " + json);

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
