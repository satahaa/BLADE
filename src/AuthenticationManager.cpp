#include "AuthenticationManager.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>
namespace blade {

AuthenticationManager::~AuthenticationManager() = default;

bool AuthenticationManager::setPassword(const std::string& password) {
    std::lock_guard lock(mutex_);
    hashedPassword_ = hashPassword(password);
    return true;
}

std::string AuthenticationManager::authenticate(const std::string& password) {
    std::lock_guard lock(mutex_);
    if (hashedPassword_.empty() || hashedPassword_ != hashPassword(password)) {
        return ""; // Invalid password or not set
    }
    std::string token = generateToken();
    tokens_.insert(token);
    return token;
}

bool AuthenticationManager::validateToken(const std::string& token) {
    std::lock_guard lock(mutex_);
    return tokens_.contains(token);
}

void AuthenticationManager::invalidateToken(const std::string& token) {
    std::lock_guard lock(mutex_);
    tokens_.erase(token);
}

std::string AuthenticationManager::hashPassword(const std::string& password) {
    const std::string salted = password + "BLADE_SALT";
    uint64_t hash = 14695981039346656037ull;
    for (const char c : salted) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ull;
    }
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
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
