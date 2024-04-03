#include "headers/differentiator.h"
#include "headers/input_output.h"
#include "..\UDL.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#define OP_DEFINITOR            \
    operation add = ADD;        \
    operation mul = MUL;        \
    operation sub = SUB;        \
    operation div = DIV;        \
    operation pow = POW;        \
    char* exp = (char*)"exp";   \
    char* ln = (char*)"ln";     

#define FREE_SUBTREE(subtree)   \
    do                          \
    {                           \
    free(subtree);              \
    subtree = NULL;             \
    } while (0)


static double calculator (Node* tree, int* var = NULL);

static void dump (Node* tree);

static Node* copy_subtree (Node* sub_tree);

static Node* create_node (data_t type, void* data, Node* left, Node* right);

static void calc_simplifier (Node* tree, int* changed);

static void second_simplifier (Node* tree, int* changed);

static Node* diff_mul (const Node* node);

static Node* diff_div (const Node* node);

static Node* diff_exp (const Node* node);

static Node* diff_pow (const Node* node);

static Node* diff_ln (const Node* node);

int main ()
{
    FOPEN(read, "mathTree.txt", "rb");
    
    Node tree = {};
    importTree(read, &tree);
    fclose(read);
    
    simplifier(&tree);
    draw_tree(&tree);
    Sleep(1000);

    Node* Diff = diff(&tree);
    draw_tree(Diff);
    Sleep(1000);

    simplifier(Diff);
    draw_tree(Diff);

    tree_kill(&tree);
} 

operation long_op_det (char* str)
{
    if (strcmp(str, "ln") == 0)
        return LN;
    else if (strcmp(str, "exp") == 0)
        return EXP;
    return ERR;
}

double calculator (Node* tree, int* var)
{
    if (tree == NULL)
        return 0;

    if (tree->type == VAR)
        *var = 1;    

    if (var != NULL && *var == 1)
        return 0;

    if (tree->type == OPERAND)
    {
        switch (tree->data.operand)
        {
            #define OPERATION(str, op)                                                \
            case str:                                                                 \
                return calculator(tree->left, var) op calculator(tree->right, var);   \
                break;      
            #include "headers//differentiator.h"
            #undef OPERATION
            case '^':
                return pow(calculator(tree->left, var), calculator(tree->right, var));
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
                return log(calculator(tree->right, var));
            case EXP:
                return exp(calculator(tree->right, var));
        }
    }
    return tree->data.value;
}

void simplifier (Node* tree)
{
    int changed = 1;
    while (changed > 0)
    {
        changed = 0;
        calc_simplifier(tree, &changed);
        second_simplifier(tree, &changed);
    }     
}

void calc_simplifier (Node* tree, int* changed)
{
    int var = 0;
    double temp = calculator(tree, &var);
    if (var == 0 && tree->type != NUM && tree->type != DEFUALT)
    {
        tree->data.value = temp;
        tree->type = NUM;

        FREE_SUBTREE(tree->left);
        FREE_SUBTREE(tree->right);

        *changed += 1;
    }
    
    if (tree->left != NULL)
        calc_simplifier(tree->left, changed);

    if (tree->right != NULL)
        calc_simplifier(tree->right, changed);
}

void second_simplifier (Node* tree, int* changed)
{    
    if (tree->type == OPERAND && tree->data.operand == '*')
    {
        if ((tree->left->type == NUM && tree->left->data.value == 0) || (tree->right->type == NUM && tree->right->data.value == 0))
        {
            tree->type = NUM;
            tree->data.value = 0;
            
            FREE_SUBTREE(tree->left);
            FREE_SUBTREE(tree->right);

            *changed += 1;
        }
        else if (tree->left->type == NUM && tree->left->data.value == 1)
        {
            FREE_SUBTREE(tree->left);
            memcpy(tree, tree->right, sizeof(Node));
            *changed += 1;
        }

        else if (tree->right->type == NUM && tree->right->data.value == 1)
        {
            FREE_SUBTREE(tree->right);
            memcpy(tree, tree->left, sizeof(Node));
            *changed += 1;
        }
    }
    else if (tree->type == OPERAND && tree->data.operand == '+')
    {
        if (tree->left->type == NUM && tree->left->data.value == 0)
        {
            FREE_SUBTREE(tree->left);
            memcpy(tree, tree->right, sizeof(Node));
            *changed += 1;
        }

        else if (tree->right->type == NUM && tree->right->data.value == 0)
        {
            FREE_SUBTREE(tree->right);
            memcpy(tree, tree->left, sizeof(Node));
            *changed += 1;
        }
    }
    
    if (tree->left != NULL)
        second_simplifier(tree->left, changed);

    if (tree->right != NULL)
        second_simplifier(tree->right, changed);
}

Node* copy_subtree (Node* sub_tree)
{
    Node* copy = (Node*)calloc(1, sizeof(Node));
    memcpy(copy, sub_tree, sizeof(Node));

    if (sub_tree->left != NULL)
        copy->left = copy_subtree(sub_tree->left);
    
    if (sub_tree->right != NULL)
        copy->right = copy_subtree(sub_tree->right);

    return copy;
}

Node* create_node (data_t type, void* data, Node* left, Node* right)
{
    Node* newNode = (Node*)calloc(1, sizeof(Node));

    newNode->type = type;

    switch (type)
    {
        case NUM:
            newNode->data.value = *(double*)data;
            break;
        case OPERAND:
            newNode->data.operand = *(char*)data;
            break;
        case LONG_OPERAND:
            newNode->data.long_operand = (char*)calloc(strlen((char*)data), sizeof(char));
            strcpy(newNode->data.long_operand, (char*)data);
            break;
    }
 
    newNode->left = left;
    newNode->right = right;

    return newNode;
}

Node* diff (const Node* node)
{
    OP_DEFINITOR
    switch (node->type)
    {
        case NUM:
        {
            double temp = 0;
            return create_node(NUM, &temp, NULL, NULL);
        }
        case VAR:
        {
            double temp = 1;
            return create_node(NUM, &temp, NULL, NULL);
        }
        case OPERAND:
            switch (node->data.operand)
            {
                case ADD:
                    return create_node(OPERAND, &add, diff(node->left), diff(node->right));
                
                case SUB:
                    return create_node(OPERAND, &sub, diff(node->left), diff(node->right));
                
                case MUL:
                    return diff_mul(node);
                    
                case DIV:
                    return diff_div(node);

                case POW:
                    return diff_pow(node);
            }
        case (LONG_OPERAND):
            switch (long_op_det(node->data.long_operand))
            {
                case LN:
                    return diff_ln(node);
    
                case EXP:
                    return diff_exp(node);     
            }
    }
    return NULL;
}

Node* diff_mul (const Node* node)
{
    OP_DEFINITOR
    Node* du = diff(node->left);    Node* cu = copy_subtree(node->left);
    Node* dv = diff(node->right);   Node* cv = copy_subtree(node->right);
    
    Node* res = create_node(OPERAND, &add, create_node(OPERAND, &mul, du, cv), create_node(OPERAND, &mul, cu, dv));

    return res;
}

Node* diff_div (const Node* node)
{
    OP_DEFINITOR
    Node* du = diff(node->left);    Node* cu = copy_subtree(node->left);
    Node* dv = diff(node->right);   Node* cv = copy_subtree(node->right);

    Node* nominator = create_node(OPERAND, &sub, create_node(OPERAND, &mul, du, cv), create_node(OPERAND, &mul, cu, dv));
    Node* denominator = create_node(OPERAND, &mul, copy_subtree(cv), copy_subtree(cv));

    Node* res = create_node(OPERAND, &div, nominator, denominator);
    return res;
}

Node* diff_ln (const Node* node)
{
    OP_DEFINITOR
    double temp = 1;
                    
    Node* dln = create_node(OPERAND, &div, create_node(NUM, &temp, NULL, NULL), copy_subtree(node->right));
    Node* dx = diff(node->right);

    Node* res = create_node(OPERAND, &mul, dln, dx);
    return res;
}

Node* diff_pow (const Node* node)
{
    OP_DEFINITOR
    if (node->left->type == NUM)
    {
        Node* cx = copy_subtree((Node*)node);
        Node* dx = diff(node->right);
        Node* lna = create_node(LONG_OPERAND, ln, NULL, copy_subtree(node->left));
        Node* res = create_node(OPERAND, &mul, lna, create_node(OPERAND, &mul, cx, dx));

        return res;
    }
    else if (node->right->type == NUM)
    {
        double temp = 1;
        Node* degree_down = create_node(NUM, &temp, NULL, NULL);
        Node* degree = create_node(OPERAND, &sub, copy_subtree(node->right), degree_down); 

        Node* cx = copy_subtree(node->left);
        Node* dx = diff(node->left);

        Node* res = create_node(OPERAND, &mul, copy_subtree(node->right), create_node(OPERAND, &mul, dx, create_node(OPERAND, &pow, cx, degree)));

        return res;
    }
    else 
    {
        Node* middle = create_node(LONG_OPERAND, exp, NULL, create_node(OPERAND, &mul, copy_subtree(node->right), create_node(LONG_OPERAND, ln, NULL, copy_subtree(node->left))));

        return diff(middle);
    }
}

Node* diff_exp (const Node* node)
{
    OP_DEFINITOR
    Node* cx = copy_subtree((Node*)node);
    Node* res = create_node(OPERAND, &mul, cx, diff(node->right));
    return res;   
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
        return tree_kill(head->left);

    if (head->right != NULL)
        return tree_kill(head->right);

    free(head);
    return SUCCESS;
}
