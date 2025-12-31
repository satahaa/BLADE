#include "AuthenticationManager.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>

namespace blade {

AuthenticationManager::AuthenticationManager() {
    // Add a default admin user for demo purposes
    addUser("admin", "admin123");
}

AuthenticationManager::~AuthenticationManager() = default;

bool AuthenticationManager::addUser(const std::string& username, const std::string& password) {
    std::lock_guard lock(mutex_);
    
    if (users_.find(username) != users_.end()) {
        return false; // User already exists
    }
    
    users_[username] = hashPassword(password);
    return true;
}

std::string AuthenticationManager::authenticate(const std::string& username, const std::string& password) {
    std::lock_guard lock(mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return ""; // User not found
    }
    
    if (it->second != hashPassword(password)) {
        return ""; // Invalid password
    }
    
    // Generate and store token
    std::string token = generateToken();
    tokens_[token] = username;
    return token;
}

bool AuthenticationManager::validateToken(const std::string& token) {
    std::lock_guard lock(mutex_);
    return tokens_.find(token) != tokens_.end();
}

void AuthenticationManager::invalidateToken(const std::string& token) {
    std::lock_guard lock(mutex_);
    tokens_.erase(token);
}

bool AuthenticationManager::userExists(const std::string& username) {
    std::lock_guard lock(mutex_);
    return users_.find(username) != users_.end();
}

std::string AuthenticationManager::hashPassword(const std::string& password) {
    // Simple hash for demonstration - in production, use bcrypt or similar
    std::hash<std::string> hasher;
    size_t hash = hasher(password + "BLADE_SALT");
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

std::string AuthenticationManager::generateToken() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string token;
    token.reserve(32);
    
    for (int i = 0; i < 32; ++i) {
        const auto hexChars = "0123456789abcdef";
        token += hexChars[dis(gen)];
    }
    
    return token;
}

} // namespace blade
