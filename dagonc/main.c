#include <stdio.h>
#include "env.h"

extern int yyparse();
extern FILE* yyin;

int main(int argc, char *argv[]) {
  DagonEnv* env = dagon_env_new();
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
  }
  yyparse(env);
  dagon_env_destroy(env);
  return 0;
}
