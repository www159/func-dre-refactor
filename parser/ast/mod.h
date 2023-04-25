#ifndef _AST_MOD_H_
#define _AST_MOD_H_
#include <glib.h>
// -------------------------------- ast --------------------------------
struct MetaData *copy_meta_data(struct MetaData *src);

struct MetaData *new_meta_data_symbol(GString *name);

// -------------------------------- func built in --------------------------------
void handle_func_builtin(GNode *ast);

// -------------------------------- help --------------------------------
int try_expand(GNode *exp);

gboolean is_declare(GNode *exp);

// -------------------------------- glib init --------------------------------
gboolean *new_gboolean(gboolean init);
#endif