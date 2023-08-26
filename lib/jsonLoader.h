#ifndef JSONLOADER_H_
#define JSONLOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define JSMN_HEADER
#include "../lib/jsmn/jsmn.h"

#define TOK_LEN(t) (t.end-t.start)

struct json {
    jsmntok_t* tokens;
    size_t tokens_len;
    jsmn_parser* parser;
    char* string;
    size_t string_len;
}typedef json;

json* load_json(char* path, size_t tokens);
void freeJson(json* object);
char* getString(jsmntok_t* tok, char* string);
char* getStringBuffer(jsmntok_t* tok, char* string, char* buffer);

extern int PRINT_JSON_FILE_OPENED;
#endif