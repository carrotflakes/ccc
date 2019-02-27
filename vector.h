#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

#endif
