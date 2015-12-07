all: dagon

dagon: main.c
	cc -o $@ $<
