#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdint.h>

/*
 * Single project-wide switch.
 *
 * DEBUG     - set in the Debug build configuration (toolchain level).
 * NDEBUG    - optionally set in the Release build configuration.
 * UNIT_TEST - set by the host test build; keeps this header HAL-free.
 *
 * Logging is compiled in only when DEBUG is defined and UNIT_TEST is not.
 */
#if defined(DEBUG) && defined(NDEBUG)
#error "Both DEBUG and NDEBUG defined - pick one build config."
#endif

#if defined(DEBUG) && !defined(UNIT_TEST)
#define DBG_ENABLED 1
#else
#define DBG_ENABLED 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DBG_LEVEL_ERROR = 0,
    DBG_LEVEL_WARN,
    DBG_LEVEL_INFO,
    DBG_LEVEL_DEBUG
} dbg_level_t;

#if defined(UNIT_TEST)

/*
 * Host test builds do not link Src/debug.c and do not have the real HAL.
 * These stubs keep the application translation units linkable and silent.
 */
static inline void debug_init(void)
{
}

static inline void debug_error(const char *format, ...)
{
    (void)format;
}

static inline void debug_warn(const char *format, ...)
{
    (void)format;
}

static inline void debug_info(const char *format, ...)
{
    (void)format;
}

static inline void debug_debug(const char *format, ...)
{
    (void)format;
}

static inline void debug_assert_failed(const char *expression, const char *file, int line)
{
    (void)expression;
    (void)file;
    (void)line;
}

#else

void debug_init(void);

void debug_error(const char *format, ...);
void debug_warn(const char *format, ...);
void debug_info(const char *format, ...);
void debug_debug(const char *format, ...);

void debug_assert_failed(const char *expression, const char *file, int line);

#endif

/*
 * Diagnostic assertion.
 *
 * Expands to a single valid expression so that it can be used anywhere in a
 * statement context. It reports and returns; it does not halt, because the
 * public function signatures of this firmware do not carry error codes.
 */
#define ASSERT(condition) \
    ((condition) ? (void)0 : debug_assert_failed(#condition, __FILE__, __LINE__))

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */
