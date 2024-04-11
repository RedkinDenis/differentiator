#include "headers/tex_out.h"
#include "headers/input_output.h"
#include "headers/DSL.h"
#include "..\UDL.h"
#include "..\err_codes.h"

#include <stdlib.h>
#include <string.h>

struct vars 
{
    char** data = NULL;
    size_t capacity = 0;
    size_t qant = 0;
};

static err copy_file (const char* to, const char* from, const char* mode);

static err derivative_tex (FILE* out, Node* tree, const char* part);

static void equation_tex (FILE* out, Node* tree);

static char* equation_tex__ (Node* tree, int* br);

static char* equation_tex_ (Node* tree);

static void dump_vars (vars* vars);

static void add_var (vars* vars, char* var);

static void create_vars (vars* vars, int capacity);

static void find_vars (Node* tree ,vars* vars);

static void full_der (FILE* out, Node* tree, vars* vars);

static void full_diff (FILE* out, Node* tree, char* part);

char* equation_tex_ (Node* tree)
{
    int br = 0;
    return equation_tex__(tree, &br);
}

char* equation_tex__ (Node* tree, int* br)
{
    int buf_size = 100;
    char* buf = (char*)calloc(buf_size, sizeof(char));

    if (tree->data.operand == MUL || tree->data.operand == DIV)
    {
        // printf("set br = 1\n");
        *br = 1;
    }

    if (tree->type == OPERAND)
    {
        if (tree->data.operand == DIV)
        {
            SPRINTF(buf, buf_size, "\\cfrac {%s} {%s}", equation_tex__(tree->left, br), equation_tex__(tree->right, br));
        }
        else  if (*br == 1 && (tree->data.operand == ADD || tree->data.operand == SUB))
        {
            SPRINTF(buf, buf_size, "(%s %c %s) ", equation_tex__(tree->left, br), tree->data.operand, equation_tex__(tree->right, br));
            *br = 0;
        }
        else 
        {
            SPRINTF(buf, buf_size, "%s %c %s ", equation_tex__(tree->left, br), tree->data.operand, equation_tex__(tree->right, br));
        }

        return buf;
    }
    
    else if (tree->type == NUM)
    {
        SPRINTF(buf, buf_size, "%lf", tree->data.value);
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

    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n \\cfrac {d} {d %s}(%s) = %s", part, equation_tex_(tree), equation_tex_(Diff));
    fprintf(out, "\n%s\n\n", "\\end{dmath}");

    return SUCCESS;
}

void equation_tex (FILE* out, Node* tree)
{
    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n%s", equation_tex_(tree));
    fprintf(out, "\n%s\n\n", "\\end{dmath}");
}

err diff_tex (Node* tree)
{
    copy_file("tex/equation.tex", "tex/preambule_article.tex", "wb");
    copy_file("tex/equation.tex", "tex/title.tex", "ab");

    FOPEN(out, "tex/equation.tex", "ab");
    fprintf(out, "Вычислим полный дифференциал следующей функции:\n");
    equation_tex(out, tree);

    vars vars = {};
    create_vars(&vars, 3);
    find_vars(tree, &vars);

    for (int i = 0; i < vars.qant; i++)
    {
        fprintf(out, "Найдем производную по %s\n", vars.data[i]);
        full_diff(out, tree, vars.data[i]);
    }

    full_der(out, tree, &vars);

    fclose(out);

    copy_file("tex/equation.tex", "tex/end.tex", "ab");

    return SUCCESS;
}

void full_diff (FILE* out, Node* tree, char* part)
{
    if (tree->left != NULL)
        full_diff(out, tree->left, part);
    
    if (tree->right != NULL)
        full_diff(out, tree->right, part);

    derivative_tex(out, tree, part);
}

void full_der (FILE* out, Node* tree, vars* vars)
{
    Node* Diff = NULL;

    fprintf(out, "Полный дифференциал: \n");
    fprintf(out, "\n\n%s", "\\begin{dmath}");
    fprintf(out, "\n d(%s) = ", equation_tex_(tree));

    for (int i = 0; i < vars->qant; i++)
    {
        Diff = diff(tree, vars->data[i]);
        simplifier(Diff);

        fprintf(out, "(%s)d%s", equation_tex_(Diff), vars->data[i]);
        
        if (i != vars->qant - 1)
            fprintf(out, " + ");
    }

    fprintf(out, "\n%s\n\n", "\\end{dmath}");
}

void find_vars (Node* tree ,vars* vars)
{
    if (tree->type == VAR)
    {
        add_var(vars, tree->data.var);
    }

    if (tree->left != NULL)
        find_vars(tree->left, vars);
    
    if (tree->right != NULL)
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
