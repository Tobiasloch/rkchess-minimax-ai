#include "boardTest.h"

void TestParser(CuTest* tc) {
    json* js = load_json("test/resources/FenParserTestData.json", 50);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* b = createBoard();
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        char* fen = getString(&t[i], js->string);
        //printf("testing fen:%s\n", fen);

        parse(fen, b);

        if (b == NULL) CuFail(tc, "Board returned NULL!");

        i++;
        int start = i;
        char msg[2048];
        msg[0] = 0;
        boardRepresentation(br, b);
        for (;i < t_len && t[i].type == JSMN_STRING;i++) { // board
            sprintf(msg, "the values did not match! board:%s fen:%s\n", br, fen);
            
            char* str = getString(&t[i], js->string);
            uint64_t expected = (uint64_t)atoll(str);
            CuAssert(tc, msg, b->board[i-start] == expected);
            free(str);
        }
        free(fen);
    }

    freeJson(js);
    free(b);
}

void boardToFenTest(CuTest* tc) {
    json* js = load_json("test/resources/checkTestData.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* b = createBoard();
    char msg[512];
    char fenActual[FEN_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        char* fen = getString(&t[i], js->string);
        i++;

        parse(fen, b);

        boardToFen(fenActual, b);

        msg[0] = 0;
        sprintf(msg, "board To fen Test failed!\nexpected:%s\nactual:%s\n", fen, fenActual);

        CuAssert(tc, msg, strcmp(fenActual, fen) == 0);
        free(fen);
    }

    freeJson(js);
    free(b);
}

void setFieldTest(CuTest* tc) {
    json* js = load_json("test/resources/setFieldTest.json", 150);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* b = createBoard();
    initZobrist();
    char msg[2048];
    int testCount = 0;
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        char* uci = getString(&t[i], js->string);
        i++;
        char* character = getString(&t[i], js->string);
        int c = atoi(character);
        i++;
        char* bitboard = getString(&t[i], js->string);
        uint64_t bb = atoll(bitboard);
        
        initBoard(b);
        int hashExpected = b->hash;

        uint8_t posIndex = uciToIndex(uci);
        setField(b, posIndex, c);
        boardRepresentation(br, b);

        msg[0] = 0;
        sprintf(msg, "setField test failed!\nexpected:%s\nactual:%ld\nboard:\n%s\nmove:%s", bitboard, b->board[c%5], br, uci);

        CuAssert(tc, msg, b->board[c%5] == bb);

        removeField(b, posIndex);
        sprintf(msg, "hash value is not as expected in Test %d!\nexpected:%d, but was actual:%ld\nboard:\n%s\n",testCount, hashExpected, b->hash, br);
        CuAssert(tc, msg, b->hash == hashExpected);
        CuAssert(tc, "removeField function does not work!", b->board[c%5]==0);

        free(uci);
        free(bitboard);
        free(character);
        testCount++;
    }

    freeJson(js);
    free(b);
}

void getFieldTest(CuTest* tc) {
    json* js = load_json("test/resources/getFieldTest.json", 150);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* b = createBoard();
    initZobrist();
    char msg[2048];
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        char* fen = getString(&t[i], js->string);
        i++;
        char* uci = getString(&t[i], js->string);
        i++;
        char* character = getString(&t[i], js->string);
        int c = atoi(character);
        
        parse(fen, b);

        uint8_t posIndex = uciToIndex(uci);
        uint8_t figure = getField(b, posIndex);
        boardRepresentation(br, b);
        
        msg[0] = 0;
        sprintf(msg, "getField test failed!\nexpected:%d\nactual:%d\nboard:\n%s\nmove:%s", c, figure, br, uci);

        CuAssert(tc, msg, c == figure);
        free(uci);
        free(fen);
        free(character);
    }

    freeJson(js);
    free(b);
}

void movePlayerTest(CuTest* tc) {
    json* js = load_json("test/resources/sampleGame.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    struct board* board = NULL;

    move* m = calloc(1, sizeof(struct move));
    initZobrist();
    int testCount = 0;
    char msg[4086];
    
    char beforeMove[BOARD_REPRESENTATION_BUFFER_SIZE];
    char afterMove[BOARD_REPRESENTATION_BUFFER_SIZE];
    char fenGen[FEN_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        char* fen = getString(&t[i], js->string);
        i++;
        char* from = getString(&t[i], js->string);
        i++;
        char* to = getString(&t[i], js->string);
        i+=2;

        if (board == NULL) {
            board = createBoard();
            parse(fen, board);

            m->from = uciToIndex(from);
            m->to = uciToIndex(to);

            free(fen);
            free(from);
            free(to);
            continue;
        }

        boardRepresentation(beforeMove, board);

        movePlayer(board, m);

        boardRepresentation(afterMove, board);

        msg[0] = 0;

        boardToFen(fenGen, board);
        sprintf(msg, "movePlayer test %d failed!\nbefore Move:\n%s\nafterMove:\n%s\nmove:%s %s\nfen expected=%s\nbut actual:%s\n", testCount,
        beforeMove, afterMove, indexToUci(m->from), indexToUci(m->to), fen, fenGen);

        CuAssert(tc, msg, strcmp(fenGen, fen) == 0);

        m->from = uciToIndex(from);
        m->to = uciToIndex(to);
        free(from);
        free(to);
        free(fen);
        testCount++;
    }

    freeJson(js);
    free(board);
    free(m);
}