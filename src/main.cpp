#include <stdio.h>
#include "gui/gui.h"
#include "gui/ChessGrid.h"

#include <SFML/Window.hpp>

const uint screenWidth = sf::VideoMode::getDesktopMode().width;
const uint screenHeight = sf::VideoMode::getDesktopMode().height;

int main(int argc, char** argv) {
    uint size = std::min(screenWidth, screenHeight)-100;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--size") == 0) {
            size = atoi(argv[++i]);
        }
    }

    ChessGrid chessGrid(0, 0, size, size);

    std::cout << size;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-pvp") == 0) {
            chessGrid.setPlayerType(0, HUMAN_PLAYER);
            chessGrid.setPlayerType(1, HUMAN_PLAYER);
        } else if (strcmp(argv[i], "-pve") == 0) {
            chessGrid.setPlayerType(0, HUMAN_PLAYER);
            chessGrid.setPlayerType(1, AI_PLAYER);
        } else if (strcmp(argv[i], "-evp") == 0) {
            chessGrid.setPlayerType(0, AI_PLAYER);
            chessGrid.setPlayerType(1, HUMAN_PLAYER);
        } else if (strcmp(argv[i], "-eve") == 0) {
            chessGrid.setPlayerType(0, AI_PLAYER);
            chessGrid.setPlayerType(1, AI_PLAYER);
        } else if (strcmp(argv[i], "--verbosity") == 0 || strcmp(argv[i], "-v") == 0) {
            chessGrid.verbosity = atoi(argv[++i]);
        }
        
        else { // fen string
            chessGrid.setBoard(argv[i]);
        }
    }

    // std::cout << "Player 1: " << chessGrid.getPlayerType(0) << "\n";
    // std::cout << "Player 2: " << chessGrid.getPlayerType(1) << "\n";

    int output = runGui(chessGrid);

    return output;
}