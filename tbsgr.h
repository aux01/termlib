/*
 *
 * tbsgr.h
 *
 * Generate ANSI/ECMA-48/VT10x escape sequences for controlling typographic
 * features like bold, italic, underline, faint, blink, reverse, and cross-out;
 * as well as foreground and background colors in a variety of color modes.
 *
 * This library generates SGR escape sequences only. It does not attempt to
 * query terminfo for terminal capability strings.
 *
 */

#include <stdint.h>
#include <stdio.h>

/*
 * Foreground/text attributes.
 *
 * These control typographical aspects of the foreground and may be combined
 * with a foreground color and color mode.
 *
 * TB_BRIGHT | TB_RED | TB_ITALIC
 *
 */
#define TB_FG        0x00000000   // color applies to the foreground (default)
#define TB_BOLD      0x00010000   // text is bold
#define TB_FAINT     0x00020000   // text is faint or dim
#define TB_ITALIC    0x00040000   // text is rendered in italic font
#define TB_UNDERLINE 0x00080000   // text is underlined
#define TB_STRIKE    0x00100000   // text is strike-through or crossed-out
#define TB_FG_MASK   0x001f0000   // all foreground attr bits

/*
 * Background attributes.
 *
 * These control rendering of the background and may be combined with a
 * background color.
 */
#define TB_BG        0x00200000   // color applies to the background
#define TB_BLINK     0x00400000   // cell is blinking
#define TB_REVERSE   0x00800000   // cell is reversed
#define TB_BG_MASK   0x00e00000   // all background attr bits

/*
 * Color mode attributes.
 *
 * These select the colormode for the first 16-bits.
 *
 * How the value at 0xFFFF is interpreted is based on these bits.
 *
 */
#define TB_8         0x00000000   // color is 4-bit/8-color mode (default)
#define TB_BRIGHT    0x01000000   // color is bright 8-color mode
#define TB_216       0x02000000   // color is 8-bit/216-color mode
#define TB_256       0x03000000   // color is 8-bit/256-color mode
#define TB_GREYSCALE 0x04000000   // color is 7-bit/24-color greyscale mode
#define TB_HIGH      0x05000000   // color is 16-bit/65536-color mode
#define TB_CM_MASK   0x07000000   // all color mode bits

/*
 * Render control attributes.
 *
 */
#define TB_INHERIT   0x00000000   // inherit unset attributes (default)
#define TB_RESET     0x10000000   // sgr0 all attribute reset before applying
#define TB_NEGATE    0x20000000   // turn on/off attrs off instead of on
#define TB_COLOR     0x40000000   // set a color
#define TB_CTRL_MASK 0x70000000   // all control bits

/*
 * Basic 8-color mode colors.
 *
 */
#define TB_BLACK      (0x00000000 | TB_COLOR)
#define TB_RED        (0x00000001 | TB_COLOR)
#define TB_GREEN      (0x00000002 | TB_COLOR)
#define TB_YELLOW     (0x00000003 | TB_COLOR)
#define TB_BLUE       (0x00000004 | TB_COLOR)
#define TB_MAGENTA    (0x00000005 | TB_COLOR)
#define TB_CYAN       (0x00000006 | TB_COLOR)
#define TB_WHITE      (0x00000007 | TB_COLOR)
#define TB_DEFAULT    (0x00000009 | TB_COLOR)
#define TB_COLOR_MASK 0x0000ffff

/*
 * Fixed buffer size constants.
 *
 */
#define TB_SGR_ELMS_MAX 16   // max number of format codes in a SGR sequence
#define TB_SGR_STR_MAX  128  // max bytes in a single SGR sequence string

/*
 * SGR sequence construction strings.
 *
 */
#define TB_SGR_OPEN            "\x1b["
#define TB_SGR_CLOSE           "m"
#define TB_SGR_SEP             ";"

/*
 * Write SGR attributes as an escape sequence to a file-like object.
 * tb_sgr_write() is like write(2); tb_sgr_fwrite() is like fwrite(3).
 *
 * Returns lengths and error information like to write(2) and fwrite(3).
 */
int      tb_sgr_write(int fd, uint32_t attrs);
unsigned tb_sgr_fwrite(FILE *stream, uint32_t attrs);

/*
 * Write SGR attributes as an escape sequence to the char buffer pointed to
 * by dest. There must be TB_SGR_STR_MAX bytes available after dest or a buffer
 * overflow could occur.
 *
 * Returns the number of bytes written after dest.
 */
int tb_sgr_strcpy(char *dest, uint32_t attrs);

/* Generic SGR value encoder. Takes a pointer to anything and a function that
 * will be called with the pointer any time chars should be emitted. This is
 * used to implement the tb_sgr_write, tb_sgr_fwrite, and tb_sgr_strcpy
 * functions and may be useful if you're writing to a custom buffer object.
 *
 * Returns the number of bytes sent to func.
 */
int tb_sgr_encode(void *p, void (*func)(void *, char *, int), uint32_t attrs);

/*
 * Encode the SGR value into an array of int formatting codes.
 * The codes buffer should be able to hold up to TB_SGR_ELMS_MAX.
 *
 * Returns the number of formatting code ints written to the codes buffer.
 */
int tb_sgr_ints(uint16_t codes[], uint32_t attrs);
