#ifndef RANDOMAI_H_
#define RANDOMAI_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "board/board.h"
#include "board/boardState.h"
#include <time.h>
#include "board/validMoves.h"
#include "util.h"

void randomSamplingMove(board* b, move* bestMove, long timeLeft);
void initRandomSamplingAI();

#endif