#ifndef _STDLIB_H_
#define _STDLIB_H_

#include "rtos.h"

void *malloc(size_t);
void *zalloc(size_t);
void *calloc(size_t num, size_t size);
void free(void *);


#endif



