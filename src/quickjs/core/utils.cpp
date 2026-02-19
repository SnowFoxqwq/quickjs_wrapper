#include "utils.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <mutex>

namespace js
{
    namespace console
    {

        static std::tm get_local_time(const time_t* time)
        {
            std::tm local_time{};
#if defined(_MSC_VER)
            localtime_s(&local_time, time);
#else
            localtime_r(time, &local_time);
#endif
            return local_time;
        }

        static std::mutex& get_console_mutex()
        {
            static std::mutex console_mutex;
            return console_mutex;
        }

        void printf(LogLevel level, const char* fmt, ...) noexcept
        {
            std::lock_guard<std::mutex> lock(get_console_mutex());

            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);

            std::tm local_time = get_local_time(&time);

            char time_buf[128]{};
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            const char* level_prefix = "";

            FILE* output_stream = stdout;

            switch (level)
            {
            case LogLevel::LOG_LEVEL_TRACE:
                level_prefix = "[TRACE] ";
                break;
            case LogLevel::LOG_LEVEL_INFO:
                level_prefix = "[INFO] ";
                break;
            case LogLevel::LOG_LEVEL_WARN:
                level_prefix = "[WARN] ";
                output_stream = stderr;
                break;
            case LogLevel::LOG_LEVEL_ERROR:
                level_prefix = "[ERROR] ";
                output_stream = stderr;
                break;
            case LogLevel::LOG_LEVEL_DEBUG:
                level_prefix = "[DEBUG] ";
                break;
            }

            fputs(time_buf, output_stream);
            fputs(level_prefix, output_stream);

            va_list vargs;
            va_start(vargs, fmt);
            vfprintf(output_stream, fmt, vargs);
            va_end(vargs);

            fputs("\n", output_stream);
            fflush(output_stream);
        }

        void trace(const char* fmt, ...) noexcept
        {
            va_list vargs;
            va_start(vargs, fmt);

            char time_buf[128]{};
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time = get_local_time(&time);
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            std::lock_guard<std::mutex> lock(get_console_mutex());
            fputs(time_buf, stdout);
            fputs("[TRACE] ", stdout);
            vfprintf(stdout, fmt, vargs);
            fputs("\n", stdout);
            fflush(stdout);

            va_end(vargs);
        }

        void info(const char* fmt, ...) noexcept
        {
            va_list vargs;
            va_start(vargs, fmt);

            char time_buf[128]{};
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time = get_local_time(&time);
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            std::lock_guard<std::mutex> lock(get_console_mutex());
            fputs(time_buf, stdout);
            fputs("[INFO] ", stdout);
            vfprintf(stdout, fmt, vargs);
            fputs("\n", stdout);
            fflush(stdout);

            va_end(vargs);
        }

        void warn(const char* fmt, ...) noexcept
        {
            va_list vargs;
            va_start(vargs, fmt);

            char time_buf[128]{};
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time = get_local_time(&time);
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            std::lock_guard<std::mutex> lock(get_console_mutex());
            fputs(time_buf, stderr);
            fputs("[WARN] ", stderr);
            vfprintf(stderr, fmt, vargs);
            fputs("\n", stderr);
            fflush(stderr);

            va_end(vargs);
        }

        void error(const char* fmt, ...) noexcept
        {
            va_list vargs;
            va_start(vargs, fmt);

            char time_buf[128]{};
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time = get_local_time(&time);
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            std::lock_guard<std::mutex> lock(get_console_mutex());
            fputs(time_buf, stderr);
            fputs("[ERROR] ", stderr);
            vfprintf(stderr, fmt, vargs);
            fputs("\n", stderr);
            fflush(stderr);

            va_end(vargs);
        }

        void debug(const char* fmt, ...) noexcept
        {
            va_list vargs;
            va_start(vargs, fmt);

            char time_buf[128]{};
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm local_time = get_local_time(&time);
            snprintf(
                time_buf,
                sizeof(time_buf),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
                local_time.tm_year + 1900,
                local_time.tm_mon + 1,
                local_time.tm_mday,
                local_time.tm_hour,
                local_time.tm_min,
                local_time.tm_sec,
                static_cast<int>(ms.count()));

            std::lock_guard<std::mutex> lock(get_console_mutex());
            fputs(time_buf, stdout);
            fputs("[DEBUG] ", stdout);
            vfprintf(stdout, fmt, vargs);
            fputs("\n", stdout);
            fflush(stdout);

            va_end(vargs);
        }
    }
}
