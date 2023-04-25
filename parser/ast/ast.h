#ifndef _AST_H_
#define _AST_H_

#include <glib.h>
// -------------------------------- yacc --------------------------------

extern int yylineno;
void yyerror(char *s, ...);

// -------------------------------- ast --------------------------------

enum BuiltinFunction
{
    B_PRINT = 1,
    B_DRE,
};

enum NodeType
{
    NODE_NUMBER = 1,
    NODE_NAME,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_POWER,
    NODE_MINUS,

    // outer minis
    NODE_OUTER_MINUS,

    NODE_LN,
    NODE_EXP,
    NODE_X,
    NODE_DREVIDE,
    NODE_FUNC_EMBED,
    NODE_FUNC_BUILT_IN,

    // control flow
    NODE_LIST,
    NODE_ASSIGN,
};

struct MetaData
{
    // basic node
    enum NodeType node_type;

    union
    {
        // symbol
        struct
        {
            GString *name;
            int declared;
        };

        // builtin function
        enum BuiltinFunction func_type;

        // number
        double val;
    };
};

GNode *new_ast(enum NodeType node_type, GNode *l_exp, GNode *r_exp);

void destroy_ast(GNode *ast);

GNode *new_func(enum BuiltinFunction func_type, GNode *exp);

GNode *new_list(GNode *pre, GNode *list);

GNode *new_num(double num);

GNode *new_x();

GNode *new_exp();


// -------------------------------- emit program --------------------------------
// emit program ast to other structure.
// refresh variable values.
// TODO
// 1. let xx = xxxx.
void emit(GNode *ast);

// --------------------------------  simplify program --------------------------------
// delete 'program structure' node
// remain variable node
void simplify(GNode *ast);

// -------------------------------- symbol table --------------------------------

extern GHashTable *symbol_table;

GNode *look_up_symbol_table(char *);

#endif