CC=clang
CFLAGS=-W -Wall -Wextra -pedantic -std=c11
JSON_FILES=lib/json.c
LIB_DIR=lib/

all: sample testing

sample:
	$(CC) -I $(LIB_DIR) $(JSON_FILES) samples.c -o bin/sample.out $(CFLAGS)

testing:
	$(CC) -I $(LIB_DIR) $(JSON_FILES) test.c -o bin/test.out $(CFLAGS)

debug:
	$(CC) -g -I $(LIB_DIR) $(JSON_FILES) test.c -o bin/debug.out $(CFLAGS)
