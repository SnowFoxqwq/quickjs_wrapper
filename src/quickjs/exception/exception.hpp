#pragma once

#include <cstdint>
#include <quickjs.h>

#include <stdexcept>
#include <string>

namespace js
{
    class Exception : public std::runtime_error
    {
    public:
        Exception(const std::string& message, JSContext* ctx) : std::runtime_error(message)
        {
            _error_message = "[JS Exception (from context: \'" + std::to_string((uintptr_t)ctx) + "\')]: ";
            _error_message += message;
        }

        Exception(const std::string& message) : std::runtime_error(message)
        {
            _error_message = "[JS Exception]: " + message;
        }

        [[nodiscard]] const char* what() const noexcept override
        {
            return _error_message.c_str();
        }

    private:
        std::string _error_message;
    };
}
