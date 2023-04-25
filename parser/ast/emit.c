#include "ast.h"
#include "mod.h"

static void handle_assign(GNode *ast);
static void handle_list(GNode *ast);
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
    case NODE_ASSIGN:
        handle_assign(ast);
        break;
    case NODE_LIST:
        handle_list(ast);
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
// parent -> symbol -> exp
static void handle_assign(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    if (symbol == NULL)
    {
        g_debug("abort:symbol in 'assign node' should not be null");
        return;
    }
    GNode *exp = symbol->next;
    g_node_unlink(symbol);
    g_node_unlink(exp);
    GNode *parent = ast->parent;
    if(parent == NULL) {
        g_debug("abort: 'assign node' should have parent");
        return;
    }
    g_node_unlink(ast);
    destroy_ast(ast);
    g_node_append(parent, symbol);
    do_assign(symbol, exp);
}

// effect symbol val in symbol_table
//  NODE_ASSIGN
//     _|________
//    |          |
// NODE_SYMBOL  NODE_EXP
//
// move exp as symbol' children
// static void handle_assign(GNode *ast)

static void do_assign(GNode *symbol, GNode *exp)
{
    if (!is_declare(exp))
    {
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
}