#include "dagon.h"
#include <stdlib.h>

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

void Init_Integer(DagonEnv* env) {
  VALUE integer = dagon_class_new("Integer", dg_cObject);
  dagon_class_set(env, "Integer", integer);
  dagon_class_add_c_method(env, integer, "<", dagon_integer_lt);
  dagon_class_add_c_method(env, integer, "+", dagon_integer_plus);
  dagon_class_add_c_method(env, integer, "-", dagon_integer_minus);
  dagon_class_add_c_method(env, integer, "=", dagon_integer_eq);
  dagon_class_add_c_method(env, integer, "chr", dagon_integer_chr);
  dagon_class_add_c_method(env, integer, "to-s", dagon_integer_to_s);
}
