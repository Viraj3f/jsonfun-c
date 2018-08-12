#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "lib/json.h"


void test_construction()
{
    printf("\nTESTING CONSTRUCTION\n");
    // Initial structure:
    // {
    //     "hello": 13,
    //     "hella": 14,
    //     "": 13.1,
    //     "good bye": false
    // }
    char buffer[256];
    JsonObject* j = create_JsonObject();

    float a1 = 13;
    float a2 = 14;
    float f1 = 13.1;
    bool b1 = false;
    set_value_float(j, "hello", a1);
    set_value_float(j, "hella", a2);
    set_value_float(j, "", f1);
    set_value_null(j, "good bye");

    JsonValue jd = get_value(j, "hello");
    assert(jd.data.f == a1);

    jd = get_value(j, "hella");
    assert(jd.data.f == a2);

    jd = get_value(j, "");
    assert(jd.data.f == f1);

    jd = get_value(j, "good bye");
    assert(jd.data.n == NULL);

    set_value_bool(j, "good bye", b1);
    jd = get_value(j, "good bye");
    assert(jd.data.b == b1);

    dump_JsonObject(j, buffer);
    printf("%s\n", buffer);

    // Structure:
    // {
    //     "hello": 13,
    //     "hella": 14,
    //     "": 13.1,
    //     "good bye":
    //     {
    //         "good bye": true;
    //     }
    // }
    JsonObject* j2 = create_JsonObject();
    b1 = true;
    set_value_bool(j2, "good bye", b1);
    set_value_object(j, "good bye", j2);

    jd = get_value(j, "good bye");
    assert(jd.data.o == j2);

    dump_JsonObject(j, buffer);
    printf("%s\n", buffer);

    // Structure:
    // {
    //     "hello": 13,
    //     "hella": 14,
    //     "": 13.1,
    //     "good bye":
    //     {
    //         "good bye": "string";
    //     }
    // }
    jd = get_value(jd.data.o, "good bye");
    assert(jd.data.b == b1);

    set_value_string(j, "good", "wurd");
    jd = get_value(j, "good");
    assert(strcmp(jd.data.s, "wurd") == 0);

    dump_JsonObject(j, buffer);
    printf("%s\n", buffer);
}

void test_arrays()
{
    printf("\nTESTING ARRAYS\n");
    // Start off with a simple multi element array:
    // {"hi": [0, 332, true, null, "henlo world"]}

    JsonObject * o = create_JsonObject();

    JsonArray * array = create_JsonArray(5);
    float i1 = 0, i2 = 332;
    bool b1 = true;
    set_element_float(array, 0, i1);
    set_element_float(array, 1, i2);
    set_element_bool(array, 2, b1);
    set_element_null(array, 3);
    set_element_string(array, 4, "henlo world");

    set_value_array(o, "hi", array);

    // test if array values are correct.
    array = get_value(o, "hi").data.a;
    JsonValue jd = get_element(array, 0);
    assert(jd.data.f == 0);

    jd = get_element(array, 1);
    assert(jd.data.f == 332);

    jd = get_element(array, 2);
    assert(jd.data.b == true);

    jd = get_element(array, 3);
    assert(jd.data.n == NULL);

    jd = get_element(array, 4);
    assert(strcmp(jd.data.s, "henlo world") == 0);

    char buffer[256];
    dump_JsonObject(o, buffer);
    printf("%s\n", buffer);
}

void test_nesting()
{
    printf("\nTESTING NESTING\n");
    // Nest that array into another array 
    // {
    //   "hi":
    //    [
    //        [0, -1, true, null, "henlo world"],
    //        {"pi": [3.14]}
    //    ]
    // }
    JsonArray * arrayInner = create_JsonArray(5);

    float i1 = 0, i2 = 332;
    bool b1 = true;
    set_element_float(arrayInner, 0, i1);
    set_element_float(arrayInner, 1, i2);
    set_element_bool(arrayInner, 2, b1);
    set_element_null(arrayInner, 3);
    set_element_string(arrayInner, 4, "henlo world");

    JsonArray * piArray = create_JsonArray(1);
    float f1 = 3.14;
    set_element_float(piArray, 0, f1);

    JsonObject * piObj = create_JsonObject();
    set_value_array(piObj, "pi", piArray);

    JsonArray * arrayOuter = create_JsonArray(2);
    set_element_array(arrayOuter, 0, arrayInner);
    set_element_object(arrayOuter, 1, piObj);

    JsonObject* o = create_JsonObject();
    set_value_array(o, "hi", arrayOuter);

    arrayOuter = get_value(o, "hi").data.a;
    JsonArray * array = get_element(arrayOuter, 0).data.a;

    // Test for first element in array
    JsonValue jd = get_element(array, 0);
    assert(jd.data.f == 0);

    jd = get_element(array, 1);
    assert(jd.data.f == 332);

    jd = get_element(array, 2);
    assert(jd.data.b == true);

    jd = get_element(array, 3);
    assert(jd.data.n == NULL);

    jd = get_element(array, 4);
    assert(strcmp(jd.data.s, "henlo world") == 0);

    // Test for first element in array
    piObj = get_element(arrayOuter, 1).data.o;
    array = get_value(piObj, "pi").data.a;

    jd = get_element(array, 1);
    assert(fabs(jd.data.f - f1) > 0.0000000001);

    char buffer[256];
    dump_JsonObject(o, buffer);
    printf("%s\n", buffer);
}

void test_printing()
{
    printf("\nTESTING PRINTING\n");
    // Create an object like:
    // {"Hen":null,"Henlo":"Gudbye","10":,"":true,"inner":{"innerinner":{},"false":false}}
    char buffer[256];
    JsonObject* o = create_JsonObject();
    dump_JsonObject(o, buffer);
    printf("%s\n", buffer);

    float i = 10;
    bool b = true;
    float f = 3.12;
    float f4 = 5.44;
    set_value_string(o, "Henlo", "Gudbye");
    set_value_float(o, "10", i);
    set_value_null(o, "Hen");
    set_value_bool(o, "", b);
    set_value_float(o, "f", f);

    JsonObject* inner = create_JsonObject();

    JsonObject* innerinner = create_JsonObject();
    set_value_object(inner, "innerinner", innerinner);
    set_value_bool(inner, "false", false);

    set_value_float(o, "inz", f4);
    set_value_object(o, "inner", inner);

    dump_JsonObject(o, buffer);
    printf("%s\n", buffer);

    const char * expected = "{\"Hen\":null,\"Henlo\":\"Gudbye\",\"10\":10,\"\":true,\"f\":3.12,\"inz\":5.44,\"inner\":{\"innerinner\":{},\"false\":false}}";
    printf("%s\n", expected);
    assert(strcmp(buffer, expected) == 0);

    JsonObject* iiii = create_JsonObject();
    JsonObject* iii = create_JsonObject();
    JsonObject* ii = create_JsonObject();
    JsonObject* in = create_JsonObject();

    set_value_object(iii, "iii", iiii);
    set_value_object(ii, "ii", iii);
    set_value_object(in, "i", ii);

    dump_JsonObject(in, buffer);
    printf("%s\n", buffer);
    const char * expected2 = "{\"i\":{\"ii\":{\"iii\":{}}}}";
    assert(strcmp(buffer, expected2) == 0);
}

void test_parsing()
{
    char buffer[256];

    Json_reset_mempool();
    char* input1 = "   {   }  ";
    JsonObject* parsed1;
    parse_JsonObject(input1, &parsed1);
    dump_JsonObject(parsed1, buffer);
    printf("%s\n", buffer);
    assert(strcmp(buffer, "{}") == 0);

    Json_reset_mempool();
    char* input2 = "{\"null\": null, \"true\": true, \"false\": false, \"inner\": {\"innerinner\": \"thisisalongstring\"}}";
    JsonObject* parsed2;
    parse_JsonObject(input2, &parsed2);
    dump_JsonObject(parsed2, buffer);
    printf("%s\n", buffer);
    char* expected2 = "{\"null\":null,\"true\":true,\"false\":false,\"inner\":{\"innerinner\":\"thisisalongstring\"}}";
    assert(strcmp(buffer, expected2) == 0);

    Json_reset_mempool();
    char* input3 = "{\"a\": 1, \"b\": 32.1e-2,\"c\": -3.012, \"d\": {\"33\": -33, \"\":{}}}";
    JsonObject* parsed3;
    parse_JsonObject(input3, &parsed3);
    dump_JsonObject(parsed3, buffer);
    printf("%s\n", buffer);
    char* expected3 = "{\"a\":1,\"b\":0.321,\"c\":-3.012,\"d\":{\"33\":-33,\"\":{}}}";
    assert(strcmp(buffer, expected3) == 0);

    Json_reset_mempool();
    char* input4 = "{\"a\": [], \"b\": [0, true, false, [], \"mystring1\", \"mystring2\", {\"yo\": [\"yothisisastring\", {\"yoinner\": [null]}]}]}";
    JsonObject* parsed4;
    parse_JsonObject(input4, &parsed4);
    dump_JsonObject(parsed4, buffer);
    printf("%s\n", buffer);
    char* expected4 = "{\"a\":[],\"b\":[0,true,false,[],\"mystring1\",\"mystring2\",{\"yo\":[\"yothisisastring\",{\"yoinner\":[null]}]}]}";
    assert(strcmp(buffer, expected4) == 0);

    Json_reset_mempool();
    char* input5 = "{\"\":[[[[], []], []], [], [], [[[], []]]]}";
    JsonObject* parsed5;
    parse_JsonObject(input5, &parsed5);
    dump_JsonObject(parsed5, buffer);
    printf("%s\n", buffer);
    char* expected5 = "{\"\":[[[[],[]],[]],[],[],[[[],[]]]]}";
    assert(strcmp(buffer, expected5) == 0);
}


int main()
{
    // Statically allocate 1024kb.
    #define MEMPOOL_SIZE 1024
    char mempool[MEMPOOL_SIZE];
    Json_set_mempool(mempool, MEMPOOL_SIZE);
    test_construction();

    Json_reset_mempool();
    test_arrays();

    Json_reset_mempool();
    test_nesting();

    Json_reset_mempool();
    test_printing();

    Json_reset_mempool();
    test_parsing();
    
    return 0;
}
