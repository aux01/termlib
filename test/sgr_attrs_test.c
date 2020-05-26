#include "../sgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * This is more of a demo / visual test to see if combining attributes in
 * extreme ways causes issues with terminal display.
 */

static void write_attr_label(char *label, struct sgr sgr)
{
	int sz;
	sgr_write(1, sgr);
	sz = write(1, label, strlen(label));
	sgr_write(1, (struct sgr){ SGR_NEGATE|sgr.at });
	sz = write(1, " ", 1);
	(void)sz;
}

#define writeln() do { sz=write(1, "\n", 1);(void)sz; } while(0);

static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return (r<<16 | g<<8 | b);
}

int main(void)
{
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	int sz;
	struct sgr sgr = {0};

	// basic typographic attributes
	write_attr_label("bold",      (struct sgr){SGR_BOLD});
	write_attr_label("faint",     (struct sgr){SGR_FAINT});
	write_attr_label("italic",    (struct sgr){SGR_ITALIC});
	write_attr_label("underline", (struct sgr){SGR_UNDERLINE});
	write_attr_label("blink",     (struct sgr){SGR_BLINK});
	write_attr_label("reverse",   (struct sgr){SGR_REVERSE});
	write_attr_label("strike",    (struct sgr){SGR_STRIKE});
	writeln();
	writeln();

	char *colors[8] = {
		"black", "red", "green", "yellow",
		"blue", "magenta", "cyan", "white"
	};

	// foreground colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (struct sgr){SGR_FG, i});
	}
	write_attr_label("default", (struct sgr){SGR_FG, SGR_DEFAULT});
	writeln();
	writeln();

	// background colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (struct sgr){SGR_BG, 0, i});
	}
	write_attr_label("default", (struct sgr){SGR_BG, 0, SGR_DEFAULT});
	writeln();
	writeln();

	// reversed foreground colors
	for (int i = 0; i < 8; i++) {
		sgr = (struct sgr){SGR_REVERSE, i};
		write_attr_label(colors[i], sgr);
	}
	sgr = (struct sgr){SGR_REVERSE|SGR_FG, SGR_DEFAULT};
	write_attr_label("default", sgr);
	writeln();
	writeln();

	// reversed background colors
	for (int i = 0; i < 8; i++) {
		sgr = (struct sgr){SGR_REVERSE|SGR_BG, 0, i};
		write_attr_label(colors[i], sgr);
	}
	sgr = (struct sgr){SGR_REVERSE|SGR_BG, 0, SGR_DEFAULT};
	write_attr_label("default", sgr);
	writeln();
	writeln();

	// 216-color mode cube
	for (int g = 0; g < 6; g++) {
		for (int r = 0; r < 6; r++) {
			for (int b = 0; b < 6; b++) {
				int color = (r * 36) + (g * 6) + b;
				char buf[4];
				sprintf(buf, "%03d", color);
				sgr = (struct sgr){ SGR_BG216, 0, color };
				write_attr_label(buf, sgr);
			}
			sz = write(1, " ", 1); (void)sz;
		}
		writeln();
	}
	writeln();

	// 256-color mode
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, (struct sgr){SGR_BG256, 0, i});
		if ((i < 16 && (i+1) % 8 == 0) ||
		    (i > 16 && (i-16+1) % 12 == 0))
			writeln();
	}
	writeln();

	// 256-color mode with many attributes
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		sgr = (struct sgr) {
			.at = SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|
			      SGR_STRIKE|SGR_BG256,
			.bg = i
		};
		write_attr_label(buf, sgr);
		if ((i < 16 && (i+1) % 8 == 0) ||
		    (i > 16 && (i-16+1) % 12 == 0))
			writeln();
	}
	writeln();

	// true color mode
	for (int colnum = 0; colnum < 77; colnum++) {
		uint32_t r = 255-(colnum*255/76);
		uint32_t g = (colnum*510/76);
		uint32_t b = (colnum*255/76);
		if (g>255) g = 510-g;
		sgr = (struct sgr) {
			.at = SGR_FG16M|SGR_BG16M,
			.fg = rgb(255-r, 255-g, 255-b),
			.bg = rgb(r, g, b)
		};
		sgr_write(1, sgr);
		sz = write(1, ":", 1);
	}
	writeln();

	sgr_write(1, (struct sgr){SGR_RESET});
	(void)sz;
	return 0;
}

// vim: noexpandtab
