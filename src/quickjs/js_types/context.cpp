#include "context.hpp"
#include "../exception/exception.hpp"
#include "module.hpp"

#include "../core/macros.hpp"
#include "../core/utils.hpp"

#include <cstdint>
#include <string>

namespace js
{

    JSEvalOptions operator|(JSEvalOptions lhs, JSEvalOptions rhs) noexcept
    {
        return static_cast<JSEvalOptions>(static_cast<int>(lhs) | static_cast<int>(rhs));
    }

    JSEvalOptions operator&(JSEvalOptions lhs, JSEvalOptions rhs) noexcept
    {
        return static_cast<JSEvalOptions>(static_cast<int>(lhs) & static_cast<int>(rhs));
    }

    Context::Context() : _context(nullptr) {}

    Context::Context(Runtime& runtime) : _context(JS_NewContext(runtime.get_runtime_handle()))
    {
        if (!_context)
        {
            console::error("Failed to create JS context.");
            QUICKJS_IF_EXCEPTIONS(throw Exception("Failed to create JS context."));
        }
    }

    void Context::import_os_module() const noexcept
    {
        js_init_module_std(_context, "os");
        static const char* preload = "import * as os from 'os';\nglobalThis.os = os;";
        JS_Eval(_context, preload, strlen(preload), "<preload-os>", JS_EVAL_TYPE_MODULE);
    }

    void Context::import_std_module() const noexcept
    {
        js_init_module_std(_context, "std");
        static const char* preload = "import * as std from 'std';\nglobalThis.std = std;";
        JS_Eval(_context, preload, strlen(preload), "<preload-std>", JS_EVAL_TYPE_MODULE);
    }

    void Context::import_json_module() const noexcept
    {
        js_init_module_std(_context, "json");
        static const char* preload = "import * as json from 'json';\nglobalThis.json = json;";
        JS_Eval(_context, preload, strlen(preload), "<preload-json>", JS_EVAL_TYPE_MODULE);
    }

    void Context::process_exception(JSContext* ctx)
    {
        if (!ctx) return;

        JSValue exception_value = JS_GetException(ctx);
        bool is_error = JS_IsException(exception_value);

        const char* str = JS_ToCString(ctx, exception_value);
        if (str)
        {
            console::error(str);
            JS_FreeCString(ctx, str);
        }

        if (is_error)
        {
            JSValue val = JS_GetPropertyStr(ctx, exception_value, "stack");
            if (!JS_IsUndefined(val))
            {
                const char* stack_info = JS_ToCString(ctx, val);
                console::error("stack: {}", stack_info);
                JS_FreeCString(ctx, stack_info);
            }
            JS_FreeValue(ctx, val);
        }

        JS_FreeValue(ctx, exception_value);
    }

    Context::~Context()
    {
        _modules.clear();
        if (_context)
        {
            JS_FreeContext(_context);
        }
    }

    Context::Context(Context&& other) noexcept
        : _context(other._context), _modules(std::move(other._modules))
    {
        other._context = nullptr;
    }

    Context& Context::operator=(Context&& other) noexcept
    {
        if (this != &other)
        {
            _modules.clear();
            if (_context)
            {
                JS_FreeContext(_context);
            }
            _context = other._context;
            _modules = std::move(other._modules);
            other._context = nullptr;
        }
        return *this;
    }

    bool Context::is_valid() const noexcept { return _context != nullptr; }

    Value Context::eval(const std::string& code, const std::string& filename, JSEvalOptions flags) QUICKJS_MAYBE_NOEXCEPT
    {
        JSValue result = JS_Eval(_context, code.c_str(), code.size(), filename.c_str(), static_cast<int32_t>(flags));

        if (JS_IsException(result))
        {
            if (_on_exception)
            {
                _on_exception(_context);
            }
            JS_FreeValue(_context, result);
            console::error("Failed to evaluate JS code (filename: \"%s\")", filename.c_str());
            QUICKJS_IF_EXCEPTIONS(throw Exception(std::string("Failed to evaluate JS code (filename: \"") + filename + "\")", _context));
            return Value(_context, JS_UNDEFINED);
        }

        return Value(_context, result);
    }

    Value Context::get_global() const
    {
        return Value(_context, JS_GetGlobalObject(_context));
    }

    Value Context::get_exception() const
    {
        return Value(_context, JS_GetException(_context));
    }

    Module& Context::add_module(const std::string& name)
    {
        _modules.emplace_back(name, _context);
        return _modules.back();
    }
}
