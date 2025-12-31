#include "Server.h"
#include "NetworkUtils.h"
#include <iostream>
#include <string>
#include <csignal>
#include <memory>

// Global server instance for signal handling
std::unique_ptr<blade::Server> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

void printUsage(const char* programName) {
    std::cout << "BLADE - Bi-Directional LAN Asset Distribution Engine" << std::endl;
    std::cout << "\nUsage: " << programName << " [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -p, --port <port>      Server port (default: 8080)" << std::endl;
    std::cout << "  -n, --no-auth          Disable authentication" << std::endl;
    std::cout << "  -h, --help             Show this help message" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << programName << " -p 9000" << std::endl;
    std::cout << "  " << programName << " --no-auth" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default settings
    int port = 8080;
    bool useAuth = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: --port requires a value" << std::endl;
                return 1;
            }
        } else if (arg == "-n" || arg == "--no-auth") {
            useAuth = false;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate port
    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Port must be between 1024 and 65535" << std::endl;
        return 1;
    }
    
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Print banner
    std::cout << "========================================" << std::endl;
    std::cout << "  BLADE - Local Network File Transfer  " << std::endl;
    std::cout << "  Version 1.0.0                         " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Create and start server
    g_server = std::make_unique<blade::Server>(port, useAuth);
    
    if (!g_server->start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Ctrl+C to stop the server..." << std::endl;
    
    // Keep the main thread alive
    while (g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
