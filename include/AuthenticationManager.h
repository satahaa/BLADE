#ifndef BLADE_AUTHENTICATION_MANAGER_H
#define BLADE_AUTHENTICATION_MANAGER_H

#include <string>
#include <mutex>
#include <unordered_set>

namespace blade {

/**
 * @brief Manages user authentication
 * 
 * Handles user credentials, authentication tokens, and session management.
 */
class AuthenticationManager {
public:
    
    /**
     * @brief Destructor
     */
    ~AuthenticationManager();

    /**
     * @brief Add a new user with password
     * @param password User password
     * @return true if user added successfully
     */
    bool setPassword(const std::string& password);
    
    /**
     * @brief Authenticate a user
     * @param password Password
     * @return Authentication token if successful, empty string otherwise
     */
    std::string authenticate(const std::string& password);
    
    /**
     * @brief Validate an authentication token
     * @param token Authentication token
     * @return true if token is valid
     */
    bool validateToken(const std::string& token);
    
    /**
     * @brief Invalidate a token (logout)
     * @param token Authentication token
     */
    void invalidateToken(const std::string& token);

private:
    std::string hashedPassword_; //hashed password
    std::unordered_set<std::string> tokens_; // token set
    std::mutex mutex_{};
    
    static std::string hashPassword(const std::string& password);

    static std::string generateToken();
};

} // namespace blade

#endif // BLADE_AUTHENTICATION_MANAGER_H
