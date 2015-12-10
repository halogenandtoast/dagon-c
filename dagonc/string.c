#include <string.h>
#include <stdlib.h>
#include "dagon.h"

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

VALUE dagon_string_plus(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonString* lhs = (DagonString*) self;
  DagonString* rhs = (DagonString*) values[0];

  int length = strlen(lhs->internal) + strlen(rhs->internal) + 1;
  char *new_string = malloc(sizeof(char) * length);
  strcpy(new_string, lhs->internal);
  strcat(new_string, rhs->internal);

  return dagon_string_new(env, new_string);
}

VALUE dagon_string_chomp(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonString* string = (DagonString*) self;
  int len = strlen(string->internal) - 1;
  char *new_string = malloc(sizeof(char) * len);
  strncpy(new_string, string->internal, len);
  new_string[len] = '\0';

  return dagon_string_new(env, new_string);
}

void Init_String(DagonEnv *env) {
  VALUE string = dagon_class_new("String", dg_cObject);
  dagon_class_set(env, "String", string);
  dagon_class_add_c_method(env, string, "length", dagon_string_length);
  dagon_class_add_c_method(env, string, "chomp", dagon_string_chomp);
  dagon_class_add_c_method(env, string, "[]", dagon_string_character_at);
  dagon_class_add_c_method(env, string, "=", dagon_string_compare);
  dagon_class_add_c_method(env, string, "to-s", dagon_string_to_s);
  dagon_class_add_c_method(env, string, "+", dagon_string_plus);
}
