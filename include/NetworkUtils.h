#ifndef BLADE_NETWORK_UTILS_H
#define BLADE_NETWORK_UTILS_H

#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

/**
 * @brief Network utility functions
 */
namespace blade::NetworkUtils {
    /**
     * @brief Initialize network subsystem (Windows only)
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Cleanup network subsystem (Windows only)
     */
    void cleanup();
    
    /**
     * @brief Create a TCP socket
     * @return Socket descriptor or INVALID_SOCKET on error
     */
    SocketType createSocket();
    
    /**
     * @brief Bind socket to a port
     * @param socket Socket descriptor
     * @param port Port number
     * @return true if successful
     */
    bool bindSocket(SocketType socket, int port);
    
    /**
     * @brief Listen on a socket
     * @param socket Socket descriptor
     * @param backlog Maximum pending connections
     * @return true if successful
     */
    bool listenSocket(SocketType socket, int backlog = 10);
    
    /**
     * @brief Accept a connection
     * @param socket Server socket descriptor
     * @param clientAddress Output for client address
     * @return Client socket descriptor or INVALID_SOCKET on error
     */
    SocketType acceptConnection(SocketType socket, std::string& clientAddress);
    
    /**
     * @brief Close a socket
     * @param socket Socket descriptor
     */
    void closeSocket(SocketType socket);
    
    /**
     * @brief Get local IP address
     * @return Local IP address
     */
    std::string getLocalIPAddress();
    
    /**
     * @brief Send data through socket
     * @param socket Socket descriptor
     * @param data Data to send
     * @return Number of bytes sent, or -1 on error
     */
    int sendData(SocketType socket, const std::string& data);
    
    /**
     * @brief Receive data from socket
     * @param socket Socket descriptor
     * @param buffer Buffer to receive data
     * @param maxSize Maximum buffer size
     * @return Number of bytes received, or -1 on error
     */
    int receiveData(SocketType socket, char* buffer, size_t maxSize);
}



#endif // BLADE_NETWORK_UTILS_H
