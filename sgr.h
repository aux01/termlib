/*
 *
 * sgr.h - Select Graphic Rendition (SGR) escape sequence generation library
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * Generate ANSI/ECMA-48/VT escape sequences for controlling typographic
 * features--like bold, italic, underline, faint, blink, reverse, and cross-out;
 * as well as foreground and background colors--in a variety of color modes.
 *
 * This library generates SGR escape sequences only. It does not attempt to
 * query terminfo for terminal capability strings.
 *
 */

#include <stdint.h>
#include <stdio.h>

/*
 * SGR struct
 *
 */
typedef struct sgr {
        int32_t  at; // attributes bitflags (defined below)
        int16_t  fg; // foreground color
        int16_t  bg; // background color
} sgr_t;

/*
 * Basic 8-color mode colors.
 *
 * The SGR_DEFAULT color specifies the terminal's default foreground or
 * background color, which may be different from color 0 / color 7.
 *
 */
#define SGR_BLACK      0x0000
#define SGR_RED        0x0001
#define SGR_GREEN      0x0002
#define SGR_YELLOW     0x0003
#define SGR_BLUE       0x0004
#define SGR_MAGENTA    0x0005
#define SGR_CYAN       0x0006
#define SGR_WHITE      0x0007
#define SGR_DEFAULT    0x0009

/*
 * SGR attributes are defined as bitflags below and combined to specify
 * rendering features.
 *
 * The following MASKs can be used to extract ranges of flags from
 * the 32-bit attribute integer.
 *
 * BFIUL.R.S.......ffffbbbbrn.............
 *
 */
#define SGR_ATTR_MASK  0x000001ff   // all typographic attribute bits
#define SGR_FG_MASK    0x000f0000   // all fg color mode selection bits
#define SGR_BG_MASK    0x00f00000   // all bg color mode selection bits
#define SGR_CTRL_MASK  0x03000000   // all control bits

/*
 * Typographic and cell display attributes
 *
 * These control typographical aspects of the text and character cell and may be
 * combined in any configuration:
 *
 *     sgr_t seq = { SGR_BOLD|SGR_UNDERLINE|SGR_ITALIC|SGR_REVERSE }
 *
 */
#define SGR_BOLD       0x00000001   // text is bold
#define SGR_FAINT      0x00000002   // text is faint or dim
#define SGR_ITALIC     0x00000004   // text is rendered in italic font
#define SGR_UNDERLINE  0x00000008   // text is underlined
#define SGR_BLINK      0x00000010   // cell is blinking
#define SGR_REVERSE    0x00000040   // cell colors are reversed
#define SGR_STRIKE     0x00000100   // text is strike-through

/*
 * Color mode attributes
 *
 * These control whether the foreground and background colors are applied and
 * the color mode palette of the color.
 *
 * IMPORTANT: No color sequences are generated unless one of the color modes
 * is specified for both the foreground and background colors.
 *
 * Example SGR sequence with bold cyan text on a bright yellow background:
 *
 *     sgr_t seq = { SGR_BOLD|SGR_FG|SGR_BG16, SGR_CYAN, SGR_YELLOW }
 *
 */
#define SGR_FG         0x00010000   // fg is normal 8-color mode color
#define SGR_FG16       0x00020000   // fg is bright 16-color mode color
#define SGR_FG24       0x00040000   // fg is 24-color greyscale color
#define SGR_FG216      0x00060000   // fg is 216-color mode color
#define SGR_FG256      0x00070000   // fg is 256-color mode color
#define SGR_FG65K      0x00080000   // fg is 65K-color "high color" mode color

#define SGR_BG         0x00100000   // bg is normal 8-color mode color
#define SGR_BG16       0x00200000   // bg is bright 16-color mode color
#define SGR_BG24       0x00400000   // bg is 24-color greyscale color
#define SGR_BG216      0x00600000   // bg is 216-color mode color
#define SGR_BG256      0x00700000   // bg is 256-color mode color
#define SGR_BG65K      0x00800000   // bg is 65K-color "high color" mode color

/*
 * Render control attributes
 *
 * When the SGR_RESET attribute is set, all attributes are reset to their
 * default values before current attributes are applied.
 *
 * When the SGR_NEGATE attribute is set, all set attributes are reset to their
 * default values. i.e., SGR_NEGATE turns attributes off.
 */
#define SGR_INHERIT    0x00000000   // inherit unset attributes (default)
#define SGR_RESET      0x01000000   // sgr0 all attribute reset before applying
#define SGR_NEGATE     0x02000000   // turn all set attrs off instead of on

/*
 * Fixed buffer size constants
 *
 * Use SGR_STR_MAX when allocating char buffers for sgr_str().
 */
#define SGR_STR_MAX  128  // max bytes in a single SGR sequence string
#define SGR_ELMS_MAX 16   // max number of format codes in a SGR sequence

/*
 * SGR sequence construction strings
 *
 */
#define SGR_OPEN            "\x1b["
#define SGR_CLOSE           "m"
#define SGR_SEP             ";"

/*
 * Write SGR attributes as an escape sequence to the char buffer pointed to
 * by dest. There must be SGR_STR_MAX bytes available after the dest pointer
 * or a buffer overflow could occur.
 *
 * Returns the number of bytes written.
 */
int sgr_str(char *dest, sgr_t sgr);

/*
 * Write SGR attributes as an escape sequence to a file descriptor.
 * Behaves similar to write(2).
 *
 * Returns number of bytes written if successful, -1 on error.
 * Error information is available via errno(2).
 */
int sgr_write(int fd, sgr_t sgr);

/*
 * Write SGR attributes as an escape sequence to a FILE stream.
 * Behaves similar to fwrite(2).
 *
 * Returns number of bytes written if successful, -1 on error.
 * Error information should be available via ferror(3) and feof(3).
 */
int sgr_fwrite(FILE *stream, sgr_t sgr);

/* Generic SGR value encoder. Takes a pointer to anything and a function that
 * will be called with the pointer any time chars should be emitted. This is
 * used to implement the sgr_str, sgr_write, and sgr_fwrite functions and may
 * be useful if you're writing to a unique output medium.
 *
 * Returns the number of bytes sent to func.
 */
int sgr_encode(void *p, void (*func)(void *, char *, int), sgr_t sgr);

/*
 * Unpack a SGR value into an array of int formatting codes.
 * The codes buffer should be allocated to hold up to SGR_ELMS_MAX elements.
 *
 * Returns the number of formatting code ints written to the codes buffer.
 */
int sgr_unpack(uint16_t codes[], sgr_t sgr);
