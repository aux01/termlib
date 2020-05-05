#include "../tbsgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

static void test_uitoa() {
	// test that our uitoa function works correctly
	char buf[16];
	int sz = uitoa(3567, buf);
	assert(sz == 4);
	assert(strcmp("3567", buf) == 0);
}

// struct to hold a string buffer and current position
struct testbuf {
	int pos;
	char str[TB_SGR_STR_MAX];
};

// write callback passed to tb_sgr_encode
static void testbuf_write(void *dest, char *src, int n) {
	struct testbuf *buf = (struct testbuf*)dest;
	memcpy(buf->str + buf->pos, src, n);
	buf->pos += n;
}

static void test_sgr_encode() {
	// Check that tb_sgr_encode() calls the write callback
	struct testbuf buf = {0};
	unsigned n = tb_sgr_encode(&buf, testbuf_write, TB_BOLD|TB_RED);
	printf("n = %d, str = %s\n", n, buf.str);
	char *expect = "\x1b[1;31m";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));

	// More complicated example using more codes
	buf.pos = 0;
	memset(buf.str, 0, TB_SGR_STR_MAX);
	n = tb_sgr_encode(&buf, testbuf_write,
	                  TB_BOLD|TB_ITALIC|TB_UNDERLINE|
	                  TB_216|TB_BG|128);
	printf("n = %d, str = %s\n", n, buf.str);
	expect = "\x1b[1;3;4;48;5;144m";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	buf.pos = 0;
	memset(buf.str, 0, TB_SGR_STR_MAX);
	n = tb_sgr_encode(&buf, testbuf_write, 0);
	printf("n = %d, str = %s\n", n, buf.str);
	expect = "";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));
}

static void test_sgr_strcpy() {
	char buf[TB_SGR_STR_MAX];

	unsigned n = tb_sgr_strcpy(buf, TB_BOLD|TB_RED);
	printf("n = %d, str = %s\n", n, buf);
	char *expect = "\x1b[1;31m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// More complicated example using more codes
	n = tb_sgr_strcpy(buf,
	                  TB_BOLD|TB_ITALIC|TB_UNDERLINE|
	                  TB_216|TB_BG|128);
	printf("n = %d, str = %s\n", n, buf);
	expect = "\x1b[1;3;4;48;5;144m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = tb_sgr_strcpy(buf, 0);
	printf("n = %d, str = %s\n", n, buf);
	expect = "";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));
}

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	test_uitoa();
	test_sgr_encode();
	test_sgr_strcpy();

	return 0;
}

// vim: noexpandtab
