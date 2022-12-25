#pragma once
#include <vector>
#include <string>
#include <SFML/Config.hpp>
#include <SFML/Network.hpp>

#define HANDSHAKE_MAGIC_STRING "UNICORNS_HANDSHAKE_MAGIC"

enum class PacketType : uint8_t { Invalid, Handshake, RPC };

enum class CardPile : uint8_t { Unicorns, Bonuses, Penalties, Hand, count };
enum class CardType : uint8_t { BabyUnicorn, BasicUnicorn, MagicalUnicorn, Magic, Upgrade, Downgrade, Instant, count };
enum class CardPack : uint8_t { BasePack, AdventuresPack, DragonsPack, RainbowApocalypsePack, UnicornsOfLegendPack, count };

struct CardID   { sf::Uint8 Value; };
struct PlayerID { sf::Uint8 Value; };

static const CardID   InvalidCardID   { -1 };
static const PlayerID InvalidPlayerID { -1 };

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

