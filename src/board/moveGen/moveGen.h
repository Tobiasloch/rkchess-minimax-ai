#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "board/board.h"
#include "board/moveIterator.h"

void moveGen_init(moveIterator* iterator, board* board);
void moveGen_next(moveIterator* iterator);
bool_t moveGen_isProtected(moveIterator* iterator, int figIdx);


#ifdef __cplusplus
}
#endif