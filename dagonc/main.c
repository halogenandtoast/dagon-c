#include <stdio.h>
extern int yyparse();
extern FILE* yyin;

int main(int argc, char *argv[]) {
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
  }
  yyparse();
  return 0;
}
