#include <stdio.h>
extern int yyparse();
extern FILE* yyin;
extern int yydebug;

int main(int argc, char *argv[]) {
  yydebug = 1;
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
  }
  yyparse();
  return 0;
}
