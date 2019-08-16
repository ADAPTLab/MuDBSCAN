#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INIT_CAPACITY 4
#include "Def.h"
#include <stdbool.h>

//vec is of type vectorc*
//type signifies whether it is an integer vector or Group* vector
void vector_init(vectorc*, VECTORTYPE type);
int vector_total(vectorc*);
void vector_resize(vectorc* v, int, void*);
void vector_add(vectorc*, void*);
void vector_set(vectorc*, int, int);
void *vector_get(vectorc*, int);
void vector_delete(vectorc*, int);
void vector_free(vectorc*);
bool vector_isEmpty(vectorc* v);
int vector_type(vectorc* v);
void vector_reserve(vectorc* v, int);

#define VECTOR_INIT(vec, type) vector_init(vec, (VECTORTYPE)type)
#define VECTOR_ADD(vec, item) vector_add(vec, (void*) item)
#define VECTOR_SET(vec, id, item) vector_set(vec, id, (void *) item)
#define VECTOR_GET(vec, type, id) (type) vector_get(vec, id)
#define VECTOR_DELETE(vec, id) vector_delete(vec, id)
#define VECTOR_TOTAL(vec) vector_total(vec)
#define VECTOR_FREE(vec) vector_free(vec)
#define VECTOR_ISEMPTY(vec) vector_isEmpty(vec)
#define VECTOR_TYPE(vec) vector_type(vec)
#define VECTOR_RESIZE(vec, size, val) vector_resize(vec, (int)size, (void*) val)
#define VECTOR_RESERVE(vec, size) vector_reserve(vec, (int)size)

#endif
