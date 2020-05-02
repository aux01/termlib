.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -D_XOPEN_SOURCE -Os -fpic
OBJECTS = termbox.o utf8.o
LIBS    = termbox.so termbox.sa

all: $(OBJECTS)

termbox.so: $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

termbox.sa: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f $(LIBS)

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

