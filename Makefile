all:
	cc main.c json.c -o test.out -W -Wall -Wextra -pedantic

debug:
	cc -g main.c json.c -o debug.out -W -Wall -Wextra -pedantic
