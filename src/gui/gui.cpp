// main.cpp
#include "gui.h"

int runGui(ChessGrid& chessGrid) {
    sf::RenderWindow window(sf::VideoMode(chessGrid.getWidth(), chessGrid.getHeight(), 1), "Racing Kings Chess");
    window.setFramerateLimit(30); // Set the framerate to 30 fps

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                chessGrid.onMouseMoved(mousePos, window);
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    chessGrid.onMouseClick(mousePos, window);
                }
            }
            if (event.type == sf::Event::Resized)
            {
            //      // disable resize
            //     // update the view to the new size of the window
            //     sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
            //     window.setView(sf::View(visibleArea));
                
                // int maxDim = std::max(event.size.width, event.size.height);
                // int minTextureDim = std::min(chessGrid.getTextureSize().x, chessGrid.getTextureSize().y);
                // if (minTextureDim < maxDim) {
                //     chessGrid.loadTextures(maxDim / minTextureDim);
                // }
                
            }
        }

        window.clear();

        chessGrid.draw(window);

        window.display();
    }

    return 0;
}
