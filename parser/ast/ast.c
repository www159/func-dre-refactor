#include <stdio.h>
#include "ast.h"
#include "mod.h"

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
        g_string_free(meta_data->name, TRUE);
    }
    g_free(meta_data);
}

struct MetaData *copy_meta_data(struct MetaData *src)
{
    struct MetaData *dest = (struct MetaData *)g_malloc(sizeof(struct MetaData));
    memcpy(dest, src, sizeof(src));
    switch (src->node_type)
    {
    case NODE_NAME:
        dest->name = g_string_new(src->name->str);
        dest->declared = src->declared;
        break;
    case NODE_FUNC_BUILT_IN:
        dest->func_type = src->func_type;
        break;
    case NODE_NUMBER:
        dest->val = src->val;
        break;
    default:
        break;
    }

    return dest;
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
    struct MetaData *const meta_data = ast->data;
    g_node_traverse(ast, G_IN_ORDER, G_TRAVERSE_ALL, -1, destroy_meta_data, NULL);
    g_node_destroy(ast);
}

GNode *new_func(enum BuiltinFunction func_type, GNode *exp)
{
    struct MetaData *meta_data = new_meta_data_func(func_type);
    GNode *const ast = g_node_new(meta_data);
    g_node_append(ast, exp);

    return ast;
}

GNode *new_num(double num)
{
    struct MetaData *const meta_data = new_meta_data_num(num);
    GNode *const ast = g_node_new(meta_data);

    return ast;
}

GNode *new_x()
{
    struct MetaData *const meta_data = new_meta_data(NODE_X);
    GNode *const ast = g_node_new(meta_data);

    return ast;
}

GNode *new_exp()
{
    struct MetaData *const meta_data = new_meta_data(NODE_EXP);
    GNode *const ast = g_node_new(meta_data);

    return ast;
}

// assignment prog
//  NODE_ASSIGN
//     _|________
//    |          |
// NODE_SYMBOL  NODE_EXP
//
// move exp as symbol' children
// static void handle_assign(GNode *ast)
GNode *do_assign(GNode *symbol, GNode *exp)
{
    if(!is_declare(exp)) {
        return;
    }
    if (!try_expand(exp))
    {
        return;
    }
    struct MetaData *const meta_data = symbol->data;
    meta_data->declared = TRUE;
    GNode *symbol_exp = g_node_first_child(symbol);

    // unlink and destroy old exp of symbol
    if (symbol_exp != NULL)
    {
        g_node_unlink(symbol_exp);
        destroy_ast(symbol_exp);
    }

    g_node_append(symbol, exp);
    
    return symbol;
}