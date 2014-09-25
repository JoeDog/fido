#ifndef __RULE_H
#define __RULE_H
#include <stdlib.h>
#include <joedog/defs.h>
#include <joedog/boolean.h>

/** 
 * a RULE object 
 */
typedef struct RULE_T *RULE;

/**
 * For memory allocation; RULESIZE 
 * provides the object size 
 */
extern size_t  RULESIZE;

RULE   new_rule(char *str);
RULE   rule_destroy(RULE this);
char * rule_get_property(RULE this);
char * rule_get_pattern(RULE this);



#endif/*__RULE_H*/
