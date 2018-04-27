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
#define DEBUG

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
    size_t length;
    JsonValue* elements;
} JsonArray;

// Node in the prefix trie that allows
typedef struct JsonNode
{
    struct JsonNode* sibling;
    struct JsonNode* child;
    JsonValue* data;
    unsigned char letter;
} JsonNode;

typedef struct JsonObject
{
    JsonNode node;
} JsonObject;

void Json_set_mempool(void * start, void * end);

JsonObject* create_JsonObject(void);
void free_JsonObject(JsonObject *obj);
JsonValue get_value(JsonObject * obj, char * key);
int set_value(JsonObject * obj, char * key, void* data, JsonDataType type);

JsonArray * create_JsonArray(size_t length);
int set_element(JsonArray * j, size_t index, void * data, JsonDataType type);
JsonValue get_element(JsonArray * j, size_t index);



#endif
