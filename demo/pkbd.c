#include "../tkbd.h"

#include <stdio.h>             // printf
#include <unistd.h>            // STDIN_FILENO
#include <errno.h>             // errno
#include <string.h>            // strerror

#define rmcup "\033[?1049l"    // enter ca mode
#define smcup "\033[?1049h"    // exit ca mode

int main(int argc, char **argv)
{
	int appmode = (argc > 1 && strcmp(argv[1], "-a") == 0);

	// attach to standard input and enter raw mode
	struct tkbd_stream s = {0};
	int err = tkbd_attach(&s, STDIN_FILENO);
	if (err) {
		err = errno;
		fprintf(stderr, "error: tkbd_attach: %s %d\n", strerror(err), err);
		return 1;
	}

	// enter application mode
	if (appmode)
		printf("%s", rmcup);

	// read keys and print info
	for (;;) {
		struct tkbd_event ev = {0};
		int n = tkbd_read(&s, &ev);

		// timeout before data arrived
		if (n == 0)
			continue;

		char desc[64];
		tkbd_desc(desc, sizeof(desc), &ev);

		char seq[TKBD_SEQ_MAX*4];
		tkbd_stresc(seq, ev.seq, ev.seqlen);

		printf("%-22s %-14s key=0x%02hhx, mod=0x%02hhx, ch=0x%02hhx, sz=%d\n",
		       desc, seq, ev.key, ev.mod, ev.ch, n);

		if (ev.key == TKBD_KEY_Q && ev.mod == TKBD_MOD_NONE)
			break;
	}

	// exit application mode
	if (appmode)
		printf("%s", smcup);

	// detach from standard input and exit raw mode
	err = tkbd_detach(&s);
	if (err) {
		err = errno;
		fprintf(stderr, "error: tkbd_detach: %s %d\n", strerror(err), err);
		return 1;
	}

	return 0;
}
