#pragma once

#include <quickjs.h>
#include <string>
#include <string_view>

namespace js
{
    // RAII wrapper for JS CStrings
    class JSString
    {
    public:
        JSString(JSContext* ctx, JSValueConst value)
            : ctx(ctx), str(JS_ToCString(ctx, value)), len(0)
        {
            if (str)
                len = strlen(str);
        }

        ~JSString()
        {
            if (str && ctx)
                JS_FreeCString(ctx, str);
        }

        // Non-copyable
        JSString(const JSString&) = delete;
        JSString& operator=(const JSString&) = delete;

        // Movable
        JSString(JSString&& other) noexcept
            : ctx(other.ctx), str(other.str), len(other.len)
        {
            other.ctx = nullptr;
            other.str = nullptr;
            other.len = 0;
        }

        const char* data() const { return str; }
        size_t size() const { return len; }
        bool empty() const { return !str || len == 0; }

        operator std::string() const { return empty() ? "" : std::string(str, len); }
        operator std::string_view() const { return empty() ? std::string_view() : std::string_view(str, len); }

    private:
        JSContext* ctx;
        const char* str;
        size_t len;
    };
}
