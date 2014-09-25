#ifndef DATE_H__
#define DATE_H__
#include <stdlib.h>
#include <stddef.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>

typedef enum {
  ATOM_FORMAT     =  0,  // 2010-05-21T13:33:59-0400
  COOKIE_FORMAT   =  1,  // Monday, 15-Aug-05 15:52:01 UTC
  ISO8601_FORMAT  =  2,  // 2010-05-21T13:33:59-0400
  SYSLOG_FORMAT   =  3,  // May 16 04:02:50
  RFC822_FORMAT   =  4,  // Mon, 15 Aug 05 15:52:01 +0000
  RFC850_FORMAT   =  5,  // Monday, 15-Aug-05 15:52:01 UTC
  RFC1036_FORMAT  =  6,  // Mon, 15 Aug 05 15:52:01 +0000
  RFC1123_FORMAT  =  7,  // Mon, 15 Aug 2005 15:52:01 +0000
  RFC2822_FORMAT  =  8,  // Mon, 15 Aug 2005 15:52:01 +0000
  RSS_FORMAT      =  9,  // Mon, 15 Aug 2005 15:52:01 EDT
  W3C_FORMAT      = 10   // 2005-08-15T15:52:01+00:00
} FORMAT;

typedef struct DATE_T *DATE;
extern size_t  DATESIZE;

enum date_arg_types
{
  DATE_FORMAT,
  DATE_STRING
};

struct date_arg
{
  enum date_arg_types type;
  union
  {
    FORMAT as_format;
    char * as_string;
  } value;
};

#define count(ARRAY) ((sizeof (ARRAY))/(sizeof *(ARRAY)))
#define new_date(...)  __new_date(count(date_args(__VA_ARGS__)), date_args(__VA_ARGS__))
#define date_args(...) ((struct date_arg []){ __VA_ARGS__ })
#define date_format(VALUE) { DATE_FORMAT, { .as_format = (VALUE) } }
#define date_string(VALUE) { DATE_STRING, { .as_string = (VALUE) } }

DATE   __new_date(size_t argc, struct date_arg *args);
char * date_get(DATE this);

#endif/*DATE_H__*/
