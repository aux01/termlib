# Termlib

*termlib* is a collection of clean, modern, single-file C libraries for
building Unix terminal programs.

Termlib started as a fork of [Termbox][] and continues in the spirit of being a
lightweight, approachable alternative to ncurses programming. Unlike Termbox,
termlib aims to provide library routines useful for many types of
terminal-enabled programs, not just curses-style full screen TUIs.

### Project status

Under heavy development and reorganization.
Serious use is not recommended at this time.

### Libraries

#### tkbd.h

The tkbd library decodes ECMA-48/VT/xterm keyboard, mouse, and UTF-8 character
sequences into a simple struct. It can be used to remove most of the tedium
involved with supporting the myriad encoding schemes employed by different
popular terminal emulators.

[Usage][tkbd.h]

#### sgr.h

The sgr library includes data structures and routines for constructing
ANSI/ECMA-48 Select Graphic Render (SGR) sequences. It supports all typographic
attributes——<b>bold</b>, faint, <i>italic</i>, <u>underline</u>,
<blink>blink</blink>, <del>cross-out</del>, and reverse——as well background and
foreground colors in 8-color, 16-color, 24-color greyscale, 216-color,
256-color, and 16M true color modes.

[Usage][sgr.h]

#### ti.h

The ti library is a standalone [terminfo(5)][terminfo] processor. It can query
the terminfo database for terminal capabilities and generate escape sequences
without ncurses.

[Usage][ti.h]

#### utf8.h

The utf8 library is a bare bones utf8 encoder / decoder with some useful utility
routines and constants. It supports the minimum baseline of features all
terminal programs should be ready to support.

[Usage][utf8.h]

### Acknowledgements

This project was originally forked from [Termbox][] by nsf and contributors.

The [ti.c][] parameter processing logic is based on the Golang [TCell
implementation][tcell] of the same by Garrett D'Amore and contributors.


[termbox]:  https://github.com/nsf/termbox
[sfl]:      https://github.com/nothings/single_file_libs
[tcell]:    https://github.com/gdamore/tcell/blob/master/terminfo/terminfo.go
[terminfo]: https://pubs.opengroup.org/onlinepubs/007908799/xcurses/terminfo.html

[ti.h]:     https://github.com/aux01/tbaux/blob/master/ti.h
[ti.c]:     https://github.com/aux01/tbaux/blob/master/ti.c
[sgr.h]:    https://github.com/aux01/tbaux/blob/master/sgr.h
[sgr.c]:    https://github.com/aux01/tbaux/blob/master/sgr.c
[tkbd.h]:   https://github.com/aux01/tbaux/blob/master/tkbd.h
[tkbd.c]:   https://github.com/aux01/tbaux/blob/master/tkbd.c
[utf8.h]:   https://github.com/aux01/tbaux/blob/master/utf8.h
[utf8.c]:   https://github.com/aux01/tbaux/blob/master/utf8.c
