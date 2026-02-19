#pragma once

#include <type_traits>
#include <vector>

namespace js
{
    // forward declaration
    template <typename T>
    class rest;

    // type trait to check if a type is rest<T>
    template <typename T>
    struct is_rest : std::false_type {};

    template <typename T>
    struct is_rest<rest<T>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_rest_v = is_rest<T>::value;

    // variadic argument template class for receiving any number of arguments
    template <typename T>
    class rest
    {
    public:
        using value_type = T;

        rest() = default;

        void push_back(const T& value) { _data.push_back(value); }
        void push_back(T&& value) { _data.push_back(std::move(value)); }

        T& operator[](size_t index) { return _data[index]; }
        const T& operator[](size_t index) const { return _data[index]; }

        typename std::vector<T>::iterator begin() { return _data.begin(); }
        typename std::vector<T>::iterator end() { return _data.end(); }
        typename std::vector<T>::const_iterator begin() const { return _data.begin(); }
        typename std::vector<T>::const_iterator end() const { return _data.end(); }

        size_t size() const { return _data.size(); }
        bool empty() const { return _data.empty(); }

    private:
        std::vector<T> _data;
    };
}
