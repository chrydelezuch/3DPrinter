#include "unity.h"
#include "circular_buffer.h"

static circ_buf_t cb;
static buffer_type storage[5];  // buffer na 5 elementów

void setUp(void) {
    // reset buffer przed każdym testem
    circ_buf_reset(&cb);
}

void tearDown(void) {
    // brak cleanup potrzebnego
}

// ------------------- INIT TESTS -------------------

void test_circ_buf_init_should_succeed(void) {
    int ret = circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_PTR(storage, cb.buffer);
    TEST_ASSERT_EQUAL_INT(0, cb.head);
    TEST_ASSERT_EQUAL_INT(0, cb.tail);
    TEST_ASSERT_EQUAL_INT(5, cb.max_len);
}

void test_circ_buf_init_should_fail_on_invalid_args(void) {
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_init(NULL, storage, sizeof(storage)/sizeof(buffer_type)));
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_init(&cb, NULL, sizeof(storage)/sizeof(buffer_type)));
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_init(&cb, storage, 0));
}

// ------------------- PUSH TESTS -------------------

void test_circ_buf_push_should_store_value(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    TEST_ASSERT_EQUAL_INT(0, circ_buf_push(&cb, 42));
    TEST_ASSERT_EQUAL_UINT32(42, storage[0]);
    TEST_ASSERT_EQUAL_INT(1, cb.head);
}

void test_circ_buf_push_should_fail_when_full(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    circ_buf_push(&cb, 1);
    circ_buf_push(&cb, 2);
    circ_buf_push(&cb, 3);
    circ_buf_push(&cb, 4);
    circ_buf_push(&cb, 5);


    TEST_ASSERT_TRUE(circ_buf_full(&cb));
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_push(&cb, 5));
}

// ------------------- PUSH MANY TESTS -------------------

void test_circ_buf_push_many_should_store_multiple_values(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    buffer_type data[3] = {10, 20, 30};
    TEST_ASSERT_EQUAL_INT(0, circ_buf_push_many(&cb, data, 3));

    TEST_ASSERT_EQUAL_UINT32(10, storage[0]);
    TEST_ASSERT_EQUAL_UINT32(20, storage[1]);
    TEST_ASSERT_EQUAL_UINT32(30, storage[2]);
}

void test_circ_buf_push_many_should_fail_if_not_enough_space(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    buffer_type data[6] = {1,2,3,4,5,6};
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_push_many(&cb, data, 6));
}

// ------------------- PUSH MANY UINT8 TEST -------------------
/*
void test_circ_buf_push_many_uint8_should_store_values(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    uint8_t data[8] = {4,0,0,0,0,0,0,0}; // 8 bytes = 2 buffer_type
    TEST_ASSERT_EQUAL_INT(0, circ_buf_push_many_uint8(&cb, data, 8));


    TEST_ASSERT_EQUAL_INT(4, storage[0]);
    TEST_ASSERT_EQUAL_INT(0, storage[1]);

}
*/
// ------------------- POP TESTS -------------------

void test_circ_buf_pop_should_return_oldest_value(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    circ_buf_push(&cb, 5);
    circ_buf_push(&cb, 6);

    buffer_type val;
    TEST_ASSERT_EQUAL_INT(0, circ_buf_pop(&cb, &val));
    TEST_ASSERT_EQUAL_INT(5, val);

    TEST_ASSERT_EQUAL_INT(0, circ_buf_pop(&cb, &val));
    TEST_ASSERT_EQUAL_INT(6, val);
}

void test_circ_buf_pop_should_fail_when_empty(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    buffer_type val;
    TEST_ASSERT_EQUAL_INT(-1, circ_buf_pop(&cb, &val));
}

// ------------------- WRAP-AROUND TEST -------------------

void test_circ_buf_push_wrap_around(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    circ_buf_push(&cb, 1);
    circ_buf_push(&cb, 2);
    circ_buf_push(&cb, 3);
    circ_buf_push(&cb, 4);


    buffer_type tmp;
    circ_buf_pop(&cb, &tmp); // remove 1

    circ_buf_push(&cb, 5); // should wrap head to 0

    TEST_ASSERT_EQUAL_INT(0, cb.head);
    TEST_ASSERT_EQUAL_INT(2, storage[1]);
    TEST_ASSERT_EQUAL_INT(3, storage[2]);
}

// ------------------- EMPTY / FULL / SIZE / CAPACITY -------------------

void test_circ_buf_empty_and_full_flags(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_FALSE(circ_buf_full(&cb));

    circ_buf_push(&cb, 1);
    circ_buf_push(&cb, 2);
    circ_buf_push(&cb, 3);
    circ_buf_push(&cb, 4);

    TEST_ASSERT_TRUE(circ_buf_full(&cb));
    TEST_ASSERT_FALSE(circ_buf_empty(&cb));
    TEST_ASSERT_EQUAL_UINT32(4, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT32(4, circ_buf_capacity(&cb));
}

// ------------------- RESET TEST -------------------

void test_circ_buf_reset_should_clear_buffer(void) {
    circ_buf_init(&cb, storage, sizeof(storage)/sizeof(buffer_type));
    circ_buf_push(&cb, 1);
    circ_buf_push(&cb, 2);

    circ_buf_reset(&cb);

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_EQUAL_INT(0, cb.head);
    TEST_ASSERT_EQUAL_INT(0, cb.tail);
}
