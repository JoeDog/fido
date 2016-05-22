dnl REQUIRE AUTOCONF 2.50 OR HIGHER...
dnl AC_PREREQ(2.50) 

dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl AC_INIT
dnl AC_CONFIG_SRCDIR([src/main.c]) 
dnl AM_CONFIG_HEADER(include/config.h)

dnl
dnl Define the utils directory
dnl
dnl AC_CONFIG_AUX_DIR(utils)  

dnl VERSION=`sed -e 's/^.*"\(.*\)";$/\1/' ${srcdir}/src/version.c`
dnl VERSION=`sed -ne 's/.*version_string.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
dnl PROGRAM=`sed -ne 's/.*program_name.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
dnl AUTHOR=`sed -ne 's/.*author_name.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
dnl EMAIL=`sed -ne 's/.*email_address.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
dnl YEARS=`sed -ne 's/.*years.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
dnl AM_INIT_AUTOMAKE($PROGRAM, $VERSION)
dnl DATE=`${CONFIG_SHELL} ${srcdir}/utils/mkstamp`
dnl AC_SUBST(DATE)
dnl AC_SUBST(PROGRAM)
dnl AC_SUBST(AUTHOR)
dnl AC_SUBST(EMAIL)
dnl REQUIRE AUTOCONF 2.50 OR HIGHER...
AC_PREREQ(2.63)

dnl
dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT(
  m4_esyscmd_s([sed -ne 's/.*program_name.*"\(.*\)"\;/\1/p' src/version.c]),
  m4_esyscmd_s([sed -ne 's/.*version_string.*"\(.*\)"\;/\1/p' src/version.c]),
)
AC_CONFIG_AUX_DIR(utils)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/setup.h])
AC_CONFIG_HEADERS(include/config.h)

dnl canonicalize the host
AC_CANONICAL_HOST

DATE=`${CONFIG_SHELL} ${srcdir}/utils/mkstamp`
VERSION=`sed -ne 's/.*version_string.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
PROGRAM=`sed -ne 's/.*program_name.*"\(.*\)"\;/\1/p' src/version.c`
AUTHOR=`sed -ne 's/.*author_name.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
EMAIL=`sed -ne 's/.*email_address.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
YEARS=`sed -ne 's/.*years.*"\(.*\)"\;/\1/p' ${srcdir}/src/version.c`
PLATFORM="${host_vendor}-${host_cpu}-${host_os}"
AC_SUBST(DATE)
AC_SUBST(PROGRAM)
AC_SUBST(AUTHOR)
AC_SUBST(EMAIL)
AC_SUBST(PLATFORM)

dnl
dnl We want to set some system defaults
case "$prefix" in
  "/usr"|NONE)
    case "$sysconfdir" in
      '${prefix}/etc')
       sysconfdir="/etc/$PROGRAM"
       ;;
     esac
     case "$localstatedir" in
       '${prefix}/var')
       localstatedir="/var"
       ;;
  *)
    sysconfdir="${sysconfdir}/$PROGRAM"
    ;;
  esac
    ;;
*)
  sysconfdir="${sysconfdir}/$PROGRAM"
  ;;
esac

dnl
dnl canonicalize the host
dnl
AC_CANONICAL_HOST
PLATFORM="${host_vendor}-${host_cpu}-${host_os}"
AC_SUBST(PLATFORM)
 
dnl Check for AIX weirdos
AC_AIX
 
dnl Checks for programs.
AC_PROG_CC 

AM_PROG_LIBTOOL 

dnl
dnl Program support
dnl
AC_PATH_PROG( PERL, perl, false )
AC_PROG_SHELL
AC_PROG_MAKE_SET
AC_PROG_INSTALL
case "$host_os" in
  *aix*)
    if test -n "${CC}" ; then
      AC_CHECK_PROGS(CC_R, xlc_r cc_r cc)
      if test "$CC_R" = cc ; then
        AC_MSG_ERROR([pthread support requires cc_r (or other suitable compiler) on AIX])
      else
        CC=$CC_R
        AC_SUBST(CC)
      fi
    fi
  ;;
esac 

if test -n "$GCC"; then
  oldCFLAGS=$CFLAGS
  CFLAGS="$CFLAGS -W -Wall -Werror"
  AC_MSG_CHECKING(for buggy pthread mutex initializers)

  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([#include <pthread.h>],
    [pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; pthread_mutex_lock(&m); ])],
    [buggy_init="no"], [buggy_init="yes"])

  if test $buggy_init = "no" ; then
     WARN_CFLAGS="-W -Wall"
     AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT(yes)
    WARN_CFLAGS="-W -Wall -Wno-missing-braces"
  fi
fi
CFLAGS=$oldCFLAGS
AC_SUBST(WARN_CFLAGS)

AC_MSG_CHECKING([for dlopen() in -ldld])
olibs=$LIBS
LIBS="$LIBS -ldld"
AC_TRY_LINK(, 
  [ extern void* dlopen(); dlopen(); ],
  AC_MSG_RESULT(yes);
  LIBS=$olibs;
  LDL=-ldld
  AC_DEFINE(HAVE_SHL_LOAD, 1, [ Define if we have shl_load() ]),
  AC_MSG_RESULT(no);
  LDL=
  LIBS=$olibs)

AC_MSG_CHECKING([for dlopen() in -ldl])
olibs=$LIBS
LIBS="$LIBS -ldl"
AC_TRY_LINK(,
  [ extern void* dlopen(); dlopen(); ],
  AC_MSG_RESULT(yes);
  LIBS=$olibs;
  LDL=-ldl,
  AC_MSG_RESULT(no);
  LDL=
  LIBS=$olibs)

AC_SUBST(LDL)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(sys/select.h)
AC_CHECK_HEADERS(sys/time.h sys/times.h)
AC_CHECK_HEADERS(sys/resource.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(pthread.h) 
AC_CHECK_HEADERS(string.h) 
AC_CHECK_HEADERS(strings.h) 
AC_CHECK_HEADERS(pthread.h) 


dnl We're gonna check for regex and perl-compat regex but at the
dnl time of this implementation, only the former is supported. 
AC_CHECK_HEADER([regex.h], [have_regex_h=yes])
if test "$have_regex_h" = "yes"; then
  AC_DEFINE([HAVE_REGEX_H], [1], [Define to 1 if regex.h is available])
else
  AC_CHECK_HEADER([pcreposix.h], [have_pcreposix_h=yes])
fi

if test "$have_pcreposix_h" = "yes"; then
  AC_DEFINE([PCRE_STATIC], [1], [Box Backup always uses static PCRE])
  AC_SEARCH_LIBS([regcomp], ["pcreposix -lpcre"],,[have_pcreposix_h=no_regcomp])
fi

if test "$have_pcreposix_h" = "yes"; then
  AC_DEFINE([HAVE_PCREPOSIX_H], [1], [Define to 1 if pcreposix.h is available])
fi

if test "$have_regex_h" = "yes" -o "$have_pcreposix_h" = "yes"; then
  have_regex_support=yes
  AC_DEFINE([HAVE_REGEX_SUPPORT], [1], [Define to 1 if regular expressions are supported])
else
  have_regex_support=no
fi

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl
dnl Return type of signal-handlers
dnl
AC_TYPE_SIGNAL

dnl
dnl Checks for library functions.
dnl
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(strchr memcpy strncpy strstr strlen)
AC_CHECK_FUNCS(strncasecmp strncmp)
AC_CHECK_FUNCS(socket, , AC_CHECK_LIB(socket, socket))
AC_CHECK_FUNCS(gethostbyname, , AC_CHECK_LIB(nsl, gethostbyname))
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(rand_r)
AC_CHECK_FUNCS(localtime_r)
AC_CHECK_FUNCS(getipnodebyname)
AC_CHECK_FUNCS(freehostent)
AC_CHECK_FUNCS(getopt_long)

dnl
dnl Check for socket library
dnl
AC_CHECK_LIB(socket, socket)

dnl
dnl Check for pthread support
dnl
PTHREAD_CFLAGS=error
PTHREAD_LDFLAGS=error

dnl If it's error, then the user didn't 
dnl define it.
if test "x$PTHREAD_LDFLAGS" = xerror; then
  AC_CHECK_LIB(pthread, pthread_attr_init, [
             PTHREAD_CFLAGS="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
             PTHREAD_LDFLAGS="-lpthread" ])
fi

if test "x$PTHREAD_LDFLAGS" = xerror; then
  AC_CHECK_LIB(pthreads, pthread_attr_init, [
             PTHREAD_CFLAGS="-D_THREAD_SAFE"
             PTHREAD_LDFLAGS="-lpthreads" ])
fi

if test "x$PTHREAD_LDFLAGS" = xerror; then
   AC_CHECK_LIB(c_r, pthread_attr_init, [
                PTHREAD_CFLAGS="-D_THREAD_SAFE -pthread"
                PTHREAD_LDFLAGS="-pthread" ])
fi

if test "x$PTHREAD_LDFLAGS" = xerror; then
   AC_CHECK_FUNC(pthread_attr_init, [
                 PTHREAD_CFLAGS="-D_REENTRANT"
                 PTHREAD_LDFLAGS="-lpthread" ])
fi

if test $PTHREAD_LDFLAGS = "error"; then
  AC_MSG_WARN(pthread library NOT found: guessing and hoping for the best....)
  PTHREAD_CFLAGS="-D_REENTRANT"
  PTHREAD_LDFLAGS="-lpthread"
fi

AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LDFLAGS)

dnl
dnl update dates and versioning in doc
dnl
AC_OUTPUT_COMMANDS([
  for file in doc/fido.1 doc/fido.conf.5;
  do
    rm -f $file
    sed -e "s|\$_VERSION|$VERSION|g" \ 
        -e "s|\$_PROGRAM|$PROGRAM|g" \     
        -e "s|\$_DATE|$DATE|g" \     
        -e "s|\$_AUTHOR|$AUTHOR|g" \     
        -e "s|\$_YEARS|$YEARS|g" \     
        -e "s|\$_EMAIL|$EMAIL|g" \     
        < $file.in > $file 
  done
], [ VERSION="$VERSION" DATE="$DATE" PROGRAM="$PROGRAM" AUTHOR="$AUTHOR" EMAIL="$EMAIL" YEARS="$YEARS"])

AC_OUTPUT(Makefile include/Makefile include/joedog/Makefile src/Makefile doc/Makefile utils/Makefile)

echo
echo "--------------------------------------------------------"
echo "Configuration is complete "
echo "  Conf  directory: $sysconfdir"
echo "  Rules directory: $sysconfdir/rules"
echo "  Run   directory: $localstatedir/run"
echo "  Sbin  directory: $sbindir"
echo "  Man   directory: $mandir"
echo
echo "Run the following commands to complete the installation:"
echo "  make "
echo "  su   "
echo "  make install"
echo
echo "For complete documentation:        http://www.joedog.org"
echo "--------------------------------------------------------" 
