//
//  json.c
//  JSON
//
//  Created by Viraj Bangari on 2018-04-24.
//  Copyright © 2018 Viraj Bangari. All rights reserved.

#define DEBUG_JSON
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>
// Add alignof for C99 compatibility
// Credit to Martin Buchholz from: http://www.wambold.com/Martin/writings/alignof.html
#ifndef alignof
    #define alignof(type) offsetof (struct { char c; type member; }, member)
#endif
#include "json.h"

typedef struct Mempool
{
    int8_t * start;
    int8_t * end;
    int8_t * top;
} Mempool;

Mempool buffer = { .end=NULL, .top=NULL };

void Json_set_mempool(void * start, size_t size)
{
    buffer.start = start;
    buffer.top = start;
    buffer.end = buffer.start + size;
}

void Json_reset_mempool()
{
    buffer.top = buffer.start;
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

    #ifdef DEBUG_JSON
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
            jd->data.b = *((bool *) data);
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

bool set_value_null(JsonObject * obj, char * key)
{
    return set_value(obj, key, NULL, JSON_NULL);
}

bool set_value_string(JsonObject * obj, char * key, char * str)
{
    return set_value(obj, key, str, JSON_STRING);
}

bool set_value_bool(JsonObject * obj, char * key, bool data)
{
    return set_value(obj, key, &data, JSON_BOOL);
}

bool set_value_int(JsonObject * obj, char * key, int data)
{
    return set_value(obj, key, &data, JSON_INT);
}

bool set_value_float(JsonObject * obj, char * key, float data)
{
    return set_value(obj, key, &data, JSON_FLOAT);
}

bool set_value_object(JsonObject * obj, char * key, JsonObject * object)
{
    return set_value(obj, key, object, JSON_OBJECT);
}

bool set_value_array(JsonObject * obj, char * key, JsonArray * array)
{
    return set_value(obj, key, array, JSON_ARRAY);
}

bool set_value(JsonObject * obj, char * key, void* data, JsonDataType type)
{
    JsonValue* jd = _json_alloc(sizeof(JsonValue), alignof(JsonValue));
    jd->type = type;

    int status = _alloc_JsonElement(jd, data);

    if (status < 0)
    {
        return false;
    }

    _set_value(obj, key, jd);
    return true;
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

#define JSON_STACK_LENGTH 128

typedef struct _Stack
{
    void* stack[JSON_STACK_LENGTH];
    int stacktop;
} _Stack;

int push_ptr(_Stack* s, void* p)
{
    if (s->stacktop >= JSON_STACK_LENGTH - 1)
    {
        printf("Json: Stack overflow\n");
        return -1;
    }

    (s->stacktop)++;
    s->stack[s->stacktop] = p;

    return 0;
}

int push_int(_Stack* s, int i)
{
    if (s->stacktop >= JSON_STACK_LENGTH - 1)
    {
        printf("Json: Stack overflow\n");
        return -1;
    }

    (s->stacktop)++;
    ((int *)s->stack)[s->stacktop] = i;

    return 0;
}

void* pop_ptr(_Stack* s)
{
    void *rval = s->stack[s->stacktop];
    (s->stacktop)--;

    return rval;
}

void* peek_ptr(_Stack* s)
{
    void *rval = s->stack[s->stacktop];
    return rval;
}

int pop_int(_Stack* s)
{
    int rval = ((int*)s->stack)[s->stacktop];
    (s->stacktop)--;

    return rval;
}

int peek_int(_Stack* s)
{
    int rval = ((int*)s->stack)[s->stacktop];
    return rval;
}

char _JSON_NULL_STR[] = "null";
char _JSON_FALSE_STR[] = "false";
char _JSON_TRUE_STR[] = "true";

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

char*
_dump_JsonObject(
    JsonObject *o,
    char* destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack,
    char* key_buffer);

char * 
_dump_JsonValue(
    JsonValue* value, 
    char * destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack,
    char* key_buffer);

char*
_dump_JsonArray(
    JsonArray *ary,
    char* destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack,
    char* key_buffer)
{
    *(destination++) = '[';
    for (int i = 0; i < ary->length; i++)
    {
        if (i > 0)
        {
            *(destination++) = ',';
        }

        JsonValue* element = &((JsonValue*)(buffer.start + ary->elements))[i];
        switch (element->type)
        {
            case JSON_OBJECT:
                destination = _dump_JsonObject(
                    element->data.o, 
                    destination, 
                    valstack, 
                    bufend_stack,
                    objIndex_stack,
                    key_buffer);
                break;
            default:
                destination = _dump_JsonValue(
                    element,
                    destination, 
                    valstack, 
                    bufend_stack,
                    objIndex_stack,
                    key_buffer);
                break;
        }
    }
    *(destination++) = ']';
    return destination;
}

void strcpyescaped(char * source, char * destination)
{
    while (*source)
    {
        switch (*source)
        {
            case '"':
                *(destination++) = '\\';
                *(destination++) = '"';
                break;
            case '\\':
                *(destination++) = '\\';
                *(destination++) = '\\';
                break;
            case '\b':
                *(destination++) = '\\';
                *(destination++) = 'b';
                break;
            case '\f':
                *(destination++) = '\\';
                *(destination++) = 'f';
                break;
            case '\n':
                *(destination++) = '\\';
                *(destination++) = 'n';
                break;
            case '\r':
                *(destination++) = '\\';
                *(destination++) = 'r';
                break;
            case '\t':
                *(destination++) = '\\';
                *(destination++) = 't';
                break;
            default:
                *destination = *source;
                break;
        }
        source++;
    }
}

char * 
_dump_JsonValue(
    JsonValue* value, 
    char * destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack,
    char* key_buffer)
{
    switch (value->type)
    {
        char *str;
        case JSON_NULL:
            str = _JSON_NULL_STR;
            while (*str) *(destination++) = *(str++);
            break;
        case JSON_STRING:
            str = value->data.s;
            *(destination++) = '"';
            while (*str) *(destination++) = *(str++);
            *(destination++) = '"';
            break;
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
        {
            destination = _dump_JsonArray(
                value->data.a, 
                destination, 
                valstack, 
                bufend_stack,
                objIndex_stack,
                key_buffer);
            break;
        }
        default:
            break;
    }
    return destination;
}

char*
_dump_JsonObject(
    JsonObject *o,
    char* destination,
    _Stack* valstack,
    _Stack* bufend_stack,
    _Stack* objIndex_stack,
    char* key_buffer)
{
    push_int(objIndex_stack, valstack->stacktop);
    push_ptr(valstack, &(o->node));
    push_int(bufend_stack, 0);

    int intial_valstack_top = valstack->stacktop;
    int intial_objIndex_stack_top = objIndex_stack->stacktop;

    *(destination++) = '{';
    while (valstack->stacktop >= intial_valstack_top)
    {
        JsonNode* node = pop_ptr(valstack);
        int strIndex  = pop_int(bufend_stack);  // This will update strIndex
        key_buffer[strIndex] = node->letter;  // Add the current key to the buffer

        // Add sibling to stack if exists
        if (node->sibling != DEFAULT_OBJECT_ADDRESS)
        {
            JsonNode* sibling = (JsonNode*)(buffer.start + node->sibling); 
            push_ptr(valstack, sibling);
            push_int(bufend_stack, strIndex);
        }

        // Add child to stack if exists
        if (node->child != DEFAULT_OBJECT_ADDRESS)
        {
            JsonNode* child = (JsonNode*)(buffer.start + node->child); 
            push_ptr(valstack, child);
            push_int(bufend_stack, strIndex + 1);
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
            destination = _dump_JsonValue(value, destination, valstack, bufend_stack, objIndex_stack, key_buffer);
        }

        if (valstack->stacktop == peek_int(objIndex_stack))
        {
            *(destination++) = '}';
            pop_int(objIndex_stack);
        }
    }

    // Check this at the end, since nested objects will not check for the '}'
    while (objIndex_stack->stacktop >= intial_objIndex_stack_top)
    {
        *(destination++) = '}';
        pop_int(objIndex_stack);
    }

    return destination;
}

size_t dump_JsonObject(JsonObject* o, char* destination)
{
    char key_buffer[256];
    _Stack valstack, bufend_stack, objIndex_stack;
    valstack.stacktop = -1;
    bufend_stack.stacktop = -1;
    objIndex_stack.stacktop = -1;

    char* end = 
        _dump_JsonObject(o, destination, &valstack, &bufend_stack, &objIndex_stack, key_buffer);
    *end = '\0';
    
    return end - destination;
}

enum JsonParseTypes
{
    Parse_JsonObjectStart,
    Parse_JsonMembers,
    Parse_JsonValue,
    Parse_Colon,
    Parse_JsonValueSeparator,
    Parse_JsonString,
    Parse_JsonFloat,
};

typedef struct _Parser
{
    char* input;
    char* buffer;
    _Stack jsonParseStack;
    _Stack jsonObjectStack;
    _Stack jsonBufferStack;
} _Parser;

void next_token(_Parser* p)
{
    #ifdef DEBUG_JSON
    printf("Current token: '%c'\n", *p->input);
    #endif
    p->input++;
}

void skip_whitespace(_Parser* parser)
{
    while (*(parser->input))
    {
        switch (*(parser->input))
        {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                break;
            default:
                return;
        }
        next_token(parser);
    }
}

bool parse_JsonObjectStart(_Parser* parser)
{
    skip_whitespace(parser);
    while (true)
    {
        switch (*(parser->input))
        {
            case '{':
                pop_int(&parser->jsonParseStack);
                push_int(&parser->jsonParseStack, Parse_JsonMembers);
                push_ptr(&parser->jsonObjectStack, create_JsonObject());
                next_token(parser);
                return true;
            default:
                return false;
        }
        next_token(parser);
    }
}

bool parse_JsonMembers(_Parser* parser)
{
    skip_whitespace(parser);
    while (true)
    {
        switch (*(parser->input))
        {
            case '}':
                pop_int(&parser->jsonParseStack);
                if (parser->jsonObjectStack.stacktop > 0)
                {
                    JsonObject* child = pop_ptr(&parser->jsonObjectStack);
                    JsonObject* parent = peek_ptr(&parser->jsonObjectStack);

                    parser->buffer = pop_ptr(&parser->jsonBufferStack);
                    set_value_object(parent, parser->buffer, child);
                }
                next_token(parser);
                return true;
            case '"':
                push_int(&parser->jsonParseStack, Parse_JsonValueSeparator);
                push_int(&parser->jsonParseStack, Parse_JsonValue);
                push_int(&parser->jsonParseStack, Parse_Colon);
                push_int(&parser->jsonParseStack, Parse_JsonString);
                next_token(parser);
                return true;
            default:
                return false;
        }
        next_token(parser);
    }
}

bool parse_EscapedChar(_Parser * parser)
{
    next_token(parser);
    switch (*(parser->input))
    {
        case '"':
        *(parser->buffer++) = '\"';
            break;
        case '\\':
            *(parser->buffer++) = '\\';
            break;
        case '/':
            *(parser->buffer++) = '/';
            break;
        case 'b':
            *(parser->buffer++) = '\b';
            break;
        case 'f':
            *(parser->buffer++) = '\f';
            break;
        case 'n':
            *(parser->buffer++) = '\n';
            break;
        case 'r':
            *(parser->buffer++) = '\r';
            break;
        case 't':
            *(parser->buffer++) = '\t';
            break;
        default:
            return false;
    }
    next_token(parser);

    return true;
}

bool parse_JsonString(_Parser * parser)
{
    push_ptr(&parser->jsonBufferStack, parser->buffer);
    while (*(parser->input))
    {
        switch (*(parser->input))
        {
            case '"':
                *(parser->buffer++) = '\0';
                pop_int(&parser->jsonParseStack);
                next_token(parser);
                return true;
            case '\\':
                if (!parse_EscapedChar(parser))
                {
                    return false;
                }
                break;
            default:
                *(parser->buffer++) = *(parser->input);
        }
        next_token(parser);
    }

    return true;
}

bool parse_Colon(_Parser * parser)
{
    skip_whitespace(parser);
    switch (*(parser->input))
    {
        case ':':
            next_token(parser);
            break;
        default:
            return false;
    }

    return true;
}

bool parse_JsonValue(_Parser * parser)
{
    skip_whitespace(parser);
    pop_int(&parser->jsonParseStack);
    switch (*(parser->input))
    {
        case '"':
            next_token(parser);
            push_int(&parser->jsonParseStack, Parse_JsonString);
            parse_JsonString(parser);

            char* value = pop_ptr(&parser->jsonBufferStack);
            parser->buffer = pop_ptr(&parser->jsonBufferStack);
            JsonObject * o = peek_ptr(&parser->jsonObjectStack);
            set_value_string(o, parser->buffer, value);
            break;
        case 'n':
        {
            for (unsigned long i = 0; i < sizeof(_JSON_NULL_STR) - 1; i++)
            {
                if (*(parser->input) != _JSON_NULL_STR[i]) return false;
                next_token(parser);
            }
            JsonObject * o = peek_ptr(&parser->jsonObjectStack);
            parser->buffer = pop_ptr(&parser->jsonBufferStack);
            set_value_null(o, parser->buffer);
            break;
        }
        case 't':
        {
            for (unsigned long i = 0; i < sizeof(_JSON_TRUE_STR) - 1; i++)
            {
                if (*(parser->input) != _JSON_TRUE_STR[i]) return false;
                next_token(parser);
            }
            JsonObject * o = peek_ptr(&parser->jsonObjectStack);
            parser->buffer = pop_ptr(&parser->jsonBufferStack);
            set_value_bool(o, parser->buffer, true);
            break;
        }
        case 'f':
        {
            for (unsigned long i = 0; i < sizeof(_JSON_FALSE_STR) - 1; i++)
            {
                if (*(parser->input) != _JSON_FALSE_STR[i]) return false;
                next_token(parser);
            }
            JsonObject * o = peek_ptr(&parser->jsonObjectStack);
            parser->buffer = pop_ptr(&parser->jsonBufferStack);
            set_value_bool(o, parser->buffer, false);
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            push_int(&parser->jsonParseStack, Parse_JsonFloat);
            break;
        case '{':
            push_int(&parser->jsonParseStack, Parse_JsonObjectStart);
            break;
        default:
            return false;
    }

    return true;
}

bool parse_JsonValueSeparator(_Parser* parser)
{
    skip_whitespace(parser);
    while (true)
    {
        switch (*(parser->input))
        {
            case '}':
                pop_int(&parser->jsonParseStack);
                return true;
            case ',':
                next_token(parser);
                pop_int(&parser->jsonParseStack);
                return true;
            default:
                return false;
        }
    }
}

bool parse_JsonObject(char* input, JsonObject** parsed)
{
    char buffer[1024];
    _Parser parser;
    parser.input = input;
    parser.buffer = &buffer[0];
    parser.jsonParseStack.stacktop = -1;
    parser.jsonObjectStack.stacktop = -1;
    parser.jsonBufferStack.stacktop = -1;

    skip_whitespace(&parser);
    push_int(&parser.jsonParseStack, Parse_JsonObjectStart);
    while (parser.jsonParseStack.stacktop >= 0)
    {
        switch ((enum JsonParseTypes) peek_int(&parser.jsonParseStack))
        {
            case Parse_JsonObjectStart:
                #ifdef DEBUG_JSON
                printf("Parsing object\n");
                #endif
                if (!parse_JsonObjectStart(&parser))
                {
                    return false;
                }
                break;
            case Parse_JsonMembers:
                #ifdef DEBUG_JSON
                printf("Parsing object members\n");
                #endif
                if (!parse_JsonMembers(&parser))
                {
                    return false;
                }
                break;
            case Parse_Colon:
                #ifdef DEBUG_JSON
                printf("Parsing colon\n");
                #endif
                if (!parse_Colon(&parser))
                {
                    return false;
                }
            case Parse_JsonString:
                #ifdef DEBUG_JSON
                printf("Parsing json string\n");
                #endif
                if (!parse_JsonString(&parser))
                {
                    return false;
                }
                break;
            case Parse_JsonFloat:
                break;
            case Parse_JsonValue:
                #ifdef DEBUG_JSON
                printf("Parsing value\n");
                #endif
                if (!parse_JsonValue(&parser))
                {
                    return false;
                }
                break;
            case Parse_JsonValueSeparator:
                #ifdef DEBUG_JSON
                printf("Parsing value separator\n");
                #endif
                if (!parse_JsonValueSeparator(&parser))
                {
                    return false;
                }
                break;
        }
    }

    *parsed = pop_ptr(&parser.jsonObjectStack);

    return true;
}
