#include <stdio.h>
#include "ast.h"

static struct MetaData *new_meta_data(enum NodeType node_type)
{
    struct MetaData *meta_data = (struct MetaData *)g_malloc(sizeof(struct MetaData));
    meta_data->node_type = node_type;

    return meta_data;
}

static struct MetaData *new_meta_data_func(enum BuiltinFunction func_type)
{
    struct MetaData *meta_data = new_meta_data(NODE_FUNC_BUILT_IN);
    meta_data->func_type = func_type;

    return meta_data;
}

static struct MetaData *new_meta_data_num(double num)
{
    struct MetaData *meta_data = new_meta_data(NODE_NUMBER);
    meta_data->val = num;

    return meta_data;
}

struct MetaData *new_meta_data_symbol(GString *name)
{
    struct MetaData *meta_data = new_meta_data(NODE_NAME);
    meta_data->declared = FALSE;
    meta_data->name = name;

    return meta_data;
}

static void destroy_meta_data(GNode *node)
{
    struct MetaData *meta_data = node->data;
    if (meta_data->node_type == NODE_NAME)
    {
        g_free(meta_data->name);
    }
    g_free(meta_data);
}

GNode *new_ast(enum NodeType node_type, GNode *l_exp, GNode *r_exp)
{
    struct MetaData *meta_data = new_meta_data(node_type);
    GNode *ast = g_node_new(meta_data);
    g_node_append(ast, l_exp);

    if (r_exp == NULL)
    {
        return ast;
    }

    g_node_append(ast, r_exp);

    return ast;
}

void destroy_ast(GNode *ast)
{
    struct MetaData *meta_data = ast->data;
    g_node_traverse(ast, G_IN_ORDER, G_TRAVERSE_ALL, -1, destroy_meta_data, NULL);
    g_node_destroy(ast);
}

GNode *new_func(enum BuiltinFunction func_type, GNode *exp)
{
    struct MetaData *meta_data = new_meta_data_func(func_type);
    GNode *ast = g_node_new(meta_data);
    g_node_append(ast, exp);

    return ast;
}

GNode *new_num(double num)
{
    struct MetaData *meta_data = new_meta_data_num(num);
    GNode *ast = g_node_new(meta_data);

    return ast;
}

// first is symbol
// second is exp
GNode *do_def(GNode *symbol, GNode *exp)
{
    struct MetaData *meta_data = symbol->data;
    meta_data->declared = TRUE;
    meta_data = new_meta_data(NODE_ASSIGN);

    GNode *ast = g_node_new(meta_data);
    g_node_append(ast, symbol);
    g_node_append(ast, exp);

    return ast;
}

void yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    g_printerr("%d, error: ", yylineno);
    vfprintf(stderr, s, ap);
    g_printerr("\n");
}