#ifndef _DAGON_ENV_
#define _DAGON_ENV_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include "node.h"

#define Dfalse 0
#define Dtrue 2

typedef uintptr_t VALUE;
#define SIGNED_VALUE intptr_t
#define FIX2INT(x) (((SIGNED_VALUE)(x)) >> 1)
#define INT2FIX(x) ((VALUE)(((x) << 1) + 1))

typedef struct {
  VALUE value;
} NativeNode;

typedef enum {
  DAGON_METHOD,
  C_METHOD
} DagonFunctionType;

typedef struct DagonListNode {
  const char *name;
  VALUE value;
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
  const char* name;
  ListNode* args;
  ListNode* statements;
  VALUE (*c_func)(DagonEnv *env, VALUE self, int argc, VALUE* argv);
  DagonFunctionType type;
} DagonMethod;

typedef struct DagonMethodList {
  DagonMethod* method;
  struct DagonMethodList* next;
} DagonMethodList;

typedef struct DagonClass {
  const char* name;
  DagonMethodList* methods;
  struct DagonClass* parent;
} DagonClass;

typedef struct {
  DagonClass *klass;
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

VALUE dagon_constant_lookup(DagonEnv* env, const char *name);
DagonClass* dagon_class_lookup(DagonEnv* env, const char *name);
VALUE dagon_local_lookup(DagonEnv* env, const char *name);

void dagon_class_set(DagonEnv* env, const char *name, DagonClass* klass);
void dagon_constant_set(DagonEnv* env, const char *name, VALUE constant);
void dagon_local_set(DagonEnv* env, const char *name, VALUE value);
void dagon_error(DagonEnv* env, const char *error, ...);

DagonMethod* dagon_method_lookup(DagonEnv* env, DagonClass* klass, const char *method);

DagonEnv* dagon_env_new();
void dagon_env_destroy(DagonEnv* env);
VALUE dagon_run(DagonEnv* env, Node* node);

#endif
