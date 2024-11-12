#pragma once
#include <queue>
#include "../../structs/Node.hpp"

struct CompareCount
{

    // Comparison operator for the priority queue
    bool operator()(const Node *a, const Node *b)
    {
        int aCount = a->count;
        int bCount = b->count;
        char aChar = a->value;
        char bChar = b->value;
        return aCount > bCount;
    }
};
