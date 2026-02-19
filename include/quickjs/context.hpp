#pragma once

#include "macros.hpp"
#include "runtime.hpp"
#include "value.hpp"

#include <cstdint>
#include <quickjs-libc.h>
#include <quickjs.h>

#include <string>
#include <vector>

#include <functional>

namespace js
{

    enum class JSEvalOptions : int32_t
    {
        TYPE_GLOBAL = (0 << 0),
        TYPE_MODULE = (1 << 0),
        TYPE_DIRECT = (2 << 0),
        TYPE_INDIRECT = (3 << 0),
        TYPE_MASK = (3 << 0),
        FLAG_STRICT = (1 << 3),
        FLAG_UNUSED = (1 << 4),
        FLAG_COMPILE_ONLY = (1 << 5),
        FLAG_BACKTRACE_BARRIER = (1 << 6),
        FLAG_ASYNC = (1 << 7)
    };

    JSEvalOptions operator|(JSEvalOptions lhs, JSEvalOptions rhs) noexcept;
    JSEvalOptions operator&(JSEvalOptions lhs, JSEvalOptions rhs) noexcept;

    class Module;

    class Context
    {
    public:
        Context();
        explicit Context(Runtime& runtime);
        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        Context(Context&& other) noexcept;
        Context& operator=(Context&& other) noexcept;

        // check if the current JS context is valid
        bool is_valid() const noexcept;

        // evaluate js code
        Value eval(const std::string& code, const std::string& filename = "<eval>", JSEvalOptions flags = JSEvalOptions::TYPE_GLOBAL | JSEvalOptions::FLAG_STRICT) QUICKJS_MAYBE_NOEXCEPT;

        // get global object of the current js context
        Value get_global() const;

        // get exception of the current js context
        Value get_exception() const;

        // add a variable to global object
        template <typename T>
        Context& add_variable(const std::string& name, T value)
        {
            JSValue global = JS_GetGlobalObject(_context);
            JSValue js_val = detail::TypeConverter<detail::remove_cvref_t<T>>::to_js(_context, value);
            JS_SetPropertyStr(_context, global, name.c_str(), js_val);
            JS_FreeValue(_context, global);
            return *this;
        }

        // add a constant to global object (read-only)
        template <typename T>
        Context& add_constant(const std::string& name, T value)
        {
            JSValue global = JS_GetGlobalObject(_context);
            JSValue js_val = detail::TypeConverter<detail::remove_cvref_t<T>>::to_js(_context, value);
            JS_DefinePropertyValueStr(_context, global, name.c_str(), js_val, JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE);
            JS_FreeValue(_context, global);
            return *this;
        }

        // add a module to the current js context
        Module& add_module(const std::string& name);

        // import os module
        // Note: This library provides some low-level control functions.
        // Please import and use it with caution.
        void import_os_module() const noexcept;

        // import std module
        // Note: This library provides some low-level control functions.
        // Please import and use it with caution.
        void import_std_module() const noexcept;

        // import json module
        void import_json_module() const noexcept;

        // set the callback function to be invoked on exception.
        void set_exception_callback(std::function<void(JSContext*)> callback = process_exception) { _on_exception = callback; }

    private:
        static void process_exception(JSContext* ctx);

        JSContext* get_context_handle() const noexcept { return _context; }

    private:
        JSContext* _context;
        std::vector<Module> _modules;
        std::function<void(JSContext*)> _on_exception{&process_exception};
    };
}
