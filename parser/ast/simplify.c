#include "ast.h"

static void unlink_symbol(GNode *ast);
void simplify(GNode *ast)
{
    if (ast == NULL)
    {
        g_debug("abort:failed to simplify NULL ast");
        return;
    }
    unlink_symbol(ast);
    destroy_ast(ast);
}

static void unlink_symbol(GNode *ast)
{
    struct MetaData *const meta_data = ast->data;

    switch (meta_data->node_type)
    {
    // symbol node
    // unlink the node
    case NODE_NAME:
        g_node_unlink(ast);
        break;

    // two exp
    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_POWER:
        GNode *const l_exp = g_node_first_child(ast);
        GNode *const r_exp = l_exp->next;
        unlink_symbol(l_exp);
        unlink_symbol(r_exp);
        break;

    // one exp
    case NODE_MINUS:
    case NODE_FUNC_BUILT_IN:
        GNode *const expr = g_node_first_child(ast);
        unlink_symbol(expr);
        break;

    // no exp
    case NODE_LN:
    case NODE_EXP:
    case NODE_NUMBER:
    case NODE_X:
        break;
    default:
        yyerror("failed to unlink node");
        break;
    }
}