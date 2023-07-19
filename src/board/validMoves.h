#ifndef VALIDMOVES_H_
#define VALIDMOVES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "board.h"
#include "boardState.h"
#include "moveIterator.h"

#ifdef BENCHMARK
long long movesCount;
#endif

#define moveIsInvalid(move) (move.from > 63 || move.to > 63)



//#define MG_OLD 1
//#define MG_NEW 2

//#define MOVEGEN MG_NEW

//#if MOVEGEN == MG_OLD

//#pragma message "OLD MoveGen selected"

#define bitboardIndexOf(b) ((sizeof(unsigned long long)-(8))+63-__builtin_clzll(b))

void initMoveIterator(moveIterator* iterator, board* board);
void iterateMoves(moveIterator* iterator);
int kingCanMoveForward(board* board, int player);
/*
#elif MOVEGEN == MG_NEW

// #pragma message "NEW MoveGen selected"

#include "moveGen/bitboard.h"
#include "moveGen/moveGen.h"

// Rename for callers
#define bitboardIndexOf findHighest1
#define initMoveIterator moveGen_init
#define iterateMoves moveGen_next

#endif
*/


#endif
