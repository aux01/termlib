.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Og -g
LDFLAGS   =

TESTS     = tbti_test

all: build
	./runtest $(TESTS)
build: $(TESTS)
clean:
	rm -f $(TESTS)
.PHONY: all build clean

# Implicit rule to build object files from .c source files
.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $< -o $@

