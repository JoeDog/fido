#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <array.h>

struct ARRAY_T
{ 
  int     size;
  int     index;
  int     capacity;
  void    **data;
  //ARRAY   next;
};

/**
 * private functions
 */
static void __expand(ARRAY this, int min); 

ARRAY 
new_array()
{
  ARRAY this;
  this           = calloc(sizeof(*this), 1);
  this->size     = 0;
  this->index    = 0;
  this->capacity = 0;
  this->data     = NULL;
  return this;
}

ARRAY 
array_destroy(ARRAY this)
{
  int x;

  if (this == NULL) {
    return this;
  }
  for (x = 0; x < this->size; x++) {
    free(this->data[x]); 
  }  
  free(this->data);
  free(this);
  return NULL;
}

int
array_length(ARRAY this)
{
  return this->size;
}

void 
array_push(ARRAY this, void *thing)
{
  if (this->size >= this->capacity) {
    __expand(this, this->size + 1);
  }
  this->data[this->size++] = thing;
}

void 
array_insert(ARRAY this, void *thing, int index)
{
  if (this->size >= this->capacity)
    __expand(this, this->size + 1);
  if (index > this->size)
    index = this->size;
  if (index < this->size)
    memmove(&this->data[index + 1], &this->data[index], (this->size - index) * sizeof(void *));
  this->data[index] = thing;
  this->size++;
}

void *
array_get(ARRAY this, int index)
{
  return this->data[index];
}

void *
array_next(ARRAY this) 
{
  void *thing = this->data[this->index];
  this->index = (this->index == this->size - 1) ? 0 : this->index + 1;
  return thing;
}

static void
__expand(ARRAY this, int min)
{
  int       delta;
  const int increment = 16;
 
  if (this->capacity > min) {
    puts("we have the capacity");
    return; // we have the capacity
  }

  delta  = min;
  delta += increment - 1;
  delta /= increment;
  delta *= increment;
  delta = (delta < 1) ? 1 : delta;
  this->capacity += delta;
  this->data = (this->data != NULL) ?
               realloc(this->data, this->capacity * sizeof(*this->data)) :
               malloc(this->capacity * sizeof(*this->data));
  memset(this->data + this->size, 0, (this->capacity - this->size) * sizeof(void *));  
}

#if 0

static const char *matches[] = {
  "ABOUT: .*about",  
  "FROM: .*jeff@joedog.org$",  
  "TO: .*jdfulmer@armstrong.com",  
  "OHMY: .*ah hoo!.*",  
  "DZAE: .*whoohoo!.*",  
  "Z22AD: .*z22ad.*"
};

#include <rule.h>

int
array_test()
{
  int   x;
  ARRAY A = new_array();

  for (x = 0; x < 6; x ++) {
    RULE R = new_rule(matches[x]);
    array_push(A, R);
  }

  x = 0;
  while (x < 1000) {
    RULE R = array_next(A);
    printf("LABEL: (%s), RULE: (%s)\n", rule_get_property(R), rule_get_pattern(R));
    if (x % 25 == 0){
      array_insert(A, new_rule("HAHA: .*thisIsIt$"), 3);
    }
    x++;
  }
  A = array_destroy(A);

  return 0;
}

int
main()
{
  int x;
  for( x = 0; x < 5; x++ )
    array_test(); 
  return 0;
}
#endif
