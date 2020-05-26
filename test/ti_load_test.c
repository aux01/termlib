#define _XOPEN_SOURCE 700    // setenv

#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

void test_legacy_storage_format() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-color", &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(ti != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	printf("ti->term_names=%s\n", ti->term_names);
	char *term_names = "xterm-color|generic \"ANSI\" color xterm (X Window System)";
	assert(strcmp(term_names, ti->term_names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded for xterm-color.
	printf("bools_count=%d, nums_count=%d, strs_count=%d\n",
	       ti->bools_count, ti->nums_count, ti->strs_count);
	assert(ti->bools_count == 38);
	assert(ti->nums_count == 16);
	assert(ti->strs_count == 413);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getbooli(ti, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnumi(ti, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstri(ti, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

void test_extended_storage_format() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-new", &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(ti != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	printf("ti->term_names=%s\n", ti->term_names);
	char *term_names = "xterm-new|modern xterm terminal emulator";
	assert(strcmp(term_names, ti->term_names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded.
	printf("bools_count=%d, nums_count=%d, strs_count=%d\n",
	       ti->bools_count, ti->nums_count, ti->strs_count);
	assert(ti->bools_count == 38);
	assert(ti->nums_count == 15);
	assert(ti->strs_count == 413);

	// read some legacy capabilities
	int has_meta_key = ti_getbooli(ti, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnumi(ti, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstri(ti, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// terminfo files have extended bool, numeric, and string capability
	// entries. verify the correct number of entries were loaded.
	printf("ext_bools_count=%d, ext_nums_count=%d, ext_strs_count=%d\n",
	       ti->ext_bools_count, ti->ext_nums_count, ti->ext_strs_count);
	assert(ti->ext_bools_count == 2);
	assert(ti->ext_nums_count == 0);
	assert(ti->ext_strs_count == 60);

	// read some extended capabilities
	int has_ax = ti_getbool(ti, "AX");
	assert(has_ax == 1);
	char *smxx = ti_getstr(ti, "smxx");
	assert(strcmp("\x1b[9m", smxx) == 0);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

void test_extended_storage_format_odd_alignment() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("xterm-kitty", &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(ti != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	printf("ti->term_names=%s\n", ti->term_names);
	char *term_names = "xterm-kitty|KovIdTTY";
	assert(strcmp(term_names, ti->term_names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded.
	printf("bools_count=%d, nums_count=%d, strs_count=%d\n",
	       ti->bools_count, ti->nums_count, ti->strs_count);
	assert(ti->bools_count == 28);
	assert(ti->nums_count == 15);
	assert(ti->strs_count == 361);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getbooli(ti, ti_km);
	assert(has_meta_key == 1);
	int colors = ti_getnumi(ti, ti_colors);
	assert(colors == 256);
	char *clr_eol = ti_getstri(ti, ti_el);
	assert(strcmp("\x1b[K", clr_eol) == 0);

	// terminfo files have extended bool, numeric, and string capability
	// entries. verify the correct number of entries were loaded.
	printf("ext_bools_count=%d, ext_nums_count=%d, ext_strs_count=%d\n",
	       ti->ext_bools_count, ti->ext_nums_count, ti->ext_strs_count);
	assert(ti->ext_bools_count == 3);
	assert(ti->ext_nums_count == 0);
	assert(ti->ext_strs_count == 56);

	// read some extended capabilities
	int fullkbd = ti_getbool(ti, "fullkbd");
	assert(fullkbd == 1);
	char *smxx = ti_getstr(ti, "smxx");
	assert(strcmp("\x1b[9m", smxx) == 0);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

// The minitel1 terminfo file is unique in a number of ways. There are many long
// binary string sequences
void test_minitel1() {
	int err = 0;

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_terminfo *ti = ti_load("minitel1", &err);
	printf("err=%d\n", err);
	assert(err == 0);
	assert(ti != NULL);

	// the terminfo data includes multiple alternative names separated by
	// pipe characters:
	printf("ti->term_names=%s\n", ti->term_names);
	char *term_names = "minitel1|minitel 1";
	assert(strcmp(term_names, ti->term_names) == 0);

	// terminfo files have bool, numeric, and string capability entries.
	// verify the correct number of entries were loaded.
	printf("bools_count=%d, nums_count=%d, strs_count=%d\n",
	       ti->bools_count, ti->nums_count, ti->strs_count);
	assert(ti->bools_count == 19);
	assert(ti->nums_count == 15);
	assert(ti->strs_count == 361);

	// read some capabilities to verify the db is being processed correctly
	int has_meta_key = ti_getbooli(ti, ti_km);
	assert(has_meta_key == 0);
	int colors = ti_getnumi(ti, ti_colors);
	assert(colors == 8);
	char *clr_eol = ti_getstri(ti, ti_el);
	assert(strcmp("\x18", clr_eol) == 0);

	// terminfo files have extended bool, numeric, and string capability
	// entries. verify the correct number of entries were loaded.
	printf("ext_bools_count=%d, ext_nums_count=%d, ext_strs_count=%d\n",
	       ti->ext_bools_count, ti->ext_nums_count, ti->ext_strs_count);
	assert(ti->ext_bools_count == 1);
	assert(ti->ext_nums_count == 0);
	assert(ti->ext_strs_count == 4);

	// read some extended capabilities
	int fullkbd = ti_getbool(ti, "G0");
	assert(fullkbd == 1);
	char *xc = ti_getstr(ti, "XC");
	char *expect = "B\031%,\241!,\242\",\243#,\244$,\245%,\246&,\247',\250("
	               ",\253+,\257P,\2600,\2611,\2622,\2633,\2655,\2677,\272k,"
		       "\273;,\274<,\275=,\276>,\277?,\300AA,\301BA,\302CA,\303"
		       "DA,\304HA,\305JA,\306a,\307KC,\310AE,\311BE,\312CE,\313"
		       "HE,\314AI,\315BI,\316CI,\317HI,\320b,\321DN,\322AO,\323"
		       "BO,\324CO,\325DO,\326HO,\3274,\330i,\331AU,\332BU,\333C"
		       "U,\334HU,\335BY,\336l,\337{,\340Aa,\341Ba,\342Ca,\343Da"
		       ",\344Ha,\345Ja,\346q,\347Kc,\350Ae,\351Be,\352Ce,\353He"
		       ",\354Ai,\355Bi,\356Ci,\357Hi,\360r,\361Dn,\362Ao,\363Bo"
		       ",\364Co,\365Do,\366Ho,\3678,\370y,\371Au,\372Bu,\373Cu,"
		       "\374Hu,\375By,\376|,\377Hy,\252c,,0\017\031%\016,}#,f0,"
		       "g1,\\,\\,,+.,./,0\177,--";
	assert(strcmp(expect, xc) == 0);

	// when you're done, remember to free terminal info memory:
	ti_free(ti);
}

// Loading a file that doesn't exist causes an error.
void test_missing_file() {
	int err = 0;
	ti_terminfo *ti = ti_load("xterm-missing", &err);
	printf("err=%d\n", err);
	assert(ti == NULL);
	assert(err == ENOENT);
	ti_free(ti);
}

// Loading a file that exists but isn't a terminfo file causes an error.
void test_non_terminfo_file() {
	int err;
	ti_terminfo *ti = ti_load("xterm-badfile", &err);
	printf("err=%d\n", err);
	assert(ti == NULL);
	assert(err == TI_ERR_BAD_MAGIC);
	ti_free(ti);
}

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	test_legacy_storage_format();
	test_extended_storage_format();
	test_extended_storage_format_odd_alignment();
	test_minitel1();
	test_missing_file();
	test_non_terminfo_file();

	return 0;
}

// vim: noexpandtab
