#include "istdlib.h"

#include <stdlib.h>

void *imalloc(const size_t size) {
    return malloc(size);
}

void *icalloc(const size_t numOfElements, const size_t sizeOfElement) {
    return calloc(numOfElements, sizeOfElement);
}

void *irealloc(void *memory, const size_t newSize) {
    return realloc(memory, newSize);
}

void ifree(void *memory) {
    free(memory);
}
