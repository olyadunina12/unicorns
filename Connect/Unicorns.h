#pragma once
#include <vector>
#include <string>
#include <SFML/Config.hpp>
#include <SFML/Network.hpp>

#define HANDSHAKE_MAGIC_STRING "UNICORNS_HANDSHAKE_MAGIC"

enum class PacketType : sf::Uint8
{
    Handshake,
    Info,
};

template<typename T>
sf::Packet& operator<<(sf::Packet& packet, T val)
{
    return (packet << std::underlying_type_t<T>(val));
}

template<typename T>
sf::Packet& operator>>(sf::Packet& packet, T& val)
{
    return (packet >> (std::underlying_type_t<T>&)(val));
}

struct CardID { sf::Int32 Value; };
struct PlayerID { sf::Int32 Value; };

struct Card
{
    CardID ID;
    std::string Description;
};

struct Player
{
    PlayerID ID;
    std::string Name;
    std::vector<CardID> Hand;
    std::vector<CardID> Stable;
};