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

	// example of keyboard escape sequence
	char buf[128] = {0};
	char *s = "\033[21;1~";
	char *expect = "\\e[21;1~";
	int n = tkbd_stresc(buf, s, strlen(s));
	printf("n=%d, buf=%s\n", n, buf);
	assert(n == (int)strlen(s)+1);
	assert(strcmp(buf, expect) == 0);


	// these special characters are printed with lettered escape codes
	s = "\0\t\r\n\\";
	expect = "\\0\\t\\r\\n\\\\";
	n = tkbd_stresc(buf, s, 5);
	printf("n=%d, buf=%s\n", n, buf);
	assert(n == 10);
	assert(strcmp(buf, expect) == 0);


	// all other non-printable characters are printed in octal
	s = "\001\002\010\016\x7f";
	expect = "\\001\\002\\010\\016\\177";
	n = tkbd_stresc(buf, s, strlen(s));
	printf("n=%d, buf=%s\n", n, buf);
	assert(n == (int)strlen(s)*4);
	assert(strcmp(buf, expect) == 0);

	return 0;
}

// vim: noexpandtab
