#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct LOG_T *LOG;

LOG     new_logger(char *file);   /* choices, syslog, stdout, stderr, /path/to/file */
LOG     logger_destroy(LOG this);
BOOLEAN logger(LOG this, const char *fmt, ...);

#endif/*__LOG_H*/

