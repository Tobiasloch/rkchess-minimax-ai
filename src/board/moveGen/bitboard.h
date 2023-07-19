#pragma once

#include <stdio.h>
#include <stdint.h>

#define BOARD_SIDE_LENGTH 8

typedef struct {
	int8_t x;
	int8_t y;
} vector_t;

static inline vector_t idx2vec(uint64_t idx) {
	return (vector_t) {
		.x = idx % BOARD_SIDE_LENGTH,
		.y = idx / BOARD_SIDE_LENGTH
	};
}

static inline uint64_t vec2idx(vector_t vec) {
	return ( vec.y * BOARD_SIDE_LENGTH + vec.x );
}

static inline void _printPos(int idx) {
	vector_t vec = idx2vec(idx);
	printf("%c%c", 'h' - vec.x, '1' + vec.y);
}

static inline void printPos(int idx) {
	_printPos(idx);
	printf("\n");
}

static inline void printMove(int idx1, int idx2) {
	_printPos(idx1);
	_printPos(idx2);
	printf("\n");
}

// Check at compile time that clzll can actually take our 64-bit int
_Static_assert(sizeof(uint64_t) == sizeof(long long), "the type long long has to be greater or equals than uint64_t");

static inline int countSetBits(uint64_t mask) {
	// To count all set bits, we use the popcnt instruction
	return __builtin_popcountll(mask);
}

static inline int findHighest1(uint64_t mask) {
	// Prevent undefined behavior
	if (mask == 0) return -1;
	// clz finds the number of leading zero-bits in a value
	// subtract from 63 to get board index
	return ( 63 - __builtin_clzll(mask) );
}

static inline int findLowest1(uint64_t mask) {
	// Prevent undefined behavior
	if (mask == 0) return -1;
	// ctz finds the number of trailing zero-bits in a value
	// this is exactly the figure index
	return ( 0 + __builtin_ctzll(mask) );
}

typedef struct { int num; int idxs[2]; } figureIndices_t;
static inline figureIndices_t figBoard2idxs(uint64_t board) {

	// Handle case that no such figures exist (anymore)
	if (board == 0)
		return (figureIndices_t) { .num = 0, .idxs = { -1, -1 } };

	int f = findHighest1(board);
	int l = findLowest1(board);

	// How many *different* figures did we find?
	int num = (f != l) ? 2 : 1;

	return (figureIndices_t) { .num = num, .idxs = { f, l } };

}

static inline int findClosest1(uint64_t mask, int forward) {

	// If this ray goes forward in the board index order,
	// we're looking fo the lowest one. If it points
	// rearward in the board index order, we look for the
	// highest one.
	return (forward) ? findLowest1(mask) : findHighest1(mask);

}

static void printMask(uint64_t mask) __attribute__((unused));
static void printMask(uint64_t mask) {

	printf("      a   b   c   d   e   f   g   h\n");
	printf("    ╔═ 0x%016llx ══╤═══╤═══╗\n", (unsigned long long) mask);

	for (int y = BOARD_SIDE_LENGTH-1; y >= 0; y--) {

		printf("  %d ║", y+1);

		for (int x = BOARD_SIDE_LENGTH-1; x >= 0; x--) {

			int i = vec2idx((vector_t) { .x = x, .y = y });
			char c = (mask & ((uint64_t) 1 << i)) ? 'X' : ' ';
			printf(" %c %s", c, (x > 0) ? "│" : "║");
			if (x == 0)
				printf(" %d\n", y+1);

		}

		if (y > 0)
			printf("    ╟───┼───┼───┼───┼───┼───┼───┼───╢\n");

	}

	printf("    ╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝\n");
	printf("      a   b   c   d   e   f   g   h\n");

}
