# Termlib Makefile
.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 $(INCLUDE) $(WARN) $(OPTIMIZE) -fPIC
WARN      = -Wall -Wextra
OPTIMIZE  = -O2
INCLUDE   = -iquote termbox -iquote .
LDFLAGS   =
LDLIBS    =

OBJS      = sgr.o ti.o tkbd.o utf8.o termbox/termbox.o
SO_NAME   = libtermlib.so
SA_NAME   = termlib.sa
LIBS      = $(SO_NAME) $(SA_NAME)

DEMO_OBJS = demo/keyboard.o demo/output.o demo/paint.o demo/capdump.o demo/pkbd.o
DEMO_CMDS = demo/keyboard demo/output demo/paint demo/capdump demo/pkbd

TESTS     = test/ti_load_test test/ti_getcaps_test test/ti_parm_test \
            test/sgr_unpack_test test/sgr_encode_test test/sgr_attrs_test \
            test/tkbd_parse_test test/tkbd_desc_test test/tkbd_stresc_test \
            test/utf8_test

# make profile=release (default)
# make profile=debug
# make profile=clang
profile = release
include build/$(profile).mk

# Build everything
all: $(OBJS) $(LIBS) demo $(TESTS)
.PHONY: all

# Main objects and their dependencies
sgr.o: sgr.h
ti.o: ti.h
tkbd.o: tkbd.h
utf8.o: utf8.h

# Termbox compatibility
termbox/termbox.o: termbox/termbox.h termbox/bytebuffer.inl termbox/term.inl termbox/input.inl

# Shared and static libraries
$(SO_NAME): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)
$(SA_NAME): $(OBJS)
	ar rcs $@ $(OBJS)

# Demo programs
demo/keyboard: demo/keyboard.o $(OBJS)
demo/output: demo/output.o $(OBJS)
demo/paint: demo/paint.o $(OBJS)
demo/capdump: demo/capdump.o $(OBJS)
demo/pkbd: demo/pkbd.o $(OBJS)
demo: $(DEMO_CMDS)
.PHONY: demo

# Test programs
TEST_CC = $(CC) $(CFLAGS) $(CFLAGS_EXTRA) -Wno-missing-field-initializers $(LDFLAGS)
$(TESTS):
	$(TEST_CC) $< -o $@
test/ti_load_test: test/ti_load_test.c ti.c ti.h
test/ti_getcaps_test: test/ti_getcaps_test.c  ti.c ti.h
test/ti_parm_test: test/ti_parm_test.c ti.c ti.h
test/sgr_unpack_test: test/sgr_unpack_test.c sgr.c sgr.h
test/sgr_encode_test: test/sgr_encode_test.c sgr.c sgr.h
test/sgr_attrs_test: test/sgr_attrs_test.c sgr.c sgr.h
test/tkbd_parse_test: test/tkbd_parse_test.c tkbd.c tkbd.h
test/tkbd_desc_test: test/tkbd_desc_test.c tkbd.c tkbd.h
test/tkbd_stresc_test: test/tkbd_stresc_test.c tkbd.c tkbd.h
test/utf8_test: test/utf8_test.c utf8.c utf8.h
test: $(TESTS)
	test/runtest $(TESTS)
.PHONY: test

# Clean everything
clean:
	rm -f $(DEMO_OBJS)
	rm -f $(DEMO_CMDS)
	rm -f $(OBJS)
	rm -f $(LIBS)
	rm -f $(TESTS)
.PHONY: clean

# Implicit rule to build object files from .c source files
.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -c $< -o $@

tags:
	ctags -R --totals
.PHONY: tags

Caps: # fetch latest curses Caps file from GitHub
	curl -o $@ https://raw.githubusercontent.com/ThomasDickey/ncurses-snapshots/master/include/Caps
.PHONY: Caps
