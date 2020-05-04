#define _POSIX_C_SOURCE 200112L

#include "tbti.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <assert.h>

// Read an entire file into a string or return NULL if an error occurs.
// Caller must free the string returned.
static char *read_file(const char *file) {
	FILE *f = fopen(file, "rb");
	if (!f) {
		return 0;
	}

	struct stat st;
	if (fstat(fileno(f), &st) != 0) {
		fclose(f);
		return 0;
	}

	char *data = malloc(st.st_size);
	if (!data) {
		fclose(f);
		return 0;
	}

	if (fread(data, 1, st.st_size, f) != (size_t)st.st_size) {
		fclose(f);
		free(data);
		return 0;
	}

	fclose(f);
	return data;
}

// Look for the terminfo file for the given term under the given path.
static char *terminfo_try_path(const char *path, const char *term) {
	char fn[4096]; fn[sizeof(fn)-1]=0;
	snprintf(fn, sizeof(fn), "%s/%c/%s", path, term[0], term);
	char *data = read_file(fn);
	if (data) {
		return data;
	}

	// fallback to darwin specific dirs structure
	snprintf(fn, sizeof(fn), "%s/%x/%s", path, term[0], term);
	return read_file(fn);
}

// Read binary terminfo file.
static char *terminfo_load_data(const char *term) {
	// if TERMINFO is set, no other directory should be searched
	const char *terminfo = getenv("TERMINFO");
	if (terminfo) {
		return terminfo_try_path(terminfo, term);
	}

	// next, consider ~/.terminfo
	const char *home = getenv("HOME");
	if (home) {
		char fn[4096]; fn[sizeof(fn)-1]=0;
		snprintf(fn, sizeof(fn), "%s/.terminfo", home);
		char *data = terminfo_try_path(fn, term);
		if (data) {
			return data;
		}
	}

	// next, TERMINFO_DIRS
	const char *dirs = getenv("TERMINFO_DIRS");
	if (dirs) {
		char buf[4096]; buf[sizeof(buf)-1]=0;
		snprintf(buf, sizeof(buf), "%s", dirs);
		char *dir = strtok(buf, ":");
		while (dir) {
			const char *cdir = dir;
			if (strcmp(cdir, "") == 0) {
				cdir = "/usr/share/terminfo";
			}
			char *data = terminfo_try_path(cdir, term);
			if (data) {
				return data;
			}
			dir = strtok(0, ":");
		}
	}

	// fallback to /usr/share/terminfo
	return terminfo_try_path("/usr/share/terminfo", term);
}

#define TI_MAGIC 0432
#define TI_ALT_MAGIC 542

static TERMINAL *tb_term;

int setupterm(char *term, int fd) {
	if (!term) term = getenv("TERM");
	if (!term) return -1;

	if (tb_term && strcmp(term, tb_term->termname) == 0) {
		return 0;
	} else if (tb_term) {
		// TODO: free tb_term struct and members
	}

	char *data = terminfo_load_data(term);
	if (!data) {
		return -1;
	}

	tb_term = malloc(sizeof(TERMINAL));
	memset(tb_term, 0, sizeof(TERMINAL));

	tb_term->termdata = data;
	tb_term->termname = (char *)malloc(strlen(term) + 1);
	strcpy(tb_term->termname, term);
	tb_term->fd = fd;

	int16_t header[6];
	memcpy(header, data, sizeof(header));

	int16_t magic        = header[0], // magic number (octal 0432)
		names_len    = header[1], // size in bytes of the names section
		bools_len    = header[2], // size in bytes of the bools section
		nums_count   = header[3], // count of shorts in nums section
		stroff_count = header[4], // count of shorts in stroffs section
		strtbl_len   = header[5], // size in bytes of the string table
		num_size     = magic == TI_ALT_MAGIC ? 4 : 2; // TODO: handle int32 nums?

	assert(magic == TI_MAGIC);
	// TODO bail if magic is wrong

	TERMTYPE *type = &tb_term->type;
	type->term_names = data + sizeof(header);

	type->bools = (int8_t*)(type->term_names + names_len);
	type->num_bools = (uint16_t)bools_len;

	type->nums = (int16_t*)(type->bools + type->num_bools + ((names_len + bools_len) % 2));
	type->num_nums = (uint16_t)nums_count;

	type->str_offs = type->nums + nums_count;
	type->num_strings = stroff_count;

	type->str_table = (char *)(type->str_offs + stroff_count);

	type->ext_str_table = type->str_table + strtbl_len;

	// TODO load extended capabilities

	return 0;
}

int tigetflag(int cap) {
	if (!tb_term) return -1;
	if (cap < 0 || cap > tb_term->type.num_bools) return -1;

	return tb_term->type.bools[cap];
}

int tigetnum(int cap) {
	if (!tb_term) return -1;
	if (cap < 0 || cap > tb_term->type.num_nums) return -1;

	return tb_term->type.nums[cap];
}

char *tigetstr(int cap) {
	if (!tb_term) return NULL;
	if (cap < 0 || cap > tb_term->type.num_strings) return NULL;

	int offset = tb_term->type.str_offs[cap];
	if (offset < 0) return NULL;
	// TODO: check offset isn't larger than str_table

	return tb_term->type.str_table + tb_term->type.str_offs[cap];
}

#ifdef TESTNOW
int main(void) {
	int rc = setupterm(NULL, 1);
	setupterm(NULL, 1);
	printf("term_names: %s\n", tb_term->type.term_names);
	printf("str 27 = %s\n", tb_term->type.str_table + tb_term->type.str_offs[27]);
	printf("tigetflag(tb_back_color_erase) = %d\n", tigetflag(tb_back_color_erase));
	printf("tigetflag(tb_auto_right_margin) = %d\n", tigetflag(tb_auto_right_margin));
	printf("tigetflag(tb_has_status_line) = %d\n", tigetflag(tb_has_status_line));
	printf("tigetnum(tb_columns) = %d\n", tigetnum(tb_columns));
	printf("tigetnum(tb_init_tabs) = %d\n", tigetnum(tb_init_tabs));
	printf("tigetnum(tb_buttons) = %d\n", tigetnum(tb_buttons));
	printf("tigetstr(tb_bell) = %s\n", tigetstr(tb_bell));
	printf("tigetstr(tb_set_a_background) = %s\n", tigetstr(tb_set_a_background));
	return rc;
}
#endif

// vim: noexpandtab
