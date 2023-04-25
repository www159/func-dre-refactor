#include "ast.h"
#include "mod.h"

GHashTable *symbol_table;

struct MetaData *new_meta_data_symbol(GString *name);

GNode *look_up_symbol_table(char *str)
{
    if (symbol_table == NULL)
    {
        symbol_table = g_hash_table_new(g_string_hash, g_string_equal);
    }

    GString *name = g_string_new(str);
    if (g_hash_table_contains(symbol_table, name))
    {
        GNode *symbol = g_hash_table_lookup(symbol_table, name);
        g_string_free(name, TRUE);
        return symbol;
    }
    struct MetaData *meta_data = new_meta_data_symbol(name);
    GNode *symbol = g_node_new(meta_data);

    g_hash_table_insert(symbol_table, meta_data->name, symbol);

    return symbol;
}
