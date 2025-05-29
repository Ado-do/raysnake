CC = gcc
CFLAGS = -Wall -Wextra -std=c17 $(shell pkg-config --cflags raylib)
LD_LIBS = $(shell pkg-config --libs raylib)

SRC = $(wildcard src/*)
OBJ = $(SRC:src/%.c=build/%.o)

GAME = build/raysnake


all: $(GAME)

$(GAME): $(OBJ)
	@mkdir -p build/
	$(CC) -o $@ $^ $(LD_LIBS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(GAME)
	./$(GAME)

clean:
	rm -rf build/

.PHONY: all clean
