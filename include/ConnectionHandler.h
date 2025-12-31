#ifndef BLADE_CONNECTION_HANDLER_H
#define BLADE_CONNECTION_HANDLER_H

#include <vector>
#include <memory>
#include <string>
#include <mutex>

namespace blade {

/**
 * @brief Represents a connected client
 */
struct Client {
    int socket;
    std::string ipAddress;
    std::string token;
    bool authenticated;
    
    Client(int sock, const std::string& ip) 
        : socket(sock), ipAddress(ip), authenticated(false) {}
};

/**
 * @brief Manages connected clients
 * 
 * Handles client connections, data transfer, and disconnections.
 */
class ConnectionHandler {
public:
    /**
     * @brief Constructor
     */
    ConnectionHandler();
    
    /**
     * @brief Destructor
     */
    ~ConnectionHandler();
    
    /**
     * @brief Add a new client connection
     * @param socket Client socket descriptor
     * @param ipAddress Client IP address
     * @return Client ID
     */
    int addClient(int socket, const std::string& ipAddress);
    
    /**
     * @brief Remove a client connection
     * @param clientId Client ID
     */
    void removeClient(int clientId);
    
    /**
     * @brief Get number of connected clients
     * @return Number of clients
     */
    size_t getClientCount() const;
    
    /**
     * @brief Authenticate a client
     * @param clientId Client ID
     * @param token Authentication token
     */
    void authenticateClient(int clientId, const std::string& token);
    
    /**
     * @brief Send data to a client
     * @param clientId Client ID
     * @param data Data to send
     * @return Number of bytes sent
     */
    int sendToClient(int clientId, const std::string& data);
    
    /**
     * @brief Receive data from a client
     * @param clientId Client ID
     * @param buffer Buffer to receive data
     * @param maxSize Maximum buffer size
     * @return Number of bytes received
     */
    int receiveFromClient(int clientId, char* buffer, size_t maxSize);
    
    /**
     * @brief Get all connected clients
     * @return Vector of client pointers
     */
    std::vector<std::shared_ptr<Client>> getClients() const;

private:
    std::vector<std::shared_ptr<Client>> clients_;
    mutable std::mutex mutex_;
    int nextClientId_;
};

} // namespace blade

#endif // BLADE_CONNECTION_HANDLER_H
