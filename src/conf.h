#ifndef __CONF_H
#define __CONF_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <hash.h>

#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef struct CONF_T *CONF;

CONF     new_conf();
void     conf_destroy(CONF this);
void     conf_reload(CONF this);
void     show(CONF this, BOOLEAN quit);
BOOLEAN  parse_cfgfile(CONF this);

/* setters */
void     set_cfgfile(CONF this, char *file);
void     set_watchfile(CONF this, char *file);
void     set_verbose(CONF this, BOOLEAN verbose);
void     set_debug(CONF this, BOOLEAN debug);
void     set_daemon(CONF this, BOOLEAN daemon);
void     set_logfile(CONF this, char *logfile);
void     set_pidfile(CONF this, char *pidfile);

/* getters */
int      conf_get_serial(CONF this);
HASH     get_items (CONF this);
BOOLEAN  is_daemon (CONF this);
BOOLEAN  is_verbose(CONF this);
char *   conf_get_watchfile(CONF this);
char *   conf_get_pidfile(CONF this);
char *   conf_rules_dir(CONF this);
char *   conf_get_rules(CONF this, char *key);
char *   conf_get_action(CONF this, char *key);
char *   conf_get_clear(CONF this, char *key);
char *   conf_get_exclude(CONF this, char *key);
BOOLEAN  conf_get_recurse(CONF this, char *key); 
char *   conf_get_interval(CONF this, char *key);
char *   conf_get_throttle(CONF this, char *key);
char *   conf_get_log(CONF this, char *key);
char *   conf_get_user(CONF this);
char *   conf_get_group(CONF this);
HASH     conf_get_items(CONF this);
int      conf_get_count(CONF this);
BOOLEAN  conf_get_capture(CONF this, char *key);

#endif/*__CONF_H*/
