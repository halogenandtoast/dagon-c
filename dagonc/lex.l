%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include "node.h"
  #include "parse.tab.h"

  static int current_indent = 0;
  static int line_indent = 0;
  static int override = 0;

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

%option noyywrap yylineno

%s start
%x indent

%%

<indent>"\n"+ { if(!override) line_indent = 0; override = 0; return NEWLINE; }
<indent>"  " {  line_indent++; }
<indent>. {
  unput(*yytext);
  if (line_indent > current_indent) {
    current_indent++;
    return INDENT;
  } else if (line_indent < current_indent) {
    current_indent--;
    unput('\n');
    override = 1;
    return DEDENT;
  } else {
    override = 0;
    BEGIN INITIAL;
  }
}

"while " { return WHILE; }
"case " { return CASE; }
"if " { return IF; }
"else" { return ELSE; }
"\n"+ { line_indent = 0; BEGIN indent; return NEWLINE; }
[0-9]+ { yylval.ival = atoi(yytext); return DIGIT; }
([A-Z][a-z]+)+ { yylval.sval = strdup(yytext); return CLASS; }
[A-Z][A-Z]+ { yylval.sval = strdup(yytext); return CONSTANT; }
[a-z]+[a-z-]* { yylval.sval = strdup(yytext); return ID; }
\"[^"]*\" { yylval.sval = strndup(yytext + 1, yyleng - 2); return STRING; }
"." { return DOT; }
": " { return ASSIGN; }
":" { return COLON; }
", " { return COMMA; }
"(" { return LPAREN; }
")" { return RPAREN; }
"[" { return LBRACE; }
"]: " { return ARRAY_ASSIGN; }
"]" { return RBRACE; }
"@" { return AT; }
" + " { return PLUS; }
" * " { return MULT; }
" - " { return MINUS; }
" < " { return LT; }
" = " { return EQL; }
"_" { return NOP; }
"-" { return NEGATE; }
. { check_token(yytext); fatal_error("Unknown token: '%s'\n", yytext); }
