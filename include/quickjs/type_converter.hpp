#pragma once

#include "macros.hpp"
#include "utils.hpp"
#include "rest.hpp"
#include "js_string.hpp"

#include <cstddef>
#include <cstdint>
#include <quickjs.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace js
{
    class Value;

    namespace detail
    {
        template <typename T, typename = void>
        struct TypeConverter;

        template <typename T>
        T unwrap_free(JSContext* ctx, JSValue val)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                JS_FreeValue(ctx, val);
                return;
            }
            else
            {
                try
                {
                    T result = TypeConverter<std::decay_t<T>>::from_js(ctx, val);
                    JS_FreeValue(ctx, val);
                    return result;
                }
                catch (...)
                {
                    JS_FreeValue(ctx, val);
                    throw;
                }
            }
        }

        template <>
        struct TypeConverter<int32_t>
        {
            static JSValue to_js(JSContext* ctx, int32_t value)
            {
                return JS_NewInt32(ctx, value);
            }

            static int32_t from_js(JSContext* ctx, JSValueConst value)
            {
                int32_t result;
                if (JS_ToInt32(ctx, &result, value) < 0)
                    throw std::runtime_error("Failed to convert to int32");
                return result;
            }
        };

        template <>
        struct TypeConverter<int64_t>
        {
            static JSValue to_js(JSContext* ctx, int64_t value)
            {
                return JS_NewInt64(ctx, value);
            }

            static int64_t from_js(JSContext* ctx, JSValueConst value)
            {
                int64_t result;
                if (JS_ToInt64(ctx, &result, value) < 0)
                    throw std::runtime_error("Failed to convert to int64");
                return result;
            }
        };

        template <>
        struct TypeConverter<uint32_t>
        {
            static JSValue to_js(JSContext* ctx, uint32_t value)
            {
                return JS_NewUint32(ctx, value);
            }

            static uint32_t from_js(JSContext* ctx, JSValueConst value)
            {
                uint32_t result;
                if (JS_ToUint32(ctx, &result, value) < 0)
                    throw std::runtime_error("Failed to convert to uint32");
                return result;
            }
        };

        template <>
        struct TypeConverter<double>
        {
            static JSValue to_js(JSContext* ctx, double value)
            {
                return JS_NewFloat64(ctx, value);
            }

            static double from_js(JSContext* ctx, JSValueConst value)
            {
                double result;
                if (JS_ToFloat64(ctx, &result, value) < 0)
                {
                    console::error("Failed to convert to double");
                    QUICKJS_IF_EXCEPTIONS(throw std::runtime_error("Failed to convert to double"));
                }
                return result;
            }
        };

        template <>
        struct TypeConverter<bool>
        {
            static JSValue to_js(JSContext* ctx, bool value)
            {
                return JS_NewBool(ctx, value);
            }

            static bool from_js(JSContext* ctx, JSValueConst value)
            {
                return JS_ToBool(ctx, value) != 0;
            }
        };

        template <>
        struct TypeConverter<std::string>
        {
            static JSValue to_js(JSContext* ctx, const std::string& value)
            {
                return JS_NewStringLen(ctx, value.data(), value.size());
            }

            static std::string from_js(JSContext* ctx, JSValueConst value)
            {
                JSString str(ctx, value);
                return std::string(str);
            }
        };

        template <>
        struct TypeConverter<std::string_view>
        {
            static JSValue to_js(JSContext* ctx, std::string_view value)
            {
                return JS_NewStringLen(ctx, value.data(), value.size());
            }

            static std::string from_js(JSContext* ctx, JSValueConst value)
            {
                JSString str(ctx, value);
                return std::string(str);
            }
        };

        template <>
        struct TypeConverter<const char*>
        {
            static JSValue to_js(JSContext* ctx, const char* value)
            {
                return JS_NewString(ctx, value);
            }

            static std::string from_js(JSContext* ctx, JSValueConst value)
            {
                JSString str(ctx, value);
                return std::string(str);
            }
        };

        // rest<T> converter - only supports from_js (used for function parameters)
        template <typename T>
        struct TypeConverter<rest<T>>
        {
            static JSValue to_js(JSContext*, const rest<T>&)
            {
                return JS_UNDEFINED;
            }

            static rest<T> from_js(JSContext* ctx, JSValueConst value)
            {
                rest<T> result;
                result.push_back(TypeConverter<T>::from_js(ctx, value));
                return result;
            }

            static rest<T> from_js_array(JSContext* ctx, int argc, JSValueConst* argv)
            {
                rest<T> result;
                result.reserve(argc);
                for (int i = 0; i < argc; ++i)
                {
                    result.push_back(TypeConverter<T>::from_js(ctx, argv[i]));
                }
                return result;
            }
        };

        // std::vector<T> converter - generic version
        template <typename T>
        struct TypeConverter<std::vector<T>>
        {
            static JSValue to_js(JSContext* ctx, const std::vector<T>& value)
            {
                JSValue arr = JS_NewArray(ctx);
                for (size_t i = 0; i < value.size(); ++i)
                {
                    JS_SetPropertyUint32(ctx, arr, static_cast<uint32_t>(i),
                                         TypeConverter<T>::to_js(ctx, value[i]));
                }
                return arr;
            }

            static std::vector<T> from_js(JSContext* ctx, JSValueConst value)
            {
                std::vector<T> result;
                int64_t len = 0;
                if (JS_GetLength(ctx, value, &len) == 0 && len > 0)
                {
                    result.reserve(static_cast<size_t>(len));
                    for (int64_t i = 0; i < len; ++i)
                    {
                        JSValue elem = JS_GetPropertyUint32(ctx, value, static_cast<uint32_t>(i));
                        result.push_back(TypeConverter<T>::from_js(ctx, elem));
                        JS_FreeValue(ctx, elem);
                    }
                }
                return result;
            }
        };

        // std::optional<T> converter
        template <typename T>
        struct TypeConverter<std::optional<T>>
        {
            static JSValue to_js(JSContext* ctx, const std::optional<T>& value)
            {
                if (value.has_value())
                {
                    return TypeConverter<T>::to_js(ctx, value.value());
                }
                return JS_NULL;
            }

            static std::optional<T> from_js(JSContext* ctx, JSValueConst value)
            {
                if (JS_IsNull(value) || JS_IsUndefined(value))
                {
                    return std::nullopt;
                }
                return TypeConverter<T>::from_js(ctx, value);
            }
        };

        template <>
        struct TypeConverter<JSValue>
        {
            static JSValue to_js(JSContext* ctx, JSValue value)
            {
                (void)ctx;
                return value;
            }

            static JSValue from_js(JSContext* ctx, JSValueConst value)
            {
                (void)ctx;
                return JS_DupValue(ctx, value);
            }
        };

        template <>
        struct TypeConverter<void>
        {
            static JSValue to_js(JSContext*, const void*)
            {
                return JS_UNDEFINED;
            }

            static void from_js(JSContext*, JSValueConst)
            {
                // nothing to do
            }
        };

        template <typename IntegerType>
        struct TypeConverter<
            IntegerType,
            std::enable_if_t<std::is_integral_v<IntegerType> && !std::is_same_v<IntegerType, bool> &&
                             !std::is_same_v<IntegerType, int32_t> && !std::is_same_v<IntegerType, int64_t> &&
                             !std::is_same_v<IntegerType, uint32_t>>>
        {
            static JSValue to_js(JSContext* ctx, IntegerType value)
            {
                if constexpr (sizeof(IntegerType) <= sizeof(int32_t))
                {
                    return JS_NewInt32(ctx, static_cast<int32_t>(value));
                }
                else
                {
                    return JS_NewInt64(ctx, static_cast<int64_t>(value));
                }
            }

            static IntegerType from_js(JSContext* ctx, JSValueConst value)
            {
                if constexpr (sizeof(IntegerType) <= sizeof(int32_t))
                {
                    int32_t result;
                    if (JS_ToInt32(ctx, &result, value) < 0)
                    {
                        console::error("Failed to convert to integer");
                        QUICKJS_IF_EXCEPTIONS(throw std::runtime_error("Failed to convert to integer"));
                    }
                    return static_cast<IntegerType>(result);
                }
                else
                {
                    int64_t result;
                    if (JS_ToInt64(ctx, &result, value) < 0)
                    {
                        console::error("Failed to convert to integer");
                        QUICKJS_IF_EXCEPTIONS(throw std::runtime_error("Failed to convert to integer"));
                    }
                    return static_cast<IntegerType>(result);
                }
            }
        };
    }
}
