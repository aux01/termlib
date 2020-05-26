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
	char buf[2] = {0};

	for (char c = 'a'; c <= 'z'; c++) {
		struct tkbd_seq seq = {0};
		buf[0] = c;
		n = parse_char_seq(&seq, buf, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == TKBD_KEY_A + (c - 'a'));
		assert(seq.mod == 0);
		assert(seq.ch == (uint32_t)c);
		assert(seq.data[0] == c && seq.data[1] == '\0');
	}

	for (char c = 'A'; c <= 'Z'; c++) {
		struct tkbd_seq seq = {0};
		buf[0] = c;
		n = parse_char_seq(&seq, buf, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == c);
		assert(seq.mod == TKBD_MOD_SHIFT);
		assert(seq.ch == (uint32_t)c);
		assert(seq.data[0] == c && seq.data[1] == '\0');
	}

	for (char c = '0'; c <= '9'; c++) {
		struct tkbd_seq seq = {0};
		buf[0] = c;
		n = parse_char_seq(&seq, buf, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == c);
		assert(seq.mod == 0);
		assert(seq.ch == (uint32_t)c);
		assert(seq.data[0] == c && seq.data[1] == '\0');
	}

	char const * const punc1 = " `-=[]\\;',./";
	for (int i = 0; punc1[i]; i++) {
		char c = punc1[i];
		buf[0] = c;
		struct tkbd_seq seq = {0};
		n = parse_char_seq(&seq, buf, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == c);
		assert(seq.mod == 0);
		assert(seq.ch == (uint32_t)c);
		assert(seq.data[0] == c && seq.data[1] == '\0');
	}

	char const * const punc2 = "~!@#$%^&*()_+{}|:\"<>?";
	for (int i = 0; punc2[i]; i++) {
		char c = punc2[i];
		buf[0] = c;
		struct tkbd_seq seq = {0};
		n = parse_char_seq(&seq, buf, 1);
		printf("n=%d expect key=0x%02x, got key=0x%02x\n",
		       n, (int)c, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == c);
		assert(seq.mod == TKBD_MOD_SHIFT);
		assert(seq.ch == (uint32_t)c);
		assert(seq.data[0] == c && seq.data[1] == '\0');
	}

	// parsing non control sequences returns zero
	struct tkbd_seq seq0 = {0};
	char *bufno = "ABCD";
	n = parse_ctrl_seq(&seq0, bufno, strlen(bufno));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(seq0.key == 0);
	assert(seq0.mod == 0);
	assert(seq0.ch == 0);
	assert(seq0.data[0] == 0);
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
		struct tkbd_seq seq = {0};
		struct key *k = &keys[i];
		n = parse_ctrl_seq(&seq, k->seq, 1);
		printf("n=%d expect key=0x%x, got key=0x%x\n",
		       n, k->key, seq.key);
		assert(n == 1);
		assert(seq.type == TKBD_KEY);
		assert(seq.key == k->key);
		assert(seq.mod == k->mod);
		assert(seq.ch == (uint32_t)k->seq[0]);
		assert(k->seq[0] == seq.data[0] && seq.data[1] == '\0');
	}

	// parsing non control sequences returns zero
	struct tkbd_seq seq0 = {0};
	char *buf = "ABCD";
	n = parse_ctrl_seq(&seq0, buf, strlen(buf));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(seq0.key == 0);
	assert(seq0.mod == 0);
	assert(seq0.ch == 0);
	assert(seq0.data[0] == 0);
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
		struct tkbd_seq seq = {0};
		struct key *k = &keys[i];
		n = parse_alt_seq(&seq, k->seq, 2);
		printf("n = %d, key = 0x%x\n", n, k->key);
		assert(n == 2);
		assert(seq.type == TKBD_KEY);
		assert(seq.mod == k->mod);
		assert(seq.ch == (uint32_t)seq.data[1]);
		assert(seq.data[0] == '\033');
		assert(seq.data[1] == k->seq[1]);
		assert(seq.data[2] == '\0');
	}

	// parsing non alt sequences
	struct tkbd_seq seq0 = {0};
	char *buf = "ABCD";
	n = parse_alt_seq(&seq0, buf, strlen(buf));
	printf("n = %d\n", n);
	assert(n == 0);
	assert(seq0.key == 0);
	assert(seq0.mod == 0);
	assert(seq0.ch == 0);
	assert(seq0.data[0] == 0);
}

static void test_parse_special_seq(void)
{
	struct tkbd_seq seq;
	char buf[256] = {0};
	int n;

	// stop at null character
	n = parse_special_seq(&seq, buf, 10);
	assert(n == 0);

	// don't read past buf len
	strcpy(buf, "\033[A");
	n = parse_special_seq(&seq, buf, 0);
	assert(n == 0);

	// read one sequence and stop
	strcpy(buf, "\033[A\033[B");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	assert(n == strlen("\033[A"));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UP);
	assert(strcmp(seq.data, "\033[A") == 0);

	// parses CSI sequence
	strcpy(buf, "\033[A");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UP);
	assert(seq.mod == TKBD_MOD_NONE);
	assert(strcmp(seq.data, buf) == 0);

	// parses SS3 sequence
	strcpy(buf, "\033OA");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UP);
	assert(seq.mod == TKBD_MOD_NONE);
	assert(strcmp(seq.data, buf) == 0);

	// parses mod parameters in ansi style sequence (form 1)
	strcpy(buf, "\033[7A");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UP);
	assert(seq.mod == (TKBD_MOD_CTRL|TKBD_MOD_ALT));
	assert(strcmp(seq.data, buf) == 0);

	// parses mod parameters in ansi style sequence (form 2)
	strcpy(buf, "\033[1;7A");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UP);
	assert(seq.mod == (TKBD_MOD_CTRL|TKBD_MOD_ALT));
	assert(strcmp(seq.data, buf) == 0);

	// parses mod parameters in DECFNK style sequence
	strcpy(buf, "\033[24;2~");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_F12);
	assert(seq.mod == TKBD_MOD_SHIFT);
	assert(strcmp(seq.data, buf) == 0);

	// handles out of range DECFNK sequences
	strcpy(buf, "\033[100;2~");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UNKNOWN);
	assert(seq.mod == TKBD_MOD_SHIFT);
	assert(strcmp(seq.data, buf) == 0);

	// handles unmapped ANSI style sequences
	strcpy(buf, "\033[2Y");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UNKNOWN);
	assert(seq.mod == TKBD_MOD_SHIFT);
	assert(strcmp(seq.data, buf) == 0);

	// handles out of table ANSI style sequences
	strcpy(buf, "\033[31;45;33@");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, seq.key, seq.mod);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UNKNOWN);
	assert(seq.mod == TKBD_MOD_NONE);
	assert(strcmp(seq.data, buf) == 0);

	// try to overflow the seq->data buffer
	assert(TKBD_SEQ_MAX == 32 && "update overflow test below");
	strcpy(buf, "\033[2;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;Y");
	memset(&seq, 0, sizeof(seq));
	n = parse_special_seq(&seq, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d, seq = %s\n",
	        n, seq.key, seq.mod, seq.data);
	assert((size_t)n == strlen(buf));
	assert(seq.type == TKBD_KEY);
	assert(seq.key == TKBD_KEY_UNKNOWN);
	assert(seq.mod == TKBD_MOD_SHIFT);
	assert(memcmp(seq.data, buf, seq.len) == 0);
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
		struct tkbd_seq seq = {0};
		struct key k = keys[i];
		n = tkbd_parse(&seq, k.seq, strlen(k.seq));
		printf("n = %d, seq=%s, expect key=%x, mod=%x; got key=%x, mod=%x\n",
		       n, k.seq, k.key, k.mod, seq.key, seq.mod);
		assert(n == (int)strlen(k.seq));
		assert(seq.type == TKBD_KEY);
		assert(seq.mod == k.mod);
		assert(seq.len == (size_t)n);
		assert(memcmp(seq.data, k.seq, seq.len) == 0);
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
