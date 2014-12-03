#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <hash.h>
#include <conf.h>
#include <version.h>
#include <util.h>
#include <joedog/boolean.h>
#include <joedog/joedog.h>
#include <joedog/defs.h>

struct CONF_T
{
  BOOLEAN         verbose;
  BOOLEAN         debug;
  BOOLEAN         daemon;
  int             serial; 
  char *          pidfile;
  char *          logfile;
  char *          cfgfile;
  char *          rulesdir;
  char *          user;
  char *          group;
  BOOLEAN         capture;
  HASH            items;
  pthread_mutex_t lock;
};

/* XXX: hardcode alert!!! */
static const int defaults_length = 4;
static const char *defaults[] = {
  "/etc/fido/fido.conf",
  "/etc/fido.conf",
  "/usr/local/etc/fido.conf",
  "/usr/local/etc/fido/fido.conf"
};

private BOOLEAN __file_exists(char *file);
private void    __set_cfgfile(CONF this);
private void    __set_rulesdir(CONF this);

CONF
new_conf()
{
  CONF this;
  
  this = calloc(sizeof(struct CONF_T),1);
  this->serial    = 1000;
  this->verbose   = FALSE;
  this->cfgfile   = NULL;
  this->rulesdir  = NULL;
  this->user      = NULL;
  this->group     = NULL;
  this->logfile   = xstrdup("syslog");
  this->pidfile   = xstrdup("/var/run/fido.pid");
  this->capture   = FALSE;
  set_debug(this,FALSE);
  __set_cfgfile(this);
  __set_rulesdir(this);
  this->items = new_hash(4);
  if (pthread_mutex_init(&this->lock, NULL) != 0) {
    this->serial = -1; // disable conf_reload
  } 
  return this;
}

void 
conf_reload(CONF this) 
{
  if (this->serial < 1000) {
    SYSLOG(ERROR, "configuration reload is disabled; you'll have to restart the daemon");
    NOTIFY(ERROR, "configuration reload is disabled; you'll have to restart the daemon");
    VERBOSE(this->verbose, "Configuration reload is disabled; you'll have to restart the daemon");
    return;
  }
  pthread_mutex_lock(&this->lock);
  hash_destroy(this->items);
  this->items  = new_hash(4); 
  parse_cfgfile(this);
  this->serial = (this->serial+10);
  pthread_sleep_np(1);
  pthread_mutex_unlock(&this->lock);
}

void
conf_destroy(CONF this) {
  pthread_mutex_destroy(&this->lock);
  //XXX: loop through this->items and destroy each item
  hash_destroy(this->items);
  xfree(this->user);
  xfree(this->group);
  xfree(this->logfile);
  xfree(this->pidfile);
  xfree(this->rulesdir);
  xfree(this->cfgfile);
  xfree(this);
  return;
}

void
show(CONF this, BOOLEAN quit)
{
  int  x;
  char **keys;

  printf("GENERAL CONFIG:\n");
  printf("  version:                      %s\n", version_string);
  printf("  verbose:                      %s\n", this->verbose?"true":"false");
  printf("  debug:                        %s\n", this->debug?"true":"false");
  printf("  daemon:                       %s\n", this->daemon?"true":"false");
  printf("  config file:                  %s\n", this->cfgfile);
  printf("  rules directory:              %s\n", this->rulesdir);
  printf("  log file:                     %s\n", this->logfile);
  printf("  pid file:                     %s\n", this->pidfile);
  printf( "\n" );
  printf("FILES TO WATCH:\n");
  keys = hash_get_keys_delim(this->items, ':');
  if (!keys || !keys[0]) {
    puts("    [none defined]");
  }
  for (x = 0; x < hash_get_entries(this->items) && keys[x] != NULL; x ++) {
    puts(keys[x]);
    char *tmp;
    tmp = hash_get(this->items, "%s:rules", keys[x]);
    if (tmp) {
      if (*tmp == '/' || *tmp == '.') {
        printf("     rules:  %s\n", tmp);
      } else if (strmatch(tmp, "modified")) {
        printf("     rules:  if modified\n");
      } else if (! strncmp(tmp, "exceeds", 7)) {
        printf("     rules:  %s\n", tmp);
      } else if (strmatch(tmp, "recurse")) {
        printf("     rules:  %s\n", tmp);
      } else {
        printf("     rules:  %s/%s\n", this->rulesdir, tmp);
      }
    }
    tmp = hash_get(this->items, "%s:action", keys[x]);
    if (tmp) {
      printf("    action:  %s\n", tmp);
    }
    tmp = hash_get(this->items, "%s:exclude", keys[x]);
    if (tmp) {
      printf("   exclude:  %s\n", tmp);
    }
    tmp = hash_get(this->items, "%s:recurse", keys[x]);
    if (tmp) {
      printf("   recurse:  %s\n", tmp);
    }
    tmp = hash_get(this->items, "%s:user", keys[x]);
    if (tmp) {
      printf("    user:   %s\n", tmp);
    }
    tmp = hash_get(this->items, "%s:group", keys[x]);
    if (tmp) {
      printf("    group:  %s\n", tmp);
    }
  }
  if (quit) exit(0);
  return;
}

int 
conf_get_serial(CONF this) 
{
  return this->serial;
}

char *
conf_get_pidfile(CONF this) 
{
  /**
   * we ensure a value at instantiation
   * default value: /var/run/watchdog.pid
   */
  return this->pidfile;
}

char *
conf_get_rules(CONF this, char *key)
{
  char *tmp;
  char fn[1024];
  char *ret = xmalloc(1024);

  tmp = hash_get(this->items, "%s:rules", key);
  memset(ret, '\0', 1024);
  if (tmp) {
    if (*tmp == '/' || *tmp == '.') {
      snprintf(ret, 1024, "%s", tmp);
    } else if (strmatch(tmp, "modified")) {
      snprintf(ret, 1024, "%s", tmp);
    } else if (strmatch(tmp, "recurse")) {
      snprintf(ret, 1024, "%s", tmp);
    } else if (! strncmp(tmp, "exceeds", 7)) {
      snprintf(ret, 1024, "%s", tmp);
    } else {
      snprintf(fn, 1024, "%s/%s", this->rulesdir, tmp);
      if (__file_exists(fn)) {
        snprintf(ret, 1024, "%s/%s", this->rulesdir, tmp);
      } else {
        snprintf(ret, 1024, "%s", tmp);
      }
    }
  }
  return ret;
}

char *
conf_get_action(CONF this, char *key)
{
  return hash_get(this->items, "%s:action", key);
}

char *
conf_get_exclude(CONF this, char *key)
{
  return hash_get(this->items, "%s:exclude", key);
}

BOOLEAN 
conf_get_recurse(CONF this, char *key) {
  char *str = hash_get(this->items, "%s:recurse", key);
  if (str == NULL) return FALSE;
  return strmatch(str, "true");
}

char * 
conf_get_throttle(CONF this, char *key) 
{
  return hash_get(this->items, "%s:throttle", key);
}


char *
conf_get_log(CONF this, char *key) {
  if (hash_get(this->items, "%s:log", key) != NULL) {
    return hash_get(this->items, "%s:log", key);
  } else {
    return this->logfile;
  }
}

char *
conf_get_user(CONF this)
{
  return this->user;
}

char *
conf_get_group(CONF this)
{
  return this->group;
}

BOOLEAN
conf_get_capture(CONF this, char *key) {
  char *tmp;

  if (hash_get(this->items, "%s:capture", key) != NULL) {
    tmp = hash_get(this->items, "%s:capture", key);
    if (strmatch(tmp, "true")) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else {
    return this->capture;
  }
}

HASH
conf_get_items(CONF this) 
{
  return this->items;
}

int 
conf_get_count(CONF this)
{
  int  x;
  int  count = 0;
  char **keys;

  keys = hash_get_keys_delim(this->items, ':');
  for (x = 0; x < hash_get_entries(this->items) && keys[x] != NULL; x ++) {
    count++;
  }
  hash_free_keys(this->items, keys);
  return count;
}

BOOLEAN
parse_cfgfile(CONF this)
{
  FILE *fp;
  int   line_num = 0;
  char *line;
  char *tmp;
  char *option;
  char *optionptr;
  char *value;
  char *section = "";
  BOOLEAN in = FALSE;

  if(!__file_exists(this->cfgfile)){
    return FALSE;
  }

  if ((fp = fopen(this->cfgfile, "r")) == NULL) {
    return FALSE;
  }

  while ((line = chomp_line(fp, &line, &line_num)) != NULL) {
    char *sb, *eb;
    tmp = line;
    optionptr = option = xstrdup(line);
    while (*optionptr && !ISSPACE((int)*optionptr) && !ISSEPARATOR(*optionptr))
      optionptr++;
    if (*optionptr) // are we already at the end?
      *optionptr++=0;
    while (ISSPACE((int)*optionptr) || ISSEPARATOR(*optionptr))
      optionptr++;
    value  = xstrdup(optionptr);
    while (*line)
      line++;
    if ((sb = strchr(tmp, '{')) != NULL) {        // start bracket
      section = xstrdup(option);
      hash_put(this->items, section, section);
      in = TRUE;
    } else if ((eb = strchr(tmp, '}')) != NULL) { // end bracket
      xfree(section);
      in = FALSE;
    } else { 
      if (in) {
        char tmp[64];
        sprintf(tmp, "%s:%s", section, option);
        hash_put(this->items, tmp, value);
      } else {
        // default section: key = val
        if (strmatch(option, "verbose")) {
          if (!strncasecmp(value, "true", 4)) {
            this->verbose = TRUE;
          } else {
            this->verbose = FALSE;
          }
        }
        if (strmatch(option, "daemon")) {
          if (!strncasecmp(value, "true", 4)) {
            this->daemon = TRUE;
          } else {
            this->daemon = FALSE;
          }
        }
        if (strmatch(option, "capture")) {
          if (!strncasecmp(value, "true", 4)) {
            this->capture = TRUE;
          } else {
            this->capture = FALSE;
          }
        }
        if (strmatch(option, "rulesdir")) {
          if (value && *value) {
            if (this->rulesdir != NULL && strlen(this->rulesdir) > 0) {
              xfree(this->rulesdir);
            }
            this->rulesdir = xstrdup(value);
          }
        }
        if (strmatch(option, "log")) {
          if (value && *value) {
            if (this->logfile != NULL && strlen(this->logfile) > 0) {
              xfree(this->logfile);
            }
            this->logfile = xstrdup(value);
          }
        }
        if (strmatch(option, "pid")) {
          if (value && *value) {
            if (this->pidfile != NULL && strlen(this->pidfile) > 0) {
              xfree(this->pidfile);
            }
            this->pidfile = xstrdup(value);
          }
        }
        if (strmatch(option, "user")) {
          if (value && *value) {
            if (this->user != NULL && strlen(this->user) > 0) {
              xfree(this->user);
            }
            this->user = xstrdup(value);
          }
        }
        if (strmatch(option, "group")) {
          if (value && *value) {
            if (this->group != NULL && strlen(this->group) > 0) {
              xfree(this->group);
            }
            this->group = xstrdup(value);
          }
        }
      }
    }
    xfree(value);
    xfree(option);
    xfree(tmp);
  } /* end of while chomp_line */
  fclose(fp);

  if (!this->rulesdir) {
    this->rulesdir = xmalloc(64);
    snprintf(this->rulesdir, 64, "/etc/%s/rules", program_name);
  } 
  if (!this->pidfile) {
    this->pidfile = xmalloc(64);
    snprintf(this->pidfile, 64, "/var/run/%s.pid", program_name);
  }
  if (!this->logfile) {
    this->logfile = xmalloc(64);
    snprintf(this->logfile, 64, "/var/log/%s.log", program_name);
  }
  return 0;
}

BOOLEAN
is_daemon(CONF this) 
{
  return this->daemon;
}

BOOLEAN
is_verbose(CONF this)
{
  return this->verbose;
}

void
set_cfgfile(CONF this, char *file)
{
  if (this->cfgfile != NULL && strlen(this->cfgfile) > 0) {
    xfree(this->cfgfile);
  }
  this->cfgfile = xstrdup(file);
  return;
}

void
set_verbose(CONF this, BOOLEAN verbose) 
{
  this->verbose = verbose;
  return;
}

void
set_debug(CONF this, BOOLEAN debug) 
{
  this->debug = debug;
  return;
}

void
set_daemon(CONF this, BOOLEAN daemon) 
{
  this->daemon = daemon;
  return;
}

void
set_logfile(CONF this, char *logfile) 
{
  if (this->logfile != NULL && strlen(this->logfile) > 0) {
    xfree(this->logfile);
  }
  this->logfile = strdup(logfile);
  return;
}

void
set_pidfile(CONF this, char *pidfile)
{
  if (this->pidfile != NULL && strlen(this->pidfile) > 0) {
    xfree(this->pidfile);
  }
  this->pidfile = strdup(pidfile);
  return;
}

private void
__set_cfgfile(CONF this)
{
  char *env;
  int   x;
  
  env = getenv("FIDORC");
  if(env && *env){
    if(!__file_exists(env)){
      fprintf(stderr, "ERROR: FIDORC points to '%s' which does not exist\n", env);
    } else {
      if (this->cfgfile != NULL && strlen(this->cfgfile) > 0) {
        xfree(this->cfgfile);
      }
      this->cfgfile = xstrdup(env);
      return;
    }
  }

  for (x = 0; x < defaults_length; x ++) { 
    if(__file_exists((char*)defaults[x])){
      if (this->cfgfile != NULL && strlen(this->cfgfile) > 0) {
        xfree(this->cfgfile);
      }
      this->cfgfile = xstrdup(defaults[x]);
      return;
    }
  }
  return;
}

private void
__set_rulesdir(CONF this) 
{
  const char *last;
  char        path[256];

  if (this->cfgfile == NULL) {
    if (this->rulesdir != NULL && strlen(this->rulesdir) > 0) {
      xfree(this->rulesdir);
    }
    this->rulesdir = xstrdup("/etc/fido/rules");
    return;
  }
  last = rindex(this->cfgfile, '/');
  memset(path, '\0', sizeof path);
  strncpy(path, this->cfgfile, strlen(this->cfgfile)-strlen(last));
  this->rulesdir = malloc(strlen(path)+7);
  memset(this->rulesdir, '\0', strlen(path)+7);
  snprintf(this->rulesdir, strlen(path)+7, "%s/rules", path);
  return;
}

private BOOLEAN
__file_exists(char *file)
{
  int  fd;

  /* open the file read only  */
  if ((fd = open(file, O_RDONLY)) < 0){
    /* the file does NOT exist  */
    //close(fd);
    return FALSE;
  } else {
    /* party on Garth... */
    close(fd);
    return TRUE;
  }

  return FALSE;
}


