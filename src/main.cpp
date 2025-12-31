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
        g_server->stop();
    }
    exit(0);
}

void printUsage(const char* programName) {
    std::cout << "BLADE - Bi-Directional LAN Asset Distribution Engine" << std::endl;
    std::cout << "\nUsage: " << programName << " -u <username> -p <password> [options]" << std::endl;
    std::cout << "       " << programName << " --no-auth" << std::endl;
    std::cout << "\nRequired (when authentication enabled):" << std::endl;
    std::cout << "  -u, --username <user>  Username for authentication" << std::endl;
    std::cout << "  -p, --password <pass>  Password for authentication" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -n, --no-auth          Disable authentication (no username/password needed)" << std::endl;
    std::cout << "  -h, --help             Show this help message" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << programName << " -u admin -p mypassword" << std::endl;
    std::cout << "  " << programName << " --username admin --password mypassword" << std::endl;
    std::cout << "  " << programName << " --no-auth" << std::endl;
    std::cout << "\nNote: Server runs on port 443 (HTTPS) and requires administrator/root privileges" << std::endl;
}

int main(int argc, char* argv[]) {
    // Fixed port 443 for HTTPS
    constexpr int port = 443;
    bool useAuth = true;
    std::string username;
    std::string password;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-n" || arg == "--no-auth") {
            useAuth = false;
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

    // Validate authentication requirements
    if (useAuth) {
        if (username.empty() || password.empty()) {
            std::cerr << "Error: Username and password are required when authentication is enabled" << std::endl;
            std::cerr << "Use -u <username> -p <password>, or use --no-auth to disable authentication" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
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
    g_server = std::make_unique<blade::Server>(port, useAuth, username, password);

    if (!g_server->start()) {
        std::cerr << "Failed to start server" << std::endl;
        std::cerr << "Hint: Port 443 typically requires administrator privileges." << std::endl;
        const auto ip = blade::NetworkUtils::getLocalIPAddress();
        std::cerr << "Detected LAN IP: " << ip << std::endl;
        std::cerr << "Try: https://" << ip << ":" << port << std::endl;
        return 1;
    }
    
    std::cout << "\nPress Ctrl+C to stop the server..." << std::endl;
    
    // Keep the main thread alive
    while (g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
