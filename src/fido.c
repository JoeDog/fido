#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array.h>
#include <fido.h>
#include <throttle.h>
#include <logger.h>
#include <rule.h>
#include <base16.h>
#include <util.h>
#include <pthread.h>
#include <version.h>
#include <dirent.h>
#ifdef HAVE_REGEX_H
# include <regex.h>
#else
# include <ereg.h>
#endif/*HAVE_REGEX_H*/
#include <joedog/defs.h>


#define MINLINES 10      
#define READSIZE BUFSIZ 
#define LINESIZE 128

struct FIDO_T
{
  char      *wfile;
  char      *rfile;
  char      *action;
  char      *exclude;
  long      cache;
  ARRAY     rules;
  LOG       logger;
  CONF      C;
  THROTTLE  throttle;
};

struct pair {
  int  num;
  char *label;
  char *match;
};

typedef enum action_t {
  put,
  get 
} action;

private BOOLEAN  __start_parser(FIDO this);
private BOOLEAN  __start_watcher(FIDO this);
private BOOLEAN  __start_agecheck(FIDO this);
private BOOLEAN  __is_readable (FIDO this);
private long     __ticks (FIDO this, long offset);
private long     __get_offset (FIDO this);
private BOOLEAN  __is_directory(char *path); 
private BOOLEAN  __eregi(char *pattern, char *string);
private BOOLEAN  __ereg (char *pattern, char *string);
private BOOLEAN  __regex(char *pattern, char *string, int cflags);
private BOOLEAN  __read_rules(FIDO this);
private long     __seconds_since_90(char *file);
private int      __run_command(FIDO this, RULE r);
private int      __parse_time(char *p);
private BOOLEAN  __exceeds(FIDO this, char *path, int age);
private void     __persist(FIDO this, action a);


FIDO
new_fido(CONF C, const char *file)
{
  FIDO this;

  this = calloc(sizeof(*this),1);
  this->C        = C;
  this->wfile    = xstrdup(file);
  this->rfile    = xstrdup(conf_get_rules(this->C,  this->wfile));
  this->action   = xstrdup(conf_get_action(this->C, this->wfile));
  this->exclude  = xstrdup(conf_get_exclude(this->C, this->wfile));
  this->throttle = new_throttle(this->wfile, __parse_time(conf_get_throttle(this->C, this->wfile)));
  this->logger   = new_logger(conf_get_log(this->C, this->wfile));
  this->rules    = new_array();
  if ((__read_rules(this)) == FALSE) {
    logger(this->logger, "%s [error] unable to read rules file: %s", program_name, this->rfile);
    if (is_verbose(C)) {
      NOTIFY(ERROR, "unable to read rules file: %s [exiting]", this->rfile);
    }
    exit(1);
  }
  return this;
}

FIDO
fido_destroy(FIDO this) {
  xfree(this->wfile);
  xfree(this->rfile);
  xfree(this->action);
  this->rules = array_destroy(this->rules);
  xfree(this);
  return NULL;
}

BOOLEAN 
start(FIDO this)
{
  VERBOSE(is_verbose(this->C), "Starting fido[id=%u] ...", pthread_self());
  if (strmatch(this->rfile, "modified")) {
    return __start_watcher(this);
  } else if (! strncmp(this->rfile, "exceeds", 7)) {
    return __start_agecheck(this);
  } else {
    return __start_parser(this);
  }
  return FALSE;
}

private BOOLEAN
__start_watcher(FIDO this) 
{
  __persist(this, get);

  while (TRUE) {
    long tmp = __seconds_since_90(this->wfile);
    if (this->cache == 0) {
      this->cache = tmp;
    }
    if (this->cache != tmp) {
      char rule[256];
      
      if (is_daemon(this->C) == TRUE) {
        logger(this->logger,   "%s was modified since last check", this->wfile);
        VERBOSE(is_verbose(this->C), "%s was modified since last check", this->wfile);
      } else {
        VERBOSE(is_verbose(this->C), "%s was modified", this->wfile);
      }

      snprintf(rule, 256, "MODIFIED: %s", this->wfile);
      __run_command(this, new_rule(rule));
      this->cache = tmp;
      __persist(this, put);
    }
    sleep(1);
  }  
}

private BOOLEAN 
__start_agecheck(FIDO this) 
{
  int sec = __parse_time(this->rfile);

  while (TRUE) {
    char rule[256];
    if (! __is_directory(this->wfile)) {
      if (__exceeds(this, this->wfile, sec) == TRUE) {
        snprintf(rule, 256, "EXCEEDS: %s", this->wfile);
        __run_command(this, new_rule(rule));
      }
    } else {
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir (this->wfile)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
          if (! __ereg(this->exclude, ent->d_name)) {
            /** 
             * XXX:  we need to test for an ending slash on
             * this->wfile in order to avoid a double-slash
             */
            char tmp[2048];
            snprintf(tmp, 2048, "%s/%s", this->wfile, ent->d_name);
            if (__exceeds(this, tmp, sec) == TRUE) {
              if (is_daemon(this->C) == TRUE) {
                logger(this->logger,   "%s (%s) exceeds %d seconds", this->wfile, ent->d_name, sec);
                VERBOSE(is_verbose(this->C), "%s (%s) exceeds %d seconds", this->wfile, ent->d_name, sec);
              } else {
                VERBOSE(is_verbose(this->C), "%s (%s) exceeds %d seconds", this->wfile, ent->d_name, sec);
              }
              snprintf(rule, 256, "EXCEEDS: %s", this->wfile);
              __run_command(this, new_rule(rule));
            }
          }
        }
        closedir (dir);
      } else {
        return FALSE;
      }
    }
    sleep(1);
  }
}


private BOOLEAN
__start_parser(FIDO this)
{
  int  rc = 0;
  long offset;
  
  while (TRUE) { // as of fido 1.0.8 we'll run whether we have a file or not
    if (__is_readable (this)) {
      offset = __get_offset(this);
      while (rc == 0 && offset >= 0) {
        offset = __ticks (this, offset);
        if (offset >= 0) {
          sleep (1);
        } else {
          logger(this->logger, "%s [error] set return to -7; offset value: %ld", program_name, offset);
          rc = -7;
        }
      }
    } else {
      logger(this->logger, "%s [error] unable to open \"%s\" for reading", program_name, this->wfile);
      if (is_verbose(this->C)) {
        NOTIFY(ERROR, "unable to open \"%s\" for reading", this->wfile);
      }
      rc = -4;
    }
    sleep(10); // we only sleep if we can't find / read the file
  }
  return rc == 0 ? FALSE : TRUE;
}

private BOOLEAN
__is_readable (FIDO this)
{
  FILE *fp;
  fp = fopen (this->wfile, "r");
  if (fp != NULL) {
    fclose (fp);
  }
  return (fp != NULL);
}

private long
__get_offset (FIDO this)
{
  FILE *fp;
  long offset;

  fp = fopen(this->wfile, "r");
  if (fp != NULL) {
    if (fseek (fp, 0, SEEK_END) == 0) {
      offset = ftell (fp);
      if (offset < 0) {
        offset = 0;
      }
    } else {
      offset = -1;
    }
    fclose (fp);
  } else {
    offset = -1;
  }
  return offset;
}

#define BLOCK 32 
char *fgetline(FILE *fp) 
{ 
  char *s; 
  char *tmp; 
  size_t count; 
  int ch; 
  
  for (count = 0, s = NULL; (ch = fgetc(fp)) != EOF && ch != '\n';count++) { 
    if (count%BLOCK == 0) { 
      tmp = realloc(s,count+BLOCK+1); 
      if (tmp == NULL) { 
        xfree(s); 
        return NULL; 
      } 
      s = tmp; 
    } 
    s[count] = ch; 
  } 
  if(s) s[count] = '\0'; 
  return s; 
}

//--ticks
private long
__ticks (FIDO this, long offset)
{
  FILE *fp;
  int   ret;
  char *line;
  long  pos;


  fp = fopen (this->wfile, "r");
  if (fp != NULL) {
    pos = ftell(fp);
    fseek(fp, pos, SEEK_END);
    pos = ftell(fp);
    if (pos < offset) {
      /* file was truncated */
      offset = pos;
      VERBOSE(is_verbose(this->C), "%s was truncated; re-reading from the top", this->wfile);
    }
    if (fseek (fp, offset, SEEK_SET) == 0) {
      line = fgetline(fp);
      if (line != NULL) {
        int x;
        for (x = 0; x < array_length(this->rules); x++) {
          RULE r = (RULE)array_get(this->rules, x);
          if (__eregi(rule_get_pattern(r), line)) {
            if (is_daemon(this->C) == TRUE) {
              logger(this->logger,   "matched %s in %s", rule_get_pattern(r), this->wfile);
              VERBOSE(is_verbose(this->C), "matched %s in %s", rule_get_pattern(r), this->wfile);
            } else {
              VERBOSE(is_verbose(this->C), "matched: %s in %s", line, this->wfile);
            }
            ret = __run_command(this, r);
          }
        }
        xfree(line);
      } else {
        offset = -1;
      }
      offset = ftell (fp);
    } else {
      logger(this->logger, "%s:%d: Can't seek to offset %ld.", __FILE__, __LINE__, offset);
      offset = -1;
    }
    fclose (fp);
  } else {
    logger(this->logger, "%s:%d: Can't re-open the file. Did it move?",__FILE__, __LINE__);
    offset = -1;
  }
  return offset;
}

private BOOLEAN
__eregi(char *pattern, char *string) 
{
  return __regex(pattern, string, REG_ICASE|REG_EXTENDED);
}

private BOOLEAN
__ereg(char *pattern, char *string) 
{
  return __regex(pattern, string, REG_EXTENDED);
}


static BOOLEAN
__regex (char *pattern, char *string, int cflags)
{
  int     res;
  regex_t reg;
  regmatch_t pm[10];
  char ebuf[128];
  const size_t nmatch = 10;

  if (!pattern) return FALSE;
  if (!string)  return FALSE;
  
  res = regcomp(&reg, pattern, cflags);
  if (res != 0) {
    regerror(res, &reg, ebuf, sizeof(ebuf));
    return FALSE;
  }
  res = regexec(&reg, string, nmatch, pm, 0);
  regfree(&reg);
  if (res == REG_NOMATCH){
    return FALSE;
  } else {
    return TRUE;
  }
  return FALSE;
}

BOOLEAN
__read_rules(FIDO this)
{
  int   num;
  char *line;
  FILE *fp;

  if ((fp = fopen(this->rfile, "r")) == NULL) {
     // the rules file is actually a rule
     RULE  R = new_rule(this->rfile);
     if (R) {
       array_push(this->rules, (RULE)R);
       return TRUE;
     } else {
       return FALSE;
     } 
  } else {
    while ((line = chomp_line(fp, &line, &num)) != NULL) {
      RULE  R;
      char *lineptr;
      char *lineval;
      lineptr = lineval = xstrdup(line);
      while (*lineptr && !ISCOMMENT(*lineptr))
        lineptr++;
      *lineptr++=0;
      R = new_rule(lineval);
      if (R) {
        array_push(this->rules, (RULE)R);
      }
      xfree(lineval);
    }
    return TRUE;
  }
  return FALSE;
}

private long
__seconds_since_90(char *file)
{
  struct tm   epoch;
  struct stat st;
  time_t basetime;
  long   nsecs   = 0;
  epoch.tm_sec   = 0;
  epoch.tm_min   = 0;
  epoch.tm_hour  = 0;
  epoch.tm_mday  = 1;
  epoch.tm_mon   = 0;
  epoch.tm_year  = 90;
  epoch.tm_isdst = -1;
  basetime = mktime(&epoch);

  if (!stat(file, &st)) {
    nsecs = difftime(st.st_mtime, basetime);
  }
  return nsecs;
}


private int
__run_command(FIDO this, RULE r) 
{
  int   ret;
  char *cmd;
  char  out[BUFSIZ];
  FILE *rp;

  if (throttled(this->throttle)) {
    return 0;
  }

  if (rule_get_property(r) != NULL) {
    cmd = xmalloc(strlen(this->action)+strlen(rule_get_property(r))+5);
    memset(cmd, '\0', strlen(this->action)+strlen(rule_get_property(r))+5);
    snprintf(
      cmd, strlen(this->action)+strlen(rule_get_property(r))+5, "%s %s", this->action, rule_get_property(r)
    );
  } else {
    cmd = xstrdup(this->action);
  }
  rp = popen(cmd, "r");
  while (fgets(out, sizeof out, rp) != NULL) {
    chomp(out);
    if (conf_get_capture(this->C, this->wfile)) {
      logger(this->logger, "%s", out);
      VERBOSE(is_verbose(this->C), "%s", out);
    }
  }
  ret = pclose(rp);
  if (ret != 0) {
    logger(this->logger, "[error] %s exited with non-zero result: %d", this->action, ret);
    if (is_verbose(this->C)) {
      NOTIFY(ERROR, "%s exited with non-zero result: %d", this->action, ret);
    }
  }
  xfree(cmd);
  return ret;
}

private void
__persist(FIDO this, action a)
{
  int fd;
  int  n;
  char c;
  char path[512];
  char buf[128];
  char *filename = base16_encode(this->wfile);

  filename = trim(filename);

  memset(path, 512, '\0');
  snprintf(path, 512, "/tmp/.%s", filename);

  if (a == get) {
    if ((fd = open(path, O_RDONLY)) < 0) {
      this->cache = 0;
      return;
    } else {
      int cnt = 0;
      char *end;
      while ((n = read(fd, &c, 1)) == 1) {
        if (!ISSPACE(c)) 
          buf[cnt] = c;
        cnt++;
      }
      this->cache = strtol(buf, &end, 10); 
      close(fd);
    }
  } else {
    if ((fd = open(path, O_WRONLY|O_CREAT, 0644)) < 0) {
      return;
    } else {
      snprintf(buf, sizeof(buf), "%ld\n", this->cache);
      write(fd, buf, strlen(buf));
      close(fd);
    } 
  }
  return;
}

void __demodify(char *s, const char *modifier)
{
  while ((s = strstr(s,modifier))) {
    memmove(s,s+strlen(modifier),1+strlen(s+strlen(modifier)));
  }
  return;
}

private int
__parse_time(char *p)
{
  size_t x   = 0;
  int  time  = 0;
  int  secs  = 0;
  char *tmp  = NULL;
  char mod[] = "exceeds ";

  if (p == NULL || strlen(p) < 1) return -1;

  p = trim(p);
  __demodify(p, mod);
  p = trim(p);
 
  while (isdigit(p[x]))
    x++;

  if (x==0)  return -1;

  tmp  = substring(p, 0, x);
  time = atoi(tmp);
  free(tmp);

  for (; x < strlen(p); x ++) {
    switch(tolower(p[x])){
      case 's':
        secs = time;
        time = 1;
        return secs;
      case 'm':
        secs = time * 60;
        time = 1;
        return secs;
      case 'h':
        secs = time * 3600;
        time = 1;
        return secs;
      case 'd':
        secs = time * 3600 * 24;
        time = 1;
        return secs;
      default:
        break;
    }
  }

  return -1;
}

private BOOLEAN
__is_directory(char *path) 
{
  struct stat s;
  if (stat(path,&s) == 0) {
    if( s.st_mode & S_IFDIR ) {
      return TRUE;
    } else {
      return FALSE; 
    }
  }
  return FALSE;
}

private BOOLEAN
__exceeds(FIDO this, char *path, int age) {
  int secs = 0;
  struct stat s;
  char   buf[20]; 
  struct tm * timeinfo;

  if (stat(path,&s) != 0) {
    /**
     * Another process likely has a lock on the file;
     * We'll log a warning but continue without rasing
     * an ALERT.
     */
    logger(this->logger, "WARNING: unable to read %s - in use by another process\n", path);
    return FALSE;
  }

  timeinfo = localtime (&(s.st_mtime)); 
  strftime(buf, 20, "%b %d %H:%M", timeinfo); 

  secs = (int)difftime(time(NULL), s.st_mtime);
  if (secs > age) {
    logger(this->logger, "ALERT: %s exceeds %d seconds (%s)\n", path, secs, age, buf);
    return TRUE;
  }
  return FALSE;
}

