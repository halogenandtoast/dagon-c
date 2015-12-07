%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #define TOKEN(TYPE) printf("%s\n", #TYPE)

  void fatal_error(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    exit(EXIT_FAILURE);
  }

  void check_token(const char *token) {
    if (strcmp(token, "+") == 0) {
      fatal_error("Unknown token. Perhaps you meant to put spaces around %s\n", yytext);
    }
  }
%}

%option noyywrap

%%

[0-9]+ { TOKEN(DIGIT); }
" + " { TOKEN(PLUS); }
[ \n\t]
. { check_token(yytext); fatal_error("Unknown token: %s\n", yytext); }
