#ifndef TEX_OUT_H
#define TEX_OUT_H

#include "differentiator.h"

struct line 
{
    char* str = NULL;
    size_t len = 0;
};

struct Data 
{
    line* lines = NULL;
    size_t quant = 0;
};

err diff_tex (Node* tree);

Data input_data (const char* file_name);

void dump_data (Data* data);

#endif // TEX_OUT_H