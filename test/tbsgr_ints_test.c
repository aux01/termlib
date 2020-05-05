#include "../tbsgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	// buffer for holding sgr codes
	uint16_t codes[TB_SGR_ELMS_MAX];

	// SGR values are formed by combining attributes:
	int n = tb_sgr_ints(codes, TB_BOLD|TB_ITALIC|TB_RED);
	assert(n == 3);         // number of codes written
	assert(codes[0] == 1);  // bold text
	assert(codes[1] == 3);  // bold text
	assert(codes[2] == 31); // red text/foreground color

	// An SGR value's color part may target either the foreground (default)
	// or the background but not both. The TB_BG attribute causes the color
	// to apply to the background:
	n = tb_sgr_ints(codes, TB_BG|TB_GREEN);
	assert(n == 1);
	assert(codes[0] == 42); // green background

	// The default 4-bit color palette has 8 normal intensity colors.
	// You can access another 8 high intensity colors by applying the
	// TB_BRIGHT attribute:
	n = tb_sgr_ints(codes, TB_BRIGHT|TB_YELLOW);
	assert(n == 1);
	assert(codes[0] == 93); // bright yellow foreground

	// You can however apply foreground typographic attributes and
	// non-color background attributes in a single SGR value.
	n = tb_sgr_ints(codes, TB_ITALIC|TB_UNDERLINE|TB_BLINK|TB_CYAN|TB_BG);
	assert(n == 4);
	assert(codes[0] == 3);  // italic type
	assert(codes[1] == 4);  // underline type
	assert(codes[2] == 5);  // blinking background
	assert(codes[3] == 46); // cyan background

	// Apply all typographic and background attrs without changing color:
	n = tb_sgr_ints(codes,
		TB_BOLD|TB_FAINT|TB_ITALIC|TB_UNDERLINE|TB_BLINK|TB_REVERSE);
	assert(n == 6);
	assert(codes[0] == 1);  // bold
	assert(codes[1] == 2);  // faint
	assert(codes[2] == 3);  // italic
	assert(codes[3] == 4);  // underline
	assert(codes[4] == 5);  // blink
	assert(codes[5] == 7);  // reverse

	// Attributes are inherited from the current context by default. So if
	// you send a SGR with bold on, all following text will be bold. You can
	// reset all foreground, background, and color attributes to their
	// default or off values before applying additional attributes to start
	// from scratch:
	n = tb_sgr_ints(codes, TB_RESET|TB_FAINT|TB_MAGENTA);
	assert(n == 3);
	assert(codes[0] == 0);  // reset
	assert(codes[1] == 2);  // faint
	assert(codes[2] == 35); // magenta foreground

	// It's also possible for an SGR value to turn its attributes off.
	// This lets you reset specific attributes while leaving others in tact:
	n = tb_sgr_ints(codes, TB_NEGATE|TB_BOLD|TB_REVERSE);
	assert(n == 2);
	assert(codes[0] == 22); // bold off (normal intensity)
	assert(codes[1] == 27); // reverse off

	// There's also the concept of default foreground and background colors
	// that may be different from any of the 8 base colors:
	n = tb_sgr_ints(codes, TB_DEFAULT);
	assert(n == 1);
	assert(codes[0] == 39);

	// Same as above but apply the default background color (remember, you
	// can't set both foreground and background in one sequence):
	n = tb_sgr_ints(codes, TB_DEFAULT|TB_BG);
	assert(n == 1);
	assert(codes[0] == 49); // default background color

	// Like all other colors, default colors may be applied along with any
	// other typographical attributes.
	n = tb_sgr_ints(codes, TB_UNDERLINE|TB_BLINK|TB_DEFAULT);
	assert(n == 3);
	assert(codes[0] == 4);  // underline
	assert(codes[1] == 5);  // blink
	assert(codes[2] == 39); // default foreground color

	// SGR values default to a 4-bit color mode but can be switched into
	// 216-color, 256-color, and 24-color greyscale modes. In these modes
	// you directly combine the color palette value with the other
	// attributes using the same binary OR operator.
	//
	// Switch into 216-color mode and apply color 172 to the foreground:
	n = tb_sgr_ints(codes, TB_ITALIC|TB_216|172);
	assert(n == 4);
	assert(codes[0] == 3);   // italic text
	assert(codes[1] == 38);  // set foreground color
	assert(codes[2] == 5);   // ...
	assert(codes[3] == 188); // to 172 (orange) in the 216-color palette

	// Switch into greyscale color mode and apply color 10 to background:
	n = tb_sgr_ints(codes, TB_UNDERLINE|TB_BG|TB_GREYSCALE|10);
	assert(n == 4);
	assert(codes[0] == 4);   // underline text
	assert(codes[1] == 48);  // set background color
	assert(codes[2] == 5);   // ...
	assert(codes[3] == 242); // to color 10 (burnt orange)

	return 0;
}

// vim: noexpandtab
