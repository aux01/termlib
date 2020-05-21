#include "../tkbd.h"

#include <stdio.h>             // printf
#include <unistd.h>            // STDIN_FILENO
#include <errno.h>             // errno
#include <string.h>            // strerror

int main(void)
{
	struct tkbd_stream s = {0};
	int err = tkbd_attach(&s, STDIN_FILENO);
	if (err) {
		err = errno;
		fprintf(stderr, "error: tkbd_attach: %s %d\n", strerror(err), err);
		return 1;
	}

	for (;;) {
		struct tkbd_event ev = {0};
		int n = tkbd_read(&s, &ev);
		if (n == 0) {
			sleep(1);
			continue;
		}

		char desc[64];
		tkbd_desc(desc, sizeof(desc), &ev);

		char seq[TKBD_SEQ_MAX*4];
		tkbd_stresc(seq, ev.seq, ev.seqlen);

		printf("%-22s %-14s key=0x%02hhx, mod=0x%02hhx, ch=0x%02hhx, sz=%d\n",
		       desc, seq, ev.key, ev.mod, ev.ch, n);

		if (ev.key == TKBD_KEY_Q && ev.mod == TKBD_MOD_NONE)
			break;
	}

	err = tkbd_detach(&s);
	if (err) {
		err = errno;
		fprintf(stderr, "error: tkbd_attach: %s %d\n", strerror(err), err);
		return 1;
	}

	return 0;
}
