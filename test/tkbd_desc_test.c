#include "../tkbd.h"
#include "../tkbd.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void)
{
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	char buf[64];
	struct tkbd_event ev = { .type = TKBD_KEY, .key = TKBD_KEY_A };

	int n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("A", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.mod = TKBD_MOD_CTRL;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+A", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.mod |= TKBD_MOD_SHIFT|TKBD_MOD_ALT;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+A", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_F12;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+F12", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_F20;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+F20", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_PGUP;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+PgUp", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_HOME;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+HOME", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_ENTER;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+Enter", buf) == 0);
	assert(n == (int)strlen(buf));

	ev.key = TKBD_KEY_ESC;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(strcmp("Ctrl+Shift+Alt+ESC", buf) == 0);
	assert(n == (int)strlen(buf));

	// returns 0 when not a key event
	ev.type = TKBD_MOUSE;
	n = tkbd_desc(buf, sizeof(buf), &ev);
	printf("n=%d, buf=%s\n", n, buf);
	assert(n == 0);

	return 0;
}

// vim: noexpandtab
