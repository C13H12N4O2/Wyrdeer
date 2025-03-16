#pragma once

#include "net_common.hpp"

namespace net {
class Socket {
public:
    Socket() : sockfd(-1) {}
    Socket(int sockfd) : sockfd(sockfd) {}
    
    ~Socket() {
        if (sockfd != -1) {
            CloseSocket();
        }
    }

    void CreateSocket() {
        sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        std::cout << "Socket created successfully!\n";
    }

    void BindSocket(const std::string &address, uint16_t port) {
        if (sockfd < 0) {
            throw std::runtime_error("Socket is not created");
        }
        
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(address.c_str());
        addr.sin_port = htons(port);
        
        int opt = 1;
        if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("Failed to set socket option");
        }
        
        if (::bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }
        
        std::cout << "Socket bound to " << address << ":" << port << "\n";
    }

    void ConnectToServer(const std::string &address, uint16_t port) {
        if (sockfd < 0) {
            throw std::runtime_error("Socket is not created");
        }
        
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(address.c_str());
        addr.sin_port = htons(port);
        
        if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Failed to connect to server");
        }
        
        std::cout << "Connected to " << address << ":" << port << "\n";
    }

    void ListenForConnections(int backlog) {
        if (sockfd < 0) {
            throw std::runtime_error("Socket is not created");
        }
        
        if (::listen(sockfd, backlog) < 0) {
            throw std::runtime_error("Failed to listen on socket");
        }
        
        std::cout << "Listening for incoming connection...\n";
    }

    std::shared_ptr<Socket> AcceptConnection() {
        if (sockfd < 0) {
            throw std::runtime_error("Socket is not created");
        }
        
        int clientSockFd = ::accept(sockfd, nullptr, nullptr);
        if (clientSockFd < 0) {
            throw std::runtime_error("Failed to accept connection");
        }
        
        std::cout << "[" << clientSockFd << "] Connection accepted!\n";
        
        return std::make_shared<Socket>(clientSockFd);
    }

    void CloseSocket() {
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
            std::cout << "Socket closed!\n";
        }
    }

    int GetSockFd() const {
        return sockfd;
    }

private:
    int sockfd;
    struct sockaddr_in addr;
};
}
