#ifndef PID_H__
#define PID_H__

#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct PID_T *PID;

PID      new_pid(char *filename);
PID      pid_destroy(PID this);
char *   pid_file(PID this);
BOOLEAN  set_pid(PID this, int pid);

#endif/*PID_H__*/

