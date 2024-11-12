
#pragma once
#include "../../structs/Node.hpp"
#include <algorithm>
#include <list>
#include <iostream>
#ifndef treeHelpers
#define treeHelpers


// Taken from https://stackoverflow.com/questions/801740/c-how-to-draw-a-binary-tree-to-the-console
// viusalizes a BST

inline int max_depth(Node* n)
{
  if (!n) return 0;
  return 1 + std::max(max_depth(n->left), max_depth(n->right));
}

inline void printTree(Node* n)
{

  struct node_depth
  {
    Node* n;
    int lvl;
    node_depth(Node* n_, int lvl_) : n(n_), lvl(lvl_) {}
  };

  int depth = max_depth(n);

  char buf[1024];
  int last_lvl = 0;
  int offset = (1 << depth) - 1;

  // using a queue means we perform a breadth first iteration through the tree
  std::list<node_depth> q;

  q.push_back(node_depth(n, last_lvl));
  while (q.size())
  {
    const node_depth& nd = *q.begin();

    // moving to a new level in the tree, output a new line and calculate new offset
    if (last_lvl != nd.lvl)
    {
      std::cout << "\n";

      last_lvl = nd.lvl;
      offset = (1 << (depth - nd.lvl)) - 1;
    }

    // output <offset><data><offset>
    if (nd.n){
    if(nd.n->isPlaceHolder){
      sprintf(buf, " %*s%d%*s", offset, " ",  nd.n->value, offset, " ");
    } else {
      sprintf(buf, " %*s%c%*s", offset, " ", (char) nd.n->value, offset, " ");
    }
    } else
      sprintf(buf, " %*s", offset << 1, " ");
    std::cout << buf;

    if (nd.n)
    {
      q.push_back(node_depth(nd.n->left, last_lvl + 1));
      q.push_back(node_depth(nd.n->right, last_lvl + 1));
    }

    q.pop_front();
  }
  std::cout << "\n";
}
#endif