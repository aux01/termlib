#define _POSIX_C_SOURCE 200112L

#include "tbti.h"

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

static tb_terminal *tb_term;

int tb_setupterm(char *term, int fd) {
	if (!term) term = getenv("TERM");
	if (!term) return -1;

	if (tb_term && strcmp(term, tb_term->termname) == 0) {
		return 0;
	} else if (tb_term) {
		// TODO: free tb_term struct and members
	}

	char *data = terminfo_load_data(term);
	if (!data) {
		return -2;
	}

	tb_term = malloc(sizeof(tb_terminal));
	memset(tb_term, 0, sizeof(tb_terminal));

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
		strtbl_len   = header[5]; // size in bytes of the string table

	assert(magic == TI_MAGIC);
	// TODO bail if magic is wrong

	tb_termtype *type = &tb_term->type;
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

int tb_getflag(int cap) {
	if (!tb_term) return -1;
	if (cap < 0 || cap > tb_term->type.num_bools) return -1;

	return tb_term->type.bools[cap];
}

int tb_getnum(int cap) {
	if (!tb_term) return -1;
	if (cap < 0 || cap > tb_term->type.num_nums) return -1;

	return tb_term->type.nums[cap];
}

char *tb_getstr(int cap) {
	if (!tb_term) return NULL;
	if (cap < 0 || cap > tb_term->type.num_strings) return NULL;

	int offset = tb_term->type.str_offs[cap];
	if (offset < 0) return NULL;
	// TODO: check offset isn't larger than str_table

	return tb_term->type.str_table + tb_term->type.str_offs[cap];
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

typedef struct stk_el {
	int type;
	union {
		char *str;
		int   num;
	} val;
} stk_el;

#define stk_str 1
#define stk_num 2

#define TB_PARM_STACK_MAX  32          // max stack size (number of elements)
#define TB_PARM_STRING_MAX 64          // max size of int converted to string
#define TB_PARM_OUTPUT_MAX 4096        // max output string size
#define TB_PARM_PARAMS_MAX 9           // max number of params

static stk_el stk[TB_PARM_STACK_MAX];
static int stk_pos = 0;

// Push a string onto the stack.
// The string must be heap allocated and must also be freed by the caller after pop.
// Returns 0 if successful, -1 on stack overflow.
static int stk_push_str(char *str) {
	if (stk_pos >= TB_PARM_STACK_MAX)
		return -1;

	stk_el *el = &stk[stk_pos++];
	el->type = stk_str;
	el->val.str = str;

	return 0;
}

// Pop a string off the stack.
// Returns an empty string on stack underflow.
// The string returned must be freed by the caller.
static char *stk_pop_str(void) {
	if (stk_pos <= 0) return calloc(1, 1);

	stk_el *el = &stk[stk_pos--];
	if (el->type == stk_str) {
		return el->val.str;
	} else {
		char *buf = malloc(TB_PARM_STRING_MAX);
		snprintf(buf, TB_PARM_STRING_MAX, "%d", el->val.num);
		return buf;
	}
}

// Push a number onto the stack.
// Returns 0 when successful, -1 on stack overflow.
static int stk_push_num(int num) {
	if (stk_pos >= TB_PARM_STACK_MAX) return -1;

	stk_el *el = &stk[stk_pos++];
	el->type = stk_num;
	el->val.num  = num;

	return 0;
}

// Pop a number off the stack.
// Returns 0 on stack underflow.
static int stk_pop_num() {
	if (stk_pos <= 0) return 0;

	stk_el *el = &stk[--stk_pos];
	if (el->type == stk_num) {
		return el->val.num;
	} else {
		return atoi(el->val.str);
	}
}

static char *svars[26]; // static variables

char *tb_parmn(char *s, int c, ...) {
	if (!s) return NULL;

	// Load varg params into fixed size int array for easier referencing.
	va_list ap;
	va_start(ap, c);
	int params[9] = {0};
	for (int i = 0; i < 9 && i < c; i++) {
		params[i] = va_arg(ap, int);
	}
	va_end(ap);

	// Variables used in instruction processing
	int ai = 0, bi = 0;              // unary, arithmetic, binary op vars
	char *str;                       // string pointer var
	char sstr[TB_PARM_STRING_MAX];   // stack allocated string
	int i;                           // loop counter

	// Dynamic variables
	char *dvars[26] = {0};

	// output buffer and pos
	int pos = 0;
	char *buf = (char*)calloc(1, TB_PARM_OUTPUT_MAX);

	// pointer to current input char
	char *pch = s;

	for (;*pch;) {
		if (*pch != '%') {
			buf[pos++] = *pch++;
			continue;
		}
		pch++; // skip over '%'

		switch (*pch++) {
		case '%':
			buf[pos++] = *pch;
			break;
		case 'i':
			// increment both params
			params[0]++;
			params[1]++;
			break;
		case 'c': case 's':
			// pop char or string, write to output buffer
			str = stk_pop_str();
			for (i = 0; str[i] && pos < TB_PARM_OUTPUT_MAX; i++) {
				buf[pos++] = str[i];
			}
			free(str);
			break;
		case 'd':
			// pop int, print
			ai = stk_pop_num();
			snprintf(sstr, TB_PARM_STRING_MAX, "%d", ai);
			for (i = 0; sstr[i] && pos < TB_PARM_OUTPUT_MAX; i++) {
				buf[pos++] = sstr[i];
			}
			break;
		case '0': case '1': case '2': case '3': case '4':
		case 'x': case 'X': case 'o': case ':':
			// TODO: we lost the specifier so need to split these
			// cases or something..
			// TODO: pop int or string and print formatted
			break;
		case 'p':
			// push parameter
			ai = *pch++ - '1';
			if (ai >= 0 && ai < TB_PARM_PARAMS_MAX) {
				stk_push_num(params[ai]);
			} else {
				stk_push_num(0);
			}
			break;
		case 'P':
			// pop & store variable
			if (*pch >= 'A' && *pch <= 'Z') {
				svars[*pch-'A'] = stk_pop_str();
			} else if (*pch >= 'a' && *pch <= 'z') {
				dvars[*pch-'a'] = stk_pop_str();
			}
			pch++;
			break;
		case 'g':
			// recall and push variable
			if (*pch >= 'A' && *pch <= 'Z') {
				stk_push_str(svars[*pch-'A']);
				// NOTE: string is on stack AND in svars now;
				//       be careful with free()
			} else if (*pch >= 'a' && *pch <= 'z') {
				stk_push_str(dvars[*pch-'a']);
				// NOTE: string is on stack AND in dvars now;
				//       be careful with free()
			}
			pch++;
			break;
		case '\'':
			// push literal char
			str = calloc(1, 2);
			str[0] = *pch++;
			stk_push_str(str);
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
			stk_push_num(ai);
			break;
		case 'l':
			str = stk_pop_str();
			stk_push_num(strlen(str));
			free(str);
			break;
		case '+':
			bi = stk_pop_num();
			ai = stk_pop_num();
			stk_push_num(ai+bi);
			break;
		case '-':
			bi = stk_pop_num();
			ai = stk_pop_num();
			stk_push_num(ai-bi);
			break;
		case '*':
			bi = stk_pop_num();
			ai = stk_pop_num();
			stk_push_num(ai*bi);
			break;
		case '/':
			bi = stk_pop_num();
			ai = stk_pop_num();
			stk_push_num(bi ? ai/bi : 0);
			break;
		default:
			// TODO: ???
			fprintf(stderr, "unhandled char: 0x%0x\n", *(pch-1));
		}
	}

	// TODO: free strings left on the stack
	// TODO: free dynamic variables

	return buf;
}

#ifdef TESTNOW
int main(void) {
	int rc = tb_setupterm(NULL, 1);
	tb_setupterm(NULL, 1);

	printf("term_names: %s\n", tb_term->type.term_names);
	printf("str 27 = %s\n", tb_term->type.str_table + tb_term->type.str_offs[27]);

	printf("tb_getflag(tb_back_color_erase) = %d\n", tb_getflag(tb_back_color_erase));
	printf("tb_getflag(tb_auto_right_margin) = %d\n", tb_getflag(tb_auto_right_margin));
	printf("tb_getflag(tb_has_status_line) = %d\n", tb_getflag(tb_has_status_line));

	printf("tb_getnum(tb_columns) = %d\n", tb_getnum(tb_columns));
	printf("tb_getnum(tb_init_tabs) = %d\n", tb_getnum(tb_init_tabs));
	printf("tb_getnum(tb_buttons) = %d\n", tb_getnum(tb_buttons));

	printf("tb_getstr(tb_bell) = %s\n", tb_getstr(tb_bell));
	printf("tb_getstr(tb_set_a_background) = %s\n", tb_getstr(tb_set_a_background));

	printf("tb_parmn(\"[%%p1%%d@\",1, 42) = %s\n", tb_parmn("[%p1%d@", 1, 42));
	printf("tb_parmn(\"[%%i%%p1%%dd\", 1, 42) = %s\n", tb_parmn("[%i%p1%dd", 1, 42));
	return rc;
}
#endif

// vim: noexpandtab
