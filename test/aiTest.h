#ifndef MINIMAXTEST_H_
#define MINIMAXTEST_H_

#ifndef TEST
#define TEST
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "CuTest.h"
#include "board/board.h"
#include "board/boardState.h"
#include "ai/minimax.h"
#include "jsonLoader.h"
#include "uthash/src/uthash.h"
#include <math.h>

void minimaxTest(CuTest* tc);
void minimaxTreeTest(CuTest* tc);

#endif // header
