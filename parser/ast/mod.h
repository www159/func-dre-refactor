#ifndef _AST_MOD_H_
#define _AST_MOD_H_
#include <glib.h>

#undef copy_node
#define copy_node(node) (g_node_copy_deep(node, copy_meta_data, NULL))

typedef GNode *(*FuncCallFunc)(GNode *exp);
// -------------------------------- ast --------------------------------
struct MetaData *copy_meta_data(struct MetaData *src);

struct MetaData *new_meta_data(enum NodeType node_type);

struct MetaData *new_meta_data_symbol(GString *name);

void destroy_meta_data(struct MetaData *meta_data);

struct MetaData *new_meta_data_func(enum BuiltinFunction func_type);

// -------------------------------- symbol table --------------------------------
GNode *replace_symbol_exp(GString *name, GNode *exp);

// -------------------------------- func built in --------------------------------
GNode *handle_func_builtin(GNode *ast);

// -------------------------------- help --------------------------------

void destroy_data(GNode *ast);

gboolean is_declare(GNode *exp);

void replace_node(GNode *src, GNode *dest);

void replace_node_unlink(GNode *src, GNode *dest);

char *const node_type_str(enum NodeType node_type);

void get_l_r_exp(GNode *ast, GNode **l_exp, GNode **r_exp);

// -------------------------------- glib init --------------------------------
gboolean *new_gboolean(gboolean init);

// -------------------------------- func call --------------------------------


GNode *func_call(GNode *func_call_exp, FuncCallFunc func_call_func);

// -------------------------------- func built in --------------------------------
void handle_dre(GNode *exp);

GNode *handle_dre_immut(GNode *exp);

// -------------------------------- format --------------------------------
GNode *clear_all(GNode *exp);

GNode *clear_zero(GNode *exp);

GNode *clear_one(GNode *exp);
#endif