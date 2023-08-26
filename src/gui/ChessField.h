#ifndef CHESS_FIELD_H
#define CHESS_FIELD_H

#include "board/board.h"
#include <SFML/Graphics.hpp>
#include <iostream>

class ChessField {
    public:

    ChessField(int x=0, int y=0, int width=0, int height=0, int character = 10, sf::RectangleShape tile = sf::RectangleShape(), sf::RectangleShape tileHovering = sf::RectangleShape(), sf::Sprite piece = sf::Sprite());

    void draw(sf::RenderWindow& window);

    // setter
    sf::RectangleShape tile_;
    sf::RectangleShape tileHovering_;
    sf::Sprite piece_;

    int character_;
    int x_;
    int y_;
    int width_;
    int height_;
    bool hovering_ = false;
    bool active_ = false;
    int padding_ = 5;

    int row_;
    int col_;
    private:

};

#endif