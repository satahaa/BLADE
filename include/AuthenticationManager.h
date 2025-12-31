#ifndef BLADE_AUTHENTICATION_MANAGER_H
#define BLADE_AUTHENTICATION_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>

namespace blade {

/**
 * @brief Manages user authentication
 * 
 * Handles user credentials, authentication tokens, and session management.
 */
class AuthenticationManager {
public:
    /**
     * @brief Constructor
     */
    AuthenticationManager();
    
    /**
     * @brief Destructor
     */
    ~AuthenticationManager();
    
    /**
     * @brief Add a user to the system
     * @param username Username
     * @param password Password
     * @return true if user was added successfully
     */
    bool addUser(const std::string& username, const std::string& password);
    
    /**
     * @brief Authenticate a user
     * @param username Username
     * @param password Password
     * @return Authentication token if successful, empty string otherwise
     */
    std::string authenticate(const std::string& username, const std::string& password);
    
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
    
    /**
     * @brief Check if a username exists
     * @param username Username to check
     * @return true if username exists
     */
    bool userExists(const std::string& username);

private:
    std::unordered_map<std::string, std::string> users_; // username -> hashed password
    std::unordered_map<std::string, std::string> tokens_; // token -> username
    std::mutex mutex_{};
    
    static std::string hashPassword(const std::string& password);

    static std::string generateToken();
};

} // namespace blade

#endif // BLADE_AUTHENTICATION_MANAGER_H
