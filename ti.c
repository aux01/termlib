/*
 *
 * ti.c - Minimal, standalone terminfo(5) processor.
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * See ti.h for usage and interface documentation.
 *
 *
 */

#define _POSIX_C_SOURCE 200112L

#include "ti.h"

#include <stdint.h>
#include <stdarg.h>
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
	char *data;

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
		if ((data = terminfo_try_path(fn, term))) {
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

	// search in system paths
	// TODO: may be different on different systems; use `infocmp -D' to ls
	char *paths[] = {
		"/etc/terminfo",
		"/lib/terminfo",
		"/usr/share/terminfo",
		""
	};
	for (int i = 0; paths[i][0]; i++) {
		data = terminfo_try_path(paths[i], term);
		if (data) return data;
	}

	return NULL;
}

#define TI_MAGIC 0432
#define TI_ALT_MAGIC 542  // TODO: version of terminfo with 32-bit int nums?

ti_term *ti_setupterm(const char *termname, int fd, int *err) {
	if (!termname) termname = getenv("TERM");
	if (!termname) {
		if (err) *err = TI_ERR_TERM_NOT_SET;
		return NULL;
	}

	char *data = terminfo_load_data(termname);
	if (!data) {
		if (err) *err = TI_ERR_FILE_NOT_FOUND;
		return NULL;
	}

	ti_term *term = calloc(1, sizeof(ti_term));

	term->data = data;
	term->name = (char *)malloc(strlen(termname) + 1);
	strcpy(term->name, termname);
	term->fd = fd;

	// TODO: check that there's actually this much data loaded from the file
	int16_t header[6];
	memcpy(header, data, sizeof(header));

	int16_t magic        = header[0], // magic number (octal 0432)
		names_len    = header[1], // size in bytes of the names section
		bools_len    = header[2], // size in bytes of the bools section
		nums_count   = header[3], // count of shorts in nums section
		stroff_count = header[4], // count of shorts in stroffs section
		strtbl_len   = header[5]; // size in bytes of the string table

	if (magic != TI_MAGIC) {
		if (err) *err = TI_ERR_FILE_INVALID;
		ti_freeterm(term);
		return NULL;
	}

	ti_terminfo *info = &term->info;
	info->names = data + sizeof(header);

	info->bools = (int8_t*)(info->names + names_len);
	info->num_bools = (uint16_t)bools_len;

	info->nums = (int16_t*)(info->bools + info->num_bools + ((names_len + bools_len) % 2));
	info->num_nums = (uint16_t)nums_count;

	info->str_offs = info->nums + nums_count;
	info->num_strings = stroff_count;

	info->str_table = (char *)(info->str_offs + stroff_count);

	info->ext_str_table = info->str_table + strtbl_len;

	// TODO load extended capabilities

	if (err) *err = 0;
	return term;
}

// Most ti_terminfo members point into the data member and so must not be freed
// directly. String returned from ti_getstr are invalid after ti_freeterm.
void ti_freeterm(ti_term *term) {
	free(term->name); term->name = NULL;
	free(term->data); term->data = NULL;
	free(term);
}

int ti_getflag(ti_term *t, int cap) {
	if (!t) return -1;
	if (cap < 0 || cap > t->info.num_bools) return -1;

	return t->info.bools[cap];
}

int ti_getnum(ti_term *t, int cap) {
	if (!t) return -1;
	if (cap < 0 || cap > t->info.num_nums) return -1;

	return t->info.nums[cap];
}

char *ti_getstr(ti_term *t, int cap) {
	if (!t) return NULL;
	if (cap < 0 || cap > t->info.num_strings) return NULL;

	int offset = t->info.str_offs[cap];
	if (offset < 0) return NULL;
	// TODO: check offset isn't larger than str_table

	return t->info.str_table + t->info.str_offs[cap];
}

/*
 * Parameterized string processing
 *
 * Yep, terminfo capability string processing requires interpreting an entire
 * semi-sophisticated stack language with arithmetic, logical, bit, and unary
 * operations, as well as variables and conditionals. fml.
 *
 * See terminfo(5) "Parameterized Strings" for the language description.
 *
 * The tinfo/lib_tparm.c source in ncurses includes some additional color along
 * with the ncurses implementation:
 *
 *    <https://github.com/mirror/ncurses/blob/master/ncurses/tinfo/lib_tparm.c>
 *
 * The implementation below is based on the golang tcell project's:
 *
 *    <https://github.com/gdamore/tcell/blob/master/terminfo/terminfo.go>
 *
 */

#define stk_str 1
#define stk_num 2

#define TI_PARM_STACK_MAX  32          // max stack size (number of elements)
#define TI_PARM_STRING_MAX 64          // max size of int converted to string
#define TI_PARM_OUTPUT_MAX 4096        // max output string size
#define TI_PARM_PARAMS_MAX 9           // max number of params

struct stk_el {
	int type;
	union {
		char *str;
		int   num;
	} val;
};

struct stk {
	int pos;
	struct stk_el el[TI_PARM_STACK_MAX];
};

// Push a string onto the stack.
// The string must be heap allocated and must also be freed by the caller after pop.
// Returns 0 if successful, -1 on stack overflow.
static int stk_push_str(struct stk *stk, char *str) {
	if (stk->pos >= TI_PARM_STACK_MAX)
		return -1;

	struct stk_el *el = &stk->el[stk->pos++];
	el->type = stk_str;
	el->val.str = str;

	return 0;
}

// Pop a string off the stack.
// Returns an empty string on stack underflow.
// The string returned must be freed by the caller.
static char *stk_pop_str(struct stk *stk) {
	if (stk->pos <= 0) return calloc(1, 1);

	struct stk_el *el = &stk->el[--stk->pos];
	if (el->type == stk_str) {
		return el->val.str;
	} else {
		char *buf = malloc(TI_PARM_STRING_MAX);
		snprintf(buf, TI_PARM_STRING_MAX, "%d", el->val.num);
		return buf;
	}
}

// Push a number onto the stack.
// Returns 0 when successful, -1 on stack overflow.
static int stk_push_num(struct stk *stk, int num) {
	if (stk->pos >= TI_PARM_STACK_MAX) return -1;

	struct stk_el *el = &stk->el[stk->pos++];
	el->type = stk_num;
	el->val.num  = num;

	return 0;
}

// Pop a number off the stack.
// Returns 0 on stack underflow.
static int stk_pop_num(struct stk *stk) {
	if (stk->pos <= 0) return 0;

	struct stk_el *el = &stk->el[--stk->pos];
	if (el->type == stk_num) {
		return el->val.num;
	} else {
		int num = atoi(el->val.str);
		free(el->val.str);
		return num;
	}
}

// Pop everything off stack and free strings.
// This doesn't free the stk pointer since it's usually stack allocated.
static void stk_free(struct stk *stk) {
	struct stk_el *el = NULL;
	while (stk->pos > 0) {
		el = &stk->el[--stk->pos];
		if (el->type == stk_str) {
			free(el->val.str);
		}
	}
}

// Static variables
//
// NOTE: Static variables are intended to live across multiple param string
// processing invocations. That is the case in this implementation but note
// that any strings set here are not freed and live until program termination.
//
// This also means ti_parm() processing is not concurrency-safe when static
// variables are used.
static char *svars[26];

int ti_parm(char *buf, const char *s, int c, ...) {
	if (!s) return 0;

	// load varg params into fixed size int array for easier referencing.
	va_list ap;
	va_start(ap, c);
	int params[9] = {0};
	for (int i = 0; i < 9 && i < c; i++) {
		params[i] = va_arg(ap, int);
	}
	va_end(ap);

	// variables used in instruction processing
	int ai = 0, bi = 0;              // unary, arithmetic, binary op vars
	char *as, *bs;                   // logical compare strs
	char *str;                       // string pointer var
	char sstr[TI_PARM_STRING_MAX];   // stack allocated string
	int i;                           // loop counter
	char fmt[16];                    // format code buffer
	int fpos;                        // format code bufer pos
	int nest, done;                  // if/then/else state vars

	// param string instruction stack
	struct stk stk = {0};

	// dynamic variables
	char *dvars[26] = {0};

	// output buffer pos and number of bytes written
	int pos = 0;
	int nwrite = 0;

	// pointer to current input char
	const char *pch = s;

	for (;*pch;) {
		if (*pch != '%') {
			buf[pos++] = *pch++;
			nwrite++;
			continue;
		}

		pch++; // skip over '%'

		switch (*pch++) {
		case '%':
			buf[pos++] = '%';
			nwrite++;
			break;
		case 'i':
			// increment both params
			params[0]++;
			params[1]++;
			break;
		case 'c': case 's':
			// pop char or string, write to output buffer
			// optimized version of formatted output operator below
			str = stk_pop_str(&stk);
			for (i = 0; str[i] && pos < TI_PARM_OUTPUT_MAX; i++) {
				buf[pos++] = str[i];
				nwrite++;
			}
			free(str);
			break;
		case 'd':
			// pop int, print
			// optimized version of formatted output operator below
			ai = stk_pop_num(&stk);
			snprintf(sstr, TI_PARM_STRING_MAX, "%d", ai);
			for (i = 0; sstr[i] && pos < TI_PARM_OUTPUT_MAX; i++) {
				buf[pos++] = sstr[i];
				nwrite++;
			}
			break;
		case 'p':
			// push parameter
			ai = *pch++ - '1';
			if (ai >= 0 && ai < TI_PARM_PARAMS_MAX) {
				stk_push_num(&stk, params[ai]);
			} else {
				stk_push_num(&stk, 0);
			}
			break;
		case 'P':
			// pop & store variable
			// free already set var before setting new
			if (*pch >= 'A' && *pch <= 'Z') {
				ai = *pch-'A';
				if (svars[ai]) {
					free(svars[ai]);
				}
				svars[ai] = stk_pop_str(&stk);
			} else if (*pch >= 'a' && *pch <= 'z') {
				ai = *pch-'a';
				if (dvars[ai]) {
					free(dvars[ai]);
				}
				dvars[ai] = stk_pop_str(&stk);
			}
			pch++;
			break;
		case 'g':
			// recall and push variable
			if (*pch >= 'A' && *pch <= 'Z') {
				if ((as = svars[*pch-'A'])) {
					// strdup since strings are freed on pop
					bs = malloc(strlen(as)+1);
					strcpy(bs, as);
					stk_push_str(&stk, bs);
				} else {
					stk_push_str(&stk, calloc(1, 1));
				}
			} else if (*pch >= 'a' && *pch <= 'z') {
				if ((as = dvars[*pch-'a'])) {
					// strdup since strings are freed on pop
					bs = malloc(strlen(as)+1);
					strcpy(bs, as);
					stk_push_str(&stk, bs);
				} else {
					stk_push_str(&stk, calloc(1, 1));
				}
			}
			pch++;
			break;
		case '\'':
			// push literal char
			str = calloc(1, 2);
			str[0] = *pch++;
			stk_push_str(&stk, str);
			pch++; // must be ' but we don't check
			break;
		case '{':
			// push int
			ai = 0;
			for (;*pch >= '0' && *pch <= '9';) {
				ai *= 10;
				ai += *pch - '0';
				pch++;
			}
			pch++; // must be } but we don't check
			stk_push_num(&stk, ai);
			break;
		case 'l':
			// pop str, push length
			str = stk_pop_str(&stk);
			stk_push_num(&stk, strlen(str));
			free(str);
			break;
		case '+':
			// pop int, pop int, add, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai+bi);
			break;
		case '-':
			// pop int, pop int, subtract, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai-bi);
			break;
		case '*':
			// pop int, pop int, multiply, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai*bi);
			break;
		case '/':
			// pop int, pop int, divide, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, bi ? ai/bi : 0);
			break;
		case 'm':
			// pop int, pop int, mod, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai%bi);
			break;
		case '&':
			// pop int, pop int, binary and, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai&bi);
			break;
		case '|':
			// pop int, pop int, binary or, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai|bi);
			break;
		case '^':
			// pop int, pop int, binary xor, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai^bi);
			break;
		case '~':
			// pop int, pop int, bit complement, push int
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ~ai);
			break;
		case 'A':
			// pop int, pop int, binary and, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai&&bi);
			break;
		case 'O':
			// pop int, pop int, binary or, push int
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai||bi);
			break;
		case '!':
			// pop int, pop int, logical not, push bool
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, !ai);
			break;
		case '=':
			// pop str, pop str, compare, push bool
			bs = stk_pop_str(&stk);
			as = stk_pop_str(&stk);
			stk_push_num(&stk, strcmp(bs, as)==0);
			free(bs); bs = NULL;
			free(as); as = NULL;
			break;
		case '>':
			// pop int, pop int, greater than, push bool
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai>bi);
			break;
		case '<':
			// pop int, pop int, greater than, push bool
			bi = stk_pop_num(&stk);
			ai = stk_pop_num(&stk);
			stk_push_num(&stk, ai<bi);
			break;

		case '0': case '1': case '2': case '3': case '4':
		case 'x': case 'X': case 'o': case ':': case ' ':
			// unoptimized (sprintf) version of formatted output
			// operator. word around the campfire is these are
			// very rarely used so the optimized versions above
			// are more likely to be run...

			// build a sprintf format string
			fpos = 0;
			fmt[fpos++] = '%';

			pch--;
			if (*pch == ':') pch++;

			while (*pch=='+'||*pch=='-'||*pch=='#'||*pch==' ') {
				char fch = *pch++;
				if ((size_t)fpos < sizeof(fmt)) {
					fmt[fpos++] = fch;
				}
			}

			while ((*pch>='0' && *pch<='9') || *pch=='.') {
				char fch = *pch++;
				if ((size_t)fpos < sizeof(fmt)) {
					fmt[fpos++] = fch;
				}
			}

			fmt[fpos++] = *pch++;
			fmt[fpos] = '\0';
			// printf("format string: %s\n", fmt);

			switch (*(pch-1)) {
			case 'd': case 'x': case 'X': case 'o':
				ai = stk_pop_num(&stk);
				snprintf(sstr, TI_PARM_STRING_MAX, fmt, ai);
				ai = TI_PARM_OUTPUT_MAX;
				for (i = 0; sstr[i] && pos < ai; i++) {
					buf[pos++] = sstr[i];
					nwrite++;
				}
				break;
			case 'c': case 's':
				str = stk_pop_str(&stk);
				snprintf(sstr, TI_PARM_STRING_MAX, fmt, str);
				ai = TI_PARM_OUTPUT_MAX;
				for (i = 0; sstr[i] && pos < ai; i++) {
					buf[pos++] = sstr[i];
					nwrite++;
				}
				free(str);
				break;
			}
			break;

		case '?':
			// if: start conditional
			break;
		case 't':
			// then: evaluate conditional result
			if (stk_pop_num(&stk)) {
				break;
			}

			// this loop consumes everything until we hit our else,
			// or the end of the conditional
			nest = 0;
			done = 0;
			while (*pch && !done) {
				if (*pch != '%') {
					pch++;
					continue;
				}

				pch++; // skip over '%'
				switch (*pch++) {
				case ';':
					done = !nest;
					nest--;
					break;
				case '?':
					nest++;
					break;
				case 'e':
					done = !nest;
					break;
				}
			};
			break;
		case 'e':
			// if we got here, it means we didn't use the else
			// in the 't' case above, and we should skip until
			// the end of the conditional
			nest = 0;
			done = 0;
			while (*pch && !done) {
				if (*pch != '%') {
					pch++;
					continue;
				}
				pch++;
				switch (*pch++) {
				case ';':
					done = !nest;
					nest--;
					break;
				case '?':
					nest++;
					break;
				}
			}
			break;
		case ';':
			// endif
			break;
		default:
			// eat invalid instructions
			break;
		}
	}

	// free anything left on the stack
	stk_free(&stk);

	// free dynamic variables
	for (i = 0; i < 26; i++) {
		free(dvars[i]);
	}

	buf[pos] = '\0';
	return nwrite;
}

// vim: noexpandtab
