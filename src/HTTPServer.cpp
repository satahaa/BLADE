#include "HTTPServer.h"

#include <cstring>
#include <filesystem>
#include <iomanip>
#include <algorithm>

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
                                 ", password_: '" + password_ + "'");
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
            std::thread([this, clientSocket, clientAddr]() {
                handleRequest(clientSocket, clientAddr);
                NetworkUtils::closeSocket(clientSocket);
            }).detach();
        }
    }
    
    NetworkUtils::closeSocket(serverSocket);
}

// Called from run() method which executes in a separate thread
void HTTPServer::handleRequest(const SocketType clientSocket, const std::string& clientIP) const {
    setSocketTimeout(clientSocket);

    auto recvSome = [&](std::vector<uint8_t>& dst) -> int {
        uint8_t tmp[4096];
        const int n = NetworkUtils::receiveData(clientSocket, reinterpret_cast<char*>(tmp), sizeof(tmp));
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

    Logger::getInstance().debug("[HTTP] " + method + " " + path + " from " + clientIP);

    if (size_t queryPos = path.find('?'); queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }

    // Handle CORS preflight OPTIONS requests
    if (method == "OPTIONS") {
        std::string response = "HTTP/1.1 204 No Content\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Cache-Control, Pragma, Expires\r\n";
        response += "Access-Control-Max-Age: 86400\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        (void)NetworkUtils::sendData(clientSocket, response);
        return;
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
                server_ && server_->handleUpload(filename, fileData, fileData.size())) {
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

    // --- Handle file upload announcement (notify UI before upload starts) ---
    if (path == "/api/upload/announce" && method == "POST") {
        // Read body
        std::string cl = getHeaderValue("Content-Length");
        size_t contentLength = 0;
        if (!cl.empty()) {
            try { contentLength = std::stoull(cl); }
            catch (...) { contentLength = 0; }
        }

        // Read body data
        while (raw.size() - bodyStart < contentLength) {
            if (int n = recvSome(raw); n <= 0)
                break;
        }

        // Parse JSON body for filename and size
        std::string bodyStr(raw.begin() + bodyStart, raw.begin() + bodyStart + contentLength);

        // Simple JSON parsing for {"filename": "...", "size": ...}
        std::string filename;
        uint64_t fileSize = 0;

        // Extract filename
        if (size_t fnPos = bodyStr.find("\"filename\""); fnPos != std::string::npos) {
            size_t colonPos = bodyStr.find(':', fnPos);
            if (colonPos != std::string::npos) {
                size_t quoteStart = bodyStr.find('"', colonPos + 1);
                if (quoteStart != std::string::npos) {
                    size_t quoteEnd = bodyStr.find('"', quoteStart + 1);
                    if (quoteEnd != std::string::npos) {
                        filename = bodyStr.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    }
                }
            }
        }

        // Extract size
        if (size_t szPos = bodyStr.find("\"size\""); szPos != std::string::npos) {
            size_t colonPos = bodyStr.find(':', szPos);
            if (colonPos != std::string::npos) {
                size_t numStart = colonPos + 1;
                while (numStart < bodyStr.size() && (bodyStr[numStart] == ' ' || bodyStr[numStart] == '\t')) ++numStart;
                size_t numEnd = numStart;
                while (numEnd < bodyStr.size() && bodyStr[numEnd] >= '0' && bodyStr[numEnd] <= '9') ++numEnd;
                if (numEnd > numStart) {
                    try { fileSize = std::stoull(bodyStr.substr(numStart, numEnd - numStart)); }
                    catch (...) { fileSize = 0; }
                }
            }
        }

        std::string resp;
        if (!filename.empty() && server_) {
            server_->announceIncomingFile(filename, fileSize);
            resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 15\r\nConnection: close\r\n\r\n{\"status\":\"ok\"}";
        } else {
            resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 18\r\nConnection: close\r\n\r\n{\"status\":\"error\"}";
        }
        (void)NetworkUtils::sendData(clientSocket, resp);
        return;
    }
    // --- End upload announcement handling ---

    // Handle heartbeat endpoint
    if (path == "/api/heartbeat") {

        if (server_) {
            const std::string heartbeatResponse = server_->handleHeartbeat(clientIP);
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(heartbeatResponse.length()) + "\r\n";
            response += "Access-Control-Allow-Origin: *\r\n";
            response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
            response += "Access-Control-Allow-Headers: Content-Type, Cache-Control, Pragma, Expires\r\n";
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
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Cache-Control, Pragma, Expires\r\n";
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
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Cache-Control, Pragma, Expires\r\n";
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Pragma: no-cache\r\n";
        response += "Expires: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += devices;
        (void)NetworkUtils::sendData(clientSocket, response);
        return;
    }

    // Handle pending files endpoint (returns list of files queued for download)
    if (path == "/api/pending-files") {
        const std::string pendingFiles = getPendingFilesJson();
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(pendingFiles.length()) + "\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Cache-Control, Pragma, Expires\r\n";
        response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        response += "Pragma: no-cache\r\n";
        response += "Expires: 0\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += pendingFiles;
        (void)NetworkUtils::sendData(clientSocket, response);
        return;
    }

    // Handle file download endpoint (streams file to client)
    if (path.find("/api/download/") == 0) {
        // Extract the file index from URL
        // URL format can be /api/download/{index} or /api/download/{index}/{filename}
        std::string remainder = path.substr(14); // "/api/download/" is 14 chars

        // If there's a slash, extract just the index part
        const size_t slashPos = remainder.find('/');
        std::string indexStr = (slashPos != std::string::npos) ? remainder.substr(0, slashPos) : remainder;

        try {
            const size_t fileIndex = std::stoul(indexStr);
            if (server_) {
                const auto pendingFiles = server_->getPendingFiles();
                if (fileIndex < pendingFiles.size()) {
                    const std::string& filePath = pendingFiles[fileIndex];
                    handleFileDownload(clientSocket, filePath);
                    return;
                }
            }
        } catch (...) {
            // Invalid index
        }

        // File not found
        std::string response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 14\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += "File not found";
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
    // Convert to lowercase for comparison
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    // Web files
    if (lowerPath.find(".html") != std::string::npos) return "text/html";
    if (lowerPath.find(".css") != std::string::npos) return "text/css";
    if (lowerPath.find(".js") != std::string::npos) return "application/javascript";
    if (lowerPath.find(".json") != std::string::npos) return "application/json";

    // Images
    if (lowerPath.find(".svg") != std::string::npos) return "image/svg+xml";
    if (lowerPath.find(".png") != std::string::npos) return "image/png";
    if (lowerPath.find(".jpg") != std::string::npos || lowerPath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (lowerPath.find(".gif") != std::string::npos) return "image/gif";
    if (lowerPath.find(".ico") != std::string::npos) return "image/x-icon";
    if (lowerPath.find(".webp") != std::string::npos) return "image/webp";
    if (lowerPath.find(".bmp") != std::string::npos) return "image/bmp";

    // Video
    if (lowerPath.find(".mp4") != std::string::npos) return "video/mp4";
    if (lowerPath.find(".mov") != std::string::npos) return "video/quicktime";
    if (lowerPath.find(".avi") != std::string::npos) return "video/x-msvideo";
    if (lowerPath.find(".mkv") != std::string::npos) return "video/x-matroska";
    if (lowerPath.find(".webm") != std::string::npos) return "video/webm";
    if (lowerPath.find(".wmv") != std::string::npos) return "video/x-ms-wmv";
    if (lowerPath.find(".flv") != std::string::npos) return "video/x-flv";
    if (lowerPath.find(".m4v") != std::string::npos) return "video/x-m4v";

    // Audio
    if (lowerPath.find(".mp3") != std::string::npos) return "audio/mpeg";
    if (lowerPath.find(".wav") != std::string::npos) return "audio/wav";
    if (lowerPath.find(".ogg") != std::string::npos) return "audio/ogg";
    if (lowerPath.find(".flac") != std::string::npos) return "audio/flac";
    if (lowerPath.find(".m4a") != std::string::npos) return "audio/mp4";
    if (lowerPath.find(".aac") != std::string::npos) return "audio/aac";
    if (lowerPath.find(".wma") != std::string::npos) return "audio/x-ms-wma";

    // Documents
    if (lowerPath.find(".pdf") != std::string::npos) return "application/pdf";
    if (lowerPath.find(".doc") != std::string::npos) return "application/msword";
    if (lowerPath.find(".docx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (lowerPath.find(".xls") != std::string::npos) return "application/vnd.ms-excel";
    if (lowerPath.find(".xlsx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if (lowerPath.find(".ppt") != std::string::npos) return "application/vnd.ms-powerpoint";
    if (lowerPath.find(".pptx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.presentationml.presentation";

    // Archives
    if (lowerPath.find(".zip") != std::string::npos) return "application/zip";
    if (lowerPath.find(".rar") != std::string::npos) return "application/x-rar-compressed";
    if (lowerPath.find(".7z") != std::string::npos) return "application/x-7z-compressed";
    if (lowerPath.find(".tar") != std::string::npos) return "application/x-tar";
    if (lowerPath.find(".gz") != std::string::npos) return "application/gzip";

    // Default to octet-stream for unknown types (forces download)
    return "application/octet-stream";
}

// Helper function - called from handleRequest() to load web files
std::string HTTPServer::loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Logger::getInstance().debug("Failed to open file: " + path);
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
        const auto devices = server_->getConnectedDevices();
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

std::string HTTPServer::getPendingFilesJson() const {
    std::string json = "{\"files\":[";

    if (server_) {
        const auto files = server_->getPendingFiles();
        for (size_t i = 0; i < files.size(); ++i) {
            // Get just the filename for display
            std::filesystem::path p(files[i]);
            std::string filename = p.filename().string();

            // Get file size
            uint64_t fileSize = 0;
            try {
                if (std::filesystem::exists(p)) {
                    fileSize = std::filesystem::file_size(p);
                }
            } catch (...) {}

            json += "{\"index\":" + std::to_string(i) + ",";
            json += "\"name\":\"" + filename + "\",";
            json += "\"size\":" + std::to_string(fileSize) + "}";

            if (i < files.size() - 1) {
                json += ",";
            }
        }
    }

    json += "]}";
    return json;
}

void HTTPServer::handleFileDownload(const SocketType clientSocket, const std::string& filePath) const {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Logger::getInstance().error("Failed to open file for download: " + filePath);
        std::string response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 14\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += "File not found";
        (void)NetworkUtils::sendData(clientSocket, response);
        // Remove from queue since file doesn't exist
        if (server_) server_->removePendingFile(filePath);
        return;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    const auto fileSize = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Get filename and extension
    std::filesystem::path p(filePath);
    std::string filename = p.filename().string();
    std::string extension = p.extension().string();

    // URL-encode the filename for Content-Disposition header
    auto urlEncode = [](const std::string& str) -> std::string {
        std::ostringstream encoded;
        for (unsigned char c : str) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded << c;
            } else {
                encoded << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            }
        }
        return encoded.str();
    };

    std::string encodedFilename = urlEncode(filename);

    // Determine content type based on extension
    std::string contentType = getContentType(extension);
    if (contentType == "text/plain") {
        // Default to octet-stream for unknown types to force download
        contentType = "application/octet-stream";
    }

    Logger::getInstance().info("Starting download: " + filename + " (" + std::to_string(fileSize) + " bytes)");

    // Set longer timeout for file transfer (5 minutes for large files)
    setSocketTimeout(clientSocket, 300);

    // Send HTTP headers with Content-Disposition for download
    // Use both filename and filename* for maximum browser compatibility
    std::string headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: " + contentType + "\r\n";
    headers += "Content-Length: " + std::to_string(fileSize) + "\r\n";
    headers += "Content-Disposition: attachment; filename=\"" + filename + "\"; filename*=UTF-8''" + encodedFilename + "\r\n";
    headers += "Access-Control-Allow-Origin: *\r\n";
    headers += "Cache-Control: no-cache\r\n";
    headers += "Connection: close\r\n";
    headers += "\r\n";

    if (NetworkUtils::sendData(clientSocket, headers) < 0) {
        Logger::getInstance().error("Failed to send download headers for: " + filename);
        if (server_) server_->removePendingFile(filePath);
        return;
    }

    // Stream file content in chunks
    constexpr size_t CHUNK_SIZE = 256 * 1024; // 256KB chunks for faster transfer
    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t sent = 0;
    bool transferFailed = false;
    int lastReportedPct = -1;

    // Report initial progress
    if (server_) {
        server_->reportOutgoingProgress(filePath, 0);
    }

    while (sent < fileSize && file.good()) {
        file.read(buffer.data(), CHUNK_SIZE);
        const auto bytesRead = static_cast<size_t>(file.gcount());

        if (bytesRead > 0) {
            if (!NetworkUtils::sendAll(clientSocket, buffer.data(), bytesRead)) {
                Logger::getInstance().error("Failed to send file chunk for: " + filename + " (sent " + std::to_string(sent) + "/" + std::to_string(fileSize) + " bytes)");
                transferFailed = true;
                break;
            }
            sent += bytesRead;

            // Report progress every 1% or every chunk for small files
            const int pct = (fileSize == 0) ? 100 : static_cast<int>((sent * 100) / fileSize);
            if (pct != lastReportedPct && server_) {
                server_->reportOutgoingProgress(filePath, pct);
                lastReportedPct = pct;
            }
        }
    }

    file.close();

    // Report final progress (100% if successful)
    if (server_) {
        if (!transferFailed && sent >= fileSize) {
            server_->reportOutgoingProgress(filePath, 100);
        }
    }

    // Always remove file from pending queue (whether success or failure)
    // This prevents infinite retry loops
    if (server_) {
        server_->removePendingFile(filePath);
        if (!transferFailed && sent >= fileSize) {
            Logger::getInstance().info("File downloaded successfully: " + filename);
        } else {
            Logger::getInstance().warning("File download incomplete: " + filename + " (sent " + std::to_string(sent) + "/" + std::to_string(fileSize) + " bytes)");
        }
    }
}


} // namespace blade
