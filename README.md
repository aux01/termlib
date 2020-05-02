# Termbox

Termbox is a minimalistic C library for writing text-based user interfaces.

This is a fork of the original [nsf/termbox][og] that seeks to refocus,
reorganize, and add some refinements to the project.

 - [x] Replace waf-based build with simple Makefile.
 - [x] Remove Python extension related components. This is a C library only.
 - [x] Add an amalgamation build for packaging as a [single-file library][sfl].
 - [ ] Add support for italics, faint, blink, and reverse.
 - [ ] Reorganize source files into terminfo.c, terminput.c, and termbuffer.c
   with separate header files such that any could be used as single-file libs.

It's not yet clear whether the fork will end up being binary compatible with the
original but maintaining source compatibility is a goal.

[og]:  https://github.com/nsf/termbox
[sfl]: https://github.com/nothings/single_file_libs

### Basic interface

Termbox's interface only consists of 12 functions::

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

See the `termbox.h` file for full details.

### Links

The original termbox library has a large number of languages bindings and clones:

- https://github.com/nsf/termbox - Python
- https://github.com/adsr/termbox-php - PHP
- https://github.com/gchp/rustbox - Rust
- https://github.com/fouric/cl-termbox - Common Lisp
- https://github.com/zyedidia/termbox-d - D
- https://github.com/dduan/Termbox - Swift
- https://github.com/andrewsuzuki/termbox-crystal - Crystal
- https://github.com/jgoldfar/Termbox.jl - Julia
- https://github.com/mitchellwrosen/termbox - Haskell
- https://github.com/dom96/nimbox - Nim
- https://github.com/ndreynolds/ex_termbox - Elixir
- https://github.com/nsf/termbox-go - Go pure Termbox implementation

Please see [nsf/termbox][og] for the original changelog and more.
