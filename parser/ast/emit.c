#include <stdlib.h>
#include <math.h>
#include "ast.h"
#include "mod.h"

// FIXME kill global var
typedef struct
{
    int dre_cnt;
} EmitSingal;

int dre_cnt = 0;
gboolean is_func_embed = FALSE;

static GNode *handle_assign(GNode *ast);
static GNode *handle_list(GNode *ast);
static GNode *handle_drevide(GNode *ast);
static GNode *handle_func_embed(GNode *ast);
static GNode *handle_symbol(GNode *ast);
static GNode *handle_exp(GNode *ast);
GNode *emit(GNode *ast)
{

    if (ast == NULL)
    {
        g_debug("abort:failed to emit NULL ast");
        return;
    }

    struct MetaData *meta_data = ast->data;

    g_debug("emit '%s'", node_type_str(meta_data->node_type));

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
    case NODE_ASSIGN:
        return handle_assign(ast);
    default:
        return handle_exp(ast);
    }
}

GNode *handle_assign(GNode *ast)
{
    GNode *symbol = g_node_first_child(ast);
    struct MetaData *meta_data = symbol->data;
    GNode *exp = symbol->next;
    emit(symbol);
    exp = emit(exp);
    replace_symbol_exp(meta_data->name, exp);
    return look_up_symbol_table(meta_data->name);
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
        l_exp = emit(l_exp);
        r_exp = emit(r_exp);
        return new_ast(meta_data->node_type, l_exp, r_exp);

    // one exp
    case NODE_LN:
    case NODE_MINUS:
        GNode *exp = g_node_first_child(ast);
        exp = emit(exp);
        return new_ast(meta_data->node_type, exp, NULL);

    // no exp
    case NODE_EXP:
        return new_exp();
    case NODE_NUMBER:
        return new_num(meta_data->val);
    case NODE_X:
        return new_x();
    default:
        return NULL;
    }
}

static GNode *handle_symbol(GNode *ast)
{
    struct MetaData *meta_data = ast->data;
    GNode *symbol = look_up_symbol_table(meta_data->name);
    meta_data = symbol->data;

    if (!meta_data->declared)
    {
        g_debug("meet undeclared symbol: '%s' when expanding", meta_data->name->str);
        return NULL;
    }
    GNode *symbol_exp = g_node_first_child(symbol);
    g_node_unlink(symbol_exp);

    GNode *symbol_exp_cleared = symbol_exp;

    if (!is_func_embed)
    {
        if (dre_cnt > 0)
        {

            while (dre_cnt > 0)
            {
                dre_cnt--;
                handle_dre(symbol_exp);
            }
            symbol_exp_cleared = clear_all(symbol_exp);
            destroy_ast(symbol_exp);
        }
    }
    return symbol_exp_cleared;
}
static void embed(GNode *exp, GNode *embed_exp);
static double calc(GNode *_exp);

static GNode *handle_func_embed(GNode *const ast)
{
    is_func_embed = TRUE;
    GNode *symbol = g_node_first_child(ast);
    GNode *embed_exp = symbol->next;
    GNode *symbol_exp = emit(symbol);
    embed_exp = emit(embed_exp);
    g_node_children_foreach(symbol_exp, G_TRAVERSE_ALL, embed, embed_exp);

    // embed_exp should dead
    destroy_ast(embed_exp);

    struct MetaData *meta_data = embed_exp->data;
    if (meta_data->node_type == NODE_EXP || meta_data->node_type == NODE_NUMBER)
    {
        double val = calc(symbol_exp);
        destroy_ast(symbol_exp);
        return new_num(val);
    }

    // handle dre id
    // .e.g
    // let b = a'''(a);
    while (dre_cnt > 0)
    {
        dre_cnt--;
        handle_dre(symbol_exp);
    }

    GNode *symbol_exp_cleared = clear_all(symbol_exp);
    destroy_ast(symbol_exp);

    is_func_embed = FALSE;
    return symbol_exp_cleared;
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
    GNode *l_exp = NULL;
    GNode *r_exp = NULL;
    get_l_r_exp(_exp, &l_exp, &r_exp);
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

static GNode *handle_drevide(GNode *ast)
{
    GNode *exp = g_node_first_child(ast);
    dre_cnt++;
    return emit(exp);
}
