#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "env.h"
#include "node.h"
#include "dagon.h"

static const char *node_labels[] = {
  FOREACH_NODE(GENERATE_STRING)
};

#define DEFAULT_STACK_SIZE 1000

DagonScope* dagon_current_scope(DagonEnv* env) {
  return env->stack->scopes[env->stack->sp - 1];
}

DagonObject* dagon_current_object(DagonEnv* env) {
  return (DagonObject*) dagon_current_scope(env)->current;
}

DagonList* dagon_list_new() {
  DagonList* list = malloc(sizeof(DagonList));
  list->head = NULL;
  list->len = 0;
  return list;
}

void dagon_list_append(DagonList* list, const char* name, VALUE value) {
  DagonListNode* node = malloc(sizeof(DagonListNode));
  node->name = name;
  node->current.value = value;
  node->next = NULL;

  if(list->head) {
    DagonListNode* current = list->head;
    while(current->next)
      current = current->next;
    current->next = node;
  } else {
    list->head = node;
  }
}

DagonList* list_node_to_dagon_list(ListNode* list_node) {
  DagonList* list = dagon_list_new();

  while(list_node) {
    dagon_list_append(list, NULL, (VALUE) list_node->current);
    list_node = list_node->next;
  }

  return list;
}

DagonStack* dagon_stack_new() {
  DagonStack* stack = malloc(sizeof(DagonStack));
  stack->sp = 0;
  stack->size = DEFAULT_STACK_SIZE;
  stack->scopes = calloc(stack->size, sizeof(DagonScope*));
  return stack;
}

void dagon_stack_push(DagonEnv* env, DagonScope *scope) {
  DagonStack* stack = env->stack;
  if(stack->sp > stack->size) {
    stack->size = stack->size * 2;
    realloc(stack->scopes, stack->size * sizeof(DagonScope));
  }

  stack->scopes[stack->sp++] = scope;
}

void dagon_const_set(DagonEnv* env, const char* name, DagonObject* constant) {
  dagon_list_append(env->constants, name, (VALUE) constant);
}

DagonScope* dagon_scope_new(DagonEnv* env) {
  DagonScope* current = dagon_current_scope(env);
  DagonScope* scope = malloc(sizeof(DagonScope));
  scope->locals = dagon_list_new();

  if(current) {
    DagonListNode* current_node = current->locals->head;
    while(current_node) {
      dagon_list_append(scope->locals, current_node->name, current_node->current.value);
      current_node = current_node->next;
    }
  }

  return scope;
}

void dagon_stack_pop(DagonEnv* env) {
  env->stack->sp--;
}

VALUE dagon_class_new(const char* class_name, VALUE parent_class) {
  DagonClass* klass = malloc(sizeof(DagonClass));
  klass->name = class_name;
  klass->parent = parent_class;
  klass->methods = NULL;
  return (VALUE) klass;
}

void dagon_class_add_method(DagonEnv* env, VALUE klass, DagonMethod* method) {
  DagonMethodList* item = malloc(sizeof(DagonMethodList));
  item->method = method;
  item->next = NULL;

  DagonClass* cClass = (DagonClass*) klass;

  if(cClass->methods) {
    DagonMethodList* current = cClass->methods;
    while(current->next) {
      current = current->next;
    }

    current->next = item;
  } else {
    cClass->methods = item;
  }
}

void dagon_class_add_dagon_method(DagonEnv* env, VALUE klass, const char* name, DagonList* args, DagonList* statements) {
  DagonMethod* method = malloc(sizeof(DagonMethod));
  method->name = name;
  method->args = args;
  method->statements = statements;
  method->type = DAGON_METHOD;

  dagon_class_add_method(env, klass, method);
}

void dagon_class_add_c_method(DagonEnv* env, VALUE klass, const char* name, void* c_func) {
  DagonMethod* method = malloc(sizeof(DagonMethod));
  method->name = name;
  method->c_func = c_func;
  method->type = C_METHOD;

  dagon_class_add_method(env, klass, method);
}

DagonEnv* dagon_env_new() {
  DagonEnv* env = malloc(sizeof(DagonEnv));
  env->stack = dagon_stack_new();
  env->constants = dagon_list_new();
  env->classes = dagon_list_new();

  Init_Object(env);

  Init_String(env);
  Init_Integer(env);
  Init_Array(env);
  Init_IO(env);

  VALUE main_object = dagon_object_alloc(dg_cObject);
  DagonScope* scope = dagon_scope_new(env);
  scope->current = main_object;
  dagon_stack_push(env, scope);

  DagonIO* dg_stdin = malloc(sizeof(DagonIO));
  dg_stdin->header.klass = dg_cIO;
  dg_stdin->file = fdopen(fcntl(STDIN_FILENO,  F_DUPFD, 0), "r");
  dagon_const_set(env, "STDIN", (DagonObject*) dg_stdin);

  DagonIO* dg_stdout = malloc(sizeof(DagonIO));
  dg_stdout->header.klass = dg_cIO;
  dg_stdout->file = fdopen(fcntl(STDOUT_FILENO, F_DUPFD, 0), "w");
  dagon_const_set(env, "STDOUT", (DagonObject*) dg_stdout);

  return env;
}

void dagon_list_destroy(DagonList* list) {
  DagonListNode* node = list->head;
  while(node) {
    DagonListNode* next = node->next;
    free(node);
    node = next;
  }
  free(list);
}

void dagon_stack_destroy(DagonStack* stack) {
  free(stack);
}

void dagon_env_destroy(DagonEnv* env) {
  dagon_stack_destroy(env->stack);
  dagon_list_destroy(env->constants);
  dagon_list_destroy(env->classes);
  free(env);
}

void dagon_class_set(DagonEnv* env, const char* name, VALUE klass) {
  dagon_list_append(env->classes, name, klass);
}

void dagon_local_set(DagonEnv* env, const char* varname, VALUE value) {
  DagonScope* scope = dagon_current_scope(env);
  DagonListNode* node = malloc(sizeof(DagonListNode));
  node->name = varname;
  node->current.value = value;
  node->next = NULL;

  DagonListNode* current = scope->locals->head;
  if(current) {
    while(current->next && strcmp(current->name, varname) != 0)
      current = current->next;
    if(strcmp(current->name, varname) == 0) {
      current->current.value = value;
    } else {
      current->next = node;
    }
  } else {
    scope->locals->head = node;
  }
}

VALUE dagon_list_lookup(DagonEnv* env, DagonList* list, const char *key, int throw) {
  VALUE return_value;
  DagonListNode* current = list->head;
  while(current && strcmp(current->name, key) != 0)
    current = current->next;

  if(current)
    return_value = current->current.value;
  else if(throw)
    dagon_error(env, "Could not find key: %s\n", key);

  return return_value;
}

VALUE dagon_local_variable_get(DagonEnv* env, const char* name) {
  DagonScope* scope = dagon_current_scope(env);
  return dagon_list_lookup(env, scope->locals, name, 1);
}

VALUE dagon_instance_variable_get(DagonEnv* env, DagonObject *object, const char *instance_variable_name) {
  return dagon_list_lookup(env, object->header.ivars, instance_variable_name, 1);
}

void dagon_instance_variable_set(DagonEnv* env, DagonObject *object, const char *instance_variable_name, VALUE value) {
  DagonListNode* current = object->header.ivars->head;
  if(current) {
    while(current->next && strcmp(current->name, instance_variable_name) != 0)
      current = current->next;
    if(strcmp(current->name, instance_variable_name) == 0) {
      current->current.value = value;
    } else {
      current->next = malloc(sizeof(DagonListNode));
      current->next->name = strdup(instance_variable_name);
      current->next->current.value = value;
      current->next->next = NULL;
    }
  } else {
    object->header.ivars->head = malloc(sizeof(DagonListNode));
    object->header.ivars->head->name = strdup(instance_variable_name);
    object->header.ivars->head->current.value = value;
    object->header.ivars->head->next = NULL;
  }
}

int dagon_class_has_method(DagonEnv *env, VALUE klass, const char *method_name) {
  if(!klass)
    return 0;
  DagonMethod* method = NULL;
  DagonMethodList* current = ((DagonClass*) klass)->methods;
  while(current && strcmp(current->method->name, method_name) != 0)
    current = current->next;

  if(current)
    return 1;
  else if(((DagonClass *) klass)->parent)
    return dagon_class_has_method(env, ((DagonClass*) klass)->parent, method_name);

  return 0;
}

DagonMethod* dagon_method_lookup(DagonEnv *env, VALUE klass, const char *method_name) {
  DagonMethod* method = NULL;
  DagonMethodList* current = ((DagonClass *) klass)->methods;
  while(current && strcmp(current->method->name, method_name) != 0)
    current = current->next;

  if(current)
    method = current->method;
  else if(dagon_class_has_method(env, ((DagonClass *) klass)->parent, method_name))
    return dagon_method_lookup(env, ((DagonClass *) klass)->parent, method_name);
  else
    dagon_error(env, "Undefined method %s#%s\n", ((DagonClass *) klass)->name, method_name);

  return method;
}

VALUE dagon_class_lookup(DagonEnv *env, const char *class_name) {
  return dagon_list_lookup(env, env->classes, class_name, 1);
}

VALUE dagon_const_get(DagonEnv *env, const char *const_name) {
  return (VALUE) dagon_list_lookup(env, env->constants, const_name, 1);
}

VALUE dagon_run_list(DagonEnv *env, DagonList* statements) {
  VALUE return_value;
  DagonListNode *statement = statements->head;
  while(statement && statement->current.value) {
    return_value = dagon_run(env, (Node*) statement->current.value);
    statement = statement->next;
  }
  return return_value;
}

VALUE dagon_class_for(DagonEnv *env, DagonObject* object) {
  if((int) object & 0x1) {
    return dagon_class_lookup(env, "Integer");
  } else {
    return object->header.klass;
  }
}

int dagon_list_node_length(ListNode* node) {
  int i = 0;
  while(node && node->current) {
    i++;
    node = node->next;
  }
  return i;
}

VALUE* dagon_list_node_to_values(DagonEnv* env, ListNode* node) {
  int length = dagon_list_node_length(node);
  VALUE* values = calloc(length, sizeof(VALUE));
  int value_ptr = 0;
  VALUE* current_value = values;
  while(node && node->current) {
    VALUE resolved = dagon_run(env, node->current);
    values[value_ptr++] = resolved;
    node = node->next;
  }

  return values;
}

const char* dagon_variable_name(DagonEnv* env, Node* node) {
  switch(node->type) {
    case VARIABLE_NODE:
      {
        VariableNode* variable_node = (VariableNode*) node->value.ptr;
        return variable_node->name;
      }
    case INSTANCE_VARIABLE_NODE:
      {
        VariableNode* variable_node = (VariableNode*) node->value.ptr;
        return variable_node->name;
      }
    case CONSTANT_NODE:
      {
        ConstantNode* constant_node = (ConstantNode*) node->value.ptr;
        return constant_node->name;
      }
    default:
      dagon_error(env, "By the beard of zeus how did you get here?!");
  }

  return "null";
}


VALUE dagon_send(DagonEnv *env, VALUE value, const char *method_name, int argc, VALUE* args) {
  VALUE return_value;
  DagonObject* object = (DagonObject*) value;
  VALUE klass = dagon_class_for(env, object);
  DagonScope* scope = dagon_scope_new(env);
  scope->current = value;
  DagonMethod* method = dagon_method_lookup(env, klass, method_name);

  if(method->type == DAGON_METHOD) {
    DagonListNode* params = method->args->head;
    int i = 0;
    while(params && params->current.value) {
      const char* varname = dagon_variable_name(env, (Node *) params->current.value);
      dagon_list_append(scope->locals, varname, args[i]);
      params = params->next;
    }
    dagon_stack_push(env, scope);
    return_value = dagon_run_list(env, method->statements);
    dagon_stack_pop(env);
  } else {
    return_value = method->c_func(env, (VALUE) object, argc, args);
  }

  return return_value;
}

VALUE dagon_scoped_method_call(DagonEnv* env, const char* name) {
  DagonObject* object = dagon_current_object(env);
  return dagon_send(env, (VALUE) object, name, 0, NULL);
}

void dagon_error(DagonEnv* env, const char* error, ...) {
  va_list args;
  va_start(args, error);
  vfprintf(stderr, error, args);
  va_end(args);
  exit(1);
}

VALUE dagon_run(DagonEnv* env, Node* node) {
  VALUE return_value;

  switch(node->type) {
    case WHILE_STATEMENT_NODE:
      {
        WhileStatementNode* while_statement_node = (WhileStatementNode*) node->value.ptr;
        VALUE conditional = dagon_run(env, while_statement_node->conditional);
        while(conditional != Dfalse) {
          dagon_run_list(env, list_node_to_dagon_list(while_statement_node->statements));
          conditional = dagon_run(env, while_statement_node->conditional);
        }
        break;
      }
    case ASSIGNMENT_NODE:
      {
        AssignmentNode* assignment_node = (AssignmentNode*) node->value.ptr;
        const char* varname = dagon_variable_name(env, assignment_node->variable);
        VALUE value = dagon_run(env, assignment_node->value);
        switch(assignment_node->variable->type) {
          case VARIABLE_NODE:
            dagon_local_set(env, varname, value);
            break;
          case INSTANCE_VARIABLE_NODE:
            {
              DagonObject* object = dagon_current_object(env);
              dagon_instance_variable_set(env, object, varname, value);
              break;
            }
          default:
            dagon_error(env, "This should never happen");
        }
        break;
      }
    case STATEMENTS_NODE:
      {
        return_value = dagon_run_list(env, list_node_to_dagon_list((ListNode*) node->value.ptr));
        break;
      }
    case INSTANCE_VARIABLE_NODE:
      {
        InstanceVariableNode* instance_variable_node = (InstanceVariableNode*) node->value.ptr;
        DagonObject* object = dagon_current_object(env);

        return_value = dagon_instance_variable_get(env, object, instance_variable_node->name);
        break;
      }
    case STRING_NODE:
      {
        DagonString* string = malloc(sizeof(DagonString));
        string->header.klass = dagon_class_lookup(env, "String");
        string->internal = strdup(node->value.sval);
        return_value = (VALUE) string;
        break;
      }
    case VARIABLE_NODE:
      {
        VariableNode* variable_node = (VariableNode*) node->value.ptr;
        VALUE value = dagon_local_variable_get(env, variable_node->name);
        if(value) {
          return_value = value;
        } else {
          return_value = dagon_scoped_method_call(env, variable_node->name);
        }
        break;
      }
    case ARRAY_NODE:
      {
        ArrayNode* array_node = (ArrayNode*) node->value.ptr;
        ListNode* items = array_node->items;
        int length = dagon_list_node_length(items);
        DagonArray* array = (DagonArray*) dagon_array_new(env);
        array->header.klass = dagon_class_lookup(env, "Array");
        array->head = malloc(sizeof(DagonListNode));
        DagonListNode* current = array->head;
        while(items && items->current) {
          current->current.value = dagon_run(env, items->current);
          array->len++;
          current->next = malloc(sizeof(DagonListNode));
          current = current->next;
          items = items->next;
        }
        return_value = (VALUE) array;
        break;
      }
    case INT_NODE:
      {
        int value = node->value.ival;
        return_value = INT2FIX(value);
        break;
      }
    case METHOD_CALL_NODE:
      {
        MethodCallNode* method_call_node = (MethodCallNode*) node->value.ptr;
        VALUE object = dagon_run(env, method_call_node->object);
        return_value = dagon_send(env, object, method_call_node->method, dagon_list_node_length(method_call_node->args), dagon_list_node_to_values(env, method_call_node->args));
        break;
      }
    case OBJECT_INITIALIZE_NODE:
      {
        ObjectInitializeNode* object_initialization_node = (ObjectInitializeNode*) node->value.ptr;
        VALUE klass = dagon_class_lookup(env, object_initialization_node->name);
        VALUE object = dagon_object_alloc(klass);
        dagon_send(env, (VALUE) object, "init", dagon_list_node_length(object_initialization_node->args), dagon_list_node_to_values(env, object_initialization_node->args));
        return_value = (VALUE) object;
        break;
      }
    case CASE_STATEMENT_NODE:
      {
        CaseStatementNode* case_statement_node = (CaseStatementNode*) node->value.ptr;
        VALUE value = dagon_run(env, case_statement_node->value);
        ListNode* cases = case_statement_node->cases;
        while(cases) {
          switch(cases->current->type) {
            case CASE_NODE:
              {
                CaseNode* case_node = (CaseNode*) cases->current->value.ptr;
                VALUE rhs = dagon_run(env, case_node->value);
                if(dagon_send(env, value, "=", 1, &rhs) == Dtrue) {
                  return dagon_run_list(env, list_node_to_dagon_list(case_node->statements));
                }
                break;
              }
            case CATCHALL_CASE_NODE:
              {
                CatchallCaseNode* case_node = (CatchallCaseNode*) cases->current;
                dagon_run_list(env, list_node_to_dagon_list(case_node->statements));
                break;
              }
            default:
              dagon_error(env, "Unknown error");
          }
          cases = cases->next;
        }
        break;
      }
    case CLASS_DEFINITION_NODE:
      {
        ClassDefinitionNode* cdn = (ClassDefinitionNode*) node->value.ptr;
        VALUE klass = dagon_class_new(cdn->name, dg_cObject);

        ListNode* item = cdn->methods;
        while(item && item->current) {
          MethodDefinitionNode* mdn = (MethodDefinitionNode*) item->current->value.ptr;
          dagon_class_add_dagon_method(env, klass, mdn->name, list_node_to_dagon_list(mdn->args), list_node_to_dagon_list(mdn->statements));
          item = item->next;
        }

        dagon_class_set(env, cdn->name, klass);
        break;
      }
    case IF_STATEMENT_NODE:
      {
        IfStatementNode* if_statement_node = (IfStatementNode*) node->value.ptr;
        if(dagon_run(env, if_statement_node->expression) != Dfalse) {
          dagon_run_list(env, list_node_to_dagon_list(if_statement_node->true_statements));
        } else {
          dagon_run_list(env, list_node_to_dagon_list(if_statement_node->false_statements));
        }
        break;
      }
    case SCOPED_METHOD_CALL_NODE:
      {
        ScopedMethodCallNode* scoped_method_call_node = (ScopedMethodCallNode*) node->value.ptr;
        VALUE object = (VALUE) dagon_current_object(env);
        return_value = dagon_send(env, object, scoped_method_call_node->method, dagon_list_node_length(scoped_method_call_node->args), dagon_list_node_to_values(env, scoped_method_call_node->args));
        break;
      }
    case CONSTANT_NODE:
      {
        const char *name = dagon_variable_name(env, node);
        return dagon_const_get(env, name);
      }
    case COMBINED_STRING_NODE:
      {
        CombinedStringNode* combined_string = (CombinedStringNode*) node->value.ptr;
        if(combined_string->next_part) {
          VALUE string = dagon_run(env, combined_string->string);
          CombinedStringNode* current = combined_string;
          while(current->next_part) {
            current = current->next_part;
            VALUE next_part = dagon_run(env, current->string);
            string = dagon_send(env, string, "+", 1, &next_part);
          }
          return string;
        } else {
          return dagon_run(env, combined_string->string);
        }
      }
    default:
      fprintf(stderr, "Could not eval node type: %s\n", node_labels[node->type]);
      exit(1);
  }

  return return_value;
}
