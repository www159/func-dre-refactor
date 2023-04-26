#ifndef _AST_MOD_H_
#define _AST_MOD_H_
#include <glib.h>
// -------------------------------- ast --------------------------------
struct MetaData *copy_meta_data(struct MetaData *src);

struct MetaData *new_meta_data_symbol(GString *name);

void destroy_meta_data(struct MetaData *meta_data);


struct MetaData *new_meta_data_func(enum BuiltinFunction func_type);

// -------------------------------- func built in --------------------------------
void handle_func_builtin(GNode *ast);

// -------------------------------- help --------------------------------

void destroy_data(GNode *ast);

gboolean is_declare(GNode *exp);

// -------------------------------- glib init --------------------------------
gboolean *new_gboolean(gboolean init);
#endif