#include "ConnectionHandler.h"
#include "NetworkUtils.h"
#include <algorithm>

namespace blade {

ConnectionHandler::ConnectionHandler() : nextClientId_(0) {}

ConnectionHandler::~ConnectionHandler() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& client : clients_) {
        if (client && client->socket != INVALID_SOCKET) {
            NetworkUtils::closeSocket(client->socket);
        }
    }
    clients_.clear();
}

int ConnectionHandler::addClient(int socket, const std::string& ipAddress) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto client = std::make_shared<Client>(socket, ipAddress);
    clients_.push_back(client);
    
    return nextClientId_++;
}

void ConnectionHandler::removeClient(int clientId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (clientId >= 0 && clientId < static_cast<int>(clients_.size())) {
        auto& client = clients_[clientId];
        if (client && client->socket != INVALID_SOCKET) {
            NetworkUtils::closeSocket(client->socket);
            client->socket = INVALID_SOCKET;
        }
    }
}

size_t ConnectionHandler::getClientCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::count_if(clients_.begin(), clients_.end(), 
        [](const std::shared_ptr<Client>& c) { 
            return c && c->socket != INVALID_SOCKET; 
        });
}

void ConnectionHandler::authenticateClient(int clientId, const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (clientId >= 0 && clientId < static_cast<int>(clients_.size())) {
        auto& client = clients_[clientId];
        if (client) {
            client->token = token;
            client->authenticated = true;
        }
    }
}

int ConnectionHandler::sendToClient(int clientId, const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (clientId >= 0 && clientId < static_cast<int>(clients_.size())) {
        auto& client = clients_[clientId];
        if (client && client->socket != INVALID_SOCKET) {
            return NetworkUtils::sendData(client->socket, data);
        }
    }
    return -1;
}

int ConnectionHandler::receiveFromClient(int clientId, char* buffer, size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (clientId >= 0 && clientId < static_cast<int>(clients_.size())) {
        auto& client = clients_[clientId];
        if (client && client->socket != INVALID_SOCKET) {
            return NetworkUtils::receiveData(client->socket, buffer, maxSize);
        }
    }
    return -1;
}

std::vector<std::shared_ptr<Client>> ConnectionHandler::getClients() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Client>> activeClients;
    
    for (const auto& client : clients_) {
        if (client && client->socket != INVALID_SOCKET) {
            activeClients.push_back(client);
        }
    }
    
    return activeClients;
}

} // namespace blade
