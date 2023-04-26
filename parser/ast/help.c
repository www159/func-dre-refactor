#include "ast.h"
#include "mod.h"

// static void expand(GNode *exp, gboolean *declared);
// int try_expand(GNode *exp)
// {
//     gboolean *declared = new_gboolean(TRUE);
//     g_node_children_foreach(exp, G_TRAVERSE_ALL, expand, declared);
//     if (*declared == FALSE)
//     {
//         g_free(declared);
//         return 0;
//     }
//     g_free(declared);
//     return 1;
// }
// expand exp with variable
// .e.g
// ```
// let a = x^5;
// let b = a+4*x;
// # b = x^5+4*x;
// ```
//      a
//      |
//     exp(x^5)
//
//      b
//   ___|___
//  |       |
//  a     exp(4*x)

//      b                        a
//   ___|____                    |
//  |        |                  exp(x^5)
// exp(x^5) exp(4*x)
// deep copy moved variable
// static void expand(GNode *exp, gboolean *declared)
// {
//     if (!*declared)
//     {
//         return;
//     }
//     struct MetaData *const meta_data = exp->data;

//     // deep copy and extend
//     if (meta_data->node_type == NODE_NAME)
//     {
//         if (!meta_data->declared)
//         {
//             *declared = FALSE;
//             g_debug("meet undeclared symbol: '%s' when expanding", meta_data->name->str);
//             return;
//         }
//         GNode *const symbol_exp = g_node_first_child(exp);
//         GNode *symbol_exp_copy = g_node_copy_deep(symbol_exp, copy_meta_data, NULL);
//         GNode *const parent = exp->parent;
//         if (parent == NULL)
//         {
//             g_debug("panic: should have parent");
//             return;
//         }
//         gint pos = g_node_child_position(parent, exp);
//         g_node_unlink(exp);
//         g_node_insert(parent, pos, symbol_exp_copy);
//     }
// }

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