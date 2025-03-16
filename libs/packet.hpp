#pragma once


class Header {
public:
    Header() {
        std::memset(this, 0x20, sizeof(*this));
    }
    
    char transactionCode[4];
    char responseCode[4];
    char dataProviderId[4];
    char dataReceiverId[4];
    char dataCode[7];
    char date[14];
    char round[4];
    char fileSize[10];
    char reserved[965];
};

template<typename T>
class Packet {
public:
    Packet() = default;
    
    ~Packet() {}
    std::shared_ptr<T> remote = nullptr;
    Header data;
};
