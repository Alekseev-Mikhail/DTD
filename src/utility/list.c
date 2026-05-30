#include "list.h"

#include <string.h>

#include "istdlib.h"

const char EXPANDING_FACTOR = 2;

status list_new(List *list, const size_t elementSize, const size_t capacity) {
    if (NULL == list) return FAILED;

    list->size = 0;
    list->capacity = capacity;
    list->elementSize = elementSize;

    list->data = imalloc(capacity * elementSize);
    if (NULL == list->data) return FAILED;
    return COMPLETED;
}

status list_dispose(List *list) {
    if (NULL == list) return FAILED;
    list->size = 0;
    list->capacity = 0;
    list->elementSize = 0;
    ifree(list->data);
    list->data = NULL;
    return COMPLETED;
}

status list_add(List *list) {
    if (NULL == list || NULL == list->data) return FAILED;

    if (list->size >= list->capacity) {
        const size_t newCapacity = list->capacity * EXPANDING_FACTOR;
        void *ptr = irealloc(list->data, newCapacity * list->elementSize);
        if (NULL == ptr) return FAILED;
        list->data = ptr;
        list->capacity = newCapacity;
    }

    memset((char *)list->data + list->size * list->elementSize, 0, sizeof(char));
    list->size++;
    return COMPLETED;
}
