/*
 *
 * file: minunit.h
 *
 * <http://www.jera.com/techinfo/jtns/jtn002.html>
 *
 */

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define mu_assert(message, test) \
        do { \
            if (!(test)) return __FILE__ ":" TOSTRING(__LINE__) ": " message; \
        } while (0)
#define mu_run_test(test) \
        do { \
            puts("TEST " #test); \
            char *message = test(); tests_run++; \
            if (message) return message; \
        } while (0)
extern int tests_run;
