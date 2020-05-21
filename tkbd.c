/*
 *
 * tkbd.c - Terminal keyboard, mouse, and character input library
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * See tkbd.h for interface documentation.
 *
 */

#include "tkbd.h"
#include "ti.h"

#include <stdlib.h>            // strtoul
#include <stddef.h>            // size_t
#include <stdio.h>             // snprintf
#include <string.h>            // memset
#include <unistd.h>            // read
#include <termios.h>           // tcgetattr, tcsetattr, struct termios
#include <ctype.h>             // isprint
#include <assert.h>

#define ARRAYLEN(a) (sizeof(a) / sizeof(a[0]))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// Checks if s1 starts with s2 and returns the strlen of s2 if so.
// Returns zero when s1 does not start with s2.
// len is the length of s1
// s2 should be null-terminated
static int starts_with(const char *s1, int len, const char *s2)
{
	int n = 0;
	while (*s2 && n < len) {
		if (*s1++ != *s2++)
			return 0;
		n++;
	}
	if (*s2 == 0)
		return n;
	else
		return 0;
}

// Parse multiple numeric parameters from a CSI sequence and store in the array
// pointed to by ar. A maximum of n parameters will be parsed and filled into
// the array. If a parameter is blank, 0 will be set in the array.
//
// Returns the number of parsed parsed and filled into the array.
static int parse_seq_params(int* ar, int n, char *pdata)
{
	int i = 0;
	while (i < n) {
		ar[i++] = strtol(pdata, &pdata, 10);
		if (*pdata++ == '\0')
			break;
	}

	return i;
}

// Parse a character from the buffer.
static int parse_char_seq(struct tkbd_event *ev, char const *buf, int len)
{
	char const *p  = buf;
	char const *pe = buf + len;

	if (p >= pe || *p < 0x20 || *p > 0x7E)
		return 0;

	ev->ch = *p;
	ev->seq[0] = *p;

	if (*p >= 'a' && *p <= 'z') {
		ev->key = TKBD_KEY_A + (*p - 'a');
		return 1;
	}

	if (*p >= '0' && *p <= '9') {
		ev->key = *p;
		return 1;
	}

	if (*p >= 'A' && *p <= 'Z') {
		ev->mod |= TKBD_MOD_SHIFT;
		ev->key = *p;
		return 1;
	}

	// punctuation character or space
	ev->key = *p;
	if (!strchr(" `-=[]\\;',./", *p))
		ev->mod |= TKBD_MOD_SHIFT;

	return 1;
}

// Parse a Ctrl+CH, BACKSPACE, TAB, ENTER, and ESC char sequences.
// These generate single-byte C0 sequences.
//
// Control sequences handled
// Ctrl+\ or Ctrl+4, Ctrl+] or Ctrl+5, Ctrl+^ or Ctrl+6, Ctrl+_ or Ctrl+7,
// Ctrl+@ or Ctrl+2, Ctrl+A...Ctrl+Z (0x01...0x1A).
static int parse_ctrl_seq(struct tkbd_event *ev, char const *buf, int len)
{
	char const *p  = buf;
	char const *pe = buf + len;

	if (p >= pe)
		return 0;

	if (*p == '\033') {
		// ESC key
		ev->key = TKBD_KEY_ESC;
	} else if (*p >= 0x1C && *p <= 0x1F) {
		// Ctrl+\ or Ctrl+4, Ctrl+] or Ctrl+5,
		// Ctrl+^ or Ctrl+6, Ctrl+_ or Ctrl+7
		ev->mod |= TKBD_MOD_CTRL;
		ev->key = TKBD_KEY_BACKSLASH + (*p - 0x1C);
	} else if (*p >= 0x08 && *p <= 0x0A) {
		// BACKSPACE (CTRL+H), TAB (CTRL+I), ENTER (CTRL+J)
		ev->key = *p;
	} else if (*p == 0x00) {
		// Ctrl+@ or Ctrl+2
		ev->mod |= TKBD_MOD_CTRL;
		ev->key = TKBD_KEY_2;
	} else if (*p == 0x7F) {
		// BACKSPACE2 or Ctrl+8
		ev->key = TKBD_KEY_BACKSPACE2;
	} else if (*p <= 0x1A) {
		// Ctrl+A (0x01) through Ctrl+Z (0x1A)
		ev->mod |= TKBD_MOD_CTRL;
		ev->key = TKBD_KEY_A + (*p - 0x01);
	} else {
		return 0;
	}

	ev->ch = *p;
	ev->seq[0] = *p;
	return 1;
}

// Parse an ALT key sequence and fill event struct.
// Any character or C0 control sequence may be preceded by ESC, indicating
// that ALT was pressed at the same time.
//
// ALT+CH:       \Eg   (parse_char_seq)
// SHIFT+ALT+CH: \EG   (parse_char_seq)
// CTRL+ALT+CH:  \E^G  (parse_ctrl_seq)
//
// Returns the number of bytes consumed to fill the event struct.
static int parse_alt_seq(struct tkbd_event *ev, char const *buf, int len)
{
	char const *p  = buf;
	char const *pe = buf + len;

	if (p >= pe || *p++ != '\033')
		return 0;

	int n = parse_char_seq(ev, p, pe - p);
	if (n == 0)
		n = parse_ctrl_seq(ev, p, pe - p);

	if (n == 0)
		return 0;

	ev->mod |= TKBD_MOD_ALT;
	p += n;
	memcpy(ev->seq, buf, p - buf);
	return p - buf;
}

/*
 * vt sequences
 *
 * Table of EMCA-48 / VT special key input sequences.
 * Array elements correspond to first parameter in escape sequence.
 * Second parameter specifies mod key flags.
 *
 * \E[0~  -          \E[10~  F0        \E[20~  F9        \E[30~   -
 * \E[1~  HOME       \E[11~  F1        \E[21~  F10       \E[31~   F17
 * \E[2~  INS        \E[12~  F2        \E[22~  -         \E[32~   F18
 * \E[3~  DEL        \E[13~  F3        \E[23~  F11       \E[33~   F19
 * \E[4~  END        \E[14~  F4        \E[24~  F12       \E[34~   F20
 * \E[5~  PGUP       \E[15~  F5        \E[25~  F13       \E[35~
 * \E[6~  PGDN       \E[16~  -         \E[26~  F14
 * \E[7~  HOME       \E[17~  F6        \E[27~  -
 * \E[8~  END        \E[18~  F7        \E[28~  F15
 * \E[9~  -          \E[19~  F8        \E[29~  F16
 *
 */
static uint16_t const vt_key_table[] = {
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_HOME,
	TKBD_KEY_INS,
	TKBD_KEY_DEL,
	TKBD_KEY_END,
	TKBD_KEY_PGUP,
	TKBD_KEY_PGDN,
	TKBD_KEY_HOME,
	TKBD_KEY_END,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F1,
	TKBD_KEY_F2,
	TKBD_KEY_F3,
	TKBD_KEY_F4,
	TKBD_KEY_F5,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F6,
	TKBD_KEY_F7,
	TKBD_KEY_F8,
	TKBD_KEY_F9,
	TKBD_KEY_F10,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F11,
	TKBD_KEY_F12,
	TKBD_KEY_F13,
	TKBD_KEY_F14,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F15,
	TKBD_KEY_F16,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F17,
	TKBD_KEY_F18,
	TKBD_KEY_F19,
	TKBD_KEY_F20,
};

/*
 * Table of xterm key input sequences.
 *
 * First parameter, when given, specifies the key modifier except when 1 and
 * the second parameter is set, in which case the second parameter specifies
 * key modifier flags.
 *
 * Array elements correspond to CHR - A in escape sequence table.
 *
 * \E[A   UP         \E[K    -         \E[U   -
 * \E[B   DOWN       \E[L    -         \E[V   -
 * \E[C   RIGHT      \E[M    -         \E[W   -
 * \E[D   LEFT       \E[N    -         \E[X   -
 * \E[E   KP 5       \E[O    -         \E[Y   -
 * \E[F   END        \E[P    F1        \E[Z   Shift+Tab
 * \E[G   KP 5       \E[Q    F2
 * \E[H   HOME       \E[R    F3
 * \E[I   -          \E[S    F4
 * \E[J   -          \E[T    -
 *
 * In some cases, SS3 is used to introduce the sequence instead of CSI. Known
 * keys of this form:
 *
 * \EOA   -          \EOK    -         \EOU   -
 * \EOB   -          \EOL    -         \EOV   -
 * \EOC   -          \EOM    -         \EOW   -
 * \EOD   -          \EON    -         \EOX   -
 * \EOE   -          \EOO    -         \EOY   -
 * \EOF   -          \EOP    F1        \EOZ   -
 * \EOG   -          \EOQ    F2
 * \EOH   -          \EOR    F3
 * \EOI   -          \EOS    F4
 * \EOJ   -          \EOT    -
 *
 */
static uint16_t const xt_key_table[] = {
	TKBD_KEY_UP,
	TKBD_KEY_DOWN,
	TKBD_KEY_RIGHT,
	TKBD_KEY_LEFT,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_END,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_HOME,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_UNKNOWN,
	TKBD_KEY_F1,
	TKBD_KEY_F2,
	TKBD_KEY_F3,
	TKBD_KEY_F4,
};

// Parse a special keyboard sequence and fill the zeroed event structure.
// No more than len bytes will be read from buf.
//
// Special keyboard sequences are typically only generated for function keys
// F1-F12, INS, DEL, HOME, END, PGUP, PGDOWN, and the cursor arrow keys.
//
// IMPORTANT: This function assumes the event struct is zeroed. Not doing so
// will lead to unpredictable behavior like ev->seq not being null terminated.
//
// Returns the number of bytes read from buf to fill the event structure.
// Returns zero when no escape sequence is present at front of buf or when the
// sequence is not recognized.
static int parse_special_seq(struct tkbd_event *ev, char const *buf, int len)
{
	char const *p  = buf;
	char const *pe = p + len;

	// bail if not an escape sequence
	if (p >= pe || *p++ != '\033')
		return 0;

	// bail if we're out of chars
	if (p >= pe)
		return 0;

	// figure out CSI vs. SS3 sequence type; bail if neither
	char const seq = *p++;
	if (seq != '[' && seq != 'O')
		return 0;

	// consume all numeric sequence parameters so we can get to the final
	// byte code. we'll use later.
	int i = 0;
	char parmdata[32] = {0};
	while (p < pe && *p >= '0' && *p <= ';') {
		// continue seeking if parms overflow buffer
		if(i < (int)sizeof(parmdata)-1)
			parmdata[i++] = *p++;
		else
			p++;
	}

	// looked like CSI/SS3 sequence but no final byte code available; bail
	if (p >= pe)
		return 0;

	// determine if vt or xterm style key sequence and map key and mods
	int parms[2] = {0};
	if (seq == '[' && *p == '~') {
		// vt style sequence: (Ex: \E[5;3~ = ALT+PGUP)
		parse_seq_params(parms, ARRAYLEN(parms), parmdata);

		if (parms[0] < (int)ARRAYLEN(vt_key_table))
			ev->key = vt_key_table[parms[0]];
		else
			ev->key = TKBD_KEY_UNKNOWN;

		if (parms[1])
			ev->mod = parms[1] - 1;

		p++;
	} else if (*p >= 'A' && *p <= 'Z') {
		// xterm style sequence (Ex: \E[3A = ALT+UP, \EOP = F1)
		parse_seq_params(parms, ARRAYLEN(parms), parmdata);

		int index = *p - 'A';
		if (index < (int)ARRAYLEN(xt_key_table))
			ev->key = xt_key_table[index];
		else
			ev->key = TKBD_KEY_UNKNOWN;

		// handle both forms: "\E[3A" and "\E[1;3A" both = ALT+UP
		if (parms[0] == 1 && parms[1])
			ev->mod = parms[1] - 1;
		else if (parms[0])
			ev->mod = parms[0] - 1;

		p++;
	} else {
		// we dont know how to handle this sequence type
		return 0;
	}

	// copy seq source data into event seq buffer
	size_t sz = MIN(p-buf, TKBD_SEQ_MAX-1);
	memcpy(ev->seq, buf, sz);

	return p - buf;

}

// Decode various mouse event char sequences into the given event struct.
// Returns the number of bytes read from buf when the sequence is recognized and
// decoded; or, a negative integer count of bytes when the sequence is recognized
// as a mouse sequence but invalid.
static int parse_mouse_seq(struct tkbd_event *ev, char const *buf, int len)
{
	// TODO: split two main cases into separate functions
	if (len >= 6 && starts_with(buf, len, "\033[M")) {
		// X10 mouse encoding, the simplest one
		// \033 [ M Cb Cx Cy
		int b = buf[3] - 32;
		switch (b & 3) {
		case 0:
			if ((b & 64) != 0)
				ev->key = TKBD_MOUSE_WHEEL_UP;
			else
				ev->key = TKBD_MOUSE_LEFT;
			break;
		case 1:
			if ((b & 64) != 0)
				ev->key = TKBD_MOUSE_WHEEL_DOWN;
			else
				ev->key = TKBD_MOUSE_MIDDLE;
			break;
		case 2:
			ev->key = TKBD_MOUSE_RIGHT;
			break;
		case 3:
			ev->key = TKBD_MOUSE_RELEASE;
			break;
		default:
			return -6;
		}
		ev->type = TKBD_MOUSE; // TBKB_KEY by default
		if ((b & 32) != 0)
			ev->mod |= TKBD_MOD_MOTION;

		// the coord is 1,1 for upper left
		ev->x = (uint8_t)buf[4] - 1 - 32;
		ev->y = (uint8_t)buf[5] - 1 - 32;

		return 6;
	} else if (starts_with(buf, len, "\033[<") ||
		   starts_with(buf, len, "\033[")) {
		// xterm 1006 extended mode or urxvt 1015 extended mode
		// xterm: \033 [ < Cb ; Cx ; Cy (M or m)
		// urxvt: \033 [ Cb ; Cx ; Cy M
		int i, mi = -1, starti = -1;
		int isM, isU, s1 = -1, s2 = -1;
		int n1 = 0, n2 = 0, n3 = 0;

		for (i = 0; i < len; i++) {
			// We search the first (s1) and the last (s2) ';'
			if (buf[i] == ';') {
				if (s1 == -1)
					s1 = i;
				s2 = i;
			}

			// We search for the first 'm' or 'M'
			if ((buf[i] == 'm' || buf[i] == 'M') && mi == -1) {
				mi = i;
				break;
			}
		}
		if (mi == -1)
			return 0;

		// whether it's a capital M or not
		isM = (buf[mi] == 'M');

		if (buf[2] == '<') {
			isU = 0;
			starti = 3;
		} else {
			isU = 1;
			starti = 2;
		}

		if (s1 == -1 || s2 == -1 || s1 == s2)
			return 0;

		n1 = strtoul(&buf[starti], NULL, 10);
		n2 = strtoul(&buf[s1 + 1], NULL, 10);
		n3 = strtoul(&buf[s2 + 1], NULL, 10);

		if (isU)
			n1 -= 32;

		switch (n1 & 3) {
		case 0:
			if ((n1&64) != 0)
				ev->key = TKBD_MOUSE_WHEEL_UP;
			else
				ev->key = TKBD_MOUSE_LEFT;
			break;
		case 1:
			if ((n1&64) != 0)
				ev->key = TKBD_MOUSE_WHEEL_DOWN;
			else
				ev->key = TKBD_MOUSE_MIDDLE;
			break;
		case 2:
			ev->key = TKBD_MOUSE_RIGHT;
			break;
		case 3:
			ev->key = TKBD_MOUSE_RELEASE;
			break;
		default:
			return mi + 1;
		}

		if (!isM) // on xterm mouse release is signaled by lowercase m
			ev->key = TKBD_MOUSE_RELEASE;

		ev->type = TKBD_MOUSE; // TB_EVENT_KEY by default
		if ((n1&32) != 0)
			ev->mod |= TKBD_MOD_MOTION;

		ev->x = (uint8_t)n2 - 1;
		ev->y = (uint8_t)n3 - 1;

		return mi + 1;
	}

	return 0;
}

// Parse mouse, special key, alt key, or ctrl key sequence and fill event.
// Order is important here since funcs like parse_alt_seq eat \033 chars.
int tkbd_parse(struct tkbd_event *ev, char const *buf, int len)
{
	int n;

	if ((n = parse_mouse_seq(ev, buf, len)))
		return n;
	if ((n = parse_special_seq(ev, buf, len)))
		return n;
	if ((n = parse_alt_seq(ev, buf, len)))
		return n;
	if ((n = parse_ctrl_seq(ev, buf, len)))
		return n;
	if ((n = parse_char_seq(ev, buf, len)))
		return n;

	return 0;
}

// Attach the keyboard input stream stuct to the given file descriptor, which
// is almost always STDIN_FILENO.
int tkbd_attach(struct tkbd_stream *s, int fd)
{
	int rc;

	// save current termios settings for detach()
	if ((rc = tcgetattr(fd, &s->tc)))
		return rc;

	// set raw mode input flags
	struct termios raw = s->tc;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;

	if ((rc = tcsetattr(fd, TCSAFLUSH, &raw)))
		return rc;

	s->fd = fd;
	memset(s->buf, 0, sizeof(s->buf));
	s->bufpos = 0;
	s->buflen = 0;

	return 0;
}

// Reset file descriptor termios attributes.
int tkbd_detach(struct tkbd_stream *s)
{
	int rc = tcsetattr(s->fd, TCSAFLUSH, &s->tc);
	return rc;
}

// Read a key, mouse, or character from the keyboard input stream.
int tkbd_read(struct tkbd_stream *s, struct tkbd_event *ev)
{
	// fill buffer with data from fd, possibly restructuring the buffer to
	// free already processed input.
	if (s->buflen < TKBD_SEQ_MAX) {
		int bufspc = sizeof(s->buf) - s->bufpos - s->buflen;

		if (bufspc < TKBD_SEQ_MAX) {
			memmove(s->buf, s->buf + s->bufpos, s->buflen);
			s->bufpos = 0;
			bufspc = sizeof(s->buf) - s->buflen;
		}

		if (bufspc > 0) {
			char *p = s->buf + s->bufpos + s->buflen;
			ssize_t sz = read(s->fd, p, bufspc);
			if (sz < 0)
				return sz;
			s->buflen += sz;
		}
	}

	char *buf = s->buf + s->bufpos;
	int len = s->buflen;
	assert(buf+len <= s->buf+sizeof(s->buf));

	int n = tkbd_parse(ev, buf, len);
	s->bufpos += n;
	s->buflen -= n;

	return n;
}


// Modifier keys map to MOD - 1
static char const * const modifier_key_names[] = {
	"Shift",
	"Alt",
	"Shift+Alt",
	"Ctrl",
	"Ctrl+Shift",
	"Ctrl+Alt",
	"Ctrl+Shift+Alt",
	"Meta",
	"Meta+Shift",
	"Meta+Alt",
	"Meta+Shift+Alt",
	"Meta+Ctrl",
	"Meta+Ctrl+Shift",
	"Meta+Ctrl+Alt",
	"Meta+Ctrl+Shift+Alt",
};

// Special key name indexes map to KEY - TKBD_KEY_UP
static char const * const special_key_names[] = {
	"Up",           // TKBD_KEY_UP      0x10
	"Down",         // TKBD_KEY_DOWN    0x11
	"Right",        // TKBD_KEY_RIGHT   0x12
	"Left",         // TKBD_KEY_LEFT    0x13
	"INS",          // TKBD_KEY_INS     0x14
	"DEL",          // TKBD_KEY_DEL     0x15
	"PgUp",         // TKBD_KEY_PGUP    0x16
	"PgDn",         // TKBD_KEY_PGDN    0x17
	"HOME",         // TKBD_KEY_HOME    0x18
	"END",          // TKBD_KEY_END     0x19
};

// Function key names map to KEY - TKBD_KEY_F1
static char const * const function_key_names[] = {
	"F1",          // TKBD_KEY_F1                0x61
	"F2",          // TKBD_KEY_F2                0x62
	"F3",          // TKBD_KEY_F3                0x63
	"F4",          // TKBD_KEY_F4                0x64
	"F5",          // TKBD_KEY_F5                0x65
	NULL,
	"F6",          // TKBD_KEY_F6                0x67
	"F7",          // TKBD_KEY_F7                0x68
	"F8",          // TKBD_KEY_F8                0x69
	"F9",          // TKBD_KEY_F9                0x6A
	"F10",         // TKBD_KEY_F10               0x6B
	"F11",         // TKBD_KEY_F11               0x6C
	"F12",         // TKBD_KEY_F12               0x6D
	"F13",         // TKBD_KEY_F13               0x6E
	"F14",         // TKBD_KEY_F14               0x6F
	NULL,
	"F15",         // TKBD_KEY_F15               0x71
	"F16",         // TKBD_KEY_F16               0x72
	NULL,
	"F17",         // TKBD_KEY_F17               0x74
	"F18",         // TKBD_KEY_F18               0x75
	"F19",         // TKBD_KEY_F19               0x76
	"F20",         // TKBD_KEY_F20               0x77
};

int tkbd_desc(char *dest, size_t sz, struct tkbd_event const *ev)
{
	if (ev->type != TKBD_KEY)
		return 0;

	// figure out modifier string part
	char const *modstr = "";
	uint8_t mod = ev->mod & (TKBD_MOD_SHIFT|TKBD_MOD_ALT|
	                         TKBD_MOD_CTRL|TKBD_MOD_META);
	if (mod)
		modstr = modifier_key_names[mod-1];

	// figure out key name string
	char const *keystr = "";
	char ch[2] = {0}; // for single char keys

	if (ev->key >= TKBD_KEY_UP && ev->key <= TKBD_KEY_END) {
		keystr = special_key_names[ev->key - TKBD_KEY_UP];
	} else if (ev->key >= TKBD_KEY_F1 && ev->key <= TKBD_KEY_F20) {
		keystr = function_key_names[ev->key - TKBD_KEY_F1];
	} else {
		switch (ev->key) {
		case TKBD_KEY_ESC:
			keystr = "ESC";
			break;
		case TKBD_KEY_TAB:
			keystr = "Tab";
			break;
		case TKBD_KEY_ENTER:
			keystr = "Enter";
			break;
		case TKBD_KEY_SPACE:
			keystr = "Space";
			break;
		case TKBD_KEY_BACKSPACE:
		case TKBD_KEY_BACKSPACE2:
			keystr = "Backspace";
			break;
		case TKBD_KEY_UNKNOWN:
			keystr = "Unknown";
			break;
		default:
			assert(isprint(ev->key));
			ch[0] = (char)ev->key;
			keystr = ch;
			break;
		}
	}

	if (keystr == NULL)
		keystr = "Unknown";

	if (modstr[0] && keystr[0])
		return snprintf(dest, sz, "%s+%s", modstr, keystr);

	if (keystr[0])
		return snprintf(dest, sz, "%s", keystr);

	if (modstr[0])
		return snprintf(dest, sz, "%s", modstr);

	return 0;
}
