#include "differentiator.h"
#include "input_output.h"
#include "..\UDL.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <windows.h>

static err tree_kill (Node* head);

static double calculator (Node* tree, int* var = NULL);

static void dump (Node* tree);

static Node* copy_subtree (Node* sub_tree);

static Node* create_node (data_t type, void* data, Node* left, Node* right);

static Node* diff (const Node* node);

static void calc_simplifier (Node* tree, int* changed);

static void second_simplifier (Node* tree, int* changed);

static void simplifier (Node* tree);

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

    //printf("answer - %lf\n", calculator(&tree));

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
            #include "operations.h"
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
        free(tree->left);
        free(tree->right);
        tree->left = NULL;
        tree->right = NULL;

        *changed += 1;
        printf("1 ");
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
            tree->left = NULL;
            tree->right = NULL;

            *changed += 1;
            printf("2 ");
        }
        else if (tree->left->type == NUM && tree->left->data.value == 1)
        {
            memcpy(tree, tree->right, sizeof(Node));
            *changed += 1;
            printf("3 ");
        }

        else if (tree->right->type == NUM && tree->right->data.value == 1)
        {
            memcpy(tree, tree->left, sizeof(Node));
            *changed += 1;
            printf("4 ");
        }
    }
    else if (tree->type == OPERAND && tree->data.operand == '+')
    {
        if (tree->left->type == NUM && tree->left->data.value == 0)
        {
            memcpy(tree, tree->right, sizeof(Node));
            *changed += 1;
            printf("5 ");
        }

        else if (tree->right->type == NUM && tree->right->data.value == 0)
        {
            memcpy(tree, tree->left, sizeof(Node));
            *changed += 1;
            printf("6 ");
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
                case '+':
                {
                    operation add = ADD;
                    return create_node(OPERAND, &add, diff(node->left), diff(node->right));
                }
                case '-':
                {
                    operation sub = SUB;
                    return create_node(OPERAND, &sub, diff(node->left), diff(node->right));
                }
                case '*':
                {   
                    Node* du = diff(node->left);    Node* cu = copy_subtree(node->left);
                    Node* dv = diff(node->right);   Node* cv = copy_subtree(node->right);

                    operation add = ADD;
                    operation mul = MUL;
                    
                    Node* res = create_node(OPERAND, &add, create_node(OPERAND, &mul, du, cv), create_node(OPERAND, &mul, cu, dv));

                    return res;
                }
                case '/':
                {   
                    Node* du = diff(node->left);    Node* cu = copy_subtree(node->left);
                    Node* dv = diff(node->right);   Node* cv = copy_subtree(node->right);

                    operation sub = SUB;
                    operation div = DIV;
                    operation mul = MUL;
                    
                    Node* nominator = create_node(OPERAND, &sub, create_node(OPERAND, &mul, du, cv), create_node(OPERAND, &mul, cu, dv));
                    Node* denominator = create_node(OPERAND, &mul, copy_subtree(cv), copy_subtree(cv));

                    Node* res = create_node(OPERAND, &div, nominator, denominator);
                    return res;
                }   
                case '^':
                {
                    if (node->left->type == NUM)
                    {
                        char* ln = (char*)"ln";
                        operation mul = MUL;

                        Node* cx = copy_subtree((Node*)node);
                        Node* dx = diff(node->right);
                        Node* lna = create_node(LONG_OPERAND, ln, NULL, copy_subtree(node->left));
                        Node* res = create_node(OPERAND, &mul, lna, create_node(OPERAND, &mul, cx, dx));

                        return res;
                    }
                    else if (node->right->type == NUM)
                    {
                        operation mul = MUL;
                        operation sub = SUB;
                        operation pow = POW;

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
                        char* exp = (char*)"exp";
                        char* ln = (char*)"ln";
                        operation mul = MUL;

                        Node* middle = create_node(LONG_OPERAND, exp, NULL, create_node(OPERAND, &mul, copy_subtree(node->right), create_node(LONG_OPERAND, ln, NULL, copy_subtree(node->left))));

                        return diff(middle);
                    }
                }
            }
        case (LONG_OPERAND):
        {
            switch (long_op_det(node->data.long_operand))
            {
                case LN:
                {
                    operation mul = MUL;
                    operation div = DIV;
                    double temp = 1;
                    
                    Node* dln = create_node(OPERAND, &div, create_node(NUM, &temp, NULL, NULL), copy_subtree(node->right));
                    Node* dx = diff(node->right);

                    Node* res = create_node(OPERAND, &mul, dln, dx);
                    return res;
                }
                case EXP:
                {
                    operation mul = MUL;

                    Node* cx = copy_subtree((Node*)node);

                    Node* res = create_node(OPERAND, &mul, cx, diff(node->right));
                    return res;
                }     
            }
        }
    }
    return NULL;
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
