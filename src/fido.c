#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <array.h>
#include <fido.h>
#include <throttle.h>
#include <logger.h>
#include <rset.h>
#include <rule.h>
#include <perl.h>
#include <notify.h>
#include <memory.h>
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
#define MAXPATH  4096

#define GREATERTHAN 1000
#define EQUALTO     1001
#define LESSTHAN    1002

#define EXCEEDS     2000
#define INTERVAL    2001

struct FIDO_T
{
  int       serial;
  char      *wfile;
  char      *rfile;
  char      *action;
  char      *clear;
  BOOLEAN   event;
  int       interval;
  char      *exclude;
  BOOLEAN   recurse;
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
private BOOLEAN  __agecheck(FIDO this, char *dir);
private BOOLEAN  __start_countcheck(FIDO this);
private BOOLEAN  __countcheck(FIDO this, char *dir);
private int      __countfiles(char *path, BOOLEAN recursive); 
private BOOLEAN  __is_readable (FIDO this);
private long     __ticks (FIDO this, long offset);
private long     __get_offset (FIDO this);
private BOOLEAN  __is_directory(char *path); 
private RSET     __eregi(char *pattern, char *string);
private RSET     __ereg (char *pattern, char *string);
private RSET     __regex(char *pattern, char *string, int cflags);
private BOOLEAN  __read_rules(FIDO this);
private long     __seconds_since_90(char *file);
private int      __run_command(FIDO this, char *cmd);
private char *   __build_command(FIDO this, RULE rule);
private int      __parse_time(int mod, char *p);
private int      __parse_count(char *p, int *op);
private BOOLEAN  __exceeds(FIDO this, char *path, int age); 
private char *   __evaluate(ARRAY array, char *cmd);
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
  this->clear    = xstrdup(conf_get_clear(this->C,  this->wfile));
  this->event    = FALSE;
  this->exclude  = xstrdup(conf_get_exclude(this->C, this->wfile));
  this->recurse  = conf_get_recurse(this->C, this->wfile);
  this->interval = __parse_time(INTERVAL, conf_get_interval(this->C, this->wfile));
  if (this->interval < 1) {
    this->interval = 5;
  }
  this->throttle = new_throttle(
    this->wfile, __parse_time(EXCEEDS, conf_get_throttle(this->C, this->wfile))
  );
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
  xfree(this->clear);
  this->rules = array_destroy(this->rules);
  xfree(this);
  return NULL;
}

void
fido_reload(FIDO this) 
{
  char *rules;
  logger(this->logger, "reloading config....");
  if (this->rfile != NULL && strlen(this->rfile) > 0) {
    logger(this->logger, "RULES: old value: %s", this->rfile);
    xfree(this->rfile);
  }
  rules = conf_get_rules(this->C, this->wfile);
  this->rfile = xstrdup(rules);
  logger(this->logger, "RULES: new value: %s", this->rfile);
  xfree(rules);

  if (this->action != NULL && strlen(this->action) > 0) {
    logger(this->logger, "ACTION: old value: %s", this->action);
    xfree(this->action);
  }
  this->action = xstrdup(conf_get_action(this->C, this->wfile));
  logger(this->logger, "ACTION: new value: %s", this->action);

  if (this->clear != NULL && strlen(this->clear) > 0) {
    logger(this->logger, "CLEAR: old value: %s", this->clear);
    xfree(this->clear);
  }
  this->clear = xstrdup(conf_get_clear(this->C, this->wfile));
  logger(this->logger, "CLEAR: new value: %s", this->clear);

  if (this->exclude != NULL && strlen(this->exclude) > 0) {
    logger(this->logger, "EXCLUDE: old value: %s", this->exclude);
    xfree(this->exclude);
  }
  this->exclude = xstrdup(conf_get_exclude(this->C, this->wfile));
  logger(this->logger, "EXCLUDE: new value: %s", this->exclude);

  logger(this->logger, "RECURSE: old value: %s", (this->recurse==TRUE)?"true":"false");
  this->recurse  = conf_get_recurse(this->C, this->wfile);
  logger(this->logger, "RECURSE: old value: %s", (this->recurse==TRUE)?"true":"false");

  logger(this->logger, "THROTTLE: old value: %s", throttle_to_string(this->throttle));
  this->throttle = throttle_destroy(this->throttle);
  this->throttle = new_throttle(
    this->wfile, __parse_time(EXCEEDS, conf_get_throttle(this->C, this->wfile))
  ); 
  logger(this->logger, "THROTTLE: new value: %s", throttle_to_string(this->throttle));

  this->serial   = conf_get_serial(this->C);
  logger(this->logger, "configuration number %d is now loaded", this->serial);
}

BOOLEAN 
start(FIDO this)
{
  VERBOSE(is_verbose(this->C), "Starting fido[pid=%d, id=%u]", getpid(), pthread_self());
  if (strmatch(this->rfile, "modified")) {
    return __start_watcher(this);
  } else if (! strncmp(this->rfile, "exceeds", 7)) {
    return __start_agecheck(this);
  } else if (! strncmp(this->rfile, "count", 5)) {
    return __start_countcheck(this);
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
      char *cmd;
      char rule[256];
      RULE R; 
      
      if (is_daemon(this->C) == TRUE) {
        logger(this->logger,   "%s was modified since last check", this->wfile);
        VERBOSE(is_verbose(this->C), "%s was modified since last check", this->wfile);
      } else {
        VERBOSE(is_verbose(this->C), "%s was modified", this->wfile);
      }

      snprintf(rule, 256, "MODIFIED: %s", this->wfile);
      R   = new_rule(rule);
      cmd = __build_command(this, R);
      __run_command(this, cmd);
      xfree(cmd);
      R = rule_destroy(R);
      this->cache = tmp;
      __persist(this, put);
    }
    sleep(this->interval);
    if (this->serial != conf_get_serial(this->C)) {
      fido_reload(this);
    }
  }  
  return TRUE;
}

private BOOLEAN
__start_agecheck(FIDO this) 
{
  while (TRUE) {
    __agecheck(this, this->wfile);
    if (this->serial != conf_get_serial(this->C)) {
      fido_reload(this);
    }
    sleep(this->interval);
  }
  return TRUE;
}

/**
 * NOTE: The serial check for fido_reload is in an
 *       endless loop inside __start_agecheck
 */
private BOOLEAN
__agecheck(FIDO this, char *dir)
{
  DIR *D;
  int sec = __parse_time(EXCEEDS, this->rfile);

  D = opendir(dir);
  if (! D) {
    fprintf (stderr, "Cannot open directory '%s'\n", dir);
    exit (EXIT_FAILURE);
  }
  while (TRUE) { 
    int res = 0;

    if (! __is_directory(this->wfile)) {
      /**
       * We're watching a single file
       */
      if (__exceeds(this, this->wfile, sec) == TRUE) {
        this->event = TRUE;
        res  = __run_command(this, this->action);
        if (res != 0) {
          VERBOSE(is_verbose(this->C), "ERROR: command failed: %s", this->action);
        }
      } else {
        if (this->event == TRUE) {
          // clear
        }
        this->event = FALSE;
      }
    } else {
      /**
       * We're monitoring a directory
       */
      RSET   rset;
      const char    *name;
      struct dirent *entry;

      entry = readdir (D);
      if (! entry) {
        break;
      }
      name = entry->d_name;
     
      rset =  __ereg(this->exclude, entry->d_name); 
      if (! rset_get_result(rset) != 0 && strcmp(name, "..") && strcmp(name, ".") != 0 ) {
        int len;
        char path[MAXPATH];

        len = snprintf(path, MAXPATH, "%s/%s", dir, name);
        if (len >= MAXPATH) {
          logger(this->logger, "FATAL: the path (%s) is too long. Maximum path lenght is %d", path, MAXPATH);
          VERBOSE( 
            is_verbose(this->C), 
            "FATAL: the path (%s) is too long. Maximum path lenght is %d", 
            path, MAXPATH
          );
          exit (1);
        }
        if (__exceeds(this, path, sec) == TRUE) {
          if (is_daemon(this->C) == TRUE) {
            logger(this->logger,   "%s (%s) exceeds %d seconds", path, entry->d_name, sec);
            VERBOSE(is_verbose(this->C), "%s (%s) exceeds %d seconds", path, entry->d_name, sec);
          } else {
            VERBOSE(is_verbose(this->C), "%s (%s) exceeds %d seconds", path, entry->d_name, sec);
          }
          res  = __run_command(this, this->action);
          if (res != 0) {
            VERBOSE(is_verbose(this->C), "ERROR: command failed: %s", this->action);
          }
        }
        if ((this->recurse) && (entry->d_type & DT_DIR)) {
           __agecheck(this, path);
        }
      }
      rset = rset_destroy(rset);
    }
  }
  if (closedir(D)) {
    fprintf (stderr, "Could not close '%s'\n", dir);
    exit (EXIT_FAILURE);
  }
  return TRUE;
}

private BOOLEAN
__start_countcheck(FIDO this)
{
  while (TRUE) {
    __countcheck(this, this->wfile);
    if (this->serial != conf_get_serial(this->C)) {
      fido_reload(this);
    }
    sleep(1);
  }
  return TRUE;
}

private BOOLEAN
__countcheck(FIDO this, char *dir)
{
  DIR     *D;
  int     cnt   = 0;
  int     sum   = -1;
  int     op    = GREATERTHAN;
  char    *tmp  = this->rfile;

  if (tmp == NULL || strlen(tmp) < 1) return FALSE;

  /**
   * op is set by __parse_count
   */
  cnt = __parse_count(this->rfile, &op);

  D = opendir(dir);
  if (! D) {
    fprintf (stderr, "Cannot open directory '%s'\n", dir);
    exit (EXIT_FAILURE);
  }
  
  while (TRUE) {
    int  res = 0;
    BOOLEAN okay  = TRUE;

    if (! __is_directory(dir)) {
      logger(this->logger, "%s [error] unable to open directory: %s and count files", program_name, dir);
      if (is_verbose(this->C)) {
        NOTIFY(ERROR, "unable to open directory: %s [exiting]", dir);
      }
      exit(1);
    } else {
      sum = __countfiles(dir, this->recurse);
      if (op == GREATERTHAN) {
        if (sum > cnt) { 
          okay = FALSE;
          VERBOSE(is_verbose(this->C), "[alert] file count in %s (%d) is greater than %d", dir, sum, cnt);
        } else { 
          okay = TRUE;
          VERBOSE(is_verbose(this->C), "[pass] file count in %s (%d) is less than %d", dir, sum, cnt);
        }
      } else if (op == EQUALTO) {
        if (sum == cnt) { 
          okay = FALSE;
          VERBOSE(is_verbose(this->C), "[alert] file count in %s (%d) is equal to %d", dir, sum, cnt);
        } else {
          okay = TRUE;
          VERBOSE(is_verbose(this->C), "[pass] file count in %s (%d) does not equal %d", dir, sum, cnt);
        }
      } else if (op == LESSTHAN) {
        if (sum < cnt) { 
          okay = FALSE;
          VERBOSE(is_verbose(this->C), "[alert] file count in %s (%d) is less than %d", dir, sum, cnt);
        } else {
          okay = TRUE;
          VERBOSE(is_verbose(this->C), "[pass] file count in %s (%d) is greater than %d", dir, sum, cnt);
        }
      }
      if (!okay) {
        this->event = TRUE;
        VERBOSE(is_verbose(this->C), "firing this command: %s", this->action);
        logger(this->logger, "%s [alert] firing: %s", program_name, this->action);
        res  = __run_command(this, this->action);
        if (res != 0) {
          VERBOSE(is_verbose(this->C), "ERROR: command failed: %s", this->action);
        }
      } else {
        if (this->event == TRUE) {
          if (this->clear != NULL && strlen(this->clear) > 2) {
            VERBOSE(is_verbose(this->C), "clearing event with: %s", this->clear);
            logger(this->logger, "%s [alert] clearing event with: %s", program_name, this->clear);
            res  = __run_command(this, this->clear);
            if (res != 0) {
              VERBOSE(is_verbose(this->C), "ERROR: command failed: %s", this->clear);
            }
          } else {
            VERBOSE(is_verbose(this->C), "clearing event: no program to run");
            logger(this->logger, "%s [alert] clearing event: no program to run", program_name);
          }
          this->event = FALSE;
        }
      }
      sleep(this->interval);
    }
  }
}

private int 
__countfiles(char *path, BOOLEAN recursive) {
  DIR    *dptr = NULL;
  struct dirent *direntp;
  char   *npath;
  if (!path) return 0;
  if ((dptr = opendir(path)) == NULL ) {
    return 0;
  }

  int count=0;
  while ((direntp = readdir(dptr))) {
    if (strcmp(direntp->d_name,".") == 0 || strcmp(direntp->d_name,"..") == 0) continue;
    switch (direntp->d_type) {
      case DT_REG:
        count++;
        break;
      case DT_DIR:
        if (! recursive) {
          break;
        }
        npath = malloc(strlen(path)+strlen(direntp->d_name)+2);
        sprintf(npath,"%s/%s",path, direntp->d_name);
        count += __countfiles(npath, recursive);
        free(npath);
        break;
    }
  }
  closedir(dptr);
  return count;
}


private BOOLEAN
__start_parser(FIDO this)
{
  int  rc = 0;
  long offset;
  
  while (TRUE) { // as of fido 1.0.8 we'll run whether we have a file or not
    if (__is_readable (this)) {
      offset = __get_offset(this);
      if (rc < 0) {
        offset = 0;
        rc     = 0;
      }
      while (rc == 0 && offset >= 0) {
        offset = __ticks (this, offset);
        if (offset >= 0) {
          sleep (1);
        } else {
          logger(this->logger, "%s [error] set return to -7; offset value: %ld", program_name, offset);
          rc = -7;
        }
        if (this->serial != conf_get_serial(this->C)) {
          fido_reload(this);
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

private long
__ticks (FIDO this, long offset)
{
  FILE *fp;
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
        for (x = 0; x < (int)array_length(this->rules); x++) {
          char *cmd;
          char *tmp;
          RSET rset;
          RULE rule = (RULE)array_get(this->rules, x);
          // XXX: do we want a condition or a an arg to __eregi for the capture??
          rset = __eregi(rule_get_pattern(rule), line);
          if (rset_get_result(rset) == TRUE) {
            int   ret;
            if (is_daemon(this->C) == TRUE) {
              logger(this->logger,   "matched %s in %s", rule_get_pattern(rule), this->wfile);
              VERBOSE(is_verbose(this->C), "matched %s in %s", rule_get_pattern(rule), this->wfile);
            } else {
              VERBOSE(is_verbose(this->C), "matched: %s in %s", line, this->wfile);
            }
            tmp  = __build_command(this, rule);
            cmd  = xmalloc(strlen(tmp)+rset_get_length(rset)+10);
            memset(cmd, '\0', strlen(tmp)+rset_get_length(rset)+10);
            strcpy(cmd, tmp);
            while (strstr(cmd, "$")){
              VERBOSE(is_verbose(this->C), "B CMD: %s", cmd);
              cmd = __evaluate(rset_get_group(rset), cmd);
              VERBOSE(is_verbose(this->C), "A CMD: %s", cmd);
            }
            VERBOSE(is_verbose(this->C), "TMP: %s", tmp);
            VERBOSE(is_verbose(this->C), "CMD: %s", cmd);
            ret  = __run_command(this, cmd);
            rset = rset_destroy(rset);
            xfree(cmd);
            xfree(tmp);
            if (ret < 0) {
              logger(this->logger, "ERROR: unable to run command: %s", this->action);
            }
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

private RSET
__eregi(char *pattern, char *string) 
{
  return __regex(pattern, string, REG_ICASE|REG_EXTENDED);
}

private RSET
__ereg(char *pattern, char *string) 
{
  return __regex(pattern, string, REG_EXTENDED);
}


static RSET
__regex (char *pattern, char *string, int cflags)
{
  int         res = 0;
  int         cnt = 0;
  regex_t     reg;
  regmatch_t *grp;
  char        buf[128];
  RSET rset = new_rset();
  rset_set_result(rset, FALSE);

  if (!pattern) return rset;
  if (!string)  return rset;
  
  res = regcomp(&reg, pattern, cflags);
  if (res != 0) {
    regerror(res, &reg, buf, sizeof(buf));
    regfree(&reg);
    return rset;
  }

  cnt = reg.re_nsub + 1;
  grp = (regmatch_t*)xmalloc(cnt * sizeof(regmatch_t));

  res = regexec(&reg, string, cnt, grp, 0);
  if (res == 0) {
    int i;

    rset_set_result(rset, TRUE);
    for (i = 1; i < cnt; i++) {
      if (grp[i].rm_so == -1)
        break;

      char *copy = malloc(strlen(string) + 1);
      memset(copy, '\0', strlen(string) +1);

      strcpy(copy, string);
      copy[grp[i].rm_eo] = '\0';
      rset_add(rset, "%s", copy + grp[i].rm_so);
      xfree(copy);
    }
  }

  regfree(&reg);
  xfree(grp);

  return rset;
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
       array_npush(this->rules, (RULE)R, RULESIZE);
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
        array_npush(this->rules, (RULE)R, RULESIZE);
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

private char *
__build_command(FIDO this, RULE rule) 
{
  char *cmd;

  if (throttled(this->throttle)) {
    return 0;
  }

  if (rule_get_property(rule) != NULL) {
    cmd = xmalloc(strlen(this->action)+strlen(rule_get_property(rule))+5);
    memset(cmd, '\0', strlen(this->action)+strlen(rule_get_property(rule))+5);
    snprintf(
      cmd, 
      strlen(this->action)+strlen(rule_get_property(rule))+5, "%s %s", 
      this->action, 
      rule_get_property(rule)
    );
  } else {
    cmd = xstrdup(this->action);
  }

  return cmd;
}

private int
__run_command(FIDO this, char *cmd) 
{
  int   ret;
  char  out[BUFSIZ];
  FILE *rp;

  if (throttled(this->throttle)) {
    return 0;
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

  memset(path, '\0', 512);
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
    int  ret;
    if ((fd = open(path, O_WRONLY|O_CREAT, 0644)) < 0) {
      return;
    } else {
      snprintf(buf, sizeof(buf), "%ld\n", this->cache);
      ret = write(fd, buf, strlen(buf));
      if (ret < 0) {
        logger(this->logger, "ERROR: unable to write tmp file: %s", path);
      }
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
__parse_count(char *p, int *op)
{
  int  i; 
  char *tmp;
  int  count = 0;
  char mod[] = "count";
 
  if (p == NULL || strlen(p) < 1) return -1;

  tmp = trim(p);
  __demodify(tmp, mod);
  tmp = trim(tmp);

  if (strmatch(tmp, ">")) {
    *op = GREATERTHAN;
  } else if (strmatch(tmp, ">=")) {
    *op = GREATERTHAN;
  } else if (strmatch(tmp, "==")) {
    *op = EQUALTO;
  } else if (strmatch(tmp, "<")) {
    *op = LESSTHAN;
  } else if (strmatch(tmp, "<=")) {
    *op = LESSTHAN;
  } else if (ISNUMBER(*tmp)){
    count = atoi(tmp);
  }
  for (i = 0; i < (int)strlen(tmp); i++) {
    if (! ISNUMBER(tmp[i])) {
       tmp++;
    }
  }
  count = atoi(tmp);
  return count;
}

private BOOLEAN
__digits_only(const char *s)
{
  while (*s) {
    if (isdigit(*s++) == 0) return FALSE;
  }
  return TRUE;
}

private int
__parse_time(int m, char *p)
{
  size_t x   = 0;
  int  time  = 0;
  int  secs  = 0;
  char *tmp  = NULL;
  char mod[] = "exceeds ";

  if (p == NULL || strlen(p) < 1) return -1;

  p = trim(p);

  if (__digits_only(p) == TRUE) {
    /** 
     * We didn't read the docs;
     * assume seconds
     */ 
    int i = atoi(p);
    return i; 
  }

  if (m == EXCEEDS) {
    __demodify(p, mod);
  }
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

#define BUFSIZE 10240
private char *
__evaluate(ARRAY array, char *cmd)
{
  int   i   = 0;
  int   len = 0;
  char *res;
  char *ptr;
  char *end;
  char  buf[BUFSIZE];
  int   index = 0;
  char *scan;


  res = realloc(cmd, BUFSIZE+10);
  if (res != NULL) {
    cmd = res;
  }

  scan  = strchr(cmd, '$') + 1;
  len   = (strlen(cmd) - strlen(scan))-1;
  ptr   = (char*)scan;
  index = atoi(ptr);

  while (*ptr && isdigit(*ptr)) {
    ptr++;
  }
  end  = strdup(ptr);

  while (*scan && *scan != ' ' && *scan != '\n' && *scan != '\r') {
    (*scan) = (*scan)+1;
    i++;
  }

  if (scan[0] == ' ' || scan[0] == '\n' || scan[0] == '\r') {
    scan++;
  }

  memset(buf, '\0', sizeof buf);
  strncpy(buf, cmd, len);
  if (ptr != NULL) {
    /**
     * The fido user counts from 1, whereas the array
     * index starts at zero.
     */
    index = (index > 0) ? index - 1 : 0;
    if (index < (int)array_length(array)) {
      strcat(buf, array_get(array, index));
      strcat(buf, " ");
    }
  }
  strcat(buf, end);
  memset(res, '\0', BUFSIZE * sizeof(char));
  strncpy(res, buf, strlen(buf));
  free(end);
  return res;
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
    logger(this->logger, "WARNING: unable to read %s - in use by another process", path);
    return FALSE;
  }

  timeinfo = localtime (&(s.st_mtime));
  memset(buf, '\0', sizeof(buf));
  strftime(buf, 20, "%b %d %H:%M", timeinfo);

  secs = (int)difftime(time(NULL), s.st_mtime);
  if (secs > age) {
    logger(this->logger, "ALERT: %s exceeds %d seconds (%s)", path, age, buf);
    return TRUE;
  }
  return FALSE;
}

