#include "dagon.h"
#include <stdlib.h>

VALUE dagon_array_get(DagonEnv *env, DagonArray* array, int index) {
  DagonListNode* item = array->head;
  for(int i = 0; i < index; i++) {
    item = item->next;
  }
  return item->current.value;
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
      current->current.value = dagon_array_get(env, array, j);
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
  item->current.value = value;
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
    array->head->current.value = values[0];
  } else {
    DagonListNode* item = array->head;
    for(int i = 0; i < length - 1; i++) {
      item = item->next;
    }

    item->next = malloc(sizeof(DagonListNode));
    item->next->current.value = values[0];
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

VALUE dagon_array_is_empty(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  return array->len == 0;
}

VALUE dagon_array_drop(DagonEnv *env, VALUE self, int argc, VALUE* values) {
  DagonArray *array = (DagonArray*) self;
  DagonListNode* current = array->head;
  int count = FIX2INT(values[0]);
  while(count > 0 && current != NULL) {
    current = current->next;
    count--;
  }

  VALUE ret = dagon_array_new(env);

  if(current) {
    ((DagonArray*) ret)->head = current;
  }

  return ret;
}

void Init_Array(DagonEnv* env) {
  VALUE array = dagon_class_new("Array", dg_cObject);
  dagon_class_set(env, "Array", array);
  dagon_class_add_c_method(env, array, "*", dagon_array_mult);
  dagon_class_add_c_method(env, array, "[]=", dagon_array_assign);
  dagon_class_add_c_method(env, array, "[]", dagon_array_ref);
  dagon_class_add_c_method(env, array, "push", dagon_array_push);
  dagon_class_add_c_method(env, array, "pop", dagon_array_pop);
  dagon_class_add_c_method(env, array, "last", dagon_array_last);
  dagon_class_add_c_method(env, array, "to-s", dagon_array_to_s);
  dagon_class_add_c_method(env, array, "drop", dagon_array_drop);
  dagon_class_add_c_method(env, array, "empty?", dagon_array_is_empty);
}
