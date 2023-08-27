#ifndef CHESS_GRID_H
#define CHESS_GRID_H

#include "board/board.h"
#include "board/boardState.h"
#include "board/moveIterator.h"
#include "board/moveGen/moveGen.h"
#include "ai/minimax.h"

#include "ChessField.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <chrono>

#include "sf-svg/SFC/Svg.hpp"

#define CHESS_GRID_SIZE 8

#define HUMAN_PLAYER 0
#define AI_PLAYER 1

#define SVGIMAGES


class ChessGrid {
public:
    ChessGrid(
            int x = 0,
            int y = 0,
            int width = 640,
            int height = 640);

    void loadTextures();
    #ifdef SVGIMAGES
        void updateTextureScale(int widnowWidth, int windowHeight);
    #endif

    void draw(sf::RenderWindow& window);
    // virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void setBoard(struct board* board);
    void setBoard(char* fen);
    void setPiece(int row, int col, int character);

    // Events
    void onMouseMoved(sf::Vector2i& mousePos, const sf::Window& window);
    void onMouseClick(sf::Vector2i& mousePos, const sf::Window& window);

    ChessField* fieldFromPixels(int x, int y);
    sf::Vector2i normalise(sf::Vector2i& vec, const sf::Window& window);
    uint8_t to_position(int row, int col);

    void activateField(ChessField& field);

    int getGridSize() const { return CHESS_GRID_SIZE; }

    int getTileSizeX() const { return (width_-(2*margin_)) / getGridSize(); }
    int getTileSizeY() const { return (height_-(2*margin_)) / getGridSize(); }
    
    void updateCalculatingAnimation(sf::Text& text);
    void resetCalculatingAnimation();

    void resetActiveFields();

    void checkForEndGame();

    void startAI();
    void doMove(int from_row, int from_col, int to_row, int to_col);
    void doMove(struct move* m);

    struct board* getBoard() { return &board_; }

    // getter for height and width
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // setter for playertypes
    void setPlayerType(int player, int type);
    // getter for playertypes
    int getPlayerType(int player) const { return playerTypes_[player]; }

    uint verbosity = 0; // 1: print moves, 2: print board and moves

    int aiMaxDepth = 9;
    long aiMaxTime = 1000; // ms
    int aiThreadCount_ = 0;
private:
    int margin_ = 20;
    ChessField fields_[8][8];
    sf::RectangleShape tiles_[8][8];
    sf::RectangleShape background_;
    sf::Font font_;
    sf::Text fieldLabels_[2][8];
    sf::Text currentPlayerText_[2];
    sf::Text endGameText_;
    

    #ifdef SVGIMAGES
        float svgFactor_ = 4;
        sfc::SVGImage svgImages_[10]; // Array to hold the piece sprites
    #endif
    sf::Texture textures_[11]; // Array to hold the piece sprites

    struct board board_;
    int playerTypes_[2] = {HUMAN_PLAYER, AI_PLAYER};

    // parameters

    int loadingAnimationSpeed_ = 250; // milis of how often the animation changes
    long lastLoadingAnimationExection_ = 0; // milis of how often the animation changes
    int maxDots_ = 3; // Number of dots displayed
    int dots_ = 0; // Number of dots displayed

    sf::Color lightColor_ = sf::Color(240, 217, 181);
    sf::Color darkColor_ = sf::Color(181, 135, 99);
    sf::Color hoverColor_ = sf::Color(255, 255, 200);
    // sf::Color backgroundColor_ = sf::Color(204, 138, 77);
    sf::Color backgroundColor_ = sf::Color::Black;
    sf::Color fieldLabelColor_ = sf::Color::White;

    ChessField* hoveredField_ = NULL;
    ChessField* activeField_ = NULL;

    int x_;
    int y_;
    int width_;
    int height_;
};

#endif // CHESS_GRID_H
