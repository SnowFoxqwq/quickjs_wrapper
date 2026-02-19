#include "runtime.hpp"
#include "../core/utils.hpp"
#include "../exception/exception.hpp"

namespace js
{
    Runtime::Runtime() QUICKJS_MAYBE_NOEXCEPT : _runtime(JS_NewRuntime())
    {
        js_std_init_handlers(_runtime);
        if (!_runtime)
        {
            console::error("Failed to create runtime.");
            QUICKJS_IF_EXCEPTIONS(throw Exception("Failed to create runtime."));
        }
    }

    Runtime::~Runtime()
    {
        if (_runtime)
        {
            js_std_free_handlers(_runtime);
            JS_FreeRuntime(_runtime);
            _runtime = nullptr;
        }
    }

    Runtime::Runtime(Runtime&& other) noexcept : _runtime(other._runtime)
    {
        other._runtime = nullptr;
    }

    Runtime& Runtime::operator=(Runtime&& other) noexcept
    {
        if (this != &other)
        {
            if (_runtime)
            {
                JS_FreeRuntime(_runtime);
            }
            _runtime = other._runtime;
            other._runtime = nullptr;
        }
        return *this;
    }

    bool Runtime::is_valid() const noexcept { return _runtime != nullptr; }

    JSRuntime* Runtime::get_runtime_handle() const noexcept { return _runtime; }
}
