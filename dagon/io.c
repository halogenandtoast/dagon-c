#include "dagon.h"
#include <stdlib.h>

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

void Init_IO(DagonEnv* env) {
  dg_cIO = dagon_class_new("IO", dg_cObject);
  dagon_class_set(env, "IO", dg_cIO);
  dagon_class_add_c_method(env, dg_cIO, "getc", dagon_io_get_c);
  dagon_class_add_c_method(env, dg_cIO, "print", dagon_io_print);
  dagon_class_add_c_method(env, dg_cIO, "puts", dagon_io_puts);
  dagon_class_add_c_method(env, dg_cIO, "gets", dagon_io_gets);
}
