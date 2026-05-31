#include "list.h"

#include <string.h>

#include "istdlib.h"

const char EXPANDING_FACTOR = 2;
const char SHRINKING_THRESHOLD = 4;
const char SHRINKING_FACTOR = 2;

static status resizeList(List *list, size_t newCapacity);

status list_new(List *list, const size_t elementSize, const size_t capacity) {
    if (NULL == list) return FAILED;

    list->_size = 0;
    list->_capacity = capacity;
    list->_elementSize = elementSize;

    list->_data = imalloc(capacity * elementSize);
    if (NULL == list->_data) return FAILED;
    return COMPLETED;
}

status list_dispose(List *list) {
    if (NULL == list) return FAILED;
    list->_size = 0;
    list->_capacity = 0;
    list->_elementSize = 0;
    ifree(list->_data);
    list->_data = NULL;
    return COMPLETED;
}

status list_add(List *list) {
    if (NULL == list || NULL == list->_data) return FAILED;

    if (list->_size >= list->_capacity) {
        if (FAILED == resizeList(list, list->_capacity * EXPANDING_FACTOR)) return FAILED;
    }

    memset((char *) list->_data + list->_size * list->_elementSize, 0, sizeof(char));
    list->_size++;
    return COMPLETED;
}

status list_remove(List *list, const size_t index) {
    if (NULL == list) return FAILED;
    if (index >= list->_size) return FAILED;

    if (list->_size - 1 <= list->_capacity / SHRINKING_THRESHOLD) {
        if (FAILED == resizeList(list, list->_capacity / SHRINKING_FACTOR)) return FAILED;
    }
    list->_size--;

    if (index != list->_size) {
        memcpy(
            (char *) list->_data + index * list->_elementSize,
            (char *) list->_data + (index + 1) * list->_elementSize,
            (list->_size - index) * list->_elementSize
        );
    }
    return COMPLETED;
}

void *list_get(const List *list, const size_t index) {
    if (NULL == list || index >= list->_size) return NULL;
    return (char *)list->_data + index * list->_elementSize;
}

static status resizeList(List *list, const size_t newCapacity) {
    void *ptr = irealloc(list->_data, newCapacity * list->_elementSize);
    if (NULL == ptr) return FAILED;
    list->_data = ptr;
    list->_capacity = newCapacity;
    return COMPLETED;
}
