typedef struct _dfile_t {
  unsigned int n; /* number of parameters */
  char **p;  /* parameter */
  char **v;  /* value */
} dfile_t;

/* loads file, strips quotes, shrinks, parses in one step.. */
dfile_t *dfile_load(char *filename);

/* read file into memory */
char *load_file(char *filename);

/* split up data into parameters and values */
dfile_t *dfile(char *data);

/* reduce the data section and pointer arrays to reduce mem usage, returns
   pointer to new data to be freed later. */
char *dfile_shrink(dfile_t *d);

/* get a value by parameter string */
char *value_by_parameter(char *str, dfile_t *d);

/* a fast string compare function, returns 1 on match 0 on fail, bails instantly
   on mismatch. */
inline int faststrcmp(char *a, char *b);

/* strip quotes from quoted 'value' fields. */
void dfile_strip_quotes(dfile_t *d);

/* for debugging, print a list of all config pairs extracted. */
void print_config(dfile_t *d);
