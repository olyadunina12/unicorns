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
#include "../Connect/RPC.h"

struct gameState
{
    int players;
    std::vector<CardID> discard;
    std::vector<CardID> poolOfCards;
};


int main(void)
{
    //REGISTER_RPC(playCard);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Unstable Unicorns");

    window.setVerticalSyncEnabled(true);

    sf::Vector2f windowScale(1, 1);

    ImGui::SFML::Init(window);

    std::thread networkingThread;

    int playerCount = 3;

    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("./assets/background.jpg"))
    {
        printf("No image \n");
    }
    bgTexture.setRepeated(true);
    bgTexture.setSmooth(true);

    sf::Texture gradientRect;
    if (!gradientRect.loadFromFile("./assets/gradient.tga"))
    {
        printf("No image \n");
    }
    gradientRect.setSmooth(true);

    sf::Texture fenceV;
    if (!fenceV.loadFromFile("./assets/fenceV.png"))
    {
        printf("No image \n");
    }
    fenceV.setSmooth(true);

    sf::Sprite fenceVert;
    fenceVert.setTexture(fenceV);

    sf::Texture fenceH;
    if (!fenceH.loadFromFile("./assets/fenceH.png"))
    {
        printf("No image \n");
    }
    fenceH.setSmooth(true);

    sf::Sprite fenceHoriz;
    fenceHoriz.setTexture(fenceH);

    sf::Sprite bgSprite;
    bgSprite.setTexture(bgTexture);
    bgSprite.setScale(2, 2.6);
    bgSprite.setTextureRect(sf::IntRect(0,0, 2000,2000));
    //bgSprite.setPosition(-500, -500);

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
    unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
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
    std::vector<CardVisual> discard;

    sf::Vector2f mousePosition;
    sf::Vector2f centerPosition(window.getSize());
    sf::Vector2f cardHandPosition = centerPosition;
    
    sf::Vector2f cardStable1 = centerPosition;
    sf::Vector2f cardStable2 = centerPosition;
    sf::Vector2f cardStable3 = centerPosition;
    cardHandPosition.x /= 1.4f;
    cardStable1.x /= 13.f;
    cardStable1.y /= 1.25f;
    cardStable2.x /= 4.6f;
    cardStable2.y /= 1.25f;
    cardStable3.x /= 2.8f;
    cardStable3.y /= 1.25f;
    centerPosition.x /= 2.f;

    CardSelection hoveredCard;
    //create stables and hand for current player
    std::vector<CardVisual>* piles[] = { &stable1, &stable2, &stable3, &cards };
    sf::Vector2f* stablePositions[] = { &cardStable1, &cardStable2, &cardStable3 };   

    //create a rectangle for stables
    sf::Vector2f rectStableSize(815, 920);
    sf::Vector2f rectStablePosition(3, 150);
    sf::RectangleShape myStableArea(rectStableSize);
    myStableArea.setPosition(rectStablePosition);
    myStableArea.setOutlineThickness(3);
    //myStableArea.setFillColor(sf::Color::Transparent);
    myStableArea.setTexture(&gradientRect);
    //myStableArea.setTextureRect(sf::IntRect(1, 1, 2, 2));
    float transparency = 0;

    sf::Vector2f rectTableSize(1070, 570);
    sf::Vector2f rectTablePosition(820, 500);
    sf::RectangleShape myTableArea(rectTableSize);
    myTableArea.setPosition(rectTablePosition);
    myTableArea.setOutlineThickness(3);
    myTableArea.setTexture(&gradientRect);


    sf::Font font;
    if (!font.loadFromFile("./assets/AmaticSC-Regular.ttf"))
    {
        std::cout << "No font!" << std::endl;
    }

    //create other players and give them hand and a basic unicorn
    std::vector<PlayerSpace> otherPlayers;
    for (int i = 0; i < playerCount; ++i)
    {
        sf::FloatRect area;

        float step = rectTableSize.x / playerCount;
        area.left = rectTablePosition.x + step * i;
        area.top = 5;
        area.width = step;
        area.height = rectTablePosition.y - 5;
        otherPlayers.push_back(createPlayerSpace("player", font, area, gradientRect));
        sf::Vector2f pos2;
        pos2.x = area.left + 10;
        pos2.y = 170;
        CardVisual newBabyCard = createIcon(poolOfBabies.back(), pos2, CardType::BabyUnicorn);
        poolOfBabies.pop_back();
        otherPlayers[i].stable.push_back(newBabyCard);
    }

    for (PlayerSpace& person : otherPlayers)
    {
        iconsPositioning(person.stable, person.area);
    }

    //create a baby-unicorn for myself
    CardVisual newBabyCard = createCard(poolOfBabies.back(), cardStable1);
    poolOfBabies.pop_back();
    stable1.push_back(newBabyCard);

    //draw 5 cards in the beginning
    for (int i = 0; i < 5; i++)
    {
        CardVisual newCard = createCard(poolOfCards.back(), cardHandPosition);
        poolOfCards.pop_back();
        cards.push_back(newCard);
        handCardPositioning(cards, cardHandPosition, -1);
    }

    // run the program as long as the window is open
    ServerHandles server{};
    std::thread serverCommsThread;
    std::mutex  serverCommsLock;
    std::vector<std::string> serverOutput;
    while (window.isOpen())
    {
        ImGui::NewFrame();

        updateCardSettings();
        // draw
        window.clear();
        window.draw(bgSprite);

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
                view.setSize(1920, 1080);
                view.setCenter(sf::Vector2f(1920 / 2, 1080 / 2));
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
                sf::FloatRect myStableBounds = myStableArea.getGlobalBounds();
                sf::FloatRect myTableBounds = myTableArea.getGlobalBounds();
                bool otherPlayerIncl = false;
                int player = -1;
                for (int i = 0; i < playerCount; i++)
                {
                    if (otherPlayers[i].area.getGlobalBounds().contains(mousePosition))
                    {
                        otherPlayerIncl = true;
                        player = i;
                    }
                }

                if (hoveredCard.isValid() && hoveredCard.pile == Pile::hand)
                {
                    CardID ChosenId = cards[hoveredCard.id].ID;
                    CardType type = cardDescs[ChosenId.Value].Type;
                    if(myStableBounds.contains(mousePosition))
                    { 
                        if (type == CardType::Upgrade)
                        {
                            stable2.push_back(cards[hoveredCard.id]);
                            cards.erase(cards.begin() + hoveredCard.id);
                        }
                        else if (type == CardType::Downgrade)
                        {
                            stable3.push_back(cards[hoveredCard.id]);
                            cards.erase(cards.begin() + hoveredCard.id);
                        }
                        else if (type == CardType::BabyUnicorn || type == CardType::MagicalUnicorn || type == CardType::BasicUnicorn)
                        {
                            stable1.push_back(cards[hoveredCard.id]);
                            cards.erase(cards.begin() + hoveredCard.id);
                        }
                    }
                    else if (otherPlayerIncl == true)
                    {                  
                        if (type != CardType::Magic && type != CardType::Instant)
                        {
                            otherPlayers[player].stable.push_back(createIcon(ChosenId, cards[hoveredCard.id].desiredPosition, type));
                            cards.erase(cards.begin() + hoveredCard.id);
                            player = -1;
                            otherPlayerIncl = false;
                        }
                    }
                    if (!myTableBounds.contains(mousePosition) && type == CardType::Magic || type == CardType::Instant)
                    {
                        discard.push_back(cards[hoveredCard.id]);
                        discard.back().desiredScale.x = 0;

                        cards.erase(cards.begin() + hoveredCard.id);
                    }
                }

                hoveredCard.reset();

                for (PlayerSpace& person : otherPlayers)
                {
                    iconsPositioning(person.stable, person.area);
                }
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

                //sf::View view;
                //view.setSize(1920, 1080);
                //sf::Vector2f viewOffset = (mousePosition - view.getSize() * 0.5f) / 2.f;
                //view.setCenter(sf::Vector2f(1920 / 2, 1080 / 2) + viewOffset);
                //window.setView(view);
                //mousePosition += viewOffset;

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
        ImGui::Begin("Server");

        if (!networkingThread.joinable())
        {
            static char name[64];
            ImGui::InputText("Name: ", name, sizeof(name));
            if (ImGui::Button("Connect to server"))
            {
                networkingThread = std::thread(&connectToServerEntry, name);
            }
        }

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

            if (ImGui::Button("Stop server"))
            {
                StopServerProcess(server);
            }
            for (auto& It : serverOutput)
            {
                ImGui::TextUnformatted(It.c_str(), It.c_str() + It.size());
            }
        }

        ImGui::End();
#endif

        // cards positioning simulation
        for (int i = 0; i < (int)Pile::count; i++)
        {
            simulation(*piles[i]);
        }
        for (PlayerSpace& person : otherPlayers)
        {
            simulation(person.stable);
        }
        simulation(discard);
        
        // draw
        window.clear();
        window.draw(bgSprite);

        for (PlayerSpace& person : otherPlayers)
        {
            sf::FloatRect area = person.area.getGlobalBounds();
            fenceVert.setPosition(area.left, area.height - fenceVert.getGlobalBounds().height);
            window.draw(fenceVert);
        }

        float widthPlayers = rectTableSize.x;
        float widthFence = fenceHoriz.getLocalBounds().width;
        int fenceQuantity = widthPlayers/ widthFence;
        float a = (widthFence * fenceQuantity - widthPlayers)/3;

        for (int i = 0; i <= fenceQuantity; i++)
        {
            fenceHoriz.setPosition(rectTablePosition.x + widthFence * i, rectTablePosition.y);
            window.draw(fenceHoriz);
        }
                   
        //window.draw();

        //card moves after mouse
        if (hoveredCard.isValid() && mousePress)
        {
            CardVisual& chosenCard = piles[(int)hoveredCard.pile]->at(hoveredCard.id);

            chosenCard.desiredPosition = mousePosition;
            chosenCard.desiredRotation = 0;
            sf::Vector2f currentPosition = chosenCard.sprite.getPosition();
            float currentAngle = chosenCard.currentRotation;
            currentPosition = lerp(currentPosition, chosenCard.desiredPosition, 0.1f);
            currentAngle = lerp(currentAngle, chosenCard.desiredRotation, 0.2f);
            chosenCard.sprite.setPosition(currentPosition);
            chosenCard.sprite.setRotation(currentAngle);
            chosenCard.currentRotation = currentAngle;

            sf::FloatRect myStableBounds = myStableArea.getGlobalBounds();

            bool otherPlayerIncl = false;
            int player = -1;
            for (int i = 0; i < playerCount; i++)
            {
                if (otherPlayers[i].area.getGlobalBounds().contains(mousePosition))
                {
                    player = i;
                    otherPlayerIncl = true;
                }
            }
            CardID ChosenId = cards[hoveredCard.id].ID;
            if (myStableBounds.contains(mousePosition) && cardDescs[ChosenId.Value].Type != CardType::Instant && cardDescs[ChosenId.Value].Type != CardType::Magic)
            {
                myStableArea.setOutlineColor(sf::Color(255, 255, 255, (sf::Uint8)transparency));
                myStableArea.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)transparency));
                transparency = lerp(transparency, 200.f, 0.03f);
                window.draw(myStableArea);
            }
            else if (otherPlayerIncl == true && cardDescs[ChosenId.Value].Type != CardType::Instant && cardDescs[ChosenId.Value].Type != CardType::Magic)
            {
                otherPlayers[player].area.setOutlineColor(sf::Color(255, 255, 255, (sf::Uint8)transparency));
                otherPlayers[player].area.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)transparency));
                transparency = lerp(transparency, 200.f, 0.03f);
                window.draw(otherPlayers[player].area);
                bool otherPlayerIncl = false;
                int player = -1;
            }
            else
                transparency = 0;
        }

        ImGui::EndFrame();

        //window.draw(card);
        for (int i = 0; i < cards.size(); i++)
        {
            drawShadow(cards[i], window);

            window.draw(cards[i].sprite);
        }

        //window.draw(myTableArea);

        for (auto& player : otherPlayers)
        {
            window.draw(player.name);
            for (auto& icon : player.stable)
            {
                window.draw(icon.sprite);
            }
        }

        //draw discard
        for (int i = 0; i < discard.size(); i++)
        {
            window.draw(discard[i].sprite);
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

            sf::Sprite picture = chosenCard.sprite;

            drawHighlight(picture, window);

            window.draw(chosenCard.sprite);
        }
        else
        {
            for (PlayerSpace& person : otherPlayers)
            {
                if (person.area.getGlobalBounds().contains(mousePosition))
                {
                    for (int i = 0; i < person.stable.size(); i++)
                    {
                        sf::FloatRect iconBounds = person.stable[i].sprite.getGlobalBounds();
                        if (iconBounds.contains(mousePosition) && !mousePress)
                        {
                            CardID id = person.stable[i].ID;
                            sf::Sprite cardToDraw = searchSprite(id);

                            sf::Vector2f position(iconBounds.left + iconBounds.width/2, iconBounds.height + iconBounds.top + cardToDraw.getOrigin().y);
                            position.x = clamp(position.x, cardToDraw.getGlobalBounds().width / 2, window.getSize().x - cardToDraw.getGlobalBounds().width/2);
                            cardToDraw.setPosition(position); 

                            drawHighlight(cardToDraw, window);

                            window.draw(cardToDraw);
                        }
                    }
                }
            }
        }        

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown(window);

    if (server.proc) StopServerProcess(server);
    if (networkingThread.joinable()) networkingThread.join();

    cleanup();
}