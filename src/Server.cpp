#include "Server.h"
#include "NetworkUtils.h"
#include "QRCodeGen.h"
#include "Logger.h"
#include <thread>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace blade {

static uint64_t htonll(uint64_t x) {
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return x;
#else
    return (static_cast<uint64_t>(htonl(static_cast<uint32_t>(x & 0xFFFFFFFFULL))) << 32) |
             htonl(static_cast<uint32_t>(x >> 32));
#endif
}


Server::Server(const int port, const bool useAuth, const std::string& password)
    : port_(port), useAuth_(useAuth), running_(false) {
    authManager_ = std::make_unique<AuthenticationManager>();

    Logger::getInstance().debug("Server constructor - useAuth: " + std::string(useAuth ? "true" : "false") +
                                ", password: '" + password + "'");

    // Register user credentials if authentication is enabled
    if (useAuth_ && !password.empty()) {
        if (authManager_ && !authManager_->setPassword(password)) {
            Logger::getInstance().warning("Failed to add password");
        }
    }

    connectionHandler_ = std::make_unique<ConnectionHandler>();
    httpServer_ = std::make_unique<HTTPServer>(80, "./web", this, useAuth, password); // Web interface on port 80 (HTTP)
}

Server::~Server() {
    stop();
}

bool Server::start() {
    if (running_) {
        return false;
    }
    
    if (!NetworkUtils::initialize()) {
        Logger::getInstance().error("Failed to initialize network subsystem");
        return false;
    }
    
    running_ = true;
    
    // Start HTTP server for web interface
    if (!httpServer_->start()) {
        Logger::getInstance().warning("HTTP server failed to start");
    }
    
    // Start accepting connections in a separate thread
    acceptThread_ = std::thread(&Server::acceptConnections, this);
    // Start cleanup thread for inactive HTTP clients
    cleanupThread_ = std::thread(&Server::cleanupInactiveHTTPClients, this);

    const std::string ip = NetworkUtils::getLocalIPAddress();

    Logger::getInstance().info("BLADE Server Started Successfully");
    Logger::getInstance().info("Web Interface Access: http://" + ip);
    Logger::getInstance().info("File Transfer Port: " + std::to_string(port_));
    Logger::getInstance().info("Authentication: " + std::string(useAuth_ ? "ENABLED" : "DISABLED"));
    Logger::getInstance().info("Waiting for client connections...");


    return true;
}

static std::string sanitizeFilename(const std::string& name) {
    std::string out;
    for (const char c : name) {
        if (c == '/' || c == '\\' || c == ':' || c == '\0' || c == '\n' || c == '\r') {
            out.push_back('_');
        } else {
            out.push_back(c);
        }
    }
    if (out.empty()) out = "file";
    return out;
}

void Server::setDownloadDirectory(const std::string& path) {
    try {
        std::filesystem::path p(path);
        if (!p.is_absolute()) {
            p = std::filesystem::absolute(p);
        }
        std::filesystem::create_directories(p);
        {
            std::lock_guard lock(downloadDirMutex_);
            downloadDir_ = p.string();
        }
        Logger::getInstance().info("Download directory set to: " + p.string());
    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("Failed to set download directory: ") + e.what());
    }
}

std::string Server::getDownloadDirectory() const {
    std::lock_guard lock(downloadDirMutex_);
    return downloadDir_;
}

bool Server::handleUpload(const std::string& filename, const std::vector<uint8_t>& data, const size_t fileSize) const {
    try {
        std::string downloadDir;
        {
            std::lock_guard lock(downloadDirMutex_);
            downloadDir = downloadDir_;
        }
        if (downloadDir.empty()) {
            Logger::getInstance().warning("No download directory set; rejecting upload for " + filename);
            return false;
        }
        const std::string safeName = sanitizeFilename(filename);
        const std::filesystem::path dest = std::filesystem::path(downloadDir) / safeName;

        // If file exists, append a numeric suffix
        std::filesystem::path base = dest.stem();
        std::filesystem::path ext = dest.extension();
        std::filesystem::path dir = dest.parent_path();
        std::filesystem::path candidate = dest;
        int idx = 1;
        while (std::filesystem::exists(candidate)) {
            candidate = dir / (base.string() + '(' + std::to_string(idx++) + ')' + ext.string());
        }

        // Report file info BEFORE starting transfer (so UI can show it immediately)
        const uint64_t actualSize = fileSize > 0 ? fileSize : data.size();
        reportIncomingFile(safeName, actualSize);

        // Report start of upload
        reportIncomingProgress(safeName, 0);

        std::ofstream out(candidate, std::ios::binary);
        if (!out) {
            Logger::getInstance().error("Failed to open file for writing: " + candidate.string());
            return false;
        }

        // Write in chunks and report progress
        constexpr size_t CHUNK_SIZE = 64 * 1024; // 64KB chunks
        size_t totalSize = data.size();
        size_t written = 0;
        int lastReportedPct = 0;

        while (written < totalSize) {
            size_t toWrite = std::min(CHUNK_SIZE, totalSize - written);
            out.write(reinterpret_cast<const char*>(data.data() + written), static_cast<std::streamsize>(toWrite));

            if (!out) {
                Logger::getInstance().error("Failed to write data to file: " + candidate.string());
                out.close();
                return false;
            }

            written += toWrite;

            // Report progress
            int pct = totalSize == 0 ? 100 : static_cast<int>((written * 100) / totalSize);
            if (pct != lastReportedPct) {
                reportIncomingProgress(safeName, pct);
                lastReportedPct = pct;
            }
        }

        out.close();

        Logger::getInstance().info("Saved uploaded file: " + candidate.string());
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("Exception saving upload: ") + e.what());
        return false;
    }
}

void Server::sendFilesToClient(const std::vector<std::string>& filePaths) {
    // Check if there are any connected HTTP clients
    if (!hasConnectedClients()) {
        Logger::getInstance().warning("sendFilesToClient(): no connected HTTP clients");
        return;
    }

    // Queue files for HTTP-based download
    {
        std::lock_guard lock(pendingFilesMutex_);
        for (const auto& path : filePaths) {
            // Only add if not already in queue
            if (std::find(pendingFiles_.begin(), pendingFiles_.end(), path) == pendingFiles_.end()) {
                pendingFiles_.push_back(path);
                Logger::getInstance().info("Queued file for download: " + path);
            }
        }
    }

    // Report initial progress for UI
    for (const auto& path : filePaths) {
        reportProgress(path, 0);
    }
}

void Server::reportProgress(const std::string& path, const int pct) const {
    std::function<void(const std::string&, int)> cb;
    {
        std::lock_guard lock(cbMutex_);
        cb = outgoingProgressCb_;
    }
    if (cb) cb(path, pct);
}

void Server::reportOutgoingProgress(const std::string& path, const int pct) const {
    reportProgress(path, pct);
}

void Server::setIncomingProgressCallback(std::function<void(const std::string&, int)> cb) {
    std::lock_guard lock(cbMutex_);
    incomingProgressCb_ = std::move(cb);
}

void Server::reportIncomingProgress(const std::string& filename, const int pct) const {
    std::function<void(const std::string&, int)> cb;
    {
        std::lock_guard lock(cbMutex_);
        cb = incomingProgressCb_;
    }
    if (cb) cb(filename, pct);
}

void Server::setIncomingFileCallback(std::function<void(const std::string&, uint64_t)> cb) {
    std::lock_guard lock(cbMutex_);
    incomingFileCb_ = std::move(cb);
}

void Server::reportIncomingFile(const std::string& filename, const uint64_t fileSize) const {
    std::function<void(const std::string&, uint64_t)> cb;
    {
        std::lock_guard lock(cbMutex_);
        cb = incomingFileCb_;
    }
    if (cb) cb(filename, fileSize);
}

void Server::announceIncomingFile(const std::string& filename, const uint64_t fileSize) const {
    // Sanitize filename to match what handleUpload uses
    const std::string safeName = sanitizeFilename(filename);
    // Immediately notify UI about incoming file (before data transfer starts)
    Logger::getInstance().debug("Announcing incoming file: " + safeName + " (" + std::to_string(fileSize) + " bytes)");
    reportIncomingFile(safeName, fileSize);
    reportIncomingProgress(safeName, 0);  // Set initial progress to 0%
}

std::vector<std::string> Server::getPendingFiles() const {
    std::lock_guard lock(pendingFilesMutex_);
    return pendingFiles_;
}

void Server::removePendingFile(const std::string& filePath) {
    std::lock_guard lock(pendingFilesMutex_);
    pendingFiles_.erase(
        std::remove(pendingFiles_.begin(), pendingFiles_.end(), filePath),
        pendingFiles_.end()
    );
    Logger::getInstance().debug("Removed pending file: " + filePath);
}

bool Server::hasConnectedClients() const {
    std::lock_guard lock(ipMutex_);
    return !httpClientActivity_.empty();
}

void Server::stop() {
    if (!running_) {
        return;
    }

    {
        std::lock_guard lock(stopMutex_);
        running_ = false;
    }
    stopCv_.notify_all();

    Logger::getInstance().info("Server stop() reached");
    httpServer_->stop();
    NetworkUtils::cleanup();

    // Join threads if joinable
    if (acceptThread_.joinable()) acceptThread_.join();
    if (cleanupThread_.joinable()) cleanupThread_.join();

    Logger::getInstance().info("Server stopped");
}

bool Server::isRunning() const {
    return running_;
}

int Server::getPort() const {
    return port_;
}

void Server::setOutgoingProgressCallback(std::function<void(const std::string&, int)> cb) {
    std::lock_guard lock(cbMutex_);
    outgoingProgressCb_ = std::move(cb);
}


void Server::setAuthRequired(const bool useAuth) {
    useAuth_ = useAuth;
}

std::vector<std::string> Server::getConnectedDevices() const {
    std::lock_guard lock(ipMutex_);
    return std::vector<std::string>{connectedIPs_.begin(), connectedIPs_.end()};
}

void Server::trackHTTPConnection(const std::string& clientIP) {
    std::lock_guard lock(ipMutex_);

    // Update the last activity timestamp for this client
    const auto now = std::chrono::steady_clock::now();
    const bool isNewIP = !httpClientActivity_.contains(clientIP);
    httpClientActivity_[clientIP] = now;

    // Add to connected IPs set
    connectedIPs_.insert(clientIP);

    if (isNewIP) {
        Logger::getInstance().info("[HTTP CLIENT] " + clientIP + " connected");
    }
}

void Server::cleanupInactiveHTTPClients() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard lock(ipMutex_);
        auto now = std::chrono::steady_clock::now();

        // Remove clients that haven't had activity in the last 30 seconds
        for (auto it = httpClientActivity_.begin(); it != httpClientActivity_.end();) {
            if (const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count(); elapsed > 30) {
                Logger::getInstance().info("[HTTP CLIENT] " + it->first + " disconnected (timeout)");
                connectedIPs_.erase(it->first);
                it = httpClientActivity_.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void Server::acceptConnections() {
    const SocketType serverSocket = NetworkUtils::createSocket();
    if (serverSocket == INVALID_SOCKET) {
        Logger::getInstance().error("Failed to create server socket");
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::bindSocket(serverSocket, port_)) {
        Logger::getInstance().error("Failed to bind to port " + std::to_string(port_));
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::listenSocket(serverSocket)) {
        Logger::getInstance().error("Failed to listen on socket");
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    const std::string serverIP = NetworkUtils::getLocalIPAddress();

    while (running_) {
        std::string clientAddr;
        SocketType clientSocket = NetworkUtils::acceptConnectionWithTimeout(serverSocket, clientAddr, 1000); // 1s timeout
        if (!running_) break;
        if (clientSocket != INVALID_SOCKET) {
            // Set client socket to non-blocking

            const int clientId = connectionHandler_->addClient(clientSocket, clientAddr);

            // Filter out local connections completely:
            // - localhost (127.0.0.1, ::1)
            // - loopback range (127.0.0.0/8)
            // - server's own LAN IP (the PC running the server)
            bool isLocalConnection = (clientAddr == "127.0.0.1" ||
                                     clientAddr == "::1" ||
                                     clientAddr.find("127.") == 0 ||
                                     clientAddr == serverIP);

            // Only track and log connections from external devices (not the local PC)
            if (!isLocalConnection) {
                bool isNewIP = false;
                {
                    std::lock_guard<std::mutex> lock(ipMutex_);
                    isNewIP = connectedIPs_.insert(clientAddr).second;
                }

                // Only log when a NEW external device connects
                if (isNewIP) {
                    if (useAuth_) {
                        Logger::getInstance().info("[CONNECTED] " + clientAddr + " (authentication required)");
                    } else {
                        Logger::getInstance().info("[CONNECTED] " + clientAddr);
                    }
                }

                std::string welcomeMsg = "Welcome to BLADE Server!\n";
                if (useAuth_) {
                    welcomeMsg += "Please authenticate to continue.\n";
                } else {
                    welcomeMsg += "Authentication is disabled.\n";
                }
                (void)connectionHandler_->sendToClient(clientId, welcomeMsg);
            }
            // If it's a local connection, just handle it silently (no logging, no welcome message)
        }
    }

    NetworkUtils::closeSocket(serverSocket);
}

std::string Server::handleHeartbeat(const std::string& clientIP) {
    std::lock_guard lock(ipMutex_);

    // Update the last activity timestamp for this client
    httpClientActivity_[clientIP] = std::chrono::steady_clock::now();

    // Return a simple JSON response
    return R"({"status":"ok"})";
}

} // namespace blade
