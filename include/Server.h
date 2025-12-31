#ifndef BLADE_SERVER_H
#define BLADE_SERVER_H

#include <string>
#include <memory>
#include <atomic>
#include "AuthenticationManager.h"
#include "ConnectionHandler.h"
#include "HTTPServer.h"

namespace blade {

/**
 * @brief Main Server class that manages network connections
 * 
 * This class handles opening network ports, accepting connections,
 * and managing the authentication and connection flow.
 */
class Server {
public:
    /**
     * @brief Constructor
     * @param port Port number to listen on
     * @param useAuth Enable/disable authentication
     */
    Server(int port, bool useAuth = true);
    
    /**
     * @brief Destructor
     */
    ~Server();
    
    /**
     * @brief Start the server
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool isRunning() const;
    
    /**
     * @brief Get the port number
     * @return Port number
     */
    int getPort() const;
    
    /**
     * @brief Set authentication requirement
     * @param useAuth Enable/disable authentication
     */
    void setAuthRequired(bool useAuth);

private:
    int port_;
    bool useAuth_;
    std::atomic<bool> running_;
    
    std::unique_ptr<AuthenticationManager> authManager_;
    std::unique_ptr<ConnectionHandler> connectionHandler_;
    std::unique_ptr<HTTPServer> httpServer_;
    
    void acceptConnections();
};

} // namespace blade

#endif // BLADE_SERVER_H
