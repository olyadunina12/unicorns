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

struct CardID { sf::Uint8 Value; };
struct PlayerID { sf::Uint8 Value; };

enum class CardType : uint8_t { BabyUnicorn, BasicUnicorn, MagicalUnicorn, Magic, Upgrade, Downgrade, Instant, count };
enum class CardPack : uint8_t { BasePack, AdventuresPack, DragonsPack, RainbowApocalypsePack, UnicornsOfLegendPack, count };

struct Card
{
    std::string Name;
    std::string Description;
    CardType Type;
    CardID ID;
    CardPack Pack;
    sf::Uint8 Copies    : 7;
    sf::Uint8 bIsNarwal : 1;
};

struct Player
{
    std::string Name;
    std::vector<CardID> Hand;
    std::vector<CardID> Stable;
    PlayerID ID;
};