%{
  #include <stdio.h>
  #include <stdarg.h>
  #include <string.h>
  #include "node.h"
  #include "dagon.h"
  #include "env.h"
  #define YYDEBUG 1

  extern int yylex();
  extern int yylineno;
  extern void fatal_error(const char *msg, ...);
  void yyerror(DagonEnv* env, const char *error) {
    fatal_error("Parsing error on line %d: %s\n", yylineno, error);
  }
%}

%right ASSIGN ARRAY_ASSIGN
%left DOT
%left MULT
%left PLUS MINUS
%nonassoc LT EQL

%union {
  int ival;
  char *sval;
  Node* node;
}

%token PLUS INDENT DEDENT COLON NEWLINE ARROW
%token DOT LPAREN RPAREN LBRACE
%token RBRACE AT MULT MINUS LT EQL NOP COMMA ASSIGN
%token WHILE CASE ARRAY_ASSIGN IF ELSE NEGATE
%token DSTRING_BEGIN DSTRING_END
%token<ival> DIGIT
%token<sval> CLASS ID CONSTANT STRING_PART

%type<node> statements statement expression class_definition while_statement
%type<node> case_statement if_statement array_assignment assignment
%type<node> method_block method_definitions method_definition
%type<node> block case_block cases case primary_value binary_operation
%type<node> method_call scoped_method_call object_initialize arglist
%type<node> value array array_list array_value variable number string
%type<node> dstring combined_string

%error-verbose
%parse-param { DagonEnv* env }

%%

program: opt_newlines statements opt_newlines { dagon_run(env, $2); dagon_free_node($2); }

statements: statements NEWLINE opt_newlines statement { dagon_list_node_append($1, $4); }
          | statement { $$ = dagon_list_node_new($1); }

statement: expression
         | class_definition
         | while_statement
         | case_statement
         | if_statement
         | array_assignment
         | assignment

class_definition: CLASS COLON method_block { $$ = dagon_class_definition_node_new($1, $3); }

method_block: NEWLINE INDENT method_definitions NEWLINE DEDENT { $$ = $3; }

method_definitions: method_definitions NEWLINE method_definition { dagon_list_node_append($1, $3); }
                    | method_definition { $$ = dagon_list_node_new($1); }

method_definition: ID LPAREN arglist RPAREN COLON block { $$ = dagon_method_definition_node_new($1, $3, $6); }
                   | ID COLON block { $$ = dagon_method_definition_node_new($1, dagon_list_node_new(NULL), $3); }

while_statement: WHILE expression block { $$ = dagon_while_statement_node_new($2, $3); }

case_statement: CASE expression case_block { $$ = dagon_case_statement_node_new($2, $3); }

case_block: NEWLINE INDENT cases NEWLINE DEDENT { $$ = $3; }

cases: cases NEWLINE case { dagon_list_node_append($1, $3); }
     | case { $$ = dagon_list_node_new($1); }

case: expression block { $$ = dagon_case_node_new($1, $2); }
    | NOP block { $$ = dagon_catchall_case_node_new($2); }

if_statement: IF expression block NEWLINE ELSE block { $$ = dagon_if_statement_node_new($2, $3, $6); }

expression: binary_operation
          | primary_value

primary_value: value
             | method_call
             | scoped_method_call
             | object_initialize
             | LPAREN expression RPAREN { $$ = $2; }

binary_operation: expression PLUS expression { $$ = dagon_method_call_node_new($1, "+", dagon_list_node_new($3)); }
                | expression MULT expression { $$ = dagon_method_call_node_new($1, "*", dagon_list_node_new($3)); }
                | expression LT expression { $$ = dagon_method_call_node_new($1, "<", dagon_list_node_new($3)); }
                | expression MINUS expression { $$ = dagon_method_call_node_new($1, "-", dagon_list_node_new($3)); }
                | expression EQL expression { $$ = dagon_method_call_node_new($1, "=", dagon_list_node_new($3)); }

assignment: variable ASSIGN expression { $$ = dagon_assignment_node_new($1, $3); }

array_assignment: primary_value LBRACE expression ARRAY_ASSIGN expression {
                Node* args = dagon_list_node_new($3);
                dagon_list_node_append(args, $5);
                $$ = dagon_method_call_node_new($1, "[]=", args);
}

method_call: primary_value DOT ID { $$ = dagon_method_call_node_new($1, $3, dagon_list_node_new(NULL)); }
           | primary_value DOT ID LPAREN arglist RPAREN { $$ = dagon_method_call_node_new($1, $3, $5); }

scoped_method_call: ID LPAREN arglist RPAREN { $$ = dagon_scoped_method_call_node_new($1, $3); }

object_initialize: CLASS LPAREN arglist RPAREN { $$ = dagon_object_initialize_node_new($1, $3); }

arglist: /* Empty List */ { $$ = dagon_list_node_new(NULL); }
       | arglist COMMA expression { dagon_list_node_append($1, $3); }
       | expression { $$ = dagon_list_node_new($1); }

value: number
     | variable
     | array
     | array_value
     | combined_string
     | CONSTANT { $$ = dagon_constant_node_new($1); }

array: LBRACE array_list RBRACE { $$ = dagon_array_node_new($2); }

array_list: /* Empty List */ { $$ = dagon_list_node_new(NULL); }
          | array_list COMMA expression { dagon_list_node_append($1, $3); }
          | expression { $$ = dagon_list_node_new($1); }

array_value: primary_value LBRACE expression RBRACE { $$ = dagon_method_call_node_new($1, "[]", dagon_list_node_new($3)); }

combined_string: combined_string dstring { dagon_combine_string_node_append($1, $2); }
               | combined_string string { dagon_combine_string_node_append($1, $2); }
               | string { $$ = dagon_combine_string_node_new($1); }
               | dstring { $$ = dagon_combine_string_node_new($1); }

string: STRING_PART { $$ = dagon_string_node_new($1); }

dstring: DSTRING_BEGIN expression DSTRING_END
       {
        $$ = dagon_method_call_node_new($2, "to-s", dagon_list_node_new(NULL));
       }

variable: ID { $$ = dagon_variable_node_new($1); }
        | AT ID { $$ = dagon_instance_variable_node_new($2); }

number: DIGIT { $$ = dagon_int_node_new($1); }
      | NEGATE DIGIT { $$ = dagon_int_node_new(-$2); }

block: NEWLINE INDENT statements NEWLINE DEDENT { $$ = $3; }

opt_newlines: /* No Newline */
            | opt_newlines NEWLINE
