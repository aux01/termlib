#include "../sgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// buffer for holding sgr codes
	uint16_t codes[SGR_ELMS_MAX];

	// The sgr_t struct type has a set of attribute bitflags, a
	// foreground color, and a background color.
	sgr_t sgr = {
		.at = SGR_BOLD|SGR_ITALIC|SGR_FG|SGR_BG,
		.fg = SGR_RED,
		.bg = SGR_CYAN
	};
	int n = sgr_unpack(codes, sgr);
	assert(n == 4);         // number of codes written
	assert(codes[0] == 1);  // bold text
	assert(codes[1] == 3);  // italic text
	assert(codes[2] == 31); // red text/foreground color
	assert(codes[3] == 46); // cyan background color

	// You can also construct them on the fly using C99 compound literal
	// syntax. Here we create an SGR for bold text on a green background:
	n = sgr_unpack(codes, (sgr_t){.at=SGR_BOLD|SGR_BG, .bg=SGR_GREEN});
	assert(n == 2);
	assert(codes[0] == 1);  // bold text
	assert(codes[1] == 42); // green background

	// The default 8-color palette includes normal intensity colors.
	// You can access another 8 high intensity colors by applying the
	// SGR_FG16 or SGR_BG16 attribute instead of SG_FG/SG_BG:
	n = sgr_unpack(codes, (sgr_t){.at=SGR_FG16, .fg=SGR_YELLOW});
	assert(n == 1);
	assert(codes[0] == 93); // bright yellow foreground

	// Apply all typographic and background attrs without changing color:
	n = sgr_unpack(codes, (sgr_t){SGR_BOLD|SGR_FAINT|SGR_ITALIC|
	                              SGR_UNDERLINE|SGR_BLINK|SGR_REVERSE|
	                              SGR_STRIKE});
	assert(n == 7);
	assert(codes[0] == 1);  // bold
	assert(codes[1] == 2);  // faint
	assert(codes[2] == 3);  // italic
	assert(codes[3] == 4);  // underline
	assert(codes[4] == 5);  // blink
	assert(codes[5] == 7);  // reverse
	assert(codes[6] == 9);  // strike

	// Attributes are inherited from the current context by default. So if
	// you send a SGR with bold on, all following text will be bold unless
	// explitly reset. You can reset all foreground, background, and color
	// attributes to their default off values:
	n = sgr_unpack(codes, (sgr_t){SGR_RESET|SGR_FAINT|SGR_FG, SGR_MAGENTA});
	assert(n == 3);
	assert(codes[0] == 0);  // reset
	assert(codes[1] == 2);  // faint
	assert(codes[2] == 35); // magenta foreground

	// It's also possible for an SGR value to turn its attributes off.
	// This lets you reset specific attributes while leaving others in tact:
	n = sgr_unpack(codes, (sgr_t){SGR_NEGATE|SGR_BOLD|SGR_REVERSE|SGR_FG});
	assert(n == 3);
	for (int i = 0; i < n; i++)
		printf("codes[%d] = %d\n", i, codes[i]);
	assert(codes[0] == 22); // bold off (normal intensity)
	assert(codes[1] == 27); // reverse off
	assert(codes[2] == 39); // default foreground color

	// There's also the concept of default foreground and background colors
	// that may be different from any of the 8 base colors:
	n = sgr_unpack(codes, (sgr_t){SGR_FG, SGR_DEFAULT});
	assert(n == 1);
	assert(codes[0] == 39);

	// Same as above but apply the default foreground and background color:
	n = sgr_unpack(codes, (sgr_t){SGR_FG|SGR_BG, SGR_DEFAULT, SGR_DEFAULT});
	assert(n == 2);
	assert(codes[0] == 39); // default foreground color
	assert(codes[1] == 49); // default background color

	// Like all other colors, default colors may be applied along with any
	// other typographical attributes.
	n = sgr_unpack(codes, (sgr_t){SGR_UNDERLINE|SGR_BLINK|SGR_FG,
	                              SGR_DEFAULT});
	assert(n == 3);
	assert(codes[0] == 4);  // underline
	assert(codes[1] == 5);  // blink
	assert(codes[2] == 39); // default foreground color

	// The SGR_FG and SGR_BG attributes cause the fg and bg colors to be
	// interpreted as 8-color mode colors. You can switch into 216-color,
	// 256-color, and 24-color greyscale modes as well.
	//
	// Switch to 216-color mode and apply color 172 to the fg:
	n = sgr_unpack(codes, (sgr_t){SGR_ITALIC|SGR_FG216, 172});
	assert(n == 4);
	assert(codes[0] == 3);   // italic text
	assert(codes[1] == 38);  // set foreground color
	assert(codes[2] == 5);   // ...
	assert(codes[3] == 188); // to 172 (orange) in the 216-color palette

	// Switch into 24-color greyscale mode and apply color 10 to the bg:
	n = sgr_unpack(codes, (sgr_t){SGR_UNDERLINE|SGR_BG24, 0, 10});
	assert(n == 4);
	assert(codes[0] == 4);   // underline text
	assert(codes[1] == 48);  // set background color
	assert(codes[2] == 5);   // ...
	assert(codes[3] == 242); // to color 10 (burnt orange)

	return 0;
}

// vim: noexpandtab
