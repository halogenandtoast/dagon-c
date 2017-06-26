#include "dagon.h"
#include "env.h"
#include <stdlib.h>

VALUE dagon_object_alloc(VALUE klass) {
  DagonObject* object = malloc(sizeof(DagonObject));
  object->header.klass = klass;
  object->header.ivars = dagon_list_new();
  return (VALUE) object;
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

VALUE dagon_object_ivar_set(DagonEnv *env, VALUE self, const char* name, VALUE value) {
  DagonObject* obj = (DagonObject*) self;
  dagon_list_append(obj->header.ivars, "root", value);
  return value;
}

VALUE dagon_object_require(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  return (VALUE) 0;
}

void Init_Object(DagonEnv* env) {
  dg_cObject = (VALUE) malloc(sizeof(DagonClass));
  ((DagonClass*) dg_cObject)->name = "Object";
  ((DagonClass*) dg_cObject)->methods = NULL;
  ((DagonClass*) dg_cObject)->parent = 0;
  dagon_class_set(env, "Object", dg_cObject);
  dagon_class_add_c_method(env, dg_cObject, "print", dagon_object_print);
  dagon_class_add_c_method(env, dg_cObject, "puts", dagon_object_puts);
  dagon_class_add_c_method(env, dg_cObject, "gets", dagon_object_gets);
  dagon_class_add_c_method(env, dg_cObject, "require", dagon_object_require);
  dagon_class_add_c_method(env, dg_cObject, "load", dagon_object_require);
}
