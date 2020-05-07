# Termbox

Termbox is a minimalistic C library for writing text-based user interfaces.

This is a fork of the original [nsf/termbox][og] that seeks to refocus,
reorganize, and add some refinements to the project.

 - [x] Replace waf-based build with simple Makefile.
 - [x] Remove Python extension related components. This is a C library only.
 - [x] Add an amalgamation build for packaging as a [Single-File Library][sfl].
 - [x] Add support for italic, faint, and crossed-out character styles.
 - [x] Fix bug loading terminfo files that leave capabilities undefined (blink)
 - [x] Don't use terminfo for SGR color, bold, italic, underline, etc.
   (See [fb2b0c3][fb2] for explanation)
 - [x] Split out SFL for loading terminfo capabilities.
   (See [tbti.h](https://github.com/aux01/termbaux/blob/master/tbti.h))
 - [x] Split out SFL for generating SGR sequences.
   (See [tbsgr.h](https://github.com/aux01/termbaux/blob/master/tbti.h))
 - [ ] Split out SFL for keyboard and mouse input handling.
 - [ ] Support for scrolling buffer (rin/ind) and/or insert/delete line.

The fork is not binary compatible with the original termbox library but seeks to
maintain source compatibility. In most cases, existing Termbox programs should
be able to switch to Termbaux by changing a couple includes and linker flags.

[og]:  https://github.com/nsf/termbox
[sfl]: https://github.com/nothings/single_file_libs
[fb2]: https://github.com/aux01/termbaux/commit/fb2b0c3b6fd2897a3ec5b52a72497ca7ff0fcffe

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
