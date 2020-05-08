#define _POSIX_C_SOURCE 200112L

#include "../ti.h"
#include "../ti.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(void) {
	int err;

	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// load terminfo data from our test directory only
	setenv("TERMINFO", "./terminfo", 1);

	// load the terminfo data into the global ti_term struct and associate
	// with standard output:
	ti_term *t = ti_setupterm("xterm-color", 1, &err);
	assert(t != NULL);

	size_t n;
	char buf[TI_PARM_OUTPUT_MAX];

	// %% = print literal %
	n = ti_parmn(buf, "hello %% there %%", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "hello % there %") == 0);
	assert(n == strlen(buf));

	// %i  = increment first two params by one
	// %pn = push param n on stack
	// %d  = pop int off stack and print
	n = ti_parmn(buf, "%i%p1%p2%d%d%p3%d", 3, 16, 42, 50);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "431750") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %{n} = push literal int
	// %c   = pop char and print
	n = ti_parmn(buf, "%'x'%c%{79}%c", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "x79") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %{n} = push literal int
	// %s   = pop string and print
	n = ti_parmn(buf, "%'y'%s%{80}%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "y80") == 0);
	assert(n == strlen(buf));

	// %{n} = push literal int
	// %PV  = pop and store static var V
	// %gV  = recall static var V and push
	// %s   = pop string, print
	n = ti_parmn(buf, "%{1234}%PI%gI%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1234") == 0);
	assert(n == strlen(buf));

	// %{n} = push literal int
	// %Pv  = pop and store dynamic var v
	// %gv  = recall dynamic var v and push
	// %s   = pop string, print
	n = ti_parmn(buf, "%{5678}%Pi%gi%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "5678") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %l   = pop string, push string length
	// %d   = pop int, print
	n = ti_parmn(buf, "%'y'%l%d", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %+   = pop int, pop int, add, push int
	n = ti_parmn(buf, "%p1%p2%+%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "42") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %-   = pop int, pop int, add, push int
	n = ti_parmn(buf, "%p1%p2%-%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "38") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %*   = pop int, pop int, multiply, push int
	n = ti_parmn(buf, "%p1%p2%*%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "80") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %*   = pop int, pop int, multiply, push int
	n = ti_parmn(buf, "%p1%p2%/%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "20") == 0);
	assert(n == strlen(buf));

	// when you're done, remember to free terminal info memory:
	ti_freeterm(t);

	return 0;
}

// vim: noexpandtab
