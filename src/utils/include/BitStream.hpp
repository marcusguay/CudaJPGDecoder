#pragma once
#include <vector>
#include <stdint.h>
#include <iostream>
#include <bitset>

struct BitStreamStruct
{
    std::vector<uint8_t> &buffer;
    size_t indexInBuffer;
    uint8_t currentBufferValue;
    uint16_t value;
    int byteIndex;
    int bufferSize;
    BitStreamStruct(std::vector<uint8_t> &buf) : buffer(buf), value(0), currentBufferValue(buffer[0]), indexInBuffer(0), byteIndex(0), bufferSize(buffer.size()) {}
};

bool getNextBit(BitStreamStruct * bitstream);
void print(BitStreamStruct *bitstream);

