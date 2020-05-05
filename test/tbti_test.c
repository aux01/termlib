#include "../tbti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	int rc;

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	// loading a file that doesn't exist returns err: -2
	rc = tb_setupterm("xterm-nooooope", 1);
	assert(rc == -2);

	// loading a file that exists but not a terminfo file returns err: -3
	rc = tb_setupterm("xterm-badfile", 1);
	assert(rc == -3);

	// load the terminfo data into the global tb_term struct and associate
	// with standard output:
	rc = tb_setupterm("xterm-color", 1);
	assert(rc == 0);
	assert(tb_term != NULL);

	// multiple same calls to tb_setupterm doesn't load the data again:
	tb_terminal *orig = tb_term;
	rc = tb_setupterm("xterm-color", 1);
	assert(rc == 0);
	assert(tb_term == orig);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	char *term_names = "xterm-color|generic \"ANSI\" color xterm (X Window System)";
	assert(strcmp(term_names, tb_term->type.term_names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded for xterm-color.
	assert(tb_term->type.num_bools == 38);
	assert(tb_term->type.num_nums == 16);
	assert(tb_term->type.num_strings == 413);

	// read boolean capabilities with the tb_getflag() function:
	int has_meta_key     = tb_getflag(tb_km),
	    back_color_erase = tb_getflag(tb_bce),
	    hard_copy        = tb_getflag(tb_hc);
	assert(has_meta_key == 1);
	assert(back_color_erase == 0);
	assert(hard_copy == 0);

	// read numeric capabilities with the tb_getnum() function:
	int colors = tb_getnum(tb_colors);
	assert(colors == 8);

	// numeric capabilities not specified in the terminfo file return -1:
	int width_status_line = tb_getnum(tb_wsl);
	assert(width_status_line == -1);

	// read string capabilities with the tb_getstr() function:
	char *clr_eol = tb_getstr(tb_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// string capabilties not specified in the terminfo file return NULL:
	char *insert_padding = tb_getstr(tb_ip);
	assert(insert_padding == NULL);

	// you can use tb_loadterm instead of tb_setupterm to load terminfo
	// data without setting the current terminal globally:
	tb_terminal *t = malloc(sizeof(tb_terminal));
	rc = tb_loadterm(t, "xterm-color", 1);
	assert(rc == 0);

	// when you're done, remember to free terminal info memory:
	tb_freeterm(t);

	return 0;
}

// vim: noexpandtab
