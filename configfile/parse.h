#define IDENTIFIER_MAX_LENGTH 512

#ifndef PARSE_H
#define PARSE_H 1


#define CONFIG_STACK_SIZE 512
struct config_stack_item {
  char *identifier;
  char *value;
};



int config_stack_search(const char *identifier);

extern struct config_stack_item config_stack[CONFIG_STACK_SIZE];

int parse();


#endif
