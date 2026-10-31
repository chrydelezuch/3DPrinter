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


void test_frame_size_is_equal_to_buffer(void){
    uint8_t in1[4] = 
    { 0x07, 0x04, 0x0, 0x01,
      0x01, 0x02, 0x03, 0x04};

    read_usb_praser(hdr, sizeof(in1));

    uint8_t in2[4] = 
    { 0x07, 0x04, 0x0, 0x02,
      0x05, 0x06, 0x07, 0x08};

    read_usb_praser(hdr, sizeof(in2));

    uint8_t in3[4] = 
    { 0x07, 0x04, 0x0, 0x03,
      0x09, 0x10, 0x11, 0x12};

    read_usb_praser(hdr, sizeof(in3));

    uint32_t expected_payload[12] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x10, 0x11, 0x12,
    }

    uint32_t expected_header[12] = {
        0x07, 0x04, 0x00, 0x01,
        0x07, 0x04, 0x00, 0x01,
        0x07, 0x04, 0x00, 0x03,
    }

    uint32_t payload[12];
    uint32_t header[12];

    circ_buf_push_many_uint8(header_buf, header, 12);
    circ_buf_push_many_uint8(axis_map[1].buffer, header, 12);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_header, header, 12U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_payload, payload, 12U);
}

void test_header_size_is_same_size_as_buffor(void){

    uint8_t in1[4] = {
        0x07, 0x04, 0x00, 0x01};

    read_usb_praser(in1, sizeof(in1));

    uint8_t in2[4] = {
        0x01, 0x02, 0x03, 0x04};

    read_usb_praser(in2, sizeof(in2));


    uint8_t in3[4] = {
        0x07, 0x04, 0x00, 0x02};

    read_usb_praser(in3, sizeof(in3));

    uint8_t in4[4] = {
        0x05, 0x06, 0x07, 0x08};

    read_usb_praser(in4, sizeof(in4));

    uint32_t expected_payload[12] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08
    }

    uint32_t expected_header[12] = {
        0x07, 0x04, 0x00, 0x01,
        0x07, 0x04, 0x00, 0x01
    }

    circ_buf_push_many_uint8(header_buf, header, 8);
    circ_buf_push_many_uint8(axis_map[1].buffer, header, 8);

    uint32_t payload[8];
    uint32_t header[8];

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_header, header, 8U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_payload, payload, 8U);
}

void test_heder_size_is_twice_large_as_buffor(void){
    uint8_t in1[2] = {0x07, 0x04};
    uint8_t in2[2] = {0x00, 0x01};

    read_usb_praser(in1, sizeof(in1));
    read_usb_praser(in2, sizeof(in2));

    uint8_t in3[2] = {0x01, 0x02};
    uint8_t in4[2] = {0x03, 0x04};

    read_usb_praser(in5, sizeof(in5));
    read_usb_praser(in6, sizeof(in6));

    uint8_t in7[2] = {0x07, 0x04};
    uint8_t in8[2] = {0x00, 0x02};

    read_usb_praser(in7, sizeof(in7));
    read_usb_praser(in8, sizeof(in8));

    uint8_t in9[2] = {0x05, 0x06};
    uint8_t in10[2] = {0x07, 0x08};

    read_usb_praser(in9, sizeof(in9));
    read_usb_praser(in10, sizeof(in10));

    circ_buf_push_many_uint8(header_buf, header, 8);
    circ_buf_push_many_uint8(axis_map[1].buffer, header, 8);

    uint32_t payload[8];
    uint32_t header[8];

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_header, header, 8U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_payload, payload, 8U);

}

void test_hardware_buffer_size_3_bytes_two_frames_payload_8_bytes(void)
{
    uint8_t in1[3] = {
        0x07, 0x08, 0x00
    };
    read_usb_praser(in1, sizeof(in1));

    uint8_t in2[3] = {
        0x01, 0x01, 0x02
    };
    read_usb_praser(in2, sizeof(in2));

    uint8_t in3[3] = {
        0x03, 0x04, 0x05
    };
    read_usb_praser(in3, sizeof(in3));

    uint8_t in4[3] = {
        0x06, 0x07, 0x08
    };
    read_usb_praser(in4, sizeof(in4));

    uint8_t in5[3] = {
        0x07, 0x08, 0x00
    };
    read_usb_praser(in5, sizeof(in5));

    uint8_t in6[3] = {
        0x02, 0x09, 0x0A
    };
    read_usb_praser(in6, sizeof(in6));

    uint8_t in7[3] = {
        0x0B, 0x0C, 0x0D
    };
    read_usb_praser(in7, sizeof(in7));

    uint8_t in8[3] = {
        0x0E, 0x0F, 0x10
    };
    read_usb_praser(in8, sizeof(in8));

    uint8_t expected_header[8] = {
        0x07, 0x08, 0x00, 0x01,
        0x07, 0x08, 0x00, 0x02
    };

    uint8_t expected_payload[16] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };

    uint8_t header[8];
    uint8_t payload[16];

    circ_buf_pop_many_uint8(header_buf, header, 8);
    circ_buf_pop_many_uint8(axis_map[1].buffer, payload, 16);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_header, header, 8U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_payload, payload, 16U);
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

void test_hardware_buffer_size_3_bytes_three_frames_payload_4_bytes(void)
{
    uint8_t in1[3] = {
        0x07, 0x04, 0x00
    };
    read_usb_praser(in1, sizeof(in1));

    uint8_t in2[3] = {
        0x01, 0x01, 0x02
    };
    read_usb_praser(in2, sizeof(in2));

    uint8_t in3[3] = {
        0x03, 0x04, 0x07
    };
    read_usb_praser(in3, sizeof(in3));

    uint8_t in4[3] = {
        0x04, 0x00, 0x02
    };
    read_usb_praser(in4, sizeof(in4));

    uint8_t in5[3] = {
        0x05, 0x06, 0x07
    };
    read_usb_praser(in5, sizeof(in5));

    uint8_t in6[3] = {
        0x08, 0x07, 0x04
    };
    read_usb_praser(in6, sizeof(in6));

    uint8_t in7[3] = {
        0x00, 0x03, 0x09
    };
    read_usb_praser(in7, sizeof(in7));

    uint8_t in8[3] = {
        0x0A, 0x0B, 0x0C
    };
    read_usb_praser(in8, sizeof(in8));

    uint8_t expected_header[12] = {
        0x07, 0x04, 0x00, 0x01,
        0x07, 0x04, 0x00, 0x02,
        0x07, 0x04, 0x00, 0x03
    };

    uint8_t expected_payload[12] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C
    };

    uint8_t header[12];
    uint8_t payload[12];

    circ_buf_pop_many_uint8(header_buf, header, 12);
    circ_buf_pop_many_uint8(axis_map[1].buffer, payload, 12);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_header, header, 12U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_payload, payload, 12U);
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
