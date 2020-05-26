#define _XOPEN_SOURCE 700    // setenv

#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void test_getcaps_by_name() {
	int err;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-color", &err);
	assert(ti != NULL);

	// read boolean capabilities with the ti_getbool() function
	// returns 1 if the terminal has the capability or 0 if not
	int has_meta_key     = ti_getbool(ti, "km"),
	    back_color_erase = ti_getbool(ti, "bce"),
	    hard_copy        = ti_getbool(ti, "hc");
	assert(has_meta_key == 1);
	assert(back_color_erase == 0);
	assert(hard_copy == 0);

	// unrecognized boolean capabilities return 0 like unset capabilities so
	// be careful with spelling...
	int has_imaginery_cap = ti_getbool(ti, "imagineryboolname");
	assert(has_imaginery_cap == 0);

	// read numeric capabilities with the ti_getnumi() function:
	int colors = ti_getnum(ti, "colors");
	assert(colors == 8);

	// numeric capabilities not supported by the terminal return -1:
	int width_status_line = ti_getnum(ti, "wsl");
	assert(width_status_line == -1);

	// unrecognized numeric capabilities return -1 like unsupported
	// capabilities so be careful with spelling...
	int imaginery_num_cap = ti_getnum(ti, "imaginerynumname");
	assert(imaginery_num_cap == -1);

	// read string capabilities with the ti_getstr() function:
	char *clr_eol = ti_getstr(ti, "el");
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// string capabilties not specified in the terminfo file return NULL:
	char *insert_padding = ti_getstr(ti, "ip");
	assert(insert_padding == NULL);

	// unrecognized string capabilities return NULL like unsupported
	// capabilities so be careful with spelling...
	char *imaginery_str_cap = ti_getstr(ti, "imaginerystrname");
	assert(imaginery_str_cap == NULL);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

void test_getcaps_by_name_extended() {
	int err;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-new", &err);
	assert(ti != NULL);

	// read boolean extended capabilities with the ti_getbool() function
	// returns 1 if the terminal has the capability or 0 if not
	int has_set_color = ti_getbool(ti, "AX"),
	    has_xt = ti_getbool(ti, "XT"),
	    has_unknown = ti_getbool(ti, "NOTACAP");
	assert(has_set_color == 1);
	assert(has_xt == 1);
	assert(has_unknown == 0);

	// read numeric capabilities with the ti_getnumi() function:
	// TODO: need a terminfo file with extended numeric capabilities

	// read string capabilities with the ti_getstr() function:
	char *cross_out_on  = ti_getstr(ti, "smxx"),
	     *cross_out_off = ti_getstr(ti, "rmxx");
	assert(strcmp("\x1b[9m", cross_out_on) == 0);
	assert(strcmp("\x1b[29m", cross_out_off) == 0);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

void test_getcaps_by_index() {
	int err;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-color", &err);
	assert(ti != NULL);

	// read boolean capabilities with the ti_getbooli() function
	// returns 1 if the terminal has the capability or 0 if not
	int has_meta_key     = ti_getbooli(ti, ti_km),
	    back_color_erase = ti_getbooli(ti, ti_bce),
	    hard_copy        = ti_getbooli(ti, ti_hc);
	assert(has_meta_key == 1);
	assert(back_color_erase == 0);
	assert(hard_copy == 0);

	// read numeric capabilities with the ti_getnumi() function:
	int colors = ti_getnumi(ti, ti_colors);
	assert(colors == 8);

	// numeric capabilities not supported by the terminal return -1:
	int width_status_line = ti_getnumi(ti, ti_wsl);
	assert(width_status_line == -1);

	// read string capabilities with the ti_getstri() function:
	char *clr_eol = ti_getstri(ti, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// string capabilties not specified in the terminfo file return NULL:
	char *insert_padding = ti_getstri(ti, ti_ip);
	assert(insert_padding == NULL);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

int main(void) {
	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	test_getcaps_by_index();
	test_getcaps_by_name();
	test_getcaps_by_name_extended();

	return 0;
}

// vim: noexpandtab
