#pragma once
#include <vector>
#include <string>

struct CardID { int Value; };
struct PlayerID { int Value; };

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