#ifndef RSET_H
#define RSET_H

#include <stdarg.h>
#include <joedog/joedog.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct RSET_T *RSET;

RSET    new_rset();
RSET    rset_destroy(RSET this);
void    rset_set_result(RSET this, BOOLEAN result);
BOOLEAN rset_get_result(RSET this);
void    rset_add(RSET this, const char *fmt, ...);
char *  rset_get_string(RSET this);

#endif/*RSET_H*/
