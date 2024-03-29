#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <ctype.h>

#include "input_output.h"
#include "../UDL.h"

#define GET_VARIABLE_NAME(variable) #variable

#define CHANGE_NODE(from, to)        \
    do                               \
    {                                \
    Node* tree_temp_ = 0;            \
    CALLOC(to, Node, 1);             \
    tree_temp_ = from;               \
    from = to;                       \
    from->parent = tree_temp_;       \
    level++;                         \
    } while(0)


static err fill_buffer (FILE* read, char** buf);

static void get_data (char* buf, int* ptr, Node* tree, int data_len);

static void goto_prace (char* buf, int* ptr);

static err print_tree__ (Node* head, int* tab);

static err fprint_tree__ (FILE* out, Node* head, int* tab);

// static void print_data (Node* head);

static int check_param (const char* data_buffer);

static void draw_tree_1 (FILE* save, Node* tree, int* node_num);

static void draw_tree_2 (FILE* save, Node* tree);

int GetFileSize(FILE* fp)
{
    int startPos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, startPos, SEEK_SET);
    return fsize;
}

err print_tree (Node* head)
{
    CHECK_PTR(head);

    int tab = 0;
    return print_tree__(head, &tab);
}

err print_tree__ (Node* head, int* tab)
{
    CHECK_PTR(head);

    printf("(");

    printf("%*s", *tab * 4, "");
    print_data(head);

    if (head->left != NULL)
    {
        *tab += 1;
        printf("\n");
        print_tree__(head->left, tab);
    }

    if (head->right != NULL)
    {
        printf("\n");
        print_tree__(head->right, tab);
        *tab -= 1;
    }

    printf(")");

    return SUCCESS;
}

void print_data (Node* head)
{
    if (head->type == NUM)
        printf("#%lf#", head->data.value);
        
    else if (head->type == OPERAND)
        printf("#%c#", head->data.operand); 
    else if (head->type == LONG_OPERAND)
        printf("#%s#", head->data.long_operand); 
    else if (head->type == PAR)
        printf("#%s#", head->data.param); 
    else if (head->type ==  DEFUALT)
        printf("#null#");
    
    return;
}

err fprint_tree (FILE* out, Node* head)
{
    CHECK_PTR(out);
    CHECK_PTR(head);

    int tab = 0;
    return fprint_tree__(out, head, &tab);
}

err fprint_tree__ (FILE* out, Node* head, int* tab)
{
    CHECK_PTR(head);

    fprintf(out, "(");

    fprintf(out, "%*s", *tab * 4, "");

    fprintf(out, "*%s*", head->data);

    if (head->left != NULL)
    {
        *tab += 1;
        fprintf(out, "\n");
        fprint_tree__(out, head->left, tab);
    }

    if (head->right != NULL)
    {
        fprintf(out, "\n");
        fprint_tree__(out, head->right, tab);
        *tab -= 1;
    }

    fprintf(out, ")");

    return SUCCESS;
}

err fill_buffer (FILE* read, char** buf)
{
    CHECK_PTR(read);
    CHECK_PTR(buf);

    int fsize = GetFileSize(read);

    CALLOC(*buf, char, (fsize + 1));

    fread(*buf, sizeof(char), fsize, read);

    return SUCCESS;
}

err importTree (FILE* read, Node* tree)
{
    CHECK_PTR(read);
    CHECK_PTR(tree);

    char* buf = 0;
    fill_buffer(read, &buf);

    int level = 0, ptr = 0;

    goto_prace(buf, &ptr);

    if (buf[ptr] == '(')
    {
        ptr++;
        level++;
        get_data(buf, &ptr, tree, DATA_LEN);
    }

    while (level > 0)
    {
        if (buf[ptr] == '(')
        {
            ptr++;
            CHANGE_NODE(tree, tree->left);

            get_data(buf, &ptr, tree, DATA_LEN);
        }
        else if (buf[ptr] == ')')
        {
            tree = tree->parent;
            level--;

            if (level == 0)
                break;

            ptr++;

            goto_prace(buf, &ptr);

            if (buf[ptr] == '(')
            {
                ptr++;

                CHANGE_NODE(tree, tree->right);

                get_data(buf, &ptr, tree, DATA_LEN);
            }
        }
    }
    free(buf);

    return SUCCESS;
}

void goto_prace (char* buf, int* ptr)
{
    while((buf[*ptr] != '(') && (buf[*ptr] != ')'))
        *ptr += 1;
}

void get_data(char* buf, int* ptr, Node* tree, int data_len)
{
    char* data_buffer = (char*)calloc(DATA_LEN + 1, sizeof(char));
    while (buf[*ptr - 1] != '#')
        *ptr += 1;

    int i = 0;
    while ((buf[*ptr] != '#') && i <= data_len)
    {
        data_buffer[i] = buf[*ptr];
        *ptr += 1;
        i++;
    }
    
    if (check_param(data_buffer))
    {
        tree->data.param = strdup(data_buffer);
        tree->type = PAR;

        free(data_buffer);
        goto_prace(buf, ptr);
        return;
    }
    else 
    {
        int x = sscanf(data_buffer, "%lf", &(tree->data.value));
        if (x == 1) 
        {
            tree->type = NUM;
            free(data_buffer);
            goto_prace(buf, ptr);
            return;
        }
    }
    if (strlen(data_buffer) == 1)
    {
        tree->data.operand = data_buffer[0];
        tree->type = OPERAND;
    }
    else
    {
        if (strcmp(data_buffer, "null") == 0)
            tree->type = DEFUALT;
        else
            tree->type = LONG_OPERAND;

        tree->data.long_operand = (char*)calloc(strlen(data_buffer) + 1, sizeof(char));
        strcpy(tree->data.long_operand, data_buffer);    
    }

    free(data_buffer);
    goto_prace(buf, ptr);
}

int check_param (const char* data_buffer)
{
    int len = (int)strlen(data_buffer);
    // printf("len - %d\n", len);
    if (isalpha(data_buffer[len - 1]))
    {
        if (len == 1)
            return 1;
        for (int i = len - 1; i >= 0; i--)
        {   
            if (('0' <= data_buffer[i]) && (data_buffer[i] <= '9'))
            {
                // printf("%s\n", data_buffer);
                return 1;
            }
        }
    }
    return 0;
}

err draw_tree (Node* tree)
{
    FOPEN(save, "drawTree.txt", "wb");

    fprintf(save, "digraph Tree {\n");

    int node_num = 0;
    draw_tree_1(save, tree, &node_num);
    draw_tree_2(save, tree);

    fprintf(save, "}");

    fclose(save);

    system("iconv -f WINDOWS-1251 -t UTF-8 drawTree.txt > buffer.txt");
    system("dot buffer.txt -Tpng -o drawTree.png");
    system("start drawTree.png");
    return SUCCESS;
}

char* data_to_string (Node* tree)
{
    char* data = 0;
    if (tree->type == NUM)
    {
        data = (char*)calloc(DATA_LEN + 1, sizeof(char));
        sprintf(data, "%f", tree->data);
    }
    else if (tree->type == OPERAND)
    {
        data = (char*)calloc(2, sizeof(char));
        sprintf(data, "%c", tree->data);
    }
    else if (tree->type == LONG_OPERAND || tree->type == DEFUALT)
    {
        data = (char*)calloc(strlen(tree->data.long_operand) + 1, sizeof(char));
        strcpy(data, tree->data.long_operand);
    }    
    else if (tree->type == PAR)
    {
        data = (char*)calloc(strlen(tree->data.param) + 1, sizeof(char));
        strcpy(data, tree->data.param);
    }
    return data;
}

char* type_to_str (Node* tree)
{
    char* type = (char*)calloc(20, sizeof(char));
    
    switch (tree->type)
    {
        case NUM:
            strcpy(type, "NUM");
            break;
        case OPERAND:
            strcpy(type, "OPERAND");
            break;
        case PAR:
            strcpy(type, "PAR");
            break;
        case LONG_OPERAND:
            strcpy(type, "LONG_OPERAND");
            break;
    }
    return type;
}

void draw_tree_1 (FILE* save, Node* tree, int* node_num)
{
    char* data = data_to_string(tree);
    char* buffer = (char*)calloc(strlen(data) + 1, sizeof(char));
    strcat(buffer, data);                                                

    tree->num_in_tree = *node_num;

    char* type = type_to_str(tree);

    if (tree->type == NUM || tree->type == PAR)
        fprintf(save, "    %d [shape = Mrecord, style = filled, fillcolor = lightgoldenrod1, label = %cTYPE: %s | DATA: %s%c];\n", *node_num, '"', type, buffer, '"');
    else if (tree->type != DEFUALT)
        fprintf(save, "    %d [shape = Mrecord, style = filled, fillcolor = cyan, label = %cTYPE: %s | DATA: %s%c];\n", *node_num, '"', type, buffer, '"');
    else 
        *node_num -= 1;

    if (tree->left != NULL)
    {
        *node_num += 1;
        draw_tree_1(save, tree->left, node_num);
    }

    if (tree->right != NULL)
    {
        *node_num += 1;
        draw_tree_1(save, tree->right, node_num);
    }

    return;
}

void draw_tree_2 (FILE* save, Node* tree)
{
    if (tree->left != NULL)
    {
        fprintf(save, "    %d -> %d;\n", tree->num_in_tree, (tree->left)->num_in_tree);
        draw_tree_2(save, tree->left);
    }

    if (tree->right != NULL)
    {
        fprintf(save, "    %d -> %d;\n", tree->num_in_tree, (tree->right)->num_in_tree);
        draw_tree_2(save, tree->right);
    }
    return;
}