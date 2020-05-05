#include "../tbsgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * This is more of a demo / visual test to see if combining attributes in
 * extreme ways causes issues with terminal display.
 */

static void write_attr_label(char *label, uint32_t attrs) {
	int sz;
	tb_sgr_write(1, attrs);
	sz = write(1, label, strlen(label));
	tb_sgr_write(1, TB_NEGATE|attrs);
	sz = write(1, " ", 1);
	(void)sz;
}

#define writeln() sz=write(1, "\n", 1);(void)sz

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	int sz;

	// basic typographic attributes
	write_attr_label("bold",      TB_BOLD);
	write_attr_label("faint",     TB_FAINT);
	write_attr_label("italic",    TB_ITALIC);
	write_attr_label("underline", TB_UNDERLINE);
	write_attr_label("blink",     TB_BLINK);
	write_attr_label("reverse",   TB_REVERSE);
	write_attr_label("strike",    TB_STRIKE);
	writeln();

	char *colors[8] = {"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"};

	// foreground colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], TB_COLOR|i);
	}
	write_attr_label("default", TB_COLOR|TB_DEFAULT);
	writeln();

	// background colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], TB_COLOR|TB_BG|i);
	}
	write_attr_label("default", TB_COLOR|TB_BG|TB_DEFAULT);
	writeln();

	// reversed foreground colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], TB_REVERSE|TB_COLOR|i);
	}
	write_attr_label("default", TB_REVERSE|TB_COLOR|TB_DEFAULT);
	writeln();

	// reversed background colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], TB_REVERSE|TB_COLOR|TB_BG|i);
	}
	write_attr_label("default", TB_REVERSE|TB_COLOR|TB_BG|TB_DEFAULT);
	writeln();

	// 216-color mode
	for (int i = 0; i < 216; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, TB_216|TB_BG|i);
	}
	writeln();

	// 256-color mode
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, TB_256|TB_BG|i);
	}
	writeln();

	// 256-color mode with many attributes
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, TB_256|TB_BG|i|
		                 TB_BOLD|TB_ITALIC|TB_UNDERLINE|TB_STRIKE);
	}
	writeln();

	tb_sgr_write(1, TB_RESET);
	(void)sz;
	return 0;
}

// vim: noexpandtab
