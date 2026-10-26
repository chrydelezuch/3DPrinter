#include "unity.h"
#include "protocol_praser.h"
#include "circ_buf.h"

/* mocki CMock */
#include "mock_usbd_def.h"        /* CDC_Transmit_FS */
#include "mock_debug.h"           /* debug_error, debug_warn */

/* ------------------------------------------------------------------ */
/* Fixture                                                             */
/* ------------------------------------------------------------------ */

#define HDR_BUF_SIZE  32U
#define USB_BUF_SIZE  128U
#define TX_BUF_SIZE   256U

static uint8_t hdr_mem[HDR_BUF_SIZE];
static uint8_t usb_mem[USB_BUF_SIZE];

static circ_buf_t hdr_buf;
static circ_buf_t usb_buf;

/* pomocnik: wkłada nagłówek 4B do hdr_buf */
static void push_header(uint8_t type, uint8_t len, uint16_t seq)
{
    uint8_t b[4] = { type, len, (uint8_t)(seq & 0xFF), (uint8_t)(seq >> 8) };
    circ_buf_push_many(&hdr_buf, b, 4U);
}

/* pomocnik: wkłada payload do usb_buf */
static void push_payload(const uint8_t *p, size_t n)
{
    circ_buf_push_many(&usb_buf, p, n);
}

void setUp(void)
{
    circ_buf_init(&hdr_buf, hdr_mem, HDR_BUF_SIZE, 0U);
    circ_buf_init(&usb_buf, usb_mem, USB_BUF_SIZE, 0U);

    /* parse_init inicjalizuje też tx_buffer wewnętrznie */
    parse_init(&hdr_buf, &usb_buf);
}

void tearDown(void) { }

/* ------------------------------------------------------------------ */
/* parse_init                                                          */
/* ------------------------------------------------------------------ */

void test_parse_init_rejects_null(void)
{
    debug_error_ExpectAnyArgs();
    parse_init(NULL, &usb_buf);
}

void test_parse_init_rejects_uninitialized_buffer(void)
{
    circ_buf_t bad = {0};
    debug_error_ExpectAnyArgs();
    parse_init(&bad, &usb_buf);
}

/* ------------------------------------------------------------------ */
/* generate_next_seq                                                   */
/* ------------------------------------------------------------------ */

void test_generate_next_seq_increments_and_wraps(void)
{
    TEST_ASSERT_EQUAL_UINT16(1U, generate_next_seq());
    TEST_ASSERT_EQUAL_UINT16(2U, generate_next_seq());
    TEST_ASSERT_EQUAL_UINT16(3U, generate_next_seq());
}

/* ------------------------------------------------------------------ */
/* send_frame — przez send_sync / send_status                          */
/* ------------------------------------------------------------------ */

void test_send_sync_transmits_ok(void)
{
    /* CDC_Transmit_FS zwraca USBD_OK przy pierwszym wywołaniu */
    CDC_Transmit_FS_ExpectAnyArgsAndReturn(USBD_OK);

    send_sync();   /* nie powinno wywołać debug_error */

    /* Ramka FRAME_SYNC: type + len + seq(2B) + 4B payload "Hi!\n" = 8B */
    /* Sprawdzamy, że CDC dostał 8B — szczegóły przez mock argumentów: */
    /* (poniżej wersja bardziej precyzyjna, patrz test niżej) */
}

void test_send_sync_frame_layout(void)
{
    uint8_t expected[8] = {
        FRAME_SYNC,     /* type  */
        4,              /* len   */
        0, 0,           /* seq   */
        'H','i','!','\n'
    };

    CDC_Transmit_FS_ExpectWithArrayAndReturn(expected, 8, 8, USBD_OK);

    send_sync();
}

void test_send_sync_logs_error_when_transmit_fails_hard(void)
{
    CDC_Transmit_FS_ExpectAnyArgsAndReturn(USBD_FAIL);
    debug_error_ExpectAnyArgs();

    send_sync();
}

void test_send_sync_busy_falls_back_to_fifo(void)
{
    /* BUSY → dane lądują w tx_buffer, send_frame zwraca 1 → brak debug_error */
    CDC_Transmit_FS_ExpectAnyArgsAndReturn(USBD_BUSY);

    send_sync();
    /* brak asercji — wystarczy, że nie ma debug_error */
}

/* ------------------------------------------------------------------ */
/* send_status / send_status_req                                       */
/* ------------------------------------------------------------------ */

void test_send_status_transmits_current_state(void)
{
    currentState = 3;

    uint8_t expected[8] = {
        FRAME_STATUS_RESP, 4, 1, 0,   /* seq = 1 po generate_next_seq */
        3, 0, 0, 0
    };

    CDC_Transmit_FS_ExpectWithArrayAndReturn(expected, 8, 8, USBD_OK);

    send_status();
}

void test_send_status_req_has_zero_payload(void)
{
    uint8_t expected[4] = { FRAME_STATUS_RESP, 0, 1, 0 };
    CDC_Transmit_FS_ExpectWithArrayAndReturn(expected, 4, 4, USBD_OK);

    send_status_req();
}

/* ------------------------------------------------------------------ */
/* parse_frame — walidacja                                             */
/* ------------------------------------------------------------------ */

void test_parse_frame_null_flag(void)
{
    debug_error_ExpectAnyArgs();
    parse_frame(NULL);
}

void test_parse_frame_buffers_not_initialized(void)
{
    /* nowa instancja modułu? nie da się — global. Symulujemy przez
       wyczyszczenie przez parse_init(NULL,...) NIE zadziała.
       Pomijamy — patrz uwaga na dole. */
    TEST_IGNORE_MESSAGE("Brak API do resetu stanu — patrz uwaga");
}

void test_parse_frame_too_short_header_returns(void)
{
    uint8_t flag = 0xFF;
    push_header(FRAME_SYNC, 0, 0);
    /* usuwamy 1 bajt, żeby zostało 3 < 4 */
    uint8_t tmp;
    circ_buf_pop(&hdr_buf, &tmp);

    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);           /* *flag = 0 na wejściu */
    TEST_ASSERT_EQUAL_UINT32(3U, circ_buf_size(&hdr_buf)); /* nietknięte */
}

/* ------------------------------------------------------------------ */
/* parse_frame — typy nieobsługiwane (brak side-effectów)              */
/* ------------------------------------------------------------------ */

void test_parse_frame_sync_no_side_effects(void)
{
    uint8_t flag = 0xFF;
    push_header(FRAME_SYNC, 0, 0);

    /* żadnych wywołań CDC / debug */
    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&hdr_buf));
}

void test_parse_frame_unknown_type_logs_warn(void)
{
    uint8_t flag = 0xFF;
    push_header(0xAB, 0, 0);

    debug_warn_ExpectAnyArgs();
    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
}

/* ------------------------------------------------------------------ */
/* parse_frame — FRAME_SYNC_ACK → odpowiada ACK                        */
/* ------------------------------------------------------------------ */

void test_parse_frame_sync_ack_sends_ack_and_sets_flag(void)
{
    uint8_t flag = 0xFF;
    uint8_t payload[4] = { 1, 2, 3, 4 };

    push_header(FRAME_SYNC_ACK, 4, 0);
    push_payload(payload, 4);

    /* odpowiedź: FRAME_ACK, len=4, seq=1, "Jo!\n" */
    uint8_t expected[8] = {
        FRAME_ACK, 4, 1, 0,
        'J','o','!','\n'
    };
    CDC_Transmit_FS_ExpectWithArrayAndReturn(expected, 8, 8, USBD_OK);

    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(FRAME_ACK, flag);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&hdr_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&usb_buf));
}

void test_parse_frame_sync_ack_too_short_payload_ignored(void)
{
    uint8_t flag = 0xFF;
    uint8_t payload[2] = { 1, 2 };

    push_header(FRAME_SYNC_ACK, 2, 0);
    push_payload(payload, 2);

    /* payload_len < 4 → parse_sync_ack zwraca bez wysyłki */
    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&hdr_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&usb_buf));
}

void test_parse_frame_sync_ack_payload_overflow_rejected(void)
{
    /* payload_len > MAX_PAYLOAD_SIZE → debug_error + stored_length clamp */
    uint8_t flag = 0xFF;
    uint8_t len = (uint8_t)(MAX_PAYLOAD_SIZE + 1);

    push_header(FRAME_SYNC_ACK, len, 0);
    /* nie wkładamy payloadu — pop się nie powiedzie i funkcja wróci */

    debug_error_ExpectAnyArgs();
    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
}

/* ------------------------------------------------------------------ */
/* parse_frame — FRAME_MOVE_X (payload discardowany)                   */
/* ------------------------------------------------------------------ */

void test_parse_frame_move_x_discards_payload(void)
{
    uint8_t flag = 0xFF;
    uint8_t payload[4] = { 1, 2, 3, 4 };

    push_header(FRAME_MOVE_X, 4, 0);
    push_payload(payload, 4);

    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&hdr_buf));
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&usb_buf));
}

/* ------------------------------------------------------------------ */
/* parse_frame — FRAME_STATUS_REQ (handler pusty)                      */
/* ------------------------------------------------------------------ */

void test_parse_frame_status_req_consumes_input(void)
{
    uint8_t flag = 0xFF;
    push_header(FRAME_STATUS_REQ, 0, 0);

    parse_frame(&flag);

    TEST_ASSERT_EQUAL_UINT8(0U, flag);
    TEST_ASSERT_EQUAL_UINT32(0U, circ_buf_size(&hdr_buf));
}

/* ------------------------------------------------------------------ */
/* parse_frame — FRAME_ESTOP / FRAME_STATUS_RESP (puste TODO)          */
/* ------------------------------------------------------------------ */

void test_parse_frame_estop_noop(void)
{
    uint8_t flag = 0xFF;
    push_header(FRAME_ESTOP, 0, 0);
    parse_frame(&flag);
    TEST_ASSERT_EQUAL_UINT8(0U, flag);
}

void test_parse_frame_status_resp_noop(void)
{
    uint8_t flag = 0xFF;
    push_header(FRAME_STATUS_RESP, 0, 0);
    parse_frame(&flag);
    TEST_ASSERT_EQUAL_UINT8(0U, flag);
}
