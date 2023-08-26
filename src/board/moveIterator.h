#pragma once

#include <stdint.h>

#include "board/board.h"

struct moveIterator {

	// The currently active move
	move active;

	// boolean value that indicates if a move was found
	int hasActive;

	// Reference to the current global board
	// Set on initialization. Do not edit content.
	struct board* board;

	/* OLD MOVEGEN (validMoves.h) */

	int moveNum;
	int direction;
	uint64_t characterBoard;

	/* NEW MOVEGEN (moveGen/moveGen.h) */

	// Map of moves and hits for currently active figure
	// Updated (reeduced) with each get()
	// Regenerated for next figure when empty
	uint64_t okMoves;
	uint64_t hitMoves;

	// Position of currently active figure
	// Updated with next figure generation
	int figIdx;

	// Map of all own figures yet to be checked
	// Updated with next figure generation
	uint64_t figuresToCheck;
	int figTypeCheckOrderIdx;

	// Maps for checking check and pin situations
	uint64_t oppAttackBoard;
	uint64_t oppKingBlocks[5];
	uint64_t pinMoves[64];
	uint64_t kingBoard;

	// Which board is ours and which is the opponents
	// Set on initialization
	uint64_t ownBoard;
	uint64_t oppBoard;

	/* MINIMAX */
	// Not used my MoveGens
	struct moveScores* preGeneratedMovesList;
	struct boardMoveMap* bmm;

} typedef moveIterator;
