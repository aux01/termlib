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
 *
 */

#pragma once

#include <stdint.h>
#include <stdio.h>

/*
 * SGR struct
 *
 * The struct is designed to pack all possible typographic attributes and color
 * information into 64 bits, making it practical to store sgr information for
 * each cell in a terminal display. An array holding sgr information for each
 * cell in a 100x100 terminal would occupy about 80KB memory.
 *
 * Note that each member is represented as a 64bit integer but packed down to
 * 16 bits for typographic attributes and 24 bits for each color.
 *
 */
struct sgr {
        uint64_t at : 16; // attributes bitflags (defined below)
        uint64_t fg : 24; // foreground color
        uint64_t bg : 24; // background color
};

/*
 * Basic 8-color mode colors.
 *
 * The SGR_DEFAULT color specifies the terminal's default foreground or
 * background color, which may be different from color 0 / color 7.
 *
 */
#define SGR_BLACK      0x00
#define SGR_RED        0x01
#define SGR_GREEN      0x02
#define SGR_YELLOW     0x03
#define SGR_BLUE       0x04
#define SGR_MAGENTA    0x05
#define SGR_CYAN       0x06
#define SGR_WHITE      0x07
#define SGR_DEFAULT    0x09

/*
 * SGR attributes are defined as bitflags below and combined to specify
 * rendering features.  These MASKs are used to extract ranges of flags from
 * the 16-bit attribute integer.
 *
 * All attributes are stored in the least significant 16-bits and look like this
 * in binary little endian:
 *
 *         nrbbbfffS.RLUIFB (flag)
 *         1111111100000000 (octal position)
 *         7654321076543210
 *
 * B=bold, F=faint, I=italic, U=underline, L=blink, R=reverse, S=strike
 * f=foreground color mode, b=background color mode
 * r=reset, n=negate
 * .=unused
 *
 */
#define SGR_ATTR_MASK  0x00ff   // all typographic attribute bits
#define SGR_FG_MASK    0x0700   // all fg color mode selection bits
#define SGR_BG_MASK    0x3800   // all bg color mode selection bits
#define SGR_CTRL_MASK  0xc000   // all control bits

/*
 * Typographic and cell display attributes
 *
 * These control typographical aspects of the text and character cell and may be
 * combined in any configuration:
 *
 *     struct sgr seq = { SGR_BOLD|SGR_UNDERLINE|SGR_ITALIC|SGR_REVERSE }
 *
 */
#define SGR_BOLD       0x0001   // text is bold
#define SGR_FAINT      0x0002   // text is faint or dim
#define SGR_ITALIC     0x0004   // text is rendered in italic font
#define SGR_UNDERLINE  0x0008   // text is underlined
#define SGR_BLINK      0x0010   // cell is blinking
#define SGR_REVERSE    0x0020   // cell colors are reversed
#define SGR_CONCEAL    0x0040   // TODO: cell is concealed
#define SGR_STRIKE     0x0080   // text is strike-through

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
 *     struct sgr seq = { SGR_BOLD|SGR_FG|SGR_BG16, SGR_CYAN, SGR_YELLOW }
 *
 */
#define SGR_FG         0x0100   // fg is normal 8-color mode color
#define SGR_FG16       0x0200   // fg is bright 16-color mode color
#define SGR_FG24       0x0300   // fg is 24-color greyscale color
#define SGR_FG216      0x0400   // fg is 216-color mode color
#define SGR_FG256      0x0500   // fg is 256-color mode color
#define SGR_FG16M      0x0600   // fg is 16M-color "true color" mode color

#define SGR_BG         0x0800   // bg is normal 8-color mode color
#define SGR_BG16       0x1000   // bg is bright 16-color mode color
#define SGR_BG24       0x1800   // bg is 24-color greyscale color
#define SGR_BG216      0x2000   // bg is 216-color mode color
#define SGR_BG256      0x2800   // bg is 256-color mode color
#define SGR_BG16M      0x3000   // bg is 16M-color "true color" mode color

/*
 * Render control attributes
 *
 * When the SGR_RESET attribute is set, all attributes are reset to their
 * default values before current attributes are applied.
 *
 * When the SGR_NEGATE attribute is set, all set attributes are reset to their
 * default values. i.e., SGR_NEGATE turns attributes off.
 */
#define SGR_INHERIT    0x0000   // inherit unset attributes (default)
#define SGR_RESET      0x4000   // sgr0 all attribute reset before applying
#define SGR_NEGATE     0x8000   // turn all set attrs off instead of on

/*
 * Fixed buffer size constants
 *
 * Use SGR_STR_MAX when allocating char buffers for sgr_str().
 */
#define SGR_STR_MAX  256  // max bytes in a single SGR sequence string
#define SGR_ELMS_MAX 32   // max number of format codes in a SGR sequence

/*
 * SGR sequence construction strings
 *
 */
#define SGR_OPEN     "\033["
#define SGR_CLOSE    "m"
#define SGR_SEP      ";"

/*
 * Write SGR attributes as an escape sequence to the char buffer pointed to
 * by dest. There must be SGR_STR_MAX bytes available after the dest pointer
 * or a buffer overflow could occur.
 *
 * Returns the number of bytes written.
 */
int sgr_str(char *dest, struct sgr);

/*
 * Write SGR attributes as an escape sequence to a file descriptor.
 * Behaves similar to write(2).
 *
 * Returns number of bytes written if successful, -1 on error.
 * Error information is available via errno(2).
 */
int sgr_write(int fd, struct sgr);

/*
 * Write SGR attributes as an escape sequence to a FILE stream.
 * Behaves similar to fwrite(2).
 *
 * Returns number of bytes written if successful, -1 on error.
 * Error information should be available via ferror(3) and feof(3).
 */
int sgr_fwrite(FILE *stream, struct sgr);

/* Generic SGR value encoder. Takes a pointer to anything and a function that
 * will be called with the pointer any time chars should be emitted. This is
 * used to implement the sgr_str, sgr_write, and sgr_fwrite functions and may
 * be useful if you're writing to a unique output medium.
 *
 * Returns the number of bytes sent to func.
 */
int sgr_encode(void *p, void (*func)(void *, char *, int), struct sgr);

/*
 * Unpack a SGR value into an array of int formatting codes.
 * The codes buffer should be allocated to hold up to SGR_ELMS_MAX elements.
 *
 * Returns the number of formatting code ints written to the codes buffer.
 */
int sgr_unpack(uint16_t codes[], struct sgr);
