#ifndef DUMMY3D_LIST_H
#define DUMMY3D_LIST_H
#include <stddef.h>

#include "exception.h"

typedef struct {
    size_t _size;
    size_t _capacity;
    size_t _elementSize;
    void *_data;
} List;

/**
 * Initializes a new list with specified element size and capacity.
 * Allocates memory for storing the list elements and configures the list's metadata.
 *
 * @param list Pointer to the list that will be initialized. Must not be NULL.
 * @param elementSize The size of each individual element in the list, in bytes.
 * @param capacity The maximum number of elements the list can initially hold.
 * @return COMPLETED if the list is successfully initialized; FAILED if the list pointer is NULL or if memory allocation fails.
 */
status list_new(List *list, size_t elementSize, size_t capacity);

/**
 * Disposes of the specified list, releasing all associated resources and resetting its metadata.
 * Frees the memory allocated for the list's data and sets the list's size, capacity,
 * and element size to zero. The list data pointer is set to NULL.
 *
 * @param list Pointer to the list to be disposed.
 * @return COMPLETED if the list is successfully disposed; FAILED if the list pointer is NULL.
 */
status list_dispose(List *list);

/**
 * Adds a new element to the list. If the current size of the list exceeds its capacity,
 * the capacity is expanded to accommodate the new element. Memory for the new element
 * is initialized to zero.
 *
 * @param list Pointer to the list where the new element is added. Must not be NULL and must point to a valid list with initialized data.
 * @return COMPLETED if the element is successfully added; FAILED if the list pointer is NULL, the list data is NULL, or if expanding the list capacity fails.
 */
status list_add(List *list);

/**
 * Removes an element from the list at the specified index. If necessary, the
 * list's capacity is reduced to optimize memory usage based on the shrinking threshold.
 *
 * @param list Pointer to the list from which the element is to be removed. Must not be NULL.
 * @param index The index of the element to remove. Must be within the bounds of the list.
 * @return COMPLETED if the element is successfully removed; FAILED if the list pointer is NULL, if the index is out of bounds, or if resizing the list fails.
 */
status list_remove(List *list, size_t index);

/**
 * Retrieves a pointer to the element at the specified index in the list.
 *
 * @param list Pointer to the List. Must not be NULL.
 * @param index The zero-based index of the element to retrieve. Must be less than the size of the list.
 * @return Pointer to the element at the specified index, or NULL if the list is NULL or the index is out of bounds.
 */
void *list_get(const List *list, size_t index);

#endif //DUMMY3D_LIST_H
