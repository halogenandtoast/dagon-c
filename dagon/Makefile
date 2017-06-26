all: dagon

dagon: main.c lex.yy.c parse.tab.c node.c env.c string.c integer.c array.c io.c object.c
	cc -ggdb -o $@ $^

lex.yy.c: lex.l parse.tab.h
	flex lex.l

parse.tab.c parse.tab.h: parse.y
	bison -d -v parse.y

clean:
	rm -rf dagonc lex.yy.c parse.output parse.tab.c parse.tab.h dagon.dSYM
