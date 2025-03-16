#include "../libs/net.hpp"
#include "../libs/packet.hpp"

class Server : public net::ServerInterface<Packet<net::Connection>> {
private:
    void HandleData(std::shared_ptr<Packet<net::Connection>> packet) override {
        
    }
};

int main() {
    Server server;
    server.Start();
    
    std::cout << "Hello, World!\n";
    return 0;
}
