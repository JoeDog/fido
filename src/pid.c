#include <pid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct PID_T {
  int    pid;
  int    count;
  char * file; 
  int    fd;
};

size_t  PIDSIZE = sizeof(struct PID_T);

private BOOLEAN __createPidFile (PID this);
private BOOLEAN __pidFileExists (PID this);

PID
new_pid(char * filename)
{
  PID this;

  this = calloc(PIDSIZE, 1);
  this->count  = 0;
  this->pid    = -1;
  this->file   = strdup(filename);
  if (__createPidFile(this) == FALSE) {
    this = pid_destroy(this);
  }
  return this; 
}

PID
pid_destroy(PID this)
{
  if (this != NULL) {
    free(this->file);
    free(this);
  } 
  return this;
}

char *
pid_file(PID this)
{
  return this->file;
}

BOOLEAN 
set_pid(PID this, int pid)
{
  char buf[16];
  int  ret = 0;
  if (pid > 0) {
    this->pid = pid;
    if ((this->fd = open(this->file, O_WRONLY| O_APPEND, 0644 )) < 0) {
      return FALSE;
    } else {
      snprintf(buf, sizeof(buf), "%d\n", pid);
      ret = write(this->fd, buf, strlen(buf)); 
      if (ret < 0) {
        fprintf(stderr, "ERROR: unable to write PID file: %s", this->file);
      }
      close(this->fd);
      return TRUE;
    } 
  }
  return FALSE; // WTF?
}

private BOOLEAN
__pidFileExists (PID this) {
  if ((this->fd = open(this->file, O_RDONLY)) < 0) {
    close(this->fd);
    return FALSE;
  } else {
    close(this->fd);
    return TRUE;
  }
  return FALSE;
}

private BOOLEAN
__createPidFile (PID this) {
  
  if (__pidFileExists(this)) {
    unlink(this->file);
  }
   
  if ((this->fd = open(this->file, O_CREAT | O_WRONLY, 0644)) < 0) {
    return FALSE;
  }

  close(this->fd);

  return TRUE;
}

#if 0
int main() {
  int i;
  PID P = new_pid("./haha.pid");
  for (i = 0; i < 16; i++) {
    switch(fork()) {
      case -1:
        exit(1);
      case 0:
        add_pid(P, getpid());
        exit(0);
    }
  }
  exit(0);
}
#endif
