#include "Server.h"
#include "NetworkUtils.h"
#include <iostream>
#include <string>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

// Global server instance for signal handling
std::unique_ptr<blade::Server> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_server) {
        // Give clients time to detect disconnection
        std::cout << "Notifying clients of shutdown..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        g_server->stop();
    }
    exit(0);
}

void printUsage(const char* programName) {
    std::cout << "BLADE - Bi-Directional LAN Asset Distribution Engine" << std::endl;
    std::cout << "\nUsage: " << programName << " [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -u, --username <user>  Username for authentication (requires -p)" << std::endl;
    std::cout << "  -p, --password <pass>  Password for authentication (requires -u)" << std::endl;
    std::cout << "  -h, --help             Show this help message" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << programName << "                                    (no authentication)" << std::endl;
    std::cout << "  " << programName << " -u admin -p mypassword             (with authentication)" << std::endl;
    std::cout << "  " << programName << " --username admin --password secret (with authentication)" << std::endl;
    std::cout << "\nNote: Web interface runs on port 80 (HTTP)" << std::endl;
    std::cout << "      File transfer server runs on port 8443" << std::endl;
    std::cout << "      Authentication is disabled by default - web interface will load directly" << std::endl;
    std::cout << "      When -u and -p are provided, web interface will show a login page" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "[DEBUG] BLADE starting..." << std::endl;
    std::cout.flush();

    // Port 8080 for file transfer server (web interface will use 80)
    constexpr int port = 8080;
    bool useAuth = false;  // Default to no authentication
    std::string username;
    std::string password;

    std::cout << "[DEBUG] Parsing arguments..." << std::endl;
    std::cout.flush();

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-u" || arg == "--username") {
            if (i + 1 < argc) {
                username = argv[++i];
            } else {
                std::cerr << "Error: --username requires a value" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
        } else if (arg == "-p" || arg == "--password") {
            if (i + 1 < argc) {
                password = argv[++i];
            } else {
                std::cerr << "Error: --password requires a value" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Enable authentication only if both username and password are provided
    if (!username.empty() && !password.empty()) {
        useAuth = true;
    } else if (!username.empty() || !password.empty()) {
        // If only one is provided, show error
        std::cerr << "Error: Both username and password must be provided together" << std::endl;
        std::cerr << "Use -u <username> -p <password> to enable authentication" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "[DEBUG] Signal handlers set up" << std::endl;
    std::cout.flush();

    // Print banner
    std::cout << "========================================" << std::endl;
    std::cout << "  BLADE - Local Network File Transfer  " << std::endl;
    std::cout << "  Version 1.0.0                         " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    std::cout << "[DEBUG] Creating server instance..." << std::endl;
    std::cout.flush();

    // Create and start server
    g_server = std::make_unique<blade::Server>(port, useAuth, username, password);

    std::cout << "[DEBUG] Server instance created, starting server..." << std::endl;
    std::cout.flush();

    if (!g_server->start()) {
        std::cerr << "Failed to start server" << std::endl;
        std::cerr << "Hint: Port 80 may require administrator privileges or be already in use." << std::endl;
        const auto ip = blade::NetworkUtils::getLocalIPAddress();
        std::cerr << "Detected LAN IP: " << ip << std::endl;
        std::cerr << "https://" << ip << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Ctrl+C to stop the server..." << std::endl;
    
    // Keep the main thread alive
    while (g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
