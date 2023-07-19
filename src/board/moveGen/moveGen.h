#pragma once

#include <stdint.h>

#include "board/board.h"
#include "board/moveIterator.h"

void moveGen_init(moveIterator* iterator, board* board);
void moveGen_next(moveIterator* iterator);
bool moveGen_isProtected(moveIterator* iterator, int figIdx);
