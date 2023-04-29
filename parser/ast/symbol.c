#include "ast.h"
#include "mod.h"

GHashTable *symbol_table;

struct MetaData *new_meta_data_symbol(GString *name);

GNode *look_up_symbol_table(GString *name)
{
    if (symbol_table == NULL)
    {
        symbol_table = g_hash_table_new(g_string_hash, g_string_equal);
    }

    if (g_hash_table_contains(symbol_table, name))
    {
        GNode *symbol = g_hash_table_lookup(symbol_table, name);
        return copy_node(symbol);
    }
    struct MetaData *meta_data = new_meta_data_symbol(g_string_new(name->str));
    GNode *symbol = g_node_new(meta_data);

    // FIXME key and value ref twice 
    g_hash_table_insert(symbol_table, meta_data->name, symbol);

    return copy_node(symbol);
}

GNode *replace_symbol_exp(GString *name, GNode *exp)
{
    GNode *symbol = g_hash_table_lookup(symbol_table, name);
    GNode *symbol_exp_origin = g_node_first_child(symbol);
    struct MetaData *meta_data = symbol->data;
    meta_data->declared = TRUE;

    if (symbol_exp_origin == NULL)
    {
        g_node_append(symbol, exp);
        return copy_node(symbol);
    }
    else
    {
        replace_node(symbol_exp_origin, exp);
        return copy_node(symbol);
    }
}