//
// Created by Sean on 2020/2/10.
//
#include <cstdint>
#include <stdio.h>
#include <iostream>

uint64_t bytesToInt64(char* bytes, int size = 8)
{
    uint64_t addr = bytes[0] & 0xFF;
    addr |= ((bytes[1] << 8) & 0xFF00);
    addr |= ((bytes[2] << 16) & 0xFF0000);
    addr |= ((bytes[3] << 24) & 0xFF000000);
    addr |= ((bytes[4] << 32) & 0xFF00000000);
    addr |= ((bytes[5] << 40) & 0xFF0000000000);
    addr |= ((bytes[6] << 48) & 0xFF000000000000);
    addr |= ((bytes[7] << 56) & 0xFF00000000000000);
    return addr;
}

uint32_t bytesToInt32(char* bytes, int size = 4)
{
    uint32_t addr = bytes[0] & 0xFF;
    addr |= ((bytes[1] << 8) & 0xFF00);
    addr |= ((bytes[2] << 16) & 0xFF0000);
    addr |= ((bytes[3] << 24) & 0xFF000000);
    return addr;
}

uint16_t bytesToInt16(char* bytes, int size = 2)
{
    uint16_t addr = bytes[0] & 0xFF;
    addr |= ((bytes[1] << 8) & 0xFF00);
    return addr;
}

uint8_t bytesToInt8(char* bytes, int size = 1)
{
    uint8_t addr = bytes[0] & 0xFF;
    return addr;
}

#pragma pack(push, 1)
typedef struct _sampleA{
    uint16_t pktSize;
    uint8_t  msgCount;
    char     filler[1];
    uint32_t seqNum;
    uint64_t sendTime;
};
#pragma pack(pop)

int main(int argc, char *argv[]) {
    char bytes1[2];
    bytes1[0] = -48;
    bytes1[1] = 1;

    char bytes2[2];
    bytes2[0] = 11;
    bytes2[1] = 0;

    uint16_t result = bytesToInt16(bytes2);
    uint32_t max = UINT32_MAX;
    std::cout << "the max value of uint32 is " << max << std::endl;
    max = max+2;
    std::cout << "the max value + 1 of uint32 is " << max << std::endl;
    std::cout << "pause...";
    return 0;
}

