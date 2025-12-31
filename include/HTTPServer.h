#ifndef BLADE_HTTP_SERVER_H
#define BLADE_HTTP_SERVER_H

#include <string>
#include <atomic>
#include <thread>
#include "NetworkUtils.h"

#ifdef _WIN32
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <wincrypt.h>
#include <schannel.h>
#include <security.h>
#endif

namespace blade {

/**
 * @brief HTTPS server for serving web interface
 *
 * Serves HTML, CSS, and JavaScript files over secure connection.
 */
class HTTPServer {
public:
    /**
     * @brief Constructor
     * @param port Port number for HTTPS server
     * @param webRoot Root directory for web files
     */
    HTTPServer(int port, std::string webRoot);

    /**
     * @brief Destructor
     */
    ~HTTPServer();
    
    /**
     * @brief Start the HTTPS server
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * @brief Stop the HTTPS server
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
    bool useHttps_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    
#ifdef _WIN32
    HCERTSTORE certStore_;
    PCCERT_CONTEXT certContext_;
    CredHandle credHandle_;
    bool sslInitialized_;

    bool initializeSSL();
    void cleanupSSL();
    bool createSelfSignedCert();
#endif

    void run();

    void handleRequest(SocketType clientSocket) const;
    static std::string getContentType(const std::string& path);
    static std::string loadFile(const std::string& path);
};

} // namespace blade

#endif // BLADE_HTTP_SERVER_H
