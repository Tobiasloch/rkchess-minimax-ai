#include <stdint.h>
#include <stdlib.h>

#include "util.h"

#include "board/board.h"

#include "rays.h"
#include "moves.h"
#include "bitboard.h"
#include "figureData.h"

#include "moveGen.h"

// rand() is seeded _once_ by initMinimax() in minimax.c
// Usually with a static seed for reproducability
bool randBool(void) {
	return (rand() % 2);
}

int randFirstOrLast1(uint64_t mask) {
	return randBool() ? findHighest1(mask) : findLowest1(mask);
}

int randFirst2(uint64_t mask) {

	int first = findHighest1(mask);

	// If only one figure or 50% chance, take the first
	if (countSetBits(mask) == 1 || randBool() )
		return first;

	// On the other 50%, take the next

	// Remove first bit
	mask &= ~((uint64_t) 1 << first);

	return findHighest1(mask);

}

// Always choose the first move/figure
// int (*choose1Function)(uint64_t) = &(findHighest1);

// Choose the first or last move/figure
// int (*choose1Function)(uint64_t) = &(randFirstOrLast1);

// Choose one of the first 2 moves/figures
int (*choose1Function)(uint64_t) = &(randFirst2);

boardContent_t posFigTypeCache[64];

boardContent_t getFigTypeAtIdx(board* board, int idx) {

	if (posFigTypeCache[idx] == NONE) {

		uint64_t posMask = (uint64_t) 1 << idx;

		posFigTypeCache[idx] =  (posMask & board->board[QUEEN])  ? QUEEN  :
								(posMask & board->board[KING])   ? KING   :
								(posMask & board->board[BISHOP]) ? BISHOP :
								(posMask & board->board[KNIGHT]) ? KNIGHT :
								(posMask & board->board[ROOK])   ? ROOK   :
								NONE;

	}

	return posFigTypeCache[idx];

}

// Create bitmap of possible moves for this figure
// Also a map for hits, beacause we like hits
// Pass a pinMoves array as in the iterator to know if figures are pinned
typedef struct { uint64_t okMoves; uint64_t hitMoves; } moveMasks_t;
moveMasks_t generateMoves(
	moveIterator* iter,		//< Iterator for some variables. Will only be read.
	int figIdx,				//< Where the figure is who's moves are calc'd
	boardContent_t figType,	//< What type that figure is supposed to be
	uint64_t ownBoard,		//< All own (completely blocked) fields
	uint64_t oppBoard		//< All enemy (hittable) fields
) {

	moveMasks_t mm = {
		.okMoves  = 0,
		.hitMoves = 0
	};

	if (figureData[figType].slider == FALSE) {

		// If this is not a slider (So a King or Knight) it's easy:
		// Just take the precalculated move map and remove all own figures
		uint64_t allMoves = FIGMOVES[figIdx][figType];

		// Can't move to fields used by ourselves
		mm.okMoves |= allMoves & ~ownBoard;

		mm.hitMoves |= mm.okMoves & oppBoard;

	} else {

		// If this _is_ a slider (So a Queean, Bishop or Rook) it's harder:
		// Firstly, let's check all possible move directions
		//TODO: Test GCC loop unrolling
		#pragma GCC unroll 4
		for (int d = 0; d < figureData[figType].directionCount; d++) {

			// Get the direction ID
			ray_t direction = figureData[figType].directions[d];
			uint64_t ray = RAYS[figIdx][direction];

			// Both our own pieces and the enemy's prevent moving forward
			uint64_t blockers = ray & (oppBoard | ownBoard);

			if (blockers) {

				// Find first blocker in ray direction
				int blockIdx = findClosest1(blockers, dirIsForward[direction]);

				// The blocked fields are exactly the ray originating from
				// That first blocker in the same direction we're checking
				uint64_t blockRay = RAYS[blockIdx][direction];

				// Remove blocked fields and own fields from ray
				ray &= ~(blockRay | ownBoard);

				// Add first blocked field to hit moves
				mm.hitMoves |= (oppBoard & ((uint64_t) 1 << blockIdx));

			}

			// Add this Ray to the OK moves
			mm.okMoves |= ray;

		}

	}

	// If this is the _real_ run (not setup), iter is passed and we can check
	// for checks.
	if (iter) {

		// Kings can't move onto fields under attack by the opponent
		if (figType == KING)
			mm.okMoves &= ~iter->oppAttackBoard;

		// Don't set the enemy in check.
		// In other words: We can't move onto any field that would be
		// attackable by our figure type from where the king is standing.
		uint64_t wouldCheckOpp = iter->oppKingBlocks[figType];
		mm.okMoves &= ~wouldCheckOpp;

		// if this figure is pinned, it can only move to fields between
		// attacker and king. These are precalculated. If the figure isn't
		// pinned, the mask if all 1s and has no effect.
		mm.okMoves &= iter->pinMoves[figIdx];

		// Apply changes to okMoves back to hitMoves
		mm.hitMoves &= mm.okMoves;

	}

	return mm;

}

// Generates all possible moves for one figure type
// Returns number of generated moves
int generateNextFigMoves(moveIterator* iter) {

	// STEP 1
	// Choose a figure type to check

	//debugln("ToCheck:");
	//printMask(iter->figuresToCheck);

	// Check if any figures are left to check
	if (iter->figuresToCheck == 0) {
		// No more figures left to check.
		iter->figIdx = INVALID_MOVE_INDEX;
		iter->okMoves = 0;
		iter->hitMoves = 0;
		return -1;
	}

	// Pre-sort figure types
	boardContent_t figTypeCheckOrder[5] = {KING, QUEEN, ROOK, BISHOP, KNIGHT};
	boardContent_t figType = NONE;
	for (; iter->figTypeCheckOrderIdx < 5; iter->figTypeCheckOrderIdx++) {

		// We continue where we left off with the last figure
		figType = figTypeCheckOrder[iter->figTypeCheckOrderIdx];

		// Find all figures of that type that we haven't checked yet
		uint64_t figTypeMask = iter->board->board[figType];
		uint64_t figTypeFigsToCheck = iter->figuresToCheck & figTypeMask;

		// Get highest figure of those
		iter->figIdx = choose1Function(figTypeFigsToCheck);

		// if the result is positive, we found a figure and can stop searching
		if (iter->figIdx >= 0)
			break;

		// Otherwise, go on to the next figure type in the order
	}

	if (figType == NONE) {
		errorln("No FigType left to check. This should never happen.");
		return -2;
	}

	// Save type to cache because we have it on hand
	posFigTypeCache[iter->figIdx] = figType;

	// Remove it from the figures yet to check
	iter->figuresToCheck ^= (uint64_t) 1 << iter->figIdx;

	// STEP 2
	// Create bitmap of possible moves for this figure

	moveMasks_t mm = generateMoves(
		iter,
		iter->figIdx,
		figType,
		iter->ownBoard,
		iter->oppBoard
	);

	iter->okMoves = mm.okMoves;
	iter->hitMoves = mm.hitMoves;

	// Move count? Just use the pop count instruction
	// to get the number of set bits in a variable!
	int generatedMoveCount = countSetBits(iter->okMoves);
	//debugln("Generated %d new moves:", generatedMoveCount);
	//printMask(iter->okMoves);

	return generatedMoveCount;

}

void moveGen_next(moveIterator* iter) {

	if (iter->okMoves == 0) {

		//debugln("Generating new moves");

		// Generate new moves for next figure
		// If a figure has no moves, immediately check the next one.
		while (generateNextFigMoves(iter) == 0);

		if (iter->figIdx == INVALID_MOVE_INDEX) {
			// No figures left to generate from
			//debugln("NO MORE MOVES, OVER");
			iter->active.from = INVALID_MOVE_INDEX;
			iter->active.to = INVALID_MOVE_INDEX;
			iter->hasActive = FALSE;
			return;
		}

		//debugln("Generated moves:");
		//vector_t vec = idx2vec(iter->figIdx);
		//printf("Pos: (%d,%d), Type: %d\n",
		//	vec.x, vec.y, getFigTypeAtIdx(iter->board, iter->figIdx));
		//printMask(iter->okMoves);

		iter->hasActive = TRUE;

	}

	int from = iter->figIdx;
	int to = INVALID_MOVE_INDEX;

	if (iter->hitMoves) {

		//debugln("Found hit");

		// Take first hit move
		to = choose1Function(iter->hitMoves);

		// Remove it from the Maps
		iter->okMoves  ^= (uint64_t) 1 << to;
		iter->hitMoves ^= (uint64_t) 1 << to;

	} else if (iter->okMoves) {

		//debugln("Found move");

		// Take first move
		to = choose1Function(iter->okMoves);

		// Remove it from the Map
		iter->okMoves  ^= (uint64_t) 1 << to;

	}

	//printf("Move: ");
	//printMove(from, to);

	iter->active.from = from;
	iter->active.to = to;

}

void dirPinCheck(
	moveIterator* iter,
	uint64_t attackers,
	int kingIdx,
	ray_t direction
) {

	uint64_t ray = RAYS[kingIdx][direction];

	// Only attackers in this ray are relevant
	attackers &= ray;

	while (attackers != 0) {
		int attacker = findClosest1(attackers, dirIsForward[direction]);
		uint64_t oppRay = RAYS[attacker][oppositeDir[direction]];

		// Fields in between
		uint64_t pinRay = ray & oppRay;

		// Figures in pin ray
		uint64_t allFigMask = iter->ownBoard | iter->oppBoard;
		uint64_t pinRayFigs = pinRay & allFigMask;
		int pinRayFigCount = countSetBits(pinRayFigs);

		// a) A piece is pinned if it's the only one in between
		// b) It is only relevant to us if it's an own fig
		if ((pinRayFigCount == 1) && (pinRayFigs & iter->ownBoard)) {

			int pinIdx = findHighest1(pinRayFigs);

			// We save the ray going out from the king as the "pinMoves" for
			// this figure. When checking moves for this figure, we will only
			// allow moves on this ray (given the figure can actually move
			// there after all other rules are evaluated)
			iter->pinMoves[pinIdx] = ray;

			return;

		}

		// If there were no figures in between, it is still possible that
		// there is another attacker beyond the first one, possibly pinning
		// the first one. In this case, we go to the next attacker (if any)
		// Otherwise, we're done.
		if (pinRayFigCount != 0)
			return;

		// Remove from attackers to check
		attackers &= ~((uint64_t) 1 << attacker);

	}

}

void checkCheck_init(moveIterator* iter) {

	iter->oppAttackBoard = 0;
	memset(iter->oppKingBlocks, 0, sizeof(iter->oppKingBlocks));
	memset(iter->pinMoves, 0xFFFFFFFF, sizeof(iter->pinMoves));

	// PART 1: Pre-Calc fields that our King can't move to bc of enemy attacks

	// We remove our king from the board, because if the king moved away from
	// the attacker, that would technically be an unattacked field, but after
	// the move the king would still be attacked because he moved.
	// Therefore, we have to consider the attacked fields under the assumption
	// That our king is not there.
	uint64_t ownBoardNoKing = iter->ownBoard & ~(iter->kingBoard);

	// But here comes the twist:
	// Since we don't just want to block all empty fields reachable from the
	// attackers but also the ones where our king might hit something, we have
	// to also declare the opponent's figures as hittable for the attackers.
	// That way, they will also be in the block mask and our king won't hit
	// them and thereby not set himself into check
	uint64_t hittableBoard = ownBoardNoKing | iter->oppBoard;

	// Traverse all enemy figures
	uint64_t figuresToCheck = iter->oppBoard;
	while (figuresToCheck > 0) {

		int figIdx = findHighest1(figuresToCheck);
		boardContent_t figType = getFigTypeAtIdx(iter->board, figIdx);

		// Get moves and pinMoves
		// own and opp swapped because enemy
		moveMasks_t mm = generateMoves(
			NULL,
			figIdx,
			figType,
			0,
			hittableBoard //< all figures are hittable, none are not, s.a.
		);

		// They are attacking everything they can reach.
		iter->oppAttackBoard |= mm.okMoves;

		// Remove from board
		figuresToCheck ^= (uint64_t) 1 << figIdx;

	}

	// PART 2: Pre-Calc fields blocked bc they would set the enemy in check
	int oppKingIdx = findHighest1(iter->kingBoard & iter->oppBoard);
	if (oppKingIdx >= 0) {
		for (boardContent_t figType = 0; figType < 5; figType++) {

			moveMasks_t mm = generateMoves(
				NULL,
				oppKingIdx,
				figType,
				iter->ownBoard,	//< own to handle edge case when hitting
				iter->oppBoard
			);
			iter->oppKingBlocks[figType] = mm.okMoves;

		}
	}

	// PART 3: Find and pre-calc moves for pinned figures
	uint64_t kingMask = iter->board->board[KING];
	uint64_t whiteMask = iter->board->board[WHITE];
	uint64_t blackMask = iter->board->board[BLACK];
	int whiteKing = findHighest1(kingMask & whiteMask);
	int blackKing = findHighest1(kingMask & blackMask);

	uint64_t queenMask  = iter->board->board[QUEEN];
	uint64_t bishopMask = iter->board->board[BISHOP];
	uint64_t rookMask   = iter->board->board[ROOK];
	uint64_t diagonalSliderMask = queenMask | bishopMask;
	uint64_t straightSliderMask = queenMask | rookMask;

	// b->w check pins
	if (whiteKing >= 0) {
		// Straights
		for (ray_t d = 0; d < 4; d++) {
			uint64_t attackers = straightSliderMask & blackMask;
			dirPinCheck(iter, attackers, whiteKing, straightRays[d]);
		}
		// Diagonals
		for (ray_t d = 0; d < 4; d++) {
			uint64_t attackers = diagonalSliderMask & blackMask;
			dirPinCheck(iter, attackers, whiteKing, diagonalRays[d]);
		}
	}
	// w->b check pins
	if (blackKing >= 0) {
		// Straights
		for (ray_t d = 0; d < 4; d++) {
			uint64_t attackers = straightSliderMask & whiteMask;
			dirPinCheck(iter, attackers, blackKing, straightRays[d]);
		}
		// Diagonals
		for (ray_t d = 0; d < 4; d++) {
			uint64_t attackers = diagonalSliderMask & whiteMask;
			dirPinCheck(iter, attackers, blackKing, diagonalRays[d]);
		}
	}

	//for (int i = 0; i < 64; i++) {
	//	if (iter->pinMoves[i] != 0xFFFFFFFFFFFFFFFF) {
	//		debugln("Pinned figure:");
	//		printMask(iter->pinMoves[i]);
	//	}
	//}

}

void moveGen_init(moveIterator* iter, board* board) {

	// Reset all values
	memset(posFigTypeCache, NONE, sizeof(posFigTypeCache));

	// Omitted for performance
	//memset(iter, 0, sizeof(moveIterator));

	iter->hasActive = 0;

	iter->okMoves = 0;
	iter->hitMoves = 0;

	iter->figIdx = INVALID_MOVE_INDEX;

	int ownBoardIdx = (board->player == WHITE_PLAYER) ? WHITE : BLACK;
	int oppBoardIdx = (board->player == WHITE_PLAYER) ? BLACK : WHITE;

	iter->ownBoard = board->board[ownBoardIdx];
	iter->oppBoard = board->board[oppBoardIdx];
	iter->kingBoard = board->board[KING];

	iter->figuresToCheck = iter->ownBoard;
	iter->figTypeCheckOrderIdx = 0;

	iter->board = board;

	checkCheck_init(iter);

}

bool moveGen_isProtected(moveIterator* iter, int figIdx) {

	// A figure is protected if it's 'hittable' by it's own figures.

	// First, let's get our figure boards
	uint64_t kingMask   = iter->ownBoard & iter->board->board[KING];
	uint64_t queenMask  = iter->ownBoard & iter->board->board[QUEEN];
	uint64_t bishopMask = iter->ownBoard & iter->board->board[BISHOP];
	uint64_t rookMask   = iter->ownBoard & iter->board->board[ROOK];
	uint64_t knightMask = iter->ownBoard & iter->board->board[KNIGHT];

	// We can group the sliders for later
	uint64_t diagonalSliderMask = queenMask | bishopMask;
	uint64_t straightSliderMask = queenMask | rookMask;

	// Now, what we do for each figure type is that we pretend it's where the
	// figure we're checking. Then we can easily see if that overlaps with
	// where any of those figures are. If yes, the field we're checking is
	// reachable by them.

	// For kings and knights this is easy, because they can't be blocked.
	// Therefore, we check them first so we can maybe avoid calculating our
	// slider moves.
	uint64_t knightMoves = FIGMOVES[figIdx][KNIGHT];
	if (knightMoves & knightMask)
		return TRUE;

	uint64_t kingMoves = FIGMOVES[figIdx][KING];
	if (kingMoves & kingMask)
		return TRUE;

	// Now for the sliders, we bundle straights and diagonals so we can save
	// the queen check. I.e. if a queen is in either the diagonal or the
	// straight rays, it can reach the field.
	uint64_t straightMoves = FIGMOVES[figIdx][ROOK];
	if (straightMoves & straightSliderMask) {

		// For performance, we check whether there even is a relevant figure in
		// the moveMask at all, but now we need to see if that slider might be
		// blocked.
		moveMasks_t mmStraight = generateMoves(
			NULL,
			figIdx,
			ROOK,
			iter->oppBoard, //< Reversed because we want to include fields with
			iter->ownBoard	//< own figures and exclude ones with opponents
		);

		if (mmStraight.okMoves & straightSliderMask)
			return TRUE;
	}

	uint64_t diagonalMoves = FIGMOVES[figIdx][BISHOP];
	if (diagonalMoves & diagonalSliderMask) {

		// For performance, we check whether there even is a relevant figure in
		// the moveMask at all, but now we need to see if that slider might be
		// blocked.
		moveMasks_t mmDiagonal = generateMoves(
			NULL,
			figIdx,
			BISHOP,
			iter->oppBoard, //< Reversed because we want to include fields with
			iter->ownBoard	//< own figures and exclude ones with opponents
		);

		if (mmDiagonal.okMoves & diagonalSliderMask)
			return TRUE;
	}

	// If none of these can reach the field, the field is unprotected.

	return FALSE;

}
