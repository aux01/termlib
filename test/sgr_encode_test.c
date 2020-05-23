#include "../sgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

static void test_uitoa(void)
{
	// test that our uitoa function works correctly
	char buf[16];
	int sz = uitoa(3567, buf);
	assert(sz == 4);
	assert(strcmp("3567", buf) == 0);
}

// struct to hold a string buffer and current position
struct testbuf {
	int pos;
	char str[SGR_STR_MAX];
};

// write callback passed to sgr_encode
static void testbuf_write(void *dest, char *src, int n)
{
	struct testbuf *buf = (struct testbuf*)dest;
	memcpy(buf->str + buf->pos, src, n);
	buf->pos += n;
}

static void test_sgr_encode(void)
{
	// Check that sgr_encode() calls the write callback
	struct testbuf buf = {0};
	struct sgr sgr = {SGR_BOLD|SGR_FG, SGR_RED};
	unsigned n = sgr_encode(&buf, testbuf_write, sgr);
	printf("n = %d, str = %s\n", n, buf.str);
	char *expect = "\x1b[1;31m";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));

	// More complicated example using more codes
	buf.pos = 0;
	memset(buf.str, 0, SGR_STR_MAX);
	sgr = (struct sgr){SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|SGR_BG216, 0, 128};
	n = sgr_encode(&buf, testbuf_write, sgr);
	printf("n = %d, str = %s\n", n, buf.str);
	expect = "\x1b[1;3;4;48;5;144m";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	buf.pos = 0;
	memset(buf.str, 0, SGR_STR_MAX);
	n = sgr_encode(&buf, testbuf_write, (struct sgr){0});
	printf("n = %d, str = %s\n", n, buf.str);
	expect = "";
	assert(strcmp(expect, buf.str) == 0);
	assert(n == strlen(expect));
}

// Write SGR to string buffer.
static void test_sgr_str(void)
{
	char buf[SGR_STR_MAX];

	unsigned n = sgr_str(buf, (struct sgr){SGR_BOLD|SGR_FG, SGR_RED});
	printf("n = %d, str = %s\n", n, buf);
	char *expect = "\x1b[1;31m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// More complicated example using more codes
	n = sgr_str(buf, (struct sgr){
		.at = SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|SGR_BG216,
		.fg = 0,
		.bg = 128
	});
	printf("n = %d, str = %s\n", n, buf);
	expect = "\x1b[1;3;4;48;5;144m";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = sgr_str(buf, (struct sgr){0});
	printf("n = %d, str = %s\n", n, buf);
	expect = "";
	assert(strcmp(expect, buf) == 0);
	assert(n == strlen(expect));
}

// Write SGR to file descriptor.
static void test_sgr_write(void)
{
	int n = sgr_write(1, (struct sgr){SGR_BOLD|SGR_FG,SGR_RED});
	char *expect = "\x1b[1;31m";
	assert((unsigned)n == strlen(expect));

	// More complicated example using more codes
	n = sgr_write(1, (struct sgr){
		.at = SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|SGR_BG216,
		.bg = 128
	});
	expect = "\x1b[1;3;4;48;5;144m";
	assert((unsigned)n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = sgr_write(1, (struct sgr){0});
	expect = "";
	assert((unsigned)n == strlen(expect));

	// Write error
	n = sgr_write(37, (struct sgr){SGR_BOLD, SGR_RED});
	assert(errno != 0);
	assert(n == -1);
}

// Write SGR to file descriptor.
static void test_sgr_fwrite(void)
{
	int n = sgr_fwrite(stdout, (struct sgr){SGR_BOLD|SGR_FG, SGR_RED});
	char *expect = "\x1b[1;31m";
	printf("n = %d\n", n);
	assert((unsigned)n == strlen(expect));

	// More complicated example using more codes
	n = sgr_fwrite(stdout, (struct sgr){
		.at = SGR_BOLD|SGR_ITALIC|SGR_UNDERLINE|SGR_BG216,
		.bg = 128
	});
	expect = "\x1b[1;3;4;48;5;144m";
	assert((unsigned)n == strlen(expect));

	// Empty SGR value shouldn't generate any output
	n = sgr_fwrite(stdout, (struct sgr){0});
	expect = "";
	assert((unsigned)n == strlen(expect));

	// Write error
	n = sgr_fwrite(stdin, (struct sgr){SGR_BOLD, SGR_RED});
	assert(n == -1);
	assert(ferror(stdin) != 0);
}

int main(void)
{
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
