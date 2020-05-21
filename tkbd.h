/*
 *
 * tkbd.h - Terminal keyboard, mouse, and character input library
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * Read input from a terminal with support for decoding special key sequences,
 * mouse events, and utf8 character data.
 *
 *
 */

#pragma once

#include <stdint.h>
#include <stddef.h>            // size_t
#include <termios.h>           // struct termios

/*
 * Limits
 */
#define TKBD_SEQ_MAX 32         // max length in bytes of an escape sequence

/*
 * Keyboard, mouse, or unicode character event structure.
 *
 * The tkbd_parse() and tkbd_read() functions fill this structure with
 * information consumed from a char buffer or file descriptor.
 */
struct tkbd_event {
	uint8_t  type;          // event type
	uint8_t  mod;           // modifiers
	uint16_t key;           // one of the TKBD_KEY_* constants
	uint32_t ch;            // unicode character
	int32_t  x, y;          // mouse coordinates
	char seq[TKBD_SEQ_MAX]; // char sequence source of event
};

/*
 * Parse a single keyboard/mouse/resize sequence or UTF8 encoded character from
 * the buffer pointed to by buf and fill the event structure pointed to by ev
 * with information. No more than sz bytes will be read from buf.
 *
 * Returns the number of bytes read from buf when the event structure is filled.
 * Returns 0 when not enough data is available to decode an event.
 */
int tkbd_parse(struct tkbd_event *ev, const char *buf, size_t sz);

/*
 * Write a key description ("Ctrl+C", "Shift+Alt+PgUp", "Z", etc.) to the buffer
 * pointed to by buf. No more than sz bytes are written.
 *
 * Returns the number of bytes (excluding terminating null byte) needed to write
 * the description. No more than sz bytes will be written by the function but if
 * the return value is greater than sz, the description was truncated.
 */
int tkbd_desc(char *buf, size_t sz, const struct tkbd_event *ev);

/*
 * Keyboard input stream structure.
 *
 * Used with tkbd_read() to manage buffering and termios state.
 */
struct tkbd_stream {
	int  fd;                // file descriptor to read from
	char buf[1024];         // input buffer
	int  bufpos;            // current byte position buf
	int  buflen;            // number of bytes available after bufpos

	struct termios tc;      // original termios
};

/*
 * Attach a keyboard input stream structure to a file descriptor.
 * The file descriptor is put into raw mode and stream buffers are reset.
 *
 * Returns 0 on success.
 * Returns -1 on failure and sets errno appropriately.
 */
int tkbd_attach(struct tkbd_stream *s, int fd);

/*
 * Detach the keyboard input stream from the attached file descriptor.
 * This must be called on the stream before the program exits or the terminal
 * will remain in raw input mode. It's recommended this be set up to happen in
 * an atexit or signal hook.
 *
 * Returns 0 on success, -1 on failure and sets errno to indicate error.
 */
int tkbd_detach(struct tkbd_stream *s);

/*
 * Read a single keyboard, mouse, or UTF8 encoded character sequence from the
 * stream and fill the event structure pointed to by ev with information.
 *
 * Returns the number of bytes consumed to fill the event on success.
 * Returns 0 when not enough data is available to decode an event.
 * Returns -1 when a read error occurs and sets errno appropriately.
 */
int tkbd_read(struct tkbd_stream *s, struct tkbd_event *ev);


/*
 * Write an escaped version of a keyboard sequence to a character buffer.
 *
 * This is most often useful when printing the tkbd_event.seq member for display
 * since writing the raw characters to the terminal may be interpreted as
 * commands instead of text.
 *
 * The strsz argument specifies the length of the sequence string in bytes since
 * it may contain null characters.
 * The character buffer is assumed to be 4x strsz in bytes.
 *
 * Returns the number of bytes written to buf, excluding the null terminator.
 */
int tkbd_stresc(char *buf, const char *str, size_t strsz);


/*
 * Event types
 */
#define TKBD_KEY    1    // A key was pressed
#define TKBD_MOUSE  2    // Move, scroll, or button event

/*
 * Key modifier flags
 */
#define TKBD_MOD_NONE   0x00
#define TKBD_MOD_SHIFT  0x01
#define TKBD_MOD_ALT    0x02
#define TKBD_MOD_CTRL   0x04
#define TKBD_MOD_META   0x08
#define TKBD_MOD_MOTION 0x80

/*
 * Key constants
 *
 * Keys map to their ascii character equivalents where possible.
 * Special keys are mapped to unused parts of C0 control range.
 * Function keys are mapped to lower alpha range.
 *
 */
#define TKBD_KEY_UNKNOWN           0xFFFF

#define TKBD_KEY_BACKSPACE         0x08
#define TKBD_KEY_TAB               0x09
#define TKBD_KEY_ENTER             0x0A
#define TKBD_KEY_ESC               0x1B
#define TKBD_KEY_SPACE             0x20
#define TKBD_KEY_BACKSPACE2        0x7F

#define TKBD_KEY_UP                0x10
#define TKBD_KEY_DOWN              0x11
#define TKBD_KEY_RIGHT             0x12
#define TKBD_KEY_LEFT              0x13

#define TKBD_KEY_INS               0x14
#define TKBD_KEY_DEL               0x15
#define TKBD_KEY_PGUP              0x16
#define TKBD_KEY_PGDN              0x17
#define TKBD_KEY_HOME              0x18
#define TKBD_KEY_END               0x19

#define TKBD_KEY_DOUBLE_QUOTE      0x22
#define TKBD_KEY_QUOTE             0x27
#define TKBD_KEY_PLUS              0x2B
#define TKBD_KEY_COMMA             0x2C
#define TKBD_KEY_DASH              0x2D
#define TKBD_KEY_MINUS             0x2D
#define TKBD_KEY_PERIOD            0x2E
#define TKBD_KEY_SLASH             0x2F
#define TKBD_KEY_BANG              0x21
#define TKBD_KEY_POUND             0x23
#define TKBD_KEY_DOLLAR            0x24
#define TKBD_KEY_PERCENT           0x25
#define TKBD_KEY_AMP               0x26
#define TKBD_KEY_PAREN_LEFT        0x28
#define TKBD_KEY_PAREN_RIGHT       0x29
#define TKBD_KEY_STAR              0x2A

#define TKBD_KEY_0                 0x30
#define TKBD_KEY_1                 0x31
#define TKBD_KEY_2                 0x32
#define TKBD_KEY_3                 0x33
#define TKBD_KEY_4                 0x34
#define TKBD_KEY_5                 0x35
#define TKBD_KEY_6                 0x36
#define TKBD_KEY_7                 0x37
#define TKBD_KEY_8                 0x38
#define TKBD_KEY_9                 0x39

#define TKBD_KEY_COLON             0x3A
#define TKBD_KEY_SEMICOLON         0x3B
#define TKBD_KEY_LT                0x3C
#define TKBD_KEY_EQUAL             0x3D
#define TKBD_KEY_GT                0x3E
#define TKBD_KEY_QUESTION          0x3F
#define TKBD_KEY_AT                0x40

#define TKBD_KEY_A                 0x41
#define TKBD_KEY_B                 0x42
#define TKBD_KEY_C                 0x43
#define TKBD_KEY_D                 0x44
#define TKBD_KEY_E                 0x45
#define TKBD_KEY_F                 0x46
#define TKBD_KEY_G                 0x47
#define TKBD_KEY_H                 0x48
#define TKBD_KEY_I                 0x49
#define TKBD_KEY_J                 0x4A
#define TKBD_KEY_K                 0x4B
#define TKBD_KEY_L                 0x4C
#define TKBD_KEY_M                 0x4D
#define TKBD_KEY_N                 0x4E
#define TKBD_KEY_O                 0x4F
#define TKBD_KEY_P                 0x50
#define TKBD_KEY_Q                 0x51
#define TKBD_KEY_R                 0x52
#define TKBD_KEY_S                 0x53
#define TKBD_KEY_T                 0x54
#define TKBD_KEY_U                 0x55
#define TKBD_KEY_V                 0x56
#define TKBD_KEY_W                 0x57
#define TKBD_KEY_X                 0x58
#define TKBD_KEY_Y                 0x59
#define TKBD_KEY_Z                 0x5A

#define TKBD_KEY_BRACKET_LEFT      0x5B
#define TKBD_KEY_BACKSLASH         0x5C
#define TKBD_KEY_BRACKET_RIGHT     0x5D
#define TKBD_KEY_CARROT            0x5E
#define TKBD_KEY_UNDERSCORE        0x5F
#define TKBD_KEY_BACKTICK          0x60
#define TKBD_KEY_BACKQUOTE         0x60

#define TKBD_KEY_F1                0x61
#define TKBD_KEY_F2                0x62
#define TKBD_KEY_F3                0x63
#define TKBD_KEY_F4                0x64
#define TKBD_KEY_F5                0x65
#define TKBD_KEY_F6                0x67
#define TKBD_KEY_F7                0x68
#define TKBD_KEY_F8                0x69
#define TKBD_KEY_F9                0x6A
#define TKBD_KEY_F10               0x6B
#define TKBD_KEY_F11               0x6C
#define TKBD_KEY_F12               0x6D
#define TKBD_KEY_F13               0x6E
#define TKBD_KEY_F14               0x6F
#define TKBD_KEY_F15               0x71
#define TKBD_KEY_F16               0x72
#define TKBD_KEY_F17               0x74
#define TKBD_KEY_F18               0x75
#define TKBD_KEY_F19               0x76
#define TKBD_KEY_F20               0x77

#define TKBD_KEY_BRACE_LEFT        0x7B
#define TKBD_KEY_PIPE              0x7C
#define TKBD_KEY_BRACE_RIGHT       0x7D
#define TKBD_KEY_TILDE             0x7E

#define TKBD_MOUSE_LEFT           (0xFFFF-1)
#define TKBD_MOUSE_RIGHT          (0xFFFF-2)
#define TKBD_MOUSE_MIDDLE         (0xFFFF-3)
#define TKBD_MOUSE_RELEASE        (0xFFFF-4)
#define TKBD_MOUSE_WHEEL_UP       (0xFFFF-5)
#define TKBD_MOUSE_WHEEL_DOWN     (0xFFFF-6)

