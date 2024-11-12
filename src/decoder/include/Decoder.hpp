
#pragma once
#include "./DecoderHuffmanTree.hpp"
#include "../../structs/DecoderStructs.hpp"
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstring> 
#include <cmath>
#include <algorithm>
#include "../../cuda/include/GPUDecoder.hpp"



class Decoder{

public:
void decode(std::string imagePath);

private:
ImageSpecification imageSpecification;
DecoderHuffmanTree* ACTrees[4];
DecoderHuffmanTree* DCTrees[4];
QuantizationTable quantizationTables[4]; 
int quantizationTableMapping [4];
std::pair<int,int> componentHuffmanTableIndex [3];



void decodeHuffmanTable(std::vector<uint8_t> & buffer, int index);
void decodeStartOfScan(std::vector<uint8_t> & buffer, int index);
void decodeQuantizationTable(std::vector<uint8_t> & buffer, int index); 
void decodeStartOfFrame(std::vector<uint8_t> & buffer, int index); 
};