#include "net_common.hpp"
#include "packet.hpp"
#include "tsqueue.hpp"

class FileManager {
public:
    explicit FileManager(const std::string& filePath) : file(filePath, std::ios::binary) {
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    std::future<std::shared_ptr<Packet<net::Connection>>> GetNextPacketAsync(std::shared_ptr<net::Connection> client) {
        return std::async(std::launch::async, [this, client]() {
            auto packet = std::make_shared<Packet<net::Connection>>();
            packet->remote = client;
            
            std::array<char, 1024> buffer;
            file.read(buffer.data(), buffer.size());
            std::memcpy(&packet->data, buffer.data(), sizeof(packet->data));
            return packet;
        });
    }
    
    bool hasMoreData() {
        return file.tellg() < fileSize;
    }
    
private:
    std::ifstream file;
    std::streampos fileSize;
};
