#ifndef BOARDCHECK_H_
#define BOARDCHECK_H_

#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "uthash/src/uthash.h"
#include <stdint.h>
#include "validMoves.h"

#define WHITEWON WHITE_PLAYER
#define BLACKWON BLACK_PLAYER
#define DRAW 3
#define WHITEKINGDEAD 4
#define BLACKKINGDEAD 5
#define REMI50ROUNDS 6

struct checkMasks {
    uint64_t id;
    uint64_t mask;

    UT_hash_handle hh;
};

#ifdef TEST
int maxRounds;
#endif


int inCheck(board* board);
void initCheckHashmap();
void freeCheckHashmap();
int isGameOver(board* b);
int isGameOverExtended(board* before, board* after);
int inCheckFast(board* b);
int inCheckFastOverCharacters(board* b);

int inCheckFastCharacter(board* b, uint64_t characterMask, int index);
int inCheckFastPlayer(board* b, int player);

extern struct checkMasks* CHECK_MASKS[5];

#endif
