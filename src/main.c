/**
 * Fido - a log file watcher
 *
 * Copyright (C) 2011-2014 by  
 * Jeffrey Fulmer - <jeff@joedog.org>, et al. 
 * This file is distributed as part of Fido
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 
 *
 */
#ifdef  HAVE_CONFIG_H
# include <config.h> 
#endif/*HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>

#define INTERN  1

#ifdef STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <version.h>
#ifdef __CYGWIN__
# include <getopt.h>
#else
# include <joedog/getopt.h>
#endif

#include <joedog/defs.h>
#include <joedog/boolean.h>
#include <joedog/joedog.h>

#include <crew.h>
#include <hash.h>
#include <conf.h>
#include <logger.h>
#include <pid.h>
#include <fido.h>
#include <util.h>
#include <runner.h>
#include <setup.h>

/**
 * long options, std options struct
 */
static struct option long_options[] =
{
  { "version",    no_argument,       NULL, 'V' },
  { "help",       no_argument,       NULL, 'h' },
  { "config",     no_argument,       NULL, 'C' },
  { "file",       required_argument, NULL, 'f' },
  { "verbose",    no_argument,       NULL, 'v' },
  { "timeout",    required_argument, NULL, 't' },
  { "debug",      no_argument,       NULL, 'D' },
  { "daemon",     optional_argument, NULL, 'd' },
  { "log",        required_argument, NULL, 'l' },
  { "pid",        required_argument, NULL, 'p' },
};

/**
 * version_string is defined in version.c adding it to a 
 * separate file allows us to parse it in configure.  
 */
void 
display_version(BOOLEAN b) {   
  char name[128];

  memset(name, 0, sizeof name);
  strncpy(name, program_name, strlen(program_name));

  if(b == TRUE){
    printf("%s %s\n\n%s\n", uppercase(name, strlen(name)), version_string, copyright);
    exit(EXIT_SUCCESS);
  } else {
    printf("%s %s\n", uppercase(name, strlen(name)), version_string);
  }
}

/**
 * display help section to STDOUT and exit
 */
void
display_help(int status)
{
  display_version(FALSE);
  printf("Usage: %s [options]\n", program_name);
  printf("Options:\n"                    );
  puts("  -V, --version             VERSION, prints version number to screen.");
  puts("  -h, --help                HELP, prints this section.");
  puts("  -v, --verbose             VERBOSE, prints notification to screen.");
  printf(
    "  -f, --file=/path/file     FILE, an alternative config file (default: /etc/%s/%s.conf)\n", 
    program_name, program_name
  );
  puts("  -C, --config              CONFIG, displays the default settings and exits");
  puts("  -d, --daemon=[true|false] DAEMON, run in the background as a daemon (-d <no arg> means true)");
  puts("  -l, --log=arg             LOG, set the default log which can be overridden at the file level.");
  puts("                            You can include a path to a file or 'syslog' to use system logging");
  puts("  -p, --pid=/path/file      PID, set an alternative path to a process ID file.");
  printf("                            The default pid is: /var/run/%s.pid\n", program_name);
  exit(status);
}

void
parse_cmdline_cfg(CONF C, int argc, char *argv[]) {
  int a = 0;

  while(a > -1){
    a = getopt_long(argc, argv,"Vhvf:CDd:l:p:", long_options, (int*)0);
    if(a == 'f'){
      set_cfgfile(C, optarg);
      a = -1;
    }
  }
  optind = 0;
}

/**
 * parses command line arguments and assigns values to run time
 * variables. relies on GNU getopts included with this distribution.  
 */
void
parse_cmdline(CONF C, int argc, char *argv[]) {
  int c = 0;
  BOOLEAN display = FALSE;
  while ((c = getopt_long(argc, argv, "Vhvf:CDd:l:p:", long_options, (int *)0)) != EOF) {
  switch (c) {
      case 'V':
        display_version(TRUE);
        break;
      case 'h':
        display_help(EXIT_SUCCESS);
      case 'v':
        set_verbose(C, TRUE);
        break;
      case 'C':
        display = TRUE;
        break;
      //case 'f':
      //XXX: parsed separately
      //  set_cfgfile(C, optarg);
      //  break;
      case 'D':
        set_debug(C, TRUE);
        break;
      case 'd':
        if (!optarg) {
          puts("NO OPTARG");
        }
        if (optarg != NULL && !strncasecmp(optarg, "false", 5)) { 
          set_daemon(C, FALSE);
        } else {
          set_daemon(C, TRUE);
        }
        break;
      case 'l':
        set_logfile(C, optarg);
        break;
      case 'p':
        set_pidfile(C, optarg);
        break;
    } /* end of switch */
  }   /* end of while  */
  if (display) {
    show(C, TRUE);
  }
  return;
} /* end of parse_cmdline */

void 
sighandler(int sig)
{
  LOG L = new_logger("syslog");

  switch(sig) {
    case SIGHUP:
      logger(L, "Received SIGHUP signal; reloading config.");
      conf_reload(C);
      break;
    case SIGINT:
    case SIGTERM:
      errno = 0;
      logger(L, "Shutting down [TERM].");
      VERBOSE(is_verbose(C), "Stopping %s[pid=%d]", program_name, getpid());
      exit(EXIT_SUCCESS);
      break;
    default:
      logger(L, "Unhandled signal %s", strsignal(sig));
      break;
  }
}

void 
sigmasker() 
{
  sigset_t sigset;
  struct   sigaction action;
         
  if (getppid() == 1) {
    return;
  }
 
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGCHLD); 
  sigaddset(&sigset, SIGTSTP); 
  sigaddset(&sigset, SIGTTOU);
  sigaddset(&sigset, SIGTTIN); 
  sigprocmask(SIG_BLOCK, &sigset, NULL); 

  action.sa_handler = sighandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
         
  sigaction(SIGHUP, &action, NULL); 
  sigaction(SIGTERM, &action, NULL);  
  sigaction(SIGINT, &action, NULL); 
}

int
main(int argc, char *argv[])
{
  CREW    crew;
  int     i;
  int     count = 0;
  int     res;
  BOOLEAN result;
  char ** keys;
  void *  statusp;

  C = new_conf();
  parse_cmdline_cfg(C, argc, argv);
  parse_cfgfile(C);
  parse_cmdline(C, argc, argv);
  
  RUNNER R = new_runner(conf_get_user(C), conf_get_group(C));
  runas(R);
  runner_destroy(R);
  sigmasker();
  
  if (is_daemon(C)) {
    res = fork();
    if (res == -1 ){
      // ERRROR
      NOTIFY(FATAL, "%s: [error] unable to run in the background\n", program_name);
    } else if (res == 0) {
      // CHILD
      PID  P = new_pid(conf_get_pidfile(C));
      HASH H = conf_get_items(C);
      count  = conf_get_count(C);
      keys   = hash_get_keys_delim(H, ':');
      if ((crew = new_crew(count, count, FALSE)) == NULL) {
        NOTIFY(FATAL, "%s: [error] unable to allocate memory for %d log files", program_name, count);
      }
      set_pid(P, getpid());
      pid_destroy(P);
      for (i = 0; i < count && crew_get_shutdown(crew) != TRUE; i++) {
        FIDO F = new_fido(C, keys[i]);
        result = crew_add(crew, (void*)start, F);
        if (result == FALSE) {
          NOTIFY(FATAL, "%s: [error] unable to spawn additional threads", program_name);
        }
      }
      crew_join(crew, TRUE, &statusp);
      conf_destroy(C);
    } else {
      // PARENT 
    }
  } else {
    HASH H = conf_get_items(C);
    count  = conf_get_count(C);
    keys   = hash_get_keys_delim(H, ':');

    if ((crew = new_crew(count, count, FALSE)) == NULL) {
      NOTIFY(FATAL, "%s: [error] unable to allocate memory for %d log files", program_name, count);
    }
    for (i = 0; i < count && crew_get_shutdown(crew) != TRUE; i++) {
      FIDO F = new_fido(C, keys[i]);
      result = crew_add(crew, (void*)start, F);
    }
    crew_join(crew, TRUE, &statusp);
    conf_destroy(C);
  } 
  exit(EXIT_SUCCESS);
} /* end of int main **/


