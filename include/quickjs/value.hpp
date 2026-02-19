#pragma once

#include "type_converter.hpp"
#include "type_traits.hpp"

#include <quickjs.h>

#include <functional>
#include <string>
#include <vector>

namespace js
{
    class Context;

    class Value
    {
        friend class Context;

    public:
        Value();
        Value(JSContext* ctx, JSValue val);

        Value(const Value& other);
        Value& operator=(const Value& other);

        Value(Value&& other) noexcept;
        Value& operator=(Value&& other) noexcept;
        ~Value();

        bool is_valid() const noexcept;
        bool is_undefined() const noexcept;
        bool is_null() const noexcept;
        bool is_function() const noexcept;
        bool is_error() const noexcept;
        bool is_array() const noexcept;

        // check if the current value is usable
        explicit operator bool() const noexcept;

        // property access
        Value operator[](const char* name) const;
        Value operator[](const std::string& name) const;

        // array index access
        Value operator[](uint32_t index) const;

        // type conversion
        std::string to_string() const;
        int32_t to_int32() const;
        double to_float64() const;
        bool to_bool() const noexcept;

        operator std::string() const;
        operator int32_t() const;
        operator double() const;

        // function call
        Value call(const std::vector<Value>& args = {}) const;
        Value call_with_this(Value& this_val, const std::vector<Value>& args = {}) const;

        // convert to std::function
        template <typename R, typename... Args>
        operator std::function<R(Args...)>() const
        {
            if (!is_function())
            {
                return nullptr;
            }

            JSContext* ctx = _ctx;
            JSValue fv = JS_DupValue(_ctx, _val);

            return std::function<R(Args...)>(
                [ctx, fv](Args... args) -> R
                {
                    JSValue js_args[] = {detail::TypeConverter<detail::remove_cvref_t<Args>>::to_js(ctx, args)...};
                    JSValue result = JS_Call(ctx, fv, JS_UNDEFINED, sizeof...(Args), js_args);

                    for (auto& v : js_args)
                    {
                        JS_FreeValue(ctx, v);
                    }

                    if constexpr (std::is_void_v<R>)
                    {
                        JS_FreeValue(ctx, result);
                    }
                    else
                    {
                        auto res = detail::TypeConverter<R>::from_js(ctx, result);
                        JS_FreeValue(ctx, result);
                        return res;
                    }
                }
                // clang-format off
            );
            // clang-format on
        }

        // swap with another value, I think it's useless
        void swap(Value& other) noexcept;

    private:
        // get raw JSValue (const reference)
        JSValue js_value() const;

        // release ownership of the JSValue (it will not be freed upon destruction)
        // this method is currently unused.
        JSValue release() noexcept;

        // get context
        JSContext* context() const noexcept;

    private:
        JSContext* _ctx;
        JSValue _val;
    };
}
