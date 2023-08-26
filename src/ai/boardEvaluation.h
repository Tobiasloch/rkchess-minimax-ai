#ifndef BOARDEVALUATION_H_
#define BOARDEVALUATION_H_

#include <stdio.h>
#include <stdlib.h>
#include "ai/minimax.h"
#include "board/board.h"
#include "board/moveGen/moveGen.h"
#include "board/validMoves.h"

#define MAX_EVALUATION 1000

// add additional value here 
struct piece {
    int value;                  // Material Value
    int pstValue[3][64];        // Piece Square Table Value; [3] -> (0 = Early Game, 1 = Mid Game, 2 = End Game); [64] -> position on board
} typedef piece;

// holds all information of pieces 
struct evaluatePiece { 
    struct piece q;             // Queen
    struct piece k;             // King
    struct piece b;             // Bishop
    struct piece n;             // kNight
    struct piece r;             // Rook
} typedef evaluatePiece;

// global evaluatePiece - use initEvaluatePiece() to set evaluatePiece
extern evaluatePiece ep[1];

void initEvaluatePiece();
int getPstValue(uint8_t p, int8_t character, evaluatePiece* ep, int gameState);
int getPieceValue(int8_t character, evaluatePiece* ep);
int evaluateBoard(board* board);
int kingOnRank(board* b, int rank);
int kingDifference(board* b);

#endif // header