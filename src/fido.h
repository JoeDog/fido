#ifndef __FIDO_H
#define __FIDO_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <hash.h>
#include <conf.h>

#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct FIDO_T *FIDO;

FIDO     new_fido(CONF C, const char *key);
FIDO     fido_destroy(FIDO this);
BOOLEAN  start(FIDO this);

#endif/*__FIDO_H*/
