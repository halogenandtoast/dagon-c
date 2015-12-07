all: dagon

dagon: main.c lex.yy.c
	cc -o $@ $^

lex.yy.c: lex.l
	flex lex.l
