#pragma once
#include <SFML/Network.hpp>
#include <type_traits>

template<typename T>
std::enable_if_t<std::is_enum_v<T>, sf::Packet&> operator<<(sf::Packet& packet, T val)
{
    return (packet << std::underlying_type_t<T>(val));
}

template<typename T>
std::enable_if_t<std::is_enum_v<T>, sf::Packet&> operator>>(sf::Packet& packet, T& val)
{
    return (packet >> (std::underlying_type_t<T>&)(val));
}

template <typename T>
std::enable_if_t<std::is_fundamental_v<T>, sf::Packet&> operator<<(sf::Packet& packet, T wrapper)
{
    return (packet << wrapper);
}

template <typename T, typename = int>
struct has_value : std::false_type { };

template <typename T>
struct has_value <T, decltype((void) T::Value, 0)> : std::true_type { };

template<typename T>
std::enable_if_t<has_value<T>::value, sf::Packet&> operator<<(sf::Packet& packet, T wrapper)
{
    return (packet << wrapper.Value);
}

template<typename T>
std::enable_if_t<has_value<T>::value, sf::Packet&> operator>>(sf::Packet& packet, T& wrapper)
{
    return (packet >> wrapper.Value);
}

template <typename T, typename = int>
struct is_container : std::false_type { };

template <typename T>
struct is_container <T, decltype((void) T::operator[], 0)> : std::true_type { };

template<typename T>
std::enable_if_t<is_container<T>::value, sf::Packet&> operator>>(sf::Packet& packet, T& container)
{
    size_t size;
    assert(packet >> size);
    container.resize(size);
    for (int i = 0; i < size; ++i)
    {
        assert(packet >> container[i]);
    }
    return packet;
}

template<typename T>
std::enable_if_t<is_container<T>::value, sf::Packet&> operator<<(sf::Packet& packet, const T& container)
{
    assert(packet << container.size());
    for (int i = 0; i < size; ++i)
    {
        assert(packet >> container[i]);
    }
    return packet;
}

