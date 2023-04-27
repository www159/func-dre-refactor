#include "ast.h"
#include "mod.h"

GNode *func_call(GNode *func_call_exp, FuncCallFunc func_call_func)
{
    GNode *exp = g_node_first_child(func_call_exp);
    emit(exp);
    exp = g_node_first_child(func_call_exp);
    g_node_unlink(exp);
    replace_node(func_call_exp, exp);
    GNode *new_exp = func_call_func(exp);
    return clear_zero(new_exp);
}