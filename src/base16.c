/**
 * Fido - a log file watcher
 *
 * Copyright (C) 2011-2012 by  
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <base16.h>

#define HEX2VALUE(x) ((x)>='0'&&(x)<='9'?(x)-'0' : \
                ((x)>='a'&&(x)<='f'?(x)-'a'+10:    \
                ((x)>='A'&&(x)<='F'?(x)-'A'+10:0)))

//static const char table[20] = "0123456789abcdef";

static const char hexa[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

char *
base16_encode(char *src)
{
  int i,j,x1,x2;
  int   len;
  char *dst;
        
  len = (strlen(src) * 2);
  
  dst = malloc(len + 1);

  if (!dst) {
    return NULL;
  }               
  
  memset(dst, '\0', len+1);

  for (i = 0,j = 0; i < (len/2); i++) {
    x1=((unsigned char)src[i])/16;
    x2=((unsigned char)src[i])%16;
    dst[j++] = hexa[x1];
    dst[j++] = hexa[x2];
  }
  dst[len] = '\0';
  return dst;
}

char *
base16_decode(char *src) 
{ 
  int i, j;   
  int   len;
  char *ret;
  unsigned char x1,x2;

  len = strlen(src) / 2;
  ret = malloc(len+1);

  if (! ret) {
    return NULL;     
  }

  memset(ret, '\0', len+1);

  for ( i=0,j=0; j < len; i+=2,j++ ) {
    x1 = (unsigned char) src[i];
    x2 = (unsigned char) src[i+1];
    ret[j]=(unsigned char)(HEX2VALUE(x1)*16+HEX2VALUE(x2));
  }
  return ret;
}

#if 0
int main()
{
  char *tmp = base16_encode("/home/jdfulmer/haha.txt", 23);
  puts(tmp);
  tmp = base16_decode(tmp);
  puts(tmp);
  return 0;
}
#endif
