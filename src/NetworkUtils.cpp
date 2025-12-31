#include "NetworkUtils.h"
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#ifndef _WIN32
    #include <netdb.h>
#endif

namespace blade::NetworkUtils {

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
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

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
    
    struct addrinfo hints{}, *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostName, nullptr, &hints, &result) != 0 || result == nullptr) {
        return "127.0.0.1";
    }

    std::string ipAddress = "127.0.0.1";
    std::string fallbackIP;

    // Iterate through all available network interfaces
    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
            auto* sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, INET_ADDRSTRLEN);

            std::string currentIP(ip);

            // Skip loopback addresses (127.x.x.x)
            if (currentIP.rfind("127.", 0) == 0) {
                continue;
            }

            // Check for common virtual adapter IP ranges
            bool isVirtual = false;

            // VMware/VirtualBox typically use 192.168.x.x
            if (currentIP.rfind("192.168.", 0) == 0) {
                isVirtual = true;
            }

            // Docker and some VPNs use 172.16.x.x - 172.31.x.x
            if (currentIP.rfind("172.", 0) == 0 && currentIP.length() > 4) {
                int secondOctet = std::atoi(currentIP.substr(4, 2).c_str());
                if (secondOctet >= 16 && secondOctet <= 31) {
                    isVirtual = true;
                }
            }

            // APIPA/Link-local addresses (169.254.x.x)
            if (currentIP.rfind("169.254.", 0) == 0) {
                isVirtual = true;
            }

            if (isVirtual) {
                // Keep as fallback if no better option found
                if (fallbackIP.empty()) {
                    fallbackIP = currentIP;
                }
                continue;
            }

            // Found a preferred non-virtual IP (e.g., 10.x.x.x)
            ipAddress = currentIP;
            break;
        }
    }

    freeaddrinfo(result);

    // Use fallback if no preferred IP was found
    if (ipAddress == "127.0.0.1" && !fallbackIP.empty()) {
        return fallbackIP;
    }

    return ipAddress;
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

} // namespace blade::NetworkUtils
