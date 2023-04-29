#include "ast.h"
#include "mod.h"

void destroy_data(GNode *node)
{
    struct MetaData *meta_data = node->data;
    destroy_meta_data(meta_data);
}

gboolean is_declare(GNode *exp)
{
    struct MetaData *const meta_data = exp->data;
    if (meta_data->node_type == NODE_NAME && !meta_data->declared)
    {
        yyerror("undeclared symbol: '%s'", meta_data->name->str);
        return FALSE;
    }
    return TRUE;
}

void replace_node(GNode *src, GNode *dest)
{
    if (G_NODE_IS_ROOT(src))
    {
        destroy_ast(src);
        return;
    }
    GNode *parent = src->parent;
    guint pos = g_node_child_position(parent, src);
    g_node_unlink(src);
    destroy_ast(src);
    g_node_insert(parent, pos, dest);
}

void replace_node_unlink(GNode *src, GNode *dest)
{
    if (G_NODE_IS_ROOT(src))
    {
        return;
    }
    GNode *parent = src->parent;
    guint pos = g_node_child_position(parent, src);
    g_node_unlink(src);
    g_node_insert(parent, pos, dest);
}

char *const node_type_str(enum NodeType node_type)
{
    switch (node_type)
    {
    case NODE_ADD:
        return "node add";
    case NODE_NAME:
        return "node name";
    case NODE_NUMBER:
        return "node number";
    case NODE_SUB:
        return "node sub";
    case NODE_MUL:
        return "node mul";
    case NODE_DIV:
        return "node div";
    case NODE_POWER:
        return "node power";
    case NODE_MINUS:
        return "node minus";
    case NODE_OUTER_MINUS:
        return "node outer minus";
    case NODE_LN:
        return "node ln";
    case NODE_EXP:
        return "node exp";
    case NODE_X:
        return "node x";
    case NODE_DREVIDE:
        return "node dervide";
    case NODE_FUNC_EMBED:
        return "node func embed";
    case NODE_FUNC_BUILT_IN:
        return "node built in";
    case NODE_LIST:
        return "node list";
    case NODE_ASSIGN:
        return "node assign";
    default:
        return "";
    }
}

void get_l_r_exp(GNode *exp, GNode **l_exp, GNode **r_exp) {
    *l_exp = g_node_first_child(exp);
    *r_exp = NULL;
    if (*l_exp != NULL)
    {
        *r_exp = (*l_exp)->next;
    }
}