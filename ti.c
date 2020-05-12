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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <assert.h>

/*
 * Capability name arrays are used to find the index of legacy format
 * capabilities within the terminfo file.
 *
 */

// Boolean capability names
const char * const ti_boolnames[] = {
	"bw", "am", "xsb", "xhp", "xenl", "eo", "gn", "hc", "km", "hs",
	"in", "da", "db", "mir", "msgr", "os", "eslok", "xt", "hz", "ul",
	"xon", "nxon", "mc5i", "chts", "nrrmc", "npc", "ndscr", "ccc",
	"bce", "hls", "xhpa", "crxm", "daisy", "xvpa", "sam", "cpix",
	"lpix", "OTbs", "OTns", "OTnc", "OTMT", "OTNL", "OTpt", "OTxr",
};

// Numeric capability names
const char * const ti_numnames[] = {
	"cols", "it", "lines", "lm", "xmc", "pb", "vt", "wsl", "nlab",
	"lh", "lw", "ma", "wnum", "colors", "pairs", "ncv", "bufsz",
	"spinv", "spinh", "maddr", "mjump", "mcs", "mls", "npins", "orc",
	"orl", "orhi", "orvi", "cps", "widcs", "btns", "bitwin", "bitype",
	"OTug", "OTdC", "OTdN", "OTdB", "OTdT", "OTkn",
};

// String capability names
const char * const ti_strnames[] = {
	"cbt", "bel", "cr", "csr", "tbc", "clear", "el", "ed", "hpa",
	"cmdch", "cup", "cud1", "home", "civis", "cub1", "mrcup",
	"cnorm", "cuf1", "ll", "cuu1", "cvvis", "dch1", "dl1", "dsl",
	"hd", "smacs", "blink", "bold", "smcup", "smdc", "dim", "smir",
	"invis", "prot", "rev", "smso", "smul", "ech", "rmacs", "sgr0",
	"rmcup", "rmdc", "rmir", "rmso", "rmul", "flash", "ff", "fsl",
	"is1", "is2", "is3", "if", "ich1", "il1", "ip", "kbs", "ktbc",
	"kclr", "kctab", "kdch1", "kdl1", "kcud1", "krmir", "kel",
	"ked", "kf0", "kf1", "kf10", "kf2", "kf3", "kf4", "kf5", "kf6",
	"kf7", "kf8", "kf9", "khome", "kich1", "kil1", "kcub1", "kll",
	"knp", "kpp", "kcuf1", "kind", "kri", "khts", "kcuu1", "rmkx",
	"smkx", "lf0", "lf1", "lf10", "lf2", "lf3", "lf4", "lf5", "lf6",
	"lf7", "lf8", "lf9", "rmm", "smm", "nel", "pad", "dch", "dl",
	"cud", "ich", "indn", "il", "cub", "cuf", "rin", "cuu", "pfkey",
	"pfloc", "pfx", "mc0", "mc4", "mc5", "rep", "rs1", "rs2", "rs3",
	"rf", "rc", "vpa", "sc", "ind", "ri", "sgr", "hts", "wind", "ht",
	"tsl", "uc", "hu", "iprog", "ka1", "ka3", "kb2", "kc1", "kc3",
	"mc5p", "rmp", "acsc", "pln", "kcbt", "smxon", "rmxon", "smam",
	"rmam", "xonc", "xoffc", "enacs", "smln", "rmln", "kbeg", "kcan",
	"kclo", "kcmd", "kcpy", "kcrt", "kend", "kent", "kext", "kfnd",
	"khlp", "kmrk", "kmsg", "kmov", "knxt", "kopn", "kopt", "kprv",
	"kprt", "krdo", "kref", "krfr", "krpl", "krst", "kres", "ksav",
	"kspd", "kund", "kBEG", "kCAN", "kCMD", "kCPY", "kCRT", "kDC",
	"kDL", "kslt", "kEND", "kEOL", "kEXT", "kFND", "kHLP", "kHOM",
	"kIC", "kLFT", "kMSG", "kMOV", "kNXT", "kOPT", "kPRV", "kPRT",
	"kRDO", "kRPL", "kRIT", "kRES", "kSAV", "kSPD", "kUND", "rfi",
	"kf11", "kf12", "kf13", "kf14", "kf15", "kf16", "kf17", "kf18",
	"kf19", "kf20", "kf21", "kf22", "kf23", "kf24", "kf25", "kf26",
	"kf27", "kf28", "kf29", "kf30", "kf31", "kf32", "kf33", "kf34",
	"kf35", "kf36", "kf37", "kf38", "kf39", "kf40", "kf41", "kf42",
	"kf43", "kf44", "kf45", "kf46", "kf47", "kf48", "kf49", "kf50",
	"kf51", "kf52", "kf53", "kf54", "kf55", "kf56", "kf57", "kf58",
	"kf59", "kf60", "kf61", "kf62", "kf63", "el1", "mgc", "smgl",
	"smgr", "fln", "sclk", "dclk", "rmclk", "cwin", "wingo", "hup",
	"dial", "qdial", "tone", "pulse", "hook", "pause", "wait", "u0",
	"u1", "u2", "u3", "u4", "u5", "u6", "u7", "u8", "u9", "op",
	"oc", "initc", "initp", "scp", "setf", "setb", "cpi", "lpi",
	"chr", "cvr", "defc", "swidm", "sdrfq", "sitm", "slm", "smicm",
	"snlq", "snrmq", "sshm", "ssubm", "ssupm", "sum", "rwidm", "ritm",
	"rlm", "rmicm", "rshm", "rsubm", "rsupm", "rum", "mhpa", "mcud1",
	"mcub1", "mcuf1", "mvpa", "mcuu1", "porder", "mcud", "mcub",
	"mcuf", "mcuu", "scs", "smgb", "smgbp", "smglp", "smgrp", "smgt",
	"smgtp", "sbim", "scsd", "rbim", "rcsd", "subcs", "supcs", "docr",
	"zerom", "csnm", "kmous", "minfo", "reqmp", "getm", "setaf",
	"setab", "pfxl", "devt", "csin", "s0ds", "s1ds", "s2ds", "s3ds",
	"smglr", "smgtb", "birep", "binel", "bicr", "colornm", "defbi",
	"endbi", "setcolor", "slines", "dispc", "smpch", "rmpch", "smsc",
	"rmsc", "pctrm", "scesc", "scesa", "ehhlm", "elhlm", "elohlm",
	"erhlm", "ethlm", "evhlm", "sgr1", "slength", "OTi2", "OTrs",
	"OTnl", "OTbc", "OTko", "OTma", "OTG2", "OTG3", "OTG1", "OTG4",
	"OTGR", "OTGL", "OTGU", "OTGD", "OTGH", "OTGV", "OTGC", "meml",
	"memu", "box1",
};

#define TI_FN_MAX   1024   // 1K max filename length
#define TI_DATA_MAX 16384  // 16K max terminfo file size

// Raw file information
struct ti_file {
	char *data;              // contents of file
	int  len;                // number of bytes loaded in data
};

// Read an entire file into the ti_file struct.
// Caller must free the string set in ti_file.data.
static int ti_read_file(struct ti_file *f, const char *fn) {
	FILE *fd = fopen(fn, "rb");
	if (!fd) {
		return errno;
	}

	struct stat st;
	if (fstat(fileno(fd), &st) != 0) {
		int err = errno;
		fclose(fd);
		return err;
	}
	f->len = st.st_size;

	if (f->len > TI_DATA_MAX) {
		fclose(fd);
		return EFBIG;
	}

	f->data = malloc(f->len);
	if (!f->data) {
		fclose(fd);
		return ENOMEM;
	}

	if (fread(f->data, 1, f->len, fd) != (size_t)f->len) {
		free(f->data);
		fclose(fd);
		return EIO;
	}

	fclose(fd);

	return 0;
}

// Look for the terminfo file for the given term under the given path.
static int ti_try_path(struct ti_file *f, const char *path, const char *term) {
	char fn[TI_FN_MAX] = {0};
	snprintf(fn, sizeof(fn), "%s/%c/%s", path, term[0], term);

	// try reading in normal dir structure
	int rc = ti_read_file(f, fn);
	if (rc == 0) return 0;

	// fallback to case-insensitive filesystem path structure
	snprintf(fn, sizeof(fn), "%s/%x/%s", path, term[0], term);
	return ti_read_file(f, fn);
}

// Find the terminfo file and load its contents into the ti_file struct.
static int ti_load_data(struct ti_file *f, const char *term) {
	int rc = 0;

	// if TERMINFO is set, no other directory should be searched
	if (getenv("TERMINFO") && getenv("TERMINFO")[0] != '\0') {
		return ti_try_path(f, getenv("TERMINFO"), term);
	}

	// next, consider ~/.terminfo
	if (getenv("HOME")) {
		char fn[TI_FN_MAX] = {0};
		snprintf(fn, sizeof(fn), "%s/.terminfo", getenv("HOME"));
		rc = ti_try_path(f, fn, term);
		if (rc == 0) return rc;
	}

	// next, TERMINFO_DIRS
	const char *dirs = getenv("TERMINFO_DIRS");
	if (dirs) {
		char buf[TI_FN_MAX] = {0};
		snprintf(buf, sizeof(buf), "%s", dirs);
		char *dir = strtok(buf, ":");
		while (dir) {
			const char *cdir = dir;
			if (strcmp(cdir, "") == 0) {
				cdir = "/usr/share/terminfo";
			}
			rc = ti_try_path(f, cdir, term);
			if (rc == 0) return rc;
			dir = strtok(0, ":");
		}
	}

	// search in system paths
	char *paths[] = {
		"/etc/terminfo",
		"/lib/terminfo",
		"/usr/share/terminfo",
		"/usr/local/share/terminfo",
		NULL
	};
	for (int i = 0; paths[i]; i++) {
		rc = ti_try_path(f, paths[i], term);
		if (rc == 0) return rc;
	}

	return ENOENT; // file not found
}

// Terminfo magic number byte values.
// TI_MAGIC_32BIT indicates that numeric capabilities are stored as 32-bit signed
// integers instead of 16-bit signed integers.
// See term(5) Extended Number Format section for more information.
#define TI_MAGIC       0432
#define TI_MAGIC_32BIT 01036

// TODO: big endian arch. terminfo files are always structured little endian.
ti_terminfo *ti_load(const char *termname, int *err) {
	if (!termname) termname = getenv("TERM");
	if (!termname) {
		if (err) *err = TI_ERR_TERM_NOT_SET;
		return NULL;
	}

	struct ti_file f = {0};
	int rc = ti_load_data(&f, termname);
	if (rc) {
		if (err) *err = rc;
		return NULL;
	}

	// if data size is less than fixed header we got problems
	// exit now before allocating a bunch of other stuff
	if (f.len < (int)(6 * sizeof(int16_t))) {
		if (err) *err = TI_ERR_NO_HEADER;
		return NULL;
	}

	// alloc and initialize term struct
	ti_terminfo *ti = calloc(1, sizeof(ti_terminfo));
	ti->data = f.data;
	ti->len = f.len;

	// pointer to current position in data
	char *p = ti->data;

	// copy header data into struct
	struct {
		int16_t magic;         // magic number (octal 0432)
		int16_t names_len;     // size in bytes of the names section
		int16_t bools_len;     // size in bytes of the bools section
		int16_t nums_count;    // count of ints in nums section
		int16_t stroffs_count; // count of shorts in stroffs section
		int16_t strtbl_len;    // size in bytes of the string table
	} h;
	memcpy(&h, p, sizeof(h));
	p += sizeof(h);

	// verify magic number checks out
	if (h.magic != TI_MAGIC && h.magic != TI_MAGIC_32BIT) {
		if (err) *err = TI_ERR_BAD_MAGIC;
		ti_free(ti);
		return NULL;
	}

	// point term_names at data section
	ti->term_names = p;
	p += h.names_len;

	// point bools array at bools data section
	ti->bools = (int8_t*)p;
	ti->bools_count = h.bools_len;
	p += h.bools_len + ((h.names_len + h.bools_len) % 2);

	// size in bytes of numeric capabilities stored in data
	const int numsz = (h.magic == TI_MAGIC_32BIT)
		? sizeof(int32_t) : sizeof(int16_t);

	// copy numeric capabilities into newly allocated array, converting from
	// 16-bit to 32-bit values if needed
	ti->nums = malloc(h.nums_count * sizeof(int32_t));
	if (numsz == sizeof(int32_t)) {
		memcpy(ti->nums, p, h.nums_count * sizeof(int32_t));
	} else {
		for (int i = 0; i < h.nums_count; i++)
			ti->nums[i] = ((int16_t*)p)[i];
	}
	ti->nums_count = h.nums_count;
	p += (h.nums_count * numsz);

	// convert string capability offsets into pointers to strtbl
	ti->strs = calloc(h.stroffs_count, sizeof(char*));
	ti->strs_count = h.stroffs_count;
	int16_t *stroffs = (int16_t*)p;
	p += (h.stroffs_count * sizeof(int16_t));
	for (int i = 0; i < h.stroffs_count; i++) {
		if (stroffs[i] < 0) continue;
		if (stroffs[i] >= h.strtbl_len) {
			if (err) *err = TI_ERR_BAD_STROFF;
			ti_free(ti);
			return NULL;
		}
		ti->strs[i] = p + stroffs[i];
	}
	p += h.strtbl_len;

	// make sure all of the above pointers point within the loaded data;
	// if not the terminfo file is corrupt
	int data_len = p - ti->data;
	if (data_len > f.len) {
		if (err) *err = TI_ERR_BAD_STRTBL;
		ti_free(ti);
		return NULL;
	} else if (data_len == f.len) {
		// no extended caps after legacy caps, return now
		if (err) *err = 0;
		return ti;
	}
	p += (data_len % 2); // alignment byte

	// extended format header comes after legacy format data in file
	struct {
		int16_t  bools_count;    // count of extended bool caps
		int16_t  nums_count;     // count of extended numeric caps
		int16_t  stroffs_count;  // count of extended string caps
		int16_t  strtbl_num;     // count strs in strtbl including names
		int16_t  strtbl_len;     // total size of strtbl
	} h2;
	memcpy(&h2, p, sizeof(h2));
	p += sizeof(h2);

	ti->ext_bools_count = h2.bools_count;
	ti->ext_nums_count = h2.nums_count;
	ti->ext_strs_count = h2.stroffs_count;
	ti->ext_names_count = h2.bools_count + h2.nums_count + h2.stroffs_count;

	// set up pointer members and counts to reference locations is data
	ti->ext_bools = (int8_t*)p;
	p += h2.bools_count + (h2.bools_count % 2); // alignment

	// copy extended numeric caps into newly allocated array, converting from
	// 16-bit to 32-bit values if needed
	ti->ext_nums = malloc(h2.nums_count * sizeof(int32_t));
	if (numsz == sizeof(int32_t)) {
		memcpy(ti->ext_nums, p, h2.nums_count * sizeof(int32_t));
	} else {
		for (int i = 0; i < h2.nums_count; i++)
			ti->ext_nums[i] = ((int16_t*)p)[i];
	}
	p += (h2.nums_count * numsz);

	// convert string capability offsets into pointers to strtbl
	stroffs = (int16_t*)p;
	p += ((h2.stroffs_count + ti->ext_names_count) * sizeof(int16_t));
	char *strtbl = p;
	ti->ext_strs = calloc(h2.stroffs_count, sizeof(char*));
	for (int i = 0; i < h2.stroffs_count; i++) {
		if (stroffs[i] < 0) continue;  // extended strings can be null
		if (stroffs[i] >= h2.strtbl_len) {
			if (err) *err = TI_ERR_BAD_STROFF;
			ti_free(ti);
			return NULL;
		}
		ti->ext_strs[i] = strtbl + stroffs[i];
		p = ti->ext_strs[i] + strlen(ti->ext_strs[i]) + 1;
	}


	// convert name offsets into pointers to strtbl
	int16_t *nameoffs = stroffs + h2.stroffs_count;
	int nametbl_len = h2.strtbl_len - (p - strtbl);

	// calculate pointers from offsets
	ti->ext_names = calloc(ti->ext_names_count, sizeof(char*));
	for (int i = 0; i < ti->ext_names_count; i++) {
		if (nameoffs[i] < 0 || nameoffs[i] >= nametbl_len) {
			if (err) *err = TI_ERR_BAD_STROFF;
			ti_free(ti);
			return NULL;
		}
		ti->ext_names[i] = p + nameoffs[i];
	}

	// set up bool, num, and str name array pointers to their positions in
	// the overall name pointers array
	ti->ext_bool_names = ti->ext_names;
	ti->ext_num_names = ti->ext_bool_names + ti->ext_bools_count;
	ti->ext_str_names = ti->ext_num_names + ti->ext_nums_count;

	if (err) *err = 0;
	return ti;
}

// Most ti_terminfo members point into the data member and so must not be freed
// directly. String returned from ti_getstr are invalid after ti_freeterm.
void ti_free(ti_terminfo *ti) {
	if (!ti) return;
	free(ti->nums);      ti->nums = NULL;
	free(ti->strs);      ti->strs = NULL;
	free(ti->ext_nums);  ti->ext_nums = NULL;
	free(ti->ext_strs);  ti->ext_strs = NULL;
	free(ti->ext_names); ti->ext_names = NULL;
	free(ti->data);      ti->data = NULL;
	free(ti);
}


/*
 * Error reporting
 *
 */

static const char * const ti_errors[] = {
	"term name not given and TERM not set",
	"missing terminfo header",
	"file is not a terminfo file",
	"illegal string offset in terminfo file",
	"terminfo string table length exceeds file size",
};

const char *ti_strerror(int errnum) {
	if (errnum > 0) {
		return strerror(errnum);
	} else if (errnum < 0) {
		int index = abs(errnum) - 1;
		if (index < (int)(sizeof(ti_errors) / sizeof(char*)))
			return ti_errors[index];
	}
	return NULL;
}


/*
 * Terminal capability access functions
 *
 */

int ti_getbooli(ti_terminfo *ti, int cap) {
	assert(ti);
	if (cap < 0 || cap > ti->bools_count) {
		return 0;
	}
	return ti->bools[cap];
}

int ti_getnumi(ti_terminfo *ti, int cap) {
	assert(ti);
	if (cap < 0 || cap > ti->nums_count) {
		return -1;
	}
	return ti->nums[cap];
}

char *ti_getstri(ti_terminfo *ti, int cap) {
	assert(ti);
	if (cap < 0 || cap >= ti->strs_count) {
		return NULL;
	}
	return ti->strs[cap];
}

int ti_getbool(ti_terminfo *ti, const char *cap) {
	assert(ti);
	if (cap == NULL) return 0;

	// O(n) linear search through bool capability names.
	// TIP: load these at startup instead of on-demand.
	static const int n = (sizeof(ti_boolnames) / sizeof(char*));
	for (int i = 0; i < n; i++) {
		if (strcmp(ti_boolnames[i], cap)) continue;
		return ti_getbooli(ti, i);
	}

	// check extended boolean capabilties
	for (int i = 0; i < ti->ext_bools_count; i++) {
		if (strcmp(ti->ext_bool_names[i], cap)) continue;
		return ti->ext_bools[i];
	}

	return 0;

}

int ti_getnum(ti_terminfo *ti, const char *cap) {
	assert(ti);
	if (cap == NULL) return -1;

	// O(n) linear search through numeric capability names.
	// TIP: load these at startup instead of on-demand.
	static const int n = (sizeof(ti_numnames) / sizeof(char*));
	for (int i = 0; i < n; i++) {
		if (strcmp(ti_numnames[i], cap)) continue;
		return ti_getnumi(ti, i);
	}

	// check extended numeric capabilties.
	for (int i = 0; i < ti->ext_nums_count; i++) {
		if (strcmp(ti->ext_num_names[i], cap)) continue;
		return ti->ext_nums[i];
	}

	return -1;
}

char *ti_getstr(ti_terminfo *ti, const char *cap) {
	assert(ti);
	if (cap == NULL) return NULL;

	// O(n) linear search through string capability names.
	// TIP: load these at startup instead of on-demand.
	static const int n = (sizeof(ti_strnames) / sizeof(char*));
	for (int i = 0; i < n; i++) {
		if (strcmp(ti_strnames[i], cap)) continue;
		return ti_getstri(ti, i);
	}

	// check extended string capabilties.
	for (int i = 0; i < ti->ext_strs_count; i++) {
		if (strcmp(ti->ext_str_names[i], cap)) continue;
		return ti->ext_strs[i];
	}

	return NULL;
}


/*
 * Utility functions
 *
 */
int ti_stresc(char *buf, const char *str, int n) {
	assert(buf);
	assert(str);

	char *pbuf = buf;
	for (const char *pch = str; *pch; pch++) {
		if (n - (pbuf - buf) < 5) {
			// eager bounds check
			break;
		}

		if (*pch >= ' ' && *pch <= '~') {
			*pbuf++ = *pch;
			continue;
		}

		*pbuf++ = '\\';
		switch (*pch) {
		case '\x1b': // ESC
			*pbuf++ = 'e';
			break;
		case '\t':
			*pbuf++ = 't';
			break;
		case '\n':
			*pbuf++ = 'n';
			break;
		case '\r':
			*pbuf++ = 'r';
			break;
		default:
			*pbuf++ = 'x';
			pbuf+= sprintf(pbuf, "%02x", *pch);
		}
	}
	*pbuf++ = 0;

	return pbuf - buf - 1;
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
