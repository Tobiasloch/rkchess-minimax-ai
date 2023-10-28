#ifndef CHECKTEST_H_
#define CHECKTEST_H_

#include <stdlib.h>
#include <stdio.h>
#include "../lib/CuTest.h"
#include "../src/board/board.h"
#include "../src/board/boardState.h"
#include "jsonLoader.h"

void checkTest(CuTest* tc);
void gameOverTest(CuTest* tc);

#endif // header
