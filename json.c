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

#include "json.h"

#define DEBUG

// Not ultra-reliable, but should work in most cases...
const int CPU_WORDSIZE = sizeof(size_t);

typedef struct Mempool
{
    char * end;
    char * top;
} Mempool;

Mempool buffer = { .end=NULL, .top=NULL };

void Json_set_mempool(void * start, void * end)
{
    buffer.top = start;
    buffer.end = end;
}


void * _json_alloc(size_t size) 
{
    // If the mempool has not been set, then just use malloc/free
    if (!buffer.end)
    {
        return malloc(size);
    }

    void * loc = (void *) buffer.top;
    buffer.top += size;

    // Align memory add:ess
    int padding = 0;

    #ifndef JSON_NO_PADDING
    int remainder = size % CPU_WORDSIZE;
    if (size >= CPU_WORDSIZE && remainder != 0)
    {
        padding = CPU_WORDSIZE - remainder;
        buffer.top += padding;
    }
    #endif

    #ifndef JSON_NO_MEMPOOL_BOUND_CHECK
    if (buffer.top >= buffer.end)
    {
        printf("Out of memory!\n");
        loc = NULL;
    }
    #endif

    #ifdef DEBUG
    printf("Requested: %lu ", size);
    printf("Padding: %lu ", (size_t)padding);
    printf("Free bytes: %lu ", (size_t)(buffer.end - buffer.top + 1));
    printf("Top: %p\n", (void *)buffer.top);
    #endif

    return loc;
}

void _json_free(void * p)
{
    if (!buffer.end)
    {
        free(p);
    }
}

/*
 * Creates an empty JSON object, equivalent of {}
 */
JsonObject* create_JsonObject()
{
    JsonNode node;
    node.sibling = NULL;
    node.child = NULL;
    node.data = NULL;
    node.letter = '\0';

    JsonObject* obj = _json_alloc(sizeof(JsonObject));
    obj->node = node;
    obj->isEmpty = true;

    return obj;
}

void _free_JsonValueData(JsonValue* value)
{
    switch (value->type)
    {
        case JSON_OBJECT:
        {
            JsonObject* obj = (JsonObject *)value->data.o;
            free_JsonObject(obj);
            break;
        }
        case JSON_STRING:
            _json_free(value->data.s);
            break;
        case JSON_ARRAY:
        {
            // Note, when freeing an array, we only need to free the values
            // that the array elements point to. The element itself is freed when the
            // entire array is freed. This is because JsonArray holds a series
            // of JsonElements sequentially in memory, not pointers to JsonElements.
            JsonValue* element = value->data.a->elements;
            size_t length = value->data.a->length;
            for (size_t i = 0; i < length; i++)
            {
                _free_JsonValueData(element);
                element++;
            }
            _json_free(value->data.a->elements);
            _json_free(value->data.a);
            break;
        }
        default:
            break;
    }
}

void _free_JsonValue(JsonValue* value)
{
    if (value != NULL)
    {
        _free_JsonValueData(value);
        _json_free(value);
    }
}

void _free_JsonNode(JsonNode * node)
{
    while (node != NULL)
    {
        if (node->child != NULL)
        {
            #ifdef DEBUG
            printf("%c", node->letter);
            #endif
            _free_JsonNode(node->child);
        }

        _free_JsonValue(node->data);

        JsonNode* temp = node;
        node = node->sibling;
        _json_free(temp);
    }
}

void free_JsonObject(JsonObject * obj)
{
    if (!buffer.end)
    {
        #ifdef DEBUG
            printf("%c", obj->node.letter);
        #endif
        _free_JsonValue(obj->node.data);
        _free_JsonNode(obj->node.child);
        _free_JsonNode(obj->node.sibling);
        _json_free(obj);
    }
}

JsonValue get_value(JsonObject * obj, char * key)
{
    JsonNode * node = &(obj->node);

    if (*key)
    {
        while (*key)
        {
            // TODO: Return an error if node == NULL
            while (*key != node->letter)
            {
                node = node->sibling;
            }

            JsonNode * child = node->child;
            node = child;
            key++;
        }
    }

    return *(node->data);
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
            char * destination = _json_alloc(strlen((char *) data) + 1);
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
            return -1; 
    }

    return 0;
}

int _set_value(JsonObject * obj, char * key, JsonValue* value)
{
    JsonNode * node = &(obj->node);

    if (obj->isEmpty)
    {
        node->letter = *key;
        obj->isEmpty = false;
    }

    // If passed in an empty string, go directly to setting the object's value.
    if (key)
    {
        while (*key)
        {
            // Otherwise traverse sideways until a matching letter is found
            while (*key != node->letter)
            {
                if (node->sibling == NULL)
                {
                    node->sibling = _json_alloc(sizeof(JsonNode));
                    node->sibling->sibling = NULL;
                    node->sibling->child = NULL;
                    node->sibling->data = NULL;
                    node->sibling->letter = *key;
                }
                node = node->sibling;
            }

            // Check if the next character is null terminating.
            // If it is, then the current node will be the one to which data is
            // added to. If node, then traverse one node down, creating a new node
            // if it does not already exist.
            key++;
            if (node->child == NULL)
            {
                node->child = _json_alloc(sizeof(JsonNode));
                node->child->sibling = NULL;
                node->child->child = NULL;
                node->child->data = NULL;
                node->child->letter = *key;
            }
            node = node->child;
        }
    }

    if (!buffer.end && node->data != NULL)
    {
        // Free the old node data before adding in new data.
        _free_JsonValue(node->data);
    }
    node->data = value;

    return 0;
}

int set_value(JsonObject * obj, char * key, void* data, JsonDataType type)
{
    JsonValue* jd = _json_alloc(sizeof(JsonValue));
    jd->type = type;

    int status = _alloc_JsonElement(jd, data);

    if (status < 0)
    {
        _json_free(jd);
        return status;
    }

    _set_value(obj, key, jd);
    return 0;
}


JsonArray * create_JsonArray(size_t length)
{
    JsonArray* j = _json_alloc(sizeof(JsonArray));
    j->length = length;
    j->elements = _json_alloc(sizeof(JsonValue) * length);
    return j;
}

int set_element(JsonArray * j, size_t index, void * data, JsonDataType type)
{
    JsonValue *jd = &(j->elements[index]);
    jd->type = type;
    int status = _alloc_JsonElement(jd, data);

    if (status < 0)
    {
        _free_JsonValueData(jd);
    }

    return status;
}

JsonValue get_element(JsonArray * j, size_t index)
{
    if (index > j->length)
    {
        return (JsonValue) {
            .type=JSON_ERROR, 
            .data.e=INDEX_OUT_OF_BOUNDS
        };
    }
    return j->elements[index];
}
