# jsonfun-c

The funnest little json parsing and serialization C library in the world. Lightweight and fast - O(m) value lookup 
(m is length of string), and O(1) array indexing. Works with dynamic and static memory allocation. 

This library supports C99+, but one would ideally be using C11.

## Overview
This library is designed for ease of use.

To allocate mempool for JSON object.
```C
void Json_set_mempool(void * start, void * end);
```

To create a JSON object:
```C
JsonObject* create_JsonObject(void);
```

To get and set an values from a JSON object:
```C
JsonValue get_value(JsonObject * obj, char * key);
int set_value(JsonObject * obj, char * key, void* data, JsonDataType type);
```

To create an array:
```C
JsonArray * create_JsonArray(int16_t length);
```

To get and set array eleements:
```C
int set_element(JsonArray * j, int16_t index, void * data, JsonDataType type);
JsonValue get_element(JsonArray * j, int16_t index);
```

To dump a JsonObject to string:
```C
size_t dump_JsonObject(JsonObject *o, char* destination);
```

To parse a JsonObject from string (Not yet implemented)
```C
JsonObject* parse_JsonObject();
```

## Example
### Allocate some memory.

Typically, ~1 word per byte in a Json string. This can be done on stack or heap. Call `Json_set_mempool` and pass pointers to the beginning and the end of the memory pool.

```C
#define MEMPOOL_SIZE 1024
char mempool[MEMPOOL_SIZE];
Json_set_mempool(mempool, &mempool[MEMPOOL_SIZE - 1]);
```

### Parsing
Pass in a string to parse, and get a pointer to a JsonObject.
Assume the JSON object is:
```JSON
{
    "hi": 123,
    "arr": [1.0, null, false],
    "inner": 
    {
        "strData": "wohoo",
    }
}
```

```C
JsonObject* obj = parse_JsonObject(jsonStr)       // Assume the json string was read. I.e. from a gile.

JsonValue hi = get_value(obj, "hi");
hi.data.i;                                        // 123

JsonValue temp = get_value(obj, "arr");
JsonArray* arr = temp.data.a;
get_element(arr, 0).data.f;                       // 1.0
get_element(arr, 1).data.n;                       // null
get_element(arr, 2).data.b;                       // false

JsonValue outOfBounds = get_elemetn(arr, 3);
outOfBounds.type;                                 // JSON_ERROR
outOfBounds.e;                                    // INDEX_OUT_OF_BOUNDS

JsonObject* inner = get_value(obj, "inner");
get_element(inner, "strData").data.s;             // "woohoo"
```

### Modifications to a Json Object
The goal is to create the following JSON object:
```JSON
{
    "hi": 123,
    "arr": [1.0, null, false],
    "inner": 
    {
        "strData": "wohoo",
        "intData": 32,
        "inner":
        {
            "vals": [true, false]     
        }
    }
}
```

```C
// Add intData
int i = 32;
set_value(inner, "intData", &i, JSON_INT);


// Object Nesting
JsonObject* innerInner = create_JsonObject();
set_value(inner, "inner", innerInner, JSON_OBJECT)

// Arrays
JsonArray* innerInnerArray = create_JsonArray(2);
bool b1 = true;
bool b2 = false;
set_element(innerInnerArray, 0, &b1, JSON_BOOL);
set_element(innerInnerArray, 1, &b2, JSON_BOOL);
set_value(innerInner, "vals", innerInnerArray, JSON_ARRAY);
```

### Dump
Pass in a buffer, and get the number of bytes written, not including the null character
```C
char buffer[256];
size_t nBytes = dump_JsonObject(obj, buffer); // Number of bytes used not including null character.
```

## Things to note
1. The size of the buffer is limited to 2^16 bytes (~65kB). In the future, it would be possible to keep the size of the buffer to 4.3 gigs, but that would increase the internal size of the object tree (essentially doubling it). This should work for now.
2. Elements in the mempool are not "freed". For instance, if you call `set_value` on a key that already exists, the old JsonValue will not be removed/replaced from the mempool.
3. Arrays are of a static size, whose elements have no guarantee of value until they are set. In order to change the size of an array, the only option would be to create a new array, and copy over the old elements to the new. However, ```set_element``` will overwrite a previous value.
