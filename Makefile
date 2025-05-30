CC = gcc
CFLAGS = -Wall -Wextra -ggdb -std=c17

ifeq ($(OS), Windows_NT)
	LD_LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else
	CFLAGS += $(shell pkg-config --cflags raylib)
	LD_LIBS = $(shell pkg-config --libs raylib)
endif

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

.PHONY: all run clean
