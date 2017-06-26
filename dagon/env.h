#ifndef _DAGON_ENV_
#define _DAGON_ENV_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "node.h"
#include "dagon.h"


VALUE dagon_constant_lookup(DagonEnv* env, const char *name);
VALUE dagon_local_lookup(DagonEnv* env, const char *name);

void dagon_constant_set(DagonEnv* env, const char *name, VALUE constant);
void dagon_local_set(DagonEnv* env, const char *name, VALUE value);
void dagon_error(DagonEnv* env, const char *error, ...);

DagonMethod* dagon_method_lookup(DagonEnv* env, VALUE klass, const char *method);

void Init_Object(DagonEnv *env);
void Init_String(DagonEnv *env);
void Init_Integer(DagonEnv *env);
void Init_Array(DagonEnv *env);
void Init_IO(DagonEnv *env);

DagonEnv* dagon_env_new(int argc, char* argv[]);
void dagon_env_destroy(DagonEnv* env);
VALUE dagon_run(DagonEnv* env, Node* node);

#endif
