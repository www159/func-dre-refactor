#include "ast.h"
#include <math.h>
#include "mod.h"

static gboolean is_num_zero(GNode *exp);
GNode *clear_zero(GNode *exp)
{
    GNode *l_exp = g_node_first_child(exp);
    GNode *r_exp = NULL;
    if (l_exp != NULL)
    {
        r_exp = l_exp->next;
    }
    struct MetaData *meta_data = exp->data;
    switch (meta_data->node_type)
    {
    case NODE_ADD:
        l_exp = clear_zero(l_exp);
        r_exp = clear_zero(r_exp);
        if (is_num_zero(l_exp) && is_num_zero(r_exp))
        {
            GNode *zero = new_num(0);
            replace_node(exp, zero);
            return zero;
        }
        if (is_num_zero(l_exp))
        {
            g_node_unlink(r_exp);
            replace_node(exp, r_exp);
            return r_exp;
        }
        if (is_num_zero(r_exp))
        {
            g_node_unlink(l_exp);
            replace_node(exp, l_exp);
            return l_exp;
        }
        break;

    case NODE_SUB:
        l_exp = clear_zero(l_exp);
        r_exp = clear_zero(r_exp);
        if (is_num_zero(l_exp) && is_num_zero(r_exp))
        {
            GNode *zero = new_num(0);
            replace_node(exp, zero);
            return zero;
        }
        if (is_num_zero(l_exp))
        {
            g_node_unlink(l_exp);
            destroy_ast(l_exp);
            meta_data->node_type = NODE_MINUS;
            return exp;
        }
        if (is_num_zero(r_exp))
        {
            g_node_unlink(l_exp);
            replace_node(exp, l_exp);
            return l_exp;
        }
        break;
    case NODE_MUL:
        l_exp = clear_zero(l_exp);
        r_exp = clear_zero(r_exp);
        if (is_num_zero(l_exp) || is_num_zero(r_exp))
        {
            GNode *zero = new_num(0);
            replace_node(exp, zero);
            return zero;
        }
        break;
    case NODE_DIV:
        l_exp = clear_zero(l_exp);
        r_exp = clear_zero(r_exp);
        if (is_num_zero(l_exp))
        {
            g_node_unlink(l_exp);
            replace_node(exp, l_exp);
            return l_exp;
        }
        break;
    case NODE_POWER:
        l_exp = clear_zero(l_exp);
        r_exp = clear_zero(r_exp);
        if (is_num_zero(r_exp))
        {
            GNode *one = new_num(1);
            replace_node(exp, one);
            return one;
        }
        if (is_num_zero(l_exp))
        {
            g_node_unlink(l_exp);
            replace_node(exp, l_exp);
            return l_exp;
        }
        break;
    case NODE_MINUS:
        l_exp = clear_zero(l_exp);
        if (is_num_zero(l_exp))
        {
            g_node_unlink(l_exp);
            replace_node(exp, l_exp);
            return l_exp;
        }
    default:
        break;
    }
    return exp;
}

static gboolean is_num_zero(GNode *exp)
{
    struct MetaData *meta_data = exp->data;
    if (meta_data->node_type == NODE_NUMBER && fabs(meta_data->val) < 1e-6)
    {
        return TRUE;
    }
    return FALSE;
}