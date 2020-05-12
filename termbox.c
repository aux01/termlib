#define _POSIX_C_SOURCE 200112L    // sigaction
#define _XOPEN_SOURCE              // wcwidth

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

#include "termbox.h"

#include "bytebuffer.inl"
#include "term.inl"
#include "input.inl"

struct cellbuf {
	int width;
	int height;
	struct tb_cell *cells;
};

#define CELL(buf, x, y) (buf)->cells[(y) * (buf)->width + (x)]
#define IS_CURSOR_HIDDEN(cx, cy) (cx == -1 || cy == -1)
#define LAST_COORD_INIT -1

static struct termios orig_tios;

static struct cellbuf back_buffer;
static struct cellbuf front_buffer;
static struct bytebuffer output_buffer;
static struct bytebuffer input_buffer;

static int termw = -1;
static int termh = -1;

static int inputmode = TB_INPUT_ESC;
static int outputmode = TB_OUTPUT_NORMAL;

static int inout;
static int winch_fds[2];

static int lastx = LAST_COORD_INIT;
static int lasty = LAST_COORD_INIT;
static int cursor_x = -1;
static int cursor_y = -1;

static sgr_t default_sgr = { 0 };

static void write_cursor(int x, int y);

static void cellbuf_init(struct cellbuf *buf, int width, int height);
static void cellbuf_resize(struct cellbuf *buf, int width, int height);
static void cellbuf_clear(struct cellbuf *buf);
static void cellbuf_free(struct cellbuf *buf);

static void update_size(void);
static void update_term_size(void);
static void send_attr(sgr_t sgr);
static void send_char(int x, int y, uint32_t c);
static void send_clear(void);
static void sigwinch_handler(int xxx);
static int wait_fill_event(struct tb_event *event, struct timeval *timeout);

/* may happen in a different thread */
static volatile int buffer_size_change_request;

/* -------------------------------------------------------- */

int tb_init_fd(int inout_)
{
	inout = inout_;
	if (inout == -1) {
		return TB_EFAILED_TO_OPEN_TTY;
	}

	if (init_term() < 0) {
		close(inout);
		return TB_EUNSUPPORTED_TERMINAL;
	}

	if (pipe(winch_fds) < 0) {
		close(inout);
		return TB_EPIPE_TRAP_ERROR;
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigwinch_handler;
	sa.sa_flags = 0;
	sigaction(SIGWINCH, &sa, 0);

	tcgetattr(inout, &orig_tios);

	struct termios tios;
	memcpy(&tios, &orig_tios, sizeof(tios));

	tios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
	tios.c_oflag &= ~OPOST;
	tios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tios.c_cflag &= ~(CSIZE | PARENB);
	tios.c_cflag |= CS8;
	tios.c_cc[VMIN] = 0;
	tios.c_cc[VTIME] = 0;
	tcsetattr(inout, TCSAFLUSH, &tios);

	bytebuffer_init(&input_buffer, 128);
	bytebuffer_init(&output_buffer, 32 * 1024);

	bytebuffer_puts(&output_buffer, funcs[T_ENTER_CA]);
	bytebuffer_puts(&output_buffer, funcs[T_ENTER_KEYPAD]);
	bytebuffer_puts(&output_buffer, funcs[T_HIDE_CURSOR]);
	send_clear();

	update_term_size();
	cellbuf_init(&back_buffer, termw, termh);
	cellbuf_init(&front_buffer, termw, termh);
	cellbuf_clear(&back_buffer);
	cellbuf_clear(&front_buffer);

	return 0;
}

int tb_init_file(const char* name){
	return tb_init_fd(open(name, O_RDWR));
}

int tb_init(void)
{
	return tb_init_file("/dev/tty");
}

void tb_shutdown(void)
{
	if (termw == -1) {
		fputs("tb_shutdown() should not be called twice.", stderr);
		abort();
	}

	bytebuffer_puts(&output_buffer, funcs[T_SHOW_CURSOR]);
	bytebuffer_puts(&output_buffer, funcs[T_SGR0]);
	bytebuffer_puts(&output_buffer, funcs[T_CLEAR_SCREEN]);
	bytebuffer_puts(&output_buffer, funcs[T_EXIT_CA]);
	bytebuffer_puts(&output_buffer, funcs[T_EXIT_KEYPAD]);
	bytebuffer_puts(&output_buffer, funcs[T_EXIT_MOUSE]);
	bytebuffer_flush(&output_buffer, inout);
	tcsetattr(inout, TCSAFLUSH, &orig_tios);

	shutdown_term();
	close(inout);
	close(winch_fds[0]);
	close(winch_fds[1]);

	cellbuf_free(&back_buffer);
	cellbuf_free(&front_buffer);
	bytebuffer_free(&output_buffer);
	bytebuffer_free(&input_buffer);
	termw = termh = -1;
}

void tb_present(void)
{
	int x,y,w,i;
	struct tb_cell *back, *front;

	/* invalidate cursor position */
	lastx = LAST_COORD_INIT;
	lasty = LAST_COORD_INIT;

	if (buffer_size_change_request) {
		update_size();
		buffer_size_change_request = 0;
	}

	for (y = 0; y < front_buffer.height; ++y) {
		for (x = 0; x < front_buffer.width; ) {
			back = &CELL(&back_buffer, x, y);
			front = &CELL(&front_buffer, x, y);
			w = wcwidth(back->ch);
			if (w < 1) w = 1;
			if (memcmp(back, front, sizeof(struct tb_cell)) == 0) {
				x += w;
				continue;
			}
			memcpy(front, back, sizeof(struct tb_cell));
			send_attr(back->sgr);
			if (w > 1 && x >= front_buffer.width - (w - 1)) {
				// Not enough room for wide ch, so send spaces
				for (i = x; i < front_buffer.width; ++i) {
					send_char(i, y, ' ');
				}
			} else {
				send_char(x, y, back->ch);
				for (i = 1; i < w; ++i) {
					front = &CELL(&front_buffer, x + i, y);
					front->ch = 0;
					front->sgr = back->sgr;
				}
			}
			x += w;
		}
	}
	if (!IS_CURSOR_HIDDEN(cursor_x, cursor_y))
		write_cursor(cursor_x, cursor_y);
	bytebuffer_flush(&output_buffer, inout);
}

void tb_set_cursor(int cx, int cy)
{
	if (IS_CURSOR_HIDDEN(cursor_x, cursor_y) && !IS_CURSOR_HIDDEN(cx, cy))
		bytebuffer_puts(&output_buffer, funcs[T_SHOW_CURSOR]);

	if (!IS_CURSOR_HIDDEN(cursor_x, cursor_y) && IS_CURSOR_HIDDEN(cx, cy))
		bytebuffer_puts(&output_buffer, funcs[T_HIDE_CURSOR]);

	cursor_x = cx;
	cursor_y = cy;
	if (!IS_CURSOR_HIDDEN(cursor_x, cursor_y))
		write_cursor(cursor_x, cursor_y);
}

void tb_put_cell(int x, int y, const struct tb_cell *cell)
{
	if ((unsigned)x >= (unsigned)back_buffer.width)
		return;
	if ((unsigned)y >= (unsigned)back_buffer.height)
		return;
	CELL(&back_buffer, x, y) = *cell;
}

static void sgr_set_fg(sgr_t *sgr, uint16_t fg) {
	uint16_t fgcol = fg&0xFF;
	if (fgcol != TB_DEFAULT) {
		sgr->fg = fgcol;

		// XXX Logic kept verbatim from original send_attr()
		// implementation for compatibility. Some odd decisions
		// i dont totally understand here. -aux01
		if (outputmode == TB_OUTPUT_NORMAL) {
			sgr->fg -= 1;
			sgr->at |= SGR_FG;
		} else if (outputmode == TB_OUTPUT_256) {
			sgr->at |= SGR_FG256;
		} else if (outputmode == TB_OUTPUT_216) {
			if (fgcol > 215) sgr->fg = 7;
			sgr->at |= SGR_FG216;
		} else if (outputmode == TB_OUTPUT_GRAYSCALE) {
			if (fgcol > 23) sgr->fg = 23;
			sgr->at |= SGR_FG24;
		}
	}
}

static void sgr_set_bg(sgr_t *sgr, uint16_t bg) {
	uint16_t bgcol = bg&0xFF;
	if (bgcol != TB_DEFAULT) {
		sgr->bg = bgcol;

		// XXX Logic kept verbatim from original send_attr()
		// implementation for compatibility. Some odd decisions
		// i dont totally understand here. -aux01
		if (outputmode == TB_OUTPUT_NORMAL) {
			sgr->bg -= 1;
			sgr->at |= SGR_BG;
		} else if (outputmode == TB_OUTPUT_256) {
			sgr->at |= SGR_BG256;
		} else if (outputmode == TB_OUTPUT_216) {
			if (bgcol > 215) sgr->bg = 0;
			sgr->at |= SGR_BG216;
		} else if (outputmode == TB_OUTPUT_GRAYSCALE) {
			if (bgcol > 23) sgr->bg = 0;
			sgr->at |= SGR_BG24;
		}
	}
}

void tb_change_cell(int x, int y, uint32_t ch, uint16_t fg, uint16_t bg)
{
	sgr_t sgr = {0};

	if (fg&TB_BOLD)      sgr.at |= SGR_BOLD;
	if (fg&TB_FAINT)     sgr.at |= SGR_FAINT;
	if (fg&TB_ITALIC)    sgr.at |= SGR_ITALIC;
	if (fg&TB_UNDERLINE) sgr.at |= SGR_UNDERLINE;
	if (fg&TB_CROSSOUT)  sgr.at |= SGR_STRIKE;

	if (fg&TB_BLINK || bg&TB_BLINK || bg&TB_BOLD)
		sgr.at |= SGR_BLINK;
	if (fg&TB_REVERSE || bg&TB_REVERSE)
		sgr.at |= SGR_REVERSE;

	sgr_set_fg(&sgr, fg);
	sgr_set_bg(&sgr, bg);

	struct tb_cell c = {ch, sgr};
	tb_put_cell(x, y, &c);
}

void tb_blit(int x, int y, int w, int h, const struct tb_cell *cells)
{
	if (x + w < 0 || x >= back_buffer.width)
		return;
	if (y + h < 0 || y >= back_buffer.height)
		return;
	int xo = 0, yo = 0, ww = w, hh = h;
	if (x < 0) {
		xo = -x;
		ww -= xo;
		x = 0;
	}
	if (y < 0) {
		yo = -y;
		hh -= yo;
		y = 0;
	}
	if (ww > back_buffer.width - x)
		ww = back_buffer.width - x;
	if (hh > back_buffer.height - y)
		hh = back_buffer.height - y;

	int sy;
	struct tb_cell *dst = &CELL(&back_buffer, x, y);
	const struct tb_cell *src = cells + yo * w + xo;
	size_t size = sizeof(struct tb_cell) * ww;

	for (sy = 0; sy < hh; ++sy) {
		memcpy(dst, src, size);
		dst += back_buffer.width;
		src += w;
	}
}

struct tb_cell *tb_cell_buffer(void)
{
	return back_buffer.cells;
}

int tb_poll_event(struct tb_event *event)
{
	return wait_fill_event(event, 0);
}

int tb_peek_event(struct tb_event *event, int timeout)
{
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
	return wait_fill_event(event, &tv);
}

int tb_width(void)
{
	return termw;
}

int tb_height(void)
{
	return termh;
}

void tb_clear(void)
{
	if (buffer_size_change_request) {
		update_size();
		buffer_size_change_request = 0;
	}
	cellbuf_clear(&back_buffer);
}

int tb_select_input_mode(int mode)
{
	if (mode) {
		if ((mode & (TB_INPUT_ESC | TB_INPUT_ALT)) == 0)
			mode |= TB_INPUT_ESC;

		/* technically termbox can handle that, but let's be nice and show here
		   what mode is actually used */
		if ((mode & (TB_INPUT_ESC | TB_INPUT_ALT)) == (TB_INPUT_ESC | TB_INPUT_ALT))
			mode &= ~TB_INPUT_ALT;

		inputmode = mode;
		if (mode&TB_INPUT_MOUSE) {
			bytebuffer_puts(&output_buffer, funcs[T_ENTER_MOUSE]);
			bytebuffer_flush(&output_buffer, inout);
		} else {
			bytebuffer_puts(&output_buffer, funcs[T_EXIT_MOUSE]);
			bytebuffer_flush(&output_buffer, inout);
		}
	}
	return inputmode;
}

int tb_select_output_mode(int mode)
{
	if (mode)
		outputmode = mode;
	return outputmode;
}

void tb_set_clear_attributes(uint16_t fg, uint16_t bg) {
	default_sgr = (sgr_t){0};
	sgr_set_fg(&default_sgr, fg);
	sgr_set_bg(&default_sgr, bg);
}

/* -------------------------------------------------------- */

static int convertnum(uint32_t num, char* buf) {
	int i, l = 0;
	int ch;
	do {
		buf[l++] = '0' + (num % 10);
		num /= 10;
	} while (num);
	for(i = 0; i < l / 2; i++) {
		ch = buf[i];
		buf[i] = buf[l - 1 - i];
		buf[l - 1 - i] = ch;
	}
	return l;
}

#define WRITE_LITERAL(X) bytebuffer_append(&output_buffer, (X), sizeof(X)-1)
#define WRITE_INT(X) bytebuffer_append(&output_buffer, buf, convertnum((X), buf))

static void write_cursor(int x, int y) {
	char buf[32];
	WRITE_LITERAL("\033[");
	WRITE_INT(y+1);
	WRITE_LITERAL(";");
	WRITE_INT(x+1);
	WRITE_LITERAL("H");
}

static void cellbuf_init(struct cellbuf *buf, int width, int height)
{
	buf->cells = (struct tb_cell*)malloc(sizeof(struct tb_cell) * width * height);
	assert(buf->cells);
	buf->width = width;
	buf->height = height;
}

static void cellbuf_resize(struct cellbuf *buf, int width, int height)
{
	if (buf->width == width && buf->height == height)
		return;

	int oldw = buf->width;
	int oldh = buf->height;
	struct tb_cell *oldcells = buf->cells;

	cellbuf_init(buf, width, height);
	cellbuf_clear(buf);

	int minw = (width < oldw) ? width : oldw;
	int minh = (height < oldh) ? height : oldh;
	int i;

	for (i = 0; i < minh; ++i) {
		struct tb_cell *csrc = oldcells + (i * oldw);
		struct tb_cell *cdst = buf->cells + (i * width);
		memcpy(cdst, csrc, sizeof(struct tb_cell) * minw);
	}

	free(oldcells);
}

static void cellbuf_clear(struct cellbuf *buf)
{
	int i;
	int ncells = buf->width * buf->height;

	for (i = 0; i < ncells; ++i) {
		buf->cells[i].ch = ' ';
		buf->cells[i].sgr = default_sgr;
	}
}

static void cellbuf_free(struct cellbuf *buf)
{
	free(buf->cells);
}

static void get_term_size(int *w, int *h)
{
	struct winsize sz;
	memset(&sz, 0, sizeof(sz));

	ioctl(inout, TIOCGWINSZ, &sz);

	if (w) *w = sz.ws_col;
	if (h) *h = sz.ws_row;
}

static void update_term_size(void)
{
	struct winsize sz;
	memset(&sz, 0, sizeof(sz));

	ioctl(inout, TIOCGWINSZ, &sz);

	termw = sz.ws_col;
	termh = sz.ws_row;
}

static void send_attr(sgr_t sgr)
{
	static sgr_t last = {0,0,0};
	if (memcmp(&sgr, &last, sizeof(sgr_t)) == 0) {
		return;
	}

	bytebuffer_append(&output_buffer, funcs[T_SGR0], strlen(funcs[T_SGR0]));

	bytebuffer_reserve(&output_buffer, output_buffer.len + SGR_STR_MAX);
	int sz = sgr_str(output_buffer.buf + output_buffer.len, sgr);
	output_buffer.len += sz;

	last = sgr;
}

static void send_char(int x, int y, uint32_t c)
{
	char buf[7];
	int bw = tb_utf8_unicode_to_char(buf, c);
	if (x-1 != lastx || y != lasty)
		write_cursor(x, y);
	lastx = x; lasty = y;
	if(!c) buf[0] = ' '; // replace 0 with whitespace
	bytebuffer_append(&output_buffer, buf, bw);
}

static void send_clear(void)
{
	send_attr(default_sgr);
	bytebuffer_puts(&output_buffer, funcs[T_CLEAR_SCREEN]);
	if (!IS_CURSOR_HIDDEN(cursor_x, cursor_y))
		write_cursor(cursor_x, cursor_y);
	bytebuffer_flush(&output_buffer, inout);

	/* we need to invalidate cursor position too and these two vars are
	 * used only for simple cursor positioning optimization, cursor
	 * actually may be in the correct place, but we simply discard
	 * optimization once and it gives us simple solution for the case when
	 * cursor moved */
	lastx = LAST_COORD_INIT;
	lasty = LAST_COORD_INIT;
}

static void sigwinch_handler(int xxx)
{
	(void) xxx;
	const int zzz = 1;
	if (write(winch_fds[1], &zzz, sizeof(int)) < (ssize_t)sizeof(int)) {
		// short write or error. resize event may not fire.
		// TODO: log this
	}
}

static void update_size(void)
{
	update_term_size();
	cellbuf_resize(&back_buffer, termw, termh);
	cellbuf_resize(&front_buffer, termw, termh);
	cellbuf_clear(&front_buffer);
	send_clear();
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

// vim: noexpandtab
