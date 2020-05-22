#include "../utf8.c"

#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_utf8_len(void)
{
	// U+00 - U+7F ascii range
	assert(utf8_seq_len('A') == 1);
	assert(utf8_seq_len('\0') == 1);
	assert(utf8_seq_len('\x7f') == 1);

	// U+80 - U+07FF range
	assert(utf8_seq_len('\xc0') == 2);
	assert(utf8_seq_len('\xdf') == 2);

	// U+0800 - U+FFFF range
	assert(utf8_seq_len('\xe0') == 3);
	assert(utf8_seq_len('\xef') == 3);

	// U+10000 - U+1FFFF range
	assert(utf8_seq_len('\xf0') == 4);
	assert(utf8_seq_len('\xf7') == 4);

	// U+200000 - U+3FFFFFF range
	assert(utf8_seq_len('\xf8') == 5);
	assert(utf8_seq_len('\xfb') == 5);

	// U+4000000 - U+7FFFFFFF range
	assert(utf8_seq_len('\xfc') == 6);
	assert(utf8_seq_len('\xfd') == 6);

	// 0x80-0xbf, 0xfe, 0xff are continuation bytes
	assert(utf8_seq_len('\x80') == 1);
	assert(utf8_seq_len('\xbf') == 1);
	assert(utf8_seq_len('\xfe') == 1);
	assert(utf8_seq_len('\xff') == 1);
}

struct t {
	char *seq;
	uint32_t codepoint;
};

static const struct t tests[] = {
	{ "A",            'A'    },
	{ "\x01",         0x0001 },
	{ "\x7f",         0x007f },

	{ "\xc2\x80",     0x0080 }, // €
	{ "\xc2\xa9",     0x00A9 }, // ©
	{ "\xca\xb0",     0x02B0 }, // ʰ
	{ "\xcd\xb0",     0x0370 }, // Ͱ
	{ "\xd0\x84",     0x0404 }, // Є
	{ "\xd4\xb1",     0x0531 }, // Ա

	{ "\xe0\xa4\x84", 0x0904 }, // ऄ
	{ "\xe1\x82\xa0", 0x10A0 }, // Ⴀ
	{ "\xe2\x86\x88", 0x2188 }, // ↈ
	{ "\xe3\x80\x84", 0x3004 }, // 〄
	{ "\xe4\x80\x87", 0x4007 }, // 䀇
	{ "\xe5\x82\x96", 0x5096 }, // 傖
	{ "\xef\xbf\xbd", 0xFFFD }, // �

	{ "\xf0\x90\x8c\x8f", 0x1030F }, // 𐌏
	{ "\xf0\x9f\x82\xbb", 0x1F0BB }, // 🂻
	{ "\xf0\x9f\x86\x92", 0x1F192 }, // 🆒
	{ "\xf0\x9f\x8c\xae", 0x1F32E }, // 🌮
	{ "\xf3\xa0\x80\xa4", 0xE0024 },

	{ "\xf4\x80\x80\x80", 0x100000 },
};

static void test_utf8_seq_to_codepoint(void)
{
	for (int i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
		const struct t *t = &tests[i];
		uint32_t codepoint;
		int n = utf8_seq_to_codepoint(&codepoint, t->seq, strlen(t->seq));
		printf("n=%d, codepoint=U+%04X\n", n, codepoint);
		assert(n == (int)strlen(t->seq));
		assert(codepoint == t->codepoint);
	}

	// test \0 separately because of strlen issues
	uint32_t codepoint;
	int n = utf8_seq_to_codepoint(&codepoint, "\0", 1);
	printf("n=%d, codepoint=U+%04X\n", n, codepoint);
	assert(n == 1);
	assert(codepoint == 0x00);
}

static void test_utf8_codepoint_to_seq(void)
{
	for (int i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
		const struct t *t = &tests[i];
		char buf[7] = {0};
		int n = utf8_codepoint_to_seq(buf, t->codepoint);
		printf("n=%d, codepoint=U+%04X, seq=%s\n", n, t->codepoint, buf);
		assert(n == (int)strlen(buf));
		assert(strcmp(buf, t->seq) == 0);
	}

	// test \0 separately because of strlen issues
	char buf[2] = { 'A', '\0' };
	int n = utf8_codepoint_to_seq(buf, 0x00);
	printf("n=%d, codepoint=U+%04X\n", n, 0x00);
	assert(n == 1);
	assert(buf[0] == '\0');
}

int main(void)
{
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	test_utf8_len();
	test_utf8_seq_to_codepoint();
	test_utf8_codepoint_to_seq();

	return 0;
}

// vim: noexpandtab
