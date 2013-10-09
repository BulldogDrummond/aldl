#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "../aldl-io/aldl-io.h"

/*-------------- data -------------------------*/

/* specifies which config file to reference */
typedef enum config_file {
  CONFIG_DEF, /* ecm definition file */
  CONFIG_GEN  /* general serial port config and whatever else */
} config_file_t;

/* specifies both the input and output type of a field */
typedef enum config_output {
  CONFIG_STR,
  CONFIG_INT,
  CONFIG_HEX,
  CONFIG_FLAG
} config_output_t;

/*----------- functions ----------------------*/

/* loads a config file */
int load_config_file(config_file_t f, char *filepath);

/* return a normal config field */
char *get_config_opt(config_file_t f, config_output_t t, char *o);

/* allocate and fill the packet definitions */
aldl_packetdef_t *get_config_pkt(config_file_t f, aldl_commdef_t *c);

/* allocate and fill the main definition array, return a pointer */
aldl_define_t *get_config_def(config_file_t f);

/* allocate and fill the commdef array */
aldl_commdef_t *get_config_comm(config_file_t f);

#endif
