%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "ast.h"
%}

%union {
    struct GNode *ast;
    double num;
    struct GNode *symbol;
    int func_type;
}

%token <num> NUMBER
%token <symbol> NAME
%token <func_type> FUNC
%token EOL

%token FN LN EXP X LET DRE_ID

%right '='
%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc UMINUS

%type <ast> dre_name exp stmt list

%start prog_list
%%

dre_name: NAME { $$ = $1; }
| dre_name DRE_ID { $$ = new_ast(NODE_DREVIDE, $1, NULL); }

exp: exp '+' exp { $$ = new_ast(NODE_ADD, $1, $3); }
| exp '-' exp { $$ = new_ast(NODE_SUB, $1, $3); }
| exp '*' exp { $$ = new_ast(NODE_MUL, $1, $3); }
| exp '/' exp { $$ = new_ast(NODE_DIV, $1, $3); }
| exp '^' exp { $$ = new_ast(NODE_POWER, $1, $3); }
| '(' exp ')' { $$ = $2; }
| '-' exp %prec UMINUS { $$ = new_ast(NODE_MINUS, $2, NULL); }
| LN '(' exp ')' { $$ = new_ast(NODE_LN, $3, NULL); }
| NUMBER { $$ = new_num($1); }
| dre_name { $$ = $1; }
| EXP { $$ = new_ast(NODE_EXP, NULL, NULL); }
| X { $$ = new_ast(NODE_PARAM, NULL, NULL); }
| dre_name '(' exp ')' { $$ = new_ast(NODE_FUNC_EMBED, $1, $3); }
| FUNC '(' exp ')' { $$ = new_func($1, $3); }
;

stmt: exp ';' { }
;

list: /* empty */ { $$ = NULL; }
| stmt list {
    if($2 == NULL)
        $$ = $1;
    else
        $$ = new_ast(NODE_LIST, $1, $2);
}
;

prog_list: /* empty */ { }
| prog_list list EOL {
    emit($2);
    printf(">>>");
}
| prog_list LET NAME '=' exp ';' EOL {
    printf(">>>"); 
    do_def($3, $5); 
}
| prog_list error EOL {
    yyerrok;
    printf(">>>");
}
%%