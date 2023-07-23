CC := gcc
SOURCES := $(wildcard *.c)
HEADERS := $(wildcard *.h)
EXECUTABLES := $(patsubst %.c,build/%,$(SOURCES))

all: build

build: $(EXECUTABLES)

build/%: %.c $(HEADERS)
	mkdir -p build
	$(CC) $< -std=c89 -pedantic -Wall -Wextra -g -o $@

clean:
	rm -rf build

format:
	clang-format -style=file -i *.c *.h
