/*
 *
 * tkbd.h - Terminal keyboard, mouse, and character input library
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * Read input from a terminal with support for decoding special key sequences,
 * mouse events, and utf8 character data.
 */

#include <stdint.h>

/*
 * Key constants
 *
 * Safe subset of terminfo keys, which exist on all popular terminals.
 */
#define TKBD_F1               (0xFFFF-0)
#define TKBD_F2               (0xFFFF-1)
#define TKBD_F3               (0xFFFF-2)
#define TKBD_F4               (0xFFFF-3)
#define TKBD_F5               (0xFFFF-4)
#define TKBD_F6               (0xFFFF-5)
#define TKBD_F7               (0xFFFF-6)
#define TKBD_F8               (0xFFFF-7)
#define TKBD_F9               (0xFFFF-8)
#define TKBD_F10              (0xFFFF-9)
#define TKBD_F11              (0xFFFF-10)
#define TKBD_F12              (0xFFFF-11)
#define TKBD_INSERT           (0xFFFF-12)
#define TKBD_DELETE           (0xFFFF-13)
#define TKBD_HOME             (0xFFFF-14)
#define TKBD_END              (0xFFFF-15)
#define TKBD_PGUP             (0xFFFF-16)
#define TKBD_PGDN             (0xFFFF-17)
#define TKBD_ARROW_UP         (0xFFFF-18)
#define TKBD_ARROW_DOWN       (0xFFFF-19)
#define TKBD_ARROW_LEFT       (0xFFFF-20)
#define TKBD_ARROW_RIGHT      (0xFFFF-21)
#define TKBD_MOUSE_LEFT       (0xFFFF-22)
#define TKBD_MOUSE_RIGHT      (0xFFFF-23)
#define TKBD_MOUSE_MIDDLE     (0xFFFF-24)
#define TKBD_MOUSE_RELEASE    (0xFFFF-25)
#define TKBD_MOUSE_WHEEL_UP   (0xFFFF-26)
#define TKBD_MOUSE_WHEEL_DOWN (0xFFFF-27)

/*
 * All ASCII code points below SPACE character and a BACKSPACE key
 */
#define TKBD_CTRL_TILDE       0x00
#define TKBD_CTRL_2           0x00 /* clash with 'CTRL_TILDE' */
#define TKBD_CTRL_A           0x01
#define TKBD_CTRL_B           0x02
#define TKBD_CTRL_C           0x03
#define TKBD_CTRL_D           0x04
#define TKBD_CTRL_E           0x05
#define TKBD_CTRL_F           0x06
#define TKBD_CTRL_G           0x07
#define TKBD_BACKSPACE        0x08
#define TKBD_CTRL_H           0x08 /* clash with 'CTRL_BACKSPACE' */
#define TKBD_TAB              0x09
#define TKBD_CTRL_I           0x09 /* clash with 'TAB' */
#define TKBD_CTRL_J           0x0A
#define TKBD_CTRL_K           0x0B
#define TKBD_CTRL_L           0x0C
#define TKBD_ENTER            0x0D
#define TKBD_CTRL_M           0x0D /* clash with 'ENTER' */
#define TKBD_CTRL_N           0x0E
#define TKBD_CTRL_O           0x0F
#define TKBD_CTRL_P           0x10
#define TKBD_CTRL_Q           0x11
#define TKBD_CTRL_R           0x12
#define TKBD_CTRL_S           0x13
#define TKBD_CTRL_T           0x14
#define TKBD_CTRL_U           0x15
#define TKBD_CTRL_V           0x16
#define TKBD_CTRL_W           0x17
#define TKBD_CTRL_X           0x18
#define TKBD_CTRL_Y           0x19
#define TKBD_CTRL_Z           0x1A
#define TKBD_ESC              0x1B
#define TKBD_CTRL_LSQ_BRACKET 0x1B /* clash with 'ESC' */
#define TKBD_CTRL_3           0x1B /* clash with 'ESC' */
#define TKBD_CTRL_4           0x1C
#define TKBD_CTRL_BACKSLASH   0x1C /* clash with 'CTRL_4' */
#define TKBD_CTRL_5           0x1D
#define TKBD_CTRL_RSQ_BRACKET 0x1D /* clash with 'CTRL_5' */
#define TKBD_CTRL_6           0x1E
#define TKBD_CTRL_7           0x1F
#define TKBD_CTRL_SLASH       0x1F /* clash with 'CTRL_7' */
#define TKBD_CTRL_UNDERSCORE  0x1F /* clash with 'CTRL_7' */
#define TKBD_SPACE            0x20
#define TKBD_DELETE           0x7F
#define TKBD_CTRL_8           0x7F /* clash with 'BACKSPACE2' */

#define TKBD_INPUT_CURRENT 0x00
#define TKBD_INPUT_ESC     0x01
#define TKBD_INPUT_ALT     0x02
#define TKBD_INPUT_MOUSE   0x04

/*
 * Event types
 *
 */
#define TKBD_KEY          1    // A key was pressed
#define TKBD_RESIZE       2    // The terminal size was changed
#define TKBD_MOUSE        3    // Move, scroll, or button event

/*
 * Alt modifier flags
 *
 */
#define TKBD_ALT    0x01
#define TKBD_MOTION 0x02


/*
 * Sizes
 *
 */
#define TKBD_MAP_SZ 22

struct tkbd_stream {
	int  fd;                       // file descriptor to read from
	int  mode;                     // the input mode
	int  timeout;                  // non-blocking
	char keymap[KEY_MAP_SZ][16];   // sequence strings for keys F1 - RARROW
	char buf[4096];                // input buffer
	int  buflen;                   // used part of buf
};

struct tkbd_event {
	uint8_t  type;       // event type
	uint8_t  mod;        // modifiers to either 'key' or 'ch' below
	uint16_t key;        // one of the TB_KEY_* constants
	uint32_t ch;         // unicode character
	int32_t  w, h;
	int32_t  x, y;
};

/*
 * Initialize the input stream structures.
 */
int tkbd_init(struct tkbd_stream *s, int fd);

/*
 * Read a single keyboard/mouse/resize sequence or UTF8 encoded character
 * from the file descriptor and fill the event structure pointed to by ev
 * with information.
 *
 * Returns the number of bytes read from buf when the event structure is filled.
 * Returns zero when not enough data is available to decode an event.
 */
int tkbd_read(struct tkbd_stream *s, struct tkbd_event *ev);

/*
 * Parse a single keyboard/mouse/resize sequence or UTF8 encoded character from
 * the buffer pointed to by buf and fill the event structure pointed to by ev
 * with information.
 *
 * Returns the number of bytes read from buf when the event structure is filled.
 * Returns zero when not enough data is available to decode an event.
 */
int tkbd_parse(struct tkbd_event *ev, const char *buf, int len, int mode);
