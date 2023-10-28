#ifndef VALIDMOVESTEST_H_
#define VALIDMOVESTEST_H_

#include <stdlib.h>
#include <stdio.h>
#include "CuTest.h"
#include "board/board.h"
#include "ai/minimax.h"
#include "board/validMoves.h"
#include "board/moveGen/moveGen.h"
#include "jsonLoader.h"
#include "uthash/src/uthash.h"

void listIteratorTest(CuTest* tc);
void validMovesTestOld(CuTest* tc);
void validMovesTestNew(CuTest* tc) ;

#endif // header
