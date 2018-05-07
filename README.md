# jsonfun-c

The funnest little json parsing and serialization C library in the world. Lightweight and fast - O(log N) value lookup, and O(1) array indexing. Works with dynamic and static memory allocation. 

## Documentation
How to use:
1. Allocate some memory. Typically, ~1 word per byte in a Json string. This can be done on stack or heap. Call `Json_set_mempool` and pass pointers to the beginning and the end of the memory pool.

```
#define MEMPOOL_SIZE 1024
char mempool[MEMPOOL_SIZE];
Json_set_mempool(mempool, &mempool[MEMPOOL_SIZE - 1]);

```

2. Parsing. Pass in a string to parse, and get a pointer to a JsonObject.

```
JsonObject* obj = Json_parse("{\"hi\": 123, \"arr\": [1.0, null, false], \"inner\": {\"strData\": \"woohoo\"}}")


JsonValue hi = get_value(obj, "hi");
hi.data.i;  // 123

JsonValue temp = get_value(obj, "arr");
JsonArray* arr = temp.data.a;
get_element(arr, 0).data.f;  // 1.0
get_element(arr, 1).data.n;  // null
get_element(arr, 2).data.b;  // false

JsonValue outOfBounds = get_elemetn(arr, 3);
outOfBounds.type; //JSON_ERROR
outOfBounds.e; //INDEX_OUT_OF_BOUNDS

JsonObject* inner = get_value(obj, "inner");
get_element(inner, "strData").data.s; // "woohoo"
```

3. Add elements to the Json Object:
```
// Final structure will be
{
    "hi": 123,
    "arr": [1.0, null, false],
    "inner": 
    {
        "strData": "wohoo",
        "intData": 32,
        "inner":
        {
            vals: [true, false]     
        }
    }
}

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

4. Dump the updated json object
```
char buffer[256];
size_t nBytes = dump_JsonObject(obj, buffer); // Number of bytes used not including null character.
```

### Internal Structure
#### JSON Objects and Values
#### JSON Arrays
#### Memory allocation

### Parsing
### Serialization
