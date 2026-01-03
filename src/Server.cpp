#include "Server.h"
#include "NetworkUtils.h"
#include "QRCodeGen.h"
#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>

namespace blade {

Server::Server(const int port, bool useAuth, const std::string& username, const std::string& password)
    : port_(port), useAuth_(useAuth), running_(false) {
    authManager_ = std::make_unique<AuthenticationManager>();

    // Debug logging
    std::cout << "[DEBUG] Server constructor - useAuth: " << (useAuth ? "true" : "false")
              << ", username: '" << username << "', password: '" << password << "'" << std::endl;

    // Register user credentials if authentication is enabled
    if (useAuth_ && !username.empty() && !password.empty()) {
        if (!authManager_->addUser(username, password)) {
            std::cerr << "Warning: Failed to add user credentials" << std::endl;
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
        std::cerr << "Failed to initialize network subsystem" << std::endl;
        return false;
    }
    
    running_ = true;
    
    // Start HTTP server for web interface
    if (!httpServer_->start()) {
        std::cerr << "Warning: HTTP server failed to start" << std::endl;
    }
    
    // Start accepting connections in a separate thread
    std::thread acceptThread(&Server::acceptConnections, this);
    acceptThread.detach();
    
    // Start cleanup thread for inactive HTTP clients
    std::thread cleanupThread(&Server::cleanupInactiveHTTPClients, this);
    cleanupThread.detach();

    const std::string ip = NetworkUtils::getLocalIPAddress();

    std::cout << "\n========================================" << std::endl;
    std::cout << "   BLADE Server Started Successfully" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nWeb Interface Access:" << std::endl;
    std::cout << "  http://" << ip << std::endl;
    std::cout << "\nFile Transfer Port: " << port_ << std::endl;
    std::cout << "Authentication: " << (useAuth_ ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "\nWaiting for client connections..." << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Display QR code for easy access
    const std::string url = "http://" + ip;
    try {
        SetConsoleOutputCP(CP_UTF8);
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(url.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);

        std::cout << "\n+============================================================+" << std::endl;
        std::cout << "|          Scan QR Code to Access Web Interface             |" << std::endl;
        std::cout << "+============================================================+\n" << std::endl;

        int border = 4;
        for (int y = -border; y < qr.getSize() + border; y++) {
            std::cout << "    ";
            for (int x = -border; x < qr.getSize() + border; x++) {
                std::cout << (qr.getModule(x, y) ? "\xE2\x96\x88\xE2\x96\x88" : "  ");
            }
            std::cout << std::endl;
        }

        std::cout << "\n              URL: " << url << "\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating QR code: " << e.what() << std::endl;
    }

    return true;
}

void Server::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    httpServer_->stop();
    NetworkUtils::cleanup();
    
    std::cout << "Server stopped" << std::endl;
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
    return std::vector<std::string>(connectedIPs_.begin(), connectedIPs_.end());
}

void Server::trackHTTPConnection(const std::string& clientIP) {
    std::lock_guard<std::mutex> lock(ipMutex_);

    // Update the last activity timestamp for this client
    const auto now = std::chrono::steady_clock::now();
    const bool isNewIP = httpClientActivity_.find(clientIP) == httpClientActivity_.end();
    httpClientActivity_[clientIP] = now;

    // Add to connected IPs set
    connectedIPs_.insert(clientIP);

    if (isNewIP) {
        std::cout << "[HTTP CLIENT] " << clientIP << " connected" << std::endl;
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
                std::cout << "[HTTP CLIENT] " << it->first << " disconnected (timeout)" << std::endl;
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
        std::cerr << "Failed to create server socket" << std::endl;
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::bindSocket(serverSocket, port_)) {
        std::cerr << "Failed to bind to port " << port_ << std::endl;
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    if (!NetworkUtils::listenSocket(serverSocket)) {
        std::cerr << "Failed to listen on socket" << std::endl;
        NetworkUtils::closeSocket(serverSocket);
        running_ = false;
        return;
    }
    
    // Get the server's own IP to filter it out
    const std::string serverIP = NetworkUtils::getLocalIPAddress();

    while (running_) {
        std::string clientAddr;
        SocketType clientSocket = NetworkUtils::acceptConnection(serverSocket, clientAddr);

        if (clientSocket != INVALID_SOCKET) {
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
                        std::cout << "[CONNECTED] " << clientAddr << " (authentication required)" << std::endl;
                    } else {
                        std::cout << "[CONNECTED] " << clientAddr << std::endl;
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
                    char buffer[1];
                    // Wait for the socket to close
                    while (running_) {
                        int result = NetworkUtils::receiveData(clientSocket, buffer, sizeof(buffer));
                        if (result <= 0) {
                            // Client disconnected
                            std::lock_guard<std::mutex> lock(ipMutex_);
                            connectedIPs_.erase(clientAddr);
                            std::cout << "[DISCONNECTED] " << clientAddr << std::endl;
                            break;
                        }
                    }
                }).detach();
            }
            // If it's a local connection, just handle it silently (no logging, no welcome message)
        }
    }

    NetworkUtils::closeSocket(serverSocket);
}

std::string Server::handleHeartbeat(const std::string& clientIP) {
    std::lock_guard<std::mutex> lock(ipMutex_);

    // Update the last activity timestamp for this client
    httpClientActivity_[clientIP] = std::chrono::steady_clock::now();

    // Return a simple JSON response
    return R"({"status":"ok"})";
}

} // namespace blade
