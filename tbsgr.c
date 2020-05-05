#include "tbsgr.h"

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
