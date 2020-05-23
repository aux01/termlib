#include "../tkbd.h"
#include "../tkbd.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_parse_seq_params(void)
{
	int parms[2] = {0};
	int n;

	n = parse_seq_params(parms, 2, "123");
	printf("n = %d, parms[0] = %d, parms[1] = %d\n", n, parms[0], parms[1]);
	assert(n == 1);
	assert(parms[0] == 123);
	assert(parms[1] == 0);

	memset(&parms, 0, sizeof(parms));
	n = parse_seq_params(parms, 2, "123;456;789;");
	printf("n = %d, parms[0] = %d, parms[1] = %d\n", n, parms[0], parms[1]);
	assert(n == 2);
	assert(parms[0] == 123);
	assert(parms[1] == 456);
}

static void test_parse_char_seq(void)
{
	int n;
	char seq[2] = {0};

	for (char c = 'a'; c <= 'z'; c++) {
		struct tkbd_event ev = {0};
		seq[0] = c;
		n = parse_char_seq(&ev, seq, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == TKBD_KEY_A + (c - 'a'));
		assert(ev.mod == 0);
		assert(ev.ch == (uint32_t)c);
		assert(ev.seq[0] == c && ev.seq[1] == '\0');
	}

	for (char c = 'A'; c <= 'Z'; c++) {
		struct tkbd_event ev = {0};
		seq[0] = c;
		n = parse_char_seq(&ev, seq, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == c);
		assert(ev.mod == TKBD_MOD_SHIFT);
		assert(ev.ch == (uint32_t)c);
		assert(ev.seq[0] == c && ev.seq[1] == '\0');
	}

	for (char c = '0'; c <= '9'; c++) {
		struct tkbd_event ev = {0};
		seq[0] = c;
		n = parse_char_seq(&ev, seq, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == c);
		assert(ev.mod == 0);
		assert(ev.ch == (uint32_t)c);
		assert(ev.seq[0] == c && ev.seq[1] == '\0');
	}

	char const * const punc1 = " `-=[]\\;',./";
	for (int i = 0; punc1[i]; i++) {
		char c = punc1[i];
		seq[0] = c;
		struct tkbd_event ev = {0};
		n = parse_char_seq(&ev, seq, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == c);
		assert(ev.mod == 0);
		assert(ev.ch == (uint32_t)c);
		assert(ev.seq[0] == c && ev.seq[1] == '\0');
	}

	char const * const punc2 = "~!@#$%^&*()_+{}|:\"<>?";
	for (int i = 0; punc2[i]; i++) {
		char c = punc2[i];
		seq[0] = c;
		struct tkbd_event ev = {0};
		n = parse_char_seq(&ev, seq, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == c);
		assert(ev.mod == TKBD_MOD_SHIFT);
		assert(ev.ch == (uint32_t)c);
		assert(ev.seq[0] == c && ev.seq[1] == '\0');
	}

	// parsing non control sequences returns zero
	struct tkbd_event ev0 = {0};
	char *buf = "ABCD";
	n = parse_ctrl_seq(&ev0, buf, strlen(buf));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(ev0.key == 0);
	assert(ev0.mod == 0);
	assert(ev0.ch == 0);
	assert(ev0.seq[0] == 0);
}

static void test_parse_ctrl_seq(void)
{
	int n;

	struct key {
		char *seq;
		int key;
		int mod;
	};

	struct key keys[] = {
		{ "\x1C", TKBD_KEY_BACKSLASH, TKBD_MOD_CTRL },
		{ "\x1D", TKBD_KEY_BRACKET_RIGHT, TKBD_MOD_CTRL },
		{ "\x08", TKBD_KEY_BACKSPACE, TKBD_MOD_NONE },
		{ "\x09", TKBD_KEY_TAB, TKBD_MOD_NONE },
		{ "\x0A", TKBD_KEY_ENTER, TKBD_MOD_NONE },
		{ "\x00", TKBD_KEY_2, TKBD_MOD_CTRL },
		{ "\x7F", TKBD_KEY_BACKSPACE2, TKBD_MOD_NONE },
		{ "\x01", TKBD_KEY_A, TKBD_MOD_CTRL },
		{ "\x06", TKBD_KEY_F, TKBD_MOD_CTRL },
		{ "\x1b", TKBD_KEY_ESC, TKBD_MOD_NONE },
	};

	for (int i = 0; i < (int)ARRAYLEN(keys); i++) {
		struct tkbd_event ev = {0};
		struct key *k = &keys[i];
		n = parse_ctrl_seq(&ev, k->seq, 1);
		printf("n=%d expect key=0x%x, got key=0x%x\n",
		       n, k->key, ev.key);
		assert(n == 1);
		assert(ev.type == TKBD_KEY);
		assert(ev.key == k->key);
		assert(ev.mod == k->mod);
		assert(ev.ch == (uint32_t)k->seq[0]);
		assert(k->seq[0] == ev.seq[0] && ev.seq[1] == '\0');
	}

	// parsing non control sequences returns zero
	struct tkbd_event ev0 = {0};
	char *buf = "ABCD";
	n = parse_ctrl_seq(&ev0, buf, strlen(buf));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(ev0.key == 0);
	assert(ev0.mod == 0);
	assert(ev0.ch == 0);
	assert(ev0.seq[0] == 0);
}

static void test_parse_alt_seq(void)
{
	int n;

	struct key {
		char *seq;
		int key;
		int mod;
	};

	struct key keys[] = {
		{ "\033A", TKBD_KEY_A, TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033M", TKBD_KEY_M, TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033Z", TKBD_KEY_Z, TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033a", TKBD_KEY_A, TKBD_MOD_ALT },
		{ "\033m", TKBD_KEY_M, TKBD_MOD_ALT },
		{ "\033z", TKBD_KEY_Z, TKBD_MOD_ALT },
		{ "\0330", TKBD_KEY_0, TKBD_MOD_ALT },
		{ "\0339", TKBD_KEY_9, TKBD_MOD_ALT },
		{ "\033;", TKBD_KEY_SEMICOLON, TKBD_MOD_ALT },
		{ "\033>", TKBD_KEY_GT, TKBD_MOD_SHIFT|TKBD_MOD_ALT },

		{ "\033\x1C", TKBD_KEY_BACKSLASH, TKBD_MOD_CTRL|TKBD_MOD_ALT },
		{ "\033\x08", TKBD_KEY_BACKSPACE, TKBD_MOD_NONE|TKBD_MOD_ALT },
		{ "\033\x09", TKBD_KEY_TAB, TKBD_MOD_NONE|TKBD_MOD_ALT },
		{ "\033\x0A", TKBD_KEY_ENTER, TKBD_MOD_NONE|TKBD_MOD_ALT },
		{ "\033\x00", TKBD_KEY_2, TKBD_MOD_CTRL|TKBD_MOD_ALT },
		{ "\033\x7F", TKBD_KEY_BACKSPACE2, TKBD_MOD_NONE|TKBD_MOD_ALT },
		{ "\033\x01", TKBD_KEY_A, TKBD_MOD_CTRL|TKBD_MOD_ALT },
		{ "\033\x06", TKBD_KEY_F, TKBD_MOD_CTRL|TKBD_MOD_ALT },
		{ "\033\033", TKBD_KEY_ESC, TKBD_MOD_ALT },

		{ "\033\033[A",   TKBD_KEY_UP, TKBD_MOD_ALT },
		{ "\033\x08[24~", TKBD_KEY_F12, TKBD_MOD_ALT },
	};

	for (int i = 0; i < (int)ARRAYLEN(keys); i++) {
		struct tkbd_event ev = {0};
		struct key *k = &keys[i];
		n = parse_alt_seq(&ev, k->seq, 2);
		printf("n = %d, key = 0x%x\n", n, k->key);
		assert(n == 2);
		assert(ev.type == TKBD_KEY);
		assert(ev.mod == k->mod);
		assert(ev.ch == (uint32_t)ev.seq[1]);
		assert(ev.seq[0] == '\033');
		assert(ev.seq[1] == k->seq[1]);
		assert(ev.seq[2] == '\0');
	}

	// parsing non alt sequences
	struct tkbd_event ev0 = {0};
	char *buf = "ABCD";
	n = parse_alt_seq(&ev0, buf, strlen(buf));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(ev0.key == 0);
	assert(ev0.mod == 0);
	assert(ev0.ch == 0);
	assert(ev0.seq[0] == 0);
}

static void test_parse_special_seq(void)
{
	struct tkbd_event ev;
	char buf[256] = {0};
	int n;

	// stop at null character
	n = parse_special_seq(&ev, buf, 10);
	assert(n == 0);

	// don't read past buf len
	strcpy(buf, "\033[A");
	n = parse_special_seq(&ev, buf, 0);
	assert(n == 0);

	// read one sequence and stop
	strcpy(buf, "\033[A\033[B");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	assert(n == strlen("\033[A"));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UP);
	assert(strcmp(ev.seq, "\033[A") == 0);

	// parses CSI xterm sequence
	strcpy(buf, "\033[A");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UP);
	assert(ev.mod == TKBD_MOD_NONE);
	assert(strcmp(ev.seq, buf) == 0);

	// parses SS3 xterm sequence
	strcpy(buf, "\033OA");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UP);
	assert(ev.mod == TKBD_MOD_NONE);
	assert(strcmp(ev.seq, buf) == 0);

	// parses mod parameters in xterm style sequence (form 1)
	strcpy(buf, "\033[7A");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UP);
	assert(ev.mod == (TKBD_MOD_CTRL|TKBD_MOD_ALT));
	assert(strcmp(ev.seq, buf) == 0);

	// parses mod parameters in xterm style sequence (form 2)
	strcpy(buf, "\033[1;7A");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UP);
	assert(ev.mod == (TKBD_MOD_CTRL|TKBD_MOD_ALT));
	assert(strcmp(ev.seq, buf) == 0);

	// parses mod parameters in vt style sequence
	strcpy(buf, "\033[24;2~");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_F12);
	assert(ev.mod == TKBD_MOD_SHIFT);
	assert(strcmp(ev.seq, buf) == 0);

	// handles out of range vt sequences
	strcpy(buf, "\033[100;2~");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UNKNOWN);
	assert(ev.mod == TKBD_MOD_SHIFT);
	assert(strcmp(ev.seq, buf) == 0);

	// handles unmapped xterm style sequences
	strcpy(buf, "\033[2Y");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UNKNOWN);
	assert(ev.mod == TKBD_MOD_SHIFT);
	assert(strcmp(ev.seq, buf) == 0);

	// try to overflow the ev->seq buffer
	assert(TKBD_SEQ_MAX == 32 && "update overflow test below");
	strcpy(buf, "\033[2;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;Y");
	memset(&ev, 0, sizeof(ev));
	n = parse_special_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d, seq = %s\n", n, ev.key, ev.mod, ev.seq);
	assert((size_t)n == strlen(buf));
	assert(ev.type == TKBD_KEY);
	assert(ev.key == TKBD_KEY_UNKNOWN);
	assert(ev.mod == TKBD_MOD_SHIFT);
	assert(memcmp(ev.seq, buf, ev.seqlen) == 0);
}

static void test_parse()
{
	int n;

	struct key {
		char *seq;
		int key;
		int mod;
	};

	struct key keys[] = {
		// parse_char_seq
		{ "a",        TKBD_KEY_A,         TKBD_MOD_NONE },
		{ "z",        TKBD_KEY_Z,         TKBD_MOD_NONE },
		{ "A",        TKBD_KEY_A,         TKBD_MOD_SHIFT },
		{ "Z",        TKBD_KEY_Z,         TKBD_MOD_SHIFT },
		{ "`",        TKBD_KEY_BACKTICK,  TKBD_MOD_NONE },
		{ "/",        TKBD_KEY_SLASH,     TKBD_MOD_NONE },

		// parse_ctrl_seq
		{ "\033",     TKBD_KEY_ESC,       TKBD_MOD_NONE },
		{ "\x01",     TKBD_KEY_A,         TKBD_MOD_CTRL },
		{ "\x1A",     TKBD_KEY_Z,         TKBD_MOD_CTRL },
		{ "\x09",     TKBD_KEY_TAB,       TKBD_MOD_NONE },
		{ "\x0A",     TKBD_KEY_ENTER,     TKBD_MOD_NONE },

		// parse_alt_seq
		{ "\033A",    TKBD_KEY_A,         TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033Z",    TKBD_KEY_Z,         TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033a",    TKBD_KEY_A,         TKBD_MOD_ALT },
		{ "\033z",    TKBD_KEY_Z,         TKBD_MOD_ALT },
		{ "\0330",    TKBD_KEY_0,         TKBD_MOD_ALT },
		{ "\0339",    TKBD_KEY_9,         TKBD_MOD_ALT },
		{ "\033;",    TKBD_KEY_SEMICOLON, TKBD_MOD_ALT },
		{ "\033>",    TKBD_KEY_GT,        TKBD_MOD_SHIFT|TKBD_MOD_ALT },
		{ "\033\x09", TKBD_KEY_TAB,       TKBD_MOD_ALT },
		{ "\033\x0A", TKBD_KEY_ENTER,     TKBD_MOD_ALT },
		{ "\033\x01", TKBD_KEY_A,         TKBD_MOD_CTRL|TKBD_MOD_ALT },
		{ "\033\033", TKBD_KEY_ESC,       TKBD_MOD_ALT },

		// parse_special_seq
		{ "\033[A",      TKBD_KEY_UP,  TKBD_MOD_NONE },
		{ "\033[1A",     TKBD_KEY_UP,  TKBD_MOD_NONE },
		{ "\033[1;2A",   TKBD_KEY_UP,  TKBD_MOD_SHIFT },
		{ "\033[1;8A",   TKBD_KEY_UP,  TKBD_MOD_SHIFT|
		                               TKBD_MOD_ALT|
		                               TKBD_MOD_CTRL },
		{ "\033[24;2~",  TKBD_KEY_F12, TKBD_MOD_SHIFT },

		// Shift+Tab special case
		{ "\033[Z",  TKBD_KEY_TAB, TKBD_MOD_SHIFT },

		// linux term special cases
		{ "\033[[A", TKBD_KEY_F1, TKBD_MOD_NONE },
		{ "\033[[E", TKBD_KEY_F5, TKBD_MOD_NONE },
	};

	for (int i = 0; i < (int)ARRAYLEN(keys); i++) {
		struct tkbd_event ev = {0};
		struct key k = keys[i];
		n = tkbd_parse(&ev, k.seq, strlen(k.seq));
		printf("n = %d, expect key=%x, mod=%x; got key=%x, mod=%x\n",
		       n, k.key, k.mod, ev.key, ev.mod);
		assert(n == (int)strlen(k.seq));
		assert(ev.type == TKBD_KEY);
		assert(ev.mod == k.mod);
		assert(ev.seqlen == (size_t)n);
		assert(memcmp(ev.seq, k.seq, ev.seqlen) == 0);
	}
}

int main(void)
{
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	test_parse_seq_params();
	test_parse_char_seq();
	test_parse_ctrl_seq();
	test_parse_alt_seq();
	test_parse_special_seq();
	test_parse();

	return 0;
}

// vim: noexpandtab
