%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include "node.h"
  #include "dagon.h"
  #include "parse.tab.h"

  static int current_indent = 0;
  static int line_indent = 0;
  static int override = 0;
  static int newline = 0;
  static int newline_comment = 0;

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
%x string
%x comment
%s dstring

%%

<comment>"\n" { BEGIN INITIAL; }
<comment>.

<string>"#{" { BEGIN dstring; return DSTRING_BEGIN; }
<string>([^"#]|\\.)* { yylval.sval = strdup(yytext); return STRING_PART; }
<string>"\"" { BEGIN INITIAL; }
<dstring>"}" { BEGIN string; return DSTRING_END; }

<indent>"\n"+ { if(!override) line_indent = 0; override = 0; if(newline == 0) { newline = 1; return NEWLINE; } }
<indent>"  " {  line_indent++; }
<indent>. {
  newline = 0;
  newline_comment = 1;
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

"#" { if (newline_comment == 1) { newline = 1; newline_comment = 0; }; BEGIN comment; }
"while " { return WHILE; }
"case " { return CASE; }
"if " { return IF; }
"else" { return ELSE; }
"\n"+ { line_indent = 0; BEGIN indent; if(newline == 0) { newline = 1; return NEWLINE; } }
[0-9]+ { yylval.ival = atoi(yytext); return DIGIT; }
([A-Z][A-Z]*[a-z]+)+ { yylval.sval = strdup(yytext); return CLASS; }
[A-Z][A-Z]+ { yylval.sval = strdup(yytext); return CONSTANT; }
[a-z]+[a-z-]*\?? { yylval.sval = strdup(yytext); return ID; }
"\"" { BEGIN string; }
"->" { return ARROW; }
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
