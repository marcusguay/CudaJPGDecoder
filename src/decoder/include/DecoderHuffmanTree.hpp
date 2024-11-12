#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cmath>
#include <bitset>
#include "../../structs/HuffNode.hpp"
#include "../../structs/Node.hpp"
#include "../../utils/include/treehelper.hpp"
#include "../../utils/include/BitStream.hpp"



/* Hash function for the Huffman tree maps since we 
   we need to treat values differently based on their
   height in the tree (we cant just compare values) */
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ (hash2 << 1); 
    }
};

class DecoderHuffmanTree
{
    Node *root;
    int lastIndex = 0;
    
public:
    std::unordered_map<std::pair<uint16_t, int>, int, pair_hash> map;
    DecoderHuffmanTree();
    int createTree(std::vector<uint8_t> &buffer, int index);
    uint16_t getBinaryRepresentation(int index, char value);
    void decode(std::vector<uint8_t> &encodedBuffer, std::vector<uint8_t> &decodedBuffer);
    int decodeOne(uint16_t var,  uint16_t * decodedInt);
    int getLastIndex();
    void recurse(Node *node, uint16_t bits, int height);
};