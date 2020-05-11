#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	int err;

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-color", &err);
	assert(ti != NULL);

	// read boolean capabilities with the ti_getflag() function
	// returns 1 if the terminal has the capability or 0 if not
	int has_meta_key     = ti_getflag(ti, ti_km),
	    back_color_erase = ti_getflag(ti, ti_bce),
	    hard_copy        = ti_getflag(ti, ti_hc);
	assert(has_meta_key == 1);
	assert(back_color_erase == 0);
	assert(hard_copy == 0);

	// read numeric capabilities with the ti_getnum() function:
	int colors = ti_getnum(ti, ti_colors);
	assert(colors == 8);

	// numeric capabilities not supported by the terminal return -1:
	int width_status_line = ti_getnum(ti, ti_wsl);
	assert(width_status_line == -1);

	// read string capabilities with the ti_getstr() function:
	char *clr_eol = ti_getstr(ti, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// string capabilties not specified in the terminfo file return NULL:
	char *insert_padding = ti_getstr(ti, ti_ip);
	assert(insert_padding == NULL);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);

	return 0;
}

// vim: noexpandtab
