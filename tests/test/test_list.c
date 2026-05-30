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
    TEST_ASSERT_EQUAL_INT(0, list.size);
    TEST_ASSERT_EQUAL_INT(5, list.capacity);
    free(list.data);
    list_dispose(&list);
}

void test_newList_longLongSizeAsElementSize_shouldAllocateMemoryForLongLong() {
    List list;
    imalloc_ExpectAndReturn(5 * sizeof(long long), malloc(5 * sizeof(long long)));
    ifree_Ignore();

    const status result = list_new(&list, sizeof(long long), 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list.size);
    TEST_ASSERT_EQUAL_INT(5, list.capacity);
    free(list.data);
    list_dispose(&list);
}

void test_newList_initialSizeAndCapacityAreMessedUp_shouldRewriteSizeAndCapacity() {
    List list;
    list.size = 45;
    list.capacity = 78;
    list.elementSize = 90;
    imalloc_ExpectAndReturn(5 * sizeof(char), malloc(5 * sizeof(char)));
    ifree_Ignore();

    const status result = list_new(&list, sizeof(char), 5);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list.size);
    TEST_ASSERT_EQUAL_INT(5, list.capacity);
    TEST_ASSERT_EQUAL_INT(sizeof(char), list.elementSize);
    free(list.data);
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
    free(list.data);
    const status result = list_dispose(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(0, list.size);
    TEST_ASSERT_EQUAL_INT(0, list.capacity);
    TEST_ASSERT_EQUAL_INT(0, list.elementSize);
    TEST_ASSERT_EQUAL(NULL, list.data);
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
    memset(list.data, 1, 5 * sizeof(char));
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(1, list.size);
    TEST_ASSERT_EQUAL_INT(5, list.capacity);
    TEST_ASSERT_EQUAL(0, ((char *)list.data)[0]);
    free(list.data);
    list_dispose(&list);
}

void test_addElement_exceedingCapacity_shouldAddAndExpand() {
    List list;
    char *data = malloc(5 * sizeof(char));
    imalloc_ExpectAndReturn(5 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), realloc(data, 10 * sizeof(char)));
    ifree_Ignore();

    list_new(&list, sizeof(char), 5);
    list.size = 5;
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(COMPLETED, result);
    TEST_ASSERT_EQUAL_INT(6, list.size);
    TEST_ASSERT_EQUAL_INT(10, list.capacity);

    free(list.data);
    list_dispose(&list);
}

void test_addElement_exceedingCapacityAndRealloctionFails_shouldFail() {
    List list;
    char *data = malloc(5 * sizeof(char));
    imalloc_ExpectAndReturn(5 * sizeof(char), data);
    irealloc_ExpectAndReturn(data, 10 * sizeof(char), NULL);
    ifree_Ignore();

    list_new(&list, sizeof(char), 5);
    list.size = 5;
    const status result = list_add(&list);

    TEST_ASSERT_EQUAL(FAILED, result);
    TEST_ASSERT_EQUAL_INT(5, list.size);
    TEST_ASSERT_EQUAL_INT(5, list.capacity);
    TEST_ASSERT_NOT_NULL(list.data);

    free(list.data);
    list_dispose(&list);
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
}

void test_removeElement_exceedingThreshold_shouldRemoveAndShrink() {
}

void test_removeElement_withZeroSize_shouldDoNothing() {
}
