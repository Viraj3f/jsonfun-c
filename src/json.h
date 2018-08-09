//
//  json.h
//  JSON
//
//  Created by Viraj Bangari on 2018-04-24.
//  Copyright © 2018 Viraj. All rights reserved.
//

#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    JSON_NULL,
    JSON_STRING,
    JSON_BOOL,
    JSON_INT,
    JSON_FLOAT,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_ERROR
} JsonDataType;

typedef enum
{
    INVALID_TYPE = -1,
    MISSING_KEY = -2,
    INDEX_OUT_OF_BOUNDS = -3
} JsonError;


typedef struct JsonValue
{
    JsonDataType type;
    union
    {
        void * n;
        char* s;
        bool b;
        int i;
        float f;
        struct JsonObject* o;
        struct JsonArray * a;
        JsonError e;
    } data;
} JsonValue;

typedef struct JsonArray {
    int16_t length;
    int16_t elements;
} JsonArray;

// Node in the prefix trie that allows
typedef struct JsonNode
{
    int16_t child;
    int16_t sibling;
    int16_t data;
    unsigned char letter;
} JsonNode;

typedef struct JsonObject
{
    JsonNode node;
} JsonObject;

// Sets the beginning and end of the memory allocate for the JSON object
void Json_set_mempool(void * start, void * end);

// Functions for creating json objects
JsonObject* create_JsonObject(void);
JsonValue get_value(JsonObject * obj, char * key);
int set_value(JsonObject * obj, char * key, void* data, JsonDataType type);

// Function for creating json arrays
JsonArray * create_JsonArray(int16_t length);
int set_element(JsonArray * j, int16_t index, void * data, JsonDataType type);
JsonValue get_element(JsonArray * j, int16_t index);

size_t dump_JsonObject(JsonObject *o, char* destination);

bool parse_JsonObject(char* input, JsonObject** parsed);

#endif

