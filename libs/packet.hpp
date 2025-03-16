#pragma once

template<typename T>
class Packet {
public:
    std::shared_ptr<T> remote = nullptr;
    
    class Data {
        char protocol[4];
        char responseCode[4];
        char dataProviderId[4];
        char dataReceiverId[4];
        char dataCode[7];
        char generatedData[8];
        char tradeDate[8];
        char round[4];
        char fileSize[10];
    };
    
    Data data;
};
