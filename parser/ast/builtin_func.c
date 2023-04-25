#include "ast.h"
#include "mod.h"

static void handle_print(GNode *ast);
static void builtin_print(GNode *exp, enum NodeType parent_type);
static void parens_or_not(enum NodeType cur_type, enum NodeType parent_type, int is_left);
static int priority_larger(enum NodeType src, enum NodeType dest);

// handle built_in_func
void handle_func_builtin(GNode *ast)
{
    struct MetaData *meta_data = ast->data;

    switch (meta_data->func_type)
    {
    case B_PRINT:
        handle_print(ast);
    }
}

// handle func call 'print(exp)'
// .e.g
// let a = x^5+3;
// print(a);
// output: x^5+3;
static void handle_print(GNode *ast)
{
    GNode *const exp = g_node_first_child(ast);
    GNode *exp_mut = NULL;
    struct MetaData *const meta_data = exp->data;
    if (meta_data->node_type == NODE_NAME)
    {
        if (!is_declare(exp))
        {
            return;
        }

        printf("%s = ", meta_data->name->str);

        exp_mut = g_node_first_child(exp);
    }
    else
    {
        exp_mut = exp;
        if (!is_declare(exp_mut))
        {
            return;
        }
        // expend rvalue
        if (!try_expand(exp_mut))
        {
            return;
        }
        printf("rvalue = ");
    }

    builtin_print(exp_mut, (enum NodeType)NULL);
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
    GNode *const l_exp = g_node_first_child(exp);
    GNode *r_exp = NULL;
    if (l_exp != NULL)
    {
        r_exp = l_exp->next;
    }

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