#include "board/board.h"
#include "board/moveGen/moveGen.h"
#include "board/validMoves.h"
#include "ai/minimax.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int defaultMinimaxDepth = 9;
int bestMovesCount = 5;
int defaultMaxRounds = 2;
int globalMoveCount = 0;

struct movesList {
    struct move move;
    struct movesList* next;
    struct movesList* prev;
};

void generateTables(FILE *stream, int maxRounds, struct board* b) {
    if (maxRounds <= 0) return;
    struct move bestMove;

    char fen[FEN_BUFFER_SIZE];
    boardToFen(fen, b);

    boardMoveMap* node;
    uint64_t hashcode = b->hash^b->player;

    // simulate move
    bestMoveMinimax(b, &bestMove, defaultMinimaxDepth, LONG_MAX);
    HASH_FIND(hh, bmm, &hashcode, sizeof(uint64_t), node);
    globalMoveCount++;


    sortMovesMinimax(node->head, node->moveCount);
    int moveCount = (bestMovesCount < node->moveCount) ? bestMovesCount : node->moveCount;
    struct moveScores bestMoves[moveCount];
    memcpy(bestMoves, node->head, sizeof(moveScores) * moveCount);
    fprintf(stream, "[\"%s\", \"%s%s\", \"%d\", \"%d\"],\n", fen, indexToUci(bestMove.from), indexToUci(bestMove.to), defaultMinimaxDepth, bestMoves[0].score);


    freeMinimaxMemory((long)0); // cap at 4 gb

    struct board b_copy;
    for (int i = 0; i < moveCount; i++) {
        memcpy(&b_copy, b, sizeof(struct board));
        movePlayer(&b_copy, &bestMoves[i].m);

        generateTables(stream, maxRounds-1, &b_copy);
    }
}

int main(int argc, char** argv) {
    char* defaultFens[1];
    defaultFens[0] = INITIAL_RACING_KINGS_FEN;

    int fenLen = 1;
    char** fens;
    fens = defaultFens;

    //char* mainFile = "table.json";
    int maxRounds = defaultMaxRounds;

    if (argc > 1) {
        fens = &argv[1];
        fenLen = argc -1;
    }


    FILE* f;
    //f = fopen("table.json", "a");
    f = stdout;

    initMinimax();
    minimaxSearchRootFull = 1;
    fprintf(f, "{[\n");
    globalMoveCount = 0;
    for (int i = 0; i < fenLen; i++) {
        char* fen = fens[i];
        struct board b;
        parse(fen, &b);
        generateTables(f, maxRounds, &b);
    }
    //printf("moves generated=%d\n", globalMoveCount);
    fprintf(f, "]}\n");
    fclose(f);
}
