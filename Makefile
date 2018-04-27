CC=clang
CFLAGS=-W -Wall -Wextra -pedantic
FILES=test.c src/json.c
SRC_DIR=src/

all:
	$(CC) -I $(SRC_DIR) $(FILES) -o test.out $(CFLAGS)

debug:
	$(CC) -g -I $(SRC_DIR) $(FILES) -o debug.out $(CFLAGS)
