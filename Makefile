.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 $(WARN) $(OPTIMIZE) -fPIC
WARN      = -Wall -Wextra
OPTIMIZE  = -O2
LDFLAGS   =
LDLIBS    =

OBJS      = termbox.o ti.o sgr.o utf8.o tkbd.o
SO_NAME   = libtermbox.so
SA_NAME   = termbox.sa
LIBS      = $(SO_NAME) $(SA_NAME)

DEMO_OBJS = demo/keyboard.o demo/output.o demo/paint.o demo/capdump.o
DEMO_CMDS = demo/keyboard demo/output demo/paint demo/capdump

TESTS    = test/ti_load_test test/ti_getcaps_test test/ti_parm_test \
           test/sgr_unpack_test test/sgr_encode_test test/sgr_attrs_test \
           test/tkbd_test

AMAL_OBJS = amalgamation/termbox.o

# make profile=release (default)
# make profile=debug
# make profile=clang
profile = release
include build/$(profile).mk

# Build everything except the amalgamation sources
all: $(OBJS) $(LIBS) demo tests
.PHONY: all

# Rebuild termbox.o when deps change
termbox.o: termbox.h bytebuffer.inl term.inl input.inl

# Shared and static libraries
$(SO_NAME): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)
$(SA_NAME): $(OBJS)
	ar rcs $@ $(OBJS)

# Demo programs
DEMOS_CC = $(CC) $(CFLAGS) $(CFLAGS_EXTRA) $(OBJS) $(LDLIBS)
demo: $(DEMO_CMDS)
demo/keyboard: demo/keyboard.o $(OBJS)
	$(DEMOS_CC) $@.o -o $@
demo/output: demo/output.o $(OBJS)
	$(DEMOS_CC) $@.o -o $@
demo/paint: demo/paint.o $(OBJS)
	$(DEMOS_CC) $@.o -o $@
demo/capdump: demo/capdump.o $(OBJS)
	$(DEMOS_CC) $@.o -o $@

# Test programs
TEST_CC = $(CC) $(CFLAGS) $(CFLAGS_EXTRA) -Wno-missing-field-initializers $(LDFLAGS)
tests: $(TESTS)
test/ti_load_test: test/ti_load_test.c
	$(TEST_CC) $< -o $@
test/ti_getcaps_test: test/ti_getcaps_test.c
	$(TEST_CC) $< -o $@
test/ti_parm_test: test/ti_parm_test.c
	$(TEST_CC) $< -o $@
test/sgr_unpack_test: test/sgr_unpack_test.c
	$(TEST_CC) $< -o $@
test/sgr_encode_test: test/sgr_encode_test.c
	$(TEST_CC) $< -o $@
test/sgr_attrs_test: test/sgr_attrs_test.c
	$(TEST_CC) $< -o $@
test: tests
	test/runtest $(TESTS)
.PHONY: test

# Targets for building the single file source library.
# The termbox.c and .h files can be copied directly into
amalgamation: $(AMAL_OBJS)
amalgamation/termbox.c: termbox.c bytebuffer.inl term.inl input.inl utf8.c
	mkdir -p amalgamation
	awk <termbox.c >$@ -F '"' \
	    '/^#include .*\.inl/ { system("cat " $$2); next }; { print }'
	grep >>$@ -v '^#include "termbox\.h"' utf8.c
amalgamation/termbox.h: termbox.h
	cat $< >$@
amalgamation/termbox.o: amalgamation/termbox.c amalgamation/termbox.h
	$(CC) $(CFLAGS) -c amalgamation/termbox.c -o $@
.PHONY: amalgamation

# Clean everything
clean:
	rm -f $(DEMO_OBJS)
	rm -f $(DEMO_CMDS)
	rm -f $(OBJS)
	rm -f $(LIBS)
	rm -f $(AMAL_OBJS)
	rm -f $(TESTS)
.PHONY: clean

# Implicit rule to build object files from .c source files
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -c $< -o $@

tags: # ignore amalgamation sources
	ctags -R --exclude='*amalgamation*' --totals
.PHONY: tags

Caps: # fetch latest curses Caps file from GitHub
	curl -o $@ https://raw.githubusercontent.com/ThomasDickey/ncurses-snapshots/master/include/Caps
.PHONY: Caps
