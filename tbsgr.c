#define _POSIX_C_SOURCE 200112L

#include "tbsgr.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Fills an array of ints with SGR formatting codes for the sgr_t structure.
// This is mostly used internally to drive encoding to SGR string sequence.
int sgr_unpack(uint16_t codes[], sgr_t sgr) {
        int pos = 0;
        int at = sgr.at;

        // flip on/off attributes off instead of on
        int neg = (at&SGR_NEGATE) ? 20 : 0;

        // reset all attributes before applying subsequent ones
        if (at&SGR_RESET) codes[pos++] = 0;

        // bold on/off is a special case because code 21 is double underline
        if (at&SGR_BOLD)
                codes[pos++] = neg ? 22 : 1;

        // on/off typographic and background attributes
        if (at&SGR_FAINT)     codes[pos++] = neg + 2;
        if (at&SGR_ITALIC)    codes[pos++] = neg + 3;
        if (at&SGR_UNDERLINE) codes[pos++] = neg + 4;
        if (at&SGR_BLINK)     codes[pos++] = neg + 5;
        if (at&SGR_REVERSE)   codes[pos++] = neg + 7;
        if (at&SGR_STRIKE)    codes[pos++] = neg + 9;

        // make it possible to loop over fg/bg colors with mode
        struct { int mode; int16_t color; } fgbg[2] = {
                { at&SGR_FG_MASK, sgr.fg },
                { at&SGR_BG_MASK, sgr.bg },
        };

        for (int i = 0; i < 2; i++) {
                int mode = fgbg[i].mode;
                if (!mode) continue;

                int16_t color = fgbg[i].color;

                int is_bg  = mode & SGR_BG_MASK;
                int is_8   = (mode == SGR_FG    || mode == SGR_BG);
                int is_16  = (mode == SGR_FG16  || mode == SGR_BG16);
                int is_24  = (mode == SGR_FG24  || mode == SGR_BG24);
                int is_216 = (mode == SGR_FG216 || mode == SGR_BG216);
                int is_256 = (mode == SGR_FG256 || mode == SGR_BG256);

                if (is_8 || is_16 || neg) {
                        // SGR_NEGATE specified: use default color
                        if (neg) color = 9;

                        // truncate to 8 colors but allow 9 (default fg/bg)
                        if (color > 7 && color != SGR_DEFAULT)
                                color = 7;

                        color += 30;               // normal fg [3x range]
                        if (is_bg) color += 10;    // normal bg [4x range]

                        if (is_16) color += 60;    // bright fg [9x] or bg [10x]
                        codes[pos++] = color;

                } else if (is_24 || is_216 || is_256) {
                        if (is_bg) codes[pos++] = 48; // set bg
                        else       codes[pos++] = 38; // set fg
                        codes[pos++] = 5;

                        // the 24, 216, and 256 color modes all use the same
                        // 256-color palette; we just adjust the index here
                        // for convenience.
                        if (is_24) {
                                if (color > 23) color = 23;
                                color += 232;
                        } else if (is_216) {
                                if (color > 215) color = 215;
                                color += 16;
                        }

                        codes[pos++] = color;
                } else {
                        // TODO: invalid color mode
                }
        }

        return pos;
}


// Simple itoa that only handles positive numbers.
// Returns the number of chars written to buf not including the \0 terminator.
static inline int uitoa(uint16_t n, char *buf) {
        // write numerics into string backwards
        int i = 0;
        do {
                buf[i++] = n % 10 + '0';
        } while ((n /= 10) > 0);
        buf[i] = '\0';

        // now reverse it
        for (int j = 0; j < i / 2; j++) {
                int k = i - j - 1;
                char a = buf[j];
                buf[j] = buf[k];
                buf[k] = a;
        }

        return i;
}

// Emit string characters for an SGR value.
// This lets us keep the uint32_t -> string logic in one place but allow writing
// to different types of mediums (FILE, fd, char* buffer, etc.).
int sgr_encode(void *p, void (*func)(void *, char *, int), sgr_t sgr) {
        uint16_t codes[SGR_ELMS_MAX];
        int ncodes = sgr_unpack(codes, sgr);
        if (ncodes == 0) return 0;

        int sz = 0;

        // write SGR open: "\e["
        func(p, SGR_OPEN, sizeof(SGR_OPEN) - 1);
        sz += sizeof(SGR_OPEN) - 1;

        // write formatting codes separated by ";"
        int n;
        char buf[8];
        for (int i = 0; i < ncodes; i++) {
                if (i > 0) {
                        // write SGR separator: ";"
                        func(p, SGR_SEP, sizeof(SGR_SEP) - 1);
                        sz += sizeof(SGR_SEP) - 1;
                }
                n = uitoa(codes[i], buf);
                func(p, buf, n);
                sz += n;
        }

        // write SGR close: "m"
        func(p, SGR_CLOSE, sizeof(SGR_CLOSE) - 1);
        sz += sizeof(SGR_CLOSE) - 1;

        return sz;
}


struct strbuf {
        int pos;
        char *str;
};

static void strbuf_write(void *dest, char *src, int n) {
        struct strbuf *buf = (struct strbuf*)dest;
        memcpy(buf->str + buf->pos, src, n);
        buf->pos += n;
}

int sgr_str(char *dest, sgr_t sgr) {
        struct strbuf buf = { 0, dest };
        int sz = sgr_encode(&buf, strbuf_write, sgr);
        dest[sz] = '\0';
        return sz;

}


struct fdinfo {
        int fd;
        int err;
        int sz;
};

// Note: be careful not to accidentally clear errno since that's
// how the caller of sgr_write() will access error info.
static void fd_write(void *dest, char *src, int n) {
        struct fdinfo *f = dest;
        if (f->err) return;

        int sz = write(f->fd, src, n);
        if (sz == -1) {
                f->err = errno;
                return;
        }

        f->sz += sz;
}

int sgr_write(int fd, sgr_t sgr) {
        struct fdinfo f = { fd, 0, 0 };
        sgr_encode(&f, fd_write, sgr);
        if (f.err) {
                return -1;
        }
        return f.sz;
}



struct streaminfo {
        FILE *stream;
        int sz;
        int err;
        int eof;
};

static void stream_write(void *dest, char *src, int n) {
        struct streaminfo *f = dest;
        if (f->err || f->eof) return;

        size_t sz = fwrite(src, 1, n, f->stream);
        f->sz += sz;

        if (sz < (size_t)n) {
                f->eof = feof(f->stream);
                f->err = ferror(f->stream);
        }
}

int sgr_fwrite(FILE *stream, sgr_t sgr) {
        struct streaminfo f = { stream, 0, 0, 0 };
        sgr_encode(&f, stream_write, sgr);
        if (f.err || f.eof) {
                return -1;
        }
        return f.sz;
}
