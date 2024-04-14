#include "headers/stack.h"

// err stack_dump(struct Stack* stk, int LINE, const char* stk_name, const char* file_name, const char* func_name)
// {
//     CHECK_PTR(stk);
//     CHECK_PTR(stk_name);
//     CHECK_PTR(file_name);
//     CHECK_PTR(func_name);

//     printf("--------STACK---------");
//     printf("\n------DUMP_BEGIN------\n");
//     printf("\nstack name: %s\n", stk_name);
//     printf("\nDUMP was called from function: %s \n"
//                              "from file: %s \n"
//                              "from line: %d \n", func_name, file_name, LINE);
//     printf("\nstack adress: %d\n", stk);
//     printf("stack->data adress: %d\n", stk->data);
//     printf("capacity - %d\nsize - %d\nstack:\n", stk->capacity, stk->size);
//     for(int i = 0; i < stk->size; i++)
//         printf("*[%d] = %f  ", i, stk->data[i]);
//     for(int i = stk->size; i < stk->capacity; i++)
//         printf("*[%d] = poison  ", i);

//     printf("\n-------DUMP_END-------\n\n");

//     return SUCCESS;
// }

err stack_ctor(struct Stack* stk, size_t capacity)
{
    CHECK_PTR(stk);

    if(stk->capacity != 0 && stk->capacity != (size_t)-1)
        return STACK_ALREDY_CREATED;

    void* temp = 0;
    CALLOC(stk->data, elem_t, capacity);

    stk->capacity = capacity;
    stk->size = 0;

    return SUCCESS;
}

err stack_push(struct Stack* stk, const elem_t* x)
{
    CHECK_PTR(stk);
    CHECK_PTR(x);

    err res = capacity_up(stk);
    if(res != SUCCESS)
        return res;

    stk->data[stk->size] = *x;
    stk->size++;
    return SUCCESS;
}

err stack_pop(struct Stack* stk, elem_t* pop_el)
{
    CHECK_PTR(stk);

    if(stk->size <= 0)
        return STK_EMPTY;

    stk->size--;

    if(pop_el != NULL)
        *pop_el = *(stk->data + stk->size);

    *(stk->data + stk->size) = poison;

    err res = capacity_down(stk);
    if(res != SUCCESS)
        return res;

    return SUCCESS;
}

err stack_dtor(struct Stack* stk)
{
    CHECK_PTR(stk);

    for (int i = 0; i < stk->size; i++)
        free(stk->data[i]);
        
    free(stk->data);
    stk->data = NULL;
    stk->size = (size_t)-1;
    stk->capacity = (size_t)-1;

    return SUCCESS;
}
err capacity_down(struct Stack* stk)
{
    CHECK_PTR(stk);

    if(stk->capacity < 2 ||
       stk->capacity / 4 < stk->size)
        return SUCCESS;

    REALLOC(stk->data, elem_t, stk->capacity / 2);

    stk->capacity /= 2;

    return SUCCESS;
}

err capacity_up(struct Stack* stk)
{
    CHECK_PTR(stk);

    if(stk->size != stk->capacity)
        return SUCCESS;

    int expander = 0;
    if(stk->capacity >= 100)
        expander = 50;
    else
        expander = stk->capacity;

    REALLOC(stk_data, elem_t, (stk->capacity + expander));

    stk->capacity += expander;
    return SUCCESS;
}
