#include <SFML/Window.hpp>

#include <vector>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Math.h"

struct CardVisual
{
    sf::Sprite sprite;
    sf::Vector2f desiredPosition;
    float desiredRotation;
    float currentRotation;
};

CardVisual CreateCard(sf::Texture& tex, sf::Vector2f& pos)
{
    CardVisual result;
    result.sprite.setTexture(tex);
    result.sprite.setPosition(pos);
    result.desiredPosition = pos;
    result.desiredRotation = 0;
    result.currentRotation = 0;

    sf::IntRect spriteSize = result.sprite.getTextureRect();
    result.sprite.setScale(0.8, 0.8);
    result.sprite.setOrigin(spriteSize.width / 2, spriteSize.height / 2);
    return result;
}

int main(void)
{
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Unstable Unicorns", sf::Style::None);

    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("./assets/background.jpg"))
    {
        printf("No image \n");
    }
    sf::Sprite bgSprite;
    bgSprite.setTexture(bgTexture);
    bgSprite.setScale(2, 2);

    sf::Texture cardTexture;
    if (!cardTexture.loadFromFile("./assets/base/Alluring Narwhal/img.png"))
    {
        printf("No image \n");
    }

    int cardQuantity = 5;
    float start = 3.14 / 4.f;
    float end = 3.14 - start;
    float startAngle = 30.f;
    float endAngle = -30.f;
    sf::Vector2f centerPosition(window.getSize());
    centerPosition.x /= 2;
    std::vector<CardVisual> cards;
    for (int i = 0; i < cardQuantity; i++)
    {
        CardVisual newCard = CreateCard(cardTexture, centerPosition);

        float alpha = i / (cardQuantity - 1.f);

        newCard.desiredRotation = lerp(startAngle, endAngle, alpha);

        float angle = lerp(start, end, alpha);
        float x = cos(angle);
        float y = sin(angle) * -0.6;
        sf::Vector2f cardPosition(x, y);
        cardPosition *= 400.f;
        cardPosition += centerPosition;
        newCard.desiredPosition = cardPosition;
        cards.push_back(newCard);
    }

    sf::Vector2f mousePosition;

    float time = 0;
    CardVisual* currentCard = nullptr;
    // run the program as long as the window is open
    while (window.isOpen())
    {
        time += 0.016;
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setCenter(event.size.width / 2, event.size.height / 2);
                view.setSize(event.size.width, event.size.height);
                window.setView(view);
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                for (int i = 0; i < cards.size(); i++)
                {
                    if (cards[i].sprite.getGlobalBounds().contains(mousePosition))
                    {
                        currentCard = &cards[i];

                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                currentCard = nullptr;
            }
            if (event.type == sf::Event::MouseMoved)
            {
                mousePosition.x = event.mouseMove.x;
                mousePosition.y = event.mouseMove.y;
            }
        }

        // simulate
        for (int i = 0; i < cards.size(); i++)
        {
            sf::Vector2f currentPosition = cards[i].sprite.getPosition();
            float currentAngle = cards[i].currentRotation;
            currentPosition = lerp(currentPosition, cards[i].desiredPosition, 0.005);
            currentAngle = lerp(currentAngle, cards[i].desiredRotation, 0.005);
            cards[i].sprite.setPosition(currentPosition);
            cards[i].sprite.setRotation(currentAngle);
            cards[i].currentRotation = currentAngle;
        }


        if (currentCard)
        {
            currentCard->desiredPosition = mousePosition;
            currentCard->desiredRotation = 0;
        }


        // draw
        window.clear();
        window.draw(bgSprite);
        for (int i = 0; i < cards.size(); i++)
        {
            window.draw(cards[i].sprite);
        }
        window.display();
    }


}