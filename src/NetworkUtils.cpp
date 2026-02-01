#include "NetworkUtils.h"
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <iphlpapi.h>
  // MinGW/ld won't honor this pragma; linking must be done via build system.
  // #pragma comment(lib, "iphlpapi.lib")
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <cerrno>
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

bool bindSocket(SocketType socket, const int port) {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    int opt = 1;
#ifdef _WIN32
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // Don't use SOCKET_ERROR here (not reliably defined in MinGW builds for POSIX-like sockets).
    return ::bind(socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != -1;
}

bool listenSocket(const SocketType socket, const int backlog) {
    // Don't use SOCKET_ERROR here either.
    return ::listen(socket, backlog) != -1;
}

SocketType acceptConnection(const SocketType socket, std::string& clientAddress) {
    sockaddr_in clientAddr{};
#ifdef _WIN32
    int clientAddrLen = sizeof(clientAddr); // WinSock accept() uses int*
    const SocketType clientSocket = ::accept(socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
#else
    socklen_t clientAddrLen = sizeof(clientAddr);
    SocketType clientSocket = ::accept(socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
#endif

    if (clientSocket != INVALID_SOCKET) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
        clientAddress = std::string(ip);
    }

    return clientSocket;
}

SocketType acceptConnectionWithTimeout(const SocketType socket, std::string& clientAddress, int timeoutMs) {
#ifdef _WIN32
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
    timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int result = select(static_cast<int>(socket) + 1, &readfds, nullptr, nullptr, &tv);
    if (result > 0 && FD_ISSET(socket, &readfds)) {
        return acceptConnection(socket, clientAddress);
    }
    return INVALID_SOCKET;
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
    timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int result = select(socket + 1, &readfds, nullptr, nullptr, &tv);
    if (result > 0 && FD_ISSET(socket, &readfds)) {
        return acceptConnection(socket, clientAddress);
    }
    return INVALID_SOCKET;
#endif
}

void closeSocket(SocketType socket) {
    if (socket != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }
}

static bool isPrivateIPv4(const std::string& ip) {
    // Minimal checks for common private ranges
    return ip.rfind("10.", 0) == 0 ||
           ip.rfind("192.168.", 0) == 0 ||
           (ip.rfind("172.", 0) == 0 && ip.size() > 4 && (ip[4] >= '1' && ip[4] <= '3')); // 172.16-172.31 (coarse)
}

std::string getLocalIPAddress() {
#ifdef _WIN32
    // MinGW-friendly: use GetAdaptersInfo (older API) instead of GetAdaptersAddresses.
    ULONG bufLen = 0;
    if (GetAdaptersInfo(nullptr, &bufLen) != ERROR_BUFFER_OVERFLOW || bufLen == 0) {
        return "127.0.0.1";
    }

    std::string best = "127.0.0.1";
    std::string wifiIP = "";
    std::string ethernetIP = "";

    auto* info = reinterpret_cast<IP_ADAPTER_INFO*>(std::malloc(bufLen));
    if (!info) return "127.0.0.1";

    if (GetAdaptersInfo(info, &bufLen) == NO_ERROR) {
        for (auto* a = info; a != nullptr; a = a->Next) {
            const char* ipStr = a->IpAddressList.IpAddress.String;
            if (!ipStr || !*ipStr) continue;

            std::string ip(ipStr);
            if (ip == "0.0.0.0") continue;
            if (ip.rfind("127.", 0) == 0) continue;

            // Check if this is a valid private IP
            if (!isPrivateIPv4(ip)) continue;

            // Get adapter description to detect WiFi vs Ethernet
            std::string desc(a->Description);
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

            // Prioritize WiFi adapters
            if (desc.find("wi-fi") != std::string::npos ||
                desc.find("wifi") != std::string::npos ||
                desc.find("wireless") != std::string::npos ||
                desc.find("802.11") != std::string::npos) {
                wifiIP = ip;
            } else if (desc.find("ethernet") != std::string::npos ||
                       desc.find("eth") != std::string::npos) {
                if (ethernetIP.empty()) ethernetIP = ip;
            } else {
                // Generic adapter - keep as fallback
                if (best == "127.0.0.1") best = ip;
            }
        }
    }

    std::free(info);

    // Priority: WiFi > Ethernet > Any other valid IP > localhost
    if (!wifiIP.empty()) return wifiIP;
    if (!ethernetIP.empty()) return ethernetIP;
    return best;
#else
    char hostName[256];
    // POSIX gethostname returns 0 on success, -1 on error (no SOCKET_ERROR here)
    if (gethostname(hostName, sizeof(hostName)) != 0) {
        return "127.0.0.1";
    }

    struct addrinfo hints{}, *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostName, nullptr, &hints, &result) != 0 || result == nullptr) {
        return "127.0.0.1";
    }

    std::string ipAddress = "127.0.0.1";

    // Get the first non-loopback IP address
    for (const addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
            const auto* sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, INET_ADDRSTRLEN);

            std::string currentIP(ip);

            // Skip loopback addresses (127.x.x.x)
            if (currentIP.rfind("127.", 0) == 0) {
                continue;
            }

            // Use the first non-loopback IP found
            ipAddress = currentIP;
            break;
        }
    }

    freeaddrinfo(result);

    return ipAddress;
#endif
}

int sendData(const SocketType socket, const std::string& data) {
    const char* ptr = data.c_str();
    size_t remaining = data.length();
    size_t totalSent = 0;

    while (remaining > 0) {
#ifdef _WIN32
        int sent = send(socket, ptr + totalSent, static_cast<int>(remaining), 0);
#else
        ssize_t sent = send(socket, ptr + totalSent, remaining, 0);
#endif
        if (sent <= 0) {
            return static_cast<int>(totalSent > 0 ? totalSent : sent);
        }
        totalSent += static_cast<size_t>(sent);
        remaining -= static_cast<size_t>(sent);
    }

    return static_cast<int>(totalSent);
}

int receiveData(const SocketType socket, char* buffer, const size_t maxSize) {
#ifdef _WIN32
    return recv(socket, buffer, static_cast<int>(maxSize), 0);
#else
    return recv(socket, buffer, maxSize, 0);
#endif
}

bool sendAll(const SocketType socket, const void* data, const size_t len) {
    const auto p = static_cast<const char*>(data);
    size_t sent = 0;
    while (sent < len) {
#ifdef _WIN32
        const int n = ::send(socket, p + sent, static_cast<int>(len - sent), 0);
#else
        ssize_t n = ::send(socket, p + sent, len - sent, 0);
#endif
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

bool recvAll(const SocketType socket, void* data, const size_t len) {
    const auto p = static_cast<char*>(data);
    size_t recvd = 0;
    while (recvd < len) {
#ifdef _WIN32
        const int n = ::recv(socket, p + recvd, static_cast<int>(len - recvd), 0);
#else
        ssize_t n = ::recv(socket, p + recvd, len - recvd, 0);
#endif
        if (n <= 0) return false;
        recvd += static_cast<size_t>(n);
    }
    return true;
}


} // namespace blade::NetworkUtils
