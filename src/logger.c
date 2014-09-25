#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/file.h>
#include <logger.h>
#include <date.h>
#include <version.h>
#include <unistd.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>
#include <joedog/joedog.h>

#define BUFSIZE 1024

#define xfree(x) free(x)
#define xstrdup(x) strdup(x)

typedef enum {
  LSYSLOG   = 0,
  LFILE     = 1,
  LSTDOUT   = 2,
  LSTDERR   = 3
} LTYPE;
  
struct LOG_T
{
  char    *file;
  int     fd;
  LTYPE   type;
};

private BOOLEAN  __strmatch(char *option, char *param);
private BOOLEAN  __message(LOG this, const char *fmt, va_list ap);

LOG
new_logger(char *file)
{
  LOG this;
  
  this = calloc(sizeof(*this),1);
  this->file = xstrdup(file);
  if (__strmatch(this->file, "syslog")) {
    this->type = LSYSLOG; 
    openlog(program_name, LOG_PID, LOG_DAEMON);
  } else if (__strmatch(this->file, "stderr")) {
    this->type = LSTDERR;
  } else if (__strmatch(this->file, "stderr")) {
    this->type = LSTDOUT;
  } else {
    this->type = LFILE;
    this->fd = open(this->file, O_CREAT | O_RDWR | O_APPEND, 0644);
    if (this->fd == -1) {
      fprintf(stderr, "%s [error] Unable to open log file: %s\n", program_name, this->file);
      return logger_destroy(this);
    }
    logger(this, "initialized the process");
  }
  return this;
}

LOG
logger_destroy(LOG this) 
{
  if (this->type == LSYSLOG) {
    closelog();
  }
  xfree(this->file);
  xfree(this);
  return this;
}

BOOLEAN
logger(LOG this, const char *fmt, ...)
{
  BOOLEAN b;
  va_list ap;
  va_start(ap, fmt);

  b = __message(this, fmt, ap);
  va_end(ap);

  return b;
}

private BOOLEAN
__message(LOG this, const char *fmt, va_list ap)
{
  char   buf[BUFSIZE];
  char   msg[BUFSIZE*2];
  char   hostbuf[BUFSIZE];
  DATE d = new_date(date_format(SYSLOG_FORMAT));

  vsprintf(buf, fmt, ap);
  sprintf(buf + strlen(buf), "\n");

  switch(this->type) {
    case LSYSLOG:
      syslog(LOG_ERR, "%s", buf);
      break;
    case LFILE:
      if (gethostname(hostbuf,sizeof(hostbuf)) < 0) {
        snprintf(hostbuf, BUFSIZE, "localhost");
      }
      flock(this->fd, LOCK_EX);
      memset(msg, '\0', sizeof(msg));
      snprintf(msg, sizeof(msg), "%s %s - %s [%d] %s", date_get(d), hostbuf, program_name, getpid(), buf);
      write(this->fd, msg, strlen(msg));
      flock(this->fd, LOCK_UN);
      break;
    case LSTDERR:
      fprintf(stderr, "%s", buf);
      break;
    case LSTDOUT:
    default:
      printf("%s", buf);
      break;
  }

  return TRUE;
}

private BOOLEAN
__strmatch(char *option, char *param) {
  if(!strncasecmp(option,param,strlen(param))&&strlen(option)==strlen(param))
    return TRUE;
  else
    return FALSE;
}

#if 0
int main() {
  LOG L = new_logger("syslog");
  LOG M = new_logger("zzzzz.txt");
  logger(L, "this is a testing...");
  logger(M, "this is a testing...");
  exit(0);
}
#endif
