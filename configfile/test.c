#include <stdio.h>
#include <stdlib.h>
#include "../aldl-io/aldlcomm.h"
#include "varstore.h"
#include "configfile.h"

// extern struct config_stack_item config_stack[CONFIG_STACK_SIZE];

void find_and_fill_simple(const char *ident, void *ptr, config_output_t type) {
  int i;
  char *tmp;

  i = find_simple_config_by_identifier(ident);
  if(i > 0) {
    tmp = get_simple_config_stack_value(i);

    switch(type) {
    case CONFIG_STR:
      *(char **)ptr = tmp;
      break;
    case CONFIG_INT:
      *(int *)ptr = strtol(tmp,NULL,10);
      break;
    case CONFIG_HEX:
      // TODO: Must be fancier
      break;
    case CONFIG_FLAG:
      *(int *)ptr = strtol(tmp,NULL,10);
      break;
    default:
      abort();
    }
  }
}

int main() {

  int i;
  char *tmp;

  parse();

  aldl_commdef_t *c = malloc(sizeof(aldl_commdef_t));
  /*
  i = find_simple_config_by_identifier("SHUTUPLENGTH");
  if(i > 0) {
    tmp = get_simple_config_stack_value(i);
    c->shutuplength = atoi(tmp);
  } else {
    fprintf(stderr,"Identifier not found\n");
  }
  */

  find_and_fill_simple("SHUTUPLENGTH",&(c->shutuplength),CONFIG_INT);

  printf("Shutup length: %d\n",c->shutuplength);

  return 0;

};
