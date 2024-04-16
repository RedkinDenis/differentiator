#include "headers/tex_out.h"
#include "headers/input_output.h"
#include "headers/DSL.h"
#include "..\UDL.h"
#include "..\err_codes.h"
#include "headers/stack.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

struct vars 
{
    char** data = NULL;
    size_t capacity = 0;
    size_t qant = 0;
};

static err copy_file (const char* to, const char* from, const char* mode);

static err derivative_tex (FILE* out, Node* tree, const char* part);

static void equation_tex (FILE* out, Node* tree, Data* data);

static char* equation_tex__ (Node* tree, int* br, Stack* mem_stk);

static char* equation_tex_ (Node* tree, Stack* mem_stk);

static void dump_vars (vars* vars);

static void add_var (vars* vars, char* var);

static void create_vars (vars* vars, int capacity);

static void find_vars (Node* tree ,vars* vars);

static void full_der (FILE* out, Node* tree, vars* vars, Data* data);

static void full_diff (FILE* out, Node* tree, char* part, Data* data);

static void tex_random_phrase (FILE* tex, Data* data);

static void clear_data (Data* data);

static void tex_teylor (FILE* out, Node* tree, vars* vars, Data* data, int accuracy);

char* equation_tex_ (Node* tree, Stack* mem_stk)
{
    int br = 0;
    return equation_tex__(tree, &br, mem_stk);
}

char* equation_tex__ (Node* tree, int* br, Stack* mem_stk)
{
    int buf_size = 10000;
    char* buf = (char*)calloc(buf_size, sizeof(char));
    stack_push(mem_stk, &buf);

    if (tree->data.operand == MUL || tree->data.operand == DIV)
        *br = 1;

    if (tree->type == OPERAND)
    {
        if (tree->data.operand == DIV)
        {
            SPRINTF(buf, buf_size, "\\cfrac {%s} {%s}", equation_tex__(tree->left, br, mem_stk), equation_tex__(tree->right, br, mem_stk));
        }
        else  if (*br == 1 && (tree->data.operand == ADD || tree->data.operand == SUB))
        {
            SPRINTF(buf, buf_size, "(%s %c %s) ", equation_tex__(tree->left, br, mem_stk), tree->data.operand, equation_tex__(tree->right, br, mem_stk));
            *br = 0;
        }
        else if (tree->data.operand == '^')
        {
            SPRINTF(buf, buf_size, "%s %c {%s} ", equation_tex__(tree->left, br, mem_stk), tree->data.operand, equation_tex__(tree->right, br, mem_stk));
        }
        else 
        {
            SPRINTF(buf, buf_size, "%s %c %s ", equation_tex__(tree->left, br, mem_stk), tree->data.operand, equation_tex__(tree->right, br, mem_stk));
        }

        return buf;
    }
    else if (tree->type == FUNCTION)
    {
        *br = 0;
        SPRINTF(buf, buf_size, "%s(%s) ", tree->data.function, equation_tex__(tree->right, br, mem_stk));

        return buf;
    }
    
    else if (tree->type == NUM)
    {
        if ((tree->data.value - (int)tree->data.value) == 0)
        {
            SPRINTF(buf, buf_size, "%d", (int)tree->data.value);    
        }
        else 
        {
            SPRINTF(buf, buf_size, "%lf", tree->data.value);
        }
        return buf;
    }

    else if (tree->type == VAR)
    {
        SPRINTF(buf, buf_size, "%s", tree->data.var);
        return buf;
    }

    return (char*)"UNKNOWN";
}

err copy_file (const char* to, const char* from, const char* mode)
{
    FOPEN(out, to, mode);
    FOPEN(in, from, "rb");

    int fsize = GetFileSize(in);
    char* buffer = NULL;
    fill_buffer(in, &buffer);

    fwrite(buffer, sizeof(char), fsize, out);

    fclose(in);
    fclose(out);

    return SUCCESS;
}

err derivative_tex (FILE* out, Node* tree, const char* part)
{
    Node* Diff = diff(tree, part);
    simplifier(Diff);

    Stack mem_stk = {};
    stack_ctor(&mem_stk, 1);

    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n \\cfrac {d} {d %s}(%s) = %s", part, equation_tex_(tree, &mem_stk), equation_tex_(Diff, &mem_stk));
    fprintf(out, "\n%s\n\n", "\\end{dmath}");

    stack_dtor(&mem_stk);

    return SUCCESS;
}

void equation_tex (FILE* out, Node* tree, Data* data)
{
    Stack mem_stk = {};
    stack_ctor(&mem_stk, 1);

    tex_random_phrase(out, data);
    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n%s", equation_tex_(tree, &mem_stk));
    fprintf(out, "\n%s\n\n", "\\end{dmath}");

    stack_dtor(&mem_stk);
}

err diff_tex (Node* tree)
{
    copy_file("tex/equation.tex", "tex/preambule_article.tex", "wb");
    copy_file("tex/equation.tex", "tex/title.tex", "ab");

    Data data = input_data("tex/phrases.txt");
    FOPEN(out, "tex/equation.tex", "ab");
    fprintf(out, "Вычислим производную слудующего выражения:\n");
    
    equation_tex(out, tree, &data);

    vars vars = {};
    create_vars(&vars, 3);
    find_vars(tree, &vars);

    for (int i = 0; i < vars.qant; i++)
    {
        fprintf(out, "Производная по %s\n", vars.data[i]);
        full_diff(out, tree, vars.data[i], &data);
    }

    full_der(out, tree, &vars, &data);

    tex_teylor(out, tree, &vars, &data, 3);

    fclose(out);

    copy_file("tex/equation.tex", "tex/end.tex", "ab");

    return SUCCESS;
}

void full_diff (FILE* out, Node* tree, char* part, Data* data)
{
    if (tree->left != NULL && tree->left->type != DEFUALT)
        full_diff(out, tree->left, part, data);
    
    if (tree->right != NULL && tree->right->type != DEFUALT)
        full_diff(out, tree->right, part, data);

    tex_random_phrase(out, data);
    derivative_tex(out, tree, part);
}

void full_der (FILE* out, Node* tree, vars* vars, Data* data)
{
    Node* Diff = NULL;

    Stack mem_stk = {};
    stack_ctor(&mem_stk, 1);

    fprintf(out, "Полный дифференциал: \n");
    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n d(%s) = ", equation_tex_(tree, &mem_stk));

    stack_dtor(&mem_stk);

    // printf("vars.qant - %d\n", vars->qant);

    for (int i = 0; i < vars->qant; i++)
    {
        Diff = diff(tree, vars->data[i]);
        simplifier(Diff);

        stack_ctor(&mem_stk, 1);

        tex_random_phrase(out, data);
        fprintf(out, "(%s)d%s", equation_tex_(Diff, &mem_stk), vars->data[i]);

        stack_dtor(&mem_stk);
        
        if (i != vars->qant - 1)
            fprintf(out, " + ");
    }

    fprintf(out, "\n%s\n\n", "\\end{dmath}");
}

void tex_teylor (FILE* out, Node* tree, vars* vars, Data* data, int accuracy)
{
    Node* Diff = NULL;

    Stack mem_stk = {};

    for (int j = 0; j < vars->qant; j++)
    {
        Diff = tree;
        stack_ctor(&mem_stk, 1);

        fprintf(out, "Разложение в ряд тейлора с точностью до %dого члена при %s -> 0: \n", accuracy, vars->data[j]);
        fprintf(out, "\n\n%s", "\\begin{dmath}");
        fprintf(out, "\n %s = %s", equation_tex_(Diff, &mem_stk), equation_tex_(Diff, &mem_stk));

        stack_dtor(&mem_stk);

        for (int i = 1; i < accuracy; i++)
        {
            fprintf(out, " + ");

            Diff = diff(Diff, vars->data[j]);
            simplifier(Diff);

            stack_ctor(&mem_stk, 1);
            fprintf(out, "(%s)", equation_tex_(Diff, &mem_stk));
            stack_dtor(&mem_stk);
        }
        fprintf(out, "\n + o(x^%d)", accuracy);
        fprintf(out, "\n%s\n\n", "\\end{dmath}");
    }
}

void find_vars (Node* tree ,vars* vars)
{
    if (tree->type == VAR)
    {
        add_var(vars, tree->data.var);
    }

    if (tree->left != NULL && tree->left->type != DEFUALT)
        find_vars(tree->left, vars);
    
    if (tree->right != NULL && tree->right->type != DEFUALT)
        find_vars(tree->right, vars);
}

void create_vars (vars* vars, int capacity)
{
    vars->capacity = capacity;
    vars->qant = 0;
    vars->data = (char**)calloc(capacity, sizeof(char*));
}

void add_var (vars* vars, char* var)
{
    for (int i = 0; i < vars->qant; i++)
        if (strcmp(vars->data[i], var) == 0)
            return;

    if (vars->qant == vars->capacity)
    {
        vars->capacity *= 2;
        vars->data = (char**)realloc(vars->data, sizeof(char*) * vars->capacity);
    }

    vars->data[vars->qant] = strdup(var);
    vars->qant++;
}

void dump_vars (vars* vars)
{
    printf("capacity - %d\n", vars->capacity);
    printf("qant - %d\n", vars->qant);

    for (int i = 0; i < vars->qant; i++)
        printf("%s ", vars->data[i]);
}

Data input_data (const char* file_name)
{
    srand( (unsigned)time(NULL) );
    FILE* input = fopen(file_name, "rb");
    int fsize = GetFileSize(input);

    char* buffer = NULL;
    fill_buffer(input, &buffer);

    char* buffer_cp = buffer;

    // printf ("Buffer: \n %s", buffer);

    fclose(input);

    Data data = {};

    for (int i = 0; i < fsize + 1; i++)
        if (buffer[i] == '\n' && buffer[i - 1] == '\r')
        {
            data.quant += 1;
            buffer[i] = '\0';
            buffer[i - 1] = '\0';
        }

    data.lines = (line*)calloc(data.quant, sizeof(line));

    if (buffer[fsize - 1] != '\n')
        data.quant += 1;

    for (int i = 0; i < data.quant; i++)
    {
        data.lines[i].str = strdup(buffer);
        data.lines[i].len = strlen(buffer);
        buffer += strlen(buffer);

        while (*buffer == '\0') 
            buffer++;
    }

    free(buffer_cp);

    return data;
}

void tex_random_phrase (FILE* tex, Data* data)
{
    fprintf(tex, "\n%s\n", data->lines[rand() % data->quant].str);
}

void clear_data (Data* data)
{
    for (int i = 0; i < data->quant; i++)
        free(data->lines[i].str);

    free(data->lines);
    free(data);
}

void dump_data (Data* data)
{
    printf ("qant - %d \n", data->quant);
    for (int i = 0; i < data->quant; i++)
        printf("%s \n", data->lines[i].str);
}