#pragma once

#include "net_common.hpp"
#include "net_socket.hpp"
#include "packet.hpp"

namespace net {
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(std::shared_ptr<Socket> socket) : m_socket(socket) {
        
    }

    ~Connection() {
        CloseSocket();
    }

    int GetSockFd() const {
        return m_socket->GetSockFd();
    }

    void Send(const std::shared_ptr<std::string> &buffer) {
        ::send(m_socket->GetSockFd(), buffer->data(), buffer->size(), 0);
    }

    std::shared_ptr<std::string> Receive() {
        auto buffer = std::make_shared<std::string>(1024, 0x00);
        ssize_t bytesRead = ::recv(m_socket->GetSockFd(), buffer->data(), 1024, 0);
        if (bytesRead <= 0) {
            throw std::runtime_error("Client disconnected");
        }
        
        buffer->resize(bytesRead);
        std::cout << "[" << m_socket->GetSockFd() << "] " << buffer->data() << "\n";
        
        return buffer;
    }
    
    void CloseSocket() {
        m_socket->CloseSocket();
    }
    
private:
    std::shared_ptr<Socket> m_socket;
};
}
