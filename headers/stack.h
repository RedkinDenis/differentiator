#ifndef STACK_H
#define STACK_H

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "C:\Users\vp717\Desktop\ilab\err_codes.h"
#include "C:\Users\vp717\Desktop\ilab\processor\redkin_processor\DSL.h"

typedef char* elem_t;

static const elem_t poison = NULL;

struct Stack
{
    elem_t* data = NULL;
    size_t capacity = 0;
    size_t size = 0;
};

err stack_pop(struct Stack* stk, elem_t* pop_el);

err stack_dump(struct Stack* stk, int LINE, const char* stk_name, const char* file_name, const char* func_name);

err stack_ctor(struct Stack* stk, size_t capacity);

err stack_push(struct Stack* stk, const elem_t* x);

err stack_dtor(struct Stack* stk);

err capacity_down(struct Stack* stk);

err capacity_up(struct Stack* stk);

#endif //STACK_H
