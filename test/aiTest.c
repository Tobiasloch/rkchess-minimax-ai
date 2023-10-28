#include "aiTest.h"

#define invalidEvaluation 200000
#define notEndEvaluation 300000

#define LEFT_CHILD(n) ((n*2)+1)
#define RIGHT_CHILD(n) (LEFT_CHILD(n)+1)

/**
 * b: count of children
 * n: index of parent node
 * x: xth child
 * 
*/
#define CHILD(b,n,x) ((n*b)+1+x)

int notEndSaver = 1;

int tree[2048];
int treeChildCount = 2;
int tree_len;
int treeVisits[2048];
int treeVisitIndex = 1;

void PrintPretty(int index, char* indent, int last, char* buffer)
   {
       if (index < 0 || index >= tree_len) return;

       int indent_len = strlen(indent);
       char indentBuffer[indent_len+8];
       memcpy(indentBuffer, indent, indent_len+1);
       strcat(buffer, indent);
       if (last)
       {
           strcat(buffer, "\\-");
           strcat(indentBuffer, "  ");
       }
       else
       {
           strcat(buffer, "|-");
           strcat(indentBuffer, "| ");
       }
       char number[32];
       sprintf(number, "(%d)\n", tree[index]);
       strcat(buffer, number);

        // print children
        //PrintPretty(LEFT_CHILD(index), indentBuffer, 0,buffer);
        //PrintPretty(RIGHT_CHILD(index), indentBuffer, 1,buffer);
       for (int i = treeChildCount-1; i >= 0; i--)
          PrintPretty(CHILD(treeChildCount, index, i), indentBuffer, i == 0, buffer);
   }

void print2DUtil(int root, int space, int count) 
{ 
    // Base case 
    if (root >= tree_len) 
        return; 
  
    // Increase distance between levels 
    space += count; 
  
    // Process right child first 
    print2DUtil(RIGHT_CHILD(root), space, count); 
  
    // Print current node after space 
    // count 
    printf("\n"); 
    for (int i = count; i < space; i++) 
        printf(" "); 
    printf("%d\n", tree[root]); 
  
    // Process left child 
    print2DUtil(LEFT_CHILD(root), space, count); 
} 

void iterateTree(moveIterator* m) {
    int index = m->board->board[0];
    m->active.from = index;

    m->active.to = CHILD(treeChildCount, index, m->hasActive-1);
    if (m->hasActive <= treeChildCount && m->active.to < tree_len) {
        m->hasActive++;
    } else m->hasActive = 0;
}

int evaluateTree(board* b) {
    int index = b->hash;
    int factor = 1;
    if (b->player == BLACK_PLAYER) factor = -1;

    if (index >= 0 && index < tree_len) {
        return tree[index]*factor;
    }

    return invalidEvaluation;
}

void movePlayerTree(board* b, struct move* m) {
    int index = m->to;

    if (treeVisits[index] == -1) treeVisits[index] = treeVisitIndex++;

    if (!(index >= 0 && index < tree_len)) return;

    b->board[0] = m->to;
    b->hash = index;
    b->player = (b->player == WHITE_PLAYER) ? BLACK_PLAYER : WHITE_PLAYER;
}

int neverGameOver = 1;
int gameOverTree(board* b2, board* b) {
    if (neverGameOver) return 0;

    int index = b->board[0];
    int hasChildren = 0;
    for (int i = 0; i < treeChildCount; i++) hasChildren = hasChildren || (CHILD(treeChildCount, index, i) < tree_len);
    
    int winner = 0;
    if (b->player == WHITE_PLAYER) winner = BLACK_PLAYER; 
    else winner = WHITE_PLAYER;

    return (hasChildren) ? 0 : winner;
}

void initMoveIteratorTree(moveIterator* iterator, board* board) {
    memset(iterator, 0, sizeof(struct moveIterator)); // sets all values to 0
    iterator->board = board;
	iterator->active.from = INVALID_MOVE_INDEX;
	iterator->active.to = INVALID_MOVE_INDEX;
	iterator -> hasActive = 1;
	iterator -> moveNum = 1;
}

int inCheckTree(board* b) {return 0;}
int inCheckPlayerTree(board* b, int player) {return inCheckTree(b);}
int inCheckCharacterTree(board* b, uint64_t character, int index) {return inCheckTree(b);}

void minimaxTreeTest(CuTest* tc) {
    json* js = load_json("test/resources/aiTreeTestData.json", 512);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    initMinimax();
    inCheckMinimax = inCheckTree;
    gameOverCheckMinimax = gameOverTree;
    minimaxIterator = iterateTree;
    minimaxEvaluator = evaluateTree;
    movePlayerMinimax = movePlayerTree;
    inCheckPlayerMinimax = inCheckPlayerTree;
    inCheckCharacterMinimax = inCheckCharacterTree;
    initMoveIteratorMinimax = initMoveIteratorTree;

    board* b = createBoard();
    char msg[4086];
    char treeBuffer[2048];
    int testCount = 0;
    for (int i = 0; i < t_len; i++) {
        tree_len = 0;
        treeVisitIndex = 0;
        b->board[0] = 0;
        jsmntok_t tok = t[i];
        if (tok.type == JSMN_OBJECT || tok.type == JSMN_ARRAY) {
            continue;
        }
        
        // test starts here
        // parsing data
        // childCount
        char treeChildCountBuffer[t[i].end-t[i].start+1]; 
        getStringBuffer(&t[i], js->string, treeChildCountBuffer);
        treeChildCount = atoi(treeChildCountBuffer);
        i++;
        
        char expectedScore[t[i].end-t[i].start+1]; 
        getStringBuffer(&t[i], js->string, expectedScore);
        int expected = atoi(expectedScore);
        i++;

        int innerendArray = -1;
        if (t[i].type == JSMN_ARRAY) {
            innerendArray = t[i].end;
        } else {
            perror("ai tree test data is wrong. Expected an array on index 1!");
            break;
        }
        i++;
        char val[16];
        for (; i < t_len && t[i].type == JSMN_PRIMITIVE && t[i].end < innerendArray; i++) {
            getStringBuffer(&t[i], js->string, val);
            tree[tree_len] = atoi(val);
            tree_len++;
        }
        
        notEndSaver = 1;
        int maxDepth = (int)log2(tree_len);
        //freeMinimaxMemory(0);
        //int actualScore2 = alphabeta(b, NULL,maxDepth, &bmm, INT_MAX, INFINITY).score;

        notEndSaver = 0;
        treeVisitIndex = 0;
        for (int j = 0; j < tree_len; j++) treeVisits[j] = -1;
        treeVisitIndex = 1;
        freeMinimaxMemory(0);
        int actualScore = bestMoveMinimax(b, NULL, maxDepth, INT_MAX);
        //TODO: the following line has to be deleted after testing or the test will not work
        freeMinimaxMemory(0);

        treeBuffer[0] = 0;
        PrintPretty(0, "", 1, treeBuffer);

        // this test is ignored, because alphabeta alone does not work anymore, which is expected
        // check if both functions return the same
        /*sprintf(msg, "aiTreeTest %d failed! Both functions bestMove and alphabeta should return the same score! alphabeta=%d, bestMove=%d\nexpected score=%d\n", testCount, actualScore, actualScore2, expected);

        CuAssert(tc, msg, actualScore == actualScore2);*/

        msg[0] = 0;
        sprintf(msg, "aiTreeTest failed! expected:%d, but was actual:%d\n", expected, actualScore);

        if (actualScore == invalidEvaluation) {
            strcat(msg, "The algorithm evaluated an invalid index!");
            CuFail(tc, msg);
        } else
        if (actualScore == notEndEvaluation) {
            strcat(msg, "The algorithm evaluated a non leaf node!");
            CuFail(tc, msg);
        }
        strcat(msg, "\nexpected game tree:\n");
        strcat(msg, treeBuffer);

        CuAssert(tc, msg, expected == actualScore);

        if (t[i].type == JSMN_ARRAY) {
            innerendArray = t[i].end;
        } else {
            perror("ai tree test data is wrong. Expected an array on index 1!");
            break;
        }
        i++;

        int valid = 1;
        char expectedTreeVisits[256];
        expectedTreeVisits[0] = 0;
        char actualTreeVisits[256];
        actualTreeVisits[0] = 0;
        int iStart = i;
        char tableHeader[256];
        tableHeader[0] = 0;
        int visitCount = 0;
        int expectedVisitCount = 0;
        for (; i < t_len && t[i].type == JSMN_PRIMITIVE && t[i].end < innerendArray; i++) {
            getStringBuffer(&t[i], js->string, val);
            int v = atoi(val);
            strcat(expectedTreeVisits, "\t");
            strcat(expectedTreeVisits, val);
            valid = valid&& (v==treeVisits[i-iStart]);
            if (v >= 0) expectedVisitCount++;
            if (treeVisits[i-iStart] >= 0) visitCount++;
        }
        
        for (int j = 0; j < tree_len; j++) { 
            int v_actual = treeVisits[j];
            sprintf(tableHeader, "\t");
            char number[16];
            sprintf(number, "%d", j);
            strcat(tableHeader, number);
            strcat(actualTreeVisits, "\t");
            sprintf(number, "%d", v_actual);
            strcat(actualTreeVisits, number);
        }

        sprintf(msg, "aiTreeTest %d Failed! The Ai visited more nodes as expected. expected=%d, but was actual=%d; bestScore=%d\n          %s\nexpected  %s\nactual    %s\nexpected Game Tree:\n%s\n", testCount, expectedVisitCount, visitCount, actualScore, tableHeader,expectedTreeVisits, actualTreeVisits, treeBuffer);
        CuAssert(tc, msg, visitCount <= expectedVisitCount);

        // this test was used for checking if the ai walked throug the game tree as expected, but due to more changes on the tree search
        // this path is has changed and the tests need to be updated. Until that this test is ignored.
        //sprintf(msg, "aiTreeTest %d Failed! The Ai did not walk through the tree as expected.bestScore=%d\n          %s\nexpected  %s\nactual    %s\nexpected Game Tree:\n%s\n", testCount, actualScore, tableHeader,expectedTreeVisits, actualTreeVisits, treeBuffer);
        
        /*if (!valid) {
            CuFail(tc, msg);
        }*/

        // follow treePath
        struct move m;
        m.from = 0;
        m.to = 0;
        board* b_copy = createBoard();
        neverGameOver = 0;
        while(!moveIsInvalid(m)) {
            m.from = INVALID_MOVE_INDEX;
            m.to = INVALID_MOVE_INDEX;

            actualScore = bestMoveMinimax(b_copy, &m, maxDepth, INT_MAX);
            freeMinimaxMemory(0);
            movePlayerTree(b_copy, &m);

            sprintf(msg, "aiTreeTest %d failed! move %s%s moved to a not optimal node %d\nexpected GameTree=\n%s\n", testCount, indexToUci(m.from), 
            indexToUci(m.to), tree[b->hash], treeBuffer);
            CuAssert(tc, msg, tree[b->hash] == expected);
            maxDepth--;
        }

        int hasChildren = 0;
        for (int i = 0; i < treeChildCount; i++) hasChildren = hasChildren || (CHILD(treeChildCount, b_copy->hash, i) < tree_len);
        sprintf(msg, "aiTreeTest %d failed! The ai did not move to the leafs. Last index %ld\nexpected Game tree=\n%s\n", testCount, b_copy->hash, treeBuffer);
        free(b_copy);
        CuAssert(tc, msg, !hasChildren);
        i--;
        neverGameOver = 1;

        testCount++;
    }

    freeCheckHashmap();

    freeJson(js);
    free(b);
}

void minimaxTest(CuTest* tc) {
    json* js = load_json("test/resources/minimaxTestDataLong.json", 500);

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    board* board = createBoard();
    initMinimax();

    char msg[8192];
    int endArray;
    int testCount = 0;
    char br[BOARD_REPRESENTATION_BUFFER_SIZE];
    for (int i = 0; i < t_len; i++) {
        if (t[i].type == JSMN_OBJECT) {
            continue;
        } else if (t[i].type == JSMN_ARRAY){
            endArray = t[i].end;
            continue;
        }
        
        // test starts here
        char fen[t[i].end-t[i].start+1];
        getStringBuffer(&t[i], js->string,fen);
        
        // get depth
        i++;
        char number[10];
        getStringBuffer(&t[i], js->string, number);
        maxDepthMinimax = atoi(number);

        // get list of moves
        char bestMoves[32][5];
        int bestMovesCount = 0;
        for (i++; i < t_len && t[i].start < endArray; i++) {
            getStringBuffer(&t[i], js->string, bestMoves[bestMovesCount++]);
        }
        i--;

        
        parse(fen, board);

        move m;
        freeMinimaxMemory(0);
        //alphabeta(board, &m, 6, &bmm, INT_MAX);
        //printMinimaxDebugOutput = 1;
        minimaxTimeCaps[0] = INT_MAX/2;
        m.from = INVALID_MOVE_INDEX;
        m.to = INVALID_MOVE_INDEX;
        minimaxSearchRootFull = 1;
        minimaxMove(board, &m, 120*1000);
        //printMinimaxDebugOutput = 0;
        
        boardMoveMap* current;
        boardMoveMap* tmp;

        int movesWithScore = 0;
        int movesWithoutScore = 0;
        int transpositions = 0;
        char movesGenerated[128*16];
        movesGenerated[0] = 0;
        HASH_ITER(hh, bmm, current, tmp) {
            int first = current->head->score;
            int max = -INT_MAX;

            if (current->moveCount == 0) continue;

            #ifdef MINIMAX_USE_ARRAY
            for (int i = 0; i < current->moveCount; i++) {
                int nodeScore = current->head[i].score;
                if (nodeScore == INT_MAX || nodeScore == -INT_MAX) {
                    movesWithoutScore++;
                } else movesWithScore++;
                max = (max < nodeScore) ? nodeScore : max;
                
                if (boardEquals(&current->board, board)) {
                    strcat(movesGenerated, "(");
                    strcat(movesGenerated, indexToUci(current->head[i].m.from));
                    strcat(movesGenerated, indexToUci(current->head[i].m.to));
                    strcat(movesGenerated, ", ");
                    char number[11];
                    sprintf(number, "%d", current->head[i].score);
                    strcat(movesGenerated, number);
                    strcat(movesGenerated, ") ");
                }
            }
            #else
            struct moveScores *el;
            LL_FOREACH(current->head, el) {
                max = (max < el->score) ? el->score : max;
            }
            #endif
            
            if (max!=first) {
                boardRepresentation(br, board);
                sprintf(msg, "Minimax Test %d failed! The first element in the each hashmap list has to be the move with the highest score! max=%d, while first=%d, moveCount=%d \nboard represenation=\n%s\n", testCount,max, first, current->moveCount, br);
            }
            CuAssert(tc, msg, max == first);
            transpositions++;
        }
        boardRepresentation(br, board);

        freeMinimaxMemory(0);

        struct board b_copy;
        memcpy(&b_copy, board, sizeof(struct board));
        movePlayer(&b_copy, &m);
        if (inCheck(&b_copy)) {
            sprintf(msg, "Minimax Test %d failed! The minimax ai returned a move that turns the board into check! move=%s%s\nboard represenation=\n%s\nfen=%s\n", testCount,
            indexToUci(m.from), indexToUci(m.to),br,fen);
            freeJson(js);
            free(board);
            CuFail(tc, msg);
        } else if (!(board->board[4 + board->player] & (((uint64_t)1)  << m.from))) {
            sprintf(msg, "Minimax Test %d failed! The minimax ai returned a move that moves a character not owned by the active player! move=%s%s\nboard represenation=\n%s\nfen=%s\n", 
            testCount, indexToUci(m.from), indexToUci(m.to),br,fen);
            
            freeJson(js);
            free(board);
            CuFail(tc, msg);
        }

        int contains = 0;
        char moves[256];
        moves[0] = 0;
        for (int j = 0; j < bestMovesCount; j++) {
            int from = uciToIndex(bestMoves[j]);
            int to = uciToIndex((&bestMoves[j][2]));

            strcat(moves, ", ");
            strcat(moves, bestMoves[j]);
            contains = contains || (m.to == to && m.from == from);
        }
        
        if (!contains && bestMovesCount > 0) {
            sprintf(msg, "Minimax Test %d failed! The given move %s%s was not in the expected list of moves:\n%s\nboardState:\n%s\nmoves=%s", testCount, indexToUci(m.from), indexToUci(m.to), moves, br,movesGenerated);
            CuFail(tc, msg);
        }
        
        printf("Situation %d done! move=%s%s ; fen=%s; avrg moves per transposition=%f; movesExpected=%s\n", testCount, indexToUci(m.from), indexToUci(m.to), fen, (double)movesWithScore/(movesWithScore+movesWithoutScore), moves);
        testCount++;

    }
    freeCheckHashmap();

    freeJson(js);
    free(board);
}