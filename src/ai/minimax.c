#include "minimax.h"


// hexadecimal for rtwo (ronja, tobi, willy, oskar)
#define DEFAULT_SEED 0x7274776f


#define MAX_MOBILITY (128)
#define SCORE_MAX (INT_MAX/2)

#ifdef MINIMAX
#undef ITERATIVEMINIMAX
#endif

#ifndef ITERATIVEMINIMAX
#undef MTDF
#endif

#ifndef MTDF
#undef NULLMOVEREDUCTION
#endif

#ifdef TEST
#undef BESTMOVERANDOMSELECTION
#undef PRINTBESTMOVEPATHS
#endif

#ifdef BENCHMARK
#undef BESTMOVERANDOMSELECTION
#undef PRINTBESTMOVEPATHS
#endif

#define MAXGAMETIME (60*1000)

struct boardMoveMap* bmm = NULL;

int minimaxSearchRootFull = 0;
long INITIAL_MINIMAX_SEED = 0;
int printMinimaxDebugOutput = 0;
#ifdef NULLMOVEREDUCTION
int maxDepthMinimax = 21;
#else
int maxDepthMinimax = 9;
#endif
int goDown[] = {30*1000, 10*1000, 5000,0};
int minimaxTimeCaps[] = {5*1000, 2*1000, 500, 100};

#ifdef NULLMOVEREDUCTION
int inNullMoveSearch = 0;
#else
double minimaxAverageBranchingFactor = 0;
int minimaxNodeCount = 0;
#endif
void minimaxMove(board* b, move* bestMove, long timeLeft) {
    int d;
    int score = 0;
    int depth = -1;

    for (d = 0; d < sizeof(goDown)/sizeof(goDown[0]); d++) {
        if (timeLeft > goDown[d]) {
            depth = (maxDepthMinimax-d > 0) ? maxDepthMinimax-d : 1;

            score = bestMoveMinimax(b, bestMove, depth, minimaxTimeCaps[d]);
            break;
        }
    }

    if (printMinimaxDebugOutput) {
        infoln(2, "Evaluation before move: %d", evaluateBoard(b));
        infoln(2, "Score: %d", score);
        infoln(2, "Memory usage: %ld KB", minimaxMemorySize()/1000);

        char br[BOARD_REPRESENTATION_BUFFER_SIZE];
        boardRepresentation(br, b);
        infoln(2, "Current Board:\n%s", br);

        uint64_t hashcode = b->hash^b->player;
        struct boardMoveMap* node = NULL;
        HASH_FIND(hh, bmm, &hashcode, sizeof(uint64_t), node);
        infof(2, "Moves: ");
        sortMovesMinimax(node->head, node->moveCount);
        for (int i = 0; i < node->moveCount; i++) {
            infofr(2, "(%s%s,%d) ", indexToUci(node->head[i].m.from), indexToUci(node->head[i].m.to), node->head[i].score);
        }
        infofr(2, "\n");
    }
}

/**
 * incremental
 * @return returns score
 * @param maxTime time the algorithm can spent maximum in millis
 * @param bestMove a pointer to a move struct in which the best move is placed
 *
*/
int bestMoveMinimax(board* b, move* bestMove, int maxDepth, long maxTime) {
    int score = SCORE_MAX;
    int depth = 1;
    minimaxHashCollisions = 0;
    #ifndef ITERATIVEMINIMAX
    depth = maxDepth;
    #endif

    if (printMinimaxDebugOutput) {
        printMinimaxDebugOutput = maxDepth;
        infoln(2, "Time to calculate: %ld", maxTime);
    }

    if (bestMove != NULL) {
        bestMove->from = INVALID_MOVE_INDEX;
        bestMove->to = INVALID_MOVE_INDEX;
    }

    double start = get_time();
    double end = start+(((double)maxTime)/1000.0);
    double active = start;
    move localMove;
    #ifdef SAVE_BEST_MOVES
    minimaxBestBoves_len = 0;
    #endif
    if (bestMove != NULL) memcpy(&localMove, bestMove, sizeof(struct move));

    #ifdef NULLMOVEREDUCTION
    long timeNeeded[maxDepth+1];
    timeNeeded[0] = 0;
    #else
    minimaxAverageBranchingFactor = 0;
    minimaxNodeCount = 0;
    #endif
    while(depth <= maxDepth && active < end) {
        struct minimaxReturn mr;
        #ifdef MTDF
        int bounds[2] = {-SCORE_MAX, SCORE_MAX};
        mr.score = score;
        //if (depth == maxDepth) infoln(2, "min=%d, max=%d", bounds[0], bounds[1]);

        do {
        int beta = mr.score;
        if (beta == bounds[0]) beta++;
        //if (depth == maxDepth) infoln(2, "beta=%d", beta);

        #ifdef NULLMOVEREDUCTION
        inNullMoveSearch = 0;
        #endif

        mr = alphabeta(b, &localMove, depth, &bmm, beta, end);
        if (mr.score < beta) bounds[1] = mr.score;
        else bounds[0] = mr.score;

        active = get_time();
        //if (depth == maxDepth) infoln(2, "min=%d, max=%d", bounds[0], bounds[1]);
        } while(bounds[0] < bounds[1] && active < end);
        #else
        mr = alphabeta(b, &localMove, depth, &bmm, SCORE_MAX, end);
        #endif

        if (active < end) {
            if (bestMove != NULL) memcpy(bestMove, &localMove, sizeof(struct move));
            score = mr.score;
        } else {
            break;
        }

        depth++;

        long timeSpent = (active-start)*1000;

        #ifdef NULLMOVEREDUCTION
        timeNeeded[depth] = timeSpent;

        if (printMinimaxDebugOutput) {
            infoln(2, "Time spent: %ld, Time: %ld", timeNeeded[depth]-timeNeeded[depth-1], timeSpent);
        }
        #else
        long expectedTimeForNextRun = timeSpent*minimaxAverageBranchingFactor;
        if (printMinimaxDebugOutput) {
            infoln(2, "Branching factor: %f, Expected time for next run: %ld, Time: %ld", minimaxAverageBranchingFactor, expectedTimeForNextRun, timeSpent);
        }
        if (expectedTimeForNextRun >= maxTime) break;
        #endif
    }
    if (printMinimaxDebugOutput)
        infoln(2, "Score taken from depth: %d", depth-1);

    return score;
}

void listIterator(moveIterator* iter) {
    if (iter->preGeneratedMovesList==NULL || iter->direction >= iter->moveNum) {
        iter->hasActive = 0;
        return;
    }

    memcpy(&iter->active, &iter->preGeneratedMovesList[iter->direction].m, sizeof(move));
    iter->hasActive = 1;
    iter->direction++;
}

void initListIterator(moveIterator* iter, board* b) {
    memcpy(iter->preGeneratedMovesList, iter->bmm->head, sizeof(struct moveScores)*iter->bmm->moveCount);
    iter->moveNum = iter->bmm->moveCount;

    iter->hasActive = iter->moveNum>0;
    iter->direction = 0;
    iter->board = b;

    // TODO: select best fitting sorting algorithm
    sortMovesMinimax(iter->preGeneratedMovesList, iter->moveNum);
}

struct board* compareBoard = NULL;
int compareMoves( const void* a, const void* b)
{
     moveScores move_a = * ( (moveScores*) a );
     moveScores move_b = * ( (moveScores*) b );

     if (move_a.score == move_b.score) return 0;
     else if ( move_a.score > move_b.score ) return -1;
     else return 1;
}


size_t minimaxMemorySize() {
    return HASH_COUNT(bmm)*(sizeof(struct boardMoveMap)+sizeof(struct moveScores)*MAX_MOVE_COUNT);
}

int OLDEST_TRANSPOSITIONS = -1;
void freeMinimaxMemory(long limit) {
    if (limit <= 0) freeMinimaxHash(&bmm);

    boardMoveMap* current;
    boardMoveMap* tmp;
    struct boardMoveMap** hashmap = &bmm;

    if (minimaxMemorySize()/1000 < limit) return;

    if (printMinimaxDebugOutput)
        infoln(2, "Hash size: %d", HASH_COUNT(*hashmap));
    while(OLDEST_TRANSPOSITIONS < 0) {
        int secondOldest = INT_MAX;
        HASH_ITER(hh, *hashmap, current, tmp) {

            if (current->board.rounds <= OLDEST_TRANSPOSITIONS) {
                HASH_DEL(*hashmap,current);
                free(current->head);
                free(current);

                if (minimaxMemorySize()/1000 < limit) return;
            } else {
                secondOldest = (secondOldest > current->board.rounds) ? current->board.rounds : secondOldest;
            }
        }
        OLDEST_TRANSPOSITIONS = secondOldest;
    }
}

void freeMinimaxHash(struct boardMoveMap** hashmap) {
    boardMoveMap* current;
    boardMoveMap* tmp;

    OLDEST_TRANSPOSITIONS = -1;
    HASH_ITER(hh, *hashmap, current, tmp) {
        HASH_DEL(*hashmap,current);
        free(current->head);
        free(current);
    }
}

void quickSortMinimax(struct moveScores* moves, size_t len) {
    qsort(moves, len, sizeof(moveScores), compareMoves);
}

void initMinimax() {
    #ifdef BESTMOVERANDOMSELECTION
    if (INITIAL_MINIMAX_SEED == 0) INITIAL_MINIMAX_SEED = time(NULL);
    #else
    if (INITIAL_MINIMAX_SEED == 0) INITIAL_MINIMAX_SEED = DEFAULT_SEED;
    #endif
    initZobrist();
    initCheckHashmap();
    initEvaluatePiece();
    initTranspositionTables();

    if (printMinimaxDebugOutput)
        infoln(2, "Minimax seed: %ld", INITIAL_MINIMAX_SEED);

    srand(INITIAL_MINIMAX_SEED);
    #ifndef USEOLDMOVEITERATOR
    minimaxIterator = &moveGen_next;
    initMoveIteratorMinimax = &moveGen_init;
    #else
    minimaxIterator = &iterateMoves;
    initMoveIteratorMinimax = &initMoveIterator;
    #endif
    minimaxEvaluator = &evaluateBoard;
    movePlayerMinimax = &movePlayer;
    gameOverCheckMinimax = &isGameOverExtended;
    sortMovesMinimax = &quickSortMinimax;
    inCheckMinimax = &inCheckFast;
}

#ifdef PRINTBESTMOVEPATHS
struct move bestMovePath[8];
#endif
struct minimaxReturn alphabeta(board* b, move* bestMove, long maxDepth, struct boardMoveMap** hashmap, int beta, double end) {
    moveIterator iter;
    void (*iterator)(moveIterator*) = minimaxIterator;
    void (*initIterator)(moveIterator*, board*) = initMoveIteratorMinimax;

    struct minimaxReturn mr;
    int alpha = -SCORE_MAX;
    mr.score = alpha;

    #ifdef ITERATIVEMINIMAX
    // this list is useful for move sorting because of the incremental alpha beta search
    struct boardMoveMap* node = NULL;

    uint64_t hashcode = b->hash^b->player;
    HASH_FIND(hh, *hashmap, &hashcode, sizeof(uint64_t), node);
    int nodeMovesCount = 128;
    int transpositionEqual = (node != NULL && boardEquals(&node->board, b));
    if (transpositionEqual) {
        // wenn ein move bereits gesehen wurde und mit gleicher oder höherer tiefe durchsucht wurde,
        // dann gebe dessen ergebnis zurück
        if (node->depth >= maxDepth) {
            struct moveScores* head = node->head;
            if (bestMove != NULL && head != NULL) memcpy(bestMove, &head->m, sizeof(struct move));
            int nodeScore = head->score;
            #ifdef MTDF
            if ((node->isUpperBound && nodeScore < beta) || (!node->isUpperBound && nodeScore >= beta)) {
                #endif
                mr.score = nodeScore;
                return mr;

                #ifdef MTDF
            }
            #endif
        }

        nodeMovesCount = node->moveCount;
        iter.bmm = node;
        iterator = &listIterator;
        initIterator = &initListIterator;
    } else if (node != NULL && !transpositionEqual) minimaxHashCollisions++;

    struct moveScores movesBuffer[nodeMovesCount];
    iter.preGeneratedMovesList = movesBuffer;
    struct moveScores* moveList = movesBuffer;
    #endif

    initIterator(&iter, b);
    iterator(&iter);

    struct minimaxReturn internalReturn;
    board b_copy;

    // Null Move reduction
    #ifdef NULLMOVEREDUCTION
    if (maxDepth > 3 && inNullMoveSearch == 0) {
        int R = (maxDepth > 6) ? 4 : 3;
        struct move nullMove = {.from = 0, .to = 0};
        struct board nullMoveBoard;
        memcpy(&nullMoveBoard, b, sizeof(struct board));
        movePlayerMinimax(&nullMoveBoard, &nullMove);
        inNullMoveSearch++;
        internalReturn = alphabeta(&nullMoveBoard, NULL, maxDepth-R, hashmap, -(beta-1), end);
        inNullMoveSearch--;
        if (internalReturn.score >= beta) {
            maxDepth -= 2;
        }
    }
    #endif

    double activeTime = get_time();
    if (maxDepth <= 0 || !iter.hasActive || activeTime >= end) {
        mr.score = minimaxEvaluator(b);
        return mr;
    }


    #ifdef PRINTBESTMOVEPATHS
    struct move localBestMove[maxDepth+1];
    #endif
    int mobility = 0;
    int nodesSearched = 0;

    for (;iter.hasActive; iterator(&iter)) {
        internalReturn.score = -SCORE_MAX;
            #ifndef MINIMAX
            if (alpha < beta || (minimaxSearchRootFull && bestMove != NULL)) {
            #endif
                memcpy(&b_copy, b, sizeof(board));

                movePlayerMinimax(&b_copy, &iter.active);
                #ifdef USEOLDMOVEITERATOR
                if (inCheckMinimax(&b_copy)) continue;
                #endif
                nodesSearched++;

                int gameOver = gameOverCheckMinimax(b, &b_copy);
                if (gameOver) {
                    int score = 0; // draw
                    if (gameOver == WHITEWON || gameOver == BLACKWON) {
                        if (b->player == gameOver) score = SCORE_MAX;
                        else score = -SCORE_MAX;
                    }
                    internalReturn.score = score;
                } else {
                    // wenn ein move einen character schlägt, dann ist das ein pv node, also ein sehr aussagekräftiger Zug
                    int depth = maxDepth-1;

                    // Quiescience search
                    if (depth <= 0) {
                        uint64_t pvNode = ((uint64_t)1) << iter.active.to;
                        pvNode = pvNode & b->board[5+(b->player%2)];
                        if (pvNode) { // wenn ein character geschlagen wird
                            depth = 1;
                        }
                    }

                    #ifdef MTDF
                    internalReturn = alphabeta(&b_copy, NULL, depth, hashmap, -(beta-1), end);
                    #else
                    internalReturn = alphabeta(b_copy, NULL, depth, hashmap, -alpha, end);
                    #endif
                    internalReturn.score *= -1;
                }


            #ifndef MINIMAX
            } else {
                internalReturn.score = -INT_MAX;
            } // continue because all values should put into hashmap
            #endif


        #ifdef ITERATIVEMINIMAX // copies the active move and score into move list
        int index = mobility;
        if (internalReturn.score > alpha && mobility > 0) {
            memcpy(&(moveList[mobility].m), &(moveList[0].m), sizeof(move));
            moveList[mobility].score = moveList[0].score;
            index = 0;
        }

        memcpy(&(moveList[index].m), &iter.active, sizeof(move));
        moveList[index].score = internalReturn.score;
        #endif
        mobility++;

        if (internalReturn.score > alpha) {
            alpha = internalReturn.score;

            if (bestMove != NULL) {
                memcpy(bestMove, &iter.active, sizeof(move));
            }

            #ifdef PRINTBESTMOVEPATHS
            if (printMinimaxDebugOutput == maxDepth) {
                bestMovePath[maxDepth] = iter.active;
                memcpy(localBestMove, bestMovePath, sizeof(struct move)*maxDepth);
            }
            #endif
        }

        #ifdef PRINTBESTMOVEPATHS
        if (printMinimaxDebugOutput == maxDepth) {
            infof(2, "Move path: (%s%s) ", indexToUci(iter.active.from), indexToUci(iter.active.to));
            for (int i = maxDepthMinimax-1; i > 0; i--) {
                infofr(2, "(%s%s) ", indexToUci(bestMovePath[i].from), indexToUci(bestMovePath[i].to));
            }
            infofr(2, "\n");
        }
        #endif
    }

    #ifdef ITERATIVEMINIMAX
    // TODO: tiefe nur vergleichen, wenn transposition auch dieselbe situation ist
    if (node == NULL || node->depth <= maxDepth) {
        struct boardMoveMap* newMoves;
        if (node == NULL) newMoves = malloc(sizeof(struct boardMoveMap));
        else {
            free(node->head);
            newMoves = node;
        }

        if (newMoves == NULL) freeMinimaxMemory(100*000);

        //copy board to newMoves
        memcpy(&newMoves->board, b, sizeof(struct board));
        newMoves->board.hash=hashcode;
        newMoves->depth = maxDepth;
        newMoves->isUpperBound = 0;
        newMoves->head = malloc(sizeof(struct moveScores) * mobility);
        if (newMoves->head == NULL) freeMinimaxMemory(100*1000);
        memcpy(newMoves->head, moveList, sizeof(struct moveScores)*mobility);
        newMoves->moveCount = mobility;
        newMoves->board.rounds *= 2;
        if (newMoves->board.player == BLACK_PLAYER) newMoves->board.rounds++;

        #ifdef MTDF
        if (alpha < beta) newMoves->isUpperBound = 1;
        else newMoves->isUpperBound = 0;
        #endif

        if (node == NULL) HASH_ADD(hh,*hashmap, board.hash, sizeof(uint64_t), newMoves); // adding this move to hashmap
    }
    #endif

    #ifdef PRINTBESTMOVEPATHS
    if (printMinimaxDebugOutput == maxDepth) {
        memcpy(bestMovePath, localBestMove, sizeof(move)*maxDepth);
    }
    #endif

    #ifndef NULLMOVEREDUCTION
    minimaxNodeCount++;
    minimaxAverageBranchingFactor += ((double)nodesSearched-minimaxAverageBranchingFactor)/(double)minimaxNodeCount;
    #endif

    mr.score = alpha;
    return mr;
}
