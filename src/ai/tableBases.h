#include <stdio.h>
#include <stdlib.h>
#include "ai/minimax.h"
#include "jsonLoader.h"
#include "uthash/src/utlist.h"
#include "uthash/src/uthash.h"
#include <limits.h>
#include "board/board.h"

void initTranspositionTable(char* file);
void initTranspositionTables();