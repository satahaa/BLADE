#include "Server.h"
#include "NetworkUtils.h"
#include <iostream>
#include <thread>
#include <unordered_set>
#include <mutex>

namespace blade {

Server::Server(int port, bool useAuth, const std::string& username, const std::string& password)
    : port_(port), useAuth_(useAuth), running_(false) {
    authManager_ = std::make_unique<AuthenticationManager>();

    // Register user credentials if authentication is enabled
    if (useAuth_ && !username.empty() && !password.empty()) {
        if (!authManager_->addUser(username, password)) {
            std::cerr << "Warning: Failed to add user credentials" << std::endl;
        }
    }

    connectionHandler_ = std::make_unique<ConnectionHandler>();
    httpServer_ = std::make_unique<HTTPServer>(port + 1, "./web"); // HTTPS always enabled
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
    
    std::cout << "BLADE Server started on port " << port_ << " (HTTPS)" << std::endl;

    const std::string ip = NetworkUtils::getLocalIPAddress();

    std::cout << "\n========================================\n";
    std::cout << "Connect from your phone (same Wi-Fi):\n";
    std::cout << "  https://" << ip << ":" << port_ << "\n";
    std::cout << "From this PC:\n";
    std::cout << "  https://localhost:" << port_ << "\n";
    std::cout << "========================================\n";
    std::cout << "Authentication: " << (useAuth_ ? "ENABLED" : "DISABLED") << std::endl;

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

void Server::acceptConnections() {
    // Track which client IPs we have already printed as "logged in"
    static std::unordered_set<std::string> announced;
    static std::mutex announcedMu;

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
    
    while (running_) {
        std::string clientAddr;
        SocketType clientSocket = NetworkUtils::acceptConnection(serverSocket, clientAddr);

        if (clientSocket != INVALID_SOCKET) {
            std::cout << "New connection from " << clientAddr << std::endl;

            const int clientId = connectionHandler_->addClient(clientSocket, clientAddr);

            std::string welcomeMsg = "Welcome to BLADE Server!\n";
            if (useAuth_) {
                welcomeMsg += "Please authenticate to continue.\n";
            } else {
                welcomeMsg += "Authentication is disabled.\n";
            }
            (void)connectionHandler_->sendToClient(clientId, welcomeMsg);

            // If auth is disabled, consider the device "logged in" immediately and print its IP once.
            if (!useAuth_) {
                std::lock_guard<std::mutex> lock(announcedMu);
                if (announced.insert(clientAddr).second) {
                    std::cout << "[CONNECTED DEVICE] " << clientAddr << " (auth disabled)\n";
                }
            }

            // If auth is enabled, print the device IP after successful login.
            // Wire this up to your actual auth flow:
            //  - wherever you currently verify username/password,
            //    call a method or callback that executes the block below.
            //
            // Example hook (pseudo):
            // connectionHandler_->onAuthenticated(clientId, [clientAddr]{ ... });
            //
            // --- BEGIN AUTH-SUCCESS PRINT BLOCK ---
            // {
            //   std::lock_guard<std::mutex> lock(announcedMu);
            //   if (announced.insert(clientAddr).second) {
            //     std::cout << "[LOGGED IN DEVICE] " << clientAddr << "\n";
            //   }
            // }
            // --- END AUTH-SUCCESS PRINT BLOCK ---

            std::cout << "Client " << clientId << " connected. Total clients: "
                      << connectionHandler_->getClientCount() << std::endl;
        }
    }

    NetworkUtils::closeSocket(serverSocket);
}

} // namespace blade
