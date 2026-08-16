
#include "unity.h"
#include "circ_buf.h"

#include <stdint.h>
#include <string.h>

/*
 * Buffer storage is intentionally 8 bytes because the implementation
 * rounds the capacity down to the nearest power of two.
 */
#define BUFFER_SIZE 8

static circ_buf_t cb;
static uint8_t storage[BUFFER_SIZE];

void setUp(void)
{
    memset(storage, 0, sizeof(storage));
    memset(&cb, 0, sizeof(cb));

    circ_buf_init(&cb,
                  storage,
                  sizeof(storage),
                  sizeof(uint8_t));
}

void tearDown(void)
{
    /* No cleanup required. */
}


/* ========================================================================== */
/* INIT TESTS                                                                 */
/* ========================================================================== */

void test_circ_buf_init_should_initialize_buffer(void)
{
    TEST_ASSERT_EQUAL_PTR(storage, cb.data);
    TEST_ASSERT_EQUAL_UINT(0, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);
    TEST_ASSERT_EQUAL_UINT(sizeof(uint8_t), cb.esize);

    /*
     * Capacity = 8
     * mask     = capacity - 1 = 7
     */
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE - 1, cb.mask);

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_FALSE(circ_buf_full(&cb));

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_capacity(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_avail(&cb));
}


void test_circ_buf_init_should_round_capacity_down_to_power_of_two(void)
{
    uint8_t local_storage[10];

    circ_buf_init(&cb,
                  local_storage,
                  sizeof(local_storage),
                  sizeof(uint8_t));

    /*
     * 10 bytes -> effective capacity = 8 bytes
     */
    TEST_ASSERT_EQUAL_UINT(8, circ_buf_capacity_uint8(&cb));
    TEST_ASSERT_EQUAL_UINT(8, circ_buf_capacity(&cb));
    TEST_ASSERT_EQUAL_UINT(7, cb.mask);
}


void test_circ_buf_init_should_store_item_size(void)
{
    uint32_t local_storage[4];

    circ_buf_init(&cb,
                  local_storage,
                  sizeof(local_storage),
                  sizeof(uint32_t));

    TEST_ASSERT_EQUAL_UINT(sizeof(uint32_t), cb.esize);

    /*
     * 16 bytes / 4 bytes per item = 4 items
     */
    TEST_ASSERT_EQUAL_UINT(4, circ_buf_capacity(&cb));
    TEST_ASSERT_EQUAL_UINT(16, circ_buf_capacity_uint8(&cb));
}


/* ========================================================================== */
/* PUSH TESTS                                                                 */
/* ========================================================================== */

void test_circ_buf_push_should_store_value(void)
{
    uint8_t value = 42;

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_push(&cb, &value));

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(1, circ_buf_size_uint8(&cb));

    TEST_ASSERT_EQUAL_HEX8(42, storage[0]);
}


void test_circ_buf_push_should_update_in_position(void)
{
    uint8_t value = 42;

    TEST_ASSERT_EQUAL_UINT(0, cb.in);

    circ_buf_push(&cb, &value);

    TEST_ASSERT_EQUAL_UINT(1, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);
}


void test_circ_buf_push_should_fail_when_full(void)
{
    uint8_t value = 0x55;

    for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_EQUAL_UINT(1, circ_buf_push(&cb, &value));
    }

    TEST_ASSERT_TRUE(circ_buf_full(&cb));
    TEST_ASSERT_EQUAL_UINT(0, circ_buf_avail(&cb));

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_push(&cb, &value));

    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_size(&cb));
}


void test_circ_buf_push_should_preserve_value(void)
{
    uint8_t value = 0xA5;

    circ_buf_push(&cb, &value);

    TEST_ASSERT_EQUAL_HEX8(0xA5, storage[0]);
}


/* ========================================================================== */
/* PUSH MANY TESTS                                                            */
/* ========================================================================== */

void test_circ_buf_push_many_should_store_multiple_values(void)
{
    uint8_t data[] = {10, 20, 30};

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_push_many(&cb, data, 3)
    );

    TEST_ASSERT_EQUAL_HEX8(10, storage[0]);
    TEST_ASSERT_EQUAL_HEX8(20, storage[1]);
    TEST_ASSERT_EQUAL_HEX8(30, storage[2]);

    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size(&cb));
}


void test_circ_buf_push_many_should_limit_data_to_available_space(void)
{
    uint8_t data[BUFFER_SIZE + 3];

    for (unsigned int i = 0; i < sizeof(data); i++) {
        data[i] = (uint8_t)i;
    }

    TEST_ASSERT_EQUAL_UINT(
        BUFFER_SIZE,
        circ_buf_push_many(&cb, data, sizeof(data))
    );

    TEST_ASSERT_TRUE(circ_buf_full(&cb));
    TEST_ASSERT_EQUAL_UINT(0, circ_buf_avail(&cb));

    for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
        TEST_ASSERT_EQUAL_HEX8(i, storage[i]);
    }
}


void test_circ_buf_push_many_should_accept_zero_items(void)
{
    uint8_t data[] = {1, 2, 3};

    TEST_ASSERT_EQUAL_UINT(
        0,
        circ_buf_push_many(&cb, data, 0)
    );

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_EQUAL_UINT(0, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);
}


/* ========================================================================== */
/* PUSH MANY UINT8 TESTS                                                      */
/* ========================================================================== */

void test_circ_buf_push_many_uint8_should_store_bytes(void)
{
    uint8_t data[] = {4, 5, 6, 7};

    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_push_many_uint8(&cb, data, 4)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        data,
        storage,
        4
    );

    TEST_ASSERT_EQUAL_UINT(4, circ_buf_size_uint8(&cb));
}


void test_circ_buf_push_many_uint8_should_limit_to_available_space(void)
{
    uint8_t data[BUFFER_SIZE + 2];

    for (unsigned int i = 0; i < sizeof(data); i++) {
        data[i] = (uint8_t)i;
    }

    TEST_ASSERT_EQUAL_UINT(
        BUFFER_SIZE,
        circ_buf_push_many_uint8(&cb, data, sizeof(data))
    );

    TEST_ASSERT_EQUAL_UINT(
        BUFFER_SIZE,
        circ_buf_size_uint8(&cb)
    );

    TEST_ASSERT_TRUE(circ_buf_full(&cb));
}


/* ========================================================================== */
/* POP TESTS                                                                  */
/* ========================================================================== */

void test_circ_buf_pop_should_return_oldest_value(void)
{
    uint8_t data[] = {5, 6};
    uint8_t value = 0;

    circ_buf_push_many(&cb, data, 2);

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_pop(&cb, &value));
    TEST_ASSERT_EQUAL_HEX8(5, value);

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_pop(&cb, &value));
    TEST_ASSERT_EQUAL_HEX8(6, value);
}


void test_circ_buf_pop_should_fail_when_empty(void)
{
    uint8_t value = 0xAA;

    TEST_ASSERT_EQUAL_UINT(
        0,
        circ_buf_pop(&cb, &value)
    );

    /*
     * Destination should not be modified when there is no data.
     */
    TEST_ASSERT_EQUAL_HEX8(0xAA, value);

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


void test_circ_buf_pop_should_decrease_buffer_size(void)
{
    uint8_t data[] = {1, 2, 3};
    uint8_t value;

    circ_buf_push_many(&cb, data, 3);

    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size(&cb));


    circ_buf_pop(&cb, &value);
    TEST_ASSERT_EQUAL_UINT(2, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(6, circ_buf_avail(&cb));
}


void test_circ_buf_pop_many_should_return_multiple_values(void)
{
    uint8_t input[] = {10, 20, 30, 40};
    uint8_t output[4] = {0};

    circ_buf_push_many(&cb, input, 4);

    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_pop_many(&cb, output, 4)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        4
    );

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


void test_circ_buf_pop_many_should_return_only_available_items(void)
{
    uint8_t input[] = {10, 20, 30};
    uint8_t output[5] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

    circ_buf_push_many(&cb, input, 3);

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_pop_many(&cb, output, 5)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        3
    );

    /*
     * Requested 5 items, but only 3 were available.
     */
    TEST_ASSERT_EQUAL_HEX8(0xAA, output[3]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, output[4]);

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


/* ========================================================================== */
/* FIFO TESTS                                                                 */
/* ========================================================================== */

void test_circ_buf_should_preserve_fifo_order(void)
{
    uint8_t input[] = {1, 2, 3, 4, 5};
    uint8_t output[5] = {0};

    circ_buf_push_many(&cb, input, 5);

    TEST_ASSERT_EQUAL_UINT(
        5,
        circ_buf_pop_many(&cb, output, 5)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        5
    );
}


void test_circ_buf_should_preserve_fifo_order_when_using_single_push_pop(void)
{
    uint8_t value;
    uint8_t input[] = {11, 22, 33, 44};

    for (unsigned int i = 0; i < sizeof(input); i++) {
        TEST_ASSERT_EQUAL_UINT(
            1,
            circ_buf_push(&cb, &input[i])
        );

        TEST_ASSERT_EQUAL_UINT(
            1,
            circ_buf_pop(&cb, &value)
        );

        TEST_ASSERT_EQUAL_HEX8(input[i], value);
    }

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


/* ========================================================================== */
/* WRAP-AROUND TESTS                                                          */
/* ========================================================================== */

void test_circ_buf_push_should_wrap_around(void)
{
    uint8_t input[BUFFER_SIZE];
    uint8_t output[BUFFER_SIZE];

    for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
        input[i] = (uint8_t)(i + 1);
    }

    /*
     * Fill buffer.
     */
    TEST_ASSERT_EQUAL_UINT(
        BUFFER_SIZE,
        circ_buf_push_many(&cb, input, BUFFER_SIZE)
    );

    /*
     * Remove first 4 items.
     */
    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_pop_many(&cb, output, 4)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        4
    );

    /*
     * There are now 4 free bytes.
     *
     * These writes must wrap around to the beginning
     * of the physical storage.
     */
    {
        uint8_t wrapped[] = {9, 10, 11, 12};

        TEST_ASSERT_EQUAL_UINT(
            4,
            circ_buf_push_many(&cb, wrapped, 4)
        );
    }

    /*
     * Expected FIFO order:
     *
     * old: 5, 6, 7, 8
     * new: 9, 10, 11, 12
     */
    TEST_ASSERT_EQUAL_UINT(
        BUFFER_SIZE,
        circ_buf_pop_many(&cb, output, BUFFER_SIZE)
    );

    {
        uint8_t expected[] = {
            5, 6, 7, 8,
            9, 10, 11, 12
        };

        TEST_ASSERT_EQUAL_HEX8_ARRAY(
            expected,
            output,
            BUFFER_SIZE
        );
    }

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


void test_circ_buf_pop_should_wrap_around(void)
{
    uint8_t input[] = {1, 2, 3, 4, 5, 6};
    uint8_t output[6] = {0};

    /*
     * First push 6 items.
     */
    TEST_ASSERT_EQUAL_UINT(
        6,
        circ_buf_push_many(&cb, input, 6)
    );

    /*
     * Pop 6 items. This moves out forward.
     */
    TEST_ASSERT_EQUAL_UINT(
        6,
        circ_buf_pop_many(&cb, output, 6)
    );

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));

    /*
     * Push again. Physical position must wrap.
     */
    TEST_ASSERT_EQUAL_UINT(
        6,
        circ_buf_push_many(&cb, input, 6)
    );

    memset(output, 0, sizeof(output));

    TEST_ASSERT_EQUAL_UINT(
        6,
        circ_buf_pop_many(&cb, output, 6)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        6
    );
}


/* ========================================================================== */
/* EMPTY / FULL / SIZE / CAPACITY / AVAILABLE                                 */
/* ========================================================================== */

void test_circ_buf_empty_and_full_flags(void)
{
    uint8_t value = 1;

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_FALSE(circ_buf_full(&cb));

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_capacity(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_avail(&cb));

    for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
        circ_buf_push(&cb, &value);
    }

    TEST_ASSERT_FALSE(circ_buf_empty(&cb));
    TEST_ASSERT_TRUE(circ_buf_full(&cb));

    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_capacity(&cb));
    TEST_ASSERT_EQUAL_UINT(0, circ_buf_avail(&cb));
}


void test_circ_buf_size_should_be_zero_after_all_items_are_popped(void)
{
    uint8_t input[] = {1, 2, 3, 4};
    uint8_t output[4];

    circ_buf_push_many(&cb, input, 4);
    circ_buf_pop_many(&cb, output, 4);

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size_uint8(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_avail(&cb));
    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    TEST_ASSERT_FALSE(circ_buf_full(&cb));
}


void test_circ_buf_uint8_size_capacity_and_available_should_use_bytes(void)
{
    uint8_t input[] = {1, 2, 3};

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size_uint8(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_capacity_uint8(&cb));
    TEST_ASSERT_EQUAL_UINT(BUFFER_SIZE, circ_buf_avail_uint8(&cb));

    circ_buf_push_many_uint8(
        &cb,
        input,
        sizeof(input)
    );

    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size_uint8(&cb));
    TEST_ASSERT_EQUAL_UINT(5, circ_buf_avail_uint8(&cb));

    /*
     * Since esize == 1, item count == byte count.
     */
    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size(&cb));
    TEST_ASSERT_EQUAL_UINT(5, circ_buf_avail(&cb));
}


/* ========================================================================== */
/* MULTI-BYTE ITEM TESTS                                                      */
/* ========================================================================== */

void test_circ_buf_should_support_multi_byte_items(void)
{
    uint32_t storage32[4];
    uint32_t input[] = {
        0x11223344,
        0x55667788,
        0xAABBCCDD
    };
    uint32_t output[3] = {0};

    circ_buf_init(
        &cb,
        storage32,
        sizeof(storage32),
        sizeof(uint32_t)
    );

    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_capacity(&cb)
    );

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_push_many(&cb, input, 3)
    );

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_size(&cb)
    );

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_pop_many(&cb, output, 3)
    );

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        input,
        output,
        3
    );

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


void test_circ_buf_multi_byte_items_should_not_split_items(void)
{
    uint32_t storage32[4];
    uint32_t input[] = {
        0x11111111,
        0x22222222,
        0x33333333,
        0x44444444,
        0x55555555
    };
    uint32_t output[5] = {0};

    circ_buf_init(
        &cb,
        storage32,
        sizeof(storage32),
        sizeof(uint32_t)
    );

    /*
     * Storage = 16 bytes = 4 uint32_t items.
     */
    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_push_many(&cb, input, 5)
    );

    TEST_ASSERT_TRUE(circ_buf_full(&cb));
    TEST_ASSERT_EQUAL_UINT(4, circ_buf_size(&cb));

    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_pop_many(&cb, output, 5)
    );

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        input,
        output,
        4
    );

    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


/* ========================================================================== */
/* OUT PEEK TESTS                                                             */
/* ========================================================================== */

void test_circ_buf_out_peek_should_read_without_removing_data(void)
{
    uint8_t input[] = {1, 2, 3, 4};
    uint8_t output[2] = {0};

    circ_buf_push_many(&cb, input, 4);

    TEST_ASSERT_EQUAL_UINT(
        2,
        circ_buf_out_peek(&cb, output, 2)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        2
    );

    /*
     * Peek must not modify the number of stored bytes.
     */
    TEST_ASSERT_EQUAL_UINT(4, circ_buf_size(&cb));
    TEST_ASSERT_FALSE(circ_buf_empty(&cb));
}


void test_circ_buf_out_peek_should_not_read_more_than_available(void)
{
    uint8_t input[] = {10, 20, 30};
    uint8_t output[5] = {
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA
    };

    circ_buf_push_many(&cb, input, 3);

    TEST_ASSERT_EQUAL_UINT(
        3,
        circ_buf_out_peek(&cb, output, 5)
    );

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
        input,
        output,
        3
    );

    TEST_ASSERT_EQUAL_HEX8(0xAA, output[3]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, output[4]);

    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size(&cb));
}


void test_circ_buf_out_peek_on_empty_buffer_should_return_zero(void)
{
    uint8_t output = 0xAA;

    TEST_ASSERT_EQUAL_UINT(
        0,
        circ_buf_out_peek(&cb, &output, 1)
    );

    TEST_ASSERT_EQUAL_HEX8(0xAA, output);
}


/* ========================================================================== */
/* PEEK + WRAP-AROUND                                                         */
/* ========================================================================== */

void test_circ_buf_out_peek_should_work_across_wrap_around(void)
{
    uint8_t input1[] = {1, 2, 3, 4, 5, 6};
    uint8_t input2[] = {7, 8, 9, 10};
    uint8_t output[8] = {0};

    /*
     * Fill first 6 bytes.
     */
    circ_buf_push_many(&cb, input1, 6);

    /*
     * Remove first 4 bytes.
     * out is now at byte 4.
     */
    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_pop_many(&cb, output, 4)
    );

    /*
     * Add 4 more bytes. This wraps around.
     */
    TEST_ASSERT_EQUAL_UINT(
        4,
        circ_buf_push_many(&cb, input2, 4)
    );

    /*
     * Buffer now contains:
     *
     * 5, 6, 7, 8, 9, 10
     */
    memset(output, 0, sizeof(output));

    TEST_ASSERT_EQUAL_UINT(
        6,
        circ_buf_out_peek(&cb, output, 6)
    );

    {
        uint8_t expected[] = {
            5, 6, 7, 8, 9, 10
        };

        TEST_ASSERT_EQUAL_HEX8_ARRAY(
            expected,
            output,
            6
        );
    }

    /*
     * Peek must not remove the data.
     */
    TEST_ASSERT_EQUAL_UINT(6, circ_buf_size(&cb));
}


/* ========================================================================== */
/* PUSH/POP AFTER WRAP-AROUND                                                 */
/* ========================================================================== */

void test_circ_buf_should_continue_working_after_multiple_wraps(void)
{
    uint8_t value;
    uint8_t input[] = {10, 20, 30, 40};

    /*
     * Repeat push/pop cycles so in/out become larger than mask
     * and the physical storage is accessed through wrapping.
     */
    for (unsigned int cycle = 0; cycle < 20; cycle++) {
        TEST_ASSERT_EQUAL_UINT(
            4,
            circ_buf_push_many(&cb, input, 4)
        );

        TEST_ASSERT_EQUAL_UINT(
            4,
            circ_buf_size(&cb)
        );

        for (unsigned int i = 0; i < 4; i++) {
            TEST_ASSERT_EQUAL_UINT(
                1,
                circ_buf_pop(&cb, &value)
            );

            TEST_ASSERT_EQUAL_HEX8(
                input[i],
                value
            );
        }

        TEST_ASSERT_TRUE(circ_buf_empty(&cb));
    }
}


/* ========================================================================== */
/* ZERO LENGTH TESTS                                                          */
/* ========================================================================== */

void test_circ_buf_push_many_uint8_zero_bytes_should_do_nothing(void)
{
    uint8_t input[] = {1, 2, 3};

    TEST_ASSERT_EQUAL_UINT(
        0,
        circ_buf_push_many_uint8(&cb, input, 0)
    );

    TEST_ASSERT_EQUAL_UINT(0, circ_buf_size_uint8(&cb));
    TEST_ASSERT_TRUE(circ_buf_empty(&cb));
}


void test_circ_buf_pop_many_zero_items_should_do_nothing(void)
{
    uint8_t input[] = {1, 2, 3};
    uint8_t output = 0xAA;

    circ_buf_push_many(&cb, input, 3);

    TEST_ASSERT_EQUAL_UINT(
        0,
        circ_buf_pop_many(&cb, &output, 0)
    );

    TEST_ASSERT_EQUAL_HEX8(0xAA, output);
    TEST_ASSERT_EQUAL_UINT(3, circ_buf_size(&cb));
}


/* ========================================================================== */
/* INTERNAL POSITION TESTS                                                    */
/* ========================================================================== */

void test_circ_buf_in_and_out_should_count_bytes(void)
{
    uint8_t input[] = {1, 2, 3};
    uint8_t output[2];

    TEST_ASSERT_EQUAL_UINT(0, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);

    circ_buf_push_many(&cb, input, 3);

    TEST_ASSERT_EQUAL_UINT(3, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);

    circ_buf_pop_many(&cb, output, 2);

    TEST_ASSERT_EQUAL_UINT(3, cb.in);
    TEST_ASSERT_EQUAL_UINT(2, cb.out);

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_size_uint8(&cb));
}


void test_circ_buf_multi_byte_in_and_out_should_count_bytes(void)
{
    uint32_t storage32[4];
    uint32_t input[] = {
        0x11111111,
        0x22222222
    };
    uint32_t output;

    circ_buf_init(
        &cb,
        storage32,
        sizeof(storage32),
        sizeof(uint32_t)
    );

    circ_buf_push_many(&cb, input, 2);

    /*
     * 2 items * 4 bytes = 8 bytes.
     */
    TEST_ASSERT_EQUAL_UINT(8, cb.in);
    TEST_ASSERT_EQUAL_UINT(0, cb.out);

    circ_buf_pop(&cb, &output);

    TEST_ASSERT_EQUAL_UINT(8, cb.in);
    TEST_ASSERT_EQUAL_UINT(4, cb.out);

    TEST_ASSERT_EQUAL_UINT(1, circ_buf_size(&cb));
}

