/* dfiler creates a structure of pointers and adds null terminators to a buf.
   it relies on a single symbol: =
   anything to the left of a = sign, until whitespace (or linefeed) is
   encountered, is stored in p[x] for parameter.
   anything to the right of a = sign, until whitespace is encountered, is stored
   in v[x] for value.
   this means that comments need not have a prefix, as long as they have no
   = contained in them.
*/

typedef struct _dfile_t {
  unsigned int n; /* number of parameters */
  char **p;  /* parameter */
  char **v;  /* value */
} dfile_t;

/* read file into memory */
char *load_file(char *filename);

/* split up data */
dfile_t *dfilefile(char *data);

/* reduce the data section and pointer arrays to reduce mem usage */
char *dfile_shrink(dfile_t *d);

