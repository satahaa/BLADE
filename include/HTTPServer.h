#ifndef BLADE_HTTP_SERVER_H
#define BLADE_HTTP_SERVER_H

#include <string>
#include <atomic>
#include "NetworkUtils.h"

namespace blade {

/**
 * @brief HTTP server for serving web interface
 * 
 * Serves HTML, CSS, and JavaScript files for the web UI.
 */
class HTTPServer {
public:
    /**
     * @brief Constructor
     * @param port Port number for HTTP server
     * @param webRoot Root directory for web files
     */
    HTTPServer(int port, std::string  webRoot);
    
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
     * @brief Check if HTTP server is running
     * @return true if running
     */
    [[nodiscard]] bool isRunning() const;

private:
    int port_;
    std::string webRoot_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    
    void run();
    void handleRequest(SocketType clientSocket);
    static std::string getContentType(const std::string& path);
    static std::string loadFile(const std::string& path);
};

} // namespace blade

#endif // BLADE_HTTP_SERVER_H
