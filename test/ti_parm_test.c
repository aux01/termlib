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
	n = ti_parm(buf, "hello %% there %%", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "hello % there %") == 0);
	assert(n == strlen(buf));

	// %i  = increment first two params by one
	// %pn = push param n on stack
	// %d  = pop int off stack and print
	n = ti_parm(buf, "%i%p1%p2%d%d%p3%d", 3, 16, 42, 50);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "431750") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %{n} = push literal int
	// %c   = pop char and print
	n = ti_parm(buf, "%'x'%c%{79}%c", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "x79") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %{n} = push literal int
	// %s   = pop string and print
	n = ti_parm(buf, "%'y'%s%{80}%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "y80") == 0);
	assert(n == strlen(buf));

	// %{n} = push literal int
	// %PV  = pop and store static var V
	// %gV  = recall static var V and push
	// %s   = pop string, print
	n = ti_parm(buf, "%{1234}%PI%gI%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1234") == 0);
	assert(n == strlen(buf));

	// test handling recall of unset static vars
	n = ti_parm(buf, "%gJ%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "") == 0);
	assert(n == strlen(buf));

	// %{n} = push literal int
	// %Pv  = pop and store dynamic var v
	// %gv  = recall dynamic var v and push
	// %s   = pop string, print
	n = ti_parm(buf, "%{5678}%Pi%gi%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "5678") == 0);
	assert(n == strlen(buf));

	// test handling of unset dynamic vars
	n = ti_parm(buf, "%gj%s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "") == 0);
	assert(n == strlen(buf));

	// %'c' = push literal char
	// %l   = pop string, push string length
	// %d   = pop int, print
	n = ti_parm(buf, "%'y'%l%d", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	// MATH OPERATORS ================================================

	// %pn = push param n on stack
	// %+  = pop int, pop int, add, push int
	n = ti_parm(buf, "%p1%p2%+%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "42") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %-  = pop int, pop int, add, push int
	n = ti_parm(buf, "%p1%p2%-%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "38") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %*  = pop int, pop int, multiply, push int
	n = ti_parm(buf, "%p1%p2%*%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "80") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %/  = pop int, pop int, multiply, push int
	n = ti_parm(buf, "%p1%p2%/%d", 2, 40, 2);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "20") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %m  = pop int, pop int, mod, push int
	n = ti_parm(buf, "%p1%p2%m%d", 2, 40, 7);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "5") == 0);
	assert(n == strlen(buf));

	// BITWISE OPERATORS =============================================

	// %pn = push param n on stack
	// %&  = pop int, pop int, binary and, push int
	// %x  = pop int, print hex lower
	n = ti_parm(buf, "%p1%p2%&%x", 2, 0xff, 0x0a);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "a") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %|   = pop int, pop int, binary or, push int
	// %X  = pop int, print hex upper
	n = ti_parm(buf, "%p1%p2%|%X", 2, 0xf1, 0x0a);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "FB") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %^   = pop int, pop int, xor, push int
	// %x  = pop int, print hex upper
	n = ti_parm(buf, "%p1%p2%^%x", 2, 0xf1, 0x0a);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "fb") == 0);
	assert(n == strlen(buf));

	// %pn  = push param n on stack
	// %~   = pop int, bit complement, push int
	// %d   = pop int, print hex upper
	n = ti_parm(buf, "%p1%~%d", 1, 5);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "-6") == 0);
	assert(n == strlen(buf));

	// LOGICAL OPERATORS =============================================

	// %pn = push param n on stack
	// %O  = pop int, pop int, logical or
	// %x  = pop int, print hex
	n = ti_parm(buf, "%p1%p2%O%x", 2, 10, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%p2%O%x", 2, 0, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %O  = pop int, pop int, logical and
	// %x  = pop int, print hex
	n = ti_parm(buf, "%p1%p2%A%x", 2, 10, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%p2%O%x", 2, 10, 10);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %!  = pop int, logical not, push bool
	// %x  = pop int, print hex
	n = ti_parm(buf, "%p1%!%x", 1, 5);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%!%x", 1, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %=  = pop int, compare, push bool
	// %d  = pop int, print decimal
	n = ti_parm(buf, "%p1%p2%=%d", 2, 5, 5);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%p2%=%d", 2, 5, 4);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %>  = pop int, greater than, push bool
	// %d  = pop int, print decimal
	n = ti_parm(buf, "%p1%p2%>%d", 2, 10, 5);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%p2%>%d", 2, 5, 10);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	// %pn = push param n on stack
	// %>  = pop int, less than, push bool
	// %d  = pop int, print decimal
	n = ti_parm(buf, "%p1%p2%<%d", 2, 10, 5);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%p2%<%d", 2, 5, 10);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "1") == 0);
	assert(n == strlen(buf));

	// FORMATTED OUTPUT OPERATORS ====================================

	// %i    = increment first two params by one
	// %pn   = push param n on stack
	// %Fx   = pop int off stack and print formatted
	n = ti_parm(buf, "%p1%:+03x", 1, 76);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "04c") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%:-02X", 1, 76);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "4C") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%p1%04o", 1, 32);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "0040") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%'z'% 4s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "   z") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%'z'%:+ 4s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "   z") == 0);
	assert(n == strlen(buf));

	n = ti_parm(buf, "%'z'%:- 4s", 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "z   ") == 0);
	assert(n == strlen(buf));

	// not sure why . is allowed since there's no float fmt code
	n = ti_parm(buf, "%p1%1.1d", 1, 32);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "32") == 0);
	assert(n == strlen(buf));


	// IF/THEN/ELSE ==================================================

	// %?  = if
	// %p1 = push param 1 on stack
	// %t  = then
	// %e  = else
	// %;  = endif
	n = ti_parm(buf, "%?%p1%tif%eelse%;", 1, 1);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "if") == 0);
	assert(n == strlen(buf));

	// if p1 then "if" else "else" endif
	n = ti_parm(buf, "%?%p1%tif%eelse%;", 1, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "else") == 0);
	assert(n == strlen(buf));

	// if p1 then if p2 then "if if" else "else" endif endif
	n = ti_parm(buf, "%?%p1%t%?%p2%tif if%eelse%;%;", 2, 1, 1);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "if if") == 0);
	assert(n == strlen(buf));

	// if p1 then if p2 then "if if" else "else" endif endif
	n = ti_parm(buf, "%?%p1%t%?%p2%tif if%eelse%;%;", 2, 0, 1);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "") == 0);
	assert(n == strlen(buf));

	// if p1 then "if" else if p2 then "else if" endif endif
	n = ti_parm(buf, "%?%p1%tif%e%?%p2%telse if%;%;", 2, 1, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "if") == 0);
	assert(n == strlen(buf));

	// if p1 then "if" else if p2 then "else if" endif endif
	n = ti_parm(buf, "%?%p1%tif%e%?%p2%telse if%;%;", 2, 0, 1);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "else if") == 0);
	assert(n == strlen(buf));

	// if p1 then "if" else if p2 then "else if" endif endif
	n = ti_parm(buf, "%?%p1%tif%e%?%p2%telse if%;%;", 2, 0, 0);
	printf("buf: %s\n", buf);
	assert(strcmp(buf, "") == 0);
	assert(n == strlen(buf));

	// when you're done, remember to free terminal info memory:
	ti_freeterm(t);

	return 0;
}

// vim: noexpandtab
