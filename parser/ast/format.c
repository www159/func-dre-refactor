#include "ast.h"
#include <math.h>
#include "mod.h"

GNode *clear_all(GNode *exp) {
    GNode *exp_zero_cleared = clear_zero(exp);
    GNode *exp_one_cleared = clear_one(exp_zero_cleared);
    destroy_ast(exp_zero_cleared);
    return exp_one_cleared;
}

#define destroy_l (destroy_ast(l_exp))
#define destroy_r (destroy_ast(r_exp))
#define destroy         \
    destroy_ast(l_exp); \
    destroy_ast(r_exp)

#define clear(node_type)              \
    l_exp = clear_##node_type(l_exp); \
    r_exp = clear_##node_type(r_exp)

#define clear_l(node_type) \
    l_exp = clear_##node_type(l_exp)
static gboolean is_num_zero(GNode *exp);
GNode *clear_zero(GNode *exp)
{

    GNode *l_exp = NULL;
    GNode *r_exp = NULL;
    get_l_r_exp(exp, &l_exp, &r_exp);
    struct MetaData *meta_data = exp->data;
    switch (meta_data->node_type)
    {
    case NODE_ADD:
        clear(zero);
        if (is_num_zero(l_exp) && is_num_zero(r_exp))
        {
            destroy;
            return new_num(0);
        }
        if (is_num_zero(l_exp))
        {
            destroy_l;
            return r_exp;
        }
        if (is_num_zero(r_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_ADD, l_exp, r_exp);
    case NODE_SUB:
        clear(zero);
        if (is_num_zero(l_exp) && is_num_zero(r_exp))
        {
            destroy;
            return new_num(0);
        }
        if (is_num_zero(l_exp))
        {
            destroy_l;
            return new_ast(NODE_MINUS, r_exp, NULL);
        }
        if (is_num_zero(r_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_SUB, l_exp, r_exp);
    case NODE_MUL:
        clear(zero);
        if (is_num_zero(l_exp) || is_num_zero(r_exp))
        {
            destroy;
            return new_num(0);
        }
        return new_ast(NODE_MUL, l_exp, r_exp);
    case NODE_DIV:
        clear(zero);
        if (is_num_zero(l_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_DIV, l_exp, r_exp);
    case NODE_POWER:
        clear(zero);
        if (is_num_zero(r_exp))
        {
            destroy;
            return new_num(1);
        }
        if (is_num_zero(l_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_POWER, l_exp, r_exp);
    case NODE_MINUS:
        clear_l(zero);
        if (is_num_zero(l_exp))
        {
            return l_exp;
        }
        return new_ast(NODE_MINUS, l_exp, NULL);
    default:
        return copy_node(exp);
    }
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

static gboolean is_num_one(GNode *exp);

GNode *clear_one(GNode *exp)
{
    GNode *l_exp = NULL;
    GNode *r_exp = NULL;
    get_l_r_exp(exp, &l_exp, &r_exp);
    struct MetaData *meta_data = exp->data;

    switch (meta_data->node_type)
    {
    case NODE_MUL:
        clear(one);
        if (is_num_one(l_exp))
        {
            destroy_l;
            return r_exp;
        }
        if (is_num_one(r_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_MUL, l_exp, r_exp);
    case NODE_DIV:
        clear(one);
        if (is_num_one(r_exp))
        {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_DIV, l_exp, r_exp);
    case NODE_POWER:
        clear(one);
        if(is_num_one(l_exp) || is_num_one(r_exp)) {
            destroy_r;
            return l_exp;
        }
        return new_ast(NODE_POWER, l_exp, r_exp);
    case NODE_ADD:
        clear(one);
        return new_ast(NODE_ADD, l_exp, r_exp);
    case NODE_SUB:
        clear(one);
        return new_ast(NODE_SUB, l_exp, r_exp);
    case NODE_MINUS:
        clear_l(one);
        return new_ast(NODE_MINUS, l_exp, NULL);
    default:
        return copy_node(exp);
    }
}

static gboolean is_num_one(GNode *exp)
{
    struct MetaData *meta_data = exp->data;
    if (meta_data->node_type == NODE_NUMBER && fabs(meta_data->val - 1) < 1e-6)
    {
        return TRUE;
    }
    return FALSE;
}

#undef destroy_l
#undef destroy_r
#undef destroy
#undef clear
#undef clear_l