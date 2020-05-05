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

// Write SGR to string buffer.
static void test_sgr_str() {
	char buf[TB_SGR_STR_MAX];

	unsigned n = tb_sgr_str(buf, TB_BOLD|TB_RED);
	printf("n = %d, str = %s\n", n, buf);
	char *expect = "\x1b[1;31m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// More complicated example using more codes
	n = tb_sgr_str(buf,
	                  TB_BOLD|TB_ITALIC|TB_UNDERLINE|
	                  TB_216|TB_BG|128);
	printf("n = %d, str = %s\n", n, buf);
	expect = "\x1b[1;3;4;48;5;144m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = tb_sgr_str(buf, 0);
	printf("n = %d, str = %s\n", n, buf);
	expect = "";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));
}

// Write SGR to file descriptor.
static void test_sgr_write() {
	int n = tb_sgr_write(1, TB_BOLD|TB_RED);
	char *expect = "\x1b[1;31m";
	assert((unsigned)n == strlen(expect));

	// More complicated example using more codes
	n = tb_sgr_write(1,
	                  TB_BOLD|TB_ITALIC|TB_UNDERLINE|
	                  TB_216|TB_BG|128);
	expect = "\x1b[1;3;4;48;5;144m";
	assert((unsigned)n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = tb_sgr_write(1, 0);
	expect = "";
	assert((unsigned)n == strlen(expect));

	// Write error
	n = tb_sgr_write(37, TB_BOLD|TB_RED);
	assert(errno != 0);
	assert(n == -1);
}

// Write SGR to file descriptor.
static void test_sgr_fwrite() {
	int n = tb_sgr_fwrite(stdout, TB_BOLD|TB_RED);
	char *expect = "\x1b[1;31m";
	printf("n = %d\n", n);
	assert((unsigned)n == strlen(expect));

	// More complicated example using more codes
	n = tb_sgr_fwrite(stdout,
	                  TB_BOLD|TB_ITALIC|TB_UNDERLINE|
	                  TB_216|TB_BG|128);
	expect = "\x1b[1;3;4;48;5;144m";
	assert((unsigned)n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = tb_sgr_fwrite(stdout, 0);
	expect = "";
	assert((unsigned)n == strlen(expect));

	// Write error
	n = tb_sgr_fwrite(stdin, TB_BOLD|TB_RED);
	assert(n == -1);
	assert(ferror(stdin) != 0);
}

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	test_uitoa();
	test_sgr_encode();
	test_sgr_str();
	test_sgr_write();
	test_sgr_fwrite();

	return 0;
}

// vim: noexpandtab
