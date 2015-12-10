#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "env.h"
#include "node.h"

static const char *node_labels[] = {
  FOREACH_NODE(GENERATE_STRING)
};

#define DEFAULT_STACK_SIZE 1000

VALUE dagon_send(DagonEnv *env, VALUE value, const char *method_name, int argc, VALUE* values);
VALUE dagon_string_new(DagonEnv* env, const char *internal);
VALUE dagon_const_get(DagonEnv *env, const char *const_name);

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

void dagon_list_append(DagonList* list, const char* name, VALUE value) {
  DagonListNode* node = malloc(sizeof(DagonListNode));
  node->name = name;
  node->value = value;
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
      dagon_list_append(scope->locals, current_node->name, current_node->value);
      current_node = current_node->next;
    }
  }

  return scope;
}

void dagon_stack_pop(DagonEnv* env) {
  env->stack->sp--;
}

DagonClass* dagon_class_new(const char* name, DagonClass* parent) {
  DagonClass* klass = malloc(sizeof(DagonClass));
  klass->name = name;
  klass->parent = parent;
  klass->methods = NULL;
  return klass;
}

DagonObject* dagon_object_alloc(DagonClass* klass) {
  DagonObject* object = malloc(sizeof(DagonObject));
  object->header.klass = klass;
  object->header.ivars = dagon_list_new();
  return object;
}

VALUE dagon_object_print(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  VALUE dg_stdout = dagon_const_get(env, "STDOUT");
  return dagon_send(env, dg_stdout, "print", 1, values);
}

VALUE dagon_object_puts(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  VALUE dg_stdout = dagon_const_get(env, "STDOUT");
  return dagon_send(env, dg_stdout, "puts", 1, values);
}

VALUE dagon_object_gets(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  VALUE dg_stdin = dagon_const_get(env, "STDIN");
  return dagon_send(env, dg_stdin, "gets", 0, NULL);
}

VALUE dagon_array_get(DagonEnv *env, DagonArray* array, int index) {
  DagonListNode* item = array->head;
  for(int i = 0; i < index; i++) {
    item = item->next;
  }
  return item->value;
}

VALUE dagon_array_new(DagonEnv* env) {
  DagonArray* array = malloc(sizeof(DagonArray));
  array->head = malloc(sizeof(DagonListNode));
  array->header.klass = dagon_class_lookup(env, "Array");
  return (VALUE) array;
}

VALUE dagon_array_mult(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray* array = (DagonArray*) self;
  int times = FIX2INT(values[0]);
  int length = array->len;
  int new_length = times * length;

  DagonArray* new_array = (DagonArray*) dagon_array_new(env);
  new_array->head = malloc(sizeof(DagonListNode));
  DagonListNode* current = new_array->head;

  for(int i = 0; i < times; i++) {
    for(int j = 0; j < length; j++) {
      current->value = dagon_array_get(env, array, j);
      current->next = malloc(sizeof(DagonListNode));
      current = current->next;
    }
  }

  new_array->len = new_length;

  return (VALUE) new_array;
}

VALUE dagon_array_assign(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  int index = FIX2INT(values[0]);
  VALUE value = values[1];
  DagonListNode* item = array->head;
  for(int i = 0; i < index; i++) {
    item = item->next;
  }
  item->value = value;
  return value;
}

VALUE dagon_array_ref(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  int index = FIX2INT(values[0]);
  return dagon_array_get(env, array, index);
}

VALUE dagon_array_last(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  return dagon_array_get(env, array, array->len - 1);
}

VALUE dagon_array_to_s(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  return dagon_string_new(env, "[]");
}

VALUE dagon_array_push(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  int length = array->len;
  if(length == 0) {
    array->head->value = values[0];
  } else {
    DagonListNode* item = array->head;
    for(int i = 0; i < length - 1; i++) {
      item = item->next;
    }

    item->next = malloc(sizeof(DagonListNode));
    item->next->value = values[0];
  }
  array->len++;

  return (VALUE) array;
}

VALUE dagon_array_pop(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  VALUE return_value = dagon_array_get(env, array, array->len - 1);
  array->len--;
  return return_value;
}

VALUE dagon_integer_lt(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int left = FIX2INT(self);
  int right = FIX2INT(values[0]); // TODO: CHECK IS INT
  return left < right ? Dtrue : Dfalse;
}

VALUE dagon_integer_chr(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int num = FIX2INT(self);
  char *str = malloc(sizeof(char) * 2);
  snprintf(str, 2, "%c", num);
  return dagon_string_new(env, str);
}

VALUE dagon_integer_to_s(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int num = FIX2INT(self);
  int len = 1;
  int tmp = num;
  while(tmp >= 10) {
    len++;
    tmp = tmp / 10;
  }
  if(num < 0) {
    len++;
  }
  char *str = malloc(sizeof(char) * (len + 1));
  snprintf(str, len + 1, "%d", num);
  return dagon_string_new(env, str);
}

VALUE dagon_integer_eq(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int left = FIX2INT(self);
  int right = FIX2INT(values[0]); // TODO: CHECK IS INT
  return left == right ? Dtrue : Dfalse;
}

VALUE dagon_integer_plus(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int left = FIX2INT(self);
  int right = FIX2INT(values[0]); // TODO: CHECK IS INT
  return INT2FIX(left + right);
}

VALUE dagon_integer_minus(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  int left = FIX2INT(self);
  int right = FIX2INT(values[0]); // TODO: CHECK IS INT
  return INT2FIX(left - right);
}

VALUE dagon_string_length(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonString* string = (DagonString*) self;
  int length = strlen(string->internal);
  return INT2FIX(length);
}

VALUE dagon_string_new(DagonEnv* env, const char *internal) {
  DagonString* string = malloc(sizeof(DagonString));
  string->header.klass = dagon_class_lookup(env, "String");
  string->internal = strdup(internal);
  return (VALUE) string;
}

VALUE dagon_string_character_at(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonString* string = (DagonString*) self;
  int index = FIX2INT(values[0]);
  char c = string->internal[index];
  char str[2] = "\0";
  str[0] = c;
  return dagon_string_new(env, str);
}

VALUE dagon_string_compare(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonString* lhs = (DagonString*) self;
  DagonString* rhs = (DagonString*) values[0];
  return strcmp(lhs->internal, rhs->internal) == 0 ? Dtrue : Dfalse;
}

VALUE dagon_string_to_s(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  return self;
}

VALUE dagon_io_get_c(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonIO* io = (DagonIO*) self;
  FILE* file = io->file;
  int chr = fgetc(file);
  return INT2FIX(chr);
}

VALUE dagon_io_print(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonIO* io = (DagonIO*) self;
  DagonString* string = (DagonString*) dagon_send(env, values[0], "to-s", 0, NULL);
  fprintf(io->file, "%s", string->internal);
  fflush(io->file);
  return Dtrue;
}

VALUE dagon_io_puts(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  dagon_io_print(env, self, argc, values);
  DagonIO* io = (DagonIO*) self;
  fprintf(io->file, "\n");
  return Dtrue;
}

VALUE dagon_io_gets(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonIO* io = (DagonIO*) self;
  char* in = malloc(sizeof(char) * 1024);
  fgets(in, 1024, io->file);
  return dagon_string_new(env, in);
}

void dagon_class_add_method(DagonEnv* env, DagonClass* klass, DagonMethod* method) {
  DagonMethodList* item = malloc(sizeof(DagonMethodList));
  item->method = method;
  item->next = NULL;

  if(klass->methods) {
    DagonMethodList* current = klass->methods;
    while(current->next) {
      current = current->next;
    }

    current->next = item;
  } else {
    klass->methods = item;
  }
}

void dagon_class_add_dagon_method(DagonEnv* env, DagonClass* klass, const char* name, ListNode* args, ListNode* statements) {
  DagonMethod* method = malloc(sizeof(DagonMethod));
  method->name = name;
  method->args = args;
  method->statements = statements;
  method->type = DAGON_METHOD;

  dagon_class_add_method(env, klass, method);
}

void dagon_class_add_c_method(DagonEnv* env, DagonClass* klass, const char* name, void* c_func) {
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

  DagonClass* object = malloc(sizeof(DagonClass));
  object->name = "Object";
  object->methods = NULL;
  object->parent = NULL;
  dagon_class_set(env, "Object", object);
  dagon_class_add_c_method(env, object, "print", dagon_object_print);
  dagon_class_add_c_method(env, object, "puts", dagon_object_puts);
  dagon_class_add_c_method(env, object, "gets", dagon_object_gets);

  DagonObject* main_object = dagon_object_alloc(object);
  DagonScope* scope = dagon_scope_new(env);
  scope->current = (VALUE) main_object;

  dagon_stack_push(env, scope);

  DagonClass* array = dagon_class_new("Array", object);
  dagon_class_set(env, "Array", array);
  dagon_class_add_c_method(env, array, "*", dagon_array_mult);
  dagon_class_add_c_method(env, array, "[]=", dagon_array_assign);
  dagon_class_add_c_method(env, array, "[]", dagon_array_ref);
  dagon_class_add_c_method(env, array, "push", dagon_array_push);
  dagon_class_add_c_method(env, array, "pop", dagon_array_pop);
  dagon_class_add_c_method(env, array, "last", dagon_array_last);
  dagon_class_add_c_method(env, array, "to-s", dagon_array_to_s);

  DagonClass* integer = dagon_class_new("Integer", object);
  dagon_class_set(env, "Integer", integer);
  dagon_class_add_c_method(env, integer, "<", dagon_integer_lt);
  dagon_class_add_c_method(env, integer, "+", dagon_integer_plus);
  dagon_class_add_c_method(env, integer, "-", dagon_integer_minus);
  dagon_class_add_c_method(env, integer, "=", dagon_integer_eq);
  dagon_class_add_c_method(env, integer, "chr", dagon_integer_chr);
  dagon_class_add_c_method(env, integer, "to-s", dagon_integer_to_s);

  DagonClass* string = dagon_class_new("String", object);
  dagon_class_set(env, "String", string);
  dagon_class_add_c_method(env, string, "length", dagon_string_length);
  dagon_class_add_c_method(env, string, "[]", dagon_string_character_at);
  dagon_class_add_c_method(env, string, "=", dagon_string_compare);
  dagon_class_add_c_method(env, string, "to-s", dagon_string_to_s);

  DagonClass* io = dagon_class_new("IO", object);
  dagon_class_set(env, "IO", io);
  dagon_class_add_c_method(env, io, "getc", dagon_io_get_c);
  dagon_class_add_c_method(env, io, "print", dagon_io_print);
  dagon_class_add_c_method(env, io, "puts", dagon_io_puts);
  dagon_class_add_c_method(env, io, "gets", dagon_io_gets);

  DagonIO* dg_stdin = malloc(sizeof(DagonIO));
  dg_stdin->header.klass = io;
  dg_stdin->file = fdopen(fcntl(STDIN_FILENO,  F_DUPFD, 0), "r");
  dagon_const_set(env, "STDIN", (DagonObject*) dg_stdin);

  DagonIO* dg_stdout = malloc(sizeof(DagonIO));
  dg_stdout->header.klass = io;
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

void dagon_class_set(DagonEnv* env, const char* name, DagonClass* klass) {
  dagon_list_append(env->classes, name, (VALUE) klass);
}

void dagon_local_set(DagonEnv* env, const char* varname, VALUE value) {
  DagonScope* scope = dagon_current_scope(env);
  DagonListNode* node = malloc(sizeof(DagonListNode));
  node->name = varname;
  node->value = value;
  node->next = NULL;

  DagonListNode* current = scope->locals->head;
  if(current) {
    while(current->next && strcmp(current->name, varname) != 0)
      current = current->next;
    if(strcmp(current->name, varname) == 0) {
      current->value = value;
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
    return_value = current->value;
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
      current->value = value;
    } else {
      current->next = malloc(sizeof(DagonListNode));
      current->next->name = strdup(instance_variable_name);
      current->next->value = value;
      current->next->next = NULL;
    }
  } else {
    object->header.ivars->head = malloc(sizeof(DagonListNode));
    object->header.ivars->head->name = strdup(instance_variable_name);
    object->header.ivars->head->value = value;
    object->header.ivars->head->next = NULL;
  }
}

int dagon_class_has_method(DagonEnv *env, DagonClass *klass, const char *method_name) {
  if(!klass)
    return 0;
  DagonMethod* method = NULL;
  DagonMethodList* current = klass->methods;
  while(current && strcmp(current->method->name, method_name) != 0)
    current = current->next;

  if(current)
    return 1;
  else if(klass->parent)
    return dagon_class_has_method(env, klass->parent, method_name);

  return 0;
}

DagonMethod* dagon_method_lookup(DagonEnv *env, DagonClass *klass, const char *method_name) {
  DagonMethod* method = NULL;
  DagonMethodList* current = klass->methods;
  while(current && strcmp(current->method->name, method_name) != 0)
    current = current->next;

  if(current)
    method = current->method;
  else if(dagon_class_has_method(env, klass->parent, method_name))
    return dagon_method_lookup(env, klass->parent, method_name);
  else
    dagon_error(env, "Undefined method %s#%s\n", klass->name, method_name);

  return method;
}

DagonClass* dagon_class_lookup(DagonEnv *env, const char *class_name) {
  return (DagonClass*) dagon_list_lookup(env, env->classes, class_name, 1);
}

VALUE dagon_const_get(DagonEnv *env, const char *const_name) {
  return (VALUE) dagon_list_lookup(env, env->constants, const_name, 1);
}

VALUE dagon_run_list(DagonEnv *env, ListNode* statement) {
  VALUE return_value;
  while(statement && statement->current) {
    return_value = dagon_run(env, statement->current);
    statement = statement->next;
  }
  return return_value;
}

DagonClass* dagon_class_for(DagonEnv *env, DagonObject* object) {
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
  DagonClass* klass = dagon_class_for(env, object);
  DagonScope* scope = dagon_scope_new(env);
  scope->current = value;
  DagonMethod* method = dagon_method_lookup(env, klass, method_name);

  if(method->type == DAGON_METHOD) {
    ListNode* params = method->args;
    int i = 0;
    while(params && params->current) {
      const char* varname = dagon_variable_name(env, params->current);
      dagon_list_append(scope->locals, varname, args[i]);
      params = params->next;
    }
    ListNode* statement = method->statements;
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
          dagon_run_list(env, while_statement_node->statements);
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
        return_value = dagon_run_list(env, (ListNode*) node->value.ptr);
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
          current->value = dagon_run(env, items->current);
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
        DagonClass* klass = dagon_class_lookup(env, object_initialization_node->name);
        DagonObject* object = dagon_object_alloc(klass);
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
                  return dagon_run_list(env, case_node->statements);
                }
                break;
              }
            case CATCHALL_CASE_NODE:
              {
                CatchallCaseNode* case_node = (CatchallCaseNode*) cases->current;
                dagon_run_list(env, case_node->statements);
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
        DagonClass* parent = dagon_class_lookup(env, "Object");
        DagonClass* klass = dagon_class_new(cdn->name, parent);

        ListNode* item = cdn->methods;
        while(item && item->current) {
          MethodDefinitionNode* mdn = (MethodDefinitionNode*) item->current->value.ptr;
          dagon_class_add_dagon_method(env, klass, mdn->name, mdn->args, mdn->statements);
          item = item->next;
        }

        dagon_class_set(env, cdn->name, klass);
        break;
      }
    case IF_STATEMENT_NODE:
      {
        IfStatementNode* if_statement_node = (IfStatementNode*) node->value.ptr;
        if(dagon_run(env, if_statement_node->expression) != Dfalse) {
          dagon_run_list(env, if_statement_node->true_statements);
        } else {
          dagon_run_list(env, if_statement_node->false_statements);
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
    default:
      fprintf(stderr, "Could not eval node type: %s\n", node_labels[node->type]);
      exit(1);
  }

  return return_value;
}
