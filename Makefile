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

DEMO_OBJS = demo/keyboard.o demo/output.o demo/paint.o demo/capdump.o demo/pkbd.o
DEMO_CMDS = demo/keyboard demo/output demo/paint demo/capdump demo/pkbd

TESTS     = test/ti_load_test test/ti_getcaps_test test/ti_parm_test \
            test/sgr_unpack_test test/sgr_encode_test test/sgr_attrs_test \
            test/tkbd_parse_test test/tkbd_desc_test test/tkbd_stresc_test

# make profile=release (default)
# make profile=debug
# make profile=clang
profile = release
include build/$(profile).mk

# Build everything
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
$(DEMO_CMDS):
	$(DEMOS_CC) $@.o -o $@
demo/keyboard: demo/keyboard.o $(OBJS)
demo/output: demo/output.o $(OBJS)
demo/paint: demo/paint.o $(OBJS)
demo/capdump: demo/capdump.o $(OBJS)
demo/pkbd: demo/pkbd.o tkbd.o
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
tests: $(TESTS)
test: tests
	test/runtest $(TESTS)
.PHONY: test tests

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
