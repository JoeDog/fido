#ifndef __SETUP_H
#define __SETUP_H

#ifdef  HAVE_CONFIG_H
# include <config.h>
#endif/*HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif/*HAVE_UNISTD_H*/

#ifdef  HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif/*HAVE_SYS_TIMES_H*/

#if  TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif/*TIME_WITH_SYS_TIME*/


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

#include <conf.h>

#if INTERN
# define EXTERN /* */
#else
# define EXTERN extern
#endif

EXTERN CONF C;

#endif/*__SETUP_H*/
