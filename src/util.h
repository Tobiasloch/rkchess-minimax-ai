#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef _DEBUG
	#define DEBUG 1
#else
	#define DEBUG 0
#endif


int verbosity_level;

#define debugf(fmt, ...) \
	if ( DEBUG ) { \
		fprintf(stderr, "\e[93m[DEBUG %s:%d:%s()]\e[39m " fmt, \
			__FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	}

#define debugln(fmt, ...) debugf(fmt "\n", ##__VA_ARGS__)

#define infofr(lvl, fmt, ...) \
	if ( lvl <= verbosity_level ) { \
		printf(fmt, ##__VA_ARGS__); \
	}

#define infof(lvl, fmt, ...) \
	infofr(lvl, "\e[94m[INFO%d]\e[39m " fmt, lvl, ##__VA_ARGS__)

#define infoln(lvl, fmt, ...) infof(lvl, fmt "\n", ##__VA_ARGS__)

// do-whiles are there to ensure code block expression and parsing
#define errorf(fmt, ...) \
	do { \
		fprintf(stderr, "\e[91m[ERROR]\e[39m " fmt, ##__VA_ARGS__); \
	} while (0)

#define errorln(fmt, ...) errorf(fmt "\n", ##__VA_ARGS__)

#define perrorf(fmt, ...) \
	do { \
		errorf(fmt ": PError message", ##__VA_ARGS__); \
		perror(""); \
	} while (0)


typedef enum {
	FALSE = 0,
	TRUE = 1
} bool;


static inline void* ecalloc(unsigned int num, size_t size) {

	void* ptr = calloc(num, size);
	if ( ptr == NULL )
		perrorf("Failed to allocate Memory");
	return ptr;

}

double get_time();
