#include "../libs/net.hpp"
#include "../libs/packet.hpp"
#include "../libs/net_filemanager.hpp"

class Server : public net::ServerInterface<Packet<net::Connection>> {
private:
    void HandleData(std::shared_ptr<Packet<net::Connection>> packet) override {
        auto connection = packet->remote;
        
        if (std::memcmp(packet->data.transactionCode, "ACKS", sizeof(packet->data.transactionCode)) == 0) {
            packet->data.transactionCode[3] = 'R';
            packet->data.reserved[0] = 0x00;
        } else if (std::memcmp(packet->data.transactionCode, "HEAD", sizeof(packet->data.transactionCode)) == 0) {
            m_activeFileTransfers[connection->GetSockFd()] = std::make_shared<FileManager>("/Users/espurr208/Documents/Wyrdeer/Wyrdeer.png");
            std::memcpy(packet->data.responseCode, "0000", sizeof(packet->data.responseCode));
            packet->data.reserved[0] = 0x00;
            
            auto it = m_activeFileTransfers.find(connection->GetSockFd());
            if (it == m_activeFileTransfers.end())
                return;
            
            auto fileManager = it->second;
            if (fileManager->hasMoreData()) {
                auto futurePacket = fileManager->GetNextPacketAsync(connection);
                packet->data = std::move(futurePacket.get()->data);
            } else {
                std::memcpy(packet->data.transactionCode, "TAIL", sizeof(packet->data.transactionCode));
                m_activeFileTransfers.erase(connection->GetSockFd());
            }
        } else {
            std::cout << "Unexpected transaction code\n";
            RemoveConnection(connection->GetSockFd());
            connection->CloseSocket();
            sleep(1);
        }
        
        m_messageQueue.emplace_back(packet);
    }
    
    std::unordered_map<int, std::shared_ptr<FileManager>> m_activeFileTransfers;
};

int main() {
    Server server;
    server.Start();
    
    std::cout << "Hello, World!\n";
    return 0;
}
