#include "unity.h"
#include "usb_praser.h"
#include "fake_axis.h"          /* teraz to stub z test/support/inc/ */
#include "circ_buf.h"
#include "mock_debug.h"

#define AXIS_BUF_SIZE   64U
#define HEADER_BUF_SIZE 16U

static uint8_t axis_mem[AXIS_COUNT][AXIS_BUF_SIZE];
static uint8_t generic_mem[AXIS_BUF_SIZE];
static uint8_t header_mem[HEADER_BUF_SIZE];

static circ_buf_t header_buf;
static circ_buf_t generic_buf;

void setUp(void)
{
    /* realny circ_buf — nie mockujemy go, bo usb_praser go używa */
    for (int i = 0; i < AXIS_COUNT; i++) {
        cbuf_handle_t h = circ_buf_init(axis_mem[i], AXIS_BUF_SIZE);
        axis_map[i].buffer  = h;
        axis_map[i].motor   = NULL;
        axis_map[i].channel = (uint32_t)i;
    }

    header_buf  = circ_buf_init(header_mem,  HEADER_BUF_SIZE);
    generic_buf = circ_buf_init(generic_mem, AXIS_BUF_SIZE);

    usb_praser_init(header_buf, generic_buf);
}

void tearDown(void) { }

void test_init_rejects_null_handle(void)
{
    debug_error_ExpectAnyArgs();
    usb_praser_init((circ_buf_t){0}, generic_buf);
}

void test_header_is_pushed_to_header_buffer(void)
{
    uint8_t hdr[4] = { 1U, 8U, 0U, 0U };
    read_usb_praser(hdr, sizeof(hdr));

    TEST_ASSERT_EQUAL_UINT32(4U, circ_buf_count(header_buf));

    uint8_t out[4];
    TEST_ASSERT_EQUAL_UINT32(4U, circ_buf_pop_many(header_buf, out, 4U));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hdr, out, 4U);
}

void test_payload_for_axis_x(void)
{
    uint8_t hdr[4]     = { 1U, 8U, 0U, 0U };
    uint8_t payload[4] = { 0xAA, 0xBB, 0xCC, 0xDD };

    read_usb_praser(hdr, sizeof(hdr));
    read_usb_praser(payload, sizeof(payload));

    uint8_t out[4];
    TEST_ASSERT_EQUAL_UINT32(4U,
        circ_buf_pop_many(axis_map[AXIS_X].buffer, out, 4U));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, out, 4U);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_count(axis_map[AXIS_Y].buffer));
}

void test_unknown_type_goes_to_generic_buffer(void)
{
    uint8_t hdr[4]     = { 0U, 8U, 0U, 0U };
    uint8_t payload[4] = { 9U, 8U, 7U, 6U };

    read_usb_praser(hdr, sizeof(hdr));
    read_usb_praser(payload, sizeof(payload));

    uint8_t out[4];
    TEST_ASSERT_EQUAL_UINT32(4U,
        circ_buf_pop_many(generic_buf, out, 4U));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, out, 4U);
}

void test_header_split_across_calls(void)
{
    uint8_t p1[2] = { 1U, 8U };
    uint8_t p2[2] = { 0U, 0U };
    read_usb_praser(p1, 2U);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_count(header_buf));
    read_usb_praser(p2, 2U);
    TEST_ASSERT_EQUAL_UINT32(4U, circ_buf_count(header_buf));
}
