//
//  json.c
//  JSON
//
//  Created by Viraj Bangari on 2018-04-24.
//  Copyright © 2018 Viraj. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdalign.h>
#include <stdint.h>

#include "json.h"


typedef struct Mempool
{
    int8_t * start;
    int8_t * end;
    int8_t * top;
} Mempool;

Mempool buffer = { .end=NULL, .top=NULL };

void Json_set_mempool(void * start, void * end)
{
    buffer.start = start;
    buffer.top = start;
    buffer.end = end;
}

void * _json_alloc(size_t size, size_t alignment) 
{
    // If the mempool has not been set, then just use malloc/free
    if (!buffer.end)
    {
        return malloc(size);
    }

    // Alignment
    // Check to see if the current top is aligned.
    int padding = 0;
    int remainder = (size_t) buffer.top % alignment;
    if (remainder != 0)
    {
        padding = alignment - remainder;
        buffer.top += padding;
    }

    void * loc = (void *) buffer.top;
    buffer.top += size;

    if (buffer.top >= buffer.end)
    {
        printf("Out of memory!\n");
        loc = NULL;
    }

    #ifdef DEBUG
    printf("Requested: %lu ", size);
    printf("Alignment: %lu ", alignment);
    printf("Padding: %d ", padding);
    printf("Free bytes: %lu ", (size_t)(buffer.end - buffer.top + 1));
    printf("Top: %p\n", (void *)buffer.top);
    #endif

    return loc;
}

const unsigned char DEFAULT_LETTER = 0x80;
const u_int16_t DEFAULT_OBJECT_ADDRESS = 0;
void _set_default_JsonNode(JsonNode* node)
{
    // 1000 0000
    // Since Ascii characters are only 7 bits long, the 
    // first bit can act as a "flag" when a node is unitialized.
    node->sibling = DEFAULT_OBJECT_ADDRESS;
    node->child = DEFAULT_OBJECT_ADDRESS;
    node->data = DEFAULT_OBJECT_ADDRESS;
    node->letter = DEFAULT_LETTER; // 1000 0000.
}

// Creates an empty JSON object, equivalent of {}
JsonObject* create_JsonObject()
{
    JsonNode node;
    _set_default_JsonNode(&node);

    JsonObject* obj = _json_alloc(sizeof(JsonObject), alignof(JsonObject));
    obj->node = node;

    return obj;
}


JsonValue get_value(JsonObject * obj, char * key)
{
    JsonNode * node = &(obj->node);

    if (!(*key))
    {
        while (*key != node->letter)
        {
            node = (JsonNode*)(buffer.start + node->sibling);
        }
    }

    while (*key)
    {
        while (*key != node->letter)
        {
            node = (JsonNode*)(buffer.start + node->sibling);
        }

        node = (JsonNode*)(buffer.start + node->child);
        key++;
    }

    return *((JsonValue*)(buffer.start + node->data));
}

int _alloc_JsonElement(JsonValue * jd, void * data)
{
    switch (jd->type)
    {
        case JSON_NULL:
            jd->data.n = NULL;
            break;
        case JSON_STRING:
        {
            char * destination = _json_alloc(strlen((char *) data) + 1, alignof(char));
            strcpy(destination, (char *) data);
            jd->data.s = destination;
            break;
        }
        case JSON_BOOL:
            jd->data.b = ((bool *) data);
            break;
        case JSON_INT:
            jd->data.i = *((int *) data);
            break;
        case JSON_FLOAT:
            jd->data.f = *((float *) data);
            break;
        case JSON_OBJECT:
            jd->data.o = (JsonObject *) data;
            break;
        case JSON_ARRAY:
        {
            jd->data.a = (JsonArray *) data;
            break;
        }
        default:
            return INVALID_TYPE; 
    }

    return 0;
}

int _set_value(JsonObject * obj, char * key, JsonValue* value)
{
    JsonNode * node = &(obj->node);

    // Check if the JSON node is set to its default values. If that is the case,
    // we can save an extra allocation by chaning the default value's key rather
    // than by creating a sibling.
    if (node->letter & DEFAULT_LETTER)
    {
        _set_default_JsonNode(node);
        node->letter = *key;
    }

    if (!(*key))
    {
        while (*key != node->letter)
        {
            if (!node->sibling)
            {
                JsonNode * sibling = _json_alloc(sizeof(JsonNode), alignof(JsonNode));
                _set_default_JsonNode(sibling);
                sibling->letter = *key;
                node->sibling = ((int8_t *) sibling - buffer.start);
            }
            node = (JsonNode*)(buffer.start + node->sibling);
        }
    }

    // If passed in an empty string, go directly to setting the object's value.
    while (*key)
    {
        // Otherwise traverse sideways until a matching letter is found
        while (*key != node->letter)
        {
            if (!node->sibling)
            {
                JsonNode * sibling = _json_alloc(sizeof(JsonNode), alignof(JsonNode));
                _set_default_JsonNode(sibling);
                sibling->letter = *key;
                node->sibling = ((int8_t *) sibling - buffer.start);
            }
            node = (JsonNode*)(buffer.start + node->sibling);
        }

        // Check if the next character is null terminating.
        // If it is, then the current node will be the one to which data is
        // added to. If node, then traverse one node down, creating a new node
        // if it does not already exist.
        key++;
        if (!node->child)
        {
            JsonNode * child = _json_alloc(sizeof(JsonNode), alignof(JsonNode));
            _set_default_JsonNode(child);
            child->letter = *key;
            node->child = ((int8_t *) child - buffer.start);
        }
        node = (JsonNode*)(buffer.start + node->child);
    }

    node->data = ((int8_t *) value - buffer.start);

    return 0;
}

int set_value(JsonObject * obj, char * key, void* data, JsonDataType type)
{
    JsonValue* jd = _json_alloc(sizeof(JsonValue), alignof(JsonValue));
    jd->type = type;

    int status = _alloc_JsonElement(jd, data);

    if (status < 0)
    {
        return status;
    }

    _set_value(obj, key, jd);
    return 0;
}


JsonArray * create_JsonArray(int16_t length)
{
    JsonArray* j = _json_alloc(sizeof(JsonArray), alignof(JsonArray));
    j->length = length;

    JsonValue * elements = _json_alloc(sizeof(JsonValue) * length, alignof(JsonArray));
    j->elements = ((int8_t *) elements - buffer.start);
    return j;
}

int set_element(JsonArray * j, int16_t index, void * data, JsonDataType type)
{
    JsonValue *jd = &(((JsonValue*)(buffer.start + j->elements))[index]);
    jd->type = type;
    int status = _alloc_JsonElement(jd, data);

    return status;
}

JsonValue get_element(JsonArray * j, int16_t index)
{
    if (index > j->length)
    {
        return (JsonValue) {
            .type=JSON_ERROR, 
            .data.e=INDEX_OUT_OF_BOUNDS
        };
    }
    return ((JsonValue*)(buffer.start + j->elements))[index];
}

typedef struct _Stack
{
    void* stack[128];
    int stacktop;
} _Stack;

int push_ptr(_Stack* s, void* p)
{
    if (s->stacktop >= 127)
    {
        return -1;
    }

    (s->stacktop)++;
    s->stack[s->stacktop] = p;

    return 0;
}

int push_int(_Stack* s, int i)
{
    if (s->stacktop >= 127)
    {
        return -1;
    }

    s->stacktop += sizeof(int);
    int* p = (int*) &(s->stack[s->stacktop]);
    *p = i;

    return 0;
}

void* pop_ptr(_Stack* s)
{
    void *rval = s->stack[s->stacktop];
    (s->stacktop)--;

    return rval;
}

int pop_int(_Stack* s)
{
    int rval = (int) s->stack[s->stacktop];
    s->stacktop -= sizeof(int);

    return rval;
}

int peek_int(_Stack* s)
{
    int rval = (int) s->stack[s->stacktop];
    return rval;
}

char * _JSON_NULL_STR = "null";
char * _JSON_FALSE_STR = "false";
char * _JSON_TRUE_STR = "true";

char* _dump_JsonObject_Key(char* buffer, int bufStart, int bufEnd, char* destination)
{
    *(destination++) = '"'; 
    for (int i = bufStart; i <= bufEnd; i++)
    {
        *(destination++) = buffer[i];
    }
    *(destination++) = '"'; 
    *(destination++) = ':'; 

    return destination;

}

char * 
_dump_JsonValue(
    JsonValue* value, 
    char * destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack)
{
    switch (value->type)
    {
        char *str;
        case JSON_NULL:
            str = _JSON_NULL_STR;
            while (*str) *(destination++) = *(str++);
            break;
        case JSON_STRING:
        {
            str = value->data.s;
            *(destination++) = '"';
            while (*str) *(destination++) = *(str++);
            *(destination++) = '"';
            break;
        }
        case JSON_BOOL:
            str = value->data.b ? _JSON_TRUE_STR : _JSON_FALSE_STR;
            while (*str) *(destination++) = *(str++);
            break;
        case JSON_INT:
        {
            int len = sprintf(destination, "%d", value->data.i);
            destination += len;
            break;
        }
        case JSON_FLOAT:
        {
            int len = sprintf(destination, "%g", value->data.f);
            destination += len;
            break;
        }
        case JSON_OBJECT:
            push_int(objIndex_stack, valstack->stacktop);
            push_ptr(valstack, &(value->data.o->node));
            push_int(bufend_stack, 0);
            *(destination++) = '{';
            break;
        case JSON_ARRAY:
            break;
        default:
            break;
    }
    return destination;
}

char*
_dump_JsonObject(
    JsonObject *o,
    char* destination)
{
    char key_buffer[256];
    _Stack valstack, bufend_stack, objIndex_stack;
    valstack.stacktop = -1;
    bufend_stack.stacktop = -1;
    objIndex_stack.stacktop = -1;

    push_int(&objIndex_stack, valstack.stacktop);
    push_ptr(&valstack, &(o->node));
    push_int(&bufend_stack, 0);

    *(destination++) = '{';
    while (valstack.stacktop >= 0)
    {
        JsonNode* node = pop_ptr(&valstack);
        int strIndex  = pop_int(&bufend_stack);  // This will update strIndex
        key_buffer[strIndex] = node->letter;  // Add the current key to the buffer

        // Add sibling to stack if exists
        if (node->sibling != DEFAULT_OBJECT_ADDRESS)
        {
            JsonNode* sibling = (JsonNode*)(buffer.start + node->sibling); 
            push_ptr(&valstack, sibling);
            push_int(&bufend_stack, strIndex);
        }

        // Add child to stack if exists
        if (node->child != DEFAULT_OBJECT_ADDRESS)
        {
            JsonNode* child = (JsonNode*)(buffer.start + node->child); 
            push_ptr(&valstack, child);
            push_int(&bufend_stack, strIndex + 1);
        }

        // Print current node if it's not empty
        if (node->data != DEFAULT_OBJECT_ADDRESS)
        {
            if (*(destination - 1) != '{')
            {
                *(destination++) = ',';
            }

            destination = _dump_JsonObject_Key(key_buffer, 0, strIndex - 1, destination);
            JsonValue* value = (JsonValue*)(buffer.start + node->data);
            destination = _dump_JsonValue(value, destination, &valstack, &bufend_stack, &objIndex_stack);
        }

        if (valstack.stacktop == peek_int(&objIndex_stack))
        {
            *(destination++) = '}';
            pop_int(&objIndex_stack);
        }
    }

    // Check this at the end, since nested objects will not check for the '}'
    while (objIndex_stack.stacktop >= 0)
    {
        *(destination++) = '}';
        pop_int(&objIndex_stack);
    }

    return destination;
}

size_t dump_JsonObject(JsonObject* o, char* destination)
{
    char* end = _dump_JsonObject(o, destination);
    *end = '\0';
    
    return end - destination;
}
