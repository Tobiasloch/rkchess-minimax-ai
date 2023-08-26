#include <stdio.h>
#include "gui/gui.h"
#include "gui/ChessGrid.h"

const int tileSize = 80;
const int gridSize = 8;

int main(int argc, char** argv) {
    uint size = tileSize * gridSize;
    ChessGrid chessGrid(0, 0, size, size);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pvp") == 0) {
            chessGrid.playerTypes_[0] = HUMAN_PLAYER;
            chessGrid.playerTypes_[1] = HUMAN_PLAYER;
        } else if (strcmp(argv[i], "--pve") == 0) {
            chessGrid.playerTypes_[0] = HUMAN_PLAYER;
            chessGrid.playerTypes_[1] = AI_PLAYER;
        } else if (strcmp(argv[i], "--evp") == 0) {
            chessGrid.playerTypes_[0] = AI_PLAYER;
            chessGrid.playerTypes_[1] = HUMAN_PLAYER;
        } else if (strcmp(argv[i], "--eve") == 0) {
            chessGrid.playerTypes_[0] = AI_PLAYER;
            chessGrid.playerTypes_[1] = AI_PLAYER;

        } else if (strcmp(argv[i], "--verbosity") == 0) {
            chessGrid.verbosity = atoi(argv[++i]);
        }
        
        else { // fen string
            chessGrid.setBoard(argv[i]);
        }
    }

    int output = runGui(chessGrid);

    return output;
}