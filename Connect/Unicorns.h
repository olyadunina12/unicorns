#pragma once
#include <vector>
#include <string>
#include <SFML/Config.hpp>

#define MAGIC_STRING "UNICORNS_HANDSHAKE_MAGIC"
    
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