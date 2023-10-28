#include <stdio.h>
#include "gui/gui.h"
#include "gui/ChessGrid.h"

#include <SFML/Window.hpp>

const uint screenWidth = sf::VideoMode::getDesktopMode().width;
const uint screenHeight = sf::VideoMode::getDesktopMode().height;
bool startGui = true;


void printParameters(ChessGrid& chessGrid) {
    // say pvp or pve or evp or eve
    std::cout << "Parameters: \n";
    
    if (startGui) {
        std::cout << "Mode: ";
        if (chessGrid.getPlayerType(0) == HUMAN_PLAYER) {
            std::cout << "Player";
        } else {
            std::cout << "AI";
        }
        std::cout << "(White) vs ";
        if (chessGrid.getPlayerType(1) == HUMAN_PLAYER) {
            std::cout << "Player";
        } else {
            std::cout << "AI";
        }
        std::cout << "(Black)\n";
    }
    std::cout << "Verbosity: " << chessGrid.verbosity << "\n";
    std::cout << "Time limit: " << chessGrid.aiMaxTime << "\n";
    std::cout << "Depth limit: " << chessGrid.aiMaxDepth << "\n";
    std::cout << "FEN: " << chessGrid.toFen() << "\n";
}


int main(int argc, char** argv) {
    uint size = std::min(screenWidth, screenHeight)-100;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--size") == 0) {
            size = atoi(argv[++i]);
        }
    }

    ChessGrid chessGrid(0, 0, size, size);

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
        } 
        else if (strcmp(argv[i], "--verbosity") == 0 || strcmp(argv[i], "-v") == 0) {
            chessGrid.verbosity = atoi(argv[++i]);
        } 
        else if (strcmp(argv[i], "--time-limit") == 0 || strcmp(argv[i], "-t") == 0) {
            chessGrid.aiMaxTime = atoi(argv[++i]);
        }
        // ai depth
        else if (strcmp(argv[i], "--depth") == 0 || strcmp(argv[i], "-d") == 0) {
            chessGrid.aiMaxDepth = atoi(argv[++i]);
        } // do not run gui 
        else if (strcmp(argv[i], "--no-gui") == 0) {
            startGui = false;
        } 
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: ./rkchess [options] [fen]\n";
            
            std::cout << "\n  fen: FEN string defining a chess board\n";

            std::cout << "\nOptions:\n";
            std::cout << "  -v, --verbosity: Verbosity level\n";
            std::cout << "  -t, --time-limit: Time limit for AI\n";
            std::cout << "  -d, --depth: Depth limit for AI\n";
            std::cout << "  --no-gui: Do not run GUI. Can be used to only get the best move for a given Situation\n";
            std::cout << "  --help, -h: Print this help message\n";
            std::cout << "  --size: resolution of the window in pixels\n";

            std::cout << "\nModes:\n";
            std::cout << "  -pvp: Player vs Player\n";
            std::cout << "  -pve: Player vs AI\n";
            std::cout << "  -evp: AI vs Player\n";
            std::cout << "  -eve: AI vs AI\n";

            return 0;
        }
        
        else if (strcmp(argv[i], "--size") != 0) { // fen string
            if (argv[i][0] == '-') {
                std::cout << "Unknown argument: " << argv[i] << "\n";
                return 1;
            }
            chessGrid.setBoard(argv[i]);
        }
    }
    
    if (chessGrid.verbosity > 0) printParameters(chessGrid);

    if (startGui) {
        if (chessGrid.verbosity > 0) std::cout << "Starting GUI...\n";
        int output = runGui(chessGrid);

        return output;
    } else {
        chessGrid.setPlayerType(0, AI_PLAYER);
        chessGrid.setPlayerType(1, HUMAN_PLAYER);
        if (chessGrid.verbosity > 0) std::cout << "Calculating best move...\n";
        chessGrid.startAI(true);
        if (chessGrid.verbosity > 0) std::cout << "Best move: ";
        std::cout << chessGrid.toUci(chessGrid.lastMove_.from) << chessGrid.toUci(chessGrid.lastMove_.to) << "\n";
    }

    return 0;
}