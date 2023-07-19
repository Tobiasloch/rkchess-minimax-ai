#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "board/board.h"
#include "board/boardState.h"
#include "minimax.h"
#include "randomAI.h"

int mctsEvaluation(board* b);
void mctsMinimaxMove(board* b, move* m, long timeLeft);

extern int MCTS_LIMIT;