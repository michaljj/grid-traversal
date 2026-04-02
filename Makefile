CC = gcc
CFLAGS = -Wall -Wextra

all: main

main: main.c gridComp.c candidate.c gridComp.h testParams.h candidate.h
	$(CC) $(CFLAGS) -o main main.c gridComp.c candidate.c -I.

clean:
	rm -f main main_san
