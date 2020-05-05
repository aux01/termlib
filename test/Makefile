.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Og -g
LDFLAGS =
LDLIBS  = -lm

TESTS   = tbti_setupterm_test tbti_getcaps_test \
          tbsgr_ints_test tbsgr_encode_test

test: clean all
all: build
	./runtest $(TESTS)
build: $(TESTS)
clean:
	rm -f $(TESTS)
.PHONY: all build clean test

# Implicit rule to build object files from .c source files
.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

