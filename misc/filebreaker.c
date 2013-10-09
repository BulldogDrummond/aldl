#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filebreaker.h"

/* end of line delimters */
#define EOL '\n'
#define DOSEOL '\r'

/* check if a char marks the end of line or file.  returns 1 if EOL,
   2 if NULL or EOF.  otherwise 0 */
inline int brk_eol(char c);

inline int brk_eol(char c) {
  if(c == EOL || c == DOSEOL ) return 1;
  if(c == EOF || c ==  0 ) return 2;
  return 0;
};

char *brk_get_field(char *dst, char start, char end, int f, char *d) {
  if(dst == NULL || d == NULL) return NULL;
  register int i=0,x=0;
  if(start > 0) {
    while(x <= f) {
      while(d[i] != start) {
        if(brk_eol(d[i]) > 0) return NULL;
        i++;
      };
      x++; i++;
    };
  };
  register int e=i;
  if(end == 0) {
    while(brk_eol(d[e]) == 0) e++;
  } else {
    while(d[e] != end) {
      if(brk_eol(d[e]) > 0) return NULL;
      e++;
    };
  };
  strncpy(dst,d+i,e-i); dst[e-i]=0;
  return dst; 
};

char *brk_get_line(char end, char *d, char *str) {
  if(str == NULL || d == NULL || end == 0) return NULL;
  int i=0,x=0;
  int adv = 0;
  while(brk_eol(d[i]) != 2) {
    if(d[i] == str[x]) {
      if(d[i+1] == end) {
        if(str[x+1] == 0) {
          return d + (i - x);
        } else adv = 1;
      }; /* advance */
    } else adv = 1;
    if(adv == 1) {
      while(brk_eol(d[i]) == 0) i++;
      x = 0; adv=0; i++;
    } else {
      i++; x++;
    };
  };
  return NULL;
};
