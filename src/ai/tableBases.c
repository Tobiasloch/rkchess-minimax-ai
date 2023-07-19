#include "tableBases.h"

char* transpositionTableFiles[] = {"src/ai/tablebases/startGameTableBase.json", "src/ai/tablebases/1.json", 
"src/ai/tablebases/2.json","src/ai/tablebases/3.json", "src/ai/tablebases/4.json", "src/ai/tablebases/5.json"};

void initTranspositionTables() {
    for (int i = 0; i < sizeof(transpositionTableFiles)/sizeof(transpositionTableFiles[0]); i++) {
        initTranspositionTable(transpositionTableFiles[i]);
    }
}

void initTranspositionTable(char* file) {
    int before = PRINT_JSON_FILE_OPENED;
    PRINT_JSON_FILE_OPENED = 0;
    json* js = load_json(file, 32768);
    PRINT_JSON_FILE_OPENED = before;
    

    jsmntok_t* t = js->tokens;
    size_t t_len = js->tokens_len;

    int endArray = 0;
    struct moveScores moveList[128];
    for (int i = 0; i < t_len; i++) {
        if (endArray < t[i].end && t[i].type != JSMN_ARRAY) {
            continue;
        } else if (t[i].type == JSMN_ARRAY){
            endArray = t[i].end;
            continue;
        }
        
        // test starts here
        char fen[t[i].end-t[i].start+1];
        getStringBuffer(&t[i], js->string,fen);
        i++;

        char uciMove[t[i].end-t[i].start+1];
        getStringBuffer(&t[i], js->string, uciMove);
        i++;

        char depthString[t[i].end-t[i].start+1];
        getStringBuffer(&t[i], js->string, depthString);
        int depth = atoi(depthString);
        i++;

        int score = 0;
        if (t[i].type == JSMN_STRING) {
            char scoreString[t[i].end-t[i].start+1];
            getStringBuffer(&t[i], js->string, scoreString);
            score = atoi(scoreString);
        } else i--;
        

        struct boardMoveMap* item = malloc(sizeof(struct boardMoveMap));
        item->isUpperBound = 0;
        item->depth = depth;

        parse(fen, &item->board);
        item->board.hash ^= item->board.player;

        struct boardMoveMap* node = NULL;
        HASH_FIND(hh, bmm, &item->board.hash, sizeof(uint64_t), node);
        if (node != NULL && node->depth >= depth) {
            free(item);
            continue;
        }

        moveIterator iter;
        moveGen_init(&iter, &item->board);
        struct moveScores* ms = &moveList[0];
        ms->m.from = uciToIndex(uciMove);
        ms->m.to = uciToIndex(&uciMove[2]);
        ms->score = score;

        int moveCount = 1;
        for (moveGen_next(&iter); iter.hasActive; moveGen_next(&iter)) {
            if (iter.active.from == moveList[0].m.from && iter.active.to == moveList[0].m.to) continue; // the bestmove

            ms = &moveList[moveCount];
            memcpy(&ms->m, &iter.active, sizeof(move));
            ms->score = -INT_MAX;
            moveCount++;
        }
        item->moveCount = moveCount;
        item->head = malloc(sizeof(struct moveScores)*moveCount);
        memcpy(item->head, moveList, sizeof(struct moveScores)*moveCount);

        item->board.rounds = item->board.rounds*2;
        if (item->board.player == BLACK_PLAYER) item->board.rounds++;

        HASH_ADD(hh, bmm, board.hash, sizeof(uint64_t), item);
    }

    freeJson(js);
}