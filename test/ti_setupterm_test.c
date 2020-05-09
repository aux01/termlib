#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	// loading a file that doesn't exist causes an error
	int err;
	ti_term *t = ti_setupterm("xterm-nooooope", 1, &err);
	assert(t == NULL);
	printf("err = %d\n", err);
	assert(err == TI_ERR_FILE_NOT_FOUND);

	// loading a file that exists but isn't a terminfo file causes an error
	t = ti_setupterm("xterm-badfile", 1, &err);
	assert(t == NULL);
	assert(err == TI_ERR_FILE_INVALID);

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	t = ti_setupterm("xterm-color", 1, &err);
	assert(t != NULL);
	assert(err == 0);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	char *term_names = "xterm-color|generic \"ANSI\" color xterm (X Window System)";
	assert(strcmp(term_names, t->info.names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded for xterm-color.
	assert(t->info.num_bools == 38);
	assert(t->info.num_nums == 16);
	assert(t->info.num_stroffs == 413);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getflag(t, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnum(t, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstr(t, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// when you're done, remember to free terminal info memory:
	ti_freeterm(t);

	return 0;
}

// vim: noexpandtab
