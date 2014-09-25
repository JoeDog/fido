#ifndef _ARRAY_H
#define _ARRAY_H

typedef struct ARRAY_T *ARRAY;

ARRAY  new_array();
ARRAY  array_destroy(ARRAY this);
int    array_length(ARRAY this);
void   array_push(ARRAY this, void *data);
void  *array_get(ARRAY this, int index);

#endif
