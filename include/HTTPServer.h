#ifndef BLADE_HTTP_SERVER_H
#define BLADE_HTTP_SERVER_H

#include <string>
#include <atomic>
#include <thread>
#include "NetworkUtils.h"

namespace blade {

class Server;

/**
 * @brief HTTP server for serving web interface
 *
 * Serves HTML, CSS, and JavaScript files over HTTP.
 */
class HTTPServer {
public:
    /**
     * @brief Constructor
     * @param port Port number for HTTP server
     * @param webRoot Root directory for web files
     * @param server Pointer to the main server instance
     * @param useAuth Enable authentication
     * @param password Password for authentication
     */
    HTTPServer(int port, std::string webRoot, Server* server, bool useAuth = false, std::string password = "");

    /**
     * @brief Destructor
     */
    ~HTTPServer();
    
    /**
     * @brief Start the HTTP server
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * @brief Stop the HTTP server
     */
    void stop();
    
    /**
     * @brief Check if HTTPS server is running
     * @return true if running
     */
    [[nodiscard]] bool isRunning() const;

private:
    int port_;
    std::string webRoot_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    
    // Reference to main server
    Server* server_;

    // Authentication configuration
    bool useAuth_;
    std::string password_;

    void run();

    void handleRequest(SocketType clientSocket) const;
    static void setSocketTimeout(SocketType socket, int seconds = 5) ;
    static std::string getContentType(const std::string& path);
    static std::string loadFile(const std::string& path);
    [[nodiscard]] std::string getAuthConfig() const;
    [[nodiscard]] std::string getConnectedDevicesJson() const;
};

} // namespace blade

#endif // BLADE_HTTP_SERVER_H
