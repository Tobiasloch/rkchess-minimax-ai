#include "ChessField.h"

ChessField::ChessField(int x, int y, int width, int height, int character, sf::RectangleShape tile, sf::RectangleShape tileHovering, sf::Sprite piece) : x_(x), y_(y), width_(width), height_(height), character_(character), tile_(tile), tileHovering_(tileHovering), piece_(piece) {}

void ChessField::draw(sf::RenderWindow& window) {
    if (hovering_ || active_) {
        tileHovering_.setSize(sf::Vector2f(width_, height_));
        tileHovering_.setPosition(x_, y_);
        window.draw(tileHovering_);
    } else {
        tile_.setSize(sf::Vector2f(width_, height_));
        tile_.setPosition(x_, y_);
        window.draw(tile_);
    }

    // pieces
    piece_.setPosition(x_ + padding_, y_ + padding_);
    
    float scaleX = static_cast<float>(width_-(2*padding_)) / piece_.getTextureRect().width;
    float scaleY = static_cast<float>(height_-(2*padding_)) / piece_.getTextureRect().height;
    piece_.setScale(scaleX, scaleY);
    
    window.draw(piece_); // Draw the piece sprites on top
}
