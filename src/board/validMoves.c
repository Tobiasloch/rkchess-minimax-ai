#include "validMoves.h"

//#if MOVEGEN == MG_OLD

uint64_t change,black,white,start,end,tmp_board;
int valid = 1;
uint64_t allowed[8];
move ret;

const int KingMove[8];
const int KnightMove[8];
const int RookMove[4];
const int QueenMove[8];
const int BishopMove[4];

const int KingMoveX[8] = {1,0,-1,-1,-1, 0, 1,1};
const int KingMoveY[8] = {1,1, 1, 0,-1,-1,-1,0};
const int QueenMoveX[8] = {1,0,-1,-1,-1, 0, 1,1};
const int QueenMoveY[8] = {1,1, 1, 0,-1,-1,-1,0};
const int KnightMoveX[8] = {2,1,-1,-2,-2,-1, 1, 2};
const int KnightMoveY[8] = {1,2, 2, 1,-1,-2,-2,-1};
const int RookMoveX[4] = {0,-1, 0,1};
const int RookMoveY[4] = {1, 0,-1,0};
const int BishopMoveX[4] = {1,-1,-1, 1};
const int BishopMoveY[4] = {1, 1,-1,-1};

const int KingConstants[2] = {sizeof(KingMove)/sizeof(KingMove[0]),1};
const int QueenConstants[2] = {sizeof(QueenMove)/sizeof(QueenMove[0]),7};
const int KnightConstants[2] = {sizeof(KnightMove)/sizeof(KnightMove[0]),1};
const int RookConstants[2] = {sizeof(RookMove)/sizeof(RookMove[0]),7};
const int BishopConstants[2] = {sizeof(BishopMove)/sizeof(BishopMove[0]),7};

const uint64_t straight[4] = {0xff00000000000000,0x101010101010101,0xff,8080808080808080};
const uint64_t diagonal[4] = {0xff01010101010101,0x1010101010101ff,0x80808080808080ff,0xff80808080808080};
const uint64_t endFields = 0xff818181818181ff;

const uint64_t forbiddenKnight[8] = {0xffff010101010101, 0xff03030303030303,0x3030303030303ff,0x10101010101ffff,0xffff808080808080,0xc0c0c0c0c0c0c0ff,0xffc0c0c0c0c0c0c0,0xffff808080808080};

#ifdef BENCHMARK
long long movesCount = 0;
#endif

int kingCanMoveForward(board* board, int player) {
	uint64_t characterBoard = board->board[4+player];
	if (characterBoard == 0) return 0;

	int from = bitboardIndexOf(characterBoard);

	uint64_t mask = 7; // bitmask for 3 1s, that can be shifted in front of the character
	if (from %8 == 0) mask = 6; // if the character is on the right edge the bitmask should be 110
	else if (from+1 % 8 == 0) mask = 3; // if on the left edge. mask=011
	mask = mask << (from+8);

	int otherPlayer = (board->player%2)+1;
	uint64_t field = board->board[4+board->player] | (board->board[4+otherPlayer]&board->board[KING]);

	return ! (field&mask);
}

void initMoveIterator(moveIterator* iterator, board* board) {
    memset(iterator, 0, sizeof(struct moveIterator)); // sets all values to 0
    iterator->board = board;
	iterator->active.from = INVALID_MOVE_INDEX;
	iterator->active.to = INVALID_MOVE_INDEX;
	iterator -> hasActive = 1;
	iterator -> moveNum = 1;
}

/**
 * @param iterator iterator object
 * @param characterOffsets index offset for figure
 *
*/
void iterateCharacter(moveIterator* iterator, const int characterOffsetX[], const int characterOffsetY[], const int characterConstants[], uint64_t characterColor) {
	// Zero check cause undefind behaviour in __builtin_clzll
	if (iterator->characterBoard == 0) {
		iterator->hasActive++;
		iterator->moveNum = 1;
		iterator->active.from = INVALID_MOVE_INDEX;
		return;
	}

	int from = bitboardIndexOf(iterator->characterBoard);
	int fromX = from%8;
	int fromY = from/8;

	// wenn alle richtungen eines characters abgearbeitet sind
	if (iterator->direction >= characterConstants[0]) {
		iterator->direction = 0;
		iterator->moveNum = 1;
		iterator->characterBoard ^= ((uint64_t)1)<<from; // remove character from active board
		if (iterator->characterBoard == 0) iterator->hasActive++;
		iterator->active.from = INVALID_MOVE_INDEX;
		return;
	}

	int toX = fromX + (characterOffsetX[iterator->direction]*iterator->moveNum);
	int toY = fromY + (characterOffsetY[iterator->direction]*iterator->moveNum);

	int to = (toY*8)+toX;
	uint64_t toMask = ((uint64_t)1)<<to;
	iterator->moveNum++;
	uint64_t enemyBoard = (iterator->board->board[WHITE] | iterator->board->board[BLACK]) ^ characterColor;

	// wenn to ein eigener character ist oder außerhalb des feldes, dann ist der move invalid und es wird die nächste position gecheckt
	if ((toMask & characterColor) || toX < 0 || toX > 7 || toY < 0 || toY > 7 || iterator->moveNum-1 > characterConstants[1]) {
		iterator->direction++;
		iterator->moveNum = 1;
		to = INVALID_MOVE_INDEX;
	} else if ((toMask & enemyBoard)) {
		// wenn to ein gegner ist, dann ist der move valid, aber die richtung sollte nicht weiter untersucht werden
		iterator->direction++;
		iterator->moveNum = 1;
	}

	iterator->active.from = from;
	iterator->active.to = to;
}


void iterateMoves(moveIterator* iterator) {
	iterator->active.to = INVALID_MOVE_INDEX;

	while(iterator->hasActive && moveIsInvalid(iterator->active)) {
		uint64_t characterColor = iterator->board->board[4+iterator->board->player];

		switch(iterator -> hasActive){
			case 1: //King
				if (!iterator->characterBoard) iterator->characterBoard = iterator->board->board[KING]&characterColor;
				iterateCharacter(iterator, KingMoveX, KingMoveY, KingConstants, characterColor);
				break;

			case 2: //Knight
				if (!iterator->characterBoard) iterator->characterBoard = iterator->board->board[KNIGHT]&characterColor;
				iterateCharacter(iterator, KnightMoveX, KnightMoveY, KnightConstants, characterColor);
				break;

			case 3: //Rook
				if (!iterator->characterBoard) iterator->characterBoard = iterator->board->board[ROOK]&characterColor;
				iterateCharacter(iterator, RookMoveX, RookMoveY, RookConstants, characterColor);
				break;

			case 4: //Bishop
				if (!iterator->characterBoard) iterator->characterBoard = iterator->board->board[BISHOP]&characterColor;
				iterateCharacter(iterator, BishopMoveX, BishopMoveY, BishopConstants, characterColor);
				break;

			case 5: //Queen
				if (!iterator->characterBoard) iterator->characterBoard = iterator->board->board[QUEEN]&characterColor;
				iterateCharacter(iterator, QueenMoveX, QueenMoveY, QueenConstants, characterColor);
				break;
			default :
				iterator-> hasActive = 0;
				break;
		}
	}
}



// TODO TESTING

//#endif
