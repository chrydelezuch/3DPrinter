#include "../Inc/debug.h"

#if defined(UNIT_TEST)

/* Host test builds do not use this translation unit; keep it non-empty. */
typedef int debug_unit_test_stub_t;

#else

#include "stm32f4xx_hal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_BUFFER_SIZE   128U
#define DEBUG_ITM_PORT      0U
#define DEBUG_ITM_PORT_MASK 0x00000001U
#define DEBUG_ITM_TIMEOUT   100000U

#if DBG_ENABLED

/*
 * The debugger controls ITM. When no debugger is attached, or SWV is not
 * enabled, every write is silently dropped so that printf never blocks.
 */
static int debug_trace_ready(void)
{
    uint32_t debugger_attached;
    uint32_t itm_enabled;
    uint32_t port_enabled;

    debugger_attached = CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk;
    itm_enabled = ITM->TCR & ITM_TCR_ITMENA_Msk;
    port_enabled = ITM->TER & DEBUG_ITM_PORT_MASK;

    if (debugger_attached == 0U) {
        return 0;
    }
    if (itm_enabled == 0U) {
        return 0;
    }
    if (port_enabled == 0U) {
        return 0;
    }
    return 1;
}

/*
 * Bounded busy wait. A fixed timeout means a detached debugger cannot hang
 * the firmware, which satisfies the fixed-loop-bound project rule.
 */
static int debug_send_byte(uint8_t byte)
{
    uint32_t timeout = DEBUG_ITM_TIMEOUT;

    while ((ITM->PORT[DEBUG_ITM_PORT].u32 == 0U) && (timeout > 0U)) {
        timeout--;
        __NOP();
    }

    if (timeout == 0U) {
        return 0;
    }

    ITM->PORT[DEBUG_ITM_PORT].u8 = byte;
    return 1;
}

static void debug_write(const char *buffer, size_t length)
{
    size_t index;

    if (buffer == NULL) {
        return;
    }

    if (length > DEBUG_BUFFER_SIZE) {
        length = DEBUG_BUFFER_SIZE;
    }

    for (index = 0U; index < length; index++) {
        if (debug_send_byte((uint8_t)buffer[index]) == 0) {
            return;
        }
    }
}

static void debug_vlog(const char *prefix, const char *format, va_list arguments)
{
    char buffer[DEBUG_BUFFER_SIZE];
    size_t prefix_length;
    size_t available;
    int body_length;

    if ((prefix == NULL) || (format == NULL)) {
        return;
    }

    if (debug_trace_ready() == 0) {
        return;
    }

    prefix_length = strlen(prefix);
    if (prefix_length >= DEBUG_BUFFER_SIZE) {
        prefix_length = DEBUG_BUFFER_SIZE - 1U;
    }
    memcpy(buffer, prefix, prefix_length);

    available = DEBUG_BUFFER_SIZE - prefix_length;
    body_length = vsnprintf(&buffer[prefix_length], available, format, arguments);
    if (body_length < 0) {
        return;
    }
    if ((size_t)body_length >= available) {
        body_length = (int)(available - 1U);
    }

    debug_write(buffer, prefix_length + (size_t)body_length);
}

#endif /* DBG_ENABLED */

void debug_init(void)
{
    /* ITM is configured by the debugger; no firmware setup is required. */
}

void debug_error(const char *format, ...)
{
    (void)format;
#if DBG_ENABLED
    va_list arguments;

    va_start(arguments, format);
    debug_vlog("[E] ", format, arguments);
    va_end(arguments);
#endif
}

void debug_warn(const char *format, ...)
{
    (void)format;
#if DBG_ENABLED
    va_list arguments;

    va_start(arguments, format);
    debug_vlog("[W] ", format, arguments);
    va_end(arguments);
#endif
}

void debug_info(const char *format, ...)
{
    (void)format;
#if DBG_ENABLED
    va_list arguments;

    va_start(arguments, format);
    debug_vlog("[I] ", format, arguments);
    va_end(arguments);
#endif
}

void debug_debug(const char *format, ...)
{
    (void)format;
#if DBG_ENABLED
    va_list arguments;

    va_start(arguments, format);
    debug_vlog("[D] ", format, arguments);
    va_end(arguments);
#endif
}

void debug_assert_failed(const char *expression, const char *file, int line)
{
    if (expression == NULL) {
        expression = "?";
    }
    if (file == NULL) {
        file = "?";
    }

    debug_error("ASSERT FAILED: %s @ %s:%d\r\n", expression, file, line);
}

/*
 * Strong replacement for the weak __io_putchar referenced by Src/syscalls.c.
 * This makes printf safe even when no debugger is attached.
 */
int __io_putchar(int character)
{
#if DBG_ENABLED
    if (debug_trace_ready() != 0) {
        (void)debug_send_byte((uint8_t)character);
    }
#endif
    return character;
}

#endif /* UNIT_TEST */
