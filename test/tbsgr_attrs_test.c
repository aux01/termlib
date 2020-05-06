#include "../tbsgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * This is more of a demo / visual test to see if combining attributes in
 * extreme ways causes issues with terminal display.
 */

static void write_attr_label(char *label, sgr_t sgr) {
	int sz;
	sgr_write(1, sgr);
	sz = write(1, label, strlen(label));
	sgr_write(1, (sgr_t){ SGR_NEGATE|sgr.at });
	sz = write(1, " ", 1);
	(void)sz;
}

#define writeln() sz=write(1, "\n", 1);(void)sz

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	int sz;

	// basic typographic attributes
	write_attr_label("bold",      (sgr_t){SGR_BOLD});
	write_attr_label("faint",     (sgr_t){SGR_FAINT});
	write_attr_label("italic",    (sgr_t){SGR_ITALIC});
	write_attr_label("underline", (sgr_t){SGR_UNDERLINE});
	write_attr_label("blink",     (sgr_t){SGR_BLINK});
	write_attr_label("reverse",   (sgr_t){SGR_REVERSE});
	write_attr_label("strike",    (sgr_t){SGR_STRIKE});
	writeln();

	char *colors[8] = {"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"};

	// foreground colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (sgr_t){SGR_FG, i});
	}
	write_attr_label("default", (sgr_t){SGR_FG, SGR_DEFAULT});
	writeln();

	// background colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (sgr_t){SGR_BG, 0, i});
	}
	write_attr_label("default", (sgr_t){SGR_BG, 0, SGR_DEFAULT});
	writeln();

	// reversed foreground colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (sgr_t){SGR_REVERSE, i});
	}
	write_attr_label("default", (sgr_t){SGR_REVERSE|SGR_FG, SGR_DEFAULT});
	writeln();

	// reversed background colors
	for (int i = 0; i < 8; i++) {
		write_attr_label(colors[i], (sgr_t){SGR_REVERSE|SGR_BG, 0, i});
	}
	write_attr_label("default", (sgr_t){SGR_REVERSE|SGR_BG, 0, SGR_DEFAULT});
	writeln();

	// 216-color mode
	for (int i = 0; i < 216; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, (sgr_t){SGR_BG216, 0, i});
	}
	writeln();

	// 256-color mode
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, (sgr_t){SGR_BG256, 0, i});
	}
	writeln();

	// 256-color mode with many attributes
	for (int i = 0; i < 256; i++) {
		char buf[4];
		sprintf(buf, "%03d", i);
		write_attr_label(buf, (sgr_t){
			.at = SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|SGR_STRIKE|
			      SGR_BG256,
			.bg = i
		});
	}
	writeln();

	sgr_write(1, (sgr_t){SGR_RESET});
	(void)sz;
	return 0;
}

// vim: noexpandtab
