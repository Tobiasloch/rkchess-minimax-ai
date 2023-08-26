#include "jsonLoader.h"


int PRINT_JSON_FILE_OPENED = 1;

json* load_json(char* path, size_t tokens) {
    if (PRINT_JSON_FILE_OPENED) printf("reading data from json %s\n", path);

    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 

    char *string = malloc(fsize + 1);
    size_t bytesRead = fread(string, 1, fsize, f);
    if (bytesRead != fsize) {
        perror("fread could not read the whole file!");
    }

    fclose(f);

    string[fsize] = 0;

    jsmn_parser* parser = malloc(sizeof(jsmn_parser));

    jsmn_init(parser);
    jsmntok_t* t = malloc(tokens * sizeof(jsmntok_t));

    int r = jsmn_parse(parser, string, fsize, t,
                 tokens);

    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        return NULL;
    }

    json* object = malloc(sizeof(json));
    object->parser = parser;
    object->string = string;
    object->string_len = fsize;
    object->tokens = t;
    object->tokens_len = r;

    /*int len = r;
    for (int i = 0; i < len; i++) {
        int type = t[i].type;
        int start = t[i].start;
        int end = t[i].end;

        printf("type:%d start:%d end:%d\n", type, start, end);
        fwrite(&string[start], sizeof(char), end-start, stdout);
        printf("\n");
    }*/

    return object;
}

char* getString(jsmntok_t* tok, char* string) {
    int len = tok->end-tok->start;
    char* str = malloc(sizeof(char)*(len+1));
    strncpy(str, &string[tok->start], len);
    str[len] = 0;
    return str;
}

char* getStringBuffer(jsmntok_t* tok, char* string, char* buffer) {
    int len = tok->end-tok->start;
    char* str = buffer;
    strncpy(str, &string[tok->start], len);
    str[len] = 0;
    return str;
}

void freeJson(json* object) {
    free(object->parser);
    free(object->tokens);
    free(object->string);
    free(object);
}