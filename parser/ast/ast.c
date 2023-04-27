#include <stdio.h>
#include "ast.h"
#include "mod.h"

struct MetaData *new_meta_data(enum NodeType node_type)
{
    struct MetaData *meta_data = (struct MetaData *)g_malloc(sizeof(struct MetaData));
    meta_data->node_type = node_type;

    return meta_data;
}

struct MetaData *new_meta_data_func(enum BuiltinFunction func_type)
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

void destroy_meta_data(struct MetaData *meta_data)
{
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
    g_node_traverse(ast, G_IN_ORDER, G_TRAVERSE_ALL, -1, destroy_data, NULL);
    g_node_destroy(ast);
}

GNode *new_func(enum BuiltinFunction func_type, GNode *exp)
{
    struct MetaData *meta_data = new_meta_data_func(func_type);
    GNode *const ast = g_node_new(meta_data);
    g_node_append(ast, exp);

    return ast;
}

GNode *new_list(GNode *pre, GNode *list)
{
    GNode *ast;
    if (list == NULL)
    {
        ast = new_ast(NODE_LIST, pre, NULL);
    }
    else
    {
        ast = list;
        struct MetaData *const meta_data = ast->data;
        if (meta_data->node_type != NODE_LIST)
        {
            g_debug("abort:list should have type 'list node'");
            return ast;
        }
        GNode *first_child = g_node_first_child(ast);
        g_node_insert_before(ast, first_child, pre);
    }
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

GNode *do_func_embed(GNode *symbol, GNode *exp)
{
    struct MetaData *meta_data = new_meta_data(NODE_FUNC_EMBED);
    GNode *ast = g_node_new(meta_data);
    g_node_append(ast, exp);
    GNode *exp_expanded = emit(exp);
    g_node_insert_before(ast, exp_expanded, symbol);
    return ast;
}