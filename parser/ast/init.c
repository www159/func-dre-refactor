#include "mod.h"

gboolean *new_gboolean(gboolean init)
{
    gboolean *bool_val = (gboolean *)g_malloc(sizeof(gboolean));
    *bool_val = init;
    return bool_val;
}