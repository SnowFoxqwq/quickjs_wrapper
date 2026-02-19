#pragma once

#define QUICKJS_USE_EXCEPTION      true
#define QUICKJS_ENABLE_ASSERTION   true
#define QUICKJS_ENABLE_CONSOLE_MSG true

#if defined(_MSC_VER)
#define QUICKJS_DEBUGBREAK() __debugbreak()
#elif defined(__clang__)
#define QUICKJS_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define QUICKJS_DEBUGBREAK() __asm__ volatile("int3;nop")
#elif defined(__GNUC__) && defined(__thumb__)
#define QUICKJS_DEBUGBREAK() __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define QUICKJS_DEBUGBREAK() __asm__ volatile(".inst 0xe7f001f0")
#else
#pragma message("From \"QUICKJS_DEBUGBREAK()\" : No debug break implementation for current platform/compiler")
#define QUICKJS_DEBUGBREAK()
#endif

/*
QUICKJS_MAYBE_NOEXCEPT
When exceptions are enabled, this function may throw exceptions;
When exceptions are disabled, this function will not throw exceptions

QUICKJS_IF_EXCEPTIONS(x)
This macro expands to x when exceptions are enabled, otherwise does nothing
*/

#if QUICKJS_USE_EXCEPTION
#define QUICKJS_MAYBE_NOEXCEPT
#define QUICKJS_IF_EXCEPTIONS(x) x
#else
#define QUICKJS_MAYBE_NOEXCEPT   noexcept
#define QUICKJS_IF_EXCEPTIONS(x) ((void)0)
#endif

#if QUICKJS_ENABLE_ASSERTION
#include <cstdio>

#define QUICKJS_ASSERT(x, ...)                                           \
    do                                                                   \
    {                                                                    \
        if (!(static_cast<bool>((x))))                                   \
        {                                                                \
            fprintf(stderr, "[QuickJS Assertion Failed]: " __VA_ARGS__); \
            fflush(stderr);                                              \
            QUICKJS_DEBUGBREAK();                                        \
        }                                                                \
    } while (0)

#else
#define QUICKJS_ASSERT(x, ...) \
    do                         \
    {                          \
    } while (0)
#endif
