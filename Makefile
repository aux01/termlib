.POSIX:
.SUFFIXES:

CC        = cc
CFLAGS    = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Os -fpic
LDFLAGS   =

OBJS      = termbox.o utf8.o
SO_NAME   = libtermbox.so
SA_NAME   = termbox.sa
LIBS      = $(SO_NAME) $(SA_NAME)

DEMO_OBJS = demo/keyboard.o demo/output.o demo/paint.o
DEMO_CMDS = demo/keyboard demo/output demo/paint

all: $(OBJS) $(LIBS) $(DEMO_OBJS) $(DEMO_CMDS)

$(SO_NAME): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

$(SA_NAME): $(OBJS)
	ar rcs $@ $(OBJS)

demo/keyboard: demo/keyboard.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@

demo/output: demo/output.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@

demo/paint: demo/paint.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $@.o -o $@

clean:
	rm -f $(DEMO_OBJS)
	rm -f $(DEMO_CMDS)
	rm -f $(OBJS)
	rm -f $(LIBS)

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
