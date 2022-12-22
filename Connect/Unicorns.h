#pragma once
#include <vector>
#include <string>
#include <SFML/Config.hpp>
#include <SFML/Network.hpp>

#define HANDSHAKE_MAGIC_STRING "UNICORNS_HANDSHAKE_MAGIC"

enum class PacketType : sf::Uint8
{
	Invalid,
    Handshake,
    RPC,
};

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
    sf::Uint8 Copies;
};

struct Player
{
    std::string Name;
    std::vector<CardID> Hand;
    std::vector<CardID> Stable;
    PlayerID ID;
};

// server RPCs
void exit_Server();

// client RPCs
void syncDecks_Client(PlayerID owner, std::vector<CardID> hand, std::vector<CardID> stable);

