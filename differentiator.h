#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <cstddef>
#define DATA_LEN 40

enum data_t
{
    NUM = 1,
    OPERAND = 2,
    DEFUALT = 0
};

union data
{
    unsigned char operand;
    double value;
};

struct Node
{
    data_t type = DEFUALT;
    
    data data = {};
    
    Node* left = NULL;
    Node* right = NULL;
    
    Node* parent = NULL;

    int num_in_tree = 0;
};


#endif //DIFFERENTIATOR_H


