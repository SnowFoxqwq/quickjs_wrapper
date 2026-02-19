#pragma once

#include <quickjs-libc.h>
#include <quickjs.h>

#include "../core/macros.hpp"

namespace js
{
    class Context;

    class Runtime
    {
        friend class Context;

    public:
        Runtime() QUICKJS_MAYBE_NOEXCEPT;
        ~Runtime();

        Runtime(const Runtime&) = delete;
        Runtime& operator=(const Runtime&) = delete;

        Runtime(Runtime&& other) noexcept;
        Runtime& operator=(Runtime&& other) noexcept;

        // check if the current runtime is valid;
        bool is_valid() const noexcept;

    private:
        JSRuntime* get_runtime_handle() const noexcept;

    private:
        JSRuntime* _runtime;
    };
}
