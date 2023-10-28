#include "validMovesTest.h"

#define hashPositions(p1,p2) (p1*64+p2)
#define unHashPositions(h) ((int[2]) {h/64, h%64})

struct moveHash {
    int id; // mix of both positions
    int generated; // boolean if the value was already generated

    UT_hash_handle hh;
};

struct moveHash* mh = NULL;

void validMovesTest(CuTest* tc, void (*initMoveIteratorTest)(moveIterator*,board*), void (*iterateMovesTest)(moveIterator*), int ignoreInCheck, char* testData) {
    json* js = load_json(testData, 48000);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* b = createBoard();
    initMinimax();
    freeMinimaxMemory(0); // disable initial values in transposition table

    moveIterator m;
    char msg[4086];
    int testCount = 0;
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        //printf("[TEST] TESTCASE #%d\n", testCount);

        // test starts here
        char* fen = getString(&t[i], js->string);
        //printf("fen=%s\n", fen);
        i++;

        parse(fen, b);
        initMoveIteratorTest(&m, b);
        if (inCheck(b)) printf("Test board is in check!\nfen=%s\n", fen);
        //sprintf(msg, "The Test %d has a board that is already in check!\n", testCount);
        //CuAssert(tc, msg, !inCheckFast(b));
        struct board* b_copy = createBoard();
        boardRepresentation(br, b);

        int movesCount = 0;
        while(i < t_len && t[i].type == JSMN_STRING) {
            memcpy(b_copy, b, sizeof(struct board));
            char* move = getString(&t[i], js->string);

            int from = uciToIndex(move);
            int to = uciToIndex((&move[2]));
            // TODO: fix bug that in check check is not working
            struct move mv;
            mv.from = from;
            mv.to = to;
            movePlayer(b_copy, &mv);
            if (ignoreInCheck || !inCheckFast(b_copy)) {
                struct moveHash* node = malloc(sizeof(struct moveHash));
                node->generated = 0;
                node->id = hashPositions(from, to);
                HASH_ADD_INT(mh, id, node);
                movesCount++;
            }
            free(move);
            i++;
        }
        free(b_copy);

        iterateMovesTest(&m);
        int movesFound = 0;
        for (int j = 0; m.hasActive; j++){
            int id = hashPositions(m.active.from, m.active.to);

            struct moveHash* node = NULL;
            HASH_FIND_INT(mh, &id, node);
            if (node != NULL) {
                if (node->generated) {
                    sprintf(msg, "Your moveIterator returned in Test %d a move twice!\nmoveIds:%s %s\nboard:\n%s\n", testCount,  indexToUci(m.active.from), indexToUci(m.active.to), br);
                    free(fen);
                    free(b);
                    freeJson(js);
                    free(node);
                    CuFail(tc, msg);
                }

                node->generated = 1;
                movesFound++;
            } else {
                sprintf(msg, "Your moveIterator returned in Test %d a move that is not viable!\nmoveIds:%s %s\nboard:\n%s\nfen=%s\n", testCount,  indexToUci(m.active.from), indexToUci(m.active.to), br, fen);
                free(fen);
                free(b);
                freeJson(js);
                CuFail(tc, msg);
            }

            // kingCanMoveForward Test
            /*if (m.active.to.index/8 > m.active.from.index/8 && getField(board, &m.active.from)%5 == KING) {
                sprintf(msg, "kingCanMoveForward test failed! expected=1, but was=0\nmoveIds:%s %s\nboard:\n%s\n", indexToUci(m.active.from.index), indexToUci(m.active.to.index), br);
                CuAssert(tc, msg, kingCanMoveForward(board, (int)board->player));
            }*/

            iterateMovesTest(&m);
        }
        int count = movesCount-movesFound;

        if (count != 0) {
            sprintf(msg, "%d of %d moves were missing by the moveIterator in Test %d!\nboard:\n%s\nmissing moves: ", count, movesCount, testCount, br);
            char numbers[16];

            struct moveHash* tmp;
            struct moveHash* current;
            HASH_ITER(hh, mh, current, tmp) {
                if (current->generated) continue;
                int unhashed[2];
                unhashed[0] = current->id/64;
                unhashed[1] = current->id%64;

                sprintf(numbers, "(%s %s) ", indexToUci(unhashed[0]), indexToUci(unhashed[1]));
                strcat(msg, numbers);

                HASH_DEL(mh,current);
                free(current);
            }

            free(b);
            free(fen);
            freeJson(js);

            CuFail(tc, msg);
        }

        // free hash
        struct moveHash* tmp;
        struct moveHash* current;
         HASH_ITER(hh, mh, current, tmp) {
            HASH_DEL(mh,current);
            free(current);
        }

        testCount++;
        free(fen);
    }

    freeJson(js);
    free(b);
    freeCheckHashmap();
}

struct moveScores movesBufferMovesTest[128];
void initListIteratorTest(moveIterator* iter, board* b) {
    // this will save the generated moves in the hashmap
    bestMoveMinimax(b, NULL, 1, LONG_MAX);
    
    struct boardMoveMap* node = NULL;
    iter->preGeneratedMovesList = movesBufferMovesTest;

    uint64_t hashcode = b->hash^b->player;
    HASH_FIND(hh, bmm, &hashcode, sizeof(uint64_t), node);
    if (node != NULL) {
        iter->bmm = node;
        initListIterator(iter, b);
    } else {
        perror("node could not be fond in initListIterator\n");
    }
}

void listIteratorTest(CuTest* tc) {
    validMovesTest(tc, initListIteratorTest, listIterator, 0, "test/resources/validMovesTestData.json");
    freeMinimaxMemory(0);
}

void validMovesTestOld(CuTest* tc) {
    validMovesTest(tc, initMoveIterator, iterateMoves, 1, "test/resources/validMovesTestData.json");
}

void validMovesTestNew(CuTest* tc) {
    validMovesTest(tc, moveGen_init, moveGen_next, 0, "test/resources/validMovesTestData.json");
}
