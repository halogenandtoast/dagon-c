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

  int is_operator(const char *token) {
    return strlen(token) == 1 && index("+*-><=", token[0]) > 0;
  }

  void check_token(const char *token) {
    if (is_operator(token)) {
      fatal_error("Unknown token. Perhaps you meant to put spaces around %s\n", yytext);
    }
  }
%}

%option noyywrap

%%

[0-9]+ { TOKEN(DIGIT); }
([A-Z][a-z]+)+ { TOKEN(CLASS); }
[A-Z][A-Z]+ { TOKEN(CONSTANT); }
[a-z]+([a-z-]+) { TOKEN(ID); }
\"[^"]*\" { TOKEN(STRING); }
"." { TOKEN(DOT); }
":" { TOKEN(COLON); }
"(" { TOKEN(LPAREN); }
")" { TOKEN(RPAREN); }
"[" { TOKEN(LBRACE); }
"]" { TOKEN(RBRACE); }
"@" { TOKEN(AT); }
" + " { TOKEN(PLUS); }
" * " { TOKEN(MULT); }
" - " { TOKEN(MINUS); }
" < " { TOKEN(LT); }
" = " { TOKEN(EQUAL); }
"_" { TOKEN(NOP); }
[ \n\t]
. { check_token(yytext); fatal_error("Unknown token: %s\n", yytext); }
