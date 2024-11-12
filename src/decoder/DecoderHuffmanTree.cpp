#include "./include/DecoderHuffmanTree.hpp"

DecoderHuffmanTree::DecoderHuffmanTree()
{
    /* idk why this is here*/
}

bool insert_node(Node *&root, char element, int pos)
{

    /* Inserts node into tree based on its position
     return true if element has been correctly inserted
    */

    if (!root)
    {
        root = new Node{0, char(0), nullptr, nullptr, true};
    }

    if (!root->isPlaceHolder)
    {
        return false;
    }

    if (pos == 0)
    {
        if (!root->left)
        {
            root->left = new Node{0, element, nullptr, nullptr, false};
            return true;
        }
        else if (!root->right)
        {
            root->right = new Node{0, element, nullptr, nullptr, false};
            return true;
        }
        return false;
    }
    return insert_node(root->left, element, pos - 1) || insert_node(root->right, element, pos - 1);
}

/* Creates a huffman tree and returns how many bytes were parsed */
int DecoderHuffmanTree::createTree(std::vector<uint8_t> &buffer, int index)
{

    int numberOfTableEntries = 0;

    std::cout << "Lengths :";
    for (int i = 0; i < 16; i++)
    {
        int huffnodeNum = buffer[index + i];
        std::cout << huffnodeNum << " ";
        numberOfTableEntries += huffnodeNum;
    }
    std::cout << "\n Total Number of entries " << numberOfTableEntries << " Values : \n";

    for (int i = 0; i < numberOfTableEntries; i++)
    {
        std::cout << (int)buffer[index + 16 + i] << " ";
    }

    std::cout << "\n";

    char nullChar = char(0);
    int offset = 0;
    uint16_t mapping = 0;
    int prevHeight = 0;
    this->root = new Node{0, nullChar, nullptr, nullptr, true};
    for (int i = 0; i < 16; i++)
    {
        int numberOfHuffNodesAtLevel = buffer[index + i];

        for (int j = 0; j < numberOfHuffNodesAtLevel; j++)
        {
            int height = i + 1;
            char c = buffer[index + 16 + j + offset];

            if (prevHeight != 0 && prevHeight != height)
            {
                mapping = mapping << (height - prevHeight);
            }

            insert_node(this->root, c, height - 1);
            prevHeight = height;
            mapping++;
        }

        offset += numberOfHuffNodesAtLevel;
    }

    recurse(this->root, 0, 0);
    return 16 + numberOfTableEntries;
}

/* recurses (DFS) through huffman tree to generate the mapping for each character */
void DecoderHuffmanTree::recurse(Node *node, uint16_t bits, int height)
{
    if (node != NULL)
    {
        
        Node *left = node->left;
        Node *right = node->right;

        if (node->isPlaceHolder == false)
        {

            this->map[std::pair(bits, height)] = node->value;
        }

        /*
        For huffman trees, node to left are given a '0' bit and '1' on the right
        */
        recurse(left, bits << 1, height + 1);
        recurse(right, (bits << 1) | 1, height + 1);
    }
}


/* decodes the whole encoded buffer */ 
void DecoderHuffmanTree::decode(std::vector<uint8_t> &encodedBuffer, std::vector<uint8_t> &decodedBuffer)
{
    uint16_t value = 0;
    int bitsRead = 0;
    BitStreamStruct *bitStream = new BitStreamStruct(encodedBuffer);
    int numBits = 0;
    for (int i = 0; i < encodedBuffer.size() * 8; i++)
    {
        getNextBit(bitStream);
        numBits++;
        int value = bitStream->value;
        std::pair<uint16_t, int> mapIndex = std::pair(value, numBits);
        if (map.count(mapIndex) > 0)
        {
            std::cout << "Found mapping for value " << std::bitset<16>(value) << " with height " << numBits << " " << (char)map[mapIndex] << "\n";
            decodedBuffer.push_back(map[mapIndex]);
            bitStream->value = 0;
            numBits = 0;
        }
    }
}

/* decodes until it finds the first decoded value, return how many bits it read */
int DecoderHuffmanTree::decodeOne(uint16_t var, uint16_t *decodedInt)
{
    uint16_t value = 0;
    int bitsRead = 0;
    std::cout << "decoding first found thing chat \n";
    Node *searchNode = this->root;
    for (int indexInVar = 1; indexInVar <= 16; indexInVar++)
    {

        bitsRead++;
        value <<= 1;

        if (var & (1 << (16 - indexInVar)))
        {
            // we have read a 1 go to the right
            searchNode = searchNode->right;
        }
        else
        {
            // we have read a 0 go to the left
            searchNode = searchNode->left;
        }

        if (searchNode == NULL)
        {
            return bitsRead;
        }

        if (searchNode != NULL && !searchNode->isPlaceHolder)
        {
            *decodedInt = searchNode->value;
            return bitsRead;
        }
    }
}

int DecoderHuffmanTree::getLastIndex()
{
    return this->lastIndex;
}
