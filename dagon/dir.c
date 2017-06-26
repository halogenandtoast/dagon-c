#include "dagon.h"
#include <stdlib.h>

VALUE dagon_dir_init(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  dagon_object_ivar_set(env, self, "root", values[0]);
  return self;
}

VALUE dagon_dir_glob(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  return (VALUE) 0;
}

void Init_Dir(DagonEnv* env) {
  VALUE dir = dagon_class_new("Dir", dg_cObject);
  dagon_class_set(env, "Dir", dir);
  dagon_class_add_c_method(env, dir, "init", dagon_dir_init);
  dagon_class_add_c_method(env, dir, "glob", dagon_dir_glob);
}
