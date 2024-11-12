#include "./include/EncoderHuffmanTree.hpp"

  struct comparePairs
{

    /* Sorts pairs based on height then lexographically for a Canonical HT */
    bool operator()(const std::pair<char,int> a, const std::pair<char,int> b)
    {

        int aHeight = a.second;
        int bHeight = b.second;
        char aChar = a.first;
        char bChar = b.first;
      

       if(aHeight == bHeight){
        return bChar > aChar;
       }

        return bHeight > aHeight;
    }
};

/* Counts frequencies of characters in a buffer, creates a priority queue that prioritizes char frequency, then lexographically if tie
   then utilizes the greedy algo for huffman encoding to construct tree */
Node *EncoderHuffmanTree::Encode(std::vector<uint8_t> &buffer, int startIndex, int length)
{

    std::unordered_map<char, int> count;

    for (int i = startIndex; i < startIndex + length; i++)
    {
        count[buffer[i]]++;
    }

    std::priority_queue<Node *, std::vector<Node *>, CompareCount> pq;

    for (auto pair : count)
    {
        char pairChar = pair.first;
        int pairCount = pair.second;
        Node *node = new Node{pairCount, pairChar, nullptr, nullptr, false};
        pq.push(node);
        std::cout << pairChar << " " << pairCount << "\n";
    }

    Node *root;
    while (!pq.empty())
    {
        if (pq.size() >= 2)
        {
            Node *firstNode = pq.top();
            pq.pop();
            if (firstNode->isPlaceHolder == false)
            {
                char value = firstNode->value;
            }

            Node *secondNode = pq.top();
            pq.pop();
            if (secondNode->isPlaceHolder == false)
            {
                char value = secondNode->value;
            }

            char nullChar = char(0);
            int parentCount = firstNode->count + secondNode->count;
            Node *parent = new Node{parentCount, nullChar, firstNode, secondNode, true};
            pq.push(parent);
        }

        /* put last node in pq as the root of the tree */
        if (pq.size() == 1)
        {

            Node *top = pq.top();

            if (top->isPlaceHolder == false)
            {
                root = new Node{top->count, char(0), top, NULL, true};
            }
            else
            {
                root = top;
            }

            pq.pop();
        }
    }
    
    // printTree(root); debug fn

    recurse(root, 0, 0);
    std::sort(this->valueAndHeights.begin(), this->valueAndHeights.end(), comparePairs());
    generateCodes();
    return root;
}

/* Generates canonical huffman codes */ 
void EncoderHuffmanTree::generateCodes(){
    uint16_t mapping = 0;
    int prevHeight = 0;
    for(auto a : this->valueAndHeights){
        char c = a.first;
        int height = a.second;

        if(prevHeight != 0 && prevHeight != height) {
            mapping = mapping << (height - prevHeight);
        }
        this->map[c] = std::pair<uint16_t, int>(mapping, height);
        std::cout << "for " << c << " mapping is " << std::bitset<16>(mapping) << " height " << height << "\n";
        prevHeight = height;
        mapping++;
    }
}


/* Debug function */
std::vector<uint8_t> EncoderHuffmanTree::getLengthsAndValues()
{
    std::vector<uint8_t> buffer;

    std::cout << "lengths : ";
    for (auto var : this->valueAndHeights)
    {    
        int height = var.second;
        this->lengthArray[height - 1]++;
    }

    for(auto var: this->lengthArray){
        buffer.push_back(var);
    }

    std::cout << "\n values : ";

    for (auto var : this->valueAndHeights)
    {
        char c = var.first;
        buffer.push_back(c);
    }

    std::cout << "\n";

    return buffer;
}

/* Applies the mapping from the huffman tree into a buffer */
void EncoderHuffmanTree::EncodeBuffer(std::vector<uint8_t> &buffer, std::vector<uint16_t> &newBuffer, Node *root)
{

    /* since we can only write min 8 bits into the buffer, we must resuse chars for multiple bits */
    uint16_t newChar = char(0);
    int currIndex = 0;
    int rem = 0;
    for (uint8_t c : buffer)
    {
        int length = this->map[c].second;
        uint16_t value = this->map[c].first;
        newChar = newChar | ((value << (16 - length)) >> currIndex);

        rem = length - (16 - currIndex);
        currIndex += length;

        if (currIndex > 15)
        {
            newBuffer.push_back(newChar);
            newChar = char(0);
            if (rem > 0)
            {
                uint16_t newVal = value << (16 - rem);
                newChar = newChar | newVal;
                currIndex = rem;
            }
            else
            {
                currIndex = 0;
            }
        }
    }

    if (currIndex > 0)
    {
        newBuffer.push_back(newChar);
    }
}


/* recurses (DFS) through huffman tree to generate the mapping for each character */
void EncoderHuffmanTree::recurse(Node *node, uint16_t bits, int height)
{
    if (node != NULL)
    {

        Node *left = node->left;
        Node *right = node->right;

        if (node->isPlaceHolder == false)
        {
            this->valueAndHeights.push_back(std::pair<char,int>(node->value, height));
        }

        /* for huffman trees, node to left are given a '0' bit and '1' on the right */
        recurse(left, bits << 1, height + 1);

        if (node->isPlaceHolder == false)
        {
        // this->valueArray.push_back(node->value);
        }
        recurse(right, (bits << 1) | 1, height + 1);
    }
}
