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
#include <mutex>

struct CardVisual
{
    sf::Sprite sprite;
    sf::Vector2f desiredPosition;
    sf::Vector2f desiredScale;
    float desiredRotation;
    float currentRotation;
};

enum Pile : uint8_t { unicorns, bonuses, penalties, hand, count };

struct CardSelection
{
    int id;
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

float gSpreadAmount = 0.195f;
float gCircleSize = 566.f;
float gFanAngleStart = RADIANS(90);
float gFanAngleEnd = RADIANS(46);
float gStartRotation = 10.f;
float gEndRotation = -10.f;
float gShadowAlpha = 80;
float gShadowOutlineAlpha = 75;
float gShadowScale = 1;
float gShadowThickness = 7;
sf::Vector2f gShadowOffset(-18,25);
sf::Vector2f gMainCardShift(0, -165.f);
sf::Vector2f gCommonCardScale(0.8, 0.8);
sf::Vector2f gMainCardScale(1.1, 1.1);
sf::Vector2f gCircleDamping(1, 0.7);

void HandCardPositioning(std::vector<CardVisual>& cards, const sf::Vector2f& cardHandPosition, int chosenIndex)
{
    float startAngle = lerp(gFanAngleStart, gFanAngleEnd, cards.size() / 10.f);
    float endAngle = 3.14 - startAngle;
    for (int i = 0; i < cards.size(); i++)
    {
        float alpha = i / (cards.size() - 1.f);

        if (alpha != alpha)
            alpha = 0.5f;
        
        if (chosenIndex > -1)
        {
            if (i < chosenIndex)
            {
                alpha = lerp(alpha, -0.3f, gSpreadAmount);
            }
            else if (i > chosenIndex)
            {
                alpha = lerp(alpha, 1.3f, gSpreadAmount);
            }
        }

        cards[i].desiredRotation = lerp(gStartRotation, gEndRotation, alpha);

        float angle = lerp(startAngle, endAngle, alpha);
        float x = cos(angle);
        float y = sin(angle) * -0.6;
        sf::Vector2f cardPosition(x, y);
        cardPosition.x *= gCircleDamping.x;
        cardPosition.y *= gCircleDamping.y;
        cardPosition *= gCircleSize;
        cardPosition += cardHandPosition;
        cards[i].desiredPosition = cardPosition;
        cards[i].desiredScale = gCommonCardScale;
        if (chosenIndex > -1 && chosenIndex == i)
        {
            cards[i].desiredPosition += gMainCardShift;
            cards[i].desiredRotation = 0;
            cards[i].desiredScale = gMainCardScale;
        }
    }
}
void stablePositioning(std::vector<CardVisual>& stable, const sf::Vector2f& cardStable, int chosenIndex)
{
    for (int i = 0; i < stable.size(); i++)
    {
        sf::Vector2f cardPosition;
        cardPosition = cardStable;
        cardPosition.y -= (100 * (stable.size() - i - 1));
        stable[i].desiredPosition = cardPosition;
        stable[i].desiredScale = gCommonCardScale;
        if (chosenIndex > -1 && chosenIndex == i)
        {
            stable[i].desiredRotation = 0;
            stable[i].desiredScale = gMainCardScale;
        }
    }
}

int cardChosen(std::vector<CardVisual>& source, int candidate, sf::Vector2f mousePosition)
{
    for (int i = source.size() - 1; i >= 0; i--)
    {
        sf::Vector2f localMousePosition = mousePosition - source[i].sprite.getPosition();

        sf::Transform transform;
        transform.rotate(-source[i].sprite.getRotation());
        localMousePosition = transform.transformPoint(localMousePosition);

        sf::Vector2f scale = source[i].sprite.getScale();
        localMousePosition.x /= scale.x;
        localMousePosition.y /= scale.y;

        localMousePosition += source[i].sprite.getOrigin();

        if (source[i].sprite.getLocalBounds().contains(localMousePosition))
        {
            return i;
        }
    }
    return -1;
}
void simulation(std::vector<CardVisual>& source)
{
    for (int i = 0; i < source.size(); i++)
    {
        sf::Vector2f currentPosition = source[i].sprite.getPosition();
        sf::Vector2f currentScale = source[i].sprite.getScale();
        currentPosition = lerp(currentPosition, source[i].desiredPosition, 0.05);
        currentScale = lerp(currentScale, source[i].desiredScale, 0.3);
        source[i].sprite.setPosition(currentPosition);
        source[i].sprite.setScale(currentScale);
        float currentAngle = source[i].currentRotation;
        currentAngle = lerp(currentAngle, source[i].desiredRotation, 0.3);
        source[i].sprite.setRotation(currentAngle);
        source[i].currentRotation = currentAngle;
    }
}

CardVisual CreateCard(sf::Texture& tex, sf::Vector2f& pos)
{
    CardVisual result;
    result.sprite.setTexture(tex);
    result.sprite.setPosition(pos);
    result.desiredPosition = pos;
    result.desiredRotation = 0;
    result.currentRotation = 0;

    sf::IntRect spriteSize = result.sprite.getTextureRect();
    result.sprite.setOrigin(spriteSize.width / 2, spriteSize.height / 2);
    return result;
}

sf::TcpSocket serverConnection;

void ConnectToServerEntry()
{
    sf::Packet handshake;
    handshake << PacketType::Handshake;
    handshake << HANDSHAKE_MAGIC_STRING;

    sf::UdpSocket socket;
    socket.setBlocking(false);

	sf::TcpListener listener;
	listener.setBlocking(false);

    for (int i = 0; i < 100; ++i)
    {
        socket.bind(4242);
		socket.send(handshake, sf::IpAddress::Broadcast, 4242);
        sf::sleep(sf::seconds(0.5f));

        sf::Socket::Status status = listener.listen(4243);
        for (int i = 0; i < 100; ++i)
        {
			sf::sleep(sf::seconds(0.1f));
            if (status == sf::Socket::Done
                && (status = listener.accept(serverConnection)) == sf::Socket::Done)
            {
                break;
            }
        }
        if (status == sf::Socket::Done)
        {
            break;
        }
    }
}

int main(void)
{
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Unstable Unicorns");
    window.setVerticalSyncEnabled(true);

	sf::Vector2f windowScale(1, 1);

    ImGui::SFML::Init(window);

    std::thread networkingThread;

    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("./assets/background.jpg"))
    {
        printf("No image \n");
    }
    bgTexture.setRepeated(true);
    bgTexture.setSmooth(true);

    sf::Sprite bgSprite;
    bgSprite.setTexture(bgTexture);
    bgSprite.setScale(1.4, 1.4);
    bgSprite.setTextureRect(sf::IntRect(0,0, 2000,2000));

    sf::Texture cardTexture;
    if (!cardTexture.loadFromFile("./assets/base/Baby Unicorn (Green)/img.jpg"))
    {
        printf("No image \n");
    }
    cardTexture.setSmooth(true);

    bool mousePress = false;
    std::vector<CardVisual> cards;
    std::vector<CardVisual> stable1;
    std::vector<CardVisual> stable2;
    std::vector<CardVisual> stable3;
    sf::Vector2f mousePosition;
    sf::Vector2f centerPosition(window.getSize());
    sf::Vector2f cardHandPosition = centerPosition;
    
    sf::Vector2f cardStable1 = centerPosition;
    sf::Vector2f cardStable2 = centerPosition;
    sf::Vector2f cardStable3 = centerPosition;
    cardHandPosition.x /= 1.4;
    cardStable1.x /= 13;
    cardStable1.y /= 1.3;
    cardStable2.x /= 4.6;
    cardStable2.y /= 1.3;
    cardStable3.x /= 2.8;
    cardStable3.y /= 1.3;
    centerPosition.x /= 2;
    
    //create a chosen baby-unicorn
    for (int i = 0; i < 3; ++i)
    {
		CardVisual newCard = CreateCard(cardTexture, cardStable1);
		stable1.push_back(newCard);
    }
    stablePositioning(stable1, cardStable1, -1);
    for (int i = 0; i < 3; ++i)
    {
        CardVisual newCard = CreateCard(cardTexture, cardStable2);
        stable2.push_back(newCard);
    }
    stablePositioning(stable2, cardStable2, -1);
    for (int i = 0; i < 3; ++i)
    {
        CardVisual newCard = CreateCard(cardTexture, cardStable3);
        stable3.push_back(newCard);
    }
    stablePositioning(stable3, cardStable3, -1);

    //int chosenIndex = -1;
    //Pile chosenPile = unicorns;
    CardSelection hoveredCard;

    std::vector<CardVisual>* piles[] = { &stable1, &stable2, &stable3, &cards };
    sf::Vector2f* stablePositions[] = { &cardStable1, &cardStable2, &cardStable3 };

    // run the program as long as the window is open
    ServerHandles server{};
    std::thread serverCommsThread;
    std::mutex  serverCommsLock;
    std::vector<std::string> serverOutput;
    while (window.isOpen())
    {
        ImGui::NewFrame();

#if defined (DEBUG)
        ImGui::SliderFloat("Spread amount", &gSpreadAmount, 0, 1);
        ImGui::SliderFloat2("Main card shift", &gMainCardShift.x, -300, +300);
        ImGui::SliderFloat2("Main card scale", &gMainCardScale.x, 0, 2);
        ImGui::SliderFloat2("Common card scale", &gCommonCardScale.x, 0, 1);
        ImGui::SliderFloat2("gCircleDamping", &gCircleDamping.x, -1, +1);
        ImGui::SliderFloat("Circle size", &gCircleSize, 0, 600);
        ImGui::SliderAngle("Start angle", &gFanAngleStart);
        ImGui::SliderAngle("End angle", &gFanAngleEnd);
        ImGui::SliderFloat("Start rotation", &gStartRotation, -50, 50);
        ImGui::SliderFloat("End rotation", &gEndRotation, -50, 50);
        ImGui::SliderFloat("Start rotation", &gStartRotation, -50, 50);

        ImGui::TextUnformatted("Shadow settings.");
        ImGui::SliderFloat2("Shadow offset", &gShadowOffset.x, -50, 50);
        ImGui::SliderFloat("Shadow Alpha", &gShadowAlpha, 0,255);
        ImGui::SliderFloat("Shadow Outline Alpha", &gShadowOutlineAlpha, 0, 255);
        ImGui::SliderFloat("Shadow Scale", &gShadowScale, 0, 50);
        ImGui::SliderFloat("Shadow Thickness", &gShadowThickness, 0, 50);
#endif
        
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
			ImGui::SFML::ProcessEvent(window, event);

            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setCenter(1920 / 2, 1080 / 2);
                view.setSize(1920, 1080);
                window.setView(view);

                windowScale.x = 1920.f / event.size.width;
                windowScale.y = 1080.f / event.size.height;
            }

 
            //check if mouse is pressed and find out on which card
            if (event.type == sf::Event::MouseButtonPressed)
            {
                mousePress = true;
            }
            //if mouse is released
            if (event.type == sf::Event::MouseButtonReleased)
            {
                hoveredCard.reset();

                HandCardPositioning(cards, cardHandPosition, -1);
                for (int i = 0; i < (Pile::count - 1); i++)
                {
                    stablePositioning(*piles[i], *stablePositions[i], -1);
                }
                
                mousePress = false;
            }
            //if mouse is moved
            if (event.type == sf::Event::MouseMoved)
            {
                mousePosition.x = event.mouseMove.x * windowScale.x;
                mousePosition.y = event.mouseMove.y * windowScale.y;

                if (mousePress)
                    continue;

                int candidate = -1;
                Pile candidatePile = Pile::unicorns;


                for (int i = 0; i < Pile::count; i++)
                {
                    candidate = cardChosen(*piles[i], candidate, mousePosition);
                    if (candidate != -1)
                    {
                        candidatePile = (Pile)i;
                        break;
                    }
                }
                
                //if (chosenIndex != candidate)
                {
                    hoveredCard.id = candidate;
                    hoveredCard.pile = candidatePile;

                    HandCardPositioning(cards, cardHandPosition, -1);
                    for (int i = 0; i < (Pile::count - 1); i++)
                    {
                        stablePositioning(*piles[i], *stablePositions[i], -1);
                    }

                    if (hoveredCard.isValid())
                    {
                        if (candidatePile == hand)
                        {
                            HandCardPositioning(cards, cardHandPosition, hoveredCard.id);
                        }
                        else
                        {
                            stablePositioning(*piles[candidatePile], *stablePositions[candidatePile], hoveredCard.id);
                        }
                    }
                }
            }
            //if key is pressed
            if (event.type == sf::Event::KeyPressed)
            {
                //Escape with ESC button
                if(event.key.code == sf::Keyboard::Escape)
                    window.close();
                //cards appearing when space button is pressed
                else if (event.key.code == sf::Keyboard::Space)
                {
                    CardVisual newCard = CreateCard(cardTexture, cardHandPosition);
                    cards.push_back(newCard);
                    HandCardPositioning(cards, cardHandPosition, -1);
                }

            }
        }

#if defined (DEBUG)
        if (server.proc == nullptr && ImGui::Button("Start server"))
        {
            serverOutput.clear();
            StartServerProcess(server);
        }
        else if (server.proc)
        {
            std::string out;
            if (ReadFromServer(server, out))
            {
                serverOutput.push_back(out);
            }

            ImGui::Begin("Server");
            if (ImGui::Button("Stop server"))
            {
				StopServerProcess(server);
            }
            for (auto& It : serverOutput)
            {
				ImGui::TextUnformatted(It.c_str(), It.c_str() + It.size());
            }
            ImGui::End();
        }

        if (!networkingThread.joinable() && ImGui::Button("Connect to server"))
        {
			networkingThread = std::thread(&ConnectToServerEntry);
        }
#endif

        // cards positioning simulation
        for (int i = 0; i < (Pile::count); i++)
        {
            simulation(*piles[i]);
        }

        
        //card moves after mouse
        if (hoveredCard.isValid() && mousePress)
        {
            CardVisual& chosenCard = piles[hoveredCard.pile]->at(hoveredCard.id);

            chosenCard.desiredPosition = mousePosition;
            chosenCard.desiredRotation = 0;
            sf::Vector2f currentPosition = chosenCard.sprite.getPosition();
            float currentAngle = chosenCard.currentRotation;
            currentPosition = lerp(currentPosition, chosenCard.desiredPosition, 0.1);
            currentAngle = lerp(currentAngle, chosenCard.desiredRotation, 0.2);
            chosenCard.sprite.setPosition(currentPosition);
            chosenCard.sprite.setRotation(currentAngle);
            chosenCard.currentRotation = currentAngle;
        }

        ImGui::EndFrame();

        // draw
        window.clear();
        window.draw(bgSprite);
        //window.draw(card);
        for (int i = 0; i < cards.size(); i++)
        {
            //create shadows for cards
            sf::IntRect texRect = cards[i].sprite.getTextureRect();
            sf::RectangleShape cardShadow(sf::Vector2f(texRect.width, texRect.height));
            cardShadow.setFillColor(sf::Color(0,0,0, gShadowAlpha));
            cardShadow.setPosition(cards[i].sprite.getPosition()+gShadowOffset);
            cardShadow.setOrigin(cards[i].sprite.getOrigin());
            cardShadow.setOutlineColor(sf::Color(0, 0, 0, gShadowOutlineAlpha));
            cardShadow.setScale(cards[i].sprite.getScale() * gShadowScale);
            cardShadow.setOutlineThickness(gShadowThickness);
            cardShadow.setRotation(cards[i].sprite.getRotation());
            window.draw(cardShadow);

            window.draw(cards[i].sprite);
        }
        //draw stables
        for (int i = 0; i < stable1.size(); i++)
        {
            window.draw(stable1[i].sprite);
        }
        for (int i = 0; i < stable2.size(); i++)
        {
            window.draw(stable2[i].sprite);
        }
        for (int i = 0; i < stable3.size(); i++)
        {
            window.draw(stable3[i].sprite);
        }
        
        if (hoveredCard.isValid())
        {
            //if a card is chosen create it's shadow, highlight and draw it again separately
            CardVisual& chosenCard = piles[hoveredCard.pile]->at(hoveredCard.id);

            sf::IntRect texRect = chosenCard.sprite.getTextureRect();
            sf::RectangleShape cardShadow(sf::Vector2f(texRect.width, texRect.height));
            cardShadow.setFillColor(sf::Color(0, 0, 0, gShadowAlpha));
            cardShadow.setPosition(chosenCard.sprite.getPosition() + gShadowOffset);
            cardShadow.setOrigin(chosenCard.sprite.getOrigin());
            cardShadow.setOutlineColor(sf::Color(0, 0, 0, gShadowOutlineAlpha));
            cardShadow.setScale(chosenCard.sprite.getScale()* gShadowScale);
            cardShadow.setOutlineThickness(gShadowThickness);
            cardShadow.setRotation(chosenCard.sprite.getRotation());
            window.draw(cardShadow);

            sf::RectangleShape cardHighlight(sf::Vector2f(texRect.width, texRect.height));
            cardHighlight.setFillColor(sf::Color::Cyan);
            cardHighlight.setPosition(chosenCard.sprite.getPosition());
            cardHighlight.setOrigin(chosenCard.sprite.getOrigin());
            cardHighlight.setOutlineColor(sf::Color(50, 150, 255, 200));
            cardHighlight.setScale(chosenCard.sprite.getScale() * 1.05f);
            cardHighlight.setOutlineThickness(5);
            window.draw(cardHighlight);

			window.draw(chosenCard.sprite);
        }

		ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown(window);

	if (server.proc) StopServerProcess(server);
    if (networkingThread.joinable()) networkingThread.join();
}