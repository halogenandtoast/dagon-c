#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"

#define DO_INDENT printf("%*s", indentation * 2, "")
#define LIST(node) (ListNode*) node->value.ptr
#define INDENT if (!inlined) DO_INDENT
#define NODE(ntype, nvalue) Node* _n = malloc(sizeof(Node)); \
                                             _n->type = ntype##_NODE; \
                                             _n->value.ptr = (void*) nvalue; \
                                             return _n

extern void fatal_error(const char *msg, ...);
void dagon_dump_indented(Node* node, int indentation, int inlined);
void dagon_dump_list_node(ListNode* list, int indentation);

void dagon_dump(Node* node) {
  dagon_dump_indented(node, 0, 0);
}

void dagon_dump_indented(Node* node, int indentation, int inlined) {
  INDENT;

  switch(node->type) {
    case OBJECT_INITIALIZE_NODE:
      {
        ObjectInitializeNode* object_initialize_node = (ObjectInitializeNode*) node->value.ptr;
        printf("<objinit \"%s\" args: [", object_initialize_node->name);
        dagon_dump_list_node(object_initialize_node->args, 0);
        printf(">");
        break;
      }
    case IF_STATEMENT_NODE:
      {
        IfStatementNode* if_statement_node = (IfStatementNode*) node->value.ptr;
        printf("<if ");
        dagon_dump_indented(if_statement_node->expression, 0, 1);
        printf("\n");
        DO_INDENT;
        printf("  true:\n");
        dagon_dump_list_node(if_statement_node->true_statements, indentation + 2);
        printf("\n");
        DO_INDENT;
        printf("  false:\n");
        dagon_dump_list_node(if_statement_node->false_statements, indentation + 2);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case CASE_NODE:
      {
        CaseNode* case_node = (CaseNode*) node->value.ptr;
        printf("<case ");
        dagon_dump_indented(case_node->value, 0, 1);
        printf("\n");
        dagon_dump_list_node(case_node->statements, indentation + 1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case CATCHALL_CASE_NODE:
      {
        CatchallCaseNode* catchall_case_node = (CatchallCaseNode*) node->value.ptr;
        printf("<catchall-case ");
        printf("\n");
        dagon_dump_list_node(catchall_case_node->statements, indentation + 1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case METHOD_CALL_NODE:
      {
        MethodCallNode* method_call_node = (MethodCallNode*) node->value.ptr;
        printf("<call \"%s\" on ", method_call_node->method);
        dagon_dump_indented(method_call_node->object, 0, 1);
        printf(" args: [");
        dagon_dump_list_node(method_call_node->args, 0);
        printf("]>");
        break;
      }
    case SCOPED_METHOD_CALL_NODE:
      {
        ScopedMethodCallNode* scoped_method_call_node = (ScopedMethodCallNode*) node->value.ptr;
        printf("<call \"%s\" on <self>", scoped_method_call_node->method);
        printf(" args: [");
        dagon_dump_list_node(scoped_method_call_node->args, 0);
        printf("]>");
        break;
      }
    case CASE_STATEMENT_NODE:
      {
        CaseStatementNode* case_statement_node = (CaseStatementNode*) node->value.ptr;
        printf("<switch ");
        dagon_dump_indented(case_statement_node->value, 0, 1);
        printf("\n");
        dagon_dump_list_node(case_statement_node->cases, indentation+1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case WHILE_STATEMENT_NODE:
      {
        WhileStatementNode* while_node = (WhileStatementNode*) node->value.ptr;
        printf("<while (");
        dagon_dump_indented(while_node->conditional, 0, 1);
        printf(")\n");
        dagon_dump_list_node(while_node->statements, indentation + 1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case ARRAY_NODE:
      {
        ArrayNode* array_node = (ArrayNode*) node->value.ptr;
        printf("[");
        dagon_dump_list_node(array_node->items, indentation);
        printf("]");
        break;
      }
    case VARIABLE_NODE:
      {
        VariableNode* variable_node = (VariableNode*) node->value.ptr;
        printf("<var(\"%s\")>", variable_node->name);
        break;
      }
    case INSTANCE_VARIABLE_NODE:
      {
        InstanceVariableNode* instance_variable_node = (InstanceVariableNode*) node->value.ptr;
        printf("<ivar(\"@%s\")>", instance_variable_node->name);
        break;
      }
    case CONSTANT_NODE:
      {
        ConstantNode* constant_node = (ConstantNode*) node->value.ptr;
        printf("<const(\"%s\")>", constant_node->name);
        break;
      }
    case ASSIGNMENT_NODE:
      {
        AssignmentNode* assignment_node = (AssignmentNode*) node->value.ptr;
        printf("(");
        dagon_dump_indented(assignment_node->variable, 0, 1);
        printf(" = ");
        dagon_dump_indented(assignment_node->value, 0, 1);
        printf(")");
        break;
      }
    case CLASS_DEFINITION_NODE:
      {
        ClassDefinitionNode *class_definition_node = (ClassDefinitionNode*) node->value.ptr;
        printf("<class(\"%s\")\n", class_definition_node->name);
        dagon_dump_list_node(class_definition_node->methods, indentation + 1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case METHOD_DEFINITION_NODE:
      {
        MethodDefinitionNode *method_definition_node = (MethodDefinitionNode*) node->value.ptr;
        printf("<method(\"%s\" params: [", method_definition_node->name);
        dagon_dump_list_node(method_definition_node->args, 0);
        printf("])\n");
        dagon_dump_list_node(method_definition_node->statements, indentation + 1);
        printf("\n");
        DO_INDENT;
        printf(">");
        break;
      }
    case STATEMENTS_NODE:
      printf("<statements [\n");
      dagon_dump_list_node(node->value.ptr, indentation + 1);
      printf("\n");
      INDENT;
      printf("]>");
      break;
    case INT_NODE:
      printf("%d", node->value.ival);
      break;
    case STRING_NODE:
      printf("\"%s\"", node->value.sval);
      break;
    case DUMMY_NODE:
      printf("(Needs to be implemented: %s)", node->value.sval);
      break;
    default:
      fatal_error("Unknown node type: %i\n", node->type);
  }

  if (!inlined) printf("\n");
}

void dagon_dump_list_node(ListNode* item, int indentation) {
  while(item && item->current) {
    DO_INDENT;
    dagon_dump_indented(item->current, indentation, 1);
    if(item->next) {
      if(indentation > 0) {
        printf(",\n");
      } else {
        printf(", ");
      }
    }
    item = item->next;
  }
}

Node* dagon_dummy_node_new(const char* name) {
  NODE(DUMMY, name);
}

Node* dagon_list_node_new(Node* item) {
  ListNode* list_node = malloc(sizeof(ListNode));
  list_node->current = item;
  list_node->next = NULL;
  NODE(STATEMENTS, list_node);
}

void dagon_list_node_append(Node* list, Node* item) {
  ListNode* list_node = malloc(sizeof(ListNode));
  list_node->current = item;
  list_node->next = NULL;

  ListNode* last_node = LIST(list);

  if(last_node) {
    while(last_node->next) {
      last_node = last_node->next;
    }
    last_node->next = list_node;
  } else {
    list->value.ptr = list_node;
  }
}

Node* dagon_int_node_new(int val) {
  Node* node = malloc(sizeof(Node));
  node->value.ival = val;
  node->type = INT_NODE;
  return node;
}

Node* dagon_string_node_new(const char *sval) {
  Node* node = malloc(sizeof(Node));
  node->value.sval = sval;
  node->type = STRING_NODE;
  return node;
}

Node* dagon_class_definition_node_new(const char *name, Node* method_definitions) {
  ClassDefinitionNode* class_definition_node = malloc(sizeof(ClassDefinitionNode));
  class_definition_node->name = name;
  class_definition_node->methods = LIST(method_definitions);
  NODE(CLASS_DEFINITION, class_definition_node);
}

Node* dagon_method_definition_node_new(const char *name, Node* arglist, Node* statements) {
  MethodDefinitionNode* method_definition_node = malloc(sizeof(MethodDefinitionNode));
  method_definition_node->name = name;
  method_definition_node->args = LIST(arglist);
  method_definition_node->statements = LIST(statements);
  NODE(METHOD_DEFINITION, method_definition_node);
}

Node* dagon_assignment_node_new(Node* variable, Node* value) {
  AssignmentNode* assignment_node = malloc(sizeof(AssignmentNode));
  assignment_node->variable = variable;
  assignment_node->value = value;
  NODE(ASSIGNMENT, assignment_node);
}

Node* dagon_variable_node_new(const char *name) {
  VariableNode* variable_node = malloc(sizeof(VariableNode));
  variable_node->name = name;
  NODE(VARIABLE, variable_node);
}

Node* dagon_instance_variable_node_new(const char *name) {
  InstanceVariableNode* instance_variable_node = malloc(sizeof(InstanceVariableNode));
  instance_variable_node->name = name;
  NODE(INSTANCE_VARIABLE, instance_variable_node);
}

Node* dagon_array_node_new(Node* items) {
  ArrayNode* array_node = malloc(sizeof(ArrayNode));
  array_node->items = LIST(items);
  NODE(ARRAY, array_node);
}

Node* dagon_while_statement_node_new(Node* conditional, Node* statements) {
  WhileStatementNode* while_node = malloc(sizeof(WhileStatementNode));
  while_node->conditional = conditional;
  while_node->statements = LIST(statements);
  NODE(WHILE_STATEMENT, while_node);
}

Node* dagon_method_call_node_new(Node* object, const char *method, Node* args) {
  MethodCallNode* method_call_node = malloc(sizeof(MethodCallNode));
  method_call_node->object = object;
  method_call_node->method = method;
  method_call_node->args = LIST(args);
  NODE(METHOD_CALL, method_call_node);
}

Node* dagon_scoped_method_call_node_new(const char *method, Node* args) {
  ScopedMethodCallNode* scoped_method_call_node = malloc(sizeof(ScopedMethodCallNode));
  scoped_method_call_node->method = method;
  scoped_method_call_node->args = LIST(args);
  NODE(SCOPED_METHOD_CALL, scoped_method_call_node);
}

Node* dagon_case_statement_node_new(Node* value, Node* cases) {
  CaseStatementNode* case_statement_node = malloc(sizeof(CaseStatementNode));
  case_statement_node->value = value;
  case_statement_node->cases = LIST(cases);
  NODE(CASE_STATEMENT, case_statement_node);
}

Node* dagon_case_node_new(Node* value, Node* statements) {
  CaseNode* case_node = malloc(sizeof(CaseNode));
  case_node->value = value;
  case_node->statements = LIST(statements);
  NODE(CASE, case_node);
}

Node* dagon_catchall_case_node_new(Node* statements) {
  CatchallCaseNode* catchall_case_node = malloc(sizeof(CatchallCaseNode));
  catchall_case_node->statements = LIST(statements);
  NODE(CATCHALL_CASE, catchall_case_node);
}

Node* dagon_constant_node_new(const char* name) {
  ConstantNode* constant_node = malloc(sizeof(ConstantNode));
  constant_node->name = name;
  NODE(CONSTANT, constant_node);
}

Node* dagon_if_statement_node_new(Node* expression, Node* true_statements, Node* false_statements) {
  IfStatementNode* if_statement_node = malloc(sizeof(IfStatementNode));
  if_statement_node->expression = expression;
  if_statement_node->true_statements = LIST(true_statements);
  if_statement_node->false_statements = LIST(false_statements);
  NODE(IF_STATEMENT, if_statement_node);
}

Node* dagon_object_initialize_node_new(const char* name, Node* args) {
  ObjectInitializeNode* object_initialize_node = malloc(sizeof(ObjectInitializeNode));
  object_initialize_node->name = name;
  object_initialize_node->args = LIST(args);
  NODE(OBJECT_INITIALIZE, object_initialize_node);
}
