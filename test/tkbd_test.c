#include "../tkbd.h"
#include "../tkbd.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_parse_keyboard_seq(void) {
	struct tkbd_event ev;
	char buf[256] = {0};
	int n;

	// stop at null character
	n = parse_keyboard_seq(&ev, buf, 10);
	assert(n == 0);

	// don't read past buf len
	strcpy(buf, "\033[A");
	n = parse_keyboard_seq(&ev, buf, 0);
	assert(n == 0);

	// read one sequence and stop
	strcpy(buf, "\033[A\033[B");
	memset(&ev, 0, sizeof(ev));
	n = parse_keyboard_seq(&ev, buf, strlen(buf));
	assert(n == strlen("\033[A"));
	assert(ev.key == TKBD_KEY_UP);

	// parses mod parameters in xterm style sequence
	strcpy(buf, "\033[7A");
	memset(&ev, 0, sizeof(ev));
	n = parse_keyboard_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.key == TKBD_KEY_UP);
	assert(ev.mod == (TKBD_MOD_CTRL|TKBD_MOD_ALT));

	// parses mod parameters in vt style sequence
	strcpy(buf, "\033[24;2~");
	memset(&ev, 0, sizeof(ev));
	n = parse_keyboard_seq(&ev, buf, strlen(buf));
	printf("n = %d, key = %d, mod = %d\n", n, ev.key, ev.mod);
	assert((size_t)n == strlen(buf));
	assert(ev.key == TKBD_KEY_F12);
	assert(ev.mod == TKBD_MOD_SHIFT);
}

static void test_parse_keyboard_seq_params(void) {
	int parms[2];
	int n;

	n = parse_keyboard_seq_params(parms, 2, "123");
	printf("n = %d, parms[0] = %d, parms[1] = %d\n", n, parms[0], parms[1]);
	assert(n == 1);
	assert(parms[0] == 123);
	assert(parms[1] == 0);

	memset(&parms, 0, sizeof(parms));
	n = parse_keyboard_seq_params(parms, 2, "123;456;789;");
	printf("n = %d, parms[0] = %d, parms[1] = %d\n", n, parms[0], parms[1]);
	assert(n == 2);
	assert(parms[0] == 123);
	assert(parms[1] == 456);
}

int main(void) {
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	test_parse_keyboard_seq_params();
	test_parse_keyboard_seq();

	return 0;
}

// vim: noexpandtab
