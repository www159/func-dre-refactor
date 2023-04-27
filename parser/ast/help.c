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
    GNode *parent = src->parent;
    guint pos = g_node_child_position(parent, src);
    g_node_unlink(src);
    destroy_ast(src);
    g_node_insert(parent, pos, dest);
}

void replace_node_unlink(GNode *src, GNode *dest)
{
    GNode *parent = src->parent;
    guint pos = g_node_child_position(parent, src);
    g_node_unlink(src);
    g_node_insert(parent, pos, dest);
}