#ifndef __THROTTLE_H
#define __THROTTLE_H

#include <joedog/boolean.h>

typedef struct THROTTLE_T *THROTTLE;

extern size_t  THROTTLESIZE;

THROTTLE   new_throttle(char *file, int seconds);
THROTTLE   throttle_destroy(THROTTLE this);
BOOLEAN    throttled(THROTTLE this);
int        throttle_seconds(THROTTLE this);
char *     throttle_filename(THROTTLE this);
char *     throttle_to_string(THROTTLE this);

#endif/*__THROTTLE_H*/ 
