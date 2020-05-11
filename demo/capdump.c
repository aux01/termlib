/*
 *
 * capdump.c - Write all terminal capabilities to standard output.
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 *
 */

#include "../ti.h"

#include <stdio.h>
#include <stdlib.h>    // getenv

int main(void) {
	char * term = getenv("TERM");

	int err;
	ti_terminfo *ti = ti_load(term, &err);
	if (!ti) {
		switch (err) {
		case TI_ERR_FILE_NOT_FOUND:
			fprintf(stderr,
			        "error: file not found: term: %s\n",
				term);
			break;
		case TI_ERR_FILE_INVALID:
			fprintf(stderr,
			        "error: invalid terminfo file: term: %s\n",
				term);
			break;
		case TI_ERR_FILE_CORRUPT:
			fprintf(stderr,
			        "error: terminfo file is corrupt: term: %s\n",
				term);
			break;
		}
		return 1;
	}

	printf("# %s\n", ti->term_names);
	for (int i = 0; i < ti->bools_count; i++) {
		printf("%s %s=%d\n", "std bool", ti_boolnames[i], ti_getbooli(ti, i));
	}
	for (int i = 0; i < ti->ext_bools_count; i++) {
		printf("%s %s=%d\n", "ext bool", ti->ext_names[i], ti->ext_bools[i]);
	}
	for (int i = 0; i < ti->nums_count; i++) {
		printf("%s %s=%d\n", "std num", ti_numnames[i], ti_getnumi(ti, i));
	}
	int offset = ti->ext_bools_count;
	for (int i = 0; i < ti->ext_nums_count; i++) {
		printf("%s %s=%d\n", "ext num", ti->ext_names[offset+i], ti->ext_nums[i]);
	}
	for (int i = 0; i < ti->strs_count; i++) {
		printf("%s %s=%s\n", "std str", ti_strnames[i], ti_getstri(ti, i));
	}
	offset+= ti->ext_nums_count;
	for (int i = 0; i < ti->ext_strs_count; i++) {
		printf("%s %s=%s\n", "ext str", ti->ext_names[offset+i], ti->ext_strs[i]);
	}

	return 0;
}
