#ifndef _DAGON_H_
#define _DAGON_H_

#include <stdint.h>
#include <stdio.h>

#define Dfalse 0
#define Dtrue 2

#define SIGNED_VALUE intptr_t
#define FIX2INT(x) (((SIGNED_VALUE)(x)) >> 1)
#define INT2FIX(x) ((VALUE)(((x) << 1) + 1))

typedef uintptr_t VALUE;

VALUE dg_cObject;
VALUE dg_cIO;

typedef struct DagonListNode {
  const char *name;
  union {
    VALUE value;
    const char *str;
  } current;
  struct DagonListNode* next;
} DagonListNode;

typedef struct DagonList {
  DagonListNode* head;
  int len;
} DagonList;

typedef struct {
  VALUE current;
  DagonList* locals;
} DagonScope;

typedef struct {
  DagonScope** scopes;
  int sp;
  int size;
} DagonStack;

typedef struct {
  DagonStack* stack;
  DagonList* constants;
  DagonList* classes;
} DagonEnv;

typedef struct {
  VALUE value;
} NativeNode;

typedef enum {
  DAGON_METHOD,
  C_METHOD
} DagonFunctionType;

typedef struct {
  const char* name;
  DagonList* args;
  DagonList* statements;
  VALUE (*c_func)(DagonEnv *env, VALUE self, int argc, VALUE* argv);
  DagonFunctionType type;
} DagonMethod;

typedef struct DagonMethodList {
  DagonMethod* method;
  struct DagonMethodList* next;
} DagonMethodList;

typedef struct {
  const char* name;
  DagonMethodList* methods;
  VALUE parent;
} DagonClass;

typedef struct {
  VALUE klass;
  DagonList* ivars;
} DagonObjectHeader;

typedef struct {
  DagonObjectHeader header;
  DagonListNode* head;
  int len;
} DagonArray;

typedef struct {
  DagonObjectHeader header;
  const char* internal;
} DagonString;

typedef struct {
  DagonObjectHeader header;
  FILE* file;
} DagonIO;

typedef struct {
  DagonObjectHeader header;
} DagonObject;

VALUE dagon_string_new(DagonEnv* env, const char *internal);
VALUE dagon_class_new(const char* class_name, VALUE parent_class);
VALUE dagon_class_lookup(DagonEnv* env, const char *name);
VALUE dagon_array_new(DagonEnv* env);
VALUE dagon_object_alloc(VALUE klass);
VALUE dagon_const_get(DagonEnv *env, const char *const_name);
void dagon_class_set(DagonEnv* env, const char *name, VALUE klass);
void dagon_class_add_c_method(DagonEnv* env, VALUE klass, const char* name, void* c_func);
VALUE dagon_send(DagonEnv *env, VALUE value, const char *method_name, int argc, VALUE* values);
DagonList* dagon_list_new();

#endif
