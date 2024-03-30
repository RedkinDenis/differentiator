#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <cstddef>
#define DATA_LEN 40

enum data_t
{
    NUM = 1,
    VAR = 4,
    OPERAND = 2,
    LONG_OPERAND = 3,
    DEFUALT = 0
};

enum operation
{
    ERR = 0, 
    ADD = '+', 
    SUB = '-', 
    MUL = '*', 
    DIV = '/', 
    LN = 'l', 
    EXP = 'e'
};

union data
{
    unsigned char operand;
    char* var;
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


