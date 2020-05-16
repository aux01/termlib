# tbaux

*tbaux* is a minimal C library for writing text-based user interfaces.

This is a fork of [termbox][og] that seeks to refocus, reorganize, and add some
refinements to the project. tbaux is mostly source compatible with termbox.
Existing termbox programs should run under tbaux with only modest changes
to include lines and linker flags.

[og]: https://github.com/nsf/termbox

### Project status

Under heavy development and reorganization.
Serious use is not recommended at this time.

Major planned and completed changes:

 - [x] Replace waf-based build with simple Makefile.
 - [x] Remove Python extension related components. This is a C library only.
 - [x] Add an amalgamation build for packaging as a [Single-File Library][sfl].
 - [x] Add support for _italic_, faint, and <del>crossed-out</del> text.
 - [x] Fix bug loading terminfo files that leave capabilities undefined (blink)
 - [x] Don't use terminfo for SGR color, bold, italic, underline, etc.
   (See [fb2b0c3][fb2] for explanation)
 - [x] Split out SFL for loading terminfo capabilities.
   (See [ti.h](https://github.com/aux01/tbaux/blob/master/ti.h))
 - [x] Split out SFL for generating SGR sequences.
   (See [sgr.h](https://github.com/aux01/tbaux/blob/master/sgr.h))
 - [ ] Split out SFL for keyboard and mouse input handling.
 - [x] Standalone terminfo reading library, including parameter string processing.
 - [ ] Support for scrolling buffer (rin/ind) and/or insert/delete line.

[sfl]: https://github.com/nothings/single_file_libs
[fb2]: https://github.com/aux01/tbaux/commit/fb2b0c3b6fd2897a3ec5b52a72497ca7ff0fcffe

### Basic interface

The interface consists of only 12 functions::

```
tb_init()              // initialization
tb_shutdown()          // shutdown

tb_width()             // width of the terminal screen
tb_height()            // height of the terminal screen

tb_clear()             // clear buffer
tb_present()           // sync internal buffer with terminal

tb_put_cell()
tb_change_cell()
tb_blit()              // drawing functions

tb_select_input_mode() // change input mode
tb_peek_event()        // peek a keyboard event
tb_poll_event()        // wait for a keyboard event
```

See the [termbox.h][] file for more details.

[termbox.h]: https://github.com/aux01/tbaux/blob/master/termbox.h

### Acknowledgements

This project was originally forked from [Termbox][og] by nsf and contributors.

The [ti.c][] parameter processing logic is based on the Golang [TCell
implementation][tcell] of the same by Garrett D'Amore and contributors.

[ti.c]:  https://github.com/aux01/tbaux/blob/master/ti.c
[tcell]: https://github.com/gdamore/tcell/blob/master/terminfo/terminfo.go
