/* term.inl */
#include "tbti.h"

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
	T_ENTER_KEYPAD,
	T_EXIT_KEYPAD,

	T_ENTER_MOUSE,
	T_EXIT_MOUSE,

	T_FUNCS_NUM,
};

#define ENTER_MOUSE_SEQ "\x1b[?1000h\x1b[?1002h\x1b[?1015h\x1b[?1006h"
#define EXIT_MOUSE_SEQ "\x1b[?1006l\x1b[?1015l\x1b[?1002l\x1b[?1000l"

#define EUNSUPPORTED_TERM -1

static tb_term *term;
static const char **keys;
static const char **funcs;

#define TB_KEYS_NUM 22

static const int16_t ti_funcs[] = {
	tb_smcup,         // T_ENTER_CA
	tb_rmcup,         // T_EXIT_CA
	tb_cnorm,         // T_SHOW_CURSOR
	tb_civis,         // T_HIDE_CURSOR
	tb_clear_seq,     // T_CLEAR_SCREEN
	tb_sgr0,          // T_SGR0
	tb_smkx,          // T_ENTER_KEYPAD
	tb_rmkx,          // T_EXIT_KEYPAD
};


static const int16_t ti_keys[] = {
	tb_kf1,           // TB_KEY_F1
	tb_kf2,           // TB_KEY_F2
	tb_kf3,           // TB_KEY_F3
	tb_kf4,           // TB_KEY_F4
	tb_kf5,           // TB_KEY_F5
	tb_kf6,           // TB_KEY_F6
	tb_kf7,           // TB_KEY_F7
	tb_kf8,           // TB_KEY_F8
	tb_kf9,           // TB_KEY_F9
	tb_kf10,          // TB_KEY_F10
	tb_kf11,          // TB_KEY_F11
	tb_kf12,          // TB_KEY_F12
	tb_kich1,         // TB_KEY_INSERT
	tb_kdch1,         // TB_KEY_DELETE
	tb_khome,         // TB_KEY_HOME
	tb_kend,          // TB_KEY_END
	tb_kpp,           // TB_KEY_PGUP
	tb_knp,           // TB_KEY_PGDN
	tb_kcuu1,         // TB_KEY_ARROW_UP
	tb_kcud1,         // TB_KEY_ARROW_DOWN
	tb_kcub1,         // TB_KEY_ARROW_LEFT
	tb_kcuf1,         // TB_KEY_ARROW_RIGHT
};

// Loads terminal escape sequences from terminfo.
static int init_term(void) {
	int err;
	term = tb_setupterm(NULL, 1, &err);
	if (!term) return EUNSUPPORTED_TERM;

	keys = malloc(sizeof(char*) * (TB_KEYS_NUM+1));
	for (int i = 0; i < TB_KEYS_NUM; i++) {
		keys[i] = tb_getstr(term, ti_keys[i]);
	}
	keys[TB_KEYS_NUM] = 0;

	funcs = malloc(sizeof(char*) * T_FUNCS_NUM);
	// the last two entries are reserved for mouse extensions.
	// because the table offset is not there, the entries have to fill in manually
	for (int i = 0; i < T_FUNCS_NUM-2; i++) {
		funcs[i] = tb_getstr(term, ti_funcs[i]);
	}

	funcs[T_FUNCS_NUM-2] = ENTER_MOUSE_SEQ;
	funcs[T_FUNCS_NUM-1] = EXIT_MOUSE_SEQ;

	return 0;
}

static void shutdown_term(void) {
	free(keys);  keys = NULL;
	free(funcs); funcs = NULL;
	tb_freeterm(term); term = NULL;
}

// vim: noexpandtab
