#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lib/json.h"

#define MEMPOOL_SIZE 65536
void parse_file(char* filename)
{   
    char* inBuffer = malloc(MEMPOOL_SIZE);
    char* outBuffer;
    FILE * file = fopen(filename, "r");
    if (file)
    {
        int c = 0, i = 0;
        while ((c = getc(file)) != EOF)
        {
            inBuffer[i++] = c;
        }
        inBuffer[i++] = '\0';
        outBuffer = malloc(i);
        fclose(file);
    }
    printf("Input:\n%s\n", inBuffer);

    // Parse the input
    JsonObject *parsed;
    bool success = parse_JsonObject(inBuffer, &parsed);
    assert(success);

    // Dump the object
    success = dump_JsonObject(parsed, outBuffer);
    assert(success);
    printf("Output:\n%s\n", outBuffer);

    free(inBuffer);
    free(outBuffer);
}

int main()
{
    // Statically allocate 65kB.
    char mempool[MEMPOOL_SIZE];
    Json_set_mempool(mempool, MEMPOOL_SIZE);
    parse_file("samples/sample1.json");
    parse_file("samples/sample2.json");
    parse_file("samples/sample3.json");

    return 0;
}
