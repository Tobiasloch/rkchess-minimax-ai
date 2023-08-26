#include "board.h"

#define GROUP_COUNT 11
#define STR_LEN 16
#define FEN_PATTERN "%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK]/\
%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK]/%8[1-8rnbqkRNBQK] %[wb] %*s %*s %s %s"

#define isLowerCase(character) (character>=97 && character <= 122)
#define toUpperCase(character) (character-32)

int ZOBRIST_INITTED = 0;
uint64_t ZobristMatrix[64][10];

board* createBoard() {
    board* b = malloc(sizeof(board));
    initBoard(b);

    return b;
}

void initMove(move* m, uint64_t from, uint64_t to) {
    m->from = from;
    m->to = to;
}

void initBoard(board* board) {
    memset(board, 0, sizeof(struct board));
    board->player = WHITE_PLAYER;
    board->rounds = 1;
    board->hash = 0;
}

int boardEquals(struct board* b1, struct board* b2) {
    int equal = b1->board[0] == b2->board[0];
    equal = equal && b1->board[1] == b2->board[1];
    equal = equal && b1->board[2] == b2->board[2];
    equal = equal && b1->board[3] == b2->board[3];
    equal = equal && b1->board[4] == b2->board[4];
    equal = equal && b1->board[5] == b2->board[5];
    equal = equal && b1->board[6] == b2->board[6];
    equal = equal && b1->player == b2->player;

    return equal;
}

board* parse(char* fen, board* board) {
    char** scanned = scan(fen);
    if (scanned == NULL) return NULL;

    memset(board->board, 0, sizeof(uint64_t)*7);

    int x = 0;
    int y = 0;
    int i = 0;
    while (x < 8) {
        char character = scanned[x][y];

        if (character == 0) {
            x++;
            y=0;

            if (i % 8 != 0) {
                perror("there can be only 8 characters in each row!");
                perror(fen);
                freeStringArray(scanned, GROUP_COUNT);
                return NULL;
            }

            continue;
        }

        uint64_t mask = ((uint64_t)9223372036854775808U)>>i; // 2^64
        switch(character) {
            case 'K':
            case 'k':
                board->board[KING] |= mask;
                i++;
            break;
            case 'q':
            case 'Q':
                board->board[QUEEN] |= mask;
                i++;
            break;
            case 'B':
            case 'b':
                board->board[BISHOP] |= mask;
                i++;
            break;
            case 'R':
            case 'r':
                board->board[ROOK] |= mask;
                i++;
            break;
            case 'n':
            case 'N':
                board->board[KNIGHT] |= mask;
                i++;
            break;
            default:
                // check for number
                if (isdigit(character)) {
                    int digit = (int)character-48;
                    i+=digit;
                } else {
                    perror("this should never happen!");
                    perror(&character);
                    freeStringArray(scanned, GROUP_COUNT);
                    return NULL;
                }
            break;
        }

        if (isalpha(character)) {
            if (isLowerCase(character)) {
                board->board[BLACK] |= mask;
            } else {
                board->board[WHITE] |= mask;
            }
        }
        y++;
    }

    if (i != 64) {
        perror("There cant be more than 64 fields on the board!");
        perror(fen);
        freeStringArray(scanned, GROUP_COUNT);
        return NULL;
    }

    board->halfRounds = atoi(scanned[9]);
    board->rounds = atoi(scanned[10]);
    board->player =  (scanned[8][0] == 'w') ? WHITE_PLAYER : BLACK_PLAYER;

    board->hash = hash(board);

    freeStringArray(scanned, GROUP_COUNT);
    return board;
}

uint64_t hash(board* board) {
    if (!ZOBRIST_INITTED) initZobrist();

    uint64_t h = 0;
    for (int i = 0; i < 64; i++) {
        int8_t character = getField(board, i);

        if (character != 10) {
            h ^= ZobristMatrix[i][character];
        }
    }
    return h;
}

void movePlayer(board* b, move* m) {

    // update halfrounds
    uint64_t toMask = (uint64_t)1 << m->to;
    if (toMask&(b->board[WHITE] | b->board[BLACK])) b->halfRounds = 0;
    else b->halfRounds++;

    // update field
    uint8_t movedCharacter = getField(b, m->from);
    removeField(b, m->from);
    setField(b, m->to, movedCharacter);

    // update player and rounds
    if (b->player == BLACK_PLAYER) {
        b->rounds++;
        b->player = WHITE_PLAYER;
    } else b->player = BLACK_PLAYER;
}

void setField(board* board, uint8_t pos, int character) {
    removeField(board, pos);

    if (character == 10) return;

    uint64_t mask = ((uint64_t)1) << pos;
    int characterIndex = character%5;
    if (characterIndex == QUEEN) {
        board->board[QUEEN] |= mask;
    } else if (characterIndex == KING) {
        board->board[KING] |= mask;
    } else if (characterIndex == BISHOP) {
        board->board[BISHOP] |= mask;
    } else if (characterIndex == KNIGHT) {
        board->board[KNIGHT] |= mask;
    } else if (characterIndex == ROOK) {
        board->board[ROOK] |= mask;
    }

    if (character <= 4) {
        board->board[WHITE] |= mask;
    } else {
        board->board[BLACK] |= mask;
    }

    board->hash ^= ZobristMatrix[pos][character]; // update hash
}


void removeField(board* board, uint8_t pos) {
    uint64_t character = getField(board, pos);

    if (character == 10) return;

    uint64_t mask = ((uint64_t)1) << pos;
    uint64_t reversed = ~mask;
    board->board[QUEEN] &= reversed;
    board->board[KING] &= reversed;
    board->board[BISHOP] &= reversed;
    board->board[KNIGHT] &= reversed;
    board->board[ROOK] &= reversed;

    board->board[WHITE] &= reversed;
    board->board[BLACK] &= reversed;
    board->hash ^= ZobristMatrix[pos][character]; // update hash
}

uint8_t getField(board* board, uint8_t pos) {
    int type = 10;

    uint64_t mask = ((uint64_t)1) << pos;
    if (mask & board->board[QUEEN]) {
        type = QUEEN;
    } else if (mask & board->board[KING]) {
        type = KING;
    } else if (mask & board->board[BISHOP]) {
        type = BISHOP;
    } else if (mask & board->board[KNIGHT]) {
        type = KNIGHT;
    } else if (mask & board->board[ROOK]) {
        type = ROOK;
    }

    if (type != 10 && mask & board->board[BLACK]) {
        type += 5;
    }

    return type;
}

void freeStringArray(char** array, int len) {
    for (int i = 0; i < len; i++) {
        free(array[i]);
    }
    free(array);
}

char** scan(char* fen) {
    char** board = malloc(sizeof(char*) * GROUP_COUNT);
    for (int i = 0; i < GROUP_COUNT; i++) {
        board[i] = calloc(STR_LEN, sizeof(char*));
    }

    int count = sscanf(fen, FEN_PATTERN, board[0], board[1], board[2], board[3], board[4],
    board[5], board[6], board[7], board[8], board[9], board[10]);

    if (count < GROUP_COUNT) {
        freeStringArray(board, GROUP_COUNT);
        perror("Syntax error in fen:");
        perror(fen);
        return NULL;
    }

    return board;
}

void boardToMatrix(char matrix[8][9], board* board) {

    for (int i = 0; i < 64; i++) {
        uint64_t mask = ((uint64_t)9223372036854775808U)>>i;
        int x = i%8;
        int y = i/8;

        if (mask & board->board[QUEEN]) {
            if (mask & board->board[WHITE] & board->board[QUEEN]) {
                matrix[y][x] = 'Q';
            } else {
                matrix[y][x] = 'q';
            }
        } else if (mask & board->board[KING]) {
            if (mask & board->board[WHITE] & board->board[KING]) {
                matrix[y][x] = 'K';
            } else {
                matrix[y][x] = 'k';
            }
        } else if (mask & board->board[KNIGHT]) {
            if (mask & board->board[WHITE] & board->board[KNIGHT]) {
                matrix[y][x] = 'N';
            } else {
                matrix[y][x] = 'n';
            }
        } else if (mask & board->board[ROOK]) {
            if (mask & board->board[WHITE] & board->board[ROOK]) {
                matrix[y][x] = 'R';
            } else {
                matrix[y][x] = 'r';
            }
        } else if (mask & board->board[BISHOP]) {
            if (mask & board->board[WHITE] & board->board[BISHOP]) {
                matrix[y][x] = 'B';
            } else {
                matrix[y][x] = 'b';
            }
        } else {
            matrix[y][x] = '.';
        }
    }
}

void characterToChessPiece(char buffer[], char character) {
    switch (character) {
        case 'K':
        sprintf(buffer, "\u265A");
        break;

        case 'k':
        sprintf(buffer, "\u2654");
        break;

        case 'Q':
        sprintf(buffer, "\u265B");
        break;

        case 'q':
        sprintf(buffer, "\u2655");
        break;

        case 'R':
        sprintf(buffer, "\u265C");
        break;

        case 'r':
        sprintf(buffer, "\u2656");
        break;

        case 'B':
        sprintf(buffer, "\u265D");
        break;

        case 'b':
        sprintf(buffer, "\u2657");
        break;

        case 'N':
        sprintf(buffer, "\u265E");
        break;

        case 'n':
        sprintf(buffer, "\u2658");
        break;
        default:
        sprintf(buffer, " ");
    }
}

char* boardRepresentation(char* string, board* board) {
    string[0] = 0;
    strcat(string, "      a   b   c   d   e   f   g   h\n");
	strcat(string, "    ╔═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╗\n");

    char matrix[8][9];
    boardToMatrix(matrix, board);
    char number[11];
    number[10] = 0;
    char character[2];
    character[1] = 0;
    for (int i = 0; i < 8; i++) {
        strcat(string, "  ");
        sprintf(number, "%d", 8-i);
        strcat(string, number);
        strcat(string," ║");

        for (int j = 0; j  < 8; j++) {
            strcat(string, " ");
            characterToChessPiece(character, matrix[i][j]);
            //sprintf(character, "%c", matrix[i][j]);
            if (character[0] == '.') character[0] = ' ';
            strcat(string, character);
            if (j < 7) strcat(string," │");
        }
        strcat(string, " ║ ");
        sprintf(number, "%d", 8-i);
        strcat(string, number);
        strcat(string, "\n");
        if (i<7) strcat(string, "    ╟───┼───┼───┼───┼───┼───┼───┼───╢\n");
    }

    strcat(string,"    ╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝\n");
	strcat(string,"      a   b   c   d   e   f   g   h\n");

    strcat(string, "player=");
    strcat(string, (board->player == WHITE_PLAYER) ? "w" : "b");
    strcat(string, ", halfrounds=");
    sprintf(number, "%d", board->halfRounds);
    strcat(string, number);
    strcat(string, ", rounds=");
    sprintf(number, "%d", board->rounds);
    strcat(string, number);
    strcat(string, "\nfen=");
    int len = strlen(string);
    boardToFen(&string[len], board);

    //printf("len=%ld\n", strlen(string));

    return string;
}

char* boardToFen(char* string, board* board) {
    char matrix[8][9];
    string[0] = 0;
    boardToMatrix(matrix, board);

    char number[12];
    int count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char c = matrix[i][j];

            if (c == '.') {
                count+=1;
            } else {
                if (count != 0) {
                    sprintf(number, "%d", count);
                    strcat(string, number);
                    count = 0;
                }

                number[0] = c;
                number[1] = 0;
                strcat(string, number);
            }
        }

        if (count != 0) {
            sprintf(number, "%d", count);
            strcat(string, number);
            count = 0;
        }
        if (i+1 < 8) strcat(string,"/");
    }

    strcat(string, (board->player == WHITE_PLAYER) ? " w" : " b");
    strcat(string, " - -");
    sprintf(number, " %d", board->halfRounds);
    strcat(string, number);
    sprintf(number, " %d", board->rounds);
    strcat(string, number);

    return string;
}

void positionToUCI(char* buffer, int position) {
    strcpy(buffer, indexToUci(position));
}

uint64_t rand_uint64_slow(void) {
  uint64_t r = 0;
  for (int i=0; i<64; i++) {
    r = r*2 + rand()%2;
  }
  return r;
}

void initZobrist() {
    if (ZOBRIST_INITTED) return;

    ZOBRIST_INITTED = 1;
    for (int i = 0;i < 64; i++) {
        for (int j = 0; j < 10; j++) {
            ZobristMatrix[i][j] = rand_uint64_slow();
        }
    }
}
