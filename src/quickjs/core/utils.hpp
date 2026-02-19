#pragma once

#include "macros.hpp"

#include <cstdint>

namespace js
{
    namespace console
    {
        enum class LogLevel : std::int32_t
        {
            LOG_LEVEL_TRACE,
            LOG_LEVEL_INFO,
            LOG_LEVEL_WARN,
            LOG_LEVEL_ERROR,
            LOG_LEVEL_DEBUG
        };

#if QUICKJS_ENABLE_CONSOLE_MSG

        void printf(LogLevel level, const char* fmt, ...) noexcept;

        void trace(const char* fmt, ...) noexcept;

        void info(const char* fmt, ...) noexcept;

        void warn(const char* fmt, ...) noexcept;

        void error(const char* fmt, ...) noexcept;

        void debug(const char* fmt, ...) noexcept;

#else

#define trace(...) ((void)0)
#define info(...)  ((void)0)
#define warn(...)  ((void)0)
#define error(...) ((void)0)
#define debug(...) ((void)0)

#endif
    }
}