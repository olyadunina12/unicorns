#pragma once
#include <SFML/Graphics.hpp>

enum class Pile : uint8_t { unicorns, bonuses, penalties, hand, count };

struct CardVisual
{
    sf::Sprite sprite;
    sf::Vector2f desiredPosition;
    sf::Vector2f desiredScale;
    float desiredRotation;
    float currentRotation;
};

struct CardSelection
{
    int id = -1;
    Pile pile;

    void reset()
    {
        id = -1;
    }

    bool isValid()
    {
        return id != -1;
    }
};


void updateCardSettings();
CardVisual createCard(sf::Texture& tex, sf::Vector2f& pos);
void handCardPositioning(std::vector<CardVisual>& cards, const sf::Vector2f& cardHandPosition, int chosenIndex);
void stablePositioning(std::vector<CardVisual>& stable, const sf::Vector2f& cardStable, int chosenIndex);
int cardChosen(std::vector<CardVisual>& source, int candidate, sf::Vector2f mousePosition);
void simulation(std::vector<CardVisual>& source);
void drawShadow(const CardVisual& card, sf::RenderWindow& window);
