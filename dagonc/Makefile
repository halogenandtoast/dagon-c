all: dagonc

dagonc: main.c lex.yy.c parse.tab.c
	cc -o $@ $^

lex.yy.c: lex.l parse.tab.h
	flex lex.l

parse.tab.c parse.tab.h: parse.y
	bison -d parse.y

clean:
	rm -rf dagonc lex.yy.c parse.output parse.tab.c parse.tab.h
