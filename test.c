#include <assert.h>
#include <string.h>
#include <math.h>

#include "json.h"

void test_construction()
{
    // Initial structure:
    // {
    //     "hello": 13,
    //     "hella": 14,
    //     "": 13.1,
    //     "good bye": false
    // }
    JsonObject* j = create_JsonObject();

    int a1 = 13;
    int a2 = 14;
    float f1 = 13.1;
    bool b1 = false;
    set_value(j, "hello", &a1, JSON_INT);
    set_value(j, "hella", &a2, JSON_INT);
    set_value(j, "", &f1, JSON_FLOAT);
    set_value(j, "good bye", NULL, JSON_NULL);

    JsonValue jd = get_value(j, "hello");
    assert(jd.data.i == a1);

    jd = get_value(j, "hella");
    assert(jd.data.i == a2);

    jd = get_value(j, "");
    assert(jd.data.f == f1);

    jd = get_value(j, "good bye");
    assert(jd.data.n == NULL);

    set_value(j, "good bye", &b1, JSON_BOOL);
    assert(jd.data.b == b1);

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
    set_value(j2, "good bye", &b1, JSON_BOOL);

    set_value(j, "good bye", j2, JSON_OBJECT);

    jd = get_value(j, "good bye");
    assert(jd.data.o == j2);

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

    set_value(j, "good", "wurd", JSON_STRING);
    jd = get_value(j, "good");
    assert(strcmp(jd.data.s, "wurd") == 0);

    free_JsonObject(j);
}

void test_arrays()
{
    // Start off with a simple multi element array:
    // {"hi": [0, 332, true, null, "henlo world"]}

    JsonObject * o = create_JsonObject();

    JsonArray * array = create_JsonArray(5);
    int i1 = 0, i2 = 332;
    bool b1 = true; void* p = NULL;
    set_element(array, 0, &i1, JSON_INT);
    set_element(array, 1, &i2, JSON_INT);
    set_element(array, 2, &b1, JSON_BOOL);
    set_element(array, 3, p, JSON_NULL);
    set_element(array, 4, "henlo world", JSON_STRING);

    set_value(o, "hi", array, JSON_ARRAY);

    // test if array values are correct.
    array = get_value(o, "hi").data.a;
    JsonValue jd = get_element(array, 0);
    assert(jd.data.i == 0);

    jd = get_element(array, 1);
    assert(jd.data.i == 332);

    jd = get_element(array, 2);
    assert(jd.data.b == true);

    jd = get_element(array, 3);
    assert(jd.data.n == NULL);

    jd = get_element(array, 4);
    assert(strcmp(jd.data.s, "henlo world") == 0);

    free_JsonObject(o);


    // Nest that array into another array 
    // {"hi": [
    //    [0, -1, true, null],
    //    {} 
    // ]}
}

void test_nesting()
{
    // Nest that array into another array 
    // {
    //   "hi":
    //    [
    //        [0, -1, true, null, "henlo world"],
    //        {"pi": [3.14]}
    //    ]
    // }
    JsonArray * arrayInner = create_JsonArray(5);

    int i1 = 0, i2 = 332;
    bool b1 = true; void* p = NULL;
    set_element(arrayInner, 0, &i1, JSON_INT);
    set_element(arrayInner, 1, &i2, JSON_INT);
    set_element(arrayInner, 2, &b1, JSON_BOOL);
    set_element(arrayInner, 3, p, JSON_NULL);
    set_element(arrayInner, 4, "henlo world", JSON_STRING);

    JsonArray * piArray = create_JsonArray(1);
    float f1 = 3.14;
    set_element(piArray, 0, &f1, JSON_FLOAT);

    JsonObject * piObj = create_JsonObject();
    set_value(piObj, "pi", piArray, JSON_ARRAY);

    JsonArray * arrayOuter = create_JsonArray(2);
    set_element(arrayOuter, 0, arrayInner, JSON_ARRAY);
    set_element(arrayOuter, 1, piObj, JSON_OBJECT);

    JsonObject* o = create_JsonObject();
    set_value(o, "hi", arrayOuter, JSON_ARRAY);

    arrayOuter = get_value(o, "hi").data.a;
    JsonArray * array = get_element(arrayOuter, 0).data.a;

    // Test for first element in array
    JsonValue jd = get_element(array, 0);
    assert(jd.data.i == 0);

    jd = get_element(array, 1);
    assert(jd.data.i == 332);

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

    free_JsonObject(o);
}


int main() {
    // Run this many times to test for memory leaks.
    for (int i = 0; i < 1000000; i++)
    {
        test_construction();
        test_arrays();
        test_nesting();
    }
    
    return 0;
}
