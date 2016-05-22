#include <stdio.h>
#include <rset.h>
#include <array.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

struct RSET_T
{
  BOOLEAN   result;
  ARRAY     group;
  char      *string;
};

size_t RSETSIZE = sizeof(struct RSET_T);

RSET
new_rset()
{
  RSET this;

  this = xcalloc(RSETSIZE, 1);
  this->group = new_array();
  return this;
}

RSET
rset_destroy(RSET this)
{
  this->group = array_destroy(this->group);
  xfree(this->string);
  xfree(this);
  this = NULL;
  return this;
}

void
rset_set_result(RSET this, BOOLEAN result) 
{
  this->result = result;
}

BOOLEAN
rset_get_result(RSET this) 
{
  return this->result;  
}

BOOLEAN
rset_has_args(RSET this) 
{
  return (array_length(this->group) > 0);
}

void 
rset_add(RSET this, const char *fmt, ...) 
{
  char    buf[4096];
  va_list args;

  memset(&buf, '\0', sizeof(buf)); 
  va_start(args, fmt);
  vsprintf(buf,  fmt, args);
  buf[strlen(buf)] = '\0';
  array_npush(this->group, buf, strlen(buf));
  va_end(args);
}

ARRAY
rset_get_group(RSET this)
{
  return this->group;
}

int 
rset_get_length(RSET this) 
{
  int i;
  int len = 0;

  for (i = 0; i < (int)array_length(this->group); i++) {
    len += strlen(array_get(this->group, i));
  }
  return len;
}

char *
rset_get_string(RSET this) 
{
  int   i; 
  int   len = 0;

  if (! this->result) return NULL;
  
  if (array_length(this->group) < 1) return NULL;

  if (this->string != NULL && strlen(this->string) > 0) return this->string;

  len = rset_get_length(this)+(int)array_length(this->group)+2;

  this->string = (char*)xmalloc(len);
  memset(this->string, '\0', len);

  for (i = 0; i < (int)array_length(this->group); i++) {
    if (i > 0)  strcat(this->string, " ");
    strcat(this->string, array_get(this->group, i));
  }

  return this->string; 
}

