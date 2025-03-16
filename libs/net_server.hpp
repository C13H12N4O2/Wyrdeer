#pragma once

#include "tsqueue.hpp"
#include "net_common.hpp"
#include "net_server.hpp"
#include "net_connection.hpp"

namespace net {
template<typename T>
class ServerInterface {
public:
    ServerInterface() {}

    ~ServerInterface() {
        StopThreads();
        Stop();
    }

    void Start() {
        m_socket.CreateSocket();
        m_socket.BindSocket("127.0.0.1", 8080);
        m_socket.ListenForConnections(5);
        CreateEventQueue();
        RegisterSocketEvent();
        CreateThreads();
        WaitForClientConnection();
    }

    void Stop() {
        m_socket.CloseSocket();
    }
    
private:
    void WaitForClientConnection() {
        struct kevent events[10];
        
        while (true) {
            int count = kevent(m_kqueue, nullptr, 0, events, 10, nullptr);
            if (count < 0) {
                sleep(1);
                throw std::runtime_error("Failed to kevent");
            }
            
            for (int i = 0; i != count; ++i) {
                if (events[i].ident == static_cast<uintptr_t>(m_socket.GetSockFd())) {
                    AcceptClientConnection();
                } else {
                    auto connection = FindConnection(events[i].ident);
                    if (connection == nullptr) {
                        continue;
                    }
                    
                    try {
                        if (events[i].filter == EVFILT_READ) {
                            auto buffer = connection->Receive();
                            auto packet = std::make_shared<Packet<Connection>>();
                            packet->remote = connection;
                            std::memcpy(&packet->data, buffer->data(), std::min(sizeof(Packet<Connection>::Data), buffer->size()));
                            HandleData(packet);
                            m_messageQueue.emplace_back(packet);
                        } else if (events[i].filter == EVFILT_TIMER) {
                            std::cout << "TIME OUT\n";
                        } else {
                            std::cout << "Unexpected result ocurred\n";
                            connection->CloseSocket();
                        }
                        
                        RegisterTimeoutEvent(connection->GetSockFd());
                    } catch (const std::exception& e) {
                        std::cerr << "Connection error: " << e.what() << "\n";
                        RemoveConnection(connection->GetSockFd());
                    }
                }
            }
        }
    }

    void AcceptClientConnection() {
        auto socket = m_socket.AcceptConnection();
        auto connection = std::make_shared<Connection>(socket);
        m_connections[socket->GetSockFd()] = connection;
        RegisterPacketEvent(socket->GetSockFd());
        RegisterTimeoutEvent(socket->GetSockFd());
    }
    
    void MessageClient() {
        while (true) {
            if (m_messageQueue.empty()) continue;
            
            auto packet = m_messageQueue.pop_front();
            auto connection = packet->remote;
            
            if (!connection) {
                std::cerr << "Invalid connection\n";
                continue;
            }
            
            auto data = std::make_shared<std::string>(reinterpret_cast<char*>(&packet->data), sizeof(Packet<Connection>::Data));
            connection->Send(data);
        }
    }

    void CreateEventQueue() {
        m_kqueue = kqueue();
        if (m_kqueue < 0) {
            throw std::runtime_error("Failed to craete kqueue");
        }
    }

    std::shared_ptr<net::Connection> FindConnection(uintptr_t sockfd) {
        auto it = m_connections.find(static_cast<int>(sockfd));
        return (it != m_connections.end()) ? it->second : nullptr;
    }

    void RemoveConnection(const int sockfd) {
        m_connections.erase(sockfd);
    }

    void RegisterSocketEvent() {
        struct kevent event;
        EV_SET(&event, m_socket.GetSockFd(), EVFILT_READ, EV_ADD, 0, 0, nullptr);
        if (kevent(m_kqueue, &event, 1, nullptr, 0, nullptr) < 0) {
            throw std::runtime_error("Failed to register event");
        }
    }
    
    void RegisterPacketEvent(const int sockfd) {
        struct kevent event;
        EV_SET(&event, sockfd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
        kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
    }
    
    void RegisterTimeoutEvent(const int sockfd) {
        struct kevent event;
        EV_SET(&event, sockfd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, m_timeout, nullptr);
        kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
    }
    
    void CreateThreads() {
        for (size_t i = 0; i != m_threadCount; ++i) {
            m_threads.emplace_back([this]() {
                while (m_running) {
                    this->MessageClient();
                }
            });
        }
    }
    
    void StopThreads() {
        if (!m_running) return;
        m_running = false;
        for (auto &t : m_threads) {
            if (t.joinable()) t.join();
        }
    }
    
protected:
    virtual void HandleData(std::shared_ptr<T> packet) {
        
    }
    
protected:
    Socket m_socket;
    int m_kqueue;
    intptr_t m_timeout = 5000;
    std::atomic<bool> m_running = true;
    size_t m_threadCount = 2;
    tsqueue<std::shared_ptr<T>> m_messageQueue;
    std::vector<std::thread> m_threads;
    std::unordered_map<int, std::shared_ptr<Connection>> m_connections;
};
}
