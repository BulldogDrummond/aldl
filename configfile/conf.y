%{
#include <stdio.h>
#include <stdlib.h>
#include "configfile.h"

#include "varstore.h"

#define MAX_BUFFER_LEN 128

  int buffer_length;

  char buffer[MAX_BUFFER_LEN];

  void dump_buffer() {
    int i;
    for(i=0;i<buffer_length;i++)
      printf("%x ",buffer[i]);
    printf("\n");
  }

%}



%token IDENTIFIER FLOAT NUMBER HEXNUMBER STRING TYPE

%nonassoc DEF1
%nonassoc DEF2
%nonassoc DEF3

%%
config:
  config decl '\n'
  | config stream_decl '\n'
  | config '\n'
  |
  ;

numeric:
  NUMBER  { printf("Yacc: %s\n",$1); }
  | HEXNUMBER { printf("Yacc: %s\n",$1); }
  | FLOAT
  ;

decl:
  IDENTIFIER '=' numeric     { push_simple((char *)$1,(char *)$3); }
  | IDENTIFIER '=' buffer  { printf("Found a buffer of length %d\n",buffer_length); dump_buffer(); }
  ;


buffer:
numeric numeric       { 
                         buffer_length = 2;
			 buffer[0] = (char)strtoul((const char *)$1,NULL,16);
			 buffer[1] = (char)strtoul((const char *)$2,NULL,16); }
| buffer numeric      { buffer[buffer_length] = (char)strtoul((const char *)$2,NULL,16); buffer_length++; }
  ;

stream_ident:
  NUMBER '.' IDENTIFIER
  ;

stream_decl:
  stream_ident '=' numeric ',' STRING ',' TYPE ',' STRING ',' numeric ',' numeric ',' numeric ',' numeric ',' numeric ',' numeric ',' numeric %prec DEF1
  | stream_ident '=' numeric ',' STRING ',' TYPE ',' numeric ',' numeric %prec DEF2
  | stream_ident '=' numeric %prec DEF3
  | stream_ident '=' buffer { printf("Found a buffer of length %d\n",buffer_length); dump_buffer(); }
  ;

%%

int yyerror(const char *err) {
  fprintf(stderr,"%s\n",err);
  return 0;
}
/*
void register_config_option(char *option, config_output_t type, void *ptr) {
  // TODO: search for duplicate

  if(config_stack_top < CONFIG_STACK_SIZE - 1) {
    config_stack_top++;
    config_stack[config_stack_top].identifier = option;
    config_stack[config_stack_top].type = type;
    config_stack[config_stack_top].ptr = ptr;
  } else {
    fprintf(stderr,"Error: config stack overflow\n");
  }
}
*/
/*
void push(char *identifier, char *value) {
  if(config_stack_top < CONFIG_STACK_SIZE - 1) {
    config_stack_top++;
    config_stack[config_stack_top].identifier = identifier;
    config_stack[config_stack_top].value = value;
  } else {
    fprintf(stderr,"Error: config stack overflow\n");
  }
}

int config_stack_search(const char *identifier) {
  int i;
  int res=-1;

  for(i=0;i<=config_stack_top;i++) {
    if(strncmp(identifier,config_stack[i].identifier,IDENTIFIER_MAX_LENGTH)==0) {
      res = i;
      break;
    }
  }

  return res;
}
*/
/*
int config_set_identifier_by_index(int index, char *value) {
  config_output_t type;
  int ival;

  if((index >= 0) && (index <= config_stack_top)) {
    switch(config_stack[index].type) {
    case CONFIG_STR:
      config_stack[index].ptr = value;
      break;
    case CONFIG_HEX:
      ival = strtoul(value,NULL,16);
      *((unsigned int *)config_stack[index].ptr) = ival;
      free(value);
      break;
    case CONFIG_INT:
      ival = strtol(value,NULL,10);
      free(value);
      *((int *)config_stack[index].ptr) = ival;
      break;
    case CONFIG_FLAG:
      ival = strtol(value,NULL,10);
      if((ival < 0) || (ival > 1)) {
	fprintf(stderr,"Flag out of range expected 0 or 1 got %s\n",value);
	break;
      }
      *((char *)config_stack[index].ptr) = ival;
      free(value);
      break;
    default:
      fprintf(stderr,"Unknown type with for identifier %s\n",config_stack[index].identifier);
    }
  }
  return 0;
}
*/
/*
int config_set_identifier(char *identifier, char *value) {
  int index;
  int ret;

  index = config_stack_search(identifier);
  if(index >= 0) 
    ret = config_set_identifier_by_index(index,value);
  else {
    fprintf(stderr,"Unknown identifier (%s)\n",identifier);
    ret = -1;
  }

  return ret;
  }*/
/*
int load_config_file(config_file_t f, char *filepath) {
  int fd = open(filepath,O_RDONLY);
  if(fd >= 0) {
    yyin = fd;
    yyparse();
  }
  return fd;
}
*/
int parse() {
  initialize_simple_config_stack();
  yyparse();
  return 0;
}



/*
int main(void) {
  config_stack_top = -1;

  unsigned int ecmid;
  int checksum_enable;
  unsigned int pcmaddress;

  register_config_option("ECMID",CONFIG_HEX,&ecmid);
  register_config_option("CHECKSUM_ENABLE",CONFIG_FLAG,&checksum_enable);
  register_config_option("PCMADDRESS",CONFIG_HEX,&pcmaddress);

  yyparse();

  printf("ECMID: %d\n",ecmid);
  printf("CHECKSUM_ENABLE: %d\n",checksum_enable);
  printf("PCMADDRESS: %d\n", pcmaddress);
  return 0;
}
*/
