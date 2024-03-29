#include "differentiator.h"
#include "input_output.h"
#include "..\UDL.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

static err tree_kill (Node* head);

static double calculator (Node* tree);

static void dump (Node* tree);

int main ()
{
    FOPEN(read, "mathTree.txt", "rb");
    
    Node tree = {};
    importTree(read, &tree);
    fclose(read);
    
    // print_tree(&tree);

    draw_tree(&tree);

    //printf("\nanswer - %lf\n", calculator(&tree));

    //tree_kill(&tree);
} 

operation long_op_det (char* str)
{
    if (strcmp(str, "ln") == 0)
        return LN;
    else if (strcmp(str, "exp") == 0)
        return EXP;
    return ERR;
}

double calculator (Node* tree)
{
    if (tree == NULL)
        return 0;

    if (tree->type == OPERAND)
    {
        switch (tree->data.operand)
        {
            #define OPERATION(str, op)                                      \
            case str:                                                       \
                return calculator(tree->left) op calculator(tree->right);   \
                break;      
            #include "operations.h"
            #undef OPERATION
            default:
                printf("unknown operator\n");
                break;            
        }
    }
    else if (tree->type == LONG_OPERAND)
    {
        switch (long_op_det(tree->data.long_operand))
        {
            case LN:
                return log(calculator(tree->right));
            case EXP:
                return exp(calculator(tree->right));
        }
    }
    return tree->data.value;
}

void dump(Node *tree)
{
    printf("\n---------------NODE_DUMP-------------\n");
    printf("type - %d\n", tree->type);
    print_data(tree);
    printf("\n---------------DUMP_END--------------\n");
}

err tree_kill (Node* head)
{
    CHECK_PTR(head);

    if (head->left != NULL)
        tree_kill(head->left);

    if (head->right != NULL)
        tree_kill(head->right);

    free(head);
    return SUCCESS;
}
