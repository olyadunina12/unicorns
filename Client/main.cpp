#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <thread>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <chrono>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include "../Connect/Unicorns.h"
#include "../Connect/Parsing.h"
#include "LocalServer.h"
#include "CardVisuals.h"
#include "Math.h"
#include "Networking.h"

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
    bgSprite.setScale(1, 1.3);
    bgSprite.setTextureRect(sf::IntRect(0,0, 2000,2000));

    //create a full set of cards with descriptions, types and pack names
    std::vector<Card> cardDescs;
    std::string path = "./assets/";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        std::string fullPath = entry.path().string();
        if (!endsWith(fullPath, "description.txt"))
        {
            continue;
        }
        Card desc;

        std::vector<std::string> text = split(fullPath, "\\/.");
        if (text[1] == "adventures")
            desc.Pack = CardPack::AdventuresPack;
        else if (text[1] == "base")
            desc.Pack = CardPack::BasePack;
        else if (text[1] == "dragons")
            desc.Pack = CardPack::DragonsPack;
        else if (text[1] == "rainbow-apocalypse")
            desc.Pack = CardPack::RainbowApocalypsePack;
        else if (text[1] == "unicorns-of-legend")
            desc.Pack = CardPack::UnicornsOfLegendPack;
        else
            std::cerr << "Pack not found " << text[1] << std::endl;
        desc.Name = text[2];
        std::vector<std::string> descriptionCard = fileRead(fullPath);
        std::string type = split(descriptionCard[0], ":")[1];
        if (type == " Baby Unicorn")
            desc.Type = CardType::BabyUnicorn;
        else if (type == " Basic Unicorn")
            desc.Type = CardType::BasicUnicorn;
        else if (type == " Magical Unicorn")
            desc.Type = CardType::MagicalUnicorn;
        else if (type == " Magic")
            desc.Type = CardType::Magic;
        else if (type == " Upgrade")
            desc.Type = CardType::Upgrade;
        else if (type == " Downgrade")
            desc.Type = CardType::Downgrade;
        else if (type == " Instant")
            desc.Type = CardType::Instant;
        else
            std::cerr << "Type not found " << type << std::endl;
        std::string copies = split(descriptionCard[1], ": ")[1];
        desc.Copies = stoi(copies);
        std::string description = descriptionCard[2];
        desc.Description = description.substr(13);

        std::string id = split(descriptionCard[3], ": ")[1];
        desc.ID.Value = stoi(id);
        cardDescs.push_back(desc);
    }
    //create a full pool of cards with all copies by ID
    std::vector<CardID> poolOfCards = {};

    for (int i = 0; i < cardDescs.size(); i++)
    {
        for (int j = 0; j < cardDescs[i].Copies; j++)
        {
            if(cardDescs[i].Type != CardType::BabyUnicorn)
                poolOfCards.push_back(cardDescs[i].ID);
        }
    }
    //create a pool of baby-unicorns with all copies by id
    std::vector<CardID> poolOfBabies = {};

    for (int i = 0; i < cardDescs.size(); i++)
    {
        for (int j = 0; j < cardDescs[i].Copies; j++)
        {
            if (cardDescs[i].Type == CardType::BabyUnicorn)
                poolOfBabies.push_back(cardDescs[i].ID);
        }
    }
    //randomize the order of the cards
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(poolOfCards.begin(), poolOfCards.end(), std::default_random_engine(seed));

    //randomize the order of baby-unicorns
    std::shuffle(poolOfBabies.begin(), poolOfBabies.end(), std::default_random_engine(seed));
      
    //create card texture
    loadAllTextures();

    //set stable and hand positions
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
    CardVisual newCard = createCard(poolOfBabies.back(), cardStable1);
    poolOfCards.pop_back();
    stable1.push_back(newCard);

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

        updateCardSettings();
        
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

                handCardPositioning(cards, cardHandPosition, -1);
                for (int i = 0; i < ((int)Pile::count - 1); i++)
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


                for (int i = 0; i < (int)Pile::count; i++)
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

                    handCardPositioning(cards, cardHandPosition, -1);
                    for (int i = 0; i < ((int)Pile::count - 1); i++)
                    {
                        stablePositioning(*piles[i], *stablePositions[i], -1);
                    }

                    if (hoveredCard.isValid())
                    {
                        if (candidatePile == Pile::hand)
                        {
                            handCardPositioning(cards, cardHandPosition, hoveredCard.id);
                        }
                        else
                        {
                            stablePositioning(*piles[(int)candidatePile], *stablePositions[(int)candidatePile], hoveredCard.id);
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
                    CardVisual newCard = createCard(poolOfCards.back(), cardHandPosition);
                    poolOfCards.pop_back();
                    cards.push_back(newCard);
                    handCardPositioning(cards, cardHandPosition, -1);
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
			networkingThread = std::thread(&connectToServerEntry);
        }
#endif

        // cards positioning simulation
        for (int i = 0; i < (int)Pile::count; i++)
        {
            simulation(*piles[i]);
        }

        
        //card moves after mouse
        if (hoveredCard.isValid() && mousePress)
        {
            CardVisual& chosenCard = piles[(int)hoveredCard.pile]->at(hoveredCard.id);

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
            drawShadow(cards[i], window);

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
            CardVisual& chosenCard = piles[(int)hoveredCard.pile]->at(hoveredCard.id);

            drawShadow(chosenCard, window);

            sf::IntRect texRect = chosenCard.sprite.getTextureRect();
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