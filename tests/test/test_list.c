#include "utility/list.h"
#include "unity.h"
#include "../gen/mocks/mock_istdlib.h"

#include <stdlib.h>
#include <string.h>

void test_newList_charSizeAsElementSize_shouldAllocateMemoryForChars() {
    List list;
    imalloc_ExpectAndReturn(5 * sizeof(char), malloc(5 * sizeof(char)));
    ifree_Ignore();

    const status result = list_new(&list, sizeof(char), 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list._size);
    TEST_ASSERT_EQUAL_INT(5, list._capacity);
    free(list._data);
    list_dispose(&list);
}

void test_newList_longLongSizeAsElementSize_shouldAllocateMemoryForLongLong() {
    List list;
    imalloc_ExpectAndReturn(5 * sizeof(long long), malloc(5 * sizeof(long long)));
    ifree_Ignore();

    const status result = list_new(&list, sizeof(long long), 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list._size);
    TEST_ASSERT_EQUAL_INT(5, list._capacity);
    free(list._data);
    list_dispose(&list);
}

void test_newList_initialSizeAndCapacityAreMessedUp_shouldRewriteSizeAndCapacity() {
    List list;
    list._size = 45;
    list._capacity = 78;
    list._elementSize = 90;
    imalloc_ExpectAndReturn(5 * sizeof(char), malloc(5 * sizeof(char)));
    ifree_Ignore();

    const status result = list_new(&list, sizeof(char), 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list._size);
    TEST_ASSERT_EQUAL_INT(5, list._capacity);
    TEST_ASSERT_EQUAL_INT(sizeof(char), list._elementSize);
    free(list._data);
    list_dispose(&list);
}

void test_newList_listPointerIsNull_shouldReturnError() {
    const status result = list_new(NULL, sizeof(char), 5);

    TEST_ASSERT(FAILED == result);
}

void test_newList_allocationFails_shouldReturnError() {
    List list;
    imalloc_IgnoreAndReturn(NULL);

    const status result = list_new(&list, sizeof(char), 5);

    TEST_ASSERT_EQUAL(FAILED, result);
}

void test_disposeList_anyList_shouldDispose() {
    List list;
    imalloc_ExpectAndReturn(5 * sizeof(char), malloc(5 * sizeof(char)));
    ifree_Ignore();

    list_new(&list, sizeof(char), 5);
    list_add(&list);
    free(list._data);
    const status result = list_dispose(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list._size);
    TEST_ASSERT_EQUAL_INT(0, list._capacity);
    TEST_ASSERT_EQUAL_INT(0, list._elementSize);
    TEST_ASSERT_EQUAL(NULL, list._data);
}

void test_disposeList_nullInsteadOfList_shouldFail() {
    const status result = list_dispose(NULL);

    TEST_ASSERT_EQUAL(FAILED, result);
}

void test_addElement_withoutExceedingCapacity_shouldAddWithoutExpanding() {
    List list;
    imalloc_ExpectAndReturn(5 * sizeof(char), malloc(5 * sizeof(char)));
    ifree_Ignore();

    list_new(&list, sizeof(char), 5);
    memset(list._data, 1, 5 * sizeof(char));
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(1, list._size);
    TEST_ASSERT_EQUAL_INT(5, list._capacity);
    TEST_ASSERT_EQUAL(0, ((char *)list._data)[0]);
    free(list._data);
    list_dispose(&list);
}

void test_addElement_exceedingCapacity_shouldAddAndExpand() {
    List list;
    char *data = malloc(5 * sizeof(char));
    imalloc_ExpectAndReturn(5 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), malloc(10 * sizeof(char)));
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 5);
    list._size = 5;
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(6, list._size);
    TEST_ASSERT_EQUAL_INT(10, list._capacity);

    list_dispose(&list);
    free(data);
}

void test_addElement_exceedingCapacityAndRealloctionFails_shouldFail() {
    List list;
    char *data = malloc(5 * sizeof(char));
    imalloc_ExpectAndReturn(5 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), NULL);
    ifree_Ignore();

    list_new(&list, sizeof(char), 5);
    list._size = 5;
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(FAILED, result);
    TEST_ASSERT_EQUAL_INT(5, list._size);
    TEST_ASSERT_EQUAL_INT(5, list._capacity);
    TEST_ASSERT_NOT_NULL(list._data);

    list_dispose(&list);
    free(list._data);
}

void test_addElement_nullPointerToList_shouldFail() {
    const status result = list_add(NULL);

    TEST_ASSERT_EQUAL(FAILED, result);
}

void test_addElement_listIsNotInitialized_shouldFail() {
    List list = {};

    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(FAILED, result);
}

void test_removeElement_withoutExceedingThreshold_shouldRemoveWithoutShrinking() {
    List list;
    imalloc_ExpectAndReturn(20 * sizeof(char), malloc(20 * sizeof(char)));
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 20;
    const status result = list_remove(&list, 19);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(19, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(list._data);
}

void test_removeElement_exceedingThreshold_shouldRemoveAndShrink() {
    List list;
    char *data = malloc(20 * sizeof(char));
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), malloc(10 * sizeof(char)));
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 6;
    const status result = list_remove(&list, 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(5, list._size);
    TEST_ASSERT_EQUAL_INT(10, list._capacity);
    list_dispose(&list);
    free(list._data);
    free(data);
}

void test_removeElement_zeroSize_shouldDoNothing() {
    List list;
    char *data = malloc(20 * sizeof(char));
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    const status result = list_remove(&list, 0);

    TEST_ASSERT_EQUAL(FAILED, result);
    TEST_ASSERT_EQUAL_INT(0, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(list._data);
}

void test_removeElement_outOfBounds_shouldFail() {
    List list;
    char *data = malloc(20 * sizeof(char));
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 20;
    const status result = list_remove(&list, 1032);
    const status result1 = list_remove(&list, -120);

    TEST_ASSERT_EQUAL(FAILED, result);
    TEST_ASSERT_EQUAL(FAILED, result1);
    TEST_ASSERT_EQUAL_INT(20, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(list._data);
}

void test_removeElement_listIsNull_shouldFail() {
    const status result = list_remove(NULL, 0);

    TEST_ASSERT_EQUAL(FAILED, result);
}

void test_removeElement_initiallySizeIsThree_shouldNotShrink() {
    List list;
    char *data = malloc(3 * sizeof(char));
    imalloc_ExpectAndReturn(3 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 3);
    list._size = 3;
    const status result = list_remove(&list, 2);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(2, list._size);
    TEST_ASSERT_EQUAL_INT(3, list._capacity);
    list_dispose(&list);
    free(list._data);
}

void test_removeElement_initiallyCapacityIsFourAndSizeIsTwo_shouldShrink() {
    List list;
    char *data = malloc(4 * sizeof(char));
    imalloc_ExpectAndReturn(4 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 2 * sizeof(char), malloc(2 * sizeof(char)));
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 4);
    list._size = 2;
    const status result = list_remove(&list, 1);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(1, list._size);
    TEST_ASSERT_EQUAL_INT(2, list._capacity);
    list_dispose(&list);
    free(list._data);
    free(data);
}

void test_removeElement_gapIsFormed_shouldMoveOtherElements() {
    List list;
    char *data = malloc(4 * sizeof(char));
    data[0] = 1;
    data[1] = 5;
    data[2] = 2;
    data[3] = 6;
    imalloc_ExpectAndReturn(4 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 4);
    list._size = 4;
    const status result = list_remove(&list, 1);

    char *expected = malloc(3 * sizeof(char));
    expected[0] = 1;
    expected[1] = 2;
    expected[2] = 6;
    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_MEMORY(expected, list._data, 3 * sizeof(char));
    TEST_ASSERT_EQUAL_INT(3, list._size);
    TEST_ASSERT_EQUAL_INT(4, list._capacity);
    list_dispose(&list);
    free(list._data);
    free(expected);
}

void test_removeElement_shrinkingFails_shouldFail() {
    List list;
    char *data = malloc(20 * sizeof(char));
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), NULL);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 6;
    const status result = list_remove(&list, 5);

    TEST_ASSERT_EQUAL(FAILED, result);
    TEST_ASSERT_EQUAL_INT(6, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(data);
}

void test_getElement_valid_shouldReturnElement() {
    List list;
    char *data = malloc(20 * sizeof(char));
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 10;
    const char *result = list_get(&list, 0);
    const char *result1 = list_get(&list, 1);
    const char *result2 = list_get(&list, 2);

    TEST_ASSERT_EQUAL_INT(1, *result);
    TEST_ASSERT_EQUAL_INT(2, *result1);
    TEST_ASSERT_EQUAL_INT(3, *result2);
    TEST_ASSERT_EQUAL_INT(10, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(data);
}

void test_getElement_outOfBounds_shouldFail() {
    List list;
    char *data = malloc(20 * sizeof(char));
    imalloc_ExpectAndReturn(20 * sizeof(char), data);
    ifree_ExpectAnyArgs();

    list_new(&list, sizeof(char), 20);
    list._size = 10;
    const char *result = list_get(&list, -2321);

    TEST_ASSERT_EQUAL_INT(NULL, result);
    TEST_ASSERT_EQUAL_INT(10, list._size);
    TEST_ASSERT_EQUAL_INT(20, list._capacity);
    list_dispose(&list);
    free(data);
}

void test_getElement_listIsNull_shouldFail() {
    const char *result = list_get(NULL, 0);

    TEST_ASSERT_EQUAL_INT(NULL, result);
}
