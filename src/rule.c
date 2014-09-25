#include <rule.h>
#include <util.h>
#include <regex.h>
#include <string.h>
#include <joedog/joedog.h>
#include <joedog/boolean.h>
#include <joedog/defs.h>

struct RULE_T
{
  int       id;
  char *    property;
  char *    pattern;
};

size_t RULESIZE = sizeof(struct RULE_T);

struct pair {
  int  num;
  char *label;
  char *match;
};

private char *      __decolon(char *str);
private BOOLEAN     __add(RULE this, char *str);
private struct pair __get_pair(char *string);

RULE
new_rule(char *str)
{
  RULE this;

  this = xcalloc(RULESIZE, 1);
  this->property = NULL;
  this->pattern  = NULL;
  if (! __add(this, str)) {
    this = rule_destroy(this);
  }
  return this;
}

RULE
rule_destroy(RULE this)
{
  xfree(this->property);
  xfree(this->pattern);
  xfree(this);
  this = NULL;
  return this;
}

char *
rule_get_property(RULE this)
{
  return this->property;
}

char *
rule_get_pattern(RULE this)
{
  return this->pattern;
}

private BOOLEAN
__add(RULE this, char *str) 
{
  struct pair tmp;
  char * pattern;
  char delim[] = "/|!;:";
  
  tmp = __get_pair(str);
  if (tmp.num == 2) {
    // we have a label
    tmp.match = trim(tmp.match);
    pattern = strtok(tmp.match, delim);
    if (tmp.label != NULL) {
      this->property = xstrdup(__decolon(tmp.label));
    }
    if (pattern != NULL) {
      this->pattern  = xstrdup(pattern);
    }
    xfree(tmp.label);
    xfree(tmp.match);
    return TRUE;
  } else {
    // no label
    this->property = NULL;
    pattern        = strtok(str, delim);
    this->pattern  = xstrdup(pattern); 
    return TRUE;
  } 
  // we reserve the right to return FALSE
  // in the future - for now we'll never get
  // here...
  return FALSE;
}

private struct pair
__get_pair(char *string)
{
  struct pair p;
  regex_t    preg;
  char       *pattern = "([A-Z0-9_-]+:)(.+)";
  int        rc;
  size_t     nmatch = 3;
  regmatch_t pmatch[3];

  p.num   = 1;
  p.label = NULL;
  p.match = NULL;

  if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
    return p;
  }

  if ((rc = regexec(&preg, string, nmatch, pmatch, REG_EXTENDED)) == 0) {
    p.num = preg.re_nsub;
    if (p.num == 2) {
      int llen;
      int mlen;
      llen    = pmatch[1].rm_eo - pmatch[1].rm_so;
      p.label = malloc(llen+1);
      memset(p.label, '\0', llen+1);
      snprintf(p.label, llen, "%.*s", pmatch[1].rm_eo - pmatch[1].rm_so, &string[pmatch[1].rm_so]);
      mlen    = strlen(&string[pmatch[2].rm_so]);
      p.match = malloc(mlen+1);
      memset(p.match, '\0', mlen+1);
      snprintf(p.match, mlen+1, "%.*s", pmatch[2].rm_eo - pmatch[2].rm_so, &string[pmatch[2].rm_so]);
    }
  }
  regfree(&preg);

  return p;
}

private char *
__decolon(char *str)
{
  int   i;
  int   len;

  len = strlen(str);
  for (i = len-1; i >= 0 && (str[i]==' ' || str[i]==':' || str[i]=='\t'); i--) {
    str[i] = '\0';
  }
  return str;
}
