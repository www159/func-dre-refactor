#include <stdio.h>
#include "ast.h"
#include "mod.h"

static void handle_print(GNode *ast);
static void builtin_print(GNode *exp, enum NodeType parent_type);
static void parens_or_not(enum NodeType cur_type, enum NodeType parent_type, int is_left);
static int priority_larger(enum NodeType src, enum NodeType dest);
// handle built_in_func
GNode *handle_func_builtin(GNode *ast)
{
    struct MetaData *meta_data = ast->data;

    switch (meta_data->func_type)
    {
    case B_PRINT:
        handle_print(ast);
        return NULL;
    case B_DRE:
        break;
    }
}

// handle func call 'print(exp)'
// .e.g
// let a = x^5+3;
// print(a);
// # output: a = x^5+3;
// argument is rvalue
// so print(a) will be expanded to
// print(x^5+3);
static void handle_print(GNode *ast)
{
    GNode *exp = g_node_first_child(ast);
    struct MetaData *meta_data = exp->data;
    if (meta_data->node_type == NODE_NAME)
    {

        printf("%s = ", meta_data->name->str);
    }
    else
    {
        printf("rvalue = ");
    }

    exp = emit(exp);
    builtin_print(exp, (enum NodeType)NULL);
    printf("\n");
}

#define print_two_exp(cur_type, parent_type, operator) \
    parens_or_not(cur_type, parent_type, TRUE);        \
    builtin_print(l_exp, cur_type);                    \
    printf(operator);                                  \
    builtin_print(r_exp, cur_type);                    \
    parens_or_not(cur_type, parent_type, FALSE)

static void builtin_print(GNode *exp, enum NodeType parent_type)
{
    struct MetaData *const meta_data = exp->data;

    // warn macro dependenies
    GNode *l_exp = NULL;
    GNode *r_exp = NULL;
    get_l_r_exp(exp, &l_exp, &r_exp);

    switch (meta_data->node_type)
    {
    // two exp
    case NODE_ADD:
        print_two_exp(NODE_ADD, parent_type, "+");
        break;

    case NODE_SUB:
        print_two_exp(NODE_SUB, parent_type, "-");
        break;
    case NODE_MUL:
        print_two_exp(NODE_MUL, parent_type, "*");
        break;
    case NODE_DIV:
        print_two_exp(NODE_DIV, parent_type, "/");
        break;
    case NODE_POWER:
        print_two_exp(NODE_POWER, parent_type, "^");
        break;
    // one_exp
    case NODE_MINUS:
        parens_or_not(NODE_MINUS, parent_type, FALSE);
        printf("-");
        builtin_print(l_exp, NODE_OUTER_MINUS);
        parens_or_not(NODE_MINUS, parent_type, FALSE);
        break;
    case NODE_LN:
        printf("ln(");
        builtin_print(l_exp, NODE_LN);
        printf(")");
        break;
    case NODE_EXP:
        printf("e");
        break;
    // no exp
    case NODE_NUMBER:
        printf("%f", meta_data->val);
        break;
    case NODE_X:
        printf("x");
        break;
    case (enum NodeType)NULL:
        break;
    default:
        yyerror("error expression define");
    }
}
static void parens_or_not(enum NodeType cur_type, enum NodeType parent_type, int is_left)
{
    if (priority_larger(parent_type, cur_type))
    {
        if (is_left)
        {
            printf("(");
        }
        else
        {
            printf(")");
        }
    }
}

// compare priority if exp
// @return TRUE if src > dest
// @return FALSE if src <= dest
static int priority_larger(enum NodeType src, enum NodeType dest)
{
    switch (src)
    {
    case NODE_SUB:
    case NODE_ADD:
        switch (dest)
        {
        case NODE_MINUS:
            return 1;
        default:
            return 0;
        }

    case NODE_MUL:
    case NODE_DIV:
        switch (dest)
        {
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MINUS:
            return 1;
        default:
            return 0;
        }

    case NODE_POWER:
        switch (dest)
        {
        case NODE_POWER:
        case NODE_OUTER_MINUS:
            return 0;
        default:
            return 1;
        }

    case NODE_OUTER_MINUS:
        switch (dest)
        {
        case NODE_OUTER_MINUS:
            return 0;
        default:
            return 1;
        }
    default:
        return 0;
    }
}

GNode *handle_dre_immut(GNode *exp)
{
    GNode *exp_copy = copy_node(exp);
    handle_dre(exp_copy);
    return exp_copy;
}

void handle_dre(GNode *exp)
{
    struct MetaData *meta_data = exp->data;
    GNode *l_exp = NULL;
    GNode *r_exp = NULL;
    get_l_r_exp(exp, &l_exp, &r_exp);

    switch (meta_data->node_type)
    {
    case NODE_ADD:
    case NODE_SUB:
        handle_dre(l_exp);
        handle_dre(r_exp);
        break;
    case NODE_MUL:
        g_node_unlink(l_exp);
        g_node_unlink(r_exp);
        GNode *l_exp_copy = copy_node(l_exp);
        GNode *r_exp_copy = copy_node(r_exp);
        handle_dre(l_exp);
        handle_dre(r_exp_copy);
        GNode *l_add = new_ast(NODE_MUL, l_exp, r_exp);
        GNode *r_add = new_ast(NODE_MUL, l_exp_copy, r_exp_copy);
        g_node_append(exp, l_add);
        g_node_append(exp, r_add);
        meta_data->node_type = NODE_ADD;
        break;
    case NODE_DIV:
    {
        g_node_unlink(l_exp);
        g_node_unlink(r_exp);
        GNode *r_exp_l_copy = copy_node(r_exp);
        GNode *r_exp_r_copy = copy_node(r_exp);
        GNode *l_exp_copy = copy_node(l_exp);
        handle_dre(l_exp);
        handle_dre(r_exp_r_copy);
        GNode *l_add = new_ast(NODE_MUL, l_exp, r_exp_l_copy);
        GNode *r_add = new_ast(NODE_MUL, l_exp_copy, r_exp_r_copy);
        GNode *l_div = new_ast(NODE_ADD, l_add, r_add);
        GNode *pow = new_num(2);
        GNode *r_div = new_ast(NODE_POWER, r_exp, pow);
        g_node_append(exp, l_div);
        g_node_append(exp, r_div);
        break;
    }
    case NODE_POWER:
    {
        GNode *exp_copy = copy_node(exp);
        g_node_unlink(l_exp);
        g_node_unlink(r_exp);
        GNode *l_exp_l_copy = copy_node(l_exp);
        GNode *l_exp_r_t_copy = copy_node(l_exp);
        GNode *l_exp_r_b_copy = copy_node(l_exp);
        GNode *r_exp_l_copy = copy_node(r_exp);
        GNode *r_exp_r_copy = copy_node(r_exp);
        handle_dre(r_exp_l_copy);
        handle_dre(l_exp_r_t_copy);
        GNode *mul = new_ast(NODE_LN, l_exp_l_copy, NULL);
        GNode *l_add = new_ast(NODE_MUL, r_exp_l_copy, mul);
        GNode *r_t = new_ast(NODE_MUL, l_exp_r_t_copy, r_exp_r_copy);
        GNode *r_add = new_ast(NODE_DIV, r_t, l_exp_r_b_copy);
        mul = new_ast(NODE_ADD, l_add, r_add);
        meta_data->node_type = NODE_MUL;
        g_node_append(exp, mul);
        g_node_append(exp, exp_copy);
        break;
    }
    case NODE_MINUS:
        handle_dre(l_exp);
        break;
    case NODE_LN:
        g_node_unlink(l_exp);
        GNode *top = copy_node(l_exp);
        handle_dre(top);
        g_node_append(exp, top);
        g_node_append(exp, l_exp);
        meta_data->node_type = NODE_DIV;
        break;
    case NODE_X:
        meta_data->node_type = NODE_NUMBER;
        meta_data->val = 1;
        break;
    case NODE_EXP:
        meta_data->node_type = NODE_NUMBER;
    case NODE_NUMBER:
        meta_data->val = 0;
        break;
    }
}