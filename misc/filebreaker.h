
/* copy the contents of field number f in *d to dst.  returns NULL if the
   field cannot be located.  start and end define the field delimters,
   or 0 for start/end of line. */
char *brk_get_field(char *dst, char start, char end, int f, char *d);

/* case sensitive string match the first field of every line until a
   match is found, return a pointer to it, or NULL if can't be found. */
char *brk_get_line(char end, char *d, char *str);

