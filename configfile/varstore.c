#include <stdlib.h>

#define STACK_SIZE 512
#define IDENTIFIER_MAX_LEN 512

#ifndef NULL
#define NULL 0
#endif

struct simple_config {
  char *ident;
  char *value;
  char buffer_flag;
};


struct simple_config *simple_config_stack[STACK_SIZE];
int simple_config_stack_top;


void initialize_simple_config_stack() {
  simple_config_stack_top = -1;
}

void *memdup(void *ptr, size_t len) {
  void *mem;
  int i;

  mem = malloc(len);
  if(mem)
    for(i=0;i<len;i++)
      ((unsigned char *)mem)[i] = ((unsigned char *)ptr)[i];

  return mem;
}

void push_simple(char *identifier, char *value) {
  if(simple_config_stack_top + 1 < STACK_SIZE) {
    simple_config_stack_top++;
    simple_config_stack[simple_config_stack_top] = (struct simple_config *)malloc(sizeof(struct simple_config));
    if(simple_config_stack[simple_config_stack_top] != NULL) {
      simple_config_stack[simple_config_stack_top]->ident = identifier;
      simple_config_stack[simple_config_stack_top]->value = value;
      simple_config_stack[simple_config_stack_top]->buffer_flag = 0;
    }
  }
}

char *get_simple_config_stack_identifier(int index) {
  char *ret = NULL;
  if((index >= 0) && (index <= simple_config_stack_top))
    if(!simple_config_stack[index]->buffer_flag)
      ret = simple_config_stack[index]->ident;
  return ret;
}

char *get_simple_config_stack_value(int index) {
  char *ret = NULL;
  if((index >= 0) && (index <= simple_config_stack_top))
    if(!simple_config_stack[index]->buffer_flag)
      ret = simple_config_stack[index]->value;
  return ret;
}

int find_simple_config_by_identifier(const char *ident) {
  int result=-1;
  int i;
  if(simple_config_stack_top >= 0)
    for(i=0;i<=simple_config_stack_top;i++)
      if(!simple_config_stack[i]->buffer_flag && strncmp(simple_config_stack[i]->ident,ident,IDENTIFIER_MAX_LEN)==0) {
	result = i;
	break;
      }

  return result;
}
