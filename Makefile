CC=clang
CFLAGS=-W -Wall -Wextra -pedantic
FILES=test.c json.c

all:
	$(CC) $(FILES) -o test.out $(CFLAGS)

debug:
	$(CC) -g $(FILES) -o debug.out $(CFLAGS)
