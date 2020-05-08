.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -Wno-missing-field-initializers \
          -D_XOPEN_SOURCE -Og -g
LDFLAGS =
LDLIBS  =

TESTS   = ti_setupterm_test ti_getcaps_test ti_parm_test \
          sgr_unpack_test sgr_encode_test sgr_attrs_test

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

