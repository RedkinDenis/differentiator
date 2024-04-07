#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <cstddef>
#include "..\..\err_codes.h"
#define DATA_LEN 40

#define DEFUALT_NODE create_node(DEFUALT, NULL, NULL, NULL)

#define OP_DEFINITOR            \
    operation add = ADD;        \
    operation mul = MUL;        \
    operation sub = SUB;        \
    operation div = DIV;        \
    operation pow = POW;        \
    char* exp = (char*)"exp";   \
    char* ln = (char*)"ln";     \
    char* sin = (char*)"sin";   \
    char* cos = (char*)"cos";   \
    char* tg = (char*)"tg";   

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
    EXP = 'e',
    POW = '^',
    SIN = 's',
    COS = 'c',
    TG = 't'
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

err tree_kill (Node* head);

Node* diff (const Node* node);

void simplifier (Node* tree);

Node* create_node (data_t type, void* data, Node* left, Node* right);

operation long_op_det (char* str, char** s = NULL);

#endif //DIFFERENTIATOR_H


