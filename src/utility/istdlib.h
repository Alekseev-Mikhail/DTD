#ifndef SIMPLE_CUBE_ISTDLIB_H
#define SIMPLE_CUBE_ISTDLIB_H
#include <stddef.h>

void *imalloc(size_t size);

void *icalloc(size_t numOfElements, size_t sizeOfElement);

void *irealloc(void *memory, size_t newSize);

void ifree(void *memory);

#endif //SIMPLE_CUBE_ISTDLIB_H
