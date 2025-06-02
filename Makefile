CC = gcc
CFLAGS = -Wall -Wextra -ggdb -std=c17

LD_LIBS = -lraylib
ifeq ($(OS), Windows_NT)
	LD_LIBS += -lopengl32 -lgdi32 -lwinmm
else
	LD_LIBS += -lm
endif

SRC = $(wildcard src/*)
OBJ = $(SRC:src/%.c=build/%.o)

GAME = build/raysnake


all: $(GAME)

$(GAME): $(OBJ)
	$(CC) -o $@ $^ $(LD_LIBS)

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -c $< -o $@

run: $(GAME)
	./$(GAME)

clean:
	rm -rf build/

.PHONY: all run clean
