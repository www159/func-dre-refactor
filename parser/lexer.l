%option noyywrap nodefault yylineno
%{
    #include "parser.h"
    #include "ast.h"
%}

EXP ([Ee][-+]?[0-9]+)

%%
"+" |
"-" |
"*" |
"/" |
"^" |
"=" |
";" |
"(" |
")" {
    return yytext[0];
}

"fn"    { return FN; }
"ln"    { return LN; }
"exp"   { return EXP; }
"x"     { return X; }
"let"   { return LET; }
"'"     { return DRE_ID; }

"print" { yylval.func_type = B_PRINT; return FUNC; }
"dre"   { yylval.func_type = B_DRE; return FUNC; }

[a-zA-Z][a-zA-Z0-9]* { 
    yylval.symbol = look_up_symbol_table(yytext); 
    return NAME; 
}

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]*{EXP}? {
    yylval.num = atof(yytext);
    return NUMBER;
}

"//".* { }
[ \t] { }

\\\n { printf(">>>"); }
\n { return EOL; }

. { yyerror("Unreconginized character %c\n", *yytext); }
%%