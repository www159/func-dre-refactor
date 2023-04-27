#include "ast.h"

void simplify(GNode *ast)
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
        simplify(l_exp);
        simplify(r_exp);
        break;

    // one exp
    case 
    default:
        simplify(ast);
    }

    if (G_NODE_IS_ROOT(ast))
    {
        destroy_ast(ast);
    }
}