/*
 *
 * tbti.h
 *
 * Minimal, standalone terminfo(5) processor.
 *
 * This library can be used as a replacement for much of the ncurses terminfo
 * loading and string processing interface. It implements loading and parsing of
 * binary terminfo files as described by term(5).
 *
 * https://pubs.opengroup.org/onlinepubs/007908799/xcurses/term.h.html
 */
#include <stdint.h>

/*
 * Read the terminfo database and set up the terminfo structures for the given
 * terminal name, or the TERM environment variable when term is NULL.
 *
 * Simplest invocation uses TERM and standard output:
 *     setupterm(NULL, 1, NULL);
 */
int setupterm(char *term, int fd);

/*
int tigetflag(char *capname);
int tigetnum(char *capname);
char *tigetstr(char *capname);
char *tparm(char *cap, long p1, long p2, long p3, long p4, long p5, long p6,
        long p7, long p8, long p9);
*/

#define BOOLCOUNT 44
#define NUMCOUNT  39
#define STRCOUNT  414

typedef struct termtype {
    char    *term_names;          /* names for terminal separated by "|" chars */
    int8_t  *bools;               /* array of boolean values */
    int16_t *nums;                /* array of integer values */
    int16_t *str_offs;            /* array of string offsets */
    char    *str_table;           /* pointer to string table */

    uint16_t num_bools;
    uint16_t num_nums;
    uint16_t num_strings;

    char  *ext_str_table;         /* pointer to extended string table */
    char  **ext_names;            /* corresponding names */

    uint16_t ext_bools;           /* count extensions to bools */
    uint16_t ext_nums;            /* count extensions to numbers */
    uint16_t ext_strings;         /* count extensions to strings */
} TERMTYPE;

typedef struct term {           /* describe an actual terminal */
    TERMTYPE    type;           /* terminal type description */
    short       fd;             /* file description being written to */
    char *      termname;       /* term name used in setupterm */
    char *      termdata;
} TERMINAL;
