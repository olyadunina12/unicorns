#include <SFML/Window.hpp>

#include <vector>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>
#include <iostream>


int main(void)
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "Unstable Unicorns");

    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("./assets/background.jpg"))
    {
        printf("No image \n");
    }
    sf::Sprite bgSprite;
    bgSprite.setTexture(bgTexture);

    sf::Texture cardTexture;
    if (!cardTexture.loadFromFile("./assets/base/Alluring Narwhal/img.png"))
    {
        printf("No image \n");
    }
    sf::Sprite cardSprite;
    cardSprite.setTexture(cardTexture);

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // simulate

        // draw
        window.clear();
        window.draw(bgSprite);
        window.draw(cardSprite);
        window.display();
    }


}