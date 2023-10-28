#ifndef FENPARSERTEST_H_
#define FENPARSERTEST_H_


#include "../lib/CuTest.h"
#include "../src/board/board.h"
#include "jsonLoader.h"
#include <string.h>

void setFieldTest(CuTest* tc);
void TestParser(CuTest* tc);
void boardToFenTest(CuTest* tc);
void getFieldTest(CuTest* tc);
void movePlayerTest(CuTest* tc);

#endif
