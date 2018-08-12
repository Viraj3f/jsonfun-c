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
        float f;
        struct JsonObject* o;
        struct JsonArray * a;
        JsonError e;
    } data;
} JsonValue;

typedef struct JsonArray {
    u_int16_t length;
    u_int16_t elements;
} JsonArray;

typedef struct JsonNode
{
    u_int16_t child;
    u_int16_t sibling;
    u_int16_t data;
    unsigned char letter;
} JsonNode;

typedef struct JsonObject
{
    JsonNode node;
} JsonObject;

// Sets the beginning and end of the memory allocate for the JSON object
void Json_set_mempool(void * start, size_t size);

// Resets the mempool, allowing it to be fully used again.
void Json_reset_mempool();

// Functions for creating json objects
JsonObject * create_JsonObject(void);
JsonValue get_value(JsonObject * obj, char * key);
bool set_value_null(JsonObject * obj, char * key);
bool set_value_string(JsonObject * obj, char * key, char * str);
bool set_value_bool(JsonObject * obj, char * key, bool data);
bool set_value_float(JsonObject * obj, char * key, float data);
bool set_value_object(JsonObject * obj, char * key, JsonObject * object);
bool set_value_array(JsonObject * obj, char * key, JsonArray * array);

// Function for creating json arrays
JsonArray * create_JsonArray(u_int16_t length);
JsonValue get_element(JsonArray * j, u_int16_t index);
bool set_element_null(JsonArray * j, u_int16_t index);
bool set_element_string(JsonArray * j, u_int16_t index, char * str);
bool set_element_bool(JsonArray * j, u_int16_t index, bool data);
bool set_element_float(JsonArray * j, u_int16_t index, float data);
bool set_element_object(JsonArray * j, u_int16_t index, JsonObject * object);
bool set_element_array(JsonArray * j, u_int16_t index, JsonArray * array);

// For dumping and parsing
bool parse_JsonObject(char* input, JsonObject** parsed);
size_t dump_JsonObject(JsonObject *o, char* destination);

#endif

