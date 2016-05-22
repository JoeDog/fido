#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <throttle.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <joedog/boolean.h>
#include <joedog/defs.h>

struct THROTTLE_T
{
  char * file;
  char * time;
  int    seconds;
};

size_t THROTTLESIZE = sizeof(struct THROTTLE_T);

private char *  __filename(char *s); 
private BOOLEAN __file_exists(char *file);
private BOOLEAN __touch(char *file);

THROTTLE
new_throttle(char *file, int seconds)
{
  THROTTLE this;

  this = xcalloc(THROTTLESIZE, 1);
  this->file    = __filename(file);
  this->seconds = seconds;
  return this;
}

THROTTLE
throttle_destroy(THROTTLE this)
{
  xfree(this->file);
  xfree(this->time);
  xfree(this);
  this = NULL;
  return this;
}

BOOLEAN 
throttled(THROTTLE this) 
{
  int     seconds = 0;
  struct  stat *buf;
  BOOLEAN result = FALSE;

  if (this->seconds == -1) {
    //unthrottled
    return FALSE;
  }

  buf = (struct stat *)xmalloc(sizeof (struct stat));

  if (! __file_exists(this->file)) {  
    __touch(this->file);
    return FALSE;
  } else {
    if ((stat(this->file, buf)) != 0) {
      xfree(buf);
      return FALSE;
    }
    seconds = (int)difftime(time(NULL),buf->st_mtime);
  }
  
  result = (seconds < this->seconds) ? TRUE : FALSE;
  if (result == FALSE) {
    __touch(this->file); 
  }

  return result;
}

int 
throttle_seconds(THROTTLE this)
{
  return this->seconds;
}

char *
throttle_to_string(THROTTLE this) 
{
  long hour, min, sec, t;

  if (this->time != NULL && strlen(this->time) > 0) {
    xfree(this->time);
  }

  hour = (this->seconds/3600);
  t    = (this->seconds%3600);
  min  = (t/60);
  sec  = t%60;

  this->time  = (char*)malloc(32); 
  memset(this->time, '\0', 32); 
  snprintf(this->time, 32, "%ld:%ld:%ld", hour, min, sec);
  return this->time;
}

char *
throttle_filename(THROTTLE this)
{
  return this->file;
}

private char *
__filename(char *src)
{
  int   i = 0;
  int   j = 0;
  int   k = 0;
  char *dest;
  char  keys[] = "/.";
  BOOLEAN found = FALSE;

  dest = (char *) malloc(sizeof(char) * strlen(src) + 13);
  memset(dest, 0x00, sizeof(char) * strlen(src) + 13);
  strcpy(dest, "/tmp/.");
  k = strlen(dest);
  for (i = 0; i < (int)strlen(src); i++) {
    found = FALSE;
    for (j = 0; j < (int)strlen(keys); j++) {
      if (src[i] == keys[j])
        found = TRUE;
    }
    if (FALSE == found) {
      dest[k++] = src[i];	    
    }
  }
  strcat(dest, ".cache");
  return dest;
}

private BOOLEAN
__file_exists(char *file)
{
  int  fd;

  /* open the file read only  */
  if((fd = open(file, O_RDONLY)) < 0){
    /* the file does NOT exist  */
    close(fd);
    return FALSE;
  } else {
    /* party on Garth... */
    close(fd);
    return TRUE;
  }
  return FALSE;
}

private BOOLEAN 
__touch(char *file) 
{
  FILE *f;

  if (__file_exists(file)) {
    unlink(file);
  }

  if ((f = fopen(file, "w")) == NULL) {
    return FALSE;
  }
  if (f) fclose(f);

  return TRUE;
}

#if 0
int main() {
  int i;
  THROTTLE t = new_throttle("/usr/local/bin/run.sh", 30);
  for (i = 0; i < 62; i ++) {
    if (! throttled(t)) {
      puts("ls -altr");
    }
    sleep(1);
  }
  exit(0);
}
#endif
