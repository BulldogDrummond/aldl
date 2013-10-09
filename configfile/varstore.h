void initialize_simple_config_stack();
void push_simple(char *identifier, char *value);
char *get_simple_config_stack_identifier(int index);
char *get_simple_config_stack_value(int index);
int find_simple_config_by_identifier(const char *ident);
void *memdup(void *ptr, size_t len);
