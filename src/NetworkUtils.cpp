#include "NetworkUtils.h"
#include <cstring>
#include <iostream>

namespace blade {

namespace NetworkUtils {

bool initialize() {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true; // Not needed on Unix-like systems
#endif
}

void cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

SocketType createSocket() {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

bool bindSocket(SocketType socket, int port) {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    // Set socket option to reuse address
    int opt = 1;
#ifdef _WIN32
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    return bind(socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR;
}

bool listenSocket(SocketType socket, int backlog) {
    return listen(socket, backlog) != SOCKET_ERROR;
}

SocketType acceptConnection(SocketType socket, std::string& clientAddress) {
    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    
#ifdef _WIN32
    SocketType clientSocket = accept(socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
#else
    SocketType clientSocket = accept(socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
#endif
    
    if (clientSocket != INVALID_SOCKET) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
        clientAddress = std::string(ip);
    }
    
    return clientSocket;
}

void closeSocket(SocketType socket) {
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
    }
}

std::string getLocalIPAddress() {
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
        return "127.0.0.1";
    }
    
    struct hostent* host = gethostbyname(hostName);
    if (host == nullptr) {
        return "127.0.0.1";
    }
    
    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
    return std::string(inet_ntoa(addr));
}

int sendData(SocketType socket, const std::string& data) {
#ifdef _WIN32
    return send(socket, data.c_str(), static_cast<int>(data.length()), 0);
#else
    return send(socket, data.c_str(), data.length(), 0);
#endif
}

int receiveData(SocketType socket, char* buffer, size_t maxSize) {
#ifdef _WIN32
    return recv(socket, buffer, static_cast<int>(maxSize), 0);
#else
    return recv(socket, buffer, maxSize, 0);
#endif
}

} // namespace NetworkUtils

} // namespace blade
