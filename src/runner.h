#ifndef __RUNNER_H
#define __RUNNER_H

#include <joedog/joedog.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct RUNNER_T *RUNNER;

RUNNER   new_runner();
void     runner_destroy(RUNNER this);
BOOLEAN  runas(RUNNER this);

#endif/*__RUNNER_H*/
