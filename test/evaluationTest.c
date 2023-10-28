#include "evaluationTest.h"
#include <stdio.h>

void evaluateBoardTest(CuTest* tc) {
    json* js = load_json("test/resources/evaluationBoardData.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    char msg[2048];
    msg[0] = 0;
    board* board = createBoard();
    initEvaluatePiece();
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
        int actual = evaluateBoard(board);
        board->player = ((board->player)%2)+1;
        int mirrored = evaluateBoard(board);

        boardRepresentation(br, board);
        sprintf(msg, "Evaluation Board Test Failed! expected score:%d, but was actual score:%d\n board represenation:\n%s\nfen:%s", 
        expected, actual, br, fen);

        if (mirrored != actual) {
            strcat(msg, "\nThis function should be symmetric!");
            CuFail(tc, msg);
        }

        CuAssert(tc, msg, actual==expected);

        free(fen);
        free(type);
    }

    freeJson(js);
    free(board);
}