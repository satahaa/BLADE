#include "HTTPServer.h"
#include "NetworkUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

namespace blade {

HTTPServer::HTTPServer(const int port, std::string  webRoot)
    : port_(port), webRoot_(std::move(webRoot)), running_(false) {}

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
    SocketType serverSocket = NetworkUtils::createSocket();
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

void HTTPServer::handleRequest(SocketType clientSocket) {
    char buffer[4096];
    int bytesRead = NetworkUtils::receiveData(clientSocket, buffer, sizeof(buffer) - 1);
    
    if (bytesRead <= 0) {
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    // Parse HTTP request
    size_t methodEnd = request.find(' ');
    size_t pathEnd = request.find(' ', methodEnd + 1);
    
    if (methodEnd == std::string::npos || pathEnd == std::string::npos) {
        return;
    }
    
    std::string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    
    // Default to index.html
    if (path == "/") {
        path = "/index.html";
    }
    
    // Load file
    std::string filePath = webRoot_ + path;
    std::string content = loadFile(filePath);
    
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

std::string HTTPServer::loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace blade
