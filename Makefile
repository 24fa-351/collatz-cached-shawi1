CC = gcc
CFLAGS = -Wall -g

SRC = src/collatz.c src/cache.c
INCLUDES = -Iinclude

all: collatz

collatz: $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o collatz $(SRC)

clean:
	rm -f collatz