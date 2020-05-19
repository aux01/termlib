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
#include <string.h>            // memset
#include <assert.h>

#define ARRAYLEN(a) (sizeof(a) / sizeof(a[0]))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void tkbd_init(struct tkbd_stream *s, int fd)
{
	s->fd = fd;
	s->timeout = 0;
	memset(s->buf, 0, sizeof(s->buf));
	s->buflen = 0;
}

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
static int parse_seq_params(int* ar, int n, char *pdata) {
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

// Parse a Ctrl+CH , BACKSPACE, TAB, or ENTER sequence.
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

	if (*p >= 0x1C && *p <= 0x1F) {
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
 * VT sequences
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
 * xterm sequences:
 *
 * Table of xterm key input sequences.
 * Array elements correspond to CHR - A in escape sequence.
 * First parameter specifies mod key flags.
 *
 * \E[A   UP         \E[K    -         \E[U   -
 * \E[B   DOWN       \E[L    -         \E[V   -
 * \E[C   RIGHT      \E[M    -         \E[W   -
 * \E[D   LEFT       \E[N    -         \E[X   -
 * \E[E   -          \E[O    -         \E[Y   -
 * \E[F   END        \E[1P   F1        \E[Z   -
 * \E[G   KP 5       \E[1Q   F2
 * \E[H   HOME       \E[1R   F3
 * \E[I   -          \E[1S   F4
 * \E[J   -          \E[T    -
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
// IMPORTANT: This function assumes the event struct is zeroed. Not doing so
// will lead to unpredictable behavior like ev->seq not being null terminated.
//
// Returns the number of bytes read from buf to fill the event structure.
// Returns zero when no escape sequence is present at front of buf or when the
// sequence is not recognized.
static int parse_keyboard_seq(struct tkbd_event *ev, const char *buf, int len)
{
	char const *p  = buf;
	char const *pe = p + len;

	// bail if not an escape sequence
	if (p >= pe || *p++ != '\033')
		return 0;

	// bail if not a CSI sequence
	if (p >= pe || *p++ != '[')
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

	// looked like CSI sequence but no final byte code available; bail
	if (p >= pe)
		return 0;

	// determine if vt or xterm style key sequence and map key and mods
	int parms[2] = {0};
	if (*p == '~') {
		// vt style sequence: \E[5;3~ = ALT+PGUP
		parse_seq_params(parms, ARRAYLEN(parms), parmdata);

		if (parms[0] < (int)ARRAYLEN(vt_key_table))
			ev->key = vt_key_table[parms[0]];
		else
			ev->key = TKBD_KEY_UNKNOWN;

		if (parms[1])
			ev->mod = parms[1] - 1;

		p++;
	} else if (*p >= 'A' && *p <= 'Z') {
		// xterm style sequence: \E[3A = ALT+UP
		parse_seq_params(parms, ARRAYLEN(parms), parmdata);

		int index = *p - 'A';
		if (index < (int)ARRAYLEN(xt_key_table))
			ev->key = xt_key_table[index];
		else
			ev->key = TKBD_KEY_UNKNOWN;

		if (parms[0])
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
static int parse_mouse_seq(struct tkbd_event *ev, const char *buf, int len)
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
static int parse_key_seq(struct tkbd_stream *s, struct tkbd_event *ev)
{
	int len;

	if ((len = parse_mouse_seq(ev, s->buf, s->buflen)))
		return len;

	if ((len = parse_keyboard_seq(ev, s->buf, s->buflen)))
		return len;

	if ((len = parse_alt_seq(ev, s->buf, s->buflen)))
		return len;

	if ((len = parse_ctrl_seq(ev, s->buf, s->buflen)))
		return len;

	if ((len = parse_char_seq(ev, s->buf, s->buflen)))
		return len;

	return 0;
}


#if 0
static int extract_event(struct tkbd_stream *s, struct tb_event *ev)
{
	const char *buf = s->buf;
	const int len = s->len;
	if (len == 0)
		return 0;

	if (buf[0] == '\033') {
		int n = parse_escape_seq(s, ev);
		if (n != 0) {
			int rc = 1;
			if (n < 0) {
				rc = 0;
				n = -n;
			}
			// TODO return unknown event
			bytebuffer_truncate(inbuf, n);
			return rc;
		} else {
			// it's not escape sequence, then it's ALT or ESC,
			// check inputmode
			if (s->mode&TB_INPUT_ESC) {
				// if we're in escape mode, fill ESC event, pop
				// buffer, return success
				ev->ch = 0;
				ev->key = TB_KEY_ESC;
				ev->mod = 0;
				bytebuffer_truncate(inbuf, 1); // TODO
				return 1;
			} else if (s->mode&TB_INPUT_ALT) {
				// if we're in alt mode, set ALT modifier to
				// event and redo parsing
				ev->mod = TB_MOD_ALT;
				bytebuffer_truncate(inbuf, 1); // TODO
				return extract_event(s, ev);
			}
			assert(!"unreachable");
		}
	}

	// if we're here, this is not an escape sequence and not an alt sequence
	// so, it's a FUNCTIONAL KEY or a UNICODE character

	// first of all check if it's a functional key
	if ((unsigned char)buf[0] <= TB_KEY_SPACE ||
	    (unsigned char)buf[0] == TB_KEY_BACKSPACE2)
	{
		// fill event, pop buffer, return success */
		ev->ch = 0;
		ev->key = (uint16_t)buf[0];
		bytebuffer_truncate(inbuf, 1);
		return 1;
	}

	// feh... we got utf8 here

	// check if there is all bytes
	if (len >= tb_utf8_char_length(buf[0])) {
		/* everything ok, fill event, pop buffer, return success */
		tb_utf8_char_to_unicode(&ev->ch, buf);
		ev->key = 0;
		bytebuffer_truncate(inbuf, tb_utf8_char_length(buf[0]));
		return 1;
	}

	// event isn't recognized, perhaps there is not enough bytes in utf8
	// sequence
	return 0;
}


static int read_up_to(int n) {
	assert(n > 0);
	const int prevlen = input_buffer.len;
	bytebuffer_resize(&input_buffer, prevlen + n);

	int read_n = 0;
	while (read_n <= n) {
		ssize_t r = 0;
		if (read_n < n) {
			r = read(inout, input_buffer.buf + prevlen + read_n, n - read_n);
		}
#ifdef __CYGWIN__
		// While linux man for tty says when VMIN == 0 && VTIME == 0, read
		// should return 0 when there is nothing to read, cygwin's read returns
		// -1. Not sure why and if it's correct to ignore it, but let's pretend
		// it's zero.
		if (r < 0) r = 0;
#endif
		if (r < 0) {
			// EAGAIN / EWOULDBLOCK shouldn't occur here
			assert(errno != EAGAIN && errno != EWOULDBLOCK);
			return -1;
		} else if (r > 0) {
			read_n += r;
		} else {
			bytebuffer_resize(&input_buffer, prevlen + read_n);
			return read_n;
		}
	}
	assert(!"unreachable");
	return 0;
}

static int wait_fill_event(struct tb_event *event, struct timeval *timeout)
{
	// ;-)
#define ENOUGH_DATA_FOR_PARSING 64
	fd_set events;
	memset(event, 0, sizeof(struct tb_event));

	// try to extract event from input buffer, return on success
	event->type = TB_EVENT_KEY;
	if (extract_event(event, &input_buffer, inputmode))
		return event->type;

	// it looks like input buffer is incomplete, let's try the short path,
	// but first make sure there is enough space
	int n = read_up_to(ENOUGH_DATA_FOR_PARSING);
	if (n < 0)
		return -1;
	if (n > 0 && extract_event(event, &input_buffer, inputmode))
		return event->type;

	// n == 0, or not enough data, let's go to select
	while (1) {
		FD_ZERO(&events);
		FD_SET(inout, &events);
		FD_SET(winch_fds[0], &events);
		int maxfd = (winch_fds[0] > inout) ? winch_fds[0] : inout;
		int result = select(maxfd+1, &events, 0, 0, timeout);
		if (!result)
			return 0;

		if (FD_ISSET(inout, &events)) {
			event->type = TB_EVENT_KEY;
			n = read_up_to(ENOUGH_DATA_FOR_PARSING);
			if (n < 0)
				return -1;

			if (n == 0)
				continue;

			if (extract_event(event, &input_buffer, inputmode))
				return event->type;
		}
		if (FD_ISSET(winch_fds[0], &events)) {
			event->type = TB_EVENT_RESIZE;
			int zzz = 0;
			if (read(winch_fds[0], &zzz, sizeof(int)) < (ssize_t)sizeof(int)) {
				// ignore short read / error
				// could be due to signal.
			}
			buffer_size_change_request = 1;
			get_term_size(&event->w, &event->h);
			return TB_EVENT_RESIZE;
		}
	}
}
#endif