CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = main.c gridComp.c candidate.c testParams.c
HDR = gridComp.h candidate.h testParams.h

all: main

main: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o main $(SRC) -I.

clean:
	rm -f main main.exe
