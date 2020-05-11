#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void test_legacy_storage_format() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_term *t = ti_setupterm("xterm-color", 1, &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(t != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	char *term_names = "xterm-color|generic \"ANSI\" color xterm (X Window System)";
	assert(strcmp(term_names, t->info.names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded for xterm-color.
	assert(t->info.bools_count == 38);
	assert(t->info.nums_count == 16);
	assert(t->info.strs_count == 413);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getflag(t, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnum(t, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstr(t, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// when you're done, remember to free terminal info memory:
	ti_freeterm(t);
}

void test_extended_storage_format() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_term *t = ti_setupterm("xterm-new", 1, &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(t != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	printf("t->info.names=%s\n", t->info.names);
	char *term_names = "xterm-new|modern xterm terminal emulator";
	assert(strcmp(term_names, t->info.names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded for xterm-color.
	printf("bools_count=%d, nums_count=%d, strs_count=%d\n",
	       t->info.bools_count, t->info.nums_count, t->info.strs_count);
	assert(t->info.bools_count == 38);
	assert(t->info.nums_count == 15);
	assert(t->info.strs_count == 413);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getflag(t, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnum(t, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstr(t, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// when you're done, remember to free terminal info memory:
	ti_freeterm(t);
}

// Loading a file that doesn't exist causes an error.
void test_missing_file() {
	int err = 0;
	ti_term *t = ti_setupterm("xterm-missing", 1, &err);
	printf("err=%d\n", err);
	assert(t == NULL);
	assert(err == TI_ERR_FILE_NOT_FOUND);
	ti_freeterm(t);
}

// Loading a file that exists but isn't a terminfo file causes an error.
void test_non_terminfo_file() {
	int err;
	ti_term *t = ti_setupterm("xterm-badfile", 1, &err);
	printf("err=%d\n", err);
	assert(t == NULL);
	assert(err == TI_ERR_FILE_INVALID);
	ti_freeterm(t);
}

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	test_legacy_storage_format();
	test_extended_storage_format();
	test_missing_file();
	test_non_terminfo_file();

	return 0;
}

// vim: noexpandtab
