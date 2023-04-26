#include <stdlib.h>
#include <math.h>
#include "ast.h"
#include "mod.h"

static void handle_assign(GNode *ast);
static void handle_list(GNode *ast);
static void handle_drevide(GNode *ast);
static void handle_func_embed(GNode *ast);
static void handle_symbol(GNode *ast);
static void handle_exp(GNode *ast);
void emit(GNode *ast)
{
    g_debug("start emit");

    if (ast == NULL)
    {
        g_debug("abort:failed to emit NULL ast");
        return;
    }

    struct MetaData *meta_data = ast->data;

    switch (meta_data->node_type)
    {
    case NODE_FUNC_BUILT_IN:
        handle_func_builtin(ast);
        break;
    case NODE_LIST:
        handle_list(ast);
        break;
    case NODE_DREVIDE:
        handle_drevide(ast);
        break;
    case NODE_FUNC_EMBED:
        handle_func_embed(ast);
        break;
    case NODE_NAME:
        handle_symbol(ast);
        break;
    default:
        handle_exp(ast);
        break;
    }
}

static void do_assign(GNode *symbol, GNode *exp);
// assignment prog
// .e.g
//
// let symbol = exp;
//       parent
//         |
//     ASSIGN_NODE
//      ___|___
//     |       |
//   symbol   exp
//
// turn to
// parent -> symbol(exp)
// effect symbol table
// symbol -> exp
void emit_assign(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    GNode *exp = symbol->next;
    // just unlink symbol
    // avoid to effect symbol table
    g_node_unlink(symbol);
    g_node_unlink(exp);

    struct MetaData *const meta_data = symbol->data;
    meta_data->declared = TRUE;
    GNode *symbol_exp = g_node_first_child(symbol);
    if (symbol_exp != NULL)
    {
        g_node_unlink(symbol_exp);
        destroy_ast(symbol_exp);
    }

    g_node_append(symbol, exp);
    // try expand and embed
    emit(exp);
}

static void handle_list(GNode *ast)
{
    GNode *const last = g_node_last_child(ast);
    GNode *next = g_node_first_child(ast);
    GNode *cur = NULL;
    do
    {
        cur = next;
        emit(cur);
        next = cur->next;
    } while (cur != last);

    // unlink and destroy 'link node'
    g_node_unlink(ast);
    destroy_ast(ast);
}

// handle expression
// .e.g +,-,*,/,^,-
//
static void handle_exp(GNode *ast)
{
    if (ast == NULL)
    {
        g_debug("abort:failed to emit NULL ast");
        return;
    }

    struct MetaData *meta_data = ast->data;

    switch (meta_data->node_type)
    {
        // two exp
    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_POWER:
        GNode *const l_exp = g_node_first_child(ast);
        GNode *const r_exp = l_exp->next;
        emit(l_exp);
        emit(r_exp);
        break;

    // one exp
    case NODE_MINUS:
        GNode *const exp = g_node_first_child(ast);
        emit(exp);
        break;

    // no exp
    case NODE_LN:
    case NODE_EXP:
    case NODE_NUMBER:
    case NODE_X:
        break;
    }
}

// handle symbol node
// 1. expand exp with variable
// .e.g
// ```
// let a = x^5;
// let b = a+4*x;
// # b = x^5+4*x;
// ```
//    name node
//      |
//     exp(x^5)
//
//     parent
//     ___|___
//    |       |
// name node  exp(4*x)       symbol table
//                               |
//    parent                   name node
//   ___|____                    |
//  |        |                  exp(x^5)
// exp(x^5) exp(4*x)
// deep copy moved variable
static void handle_symbol(GNode *ast)
{
    struct MetaData *const meta_data = ast->data;
    if (!meta_data->declared)
    {
        g_debug("meet undeclared symbol: '%s' when expanding", meta_data->name->str);
        return;
    }
    GNode *const symbol_exp = g_node_first_child(ast);
    GNode *symbol_exp_copy = g_node_copy_deep(symbol_exp, copy_meta_data, NULL);
    GNode *const parent = ast->parent;
    if (parent == NULL)
    {
        g_debug("abort: should have parent");
        return;
    }
    gint pos = g_node_child_position(parent, ast);
    // just unlink, don't destroy
    // avoid to effect symbol table
    g_node_unlink(ast);

    g_node_insert(parent, pos, symbol_exp_copy);
    emit(symbol_exp_copy);
}

static void embed(GNode *exp, GNode *embed_exp);
static double calc(GNode *_exp);
// handle func embed
// .e.g
// let a = x^2 + x;
// let b = a(2*x);
// print(b);
// # output (2*x)^2 + 2*x;
// exception :
// let a = b^2 + b;
// let c = a(2*x);
// # c = a(2*x) = b(2*x)^2 + b(2*x);
// but expanding after every assignment;
//         parent
//           |
//       embed node
//       ____|____
//      |         |
//  name node    embed exp
//      |
//  symbol exp
// turn to
// parent -> (symbol exp)(embed exp)
static void handle_func_embed(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    GNode *embed_exp = symbol->next;
    // expand embedded exp and symbol
    emit(symbol);
    emit(embed_exp);
    // symbol node is deleted after expanding emit
    GNode *symbol_exp = g_node_first_child(ast);

    struct MetaData *meta_data = symbol_exp->data;
    if (meta_data->node_type == NODE_X)
    {
        // symbol exp is replaced with embed exp
        embed(symbol_exp, embed_exp);
        symbol_exp = g_node_first_child(ast);
    }
    else
    {
        g_node_children_foreach(symbol_exp, G_TRAVERSE_LEAVES, embed, embed_exp);
    }

    // replace embed node with symbol exp
    GNode *parent = ast->parent;
    guint pos = g_node_child_position(parent, ast);
    // unlink symbol exp before ast destroy
    g_node_unlink(symbol_exp);
    // if embed exp is number
    // replace symbol exp with number node with 
    // calc result
    struct MetaData *embed_exp_meta_data = embed_exp->data;
    if (embed_exp_meta_data->node_type == NODE_NUMBER)
    {
        double val = calc(symbol_exp);
        destroy_ast(symbol_exp);
        symbol_exp = new_num(val);
    }
    // unlink and destroy embed exp
    g_node_unlink(embed_exp);
    destroy_ast(embed_exp);
    // unlink and destroy ast
    g_node_unlink(ast);
    destroy_ast(ast);

    g_node_insert(parent, pos, symbol_exp);
}

// replace 'x' with embed exp
static void embed(GNode *exp, GNode *embed_exp)
{
    struct MetaData *const meta_data = exp->data;
    if (meta_data->node_type == NODE_X)
    {
        // deep copy embed exp
        GNode *embed_exp_copy = g_node_copy_deep(embed_exp, copy_meta_data, NULL);
        GNode *parent = exp->parent;
        guint pos = g_node_child_position(parent, exp);
        g_node_unlink(exp);
        destroy_ast(exp);
        g_node_insert(parent, pos, embed_exp_copy);
    }
}

static double calc(GNode *_exp)
{
    GNode *l_exp = g_node_first_child(_exp);
    GNode *r_exp = NULL;
    if (l_exp != NULL)
    {
        r_exp = l_exp->next;
    }
    struct MetaData *const meta_data = _exp->data;
    switch (meta_data->node_type)
    {
    case NODE_ADD:
        return calc(l_exp) + calc(r_exp);
    case NODE_SUB:
        return calc(l_exp) - calc(r_exp);
    case NODE_MUL:
        return calc(l_exp) * calc(r_exp);
    case NODE_DIV:
        return calc(l_exp) / calc(r_exp);
    case NODE_POWER:
        return pow(calc(l_exp), calc(r_exp));
    case NODE_EXP:
        return exp(1);
    case NODE_LN:
        return log(calc(l_exp));
    case NODE_MINUS:
        return -calc(l_exp);
    case NODE_NUMBER:
        return meta_data->val;
    default:
        g_debug("failed to calc unknown node");
        break;
    }
}

// handle derevide node
// .e.g
// let a = x^2;
// let b = a';
// similiar with func call:
// let b = dre(a);
// just replace meta data with 'dre func node' and re-emit it
static void handle_drevide(GNode *ast)
{
    struct MetaData *meta_data = ast->data;
    destroy_data(meta_data);
    meta_data = new_meta_data_func(B_DRE);
    ast->data = meta_data;
    emit(ast);
}
