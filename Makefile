.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Os -fpic
LDFLAGS   =

OBJS      = termbox.o utf8.o tbti.o tbsgr.o
SO_NAME   = libtermbox.so
SA_NAME   = termbox.sa
LIBS      = $(SO_NAME) $(SA_NAME)

DEMO_OBJS = demo/keyboard.o demo/output.o demo/paint.o
DEMO_CMDS = demo/keyboard demo/output demo/paint

AMAL_OBJS = amalgamation/termbox.o

# Build everything except the amalgamation sources
all: $(OBJS) $(LIBS) $(DEMO_OBJS) $(DEMO_CMDS)
.PHONY: all

# Rebuild termbox.o when deps change
termbox.o: termbox.h bytebuffer.inl term.inl input.inl

# Shared and static libraries
$(SO_NAME): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)
$(SA_NAME): $(OBJS)
	ar rcs $@ $(OBJS)

# Demo programs
demo/keyboard: demo/keyboard.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@
demo/output: demo/output.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@
demo/paint: demo/paint.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@

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

test:
	$(MAKE) -C test clean all
.PHONY: test

# Clean everything
clean:
	rm -f $(DEMO_OBJS)
	rm -f $(DEMO_CMDS)
	rm -f $(OBJS)
	rm -f $(LIBS)
	rm -f $(AMAL_OBJS)
.PHONY: clean

# Implicit rule to build object files from .c source files
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

tags: # ignore amalgamation sources
	ctags -R --exclude='*amalgamation*' --totals
.PHONY: tags

Caps: # fetch latest curses Caps file from GitHub
	curl -o $@ https://raw.githubusercontent.com/ThomasDickey/ncurses-snapshots/master/include/Caps
.PHONY: Caps
