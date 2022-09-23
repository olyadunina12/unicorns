#include <SFML/Window.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Math.h"
#include <SFML/Network.hpp>
#include <thread>

#include <imgui.h>
#include <imgui-SFML.h>

#include "../Connect/Unicorns.h"
#include "LocalServer.h"

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

void NetworkingThreadEntry(const sf::RenderWindow* window)
{
    sf::Packet handshake;
    handshake << MAGIC_STRING;

    sf::UdpSocket socket;

    while (window->isOpen())
    {
		socket.send(handshake, sf::IpAddress::Broadcast, 4242);
        sf::sleep(sf::seconds(0.5f));
    }
}

int main(void)
{
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Unstable Unicorns");

    ImGui::SFML::Init(window);

    std::thread networkingThread(&NetworkingThreadEntry, &window);

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
        if (alpha != alpha)
            alpha = 0.5f;

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

    CardVisual* currentCard = nullptr;
    // run the program as long as the window is open
    void* serverProc = nullptr;
    while (window.isOpen())
    {
        ImGui::NewFrame();

        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
			ImGui::SFML::ProcessEvent(window, event);

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
            //check if mouse is pressed and find out on which card
            if (event.type == sf::Event::MouseButtonPressed)
            {
                for (int i = 0; i < cards.size(); i++)
                {
                    sf::Transform transform;
                    //transform.rotate(cards[i].sprite.getRotation()).scale(cards[i].sprite.getScale()).translate(cards[i].sprite.getPosition());
                    //sf::Vector2f localMousePosition = transform.getInverse().transformPoint(mousePosition);
                    
                    sf::Vector2f localMousePosition = mousePosition - cards[i].sprite.getPosition();
                    
                    transform.rotate(-cards[i].sprite.getRotation());
                    localMousePosition = transform.transformPoint(localMousePosition);

                    sf::Vector2f scale = cards[i].sprite.getScale();
                    localMousePosition.x /= scale.x;
                    localMousePosition.y /= scale.y;
                    
                    localMousePosition += cards[i].sprite.getOrigin();

                    if (cards[i].sprite.getLocalBounds().contains(localMousePosition))
                    {
                        currentCard = &cards[i];
                    }
                }
            }
            //defying mouse position
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
        //Escape with ESC button
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            window.close();
        }

        if (!serverProc && ImGui::Button("Start server"))
        {
            serverProc = StartServerProcess();
        }
        else if (serverProc && ImGui::Button("Stop server"))
        {
            StopServerProcess(serverProc);
            serverProc = nullptr;
        }

        // simulate cards in a fan
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

        //card moves after mouse
        if (currentCard)
        {
            currentCard->desiredPosition = mousePosition;
            currentCard->desiredRotation = 0;
        }

        ImGui::EndFrame();

        // draw
        window.clear();
        window.draw(bgSprite);
        for (int i = 0; i < cards.size(); i++)
        {
            window.draw(cards[i].sprite);
        }

		ImGui::SFML::Render(window);
        window.display();
    }

	if (serverProc)
	{
		StopServerProcess(serverProc);
		serverProc = nullptr;
	}

    ImGui::SFML::Shutdown(window);

    networkingThread.join();
}