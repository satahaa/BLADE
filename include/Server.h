#ifndef BLADE_SERVER_H
#define BLADE_SERVER_H

#include <memory>
#include <atomic>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <filesystem>
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
     * @param password Password for authentication (required if useAuth is true)
     */
    explicit Server(int port, bool useAuth = true, const std::string& password = "");

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
     * @brief Set the download directory for uploaded files
     * @param path Path to the download directory
     */
    void setDownloadDirectory(const std::string& path);

    /**
     * @brief Get the download directory
     * @return Path to the download directory
     */
    std::string getDownloadDirectory() const;

    /**
     * @brief Handle file upload
     * @param filename Name of the uploaded file
     * @param data File data as byte vector
     * @return true if upload was successful
     */
    bool handleUpload(const std::string& filename, const std::vector<uint8_t>& data) const;
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    [[nodiscard]] bool isRunning() const;
    
    /**
     * @brief Get the port number
     * @return Port number
     */
    [[nodiscard]] int getPort() const;
    
    /**
     * @brief Set authentication requirement
     * @param useAuth Enable/disable authentication
     */
    void setAuthRequired(bool useAuth);

    /**
     * @brief Get list of connected device IPs
     * @return Vector of connected IP addresses
     */
    std::vector<std::string> getConnectedDevices() const;

    /**
     * @brief Track an HTTP client connection
     * @param clientIP IP address of the HTTP client
     */
    void trackHTTPConnection(const std::string& clientIP);

    /**
     * @brief Handle heartbeat endpoint request
     * @param clientIP IP address of the client making the request
     * @return JSON response string
     */
    std::string handleHeartbeat(const std::string& clientIP);

private:
    int port_;
    bool useAuth_;
    std::string downloadDir_;
    std::atomic<bool> running_;
    
    std::unique_ptr<AuthenticationManager> authManager_;
    std::unique_ptr<ConnectionHandler> connectionHandler_;
    std::unique_ptr<HTTPServer> httpServer_;
    
    // Track connected client IPs for clean logging
    std::unordered_set<std::string> connectedIPs_;

    // Track HTTP client IPs with last activity timestamp (for active session tracking)
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> httpClientActivity_;

    mutable std::mutex ipMutex_;
    mutable std::mutex stopMutex_;
    std::condition_variable stopCv_;

    std::thread acceptThread_;
    std::thread cleanupThread_;

    void acceptConnections();
    void cleanupInactiveHTTPClients();
};

} // namespace blade

#endif // BLADE_SERVER_H
