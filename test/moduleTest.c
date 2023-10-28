#include "moduleTest.h"

void gameOverTest(CuTest* tc) {
    json* js = load_json("test/resources/gameOverTestData.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    char msg[2048];
    msg[0] = 0;
    board* board = createBoard();
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }

        // test starts here
        char* fen = getString(&t[i], js->string);
        i++;
        char* type = getString(&t[i], js->string);
        int expected = atoi(type);

        parse(fen, board);

        int actual = isGameOver(board);

        boardRepresentation(br, board);
        sprintf(msg, "game Over Test Failed! expected:%d, but was actual:%d\n board represenation:\n%s\nfen:%s",
        expected, actual, br, fen);

        CuAssert(tc, msg, actual==expected);

        free(fen);
        free(type);
    }

    freeJson(js);
    free(board);
}

void checkTest(CuTest* tc) {
    json* js = load_json("test/resources/checkTestData.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    initCheckHashmap();

    char msg[2048];
    msg[0] = 0;
    board* board = createBoard();
    int testCount = 0;
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT  || t[i].type == JSMN_ARRAY) {
            continue;
        }

        // test starts here
        char* fen = getString(&t[i], js->string);
        i++;
        char* boolean = getString(&t[i], js->string);

        parse(fen, board);

        int actual = inCheck(board);
        int actualFastPlayer = inCheckFast(board);
        int actualFastCharacter = inCheckFastOverCharacters(board);

        boardRepresentation(br, board);
        sprintf(msg, "check Test %d Failed! expected:%s, but was actual:%s, actualFastPlayer=%s, actualFastCharacter=%s\n board represenation:\n%s\nfen:%s",
            testCount, boolean, (actual != 0) ? "True" : "False", (actualFastPlayer != 0) ? "True" : "False",(actualFastCharacter != 0) ? "True" : "False", br, fen);

        if (boolean[0] == 'T') {
            CuAssert(tc, msg, actual);
            CuAssert(tc, msg, actualFastPlayer);
            CuAssert(tc, msg, actualFastCharacter);
        } else if (boolean[0] == 'F') {
            CuAssert(tc, msg, !actual);
            CuAssert(tc, msg, !actualFastPlayer);
            CuAssert(tc, msg, !actualFastCharacter);
        } else {
            perror("tests are wrong!\n");
        }

        free(fen);
        testCount++;
        free(boolean);
    }
    freeCheckHashmap();

    freeJson(js);
    free(board);
}
