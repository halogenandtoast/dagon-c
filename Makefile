all: dagonc

dagonc: main.c lex.yy.c parse.tab.c
	cc -o $@ $^

lex.yy.c: lex.l parse.tab.h
	flex lex.l

parse.tab.c parse.tab.h: parse.y
	bison -d -v -t parse.y

clean:
	rm -rf dagonc lex.yy.c
