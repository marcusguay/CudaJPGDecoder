
#ifndef HNODE_H
#define HNODE_H


struct Node
{
    int count;
    char value;
    Node *left;
    Node *right;
    bool isPlaceHolder = false;
};

#endif