.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Og -g
LDFLAGS   =

TESTS     = tbti_test

all: build
	@(for t in $(TESTS); do echo $$t; ./$$t; done)
build: $(TESTS)
clean:
	rm -f $(TESTS)
.PHONY: all build clean



$(TESTS): miniunit.h

# Implicit rule to build object files from .c source files
.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $< -o $@

