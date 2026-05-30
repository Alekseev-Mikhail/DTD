#ifndef DUMMY3D_LIST_H
#define DUMMY3D_LIST_H
#include <stddef.h>

#include "exception.h"

typedef struct {
    size_t size;
    size_t capacity;
    size_t elementSize;
    void *data;
} List;

status list_new(List *list, size_t elementSize, size_t capacity);

status list_dispose(List *list);

status list_add(List *list);

#endif //DUMMY3D_LIST_H
