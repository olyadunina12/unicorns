#pragma once
#include <SFML/Graphics.hpp>
#include "../Connect/Unicorns.h"

struct CardVisual
{
    sf::Sprite sprite;
    sf::Vector2f desiredPosition;
    sf::Vector2f desiredScale;
    float desiredRotation;
    float currentRotation;
    CardID ID;
};

struct PlayerSpace
{
    sf::Text name;
    sf::RectangleShape area;
    std::vector<CardVisual> hand;
    std::vector<CardVisual> stable;
};

void cleanup();
void loadAllTextures();
void updateCardSettings();
CardVisual createIcon(CardID id, sf::Vector2f& pos, CardType type);
CardVisual createCard(CardID id, sf::Vector2f& pos);
sf::Sprite searchSprite(CardID& id);
void handCardPositioning(std::vector<CardVisual>& cards, const sf::Vector2f& cardHandPosition, int chosenIndex);
void stablePositioning(std::vector<CardVisual>& stable, const sf::Vector2f& cardStable, int chosenIndex);
void iconsPositioning(std::vector<CardVisual>& stable, sf::RectangleShape& area);
int cardChosen(std::vector<CardVisual>& source, int candidate, sf::Vector2f mousePosition);
void simulation(std::vector<CardVisual>& source);
void drawShadow(const CardVisual& card, sf::RenderWindow& window);
void drawHighlight(sf::Sprite& picture, sf::RenderWindow& window);
PlayerSpace createPlayerSpace(const std::string& playerName, sf::Font& font, sf::FloatRect area, sf::Texture& gradient);
