#ifndef _AST_MOD_H_
#define _AST_MOD_H_
#include <glib.h>

#undef copy_node
#define copy_node(node) (g_node_copy_deep(node, copy_meta_data, NULL))
// -------------------------------- ast --------------------------------
struct MetaData *copy_meta_data(struct MetaData *src);

struct MetaData *new_meta_data(enum NodeType node_type);

struct MetaData *new_meta_data_symbol(GString *name);

void destroy_meta_data(struct MetaData *meta_data);

struct MetaData *new_meta_data_func(enum BuiltinFunction func_type);

// -------------------------------- func built in --------------------------------
GNode *handle_func_builtin(GNode *ast);

// -------------------------------- help --------------------------------

void destroy_data(GNode *ast);

gboolean is_declare(GNode *exp);

void replace_node(GNode *src, GNode *dest);

void replace_node_unlink(GNode *src, GNode *dest);

// -------------------------------- glib init --------------------------------
gboolean *new_gboolean(gboolean init);

// -------------------------------- func call --------------------------------
typedef GNode *(*FuncCallFunc)(GNode *exp);

GNode *func_call(GNode *func_call_exp, FuncCallFunc func_call_func);

// -------------------------------- clear zero --------------------------------
GNode *clear_zero(GNode *exp);
#endif