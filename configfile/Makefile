all: clean y.tab.o lex.yy.o varstore.o
conf:y.tab.o lex.yy.o varstore.o
	gcc $(CFLAGS) -o conf test.c lex.yy.o y.tab.o varstore.o
y.tab.o:y.tab.c
	gcc $(CFLAGS) -c $<
lex.yy.o:y.tab.o lex.yy.c
	gcc $(CFLAGS) -c lex.yy.c
y.tab.c:
	yacc -d conf.y
lex.yy.c:
	lex conf.lex
%.o:%.c
	gcc $(CFLAGS) -c $<

.PHONY:clean
clean:
	rm -f *.o y.tab.c lex.yy.c conf
