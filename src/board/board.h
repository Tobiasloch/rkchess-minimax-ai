#ifndef BOARD_H_
#define BOARD_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "util.h"

typedef enum {
    NONE = -1, // For posFigTypeCache
    QUEEN = 0,
    KING = 1,
    BISHOP = 2,
    KNIGHT = 3,
    ROOK = 4,
    WHITE = 5,
    BLACK = 6
} boardContent_t;

typedef enum {
    WHITE_PLAYER = 1,
    BLACK_PLAYER = 2
} boardPlayer_t;

#define INITIAL_RACING_KINGS_FEN "8/8/8/8/8/8/krbnNBRK/qrbnNBRQ w - - 0 1"
#define BOARD_REPRESENTATION_BUFFER_SIZE (1550+FEN_BUFFER_SIZE+5)
#define FEN_BUFFER_SIZE 128

#define charToIndex(character) ((character > ) )

#define uci_0(uci) ((uci)[0]-97)
#define uci_1(uci) ((uci)[1]-48)
#define uciToIndex(uci) (uint64_t)((7-uci_0(uci)) + (8*(uci_1(uci) - 1)))
#define indexToUci(index) ((index < 64 && index >= 0) ? (char[3]){7-(index%8)+97, 1+(index/8)+48, 0} : (char[3]){'I','V'})

#define characterToIndex(c) ((c=='Q') ? 0 :\
                            (c=='K') ? 1 : \
                            (c=='B') ? 2 : \
                            (c=='N') ? 3 : \
                            (c=='R') ? 4 : \
                            (c=='q') ? 5 : \
                            (c=='k') ? 6 : \
                            (c=='b') ? 7 : \
                            (c=='n') ? 8 : \
                            (c=='r') ? 9 : \
                            (c=='.') ? 10 : NULL)

#define indexToCharacter(c) ((c==0) ? 'Q' :\
                            (c==1) ? 'K' : \
                            (c==2) ? 'B' : \
                            (c==3) ? 'N' : \
                            (c==4) ? 'R' : \
                            (c==5) ? 'q' : \
                            (c==6) ? 'k' : \
                            (c==7) ? 'b' : \
                            (c==8) ? 'n' : \
                            (c==9) ? 'r' : \
                            (c==10) ? '.' : NULL)

struct board { // pawns, rochade and enpessant ignored
    uint64_t board[7];
    boardPlayer_t player;
    uint8_t halfRounds; // after 50 half rounds the game ends
    uint16_t rounds;
    uint64_t hash;
} typedef board;

#define INVALID_MOVE_INDEX 64

// Not used anymore
/*struct position {
    uint8_t index; // index is a 1 dimensional index on the bitboard
    int8_t character; // has to be -1 initially
} typedef position;*/

struct move {
    uint8_t from;
    uint8_t to;
} typedef move;

static inline int moveToUci(move* m, char* uci) {

    uint64_t f = m->from;

    if (f >= 64) {
        errorln("Move 'from' field out of range");
        return -1;
    }

    uci[0] = 7 - (f % 8)+97;
    uci[1] = 1 + (f / 8)+48;

    uint64_t t = m->to;

    if (t >= 64) {
        errorln("Move 'to' field out of range");
        return -1;
    }

    uci[2] = 7 - (t % 8)+97;
    uci[3] = 1 + (t / 8)+48;

    return 0;

}

board* parse(char* fen, board* board);
void freeStringArray(char** array, int len);
char** scan(char* fen);
char* boardRepresentation(char* string, board* board);
void boardToMatrix(char matrix[8][9], board* board);
board* createBoard();
void initBoard(board* board);
uint64_t hash(board* board);
void initZobrist();
uint8_t getField(board* board, uint8_t pos);
void setField(board* board, uint8_t pos, int character);
void removeField(board* board, uint8_t pos);
void movePlayer(board* b, move* m);
char* boardToFen(char* string, board* board);
void initMove(move* m, uint64_t from, uint64_t to);
int boardEquals(struct board* b1, struct board* b2);

#endif
