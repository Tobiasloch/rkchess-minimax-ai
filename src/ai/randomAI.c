#include "randomAI.h"

//#define PRINT_MOVES_RANDOM_AI

int AIInitted = 0;

void initRandomSamplingAI() {
    if (AIInitted) return;
    AIInitted = 1;

    srand(time(NULL));
}

void randomSamplingMove(board* b, move* bestMove, long timeLeft) {
    initRandomSamplingAI();
    void (*iterator)(moveIterator*) = iterateMoves;
    moveIterator iter;
    board* b_copy = createBoard();

    initMoveIterator(&iter, b);
    iterator(&iter);
    int moveCount = 1;
    while(iter.hasActive) { // iterate over all moves and pick a random move by using reservoir sampling
        #ifdef PRINT_MOVES_RANDOM_AI
        if (moveCount > 1) printf(", ");
        #endif
        memcpy(b_copy, b, sizeof(struct board));
        int randValue = rand();
        int random = randValue % moveCount;
        #ifdef PRINT_MOVES_RANDOM_AI
        printf("\"%s%s\"", indexToUci(iter.active.from.index), indexToUci(iter.active.to.index));
        #endif


        // test if king is in chess
        movePlayer(b_copy, &iter.active);
        if (inCheck(b_copy)) {
            iterator(&iter);
            continue;
        }

        if (random == 0) {
            memcpy(bestMove, &iter.active, sizeof(struct move));
        }

        iterator(&iter);
        moveCount++;
    }
    #ifdef PRINT_MOVES_RANDOM_AI
    //printf("\n");
    #endif
    free(b_copy);
}
