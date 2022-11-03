#pragma once
#include <vector>
#include <functional>
#include <type_traits>
#include <SFML/Network.hpp>

#include "Serialization.h"
#include "Unicorns.h"

#define REGISTER_RPC(func)  registerRPC(#func, &func)
#define PACK_RPC(func, ...) packRPC(#func, &func, __VA_ARGS__)

void bindFunctionToName(const std::string& name, std::function<void(sf::Packet&)> f);
void receiveRPC(sf::Packet& packet);

inline void appendWithSpace(std::string& str, const char* suffix)
{
    str += ' ';
    str += suffix;
}

template<typename... Args>
std::string generateNameWithArgs(const char* name, void (*f)(Args...))
{
    std::tuple<Args...> params;
    std::string nameWithArgs = name;
    nameWithArgs += '(';
    std::apply([&nameWithArgs](auto& ...x) {(appendWithSpace(nameWithArgs, typeid(decltype(x)).name()), ...); }, params);
    nameWithArgs += " )";
    return nameWithArgs;
}

template<typename... Args>
void registerRPC(const char *name, void (*f)(Args...))
{
    std::string nameWithArgs = generateNameWithArgs(name, f);

    bindFunctionToName(nameWithArgs, [f](sf::Packet& packet)
    {
        std::tuple<Args...> params;
        //if constexpr (sizeof...(arguments) > 0)
        {
            std::apply([&packet](auto& ...x) {(...,operator <<(packet, x)); }, params);
        }
        std::apply(f, params);
    });
}

template<typename ...Args>
sf::Packet packRPC(const char* name, void (*f)(Args...), Args... arguments)
{
    sf::Packet packet;
    packet << PacketType::RPC;

    std::string nameWithArgs = generateNameWithArgs(name, f);
    packet << nameWithArgs;

    if constexpr (sizeof...(arguments) > 0)
    {
        (..., operator >>(packet, arguments));
    }
    return packet;
}
