%{
  #include <stdio.h>
  extern int yylex();
  extern int yylineno;
  void yyerror(const char *error) {
    fprintf(stderr, "Parsing error on line %d: %s", yylineno, error);
  }
  #define YYDEBUG 1
  #define YYVERBOSE 1
%}

%right ASSIGN ARRAY_ASSIGN
%left DOT
%left MULT
%left PLUS MINUS
%nonassoc LT EQL

%token DIGIT PLUS CLASS INDENT DEDENT COLON NEWLINE
%token ID CONSTANT STRING DOT LPAREN RPAREN LBRACE
%token RBRACE AT MULT MINUS LT EQL NOP COMMA ASSIGN
%token WHILE CASE ARRAY_ASSIGN IF ELSE

%error-verbose

%%

program: statements opt_newline

statements: statements NEWLINE statement
          | statement

statement: expression
         | class_definition
         | while_statement
         | case_statement
         | if_statement
         | array_assignment
         | assignment

class_definition: CLASS COLON function_block

function_block: NEWLINE INDENT function_definitions NEWLINE DEDENT

function_definitions: function_definitions NEWLINE function_definition
                    | function_definition

function_definition: ID LPAREN arglist RPAREN COLON block
                   | ID COLON block

while_statement: WHILE expression block

case_statement: CASE expression case_block

case_block: NEWLINE INDENT cases NEWLINE DEDENT

cases: cases NEWLINE case
     | case

case: expression block
    | NOP block

if_statement: IF expression block NEWLINE ELSE block

expression: binary_operation
          | primary_value

primary_value: value
             | method_call
             | top_level_method_call
             | object_initialize
             | LPAREN expression RPAREN

binary_operation: expression PLUS expression
                | expression MULT expression
                | expression LT expression
                | expression MINUS expression
                | expression EQL expression

assignment: variable ASSIGN expression

array_assignment: primary_value LBRACE expression ARRAY_ASSIGN expression

method_call: primary_value DOT ID
           | primary_value DOT ID LPAREN arglist RPAREN

top_level_method_call: ID LPAREN arglist RPAREN

object_initialize: CLASS LPAREN arglist RPAREN

arglist: /* Empty List */
       | arglist COMMA expression
       | expression

value: number
     | variable
     | array
     | array_value
     | STRING
     | CONSTANT

array: LBRACE array_list RBRACE

array_list: /* Empty List */
          | array_list COMMA expression
          | expression

array_value: primary_value LBRACE expression RBRACE

variable: ID
        | AT ID

number: DIGIT

block: NEWLINE INDENT statements NEWLINE DEDENT

opt_newline: /* No Newline */
           | NEWLINE
