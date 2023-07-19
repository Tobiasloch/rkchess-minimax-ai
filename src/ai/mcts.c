#include "mcts.h"

#define MAX_SCORE 10000

int MCTS_LIMIT = 10;

void mctsMinimaxMove(board* b, move* bestMove, long timeLeft) {
    int (*oldEvaluator)(board*) = minimaxEvaluator;
    minimaxEvaluator = mctsEvaluation;
    bestMove->from = INVALID_MOVE_INDEX;
    bestMove->to = INVALID_MOVE_INDEX;
    if (timeLeft > 120/10) {
        bestMoveMinimax(b, bestMove, 4, INT_MAX);
    } else {
        bestMoveMinimax(b, bestMove, 3, INT_MAX);
    }

    minimaxEvaluator = oldEvaluator;
}

/**
 * limit can be either time or sample count (default:samplecount)
 * 
*/
int mctsEvaluation(board* b) {
    board* b_copy = createBoard();

    double score = 0;
    int samples = 0;
    
    for (int i = 0; i < MCTS_LIMIT; i++) {
        memcpy(b_copy, b, sizeof(board));

        moveIterator iter;
        move m;
        int moveCount = 2;
        int gameOver = isGameOver(b);
        while(!gameOver && moveCount > 1) {
            moveGen_init(&iter, b_copy);
            moveGen_next(&iter);
            moveCount = 1;
            while(iter.hasActive) { // iterate over all moves and pick a random move by using reservoir sampling
                int randValue = rand();
                int random = randValue % moveCount;

                if (random == 0) {
                    memcpy(&m, &iter.active, sizeof(struct move));
                }

                moveGen_next(&iter);
                moveCount++;
            }
            movePlayer(b_copy, &m);
            gameOver = isGameOver(b_copy);
        }

        int boardScore = 0;
        if (gameOver == WHITEWON) {
            if (b->player == WHITE_PLAYER) boardScore = 1;
            else boardScore = -1;
        } else if (gameOver == BLACKWON) {
            if (b->player == WHITE_PLAYER) boardScore = 1;
            else boardScore = -1;
        }
        score += boardScore;
        samples++;
    }

    free(b_copy);

    return ((double)score/samples)*MAX_SCORE;
}