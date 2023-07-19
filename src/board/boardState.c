#include "boardState.h"

#include "util.h"

#define GAMEWONMASK ((uint64_t) 18374686479671623680U)

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

int maxRounds = -1;

struct checkMasks* CHECK_MASKS[5];
int inCheckHashMapInited = 0;

void freeCheckHashmap() {
    inCheckHashMapInited = 0;
    for (int i = 0; i < 5; i++) {
        struct checkMasks* current;
        struct checkMasks* tmp;

        HASH_ITER(hh, CHECK_MASKS[i], current, tmp) {
            HASH_DEL(CHECK_MASKS[i],current);
            free(current);
        }
    }
}

void initCheckHashmap() {
    if (inCheckHashMapInited) return;

    board b;
    inCheckHashMapInited = 1;

    for (int i = 0; i < 5; i++) {
        CHECK_MASKS[i] = NULL;
    }

    for (int i = 0; i<64; i++) {
        for (int j = i+1; j < 64; j++) {
            uint64_t iMask = ((uint64_t)1)<<i;
            uint64_t jMask = ((uint64_t)1)<<j;

            for (int c = 0; c < 5; c++) {
                initBoard(&b);
                moveIterator iter;
                initMoveIterator(&iter, &b);
                b.board[BLACK] |= iMask;
                b.board[KING] |= iMask;

                b.board[WHITE] |= jMask;
                b.board[c] |= jMask;

                int x1 = i%8;
                int y1 = i/8;
                int x2 = j%8;
                int y2 = j/8;

                int min_x = min(x1,x2);
                int min_y = min(y1,y2);
                int max_x = max(x1,x2);
                int max_y = max(y1,y2);

                if (max_x != min_x) {
                    max_x--;
                    min_x++;
                }
                if (min_y != max_y) {
                    max_y--;
                    min_y++;
                }

                uint64_t mask = 0;
                iterateMoves(&iter);
                int inReach = 0;
                while(iter.hasActive) {
                    if (iter.active.to == i) inReach = 1;
                    int index = iter.active.to;
                    int x = index%8;
                    int y = index/8;
                    if (x <= max_x && x >= min_x && y <=max_y && y >= min_y) {
                        mask |= ((uint64_t)1)<<index;
                    }

                    iterateMoves(&iter);
                }
                if (inReach) {
                    struct checkMasks* cm = malloc(sizeof(struct checkMasks));
                    cm->id = iMask | jMask;
                    cm->mask = mask;
                    HASH_ADD(hh, CHECK_MASKS[c],id, sizeof(uint64_t), cm);
                }
            }
        }
    }
}


int inCheckFastPlayer(board* b, int player) {
    uint64_t myBoard = b->board[4+player];
    uint64_t enemyBoard = b->board[5+(player%2)];

    uint64_t kingBoard = myBoard & b->board[KING];
    struct checkMasks* node= NULL;
    //int check = 0;
    uint64_t allCharacters = (b->board[WHITE]|b->board[BLACK]);
    int index;
    uint64_t leftMask;
    uint64_t rightMask;
    uint64_t characterBoard;

    uint64_t mask = kingBoard | (b->board[QUEEN]&enemyBoard);
    HASH_FIND(hh, CHECK_MASKS[QUEEN], &mask, sizeof(uint64_t), node);

    // if the king cant be reached by the character
    // if there is another figure between the character and the king
    if (!(node == NULL || allCharacters&node->mask)) return 1;

    mask = kingBoard | (b->board[KING]&enemyBoard);
    HASH_FIND(hh, CHECK_MASKS[KING], &mask, sizeof(uint64_t), node);
    if (!(node == NULL || allCharacters&node->mask)) return 1;


    characterBoard = b->board[BISHOP]&enemyBoard;
    if (characterBoard!=0) {
        index = bitboardIndexOf(characterBoard);
        leftMask = ((uint64_t)1)<<index;
        mask = kingBoard | (leftMask); // left index
        HASH_FIND(hh, CHECK_MASKS[BISHOP], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;

        rightMask = (~leftMask)&characterBoard;
        mask = kingBoard | rightMask; // right index
        HASH_FIND(hh, CHECK_MASKS[BISHOP], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;
    }


    characterBoard = b->board[KNIGHT]&enemyBoard;
    if (characterBoard!=0) {
        index = bitboardIndexOf(characterBoard);
        leftMask = ((uint64_t)1)<<index;
        mask = kingBoard | (leftMask&enemyBoard); // left index
        HASH_FIND(hh, CHECK_MASKS[KNIGHT], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;

        rightMask = (~leftMask)&characterBoard;
        mask = kingBoard | (rightMask); // right index
        HASH_FIND(hh, CHECK_MASKS[KNIGHT], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;
    }


    characterBoard = b->board[ROOK]&enemyBoard;
    if (characterBoard!=0) {
        index = bitboardIndexOf(characterBoard);
        leftMask = ((uint64_t)1)<<index;
        mask = kingBoard | leftMask; // left index
        HASH_FIND(hh, CHECK_MASKS[ROOK], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;

        rightMask = (~leftMask)&(characterBoard);
        mask = kingBoard | rightMask; // right index
        HASH_FIND(hh, CHECK_MASKS[ROOK], &mask, sizeof(uint64_t), node);
        if (!(node == NULL || allCharacters&node->mask)) return 1;
    }

    return 0;
}

int inCheckFastCharacter(board* b, uint64_t characterMask, int index) {
    uint64_t enemyBoard = 0;

    if (characterMask & b->board[WHITE]) {
        enemyBoard = b->board[BLACK];
    } else if (characterMask & b->board[BLACK]) {
        enemyBoard = b->board[WHITE];
    } else {
        perrorf("this should not happen in function inCheckFastCharacter!");
        errorf("Info: mask: %ld, hash: %ld\n", characterMask, b->hash);
    }

    uint64_t kingBoard = enemyBoard & b->board[KING];
    uint64_t mask = kingBoard | characterMask;
    struct checkMasks* node= NULL;
    HASH_FIND(hh, CHECK_MASKS[index], &mask, sizeof(uint64_t), node);

    // if the king cant be reached by the character
    if (node == NULL) return 0;

    // if there is another figure between the character and the king
    uint64_t allCharacters = b->board[WHITE]|b->board[BLACK];
    uint64_t withoutCharacters = node->mask;
    if (allCharacters & withoutCharacters) return 0;

    return 1;
}

int inCheckFast(board* b) {
    int check = inCheckFastPlayer(b, WHITE_PLAYER);
    check = check || inCheckFastPlayer(b,BLACK_PLAYER);
    return check;
}

int inCheckFastOverCharacters(board* b) {
    for (int c = 0; c < 5; c++) {
        uint64_t characterBoard = b->board[c];
        while(characterBoard != 0) {
            uint64_t activeCharacter = ((uint64_t)1)<<bitboardIndexOf(characterBoard);

            if (inCheckFastCharacter(b, activeCharacter, c)) return 1;

            characterBoard ^= activeCharacter;
        }
    }

    return 0;
}

int inCheck(board* board) {

    int check = 0;
    int initialPlayer = board->player;
    int players[] = {WHITE_PLAYER, BLACK_PLAYER};
    int players_len = sizeof(players)/sizeof(players[0]);
    for (int i = 0; i < players_len; i++) {
        int player = players[i];
        moveIterator iter;
        board->player = player;
        initMoveIterator(&iter, board);
        iterateMoves(&iter);
        while(iter.hasActive) {
            int otherOffset = (5*(player%2));
            if (getField(board, iter.active.to) == KING+otherOffset) {
                check = 1;
                i = players_len;
                break;
            }
            iterateMoves(&iter);
        }
    }
    board->player = initialPlayer;

    return check;
}

int isGameOver(board* b) {
    if (b->halfRounds >= 50) return REMI50ROUNDS;

    uint64_t whiteKing = b->board[WHITE] & b->board[KING];
    uint64_t blackKing = b->board[BLACK] & b->board[KING];

    // check if one of the kings are missing
    if (whiteKing == 0) {
        return WHITEKINGDEAD;
    } else if (blackKing == 0) {
        return BLACKKINGDEAD;
    }

    // check if one of the kings is on the other side
    if (whiteKing & GAMEWONMASK) {
        if (blackKing & GAMEWONMASK) return DRAW;
        return WHITEWON;
    } else if (blackKing & GAMEWONMASK) {
        if (whiteKing & GAMEWONMASK) return DRAW;
        return BLACKWON;
    }

    #ifdef TEST
    // for testing -> limits the complexity
    if (maxRounds >= 0 && maxRounds <= b->rounds) {
        return DRAW;
    }
    #endif

    return 0;
}

int isGameOverExtended(board* before, board* after) {
    int gameOver = isGameOver(before);
    if (gameOver == WHITEWON) {
        if (before->player == WHITE_PLAYER) gameOver = 0;
        else if (before->player == BLACK_PLAYER) {
            gameOver = isGameOver(after);
        }
    }


    return gameOver;
}
