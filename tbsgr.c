#define _POSIX_C_SOURCE 200112L

#include "tbsgr.h"

#include <string.h>
#include <math.h>

int tb_sgr_ints(uint16_t codes[], uint32_t attrs) {
        int pos = 0;

        // flip on/off attributes off instead of on
        uint32_t neg = (attrs&TB_NEGATE) ? 20 : 0;

        // reset all attributes before applying subsequent ones
        if (attrs&TB_RESET) codes[pos++] = 0;

        // bold on/off is a special case because code 21 is double underline
        if (attrs&TB_BOLD)
                codes[pos++] = neg ? 22 : 1;

        // on/off typographic and background attributes
        if (attrs&TB_FAINT)     codes[pos++] = neg + 2;
        if (attrs&TB_ITALIC)    codes[pos++] = neg + 3;
        if (attrs&TB_UNDERLINE) codes[pos++] = neg + 4;
        if (attrs&TB_BLINK)     codes[pos++] = neg + 5;
        if (attrs&TB_REVERSE)   codes[pos++] = neg + 7;
        if (attrs&TB_STRIKE)    codes[pos++] = neg + 9;

        // bail out now if we're not applying color attributes
        if ((attrs&(TB_COLOR|TB_CM_MASK)) == 0)
                return pos;

        uint32_t colormode = (attrs&TB_CM_MASK);
        uint16_t color = (attrs&TB_COLOR_MASK);

        if (colormode == TB_8 || colormode == TB_BRIGHT) {
                // TODO: if (color > 7) color = ??
                color += 30;                              // foreground colors
                if (attrs&TB_BG)            color += 10;  // background colors
                if (colormode == TB_BRIGHT) color += 60;  // bright colors

                codes[pos++] = color;
        } else if (colormode >= TB_216 && colormode < TB_HIGH) {
                if (attrs&TB_BG)           codes[pos++] = 48; // set bg
                else                       codes[pos++] = 38; // set fg
                codes[pos++] = 5;

                if (colormode == TB_216) {
                        if (color > 215) color = 215;
                        color += 0x10;
                } else if (colormode == TB_GREYSCALE) {
                        if (color > 23) color = 23;
                        color += 0xe8;
                }
                codes[pos++] = color;
        }

        return pos;
}

// Simple itoa that only handles positive numbers.
// Returns the number of chars written to buf not including the \0 terminator.
static inline int uitoa(uint16_t n, char *buf) {
        int sz = floor(log10(n)) + 1; // length of string
        int i = sz;                   // start at end
        do {
                buf[--i] = n % 10 + '0';
        } while ((n /= 10) > 0);
        buf[sz] = '\0';
        return sz;
}

// Emit string characters for an SGR value.
// This lets us keep the uint32_t -> string logic in one place but allow writing
// to different types of mediums (FILE, fd, char* buffer, etc.).
int tb_sgr_encode(void *p, void (*func)(void *, char *, int), uint32_t attrs) {
        uint16_t codes[TB_SGR_ELMS_MAX];
        int ncodes = tb_sgr_ints(codes, attrs);
        if (ncodes == 0) return 0;

        int sz = 0;

        // write SGR open: "\e["
        func(p, TB_SGR_OPEN, sizeof(TB_SGR_OPEN) - 1);
        sz += sizeof(TB_SGR_OPEN) - 1;

        // write formatting codes separated by ";"
        int n;
        char buf[8];
        for (int i = 0; i < ncodes; i++) {
                if (i > 0) {
                        // write SGR separator: ";"
                        func(p, TB_SGR_SEP, sizeof(TB_SGR_SEP) - 1);
                        sz += sizeof(TB_SGR_SEP) - 1;
                }
                n = uitoa(codes[i], buf);
                func(p, buf, n);
                sz += n;
        }

        // write SGR close: "m"
        func(p, TB_SGR_CLOSE, sizeof(TB_SGR_CLOSE) - 1);
        sz += sizeof(TB_SGR_CLOSE) - 1;

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

int tb_sgr_strcpy(char *dest, uint32_t attrs) {
        struct strbuf buf = { 0, dest };
        int sz = tb_sgr_encode(&buf, strbuf_write, attrs);
        dest[sz] = '\0';
        return sz;

}
