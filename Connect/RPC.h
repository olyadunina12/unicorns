#pragma once
#include <vector>
#include <functional>
#include <type_traits>
#include <SFML/Network.hpp>

#include "Serialization.h"
#include "Unicorns.h"

#define REGISTER_RPC(func)  registerRPC(#func, &func)
#define PACK_RPC(func, ...) packRPC<decltype(func)>(#func, std::tuple(__VA_ARGS__))

void bindFunctionToName(const std::string& name, std::function<void(sf::Packet&)> f);
void receiveRPC(sf::Packet& packet);

template<typename S>
struct signature;

template<typename R, typename... Args>
struct signature<R(Args...)>
{
    using return_type = R;
    using argument_type = std::tuple<Args...>;
};

template<typename... Args>
std::string generateNameWithArgs(const char* name)
{
    auto appendWithSpace = [](std::string& str, const char* suffix)
    {
        str += ',';
        str += ' ';
        str += suffix;
    };

    std::string nameWithArgs = name;
    nameWithArgs += '(';

    if constexpr (sizeof...(Args) > 0)
    {
        std::tuple<Args...> params;
        std::apply([&nameWithArgs, appendWithSpace](auto& first, auto&... x) {
            nameWithArgs += typeid(decltype(first)).name();
            (appendWithSpace(nameWithArgs, typeid(decltype(x)).name()), ...);
        }, params);
    }
    nameWithArgs += " )";
    return nameWithArgs;
}

template<typename... Args>
std::string generateNameWithArgs(const char* name, std::tuple<Args...>& params)
{
    return generateNameWithArgs<Args... >(name);
}

template<typename... Args>
void registerRPC(const char *name, void (*f)(Args...))
{
    std::string nameWithArgs = generateNameWithArgs<Args...>(name);

    bindFunctionToName(nameWithArgs, [f](sf::Packet& packet)
    {
        std::tuple<Args...> params;
        std::apply([&packet](auto& ...x) {(operator >>(packet, x), ...); }, params);
        std::apply(f, params);
    });
}

template<typename Func>
sf::Packet packRPC(const char* name, typename signature<Func>::argument_type arguments)
{
    sf::Packet packet;
    packet << PacketType::RPC;

    std::string nameWithArgs = generateNameWithArgs(name, arguments);
    packet << nameWithArgs;

    std::apply([&packet](auto& ...x) { (operator <<(packet, x), ...); }, arguments);
    return packet;
}
