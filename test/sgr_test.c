
#include "../sgr.c"

#include <stdio.h>
#include <assert.h>

int main(void)
{
	// make stdout line buffered
	setvbuf(stdout, NULL, _IOLBF, -BUFSIZ);

	// check that sgr struct is 8 bytes / 64 bits
	size_t sz = sizeof(struct sgr);
	printf("sizeof(struct sgr) = %ld bytes / %ld bits\n", sz, sz*8);
	assert(sz == 8 || "sgr struct should be 64 bits");

	return 0;
}

// vim: noexpandtab
