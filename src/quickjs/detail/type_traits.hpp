#pragma once

#include <type_traits>

namespace js
{
    namespace detail
    {
        template <typename T>
        struct remove_cvref
        {
            using type = std::remove_cv_t<std::remove_reference_t<T>>;
        };

        template <typename T>
        using remove_cvref_t = typename remove_cvref<T>::type;

        // argument type extraction
        template <size_t N, typename... Args>
        struct ArgType;

        template <typename First, typename... Rest>
        struct ArgType<0, First, Rest...>
        {
            using type = First;
        };

        template <size_t N, typename First, typename... Rest>
        struct ArgType<N, First, Rest...> : ArgType<N - 1, Rest...>
        {
        };

        template <typename F>
        struct FunctionTraits;

        template <typename R, typename... Args>
        struct FunctionTraits<R(Args...)>
        {
            using ReturnType = R;
            static constexpr size_t arity = sizeof...(Args);
            template <size_t N>
            using ArgType = typename detail::ArgType<N, Args...>::type;
        };

        template <typename R, typename... Args>
        struct FunctionTraits<R (*)(Args...)> : FunctionTraits<R(Args...)>
        {
        };

        template <typename C, typename R, typename... Args>
        struct FunctionTraits<R (C::*)(Args...)>
        {
            using ClassType = C;
            using ReturnType = R;
            static constexpr size_t arity = sizeof...(Args);
            template <size_t N>
            using ArgType = typename detail::ArgType<N, Args...>::type;
        };

        template <typename C, typename R, typename... Args>
        struct FunctionTraits<R (C::*)(Args...) const>
        {
            using ClassType = C;
            using ReturnType = R;
            static constexpr size_t arity = sizeof...(Args);
            template <size_t N>
            using ArgType = typename detail::ArgType<N, Args...>::type;
        };
    }
}
