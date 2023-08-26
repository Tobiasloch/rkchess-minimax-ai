#ifndef MNIMAX_H
#define MNIMAX_H


#ifdef __cplusplus
extern "C" {
#endif

// here you can specify the minimax algorithm

// this macro says that the minimax algorithm should be iterative
#define ITERATIVEMINIMAX
// this macro says if the ai algorithm should do minimax only without iteration and alphabeta
//#define MINIMAX
// this macro says if the algorithm should random select a move from all best moves with the same score
#define BESTMOVERANDOMSELECTION

// wenn das makro an ist, dann printed die ai alle moves auf der ersten ebene mit den dazugehörigen pfaden
//#define PRINTBESTMOVEPATHS

#define MTDF
//#define NULLMOVEREDUCTION
// dieses makro enabled den neuen moveIterator
//#define USEOLDMOVEITERATOR


#include <stdio.h>
#include <stdlib.h>
#include "board/board.h"
#include "uthash/src/uthash.h"
#include "uthash/src/utlist.h"
#include "board/validMoves.h"
#include "board/boardState.h"
#include "board/moveGen/moveGen.h"
#include "boardEvaluation.h"
#include "tableBases.h"
#include "util.h"
#include <time.h>
#include <limits.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MINIMAX_FREE_ALL 0

#define MAX_MOVE_COUNT 32

#define MINIMAX_USE_ARRAY

struct moveScores{
    move m;
    int score;
#ifndef MINIMAX_USE_ARRAY
    struct moveScores* next;
#endif
}typedef moveScores;

struct boardMoveMap {
    struct moveScores* head;
    struct board board;
#ifdef MINIMAX_USE_ARRAY
    uint8_t moveCount;
#endif
    uint8_t isUpperBound;
    uint8_t depth;
    UT_hash_handle hh;
} typedef boardMoveMap;

struct minimaxReturn {
    int score;
};


extern int goDown[16];
extern int minimaxTimeCaps[16];

struct minimaxReturn alphabeta(board* b, move* bestMove, long maxDepth, struct boardMoveMap** bmm, int beta, double end);
int bestMoveMinimax(board* b, move* bestMove, int maxDepth, long maxTime);
void freeMinimaxHash(struct boardMoveMap** bmm);
/**
 * @param limit limit in kbytes
*/
void freeMinimaxMemory(long limit);
size_t minimaxMemorySize();
void minimaxMove(board* b, move* bestMove, long timeLeft);
void initMinimax();
void listIterator(moveIterator* iter);
void initListIterator(moveIterator* iter, board* b);
void quickSortMinimax(struct moveScores* moves, size_t len);

extern int (*minimaxEvaluator)(board*);
extern void (*minimaxIterator)(moveIterator*);
extern void (*movePlayerMinimax)(board*, struct move*);
extern int (*gameOverCheckMinimax)(board*, board*);
extern void (*sortMovesMinimax)(struct moveScores*, size_t);
extern int (*inCheckMinimax)(struct board*);
extern int (*inCheckPlayerMinimax)(struct board*, int);
extern int (*inCheckCharacterMinimax)(struct board*, uint64_t, int);
extern void (*initMoveIteratorMinimax)(moveIterator*, board*);

extern struct boardMoveMap* bmm;
extern int printMinimaxDebugOutput;
extern int maxDepthMinimax;
extern long INITIAL_MINIMAX_SEED;
extern int minimaxSearchRootFull;
extern int minimaxHashCollisions;

#ifdef __cplusplus
}
#endif

#endif // header

