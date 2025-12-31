#include "Server.h"
#include "NetworkUtils.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace blade {

Server::Server(int port, bool useAuth)
    : port_(port), useAuth_(useAuth), running_(false) {
    authManager_ = std::make_unique<AuthenticationManager>();
    connectionHandler_ = std::make_unique<ConnectionHandler>();
    httpServer_ = std::make_unique<HTTPServer>(port + 1, "./web");
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
    
    std::cout << "BLADE Server started on port " << port_ << std::endl;
    std::cout << "Web interface available at http://" 
              << NetworkUtils::getLocalIPAddress() << ":" << (port_ + 1) << std::endl;
    std::cout << "Authentication: " << (useAuth_ ? "ENABLED" : "DISABLED") << std::endl;
    
    if (useAuth_) {
        std::cout << "Default credentials - Username: admin, Password: admin123" << std::endl;
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

void Server::setAuthRequired(bool useAuth) {
    useAuth_ = useAuth;
}

void Server::acceptConnections() {
    SocketType serverSocket = NetworkUtils::createSocket();
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
            
            int clientId = connectionHandler_->addClient(clientSocket, clientAddr);
            
            // Send welcome message
            std::string welcomeMsg = "Welcome to BLADE Server!\n";
            if (useAuth_) {
                welcomeMsg += "Please authenticate to continue.\n";
            } else {
                welcomeMsg += "Authentication is disabled.\n";
            }
            
            connectionHandler_->sendToClient(clientId, welcomeMsg);
            
            std::cout << "Client " << clientId << " connected. Total clients: " 
                      << connectionHandler_->getClientCount() << std::endl;
        }
    }
    
    NetworkUtils::closeSocket(serverSocket);
}

} // namespace blade
