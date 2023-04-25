#include <stdarg.h>
#include <stdio.h>
#include "ast.h"

void yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    g_printerr("%d, error: ", yylineno);
    vfprintf(stderr, s, ap);
    g_printerr("\n");
}