#include "../tbti.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "miniunit.h"
int tests_run = 0;

static char *test_setupterm() {
	int rc = tb_setupterm("xterm-color", 1);
	mu_assert("tb_setupterm succeeds", rc == 0);
	mu_assert("tb_term is set", tb_term != NULL);
	return 0;
}

static char *test_setupterm_loads_term_names() {
	int rc = strcmp("xterm-color|generic \"ANSI\" color xterm (X Window System)",
	            tb_term->type.term_names);
	mu_assert("term_names", rc == 0);
	return 0;
}

static char *test_setupterm_missing_terminfo_file() {
	int rc = tb_setupterm("xterm-nooooope", 1);
	mu_assert("tb_setupterm returns -2", rc == -2);
	return 0;
}

static char *all_tests() {
	mu_run_test(test_setupterm);
	mu_run_test(test_setupterm_loads_term_names);
	mu_run_test(test_setupterm_missing_terminfo_file);
	return 0;
}

int main(void) {
	setenv("TERMINFO", "./terminfo", 1);

	char *result = all_tests();
	if (result != 0) {
		printf("FAILED: %s\n", result);
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}

// vim: noexpandtab
