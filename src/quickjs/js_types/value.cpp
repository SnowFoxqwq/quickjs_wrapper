#include "value.hpp"

#include "../core/utils.hpp"
#include "quickjs.h"

namespace js
{
    Value::Value() : _ctx(nullptr), _val(JS_UNDEFINED) {}

    Value::Value(JSContext* ctx, JSValue val) : _ctx(ctx), _val(val) {}

    Value::Value(const Value& other) : _ctx(other._ctx)
    {
        _val = _ctx ? JS_DupValue(_ctx, other._val) : JS_UNDEFINED;
    }

    Value::Value(Value&& other) noexcept : _ctx(other._ctx), _val(other._val)
    {
        other._ctx = nullptr;
        other._val = JS_UNDEFINED;
    }

    Value& Value::operator=(const Value& other)
    {
        if (this != &other)
        {
            if (_ctx)
            {
                JS_FreeValue(_ctx, _val);
            }
            _ctx = other._ctx;
            _val = _ctx ? JS_DupValue(_ctx, other._val) : JS_UNDEFINED;
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept
    {
        if (this != &other)
        {
            if (_ctx)
            {
                JS_FreeValue(_ctx, _val);
            }
            _ctx = other._ctx;
            _val = other._val;
            other._ctx = nullptr;
            other._val = JS_UNDEFINED;
        }
        return *this;
    }

    Value::~Value()
    {
        if (_ctx)
        {
            JS_FreeValue(_ctx, _val);
        }
    }

    bool Value::is_valid() const noexcept { return _ctx != nullptr; }
    bool Value::is_undefined() const noexcept { return JS_IsUndefined(_val); }
    bool Value::is_null() const noexcept { return JS_IsNull(_val); }
    bool Value::is_function() const noexcept { return _ctx && JS_IsFunction(_ctx, _val); }
    bool Value::is_error() const noexcept { return _ctx && JS_IsError(_val); }
    bool Value::is_array() const noexcept { return _ctx && JS_IsArray(_val); }

    Value::operator bool() const noexcept
    {
        return is_valid() && !is_null() && !is_undefined();
    }

    Value Value::operator[](const char* name) const
    {
        if (!_ctx)
        {
            return Value();
        }

        JSAtom atom = JS_NewAtom(_ctx, name);
        JSValue result = JS_GetProperty(_ctx, _val, atom);
        JS_FreeAtom(_ctx, atom);
        if (!JS_IsException(result))
        {
            return Value(_ctx, result);
        }
        else
        {
            console::warn("Failed to get property: %s", name);
            return Value();
        }
    }

    Value Value::operator[](const std::string& name) const
    {
        return (*this)[name.c_str()];
    }

    Value Value::operator[](uint32_t index) const
    {
        if (!is_array())
        {
            console::warn("The type of the current value is not array.");
            return Value();
        }
        JSValue result = JS_GetPropertyUint32(_ctx, _val, index);
        return Value(_ctx, result);
    }

    // type conversion
    std::string Value::to_string() const
    {
        return _ctx ? detail::TypeConverter<std::string>::from_js(_ctx, _val) : "";
    }

    int32_t Value::to_int32() const
    {
        return _ctx ? detail::TypeConverter<int32_t>::from_js(_ctx, _val) : 0;
    }

    double Value::to_float64() const
    {
        return _ctx ? detail::TypeConverter<double>::from_js(_ctx, _val) : 0.0;
    }

    bool Value::to_bool() const noexcept
    {
        return _ctx ? JS_ToBool(_ctx, _val) != 0 : false;
    }

    Value::operator std::string() const { return to_string(); }
    Value::operator int32_t() const { return to_int32(); }
    Value::operator double() const { return to_float64(); }

    // function call
    Value Value::call(const std::vector<Value>& args) const
    {
        if (!_ctx)
        {
            return Value();
        }

        std::vector<JSValue> js_args;
        js_args.reserve(args.size());
        for (const auto& arg : args)
        {
            js_args.push_back(arg._ctx ? JS_DupValue(_ctx, arg._val) : JS_UNDEFINED);
        }

        JSValue result = JS_Call(_ctx, _val, JS_UNDEFINED, static_cast<int>(args.size()),
                                 js_args.empty() ? nullptr : js_args.data());

        for (auto& v : js_args)
        {
            JS_FreeValue(_ctx, v);
        }

        return Value(_ctx, result);
    }

    Value Value::call_with_this(Value& this_val, const std::vector<Value>& args) const
    {
        if (!_ctx)
        {
            return Value();
        }

        std::vector<JSValue> js_args;
        js_args.reserve(args.size());
        for (const auto& arg : args)
        {
            js_args.push_back(arg._ctx ? JS_DupValue(_ctx, arg._val) : JS_UNDEFINED);
        }

        // clang-format off
        JSValue result = JS_Call(
            _ctx,
            _val,
            this_val._val,
            static_cast<int>(args.size()),

            js_args.empty() ? nullptr : js_args.data()
        );
        // clang-format on

        for (auto& v : js_args)
        {
            JS_FreeValue(_ctx, v);
        }

        return Value(_ctx, result);
    }

    JSValue Value::js_value() const { return _val; }

    JSValue Value::release() noexcept
    {
        _ctx = nullptr;
        return _val;
    }

    JSContext* Value::context() const noexcept { return _ctx; }

    void Value::swap(Value& other) noexcept
    {
        using std::swap;
        swap(_ctx, other._ctx);
        swap(_val, other._val);
    }
}