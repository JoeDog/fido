#include <date.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#if 0
  ATOM_FORMAT     =  0;  // 2010-05-21T13:33:59-0400
  COOKIE_FORMAT   =  1;  // Monday, 15-Aug-05 15:52:01 UTC
  ISO8601_FORMAT  =  2;  // 2010-05-21T13:33:59-0400
  SYSLOG_FORMAT   =  3;  // May 16 04:02:50
  RFC822_FORMAT   =  4;  // Mon, 15 Aug 05 15:52:01 +0000
  RFC850_FORMAT   =  5;  // Monday, 15-Aug-05 15:52:01 UTC
  RFC1036_FORMAT  =  6;  // Mon, 15 Aug 05 15:52:01 +0000
  RFC1123_FORMAT  =  7;  // Mon, 15 Aug 2005 15:52:01 +0000
  RFC2822_FORMAT  =  8;  // Mon, 15 Aug 2005 15:52:01 +0000
  RSS_FORMAT      =  9;  // Mon, 15 Aug 2005 15:52:01 EDT
  W3C_FORMAT      = 10;  // 2005-08-15T15:52:01+00:00
#endif

struct Formats {
  FORMAT id;
  char * format;
};

static const struct Formats fmap[] = {
  {ATOM_FORMAT,      "%Y-%m-%dT%H:%M:%S%z"},
  {COOKIE_FORMAT,    "%A, %d-%b-%Y %H:%M:%S UTC"},
  {ISO8601_FORMAT,   "%Y-%m-%dT%H:%M:%S%z"},
  {SYSLOG_FORMAT,    "%b %d %H:%M:%S"},
  {RFC822_FORMAT,    "%D, %d %m %Y %H:%M:%S %T"},
  {RFC850_FORMAT,    "%l, %d-%m-%y %H:%M:%S %T"},
  {RFC1036_FORMAT,   "%l, %d-%m-%y %H:%M:%S %T"},
  {RFC1123_FORMAT,   "%a, %d %b %Y %H:%M:%S GMT"},
  {RFC2822_FORMAT,   "%D, %d %m %Y %H:%M:%S %O"},
  {RSS_FORMAT,       "%D, %d %m %Y %H:%M:%S %T"},
  {W3C_FORMAT,       "%Y-%m-%dT%H:%M:%S%SO"}
};

enum assume 
{
  DATE_MDAY,
  DATE_YEAR,
  DATE_TIME
};

const char * const wday[] = 
{
  "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

const char * const weekday[] = 
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

const char * const month[] = 
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

struct tzinfo 
{
  const char *name;
  int offset; /* +/- in minutes */
};

#define tDAYZONE -60          /* offset for daylight savings time */
static const struct tzinfo tz[]= {
  {"GMT",    0},              /* Greenwich Mean */
  {"UTC",    0},              /* Universal (Coordinated) */
  {"WET",    0},              /* Western European */
  {"BST",    0 tDAYZONE},     /* British Summer */
  {"WAT",   60},              /* West Africa */
  {"AST",   240},             /* Atlantic Standard */
  {"ADT",   240 tDAYZONE},    /* Atlantic Daylight */
  {"EST",   300},             /* Eastern Standard */
  {"EDT",   300 tDAYZONE},    /* Eastern Daylight */
  {"CST",   360},             /* Central Standard */
  {"CDT",   360 tDAYZONE},    /* Central Daylight */
  {"MST",   420},             /* Mountain Standard */
  {"MDT",   420 tDAYZONE},    /* Mountain Daylight */
  {"PST",   480},             /* Pacific Standard */
  {"PDT",   480 tDAYZONE},    /* Pacific Daylight */
  {"YST",   540},             /* Yukon Standard */
  {"YDT",   540 tDAYZONE},    /* Yukon Daylight */
  {"HST",   600},             /* Hawaii Standard */
  {"HDT",   600 tDAYZONE},    /* Hawaii Daylight */
  {"CAT",   600},             /* Central Alaska */
  {"AHST",  600},             /* Alaska-Hawaii Standard */
  {"NT",    660},             /* Nome */
  {"IDLW",  720},             /* International Date Line West */
  {"CET",   -60},             /* Central European */
  {"MET",   -60},             /* Middle European */
  {"MEWT",  -60},             /* Middle European Winter */
  {"MEST",  -60 tDAYZONE},    /* Middle European Summer */
  {"CEST",  -60 tDAYZONE},    /* Central European Summer */
  {"MESZ",  -60 tDAYZONE},    /* Middle European Summer */
  {"FWT",   -60},             /* French Winter */
  {"FST",   -60 tDAYZONE},    /* French Summer */
  {"EET",  -120},             /* Eastern Europe, USSR Zone 1 */
  {"WAST", -420},             /* West Australian Standard */
  {"WADT", -420 tDAYZONE},    /* West Australian Daylight */
  {"CCT",  -480},             /* China Coast, USSR Zone 7 */
  {"JST",  -540},             /* Japan Standard, USSR Zone 8 */
  {"EAST", -600},             /* Eastern Australian Standard */
  {"EADT", -600 tDAYZONE},    /* Eastern Australian Daylight */
  {"GST",  -600},             /* Guam Standard, USSR Zone 9 */
  {"NZT",  -720},             /* New Zealand */
  {"NZST", -720},             /* New Zealand Standard */
  {"NZDT", -720 tDAYZONE},    /* New Zealand Daylight */
  {"IDLE", -720},             /* International Date Line East */
};


struct DATE_T
{
  int format;
};

size_t DATESIZE = sizeof(struct DATE_T);

DATE
__new_date(size_t argc, struct date_arg * args)
{
  DATE   this;
  size_t i;

  this = calloc(1,  DATESIZE);
  this->format = -1;
  for (i = 0; i < argc; i++) {
    switch(args[i].type) {
    case DATE_FORMAT:
      this->format = args[i].value.as_format;
      break;
    case DATE_STRING:
      puts(args[i].value.as_string);
      break;
    }
  }
  if (this->format == -1) 
    this->format = SYSLOG_FORMAT;
  return this;
}

char *
date_get(DATE this)
{
#define DATESZ 128  
  time_t now;
  char * date;
  date = xmalloc(DATESZ+1);
  memset(date, '\0', DATESZ+1);
   
  now = time(NULL);
  strftime(date, DATESZ, fmap[this->format].format, localtime(&now));
  return date;
}

/**
 * Copyright (C) 1998 - 2006, Daniel Stenberg, <daniel@haxx.se>, et al.  
 * (modified - use the original: http://curl.haxx.se/)
 */
#if 0
static int checktz(char *check)
{
  unsigned int i;
  const struct tzinfo *what;
  BOOLEAN found = FALSE;

  what = tz;
  for(i=0; i< sizeof(tz)/sizeof(tz[0]); i++) {
    if(__strmatch(check, (char*)what->name)) {
      found=TRUE;
      break;
    }
    what++;
  }
  return found ? what->offset*60 : -1;
}

/**
 * Copyright (C) 1998 - 2006, Daniel Stenberg, <daniel@haxx.se>, et al.  
 */
static void skip(const char **date) {
  /* skip everything that aren't letters or digits */
  while(**date && !isalnum((unsigned char)**date))
    (*date)++;
}
#endif

#if 0
int
main() 
{
  DATE d1 = new_date();
  DATE d2 = new_date(date_format(SYSLOG));
  DATE d3 = new_date(date_string("5-24-2010"), date_format(RFC850));
  printf("%s\n", date_get(d1));
  printf("%s\n", date_get(d2));
  printf("%s\n", date_get(d3));
  return 0;
}
#endif


