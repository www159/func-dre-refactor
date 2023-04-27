#include <stdlib.h>
#include <math.h>
#include "ast.h"
#include "mod.h"

static GNode *handle_assign(GNode *ast);
static GNode *handle_list(GNode *ast);
static GNode *handle_drevide(GNode *ast);
static GNode *handle_func_embed(GNode *ast);
static GNode *handle_symbol(GNode *ast);
static GNode *handle_exp(GNode *ast);
GNode *emit(GNode *ast)
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
        return handle_func_builtin(ast);
    case NODE_LIST:
        return handle_list(ast);
    case NODE_DREVIDE:
        return handle_drevide(ast);
    case NODE_FUNC_EMBED:
        return handle_func_embed(ast);
    case NODE_NAME:
        return handle_symbol(ast);
    default:
        return handle_exp(ast);
    }
}

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
    // try to expand and embed
    emit(exp);
}

static GNode *handle_list(GNode *ast)
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
    return (GNode *)NULL;
}

// handle expression
// .e.g +,-,*,/,^,-
//
static GNode *handle_exp(GNode *ast)
{
    if (ast == NULL)
    {
        g_debug("abort:failed to emit NULL ast");
        return ast;
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
        GNode *l_exp = g_node_first_child(ast);
        GNode *r_exp = l_exp->next;
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
    return ast;
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
static GNode *handle_symbol(GNode *ast)
{
    struct MetaData *const meta_data = ast->data;
    if (!meta_data->declared)
    {
        g_debug("meet undeclared symbol: '%s' when expanding", meta_data->name->str);
        return ast;
    }
    GNode *const symbol_exp = g_node_first_child(ast);
    GNode *symbol_exp_copy = copy_node(symbol_exp);
    replace_node_unlink(ast, symbol_exp_copy);
    return emit(symbol_exp_copy);
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
static GNode *handle_func_embed(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    GNode *embed_exp = symbol->next;
    // expand embedded exp and symbol
    GNode *symbol_exp = emit(symbol);

    // symbol node is deleted after expanding emit

    struct MetaData *meta_data = symbol_exp->data;
    if (meta_data->node_type == NODE_X)
    {
        // symbol exp is replaced with embed exp
        embed(symbol_exp, embed_exp);
        symbol_exp = g_node_first_child(ast);
    }
    else
    {
        g_node_children_foreach(symbol_exp, G_TRAVERSE_ALL, embed, embed_exp);
    }
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

    // unlink symbol exp before ast destroy
    g_node_unlink(symbol_exp);
    replace_node(ast, symbol_exp);

    GNode *ret = symbol_exp;

    struct MetaData *parent_meta_data = symbol_exp->parent->data;
    while (parent_meta_data->node_type == NODE_FUNC_BUILT_IN && parent_meta_data->func_type == B_DRE)
    {
        // call built in drevide func
        ret = emit(symbol_exp->parent);
        parent_meta_data = symbol_exp->parent->data;
    }
    return ret;
}

// replace 'x' with embed exp
static void embed(GNode *exp, GNode *embed_exp)
{
    g_node_children_foreach(exp, G_TRAVERSE_ALL, embed, embed_exp);
    struct MetaData *const meta_data = exp->data;
    if (meta_data->node_type == NODE_X)
    {
        // deep copy embed exp
        GNode *embed_exp_copy = copy_node(embed_exp);
        GNode *parent = exp->parent;
        replace_node(exp, embed_exp_copy);
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
// .e.g.1
// let a = x^2;
// let b = a';
// similiar with func call:
// let b = dre(a);
//          assign node
//          _____|_____
//         |           |
//       symbol      dre node
//                     |
//                   symbol
// turn to
//          assign node
//          _____|_____
//         |           |
//       symbol     builtin dre
//                     |
//                   symbol
// just replace meta data with 'dre func node' and re-emit it
// .e.g.2
// let b = a'(x+3);
// similiar with let b = dre(a(x+3))
//         embed func node
//           _____|_____
//          |           |
// ast -> dre node      exp
//          |
//        symbol
// turn to
//          builtin dre
//               |
//         embed func node
//          _____|_____
//         |           |
// ast -> symbol       exp
// do not optimize urgently.
// code recurively
static GNode *handle_drevide(GNode *ast)
{
    GNode *parent = ast->parent;
    struct MetaData *meta_data = parent->data;
    // null node
    GNode *builtin_dre_node = g_node_new(new_meta_data_func(B_DRE));
    GNode *maybe_symbol = g_node_first_child(ast);
    GNode *ret = NULL;
    g_node_unlink(maybe_symbol);
    // .e.g.2
    if (meta_data->node_type == NODE_FUNC_BUILT_IN)
    {
        replace_node(ast, maybe_symbol);
        replace_node_unlink(parent, builtin_dre_node);
        g_node_append(builtin_dre_node, parent);
        return emit(maybe_symbol);
    }
    //.e.g.1
    else
    {
        replace_node(ast, builtin_dre_node);
        g_node_append(builtin_dre_node, maybe_symbol);
        emit(maybe_symbol);
        // call built in drevide func
        return emit(builtin_dre_node);
    }
}
