#pragma once

#include <stdint.h>

#include "util.h"
#include "rays.h"

#define MAX_DIRECTIONS 8

typedef struct {
	ray_t directions[MAX_DIRECTIONS];
	int directionCount;
	bool_t slider;
} figureData_t;

struct figures_s {
	figureData_t queen;
	figureData_t king;
	figureData_t bishop;
	figureData_t knight;
	figureData_t rook;
};

#define FIGURE_TYPE_COUNT (sizeof(struct figures_s)/sizeof(figureData_t))
_Static_assert(FIGURE_TYPE_COUNT == 5, "Should be 5 figures long");

typedef union {
	struct figures_s asStruct;
	figureData_t asMap[FIGURE_TYPE_COUNT];
} figureUnion_t;

const figureUnion_t figureUnion = {
	.asStruct = {
		.queen = {
			.directions = { R_N, R_S, R_E, R_W, R_NW, R_NE, R_SW, R_SE },
			.directionCount = 8,
			.slider = TRUE,
		},
		.king = {
			.directions = {},
			.directionCount = 0,
			.slider = FALSE,
		},
		.bishop = {
			.directions = { R_NW, R_NE, R_SW, R_SE },
			.directionCount = 4,
			.slider = TRUE,
		},
		.knight = {
			.directions = {},
			.directionCount = 0,
			.slider = FALSE,
		},
		.rook = {
			.directions = { R_N, R_S, R_E, R_W },
			.directionCount = 4,
			.slider = TRUE,
		}
	}
};

const figureData_t* figureData = figureUnion.asMap;
