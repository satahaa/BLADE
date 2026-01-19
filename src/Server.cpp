#include "Server.h"
#include "NetworkUtils.h"
#include "QRCodeGen.h"
#include "Logger.h"
#include <thread>
#include <vector>
#include <windows.h>
#include <fcntl.h>

namespace blade {

Server::Server(const int port, const bool useAuth, const std::string& username, const std::string& password)
    : port_(port), useAuth_(useAuth), running_(false) {
    authManager_ = std::make_unique<AuthenticationManager>();

    Logger::getInstance().debug("Server constructor - useAuth: " + std::string(useAuth ? "true" : "false") +
                                ", username: '" + username + "', password: '" + password + "'");

    // Register user credentials if authentication is enabled
    if (useAuth_ && !username.empty() && !password.empty()) {
        if (authManager_ && !authManager_->addUser(username, password)) {
            Logger::getInstance().warning("Failed to add user credentials");
        }
    }

    connectionHandler_ = std::make_unique<ConnectionHandler>();
    httpServer_ = std::make_unique<HTTPServer>(80, "./web", this, useAuth_, username, password); // Web interface on port 80 (HTTP)
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

    Logger::getInstance().info("========================================");
    Logger::getInstance().info("   BLADE Server Started Successfully");
    Logger::getInstance().info("========================================");
    Logger::getInstance().info("Web Interface Access: http://" + ip);
    Logger::getInstance().info("File Transfer Port: " + std::to_string(port_));
    Logger::getInstance().info("Authentication: " + std::string(useAuth_ ? "ENABLED" : "DISABLED"));
    Logger::getInstance().info("Waiting for client connections...");
    Logger::getInstance().info("========================================");


    return true;
}

void Server::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
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

void Server::setAuthRequired(const bool useAuth) {
    useAuth_ = useAuth;
}

std::vector<std::string> Server::getConnectedDevices() const {
    std::lock_guard<std::mutex> lock(ipMutex_);
    return std::vector(connectedIPs_.begin(), connectedIPs_.end());
}

void Server::trackHTTPConnection(const std::string& clientIP) {
    std::lock_guard<std::mutex> lock(ipMutex_);

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
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::lock_guard<std::mutex> lock(ipMutex_);
        auto now = std::chrono::steady_clock::now();

        // Remove clients that haven't had activity in the last 10 seconds
        for (auto it = httpClientActivity_.begin(); it != httpClientActivity_.end();) {
            if (const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count(); elapsed > 10) {
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
#ifdef _WIN32
            u_long mode = 1;
            ioctlsocket(clientSocket, FIONBIO, &mode);
#else
            fcntl(clientSocket, F_SETFL, O_NONBLOCK);
#endif
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

                // Start a thread to monitor this client for disconnection
                std::thread([this, clientSocket, clientAddr]() {
                    while (running_) {
                        fd_set readfds;
                        FD_ZERO(&readfds);
                        FD_SET(clientSocket, &readfds);
                        timeval tv;
                        tv.tv_sec = 0;
                        tv.tv_usec = 500000; // 0.5s timeout
                        int selResult = select(clientSocket + 1, &readfds, nullptr, nullptr, &tv);
                        if (selResult > 0 && FD_ISSET(clientSocket, &readfds)) {
                            char buffer[1];
                            int bytes = NetworkUtils::receiveData(clientSocket, buffer, sizeof(buffer));
                            if (bytes <= 0) {
                                std::lock_guard lock(ipMutex_);
                                connectedIPs_.erase(clientAddr);
                                Logger::getInstance().info("[DISCONNECTED] " + clientAddr);
                                break;
                            }
                        }
                        // If selResult == 0, timeout, just loop again and check running_
                    }
                }).detach();
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
