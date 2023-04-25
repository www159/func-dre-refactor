#include "ast.h"

static void handle_assign(GNode *ast);
void emit(GNode *ast)
{
    printf("start emit\n");

    if (ast == NULL)
    {
        printf("empty ast!");
    }

    struct MetaData *meta_data = ast->data;

    switch (meta_data->node_type)
    {
    case NODE_ASSIGN:
        handle_assign(ast);
        break;
    case NODE_FUNC_BUILT_IN:
        handle_func_builtin(ast);
        break;
    }
}


// assignment prog
//  NODE_ASSIGN
//     _|________
//    |          |
// NODE_SYMBOL  NODE_EXP
//
// move exp as symbol' children
static void handle_assign(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    GNode *exp = symbol->next;
    GNode *symbol_exp = g_tree_node_first(symbol);

    // unlink and destroy old exp of symbol
    if (symbol_exp != NULL)
    {
        g_node_unlink(symbol_exp);
        g_node_destroy(symbol_exp);
    }

    // unlink symbol from ast and replace ast
    g_node_unlink(symbol);
    destroy_ast(ast);
    g_node_append(symbol, exp);
    ast = symbol;
}