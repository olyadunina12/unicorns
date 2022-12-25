#include "CardVisuals.h"
#include "Math.h"
#include "../Connect/Parsing.h"

#include "imgui.h"
#include <filesystem>

std::vector<sf::Texture> gTextures;

std::vector<sf::Texture> gMiniTextures;

void cleanup()
{
    gTextures.clear();
    gMiniTextures.clear();
}

void loadAllTextures()
{
    std::string path = "./assets/";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        sf::Texture cardTexture;
        std::string fullPath = entry.path().string();
        if (!endsWith(fullPath, "pic.jpg"))
        {
            continue;
        }
        if (!cardTexture.loadFromFile(fullPath))
        {
            printf("No image \n");
        }
        cardTexture.setSmooth(true);
        gTextures.push_back(cardTexture);
    }
 
    gMiniTextures.resize(int(CardType::count));

    if (!gMiniTextures[int(CardType::BabyUnicorn)].loadFromFile("./assets/cardTypes/babyU1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::BasicUnicorn)].loadFromFile("./assets/cardTypes/basicU1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::MagicalUnicorn)].loadFromFile("./assets/cardTypes/magicU1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::Magic)].loadFromFile("./assets/cardTypes/magic1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::Upgrade)].loadFromFile("./assets/cardTypes/upgrade1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::Downgrade)].loadFromFile("./assets/cardTypes/downgrade1.png"))
    {
        printf("No image \n");
    }
    if (!gMiniTextures[int(CardType::Instant)].loadFromFile("./assets/cardTypes/instant1.png"))
    {
        printf("No image \n");
    }
}

float gSpreadAmount = 0.195f;
float gCircleSize = 566.f;
float gFanAngleStart = RADIANS(90.f);
float gFanAngleEnd = RADIANS(46.f);
float gStartRotation = 10.f;
float gEndRotation = -10.f;
float gShadowAlpha = 80;
float gShadowOutlineAlpha = 75;
float gShadowScale = 1;
float gShadowThickness = 7;
sf::Vector2f gShadowOffset(-18.f, 25.f);
sf::Vector2f gMainCardShift(0.f, -165.f);
sf::Vector2f gCommonCardScale(0.8f, 0.8f);
sf::Vector2f gMainCardScale(1.1f, 1.1f);
sf::Vector2f gCircleDamping(1.f, 0.7f);

void updateCardSettings()
{
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

    ImGui::TextUnformatted("Shadow settings.");
    ImGui::SliderFloat2("Shadow offset", &gShadowOffset.x, -50, 50);
    ImGui::SliderFloat("Shadow Alpha", &gShadowAlpha, 0, 255);
    ImGui::SliderFloat("Shadow Outline Alpha", &gShadowOutlineAlpha, 0, 255);
    ImGui::SliderFloat("Shadow Scale", &gShadowScale, 0, 50);
    ImGui::SliderFloat("Shadow Thickness", &gShadowThickness, 0, 50);
#endif
}

CardVisual createIcon(CardID id, sf::Vector2f& pos, CardType type)
{
    CardVisual result;
    result.sprite.setTexture(gMiniTextures[int(type)]);
    result.sprite.setPosition(pos);
    result.desiredPosition = pos;
    result.desiredRotation = 0;
    result.currentRotation = 0;
    result.desiredScale = sf::Vector2f(1,1);
    result.ID = id;

    sf::IntRect spriteSize = result.sprite.getTextureRect();
    result.sprite.setOrigin(spriteSize.width / 2.f, spriteSize.height / 2.f);
    return result;
}

CardVisual createCard(CardID id, sf::Vector2f& pos)
{
    CardVisual result;
    result.sprite.setTexture(gTextures[id.Value]);
    result.sprite.setPosition(pos);
    result.desiredPosition = pos;
    result.desiredRotation = 0;
    result.desiredScale = sf::Vector2f(1,1);
    result.currentRotation = 0;
    result.ID = id;

    sf::IntRect spriteSize = result.sprite.getTextureRect();
    result.sprite.setOrigin(spriteSize.width / 2.f, spriteSize.height / 2.f);
    return result;
}

sf::Sprite searchSprite(CardID &id)
{
    sf::Sprite result;
    result.setTexture(gTextures[id.Value]);
    sf::IntRect spriteSize = result.getTextureRect();
    result.setOrigin(spriteSize.width / 2, spriteSize.height / 2);
    return result;
}

void handCardPositioning(std::vector<CardVisual>& cards, const sf::Vector2f& cardHandPosition, int chosenIndex)
{
    float startAngle = lerp(gFanAngleStart, gFanAngleEnd, cards.size() / 10.f);
    float endAngle = 3.14f - startAngle;
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
        float y = sin(angle) * -0.6f;
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

void iconsPositioning(std::vector<CardVisual>& stable, sf::RectangleShape& area)
{
    for (int i = 0; i < stable.size(); i++)
    {
        sf::Vector2f cardPosition;
        auto rect = stable[i].sprite.getLocalBounds();
        sf::Vector2f iconHalfSize(rect.width/2,rect.height/2);
        cardPosition = area.getSize() + area.getPosition() - iconHalfSize;
        int columnWidth = (int)(area.getSize().x / rect.width)-1;
        int row = i / columnWidth;
        int column = i % columnWidth;
        cardPosition.x -= rect.width * column;
        cardPosition.y -= rect.height * row;
        stable[i].desiredPosition = cardPosition;
    }
}

int cardChosen(std::vector<CardVisual>& source, int candidate, sf::Vector2f mousePosition)
{
    for (int i = (int)source.size() - 1; i >= 0; i--)
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
        currentPosition = lerp(currentPosition, source[i].desiredPosition, 0.05f);
        currentScale = lerp(currentScale, source[i].desiredScale, 0.3f);
        source[i].sprite.setPosition(currentPosition);
        source[i].sprite.setScale(currentScale);
        float currentAngle = source[i].currentRotation;
        currentAngle = lerp(currentAngle, source[i].desiredRotation, 0.3f);
        source[i].sprite.setRotation(currentAngle);
        source[i].currentRotation = currentAngle;
    }
}

void drawShadow(const CardVisual& card, sf::RenderWindow& window)
{
    //create shadows for cards
    sf::IntRect texRect = card.sprite.getTextureRect();
    sf::RectangleShape cardShadow(sf::Vector2f((float)texRect.width, (float)texRect.height));
    cardShadow.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)gShadowAlpha));
    cardShadow.setPosition(card.sprite.getPosition() + gShadowOffset);
    cardShadow.setOrigin(card.sprite.getOrigin());
    cardShadow.setOutlineColor(sf::Color(0, 0, 0, (sf::Uint8)gShadowOutlineAlpha));
    cardShadow.setScale(card.sprite.getScale() * gShadowScale);
    cardShadow.setOutlineThickness(gShadowThickness);
    cardShadow.setRotation(card.sprite.getRotation());
    
    window.draw(cardShadow);
}

void drawHighlight(sf::Sprite& picture, sf::RenderWindow& window)
{
    sf::IntRect texRect = picture.getTextureRect();
    sf::RectangleShape cardHighlight(sf::Vector2f(texRect.width, texRect.height));
    cardHighlight.setFillColor(sf::Color::Cyan);
    sf::Vector2f position = picture.getPosition();
    cardHighlight.setPosition(position);
    cardHighlight.setOrigin(picture.getOrigin());
    cardHighlight.setOutlineColor(sf::Color(50, 150, 255, 200));
    cardHighlight.setScale(picture.getScale() * 1.05f);
    cardHighlight.setOutlineThickness(5);
    window.draw(cardHighlight);
}

PlayerSpace createPlayerSpace(const std::string& playerName, sf::Font& font, sf::FloatRect area, sf::Texture& gradient)
{
    PlayerSpace result;
    result.area.setPosition(sf::Vector2f(area.left, area.top));
    result.area.setSize(sf::Vector2f(area.width, area.height));
    result.area.setOutlineThickness(3);
    result.area.setTexture(&gradient);

    result.name.setPosition(sf::Vector2f(area.left + area.width / 2, area.top));
    result.name.setString(playerName);
    result.name.setFillColor(sf::Color::Red);
    result.name.setColor(sf::Color::Red);
    result.name.setOutlineColor(sf::Color::White);
    result.name.setOutlineThickness(4.5);
    result.name.setCharacterSize(120);
    result.name.setStyle(sf::Text::Bold);
    result.name.setFont(font);
    result.name.setOrigin(result.name.getLocalBounds().width / 2, 0);

    return result;
}