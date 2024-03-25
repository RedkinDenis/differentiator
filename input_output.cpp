#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "input_output.h"
#include "../UDL.h"

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
    
    //printf("buf - %s\n", buf);
    int i = 1;
    while (buf[*ptr - 1] != '#')
        *ptr += 1;

    data_buffer[0] = '#';
    while ((buf[*ptr] != '#') && i <= data_len)
    {
        data_buffer[i] = buf[*ptr];
        *ptr += 1;
        i++;
    }
    strcat(data_buffer, "#");

    //printf("data_buffer - %s\n", data_buffer);
    // for (int i = 0; i < data_len; i++)
    //     printf("%c", data_buffer[i]);
    // printf("\n");
    
    //double temp = 0;
    int x = sscanf(data_buffer, "#%lf#", &(tree->data.value));
    if (x == 1) 
    {
        //printf("here\n");
        tree->type = NUM;
        goto_prace(buf, ptr);
        return;
    }

    // while (buf[*ptr] == ' ')
    //     *ptr += 1;
    
    tree->data.operand = data_buffer[1];
    tree->type = OPERAND;

    free(data_buffer);

    goto_prace(buf, ptr);
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
    return data;
}

void draw_tree_1 (FILE* save, Node* tree, int* node_num)
{
    char* data = data_to_string(tree);

    char* buffer = (char*)calloc(strlen(data) + 3, sizeof(char));
    buffer[0] = '"';
    strcat(buffer, data);                                                
    buffer[strlen(data) + 1] = '"';

    tree->num_in_tree = *node_num;

    if (tree->right == NULL && tree->left == NULL)
        fprintf(save, "    %d [shape = Mrecord, style = filled, fillcolor = cyan, label = %s ];\n", *node_num, buffer);
    else
        fprintf(save, "    %d [shape = Mrecord, style = filled, fillcolor = lightgoldenrod1, label = %s ];\n", *node_num, buffer);


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