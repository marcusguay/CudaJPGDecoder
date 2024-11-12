#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <bitset>
#include "../../structs/HuffNode.hpp"
#include "../../structs/Node.hpp"
#include "../../utils/include/PriorityQueue.hpp"
#include "../../utils/include/treehelper.hpp"




class EncoderHuffmanTree
{

     


    std::pair<uint16_t,int> map[256];
    char lengthArray[16];
    std::vector<char> valueArray;
    std::vector<std::pair<char,int>> valueAndHeights;

    

    public:
    Node *Encode(std::vector<uint8_t> &buffer, int index, int length);

    void EncodeBuffer(std::vector<uint8_t> &buffer, std::vector<uint16_t> & newBuffer, Node *root);

    void recurse(Node *node, uint16_t bits, int height);

    void generateCodes();


    std::vector<uint8_t> getLengthsAndValues();

};