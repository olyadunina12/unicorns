#pragma once
#include <SFML/Network.hpp>
#include <type_traits>
#include <cassert>

template<typename T, typename S>
std::enable_if_t<std::is_enum_v<T>, S&> operator<<(S& stream, T val)
{
    stream << typeid(decltype(val)).name();
    return (stream << std::underlying_type_t<T>(val));
}

template<typename T, typename S>
std::enable_if_t<std::is_enum_v<T>, S&> operator>>(S& stream, T& val)
{
    std::string name;
    stream >> name;

    return (stream >> (std::underlying_type_t<T>&)(val));
}

template <typename T, typename S>
std::enable_if_t<std::is_fundamental_v<T>, S&> operator<<(S& stream, T wrapper)
{
    stream << typeid(decltype(val)).name();
    return (stream << wrapper);
}

template <typename T, typename S>
std::enable_if_t<std::is_fundamental_v<T>, S&> operator>>(S& stream, T& wrapper)
{
    std::string name;
    stream >> name;

    return (stream >> val);
}

template <typename T, typename = int>
struct has_value : std::false_type { };

template <typename T>
struct has_value <T, decltype((void) T::Value, 0)> : std::true_type { };

template<typename T, typename S>
std::enable_if_t<has_value<T>::value, S&> operator<<(S& stream, T wrapper)
{
    stream << typeid(decltype(wrapper)).name();
    return (stream << wrapper.Value);
}

template<typename T, typename S>
std::enable_if_t<has_value<T>::value, S&> operator>>(S& stream, T& wrapper)
{
    std::string name;
    stream >> name;

    return (stream >> wrapper.Value);
}

template<typename T, typename S>
S& operator>>(S& stream, std::vector<T>& container)
{
    std::string name;
    stream >> name;

    size_t size;
    assert(stream >> size);
    container.resize(size);
    for (int i = 0; i < size; ++i)
    {
        assert(stream >> container[i]);
    }
    return stream;
}

template<typename T, typename S>
S& operator<<(S& stream, const std::vector<T>& container)
{
    stream << typeid(decltype(container)).name();
    auto size = container.size();
    assert(stream << size);
    for (int i = 0; i < size; ++i)
    {
        assert(stream << container[i]);
    }
    return stream;
}

