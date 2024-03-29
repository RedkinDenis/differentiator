#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <cstddef>
#define DATA_LEN 40

enum data_t
{
    NUM = 1,
    PAR = 4,
    OPERAND = 2,
    LONG_OPERAND = 3,
    DEFUALT = 0
};

enum operation
{
    ERR, ADD, SUB, MUL, DIV, LN, EXP
};

union data
{
    unsigned char operand;
    char* param;
    char* long_operand;
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


