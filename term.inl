/* term.inl */

// SGR sequence construction
#define SGR_OPEN            "\x1b["
#define SGR_CLOSE           "m"
#define SGR(codes)          SGR_OPEN codes SGR_CLOSE

// Turn on typographic feature
#define SGR_TYPO_RESET      "0"
#define SGR_TYPO_BOLD       "1"
#define SGR_TYPO_FAINT      "2"
#define SGR_TYPO_ITALIC     "3"
#define SGR_TYPO_UNDERLINE  "4"
#define SGR_TYPO_BLINK      "5"
#define SGR_TYPO_REVERSE    "7"
#define SGR_TYPO_CONCEAL    "8"
#define SGR_TYPO_CROSSOUT   "9"
#define SGR_TYPO_ON(n)      n

// Turn off typographic feature.
// Ex: SGR_TYPO_OFF(SGR_TYPO_ITALIC) = "23"
#define SGR_TYPO_OFF_BASE   "2"
#define SGR_TYPO_OFF(n)     SGR_TYPO_OFF_BASE n

// Color suffixes
#define SGR_COLOR_BLACK     "0"
#define SGR_COLOR_RED       "1"
#define SGR_COLOR_GREEN     "2"
#define SGR_COLOR_YELLOW    "3"
#define SGR_COLOR_BLUE      "4"
#define SGR_COLOR_MAGENTA   "5"
#define SGR_COLOR_CYAN      "6"
#define SGR_COLOR_WHITE     "7"
#define SGR_COLOR_DEFAULT   "9"

// Color bases
#define SGR_NORMAL_FG_BASE  "3"
#define SGR_NORMAL_BG_BASE  "4"
#define SGR_BRIGHT_FG_BASE  "9"
#define SGR_BRIGHT_BG_BASE  "10"

// Color macros
#define SGR_NORMAL_FG(col)  SGR_NORMAL_FG_BASE col
#define SGR_NORMAL_BG(col)  SGR_NORMAL_BG_BASE col
#define SGR_BRIGHT_FG(col)  SGR_BRIGHT_FG_BASE col
#define SGR_BRIGHT_BG(col)  SGR_BRIGHT_BG_BASE col

enum {
	T_ENTER_CA,
	T_EXIT_CA,
	T_SHOW_CURSOR,
	T_HIDE_CURSOR,
	T_CLEAR_SCREEN,
	T_SGR0,
	T_UNDERLINE,
	T_BOLD,
	T_FAINT,
	T_ITALIC,
	T_BLINK,
	T_REVERSE,
	T_ENTER_KEYPAD,
	T_EXIT_KEYPAD,

	T_CROSSOUT,
	T_ENTER_MOUSE,
	T_EXIT_MOUSE,

	T_FUNCS_NUM,
};

#define CROSSOUT "\x1b[9m"
#define ENTER_MOUSE_SEQ "\x1b[?1000h\x1b[?1002h\x1b[?1015h\x1b[?1006h"
#define EXIT_MOUSE_SEQ "\x1b[?1006l\x1b[?1015l\x1b[?1002l\x1b[?1000l"

#define EUNSUPPORTED_TERM -1

// rxvt-256color
static const char *rxvt_256color_keys[] = {
	"\033[11~",                 // TB_KEY_F1
	"\033[12~",                 // TB_KEY_F2
	"\033[13~",                 // TB_KEY_F3
	"\033[14~",                 // TB_KEY_F4
	"\033[15~",                 // TB_KEY_F5
	"\033[17~",                 // TB_KEY_F6
	"\033[18~",                 // TB_KEY_F7
	"\033[19~",                 // TB_KEY_F8
	"\033[20~",                 // TB_KEY_F9
	"\033[21~",                 // TB_KEY_F10
	"\033[23~",                 // TB_KEY_F11
	"\033[24~",                 // TB_KEY_F12
	"\033[2~",                  // TB_KEY_INSERT
	"\033[3~",                  // TB_KEY_DELETE
	"\033[7~",                  // TB_KEY_HOME
	"\033[8~",                  // TB_KEY_END
	"\033[5~",                  // TB_KEY_PGUP
	"\033[6~",                  // TB_KEY_PGDN
	"\033[A",                   // TB_KEY_ARROW_UP
	"\033[B",                   // TB_KEY_ARROW_DOWN
	"\033[D",                   // TB_KEY_ARROW_LEFT
	"\033[C",                   // TB_KEY_ARROW_RIGHT
	0
};
static const char *rxvt_256color_funcs[] = {
	"\0337\033[?47h",           // T_ENTER_CA
	"\033[2J\033[?47l\0338",    // T_EXIT_CA
	"\033[?25h",                // T_SHOW_CURSOR
	"\033[?25l",                // T_HIDE_CURSOR
	"\033[H\033[2J",            // T_CLEAR_SCREEN
	"\033[m",                   // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"\033=",                    // T_ENTER_KEYPAD
	"\033>",                    // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	ENTER_MOUSE_SEQ,            // T_ENTER_MOUSE
	EXIT_MOUSE_SEQ,             // T_EXIT_MOUSE
};

// Eterm
static const char *eterm_keys[] = {
	"\033[11~",
	"\033[12~",
	"\033[13~",
	"\033[14~",
	"\033[15~",
	"\033[17~",
	"\033[18~",
	"\033[19~",
	"\033[20~",
	"\033[21~",
	"\033[23~",
	"\033[24~",
	"\033[2~",
	"\033[3~",
	"\033[7~",
	"\033[8~",
	"\033[5~",
	"\033[6~",
	"\033[A",
	"\033[B",
	"\033[D",
	"\033[C",
	0
};
static const char *eterm_funcs[] = {
	"\0337\033[?47h",           // T_ENTER_CA
	"\033[2J\033[?47l\0338",    // T_EXIT_CA
	"\033[?25h",                // T_SHOW_CURSOR
	"\033[?25l",                // T_HIDE_CURSOR
	"\033[H\033[2J",            // T_CLEAR_SCREEN
	"\033[m",                   // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[2m",                  // T_FAINT
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"",                         // T_ENTER_KEYPAD
	"",                         // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	"",                         // T_ENTER_MOUSE
	"",                         // T_EXIT_MOUSE
};

// screen
static const char *screen_keys[] = {
	"\033OP",                   // TB_KEY_F1
	"\033OQ",                   // TB_KEY_F2
	"\033OR",                   // TB_KEY_F3
	"\033OS",                   // TB_KEY_F4
	"\033[15~",                 // TB_KEY_F5
	"\033[17~",                 // TB_KEY_F6
	"\033[18~",                 // TB_KEY_F7
	"\033[19~",                 // TB_KEY_F8
	"\033[20~",                 // TB_KEY_F9
	"\033[21~",                 // TB_KEY_F10
	"\033[23~",                 // TB_KEY_F11
	"\033[24~",                 // TB_KEY_F12
	"\033[2~",                  // TB_KEY_INSERT
	"\033[3~",                  // TB_KEY_DELETE
	"\033[1~",                  // TB_KEY_HOME
	"\033[4~",                  // TB_KEY_END
	"\033[5~",                  // TB_KEY_PGUP
	"\033[6~",                  // TB_KEY_PGDN
	"\033OA",                   // TB_KEY_ARROW_UP
	"\033OB",                   // TB_KEY_ARROW_DOWN
	"\033OD",                   // TB_KEY_ARROW_LEFT
	"\033OC",                   // TB_KEY_ARROW_RIGHT
	0
};
static const char *screen_funcs[] = {
	"\033[?1049h",              // T_ENTER_CA
	"\033[?1049l",              // T_EXIT_CA
	"\033[34h\033[?25h",        // T_SHOW_CURSOR
	"\033[?25l",                // T_HIDE_CURSOR
	"\033[H\033[J",             // T_CLEAR_SCREEN
	"\033[m",                   // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[2m",                  // T_FAINT
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"\033[?1h\033=",            // T_ENTER_KEYPAD
	"\033[?1l\033>",            // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	ENTER_MOUSE_SEQ,            // T_ENTER_MOUSE
	EXIT_MOUSE_SEQ,             // T_EXIT_MOUSE
};

// rxvt-unicode
static const char *rxvt_unicode_keys[] = {
	"\033[11~",                 // TB_KEY_F1
	"\033[12~",                 // TB_KEY_F2
	"\033[13~",                 // TB_KEY_F3
	"\033[14~",                 // TB_KEY_F4
	"\033[15~",                 // TB_KEY_F5
	"\033[17~",                 // TB_KEY_F6
	"\033[18~",                 // TB_KEY_F7
	"\033[19~",                 // TB_KEY_F8
	"\033[20~",                 // TB_KEY_F9
	"\033[21~",                 // TB_KEY_F10
	"\033[23~",                 // TB_KEY_F11
	"\033[24~",                 // TB_KEY_F12
	"\033[2~",                  // TB_KEY_INSERT
	"\033[3~",                  // TB_KEY_DELETE
	"\033[7~",                  // TB_KEY_HOME
	"\033[8~",                  // TB_KEY_END
	"\033[5~",                  // TB_KEY_PGUP
	"\033[6~",                  // TB_KEY_PGDN
	"\033[A",                   // TB_KEY_ARROW_UP
	"\033[B",                   // TB_KEY_ARROW_DOWN
	"\033[D",                   // TB_KEY_ARROW_LEFT
	"\033[C",                   // TB_KEY_ARROW_RIGHT
	0
};
static const char *rxvt_unicode_funcs[] = {
	"\033[?1049h",              // T_ENTER_CA
	"\033[r\033[?1049l",        // T_EXIT_CA
	"\033[?25h",                // T_SHOW_CURSOR
	"\033[?25l",                // T_HIDE_CURSOR
	"\033[H\033[2J",            // T_CLEAR_SCREEN
	"\033[m\033(B",             // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[2m",                  // T_FAINT
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"\033=",                    // T_ENTER_KEYPAD
	"\033>",                    // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	ENTER_MOUSE_SEQ,            // T_ENTER_MOUSE
	EXIT_MOUSE_SEQ,             // T_EXIT_MOUSE
};

// linux
static const char *linux_keys[] = {
	"\033[[A",                  // TB_KEY_F1
	"\033[[B",                  // TB_KEY_F2
	"\033[[C",                  // TB_KEY_F3
	"\033[[D",                  // TB_KEY_F4
	"\033[[E",                  // TB_KEY_F5
	"\033[17~",                 // TB_KEY_F6
	"\033[18~",                 // TB_KEY_F7
	"\033[19~",                 // TB_KEY_F8
	"\033[20~",                 // TB_KEY_F9
	"\033[21~",                 // TB_KEY_F10
	"\033[23~",                 // TB_KEY_F11
	"\033[24~",                 // TB_KEY_F12
	"\033[2~",                  // TB_KEY_INSERT
	"\033[3~",                  // TB_KEY_DELETE
	"\033[1~",                  // TB_KEY_HOME
	"\033[4~",                  // TB_KEY_END
	"\033[5~",                  // TB_KEY_PGUP
	"\033[6~",                  // TB_KEY_PGDN
	"\033[A",                   // TB_KEY_ARROW_UP
	"\033[B",                   // TB_KEY_ARROW_DOWN
	"\033[D",                   // TB_KEY_ARROW_LEFT
	"\033[C",                   // TB_KEY_ARROW_RIGHT
	0
};
static const char *linux_funcs[] = {
	"",                         // T_ENTER_CA
	"",                         // T_EXIT_CA
	"\033[?25h\033[?0c",        // T_SHOW_CURSOR
	"\033[?25l\033[?1c",        // T_HIDE_CURSOR
	"\033[H\033[J",             // T_CLEAR_SCREEN
	"\033[0;10m",               // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[2m",                  // T_FAINT
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"",                         // T_ENTER_KEYPAD
	"",                         // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	"",                         // T_ENTER_MOUSE
	"",                         // T_EXIT_MOUSE
};

// xterm
static const char *xterm_keys[] = {
	"\033OP",                   // TB_KEY_F1
	"\033OQ",                   // TB_KEY_F2
	"\033OR",                   // TB_KEY_F3
	"\033OS",                   // TB_KEY_F4
	"\033[15~",                 // TB_KEY_F5
	"\033[17~",                 // TB_KEY_F6
	"\033[18~",                 // TB_KEY_F7
	"\033[19~",                 // TB_KEY_F8
	"\033[20~",                 // TB_KEY_F9
	"\033[21~",                 // TB_KEY_F10
	"\033[23~",                 // TB_KEY_F11
	"\033[24~",                 // TB_KEY_F12
	"\033[2~",                  // TB_KEY_INSERT
	"\033[3~",                  // TB_KEY_DELETE
	"\033OH",                   // TB_KEY_HOME
	"\033OF",                   // TB_KEY_END
	"\033[5~",                  // TB_KEY_PGUP
	"\033[6~",                  // TB_KEY_PGDN
	"\033OA",                   // TB_KEY_ARROW_UP
	"\033OB",                   // TB_KEY_ARROW_DOWN
	"\033OD",                   // TB_KEY_ARROW_LEFT
	"\033OC",                   // TB_KEY_ARROW_RIGHT
	0
};
static const char *xterm_funcs[] = {
	"\033[?1049h",              // T_ENTER_CA
	"\033[?1049l",              // T_EXIT_CA
	"\033[?12l\033[?25h",       // T_SHOW_CURSOR
	"\033[?25l",                // T_HIDE_CURSOR
	"\033[H\033[2J",            // T_CLEAR_SCREEN
	"\033(B\033[m",             // T_SGR0
	"\033[4m",                  // T_UNDERLINE
	"\033[1m",                  // T_BOLD
	"\033[2m",                  // T_FAINT
	"\033[3m",                  // T_ITALIC
	"\033[5m",                  // T_BLINK
	"\033[7m",                  // T_REVERSE
	"\033[?1h\033=",            // T_ENTER_KEYPAD
	"\033[?1l\033>",            // T_EXIT_KEYPAD
	CROSSOUT,                   // T_CROSSOUT
	ENTER_MOUSE_SEQ,            // T_ENTER_MOUSE
	EXIT_MOUSE_SEQ,             // T_EXIT_MOUSE
};

static struct term {
	const char *name;
	const char **keys;
	const char **funcs;
} terms[] = {
	{"rxvt-256color", rxvt_256color_keys, rxvt_256color_funcs},
	{"Eterm", eterm_keys, eterm_funcs},
	{"screen", screen_keys, screen_funcs},
	{"rxvt-unicode", rxvt_unicode_keys, rxvt_unicode_funcs},
	{"linux", linux_keys, linux_funcs},
	{"xterm", xterm_keys, xterm_funcs},
	{0, 0, 0},
};

static bool init_from_terminfo = false;
static const char **keys;
static const char **funcs;

static int try_compatible(const char *term, const char *name,
			  const char **tkeys, const char **tfuncs)
{
	if (strstr(term, name)) {
		keys = tkeys;
		funcs = tfuncs;
		return 0;
	}

	return EUNSUPPORTED_TERM;
}

static int init_term_builtin(void)
{
	int i;
	const char *term = getenv("TERM");

	if (term) {
		for (i = 0; terms[i].name; i++) {
			if (!strcmp(terms[i].name, term)) {
				keys = terms[i].keys;
				funcs = terms[i].funcs;
				return 0;
			}
		}

		/* let's do some heuristic, maybe it's a compatible terminal */
		if (try_compatible(term, "xterm", xterm_keys, xterm_funcs) == 0)
			return 0;
		if (try_compatible(term, "rxvt", rxvt_unicode_keys, rxvt_unicode_funcs) == 0)
			return 0;
		if (try_compatible(term, "linux", linux_keys, linux_funcs) == 0)
			return 0;
		if (try_compatible(term, "Eterm", eterm_keys, eterm_funcs) == 0)
			return 0;
		if (try_compatible(term, "screen", screen_keys, screen_funcs) == 0)
			return 0;
		/* let's assume that 'cygwin' is xterm compatible */
		if (try_compatible(term, "cygwin", xterm_keys, xterm_funcs) == 0)
			return 0;
	}

	return EUNSUPPORTED_TERM;
}

//----------------------------------------------------------------------
// terminfo
//----------------------------------------------------------------------

static char *read_file(const char *file) {
	FILE *f = fopen(file, "rb");
	if (!f)
		return 0;

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

static char *terminfo_try_path(const char *path, const char *term) {
	char tmp[4096];
	snprintf(tmp, sizeof(tmp), "%s/%c/%s", path, term[0], term);
	tmp[sizeof(tmp)-1] = '\0';
	char *data = read_file(tmp);
	if (data) {
		return data;
	}

	// fallback to darwin specific dirs structure
	snprintf(tmp, sizeof(tmp), "%s/%x/%s", path, term[0], term);
	tmp[sizeof(tmp)-1] = '\0';
	return read_file(tmp);
}

static char *load_terminfo(void) {
	char tmp[4096];
	const char *term = getenv("TERM");
	if (!term) {
		return 0;
	}

	// if TERMINFO is set, no other directory should be searched
	const char *terminfo = getenv("TERMINFO");
	if (terminfo) {
		return terminfo_try_path(terminfo, term);
	}

	// next, consider ~/.terminfo
	const char *home = getenv("HOME");
	if (home) {
		snprintf(tmp, sizeof(tmp), "%s/.terminfo", home);
		tmp[sizeof(tmp)-1] = '\0';
		char *data = terminfo_try_path(tmp, term);
		if (data)
			return data;
	}

	// next, TERMINFO_DIRS
	const char *dirs = getenv("TERMINFO_DIRS");
	if (dirs) {
		snprintf(tmp, sizeof(tmp), "%s", dirs);
		tmp[sizeof(tmp)-1] = '\0';
		char *dir = strtok(tmp, ":");
		while (dir) {
			const char *cdir = dir;
			if (strcmp(cdir, "") == 0) {
				cdir = "/usr/share/terminfo";
			}
			char *data = terminfo_try_path(cdir, term);
			if (data)
				return data;
			dir = strtok(0, ":");
		}
	}

	// fallback to /usr/share/terminfo
	return terminfo_try_path("/usr/share/terminfo", term);
}

#define TI_MAGIC 0432
#define TI_ALT_MAGIC 542
#define TI_HEADER_LENGTH 12
#define TB_KEYS_NUM 22

static const char *terminfo_copy_string(char *data, int str, int table) {
	const int16_t off = *(int16_t*)(data + str);
	if (off == -1) {
		// the terminfo file does not define this capability; use blank
		// string. note we still have to malloc since it will be freed.
		char *dst = malloc(sizeof(char));
		*dst = '\0';
		return dst;
	}

	const char *src = data + table + off;
	int len = strlen(src);
	char *dst = malloc(len+1);
	strcpy(dst, src);
	return dst;
}

// Note: The terminfo capability index numbers come from ncurses term.h.
static const int16_t ti_funcs[] = {
	28,     // T_ENTER_CA
	40,     // T_EXIT_CA
	16,     // T_SHOW_CURSOR
	13,     // T_HIDE_CURSOR
	5,      // T_CLEAR_SCREEN
	39,     // T_SGR0
	36,     // T_UNDERLINE
	27,     // T_BOLD
	30,     // T_FAINT
	311,    // T_ITALIC
	26,     // T_BLINK
	34,     // T_REVERSE
	89,     // T_ENTER_KEYPAD
	88,     // T_EXIT_KEYPAD
};


// Note: The terminfo capability index numbers come from ncurses term.h.
static const int16_t ti_keys[] = {
	66,     // TB_KEY_F1
	68,     // TB_KEY_F2 /* apparently not a typo; 67 is F10 for whatever reason */
	69,     // TB_KEY_F3
	70,     // TB_KEY_F4
	71,     // TB_KEY_F5
	72,     // TB_KEY_F6
	73,     // TB_KEY_F7
	74,     // TB_KEY_F8
	75,     // TB_KEY_F9
	67,     // TB_KEY_F10
	216,    // TB_KEY_F11
	217,    // TB_KEY_F12
	77,     // TB_KEY_INSERT
	59,     // TB_KEY_DELETE
	76,     // TB_KEY_HOME
	164,    // TB_KEY_END
	82,     // TB_KEY_PGUP
	81,     // TB_KEY_PGDN
	87,     // TB_KEY_ARROW_UP
	61,     // TB_KEY_ARROW_DOWN
	79,     // TB_KEY_ARROW_LEFT
	83,     // TB_KEY_ARROW_RIGHT
};

// Loads terminal escape sequences from terminfo file or falls back on
// pre-definitions above. The terminfo header consists of six 16-bit ints as
// follows:
//
// 0 the magic number (octal 0432);
// 1 the size, in bytes, of the names section;
// 2 the number of bytes in the boolean section;
// 3 the number of short integers in the numbers section;
// 4 the number of offsets (short integers) in the strings section;
// 5 the size, in bytes, of the string table.
//
// See term(5) manual for description of terminfo binary format.
static int init_term(void) {
	int i;
	char *data = load_terminfo();
	if (!data) {
		init_from_terminfo = false;
		return init_term_builtin();
	}

	int16_t *header = (int16_t*)data;

	const int number_sec_len = header[0] == TI_ALT_MAGIC ? 4 : 2;

	if ((header[1] + header[2]) % 2) {
		// old quirk to align everything on word boundaries
		header[2] += 1;
	}

	const int str_offset = TI_HEADER_LENGTH +
		header[1] + header[2] +	number_sec_len * header[3];
	const int table_offset = str_offset + 2 * header[4];

	keys = malloc(sizeof(const char*) * (TB_KEYS_NUM+1));
	for (i = 0; i < TB_KEYS_NUM; i++) {
		keys[i] = terminfo_copy_string(data,
			str_offset + 2 * ti_keys[i], table_offset);
	}
	keys[TB_KEYS_NUM] = 0;

	funcs = malloc(sizeof(const char*) * T_FUNCS_NUM);
	// the last three entries are reserved for mouse and terminfo extensions.
	// because the table offset is not there, the entries have to fill in manually
	for (i = 0; i < T_FUNCS_NUM-3; i++) {
		funcs[i] = terminfo_copy_string(data,
			str_offset + 2 * ti_funcs[i], table_offset);
	}

	funcs[T_FUNCS_NUM-3] = CROSSOUT;
	funcs[T_FUNCS_NUM-2] = ENTER_MOUSE_SEQ;
	funcs[T_FUNCS_NUM-1] = EXIT_MOUSE_SEQ;

	init_from_terminfo = true;
	free(data);
	return 0;
}

static void shutdown_term(void) {
	if (init_from_terminfo) {
		int i;
		for (i = 0; i < TB_KEYS_NUM; i++) {
			free((void*)keys[i]);
		}
		// the last three entries are reserved for mouse and terminfo extensions.
		// because the table offset is not there, the entries have to fill in
		// manually and do not need to be freed.
		for (i = 0; i < T_FUNCS_NUM-3; i++) {
			free((void*)funcs[i]);
		}
		free(keys);
		free(funcs);
	}
}

// vim: noexpandtab
