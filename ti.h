/*
 *
 * ti.h - Minimal, standalone terminfo(5) processor library.
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * This library can be used as a replacement for much of the ncurses terminfo
 * loading and string processing interface. It handles loading and parsing of
 * binary terminfo files as described by term(5), including legacy and extended
 * format capabilities. Parameterized string processing as described in
 * terminfo(5) is also supported.
 *
 *
 */

#pragma once

#include <stdint.h>

/*
 * Terminfo struct
 *
 * Describes the terminal's capabilities and special escape sequences. The
 * ti_load() function parses a binary terminfo file and populates the
 * ti_terminfo struct. Use ti_free() to release resources associated with
 * the terminfo structure.
 *
 * Most of the struct members are for internal use only. See the ti_getxxx()
 * functions for retrieving capabilities.
 *
 */
typedef struct ti_terminfo {
	char    *term_names;         // names for terminal separated by "|" chars

	int8_t  *bools;              // array of boolean capability values
	int32_t *nums;               // array of integer capability values
	char   **strs;               // array of string capability pointers

	int16_t  bools_count;        // bools array size
	int16_t  nums_count;         // nums array size
	int16_t  strs_count;         // strs array size

	int8_t  *ext_bools;          // array of extended boolean cap values
	int32_t *ext_nums;           // array of extended integer cap values
	char   **ext_strs;           // array of extended string caps pointers

	char   **ext_bool_names;     // array of extended boolean cap names
	char   **ext_num_names;      // array of extended numeric cap names
	char   **ext_str_names;      // array of extended string cap names

	int16_t ext_bools_count;     // ext_bools and ext_bool_names array size
	int16_t ext_nums_count;      // ext_nums and ext_num_names array size
	int16_t ext_strs_count;      // ext_strs and ext_str_names array size

	char   **ext_names;          // array of extended cap names pointers
	int16_t  ext_names_count;    // ext_names array size

	char *   data;               // raw terminfo data loaded from file
	int      len;                // size of data in bytes
} ti_terminfo;

/*
 * Read the terminfo database and set up the structures for the given
 * terminal name, or the TERM environment variable when termname is NULL.
 *
 * Returns a pointer to a newly allocated ti_terminfo struct on success, or
 * NULL when an error occurs. Passing an int pointer in the err argument gives
 * more info on what kind of error occured. See the TI_ERR_XXX defines for more.
 *
 * The ti_free() function must be called with the ti_terminfo pointer returned
 * from ti_load() in order to free ti memory.
 *
 * Ncurses counterpart: setupterm().
 */
ti_terminfo *ti_load(const char *termname, int *err);
void         ti_free(ti_terminfo *ti);

/*
 * Error codes
 *
 * ti_load() takes a pointer to int err argument that's set to one of the
 * following error codes when a terminfo file cannot be loaded.
 *
 * Positive error code values correspond to system errno values.
 *
 */
#define TI_ERR_TERM_NOT_SET -1  // TERM environ var not set
#define TI_ERR_NO_HEADER    -2  // file does not have a terminfo header
#define TI_ERR_BAD_MAGIC    -3  // file magic number is not terminfo
#define TI_ERR_BAD_STROFF   -4  // string offset is beyond string table
#define TI_ERR_BAD_STRTBL   -5  // string table len is beyond EOF

/*
 * Convert an error code set by ti_load() to an error string.
 * The returned string should not be freed.
 */
const char *ti_strerror(int errnum);

/*
 * Read boolean, numeric, or string terminal capabilities for the given
 * capability name string. The capname string must be a terminfo defined
 * capability name; termcap codes and ncurses long names are not supported.
 *
 * The ti_getbool() function returns non-zero if the boolean flag capability is
 * set; zero if the flag is unset or when the capability name is unknown.
 * The ti_getnum() function returns the numeric value set in the terminfo
 * file; -1 when the numeric capability is unset or unknown.
 * The ti_getstr() function returns a pointer to the capability string set in
 * the terminfo file; NULL when the string capability is unset or unknown.
 *
 * NOTE: Strings returned from ti_getstr() are owned by the ti_terminfo struct
 * and should not be modified or freed. All strings are invalid after ti_free()
 * is called so it may be wise to make copies.
 *
 * Ncurses counterpart: tigetflag(), tigetnum(), tigetstr().
 */
int   ti_getbool(ti_terminfo *ti, const char *capname);
int   ti_getnum(ti_terminfo *ti, const char *capname);
char *ti_getstr(ti_terminfo *ti, const char *capname);

/*
 * Read boolean, numeric, or string terminal capabilities for the given
 * integer capability index. See the ti_xxx definitions later in this file for a
 * full list of possible values for each function.
 *
 * These versions of the capability reading functions are a bit faster but are
 * not able to read extended capabilities (setrgbb, setrgbf, smxx, etc.).
 *
 */
int   ti_getbooli(ti_terminfo *ti, int capindex);
int   ti_getnumi(ti_terminfo *ti, int capindex);
char *ti_getstri(ti_terminfo *ti, int capindex);

/*
 * Process terminfo parameterized string.
 * The c argument specifies the number of variadic arguments that follow.
 *
 * Returns the number of bytes written not counting the null terminator.
 */
int ti_parm(char *buf, const char *ps, int c, ...);


/*
 * Write escaped version of str to buf. All non-printable and control characters
 * are escaped. buf should be allocated to be 4x the size of str.
 *
 * Returns the number of bytes written to buf.
 */
int ti_stresc(char *buf, const char *str, int n);

/*
 * Capability name arrays.
 *
 * Each array contains the terminfo names for all defined capabilities in
 * terminfo file order. These are used internally to find the capability's index
 * but can also be used to loop over all available capabilities.
 */
extern const char * const ti_boolnames[];
extern const char * const ti_numnames[];
extern const char * const ti_strnames[];

/*
 * Capability indexes generated with tools/gencap-defs.sh at 2020-05-10T22:56:28 CDT
 *
 */

// Boolean capability indexes
#define ti_bw     0   // cub1 wraps from column 0 to last column
#define ti_am     1   // terminal has automatic margins
#define ti_xsb    2   // beehive (f1=escape, f2=ctrl C)
#define ti_xhp    3   // standout not erased by overwriting (hp)
#define ti_xenl   4   // newline ignored after 80 cols (concept)
#define ti_eo     5   // can erase overstrikes with a blank
#define ti_gn     6   // generic line type
#define ti_hc     7   // hardcopy terminal
#define ti_km     8   // Has a meta key (i.e., sets 8th-bit)
#define ti_hs     9   // has extra status line
#define ti_in     10  // insert mode distinguishes nulls
#define ti_da     11  // display may be retained above the screen
#define ti_db     12  // display may be retained below the screen
#define ti_mir    13  // safe to move while in insert mode
#define ti_msgr   14  // safe to move while in standout mode
#define ti_os     15  // terminal can overstrike
#define ti_eslok  16  // escape can be used on the status line
#define ti_xt     17  // tabs destructive, magic so char (t1061)
#define ti_hz     18  // cannot print ~'s (Hazeltine)
#define ti_ul     19  // underline character overstrikes
#define ti_xon    20  // terminal uses xon/xoff handshaking
#define ti_nxon   21  // padding will not work, xon/xoff required
#define ti_mc5i   22  // printer will not echo on screen
#define ti_chts   23  // cursor is hard to see
#define ti_nrrmc  24  // smcup does not reverse rmcup
#define ti_npc    25  // pad character does not exist
#define ti_ndscr  26  // scrolling region is non-destructive
#define ti_ccc    27  // terminal can re-define existing colors
#define ti_bce    28  // screen erased with background color
#define ti_hls    29  // terminal uses only HLS color notation (Tektronix)
#define ti_xhpa   30  // only positive motion for hpa/mhpa caps
#define ti_crxm   31  // using cr turns off micro mode
#define ti_daisy  32  // printer needs operator to change character set
#define ti_xvpa   33  // only positive motion for vpa/mvpa caps
#define ti_sam    34  // printing in last column causes cr
#define ti_cpix   35  // changing character pitch changes resolution
#define ti_lpix   36  // changing line pitch changes resolution
#define ti_OTbs   37  // uses ^H to move left
#define ti_OTns   38  // crt cannot scroll
#define ti_OTnc   39  // no way to go to start of line
#define ti_OTMT   40  // has meta key
#define ti_OTNL   41  // move down with
#define ti_OTpt   42  // has 8-char tabs invoked with ^I
#define ti_OTxr   43  // return clears the line

// Numeric capability indexes
#define ti_cols    0   // number of columns in a line
#define ti_it      1   // tabs initially every # spaces
#define ti_lines   2   // number of lines on screen or page
#define ti_lm      3   // lines of memory if > line. 0 means varies
#define ti_xmc     4   // number of blank characters left by smso or rmso
#define ti_pb      5   // lowest baud rate where padding needed
#define ti_vt      6   // virtual terminal number (CB/unix)
#define ti_wsl     7   // number of columns in status line
#define ti_nlab    8   // number of labels on screen
#define ti_lh      9   // rows in each label
#define ti_lw      10  // columns in each label
#define ti_ma      11  // maximum combined attributes terminal can handle
#define ti_wnum    12  // maximum number of definable windows
#define ti_colors  13  // maximum number of colors on screen
#define ti_pairs   14  // maximum number of color-pairs on the screen
#define ti_ncv     15  // video attributes that cannot be used with colors
#define ti_bufsz   16  // numbers of bytes buffered before printing
#define ti_spinv   17  // spacing of pins vertically in pins per inch
#define ti_spinh   18  // spacing of dots horizontally in dots per inch
#define ti_maddr   19  // maximum value in micro_..._address
#define ti_mjump   20  // maximum value in parm_..._micro
#define ti_mcs     21  // character step size when in micro mode
#define ti_mls     22  // line step size when in micro mode
#define ti_npins   23  // numbers of pins in print-head
#define ti_orc     24  // horizontal resolution in units per line
#define ti_orl     25  // vertical resolution in units per line
#define ti_orhi    26  // horizontal resolution in units per inch
#define ti_orvi    27  // vertical resolution in units per inch
#define ti_cps     28  // print rate in characters per second
#define ti_widcs   29  // character step size when in double wide mode
#define ti_btns    30  // number of buttons on mouse
#define ti_bitwin  31  // number of passes for each bit-image row
#define ti_bitype  32  // type of bit-image device
#define ti_OTug    33  // number of blanks left by ul
#define ti_OTdC    34  // pad needed for CR
#define ti_OTdN    35  // pad needed for LF
#define ti_OTdB    36  // padding required for ^H
#define ti_OTdT    37  // padding required for ^I
#define ti_OTkn    38  // count of function keys

// String capability indexes
#define ti_cbt       0    // back tab (P)
#define ti_bel       1    // audible signal (bell) (P)
#define ti_cr        2    // carriage return (P*) (P*)
#define ti_csr       3    // change region to line #1 to line #2 (P)
#define ti_tbc       4    // clear all tab stops (P)
#define ti_clear     5    // clear screen and home cursor (P*)
#define ti_el        6    // clear to end of line (P)
#define ti_ed        7    // clear to end of screen (P*)
#define ti_hpa       8    // horizontal position #1, absolute (P)
#define ti_cmdch     9    // terminal settable cmd character in prototype !?
#define ti_cup       10   // move to row #1 columns #2
#define ti_cud1      11   // down one line
#define ti_home      12   // home cursor (if no cup)
#define ti_civis     13   // make cursor invisible
#define ti_cub1      14   // move left one space
#define ti_mrcup     15   // memory relative cursor addressing, move to row #1 columns #2
#define ti_cnorm     16   // make cursor appear normal (undo civis/cvvis)
#define ti_cuf1      17   // non-destructive space (move right one space)
#define ti_ll        18   // last line, first column (if no cup)
#define ti_cuu1      19   // up one line
#define ti_cvvis     20   // make cursor very visible
#define ti_dch1      21   // delete character (P*)
#define ti_dl1       22   // delete line (P*)
#define ti_dsl       23   // disable status line
#define ti_hd        24   // half a line down
#define ti_smacs     25   // start alternate character set (P)
#define ti_blink     26   // turn on blinking
#define ti_bold      27   // turn on bold (extra bright) mode
#define ti_smcup     28   // string to start programs using cup
#define ti_smdc      29   // enter delete mode
#define ti_dim       30   // turn on half-bright mode
#define ti_smir      31   // enter insert mode
#define ti_invis     32   // turn on blank mode (characters invisible)
#define ti_prot      33   // turn on protected mode
#define ti_rev       34   // turn on reverse video mode
#define ti_smso      35   // begin standout mode
#define ti_smul      36   // begin underline mode
#define ti_ech       37   // erase #1 characters (P)
#define ti_rmacs     38   // end alternate character set (P)
#define ti_sgr0      39   // turn off all attributes
#define ti_rmcup     40   // strings to end programs using cup
#define ti_rmdc      41   // end delete mode
#define ti_rmir      42   // exit insert mode
#define ti_rmso      43   // exit standout mode
#define ti_rmul      44   // exit underline mode
#define ti_flash     45   // visible bell (may not move cursor)
#define ti_ff        46   // hardcopy terminal page eject (P*)
#define ti_fsl       47   // return from status line
#define ti_is1       48   // initialization string
#define ti_is2       49   // initialization string
#define ti_is3       50   // initialization string
#define ti_if        51   // name of initialization file
#define ti_ich1      52   // insert character (P)
#define ti_il1       53   // insert line (P*)
#define ti_ip        54   // insert padding after inserted character
#define ti_kbs       55   // backspace key
#define ti_ktbc      56   // clear-all-tabs key
#define ti_kclr      57   // clear-screen or erase key
#define ti_kctab     58   // clear-tab key
#define ti_kdch1     59   // delete-character key
#define ti_kdl1      60   // delete-line key
#define ti_kcud1     61   // down-arrow key
#define ti_krmir     62   // sent by rmir or smir in insert mode
#define ti_kel       63   // clear-to-end-of-line key
#define ti_ked       64   // clear-to-end-of-screen key
#define ti_kf0       65   // F0 function key
#define ti_kf1       66   // F1 function key
#define ti_kf10      67   // F10 function key
#define ti_kf2       68   // F2 function key
#define ti_kf3       69   // F3 function key
#define ti_kf4       70   // F4 function key
#define ti_kf5       71   // F5 function key
#define ti_kf6       72   // F6 function key
#define ti_kf7       73   // F7 function key
#define ti_kf8       74   // F8 function key
#define ti_kf9       75   // F9 function key
#define ti_khome     76   // home key
#define ti_kich1     77   // insert-character key
#define ti_kil1      78   // insert-line key
#define ti_kcub1     79   // left-arrow key
#define ti_kll       80   // lower-left key (home down)
#define ti_knp       81   // next-page key
#define ti_kpp       82   // previous-page key
#define ti_kcuf1     83   // right-arrow key
#define ti_kind      84   // scroll-forward key
#define ti_kri       85   // scroll-backward key
#define ti_khts      86   // set-tab key
#define ti_kcuu1     87   // up-arrow key
#define ti_rmkx      88   // leave 'keyboard_transmit' mode
#define ti_smkx      89   // enter 'keyboard_transmit' mode
#define ti_lf0       90   // label on function key f0 if not f0
#define ti_lf1       91   // label on function key f1 if not f1
#define ti_lf10      92   // label on function key f10 if not f10
#define ti_lf2       93   // label on function key f2 if not f2
#define ti_lf3       94   // label on function key f3 if not f3
#define ti_lf4       95   // label on function key f4 if not f4
#define ti_lf5       96   // label on function key f5 if not f5
#define ti_lf6       97   // label on function key f6 if not f6
#define ti_lf7       98   // label on function key f7 if not f7
#define ti_lf8       99   // label on function key f8 if not f8
#define ti_lf9       100  // label on function key f9 if not f9
#define ti_rmm       101  // turn off meta mode
#define ti_smm       102  // turn on meta mode (8th-bit on)
#define ti_nel       103  // newline (behave like cr followed by lf)
#define ti_pad       104  // padding char (instead of null)
#define ti_dch       105  // delete #1 characters (P*)
#define ti_dl        106  // delete #1 lines (P*)
#define ti_cud       107  // down #1 lines (P*)
#define ti_ich       108  // insert #1 characters (P*)
#define ti_indn      109  // scroll forward #1 lines (P)
#define ti_il        110  // insert #1 lines (P*)
#define ti_cub       111  // move #1 characters to the left (P)
#define ti_cuf       112  // move #1 characters to the right (P*)
#define ti_rin       113  // scroll back #1 lines (P)
#define ti_cuu       114  // up #1 lines (P*)
#define ti_pfkey     115  // program function key #1 to type string #2
#define ti_pfloc     116  // program function key #1 to execute string #2
#define ti_pfx       117  // program function key #1 to transmit string #2
#define ti_mc0       118  // print contents of screen
#define ti_mc4       119  // turn off printer
#define ti_mc5       120  // turn on printer
#define ti_rep       121  // repeat char #1 #2 times (P*)
#define ti_rs1       122  // reset string
#define ti_rs2       123  // reset string
#define ti_rs3       124  // reset string
#define ti_rf        125  // name of reset file
#define ti_rc        126  // restore cursor to position of last save_cursor
#define ti_vpa       127  // vertical position #1 absolute (P)
#define ti_sc        128  // save current cursor position (P)
#define ti_ind       129  // scroll text up (P)
#define ti_ri        130  // scroll text down (P)
#define ti_sgr       131  // define video attributes #1-#9 (PG9)
#define ti_hts       132  // set a tab in every row, current columns
#define ti_wind      133  // current window is lines #1-#2 cols #3-#4
#define ti_ht        134  // tab to next 8-space hardware tab stop
#define ti_tsl       135  // move to status line, column #1
#define ti_uc        136  // underline char and move past it
#define ti_hu        137  // half a line up
#define ti_iprog     138  // path name of program for initialization
#define ti_ka1       139  // upper left of keypad
#define ti_ka3       140  // upper right of keypad
#define ti_kb2       141  // center of keypad
#define ti_kc1       142  // lower left of keypad
#define ti_kc3       143  // lower right of keypad
#define ti_mc5p      144  // turn on printer for #1 bytes
#define ti_rmp       145  // like ip but when in insert mode
#define ti_acsc      146  // graphics charset pairs, based on vt100
#define ti_pln       147  // program label #1 to show string #2
#define ti_kcbt      148  // back-tab key
#define ti_smxon     149  // turn on xon/xoff handshaking
#define ti_rmxon     150  // turn off xon/xoff handshaking
#define ti_smam      151  // turn on automatic margins
#define ti_rmam      152  // turn off automatic margins
#define ti_xonc      153  // XON character
#define ti_xoffc     154  // XOFF character
#define ti_enacs     155  // enable alternate char set
#define ti_smln      156  // turn on soft labels
#define ti_rmln      157  // turn off soft labels
#define ti_kbeg      158  // begin key
#define ti_kcan      159  // cancel key
#define ti_kclo      160  // close key
#define ti_kcmd      161  // command key
#define ti_kcpy      162  // copy key
#define ti_kcrt      163  // create key
#define ti_kend      164  // end key
#define ti_kent      165  // enter/send key
#define ti_kext      166  // exit key
#define ti_kfnd      167  // find key
#define ti_khlp      168  // help key
#define ti_kmrk      169  // mark key
#define ti_kmsg      170  // message key
#define ti_kmov      171  // move key
#define ti_knxt      172  // next key
#define ti_kopn      173  // open key
#define ti_kopt      174  // options key
#define ti_kprv      175  // previous key
#define ti_kprt      176  // print key
#define ti_krdo      177  // redo key
#define ti_kref      178  // reference key
#define ti_krfr      179  // refresh key
#define ti_krpl      180  // replace key
#define ti_krst      181  // restart key
#define ti_kres      182  // resume key
#define ti_ksav      183  // save key
#define ti_kspd      184  // suspend key
#define ti_kund      185  // undo key
#define ti_kBEG      186  // shifted begin key
#define ti_kCAN      187  // shifted cancel key
#define ti_kCMD      188  // shifted command key
#define ti_kCPY      189  // shifted copy key
#define ti_kCRT      190  // shifted create key
#define ti_kDC       191  // shifted delete-character key
#define ti_kDL       192  // shifted delete-line key
#define ti_kslt      193  // select key
#define ti_kEND      194  // shifted end key
#define ti_kEOL      195  // shifted clear-to-end-of-line key
#define ti_kEXT      196  // shifted exit key
#define ti_kFND      197  // shifted find key
#define ti_kHLP      198  // shifted help key
#define ti_kHOM      199  // shifted home key
#define ti_kIC       200  // shifted insert-character key
#define ti_kLFT      201  // shifted left-arrow key
#define ti_kMSG      202  // shifted message key
#define ti_kMOV      203  // shifted move key
#define ti_kNXT      204  // shifted next key
#define ti_kOPT      205  // shifted options key
#define ti_kPRV      206  // shifted previous key
#define ti_kPRT      207  // shifted print key
#define ti_kRDO      208  // shifted redo key
#define ti_kRPL      209  // shifted replace key
#define ti_kRIT      210  // shifted right-arrow key
#define ti_kRES      211  // shifted resume key
#define ti_kSAV      212  // shifted save key
#define ti_kSPD      213  // shifted suspend key
#define ti_kUND      214  // shifted undo key
#define ti_rfi       215  // send next input char (for ptys)
#define ti_kf11      216  // F11 function key
#define ti_kf12      217  // F12 function key
#define ti_kf13      218  // F13 function key
#define ti_kf14      219  // F14 function key
#define ti_kf15      220  // F15 function key
#define ti_kf16      221  // F16 function key
#define ti_kf17      222  // F17 function key
#define ti_kf18      223  // F18 function key
#define ti_kf19      224  // F19 function key
#define ti_kf20      225  // F20 function key
#define ti_kf21      226  // F21 function key
#define ti_kf22      227  // F22 function key
#define ti_kf23      228  // F23 function key
#define ti_kf24      229  // F24 function key
#define ti_kf25      230  // F25 function key
#define ti_kf26      231  // F26 function key
#define ti_kf27      232  // F27 function key
#define ti_kf28      233  // F28 function key
#define ti_kf29      234  // F29 function key
#define ti_kf30      235  // F30 function key
#define ti_kf31      236  // F31 function key
#define ti_kf32      237  // F32 function key
#define ti_kf33      238  // F33 function key
#define ti_kf34      239  // F34 function key
#define ti_kf35      240  // F35 function key
#define ti_kf36      241  // F36 function key
#define ti_kf37      242  // F37 function key
#define ti_kf38      243  // F38 function key
#define ti_kf39      244  // F39 function key
#define ti_kf40      245  // F40 function key
#define ti_kf41      246  // F41 function key
#define ti_kf42      247  // F42 function key
#define ti_kf43      248  // F43 function key
#define ti_kf44      249  // F44 function key
#define ti_kf45      250  // F45 function key
#define ti_kf46      251  // F46 function key
#define ti_kf47      252  // F47 function key
#define ti_kf48      253  // F48 function key
#define ti_kf49      254  // F49 function key
#define ti_kf50      255  // F50 function key
#define ti_kf51      256  // F51 function key
#define ti_kf52      257  // F52 function key
#define ti_kf53      258  // F53 function key
#define ti_kf54      259  // F54 function key
#define ti_kf55      260  // F55 function key
#define ti_kf56      261  // F56 function key
#define ti_kf57      262  // F57 function key
#define ti_kf58      263  // F58 function key
#define ti_kf59      264  // F59 function key
#define ti_kf60      265  // F60 function key
#define ti_kf61      266  // F61 function key
#define ti_kf62      267  // F62 function key
#define ti_kf63      268  // F63 function key
#define ti_el1       269  // Clear to beginning of line
#define ti_mgc       270  // clear right and left soft margins
#define ti_smgl      271  // set left soft margin at current column. See smgl. (ML is not in BSD termcap).
#define ti_smgr      272  // set right soft margin at current column
#define ti_fln       273  // label format
#define ti_sclk      274  // set clock, #1 hrs #2 mins #3 secs
#define ti_dclk      275  // display clock
#define ti_rmclk     276  // remove clock
#define ti_cwin      277  // define a window #1 from #2,#3 to #4,#5
#define ti_wingo     278  // go to window #1
#define ti_hup       279  // hang-up phone
#define ti_dial      280  // dial number #1
#define ti_qdial     281  // dial number #1 without checking
#define ti_tone      282  // select touch tone dialing
#define ti_pulse     283  // select pulse dialing
#define ti_hook      284  // flash switch hook
#define ti_pause     285  // pause for 2-3 seconds
#define ti_wait      286  // wait for dial-tone
#define ti_u0        287  // User string #0
#define ti_u1        288  // User string #1
#define ti_u2        289  // User string #2
#define ti_u3        290  // User string #3
#define ti_u4        291  // User string #4
#define ti_u5        292  // User string #5
#define ti_u6        293  // User string #6
#define ti_u7        294  // User string #7
#define ti_u8        295  // User string #8
#define ti_u9        296  // User string #9
#define ti_op        297  // Set default pair to its original value
#define ti_oc        298  // Set all color pairs to the original ones
#define ti_initc     299  // initialize color #1 to (#2,#3,#4)
#define ti_initp     300  // Initialize color pair #1 to fg=(#2,#3,#4), bg=(#5,#6,#7)
#define ti_scp       301  // Set current color pair to #1
#define ti_setf      302  // Set foreground color #1
#define ti_setb      303  // Set background color #1
#define ti_cpi       304  // Change number of characters per inch to #1
#define ti_lpi       305  // Change number of lines per inch to #1
#define ti_chr       306  // Change horizontal resolution to #1
#define ti_cvr       307  // Change vertical resolution to #1
#define ti_defc      308  // Define a character #1, #2 dots wide, descender #3
#define ti_swidm     309  // Enter double-wide mode
#define ti_sdrfq     310  // Enter draft-quality mode
#define ti_sitm      311  // Enter italic mode
#define ti_slm       312  // Start leftward carriage motion
#define ti_smicm     313  // Start micro-motion mode
#define ti_snlq      314  // Enter NLQ mode
#define ti_snrmq     315  // Enter normal-quality mode
#define ti_sshm      316  // Enter shadow-print mode
#define ti_ssubm     317  // Enter subscript mode
#define ti_ssupm     318  // Enter superscript mode
#define ti_sum       319  // Start upward carriage motion
#define ti_rwidm     320  // End double-wide mode
#define ti_ritm      321  // End italic mode
#define ti_rlm       322  // End left-motion mode
#define ti_rmicm     323  // End micro-motion mode
#define ti_rshm      324  // End shadow-print mode
#define ti_rsubm     325  // End subscript mode
#define ti_rsupm     326  // End superscript mode
#define ti_rum       327  // End reverse character motion
#define ti_mhpa      328  // Like column_address in micro mode
#define ti_mcud1     329  // Like cursor_down in micro mode
#define ti_mcub1     330  // Like cursor_left in micro mode
#define ti_mcuf1     331  // Like cursor_right in micro mode
#define ti_mvpa      332  // Like row_address #1 in micro mode
#define ti_mcuu1     333  // Like cursor_up in micro mode
#define ti_porder    334  // Match software bits to print-head pins
#define ti_mcud      335  // Like parm_down_cursor in micro mode
#define ti_mcub      336  // Like parm_left_cursor in micro mode
#define ti_mcuf      337  // Like parm_right_cursor in micro mode
#define ti_mcuu      338  // Like parm_up_cursor in micro mode
#define ti_scs       339  // Select character set, #1
#define ti_smgb      340  // Set bottom margin at current line
#define ti_smgbp     341  // Set bottom margin at line #1 or (if smgtp is not given) #2 lines from bottom
#define ti_smglp     342  // Set left (right) margin at column #1
#define ti_smgrp     343  // Set right margin at column #1
#define ti_smgt      344  // Set top margin at current line
#define ti_smgtp     345  // Set top (bottom) margin at row #1
#define ti_sbim      346  // Start printing bit image graphics
#define ti_scsd      347  // Start character set definition #1, with #2 characters in the set
#define ti_rbim      348  // Stop printing bit image graphics
#define ti_rcsd      349  // End definition of character set #1
#define ti_subcs     350  // List of subscriptable characters
#define ti_supcs     351  // List of superscriptable characters
#define ti_docr      352  // Printing any of these characters causes CR
#define ti_zerom     353  // No motion for subsequent character
#define ti_csnm      354  // Produce #1'th item from list of character set names
#define ti_kmous     355  // Mouse event has occurred
#define ti_minfo     356  // Mouse status information
#define ti_reqmp     357  // Request mouse position
#define ti_getm      358  // Curses should get button events, parameter #1 not documented.
#define ti_setaf     359  // Set foreground color to #1, using ANSI escape
#define ti_setab     360  // Set background color to #1, using ANSI escape
#define ti_pfxl      361  // Program function key #1 to type string #2 and show string #3
#define ti_devt      362  // Indicate language/codeset support
#define ti_csin      363  // Init sequence for multiple codesets
#define ti_s0ds      364  // Shift to codeset 0 (EUC set 0, ASCII)
#define ti_s1ds      365  // Shift to codeset 1
#define ti_s2ds      366  // Shift to codeset 2
#define ti_s3ds      367  // Shift to codeset 3
#define ti_smglr     368  // Set both left and right margins to #1, #2. (ML is not in BSD termcap).
#define ti_smgtb     369  // Sets both top and bottom margins to #1, #2
#define ti_birep     370  // Repeat bit image cell #1 #2 times
#define ti_binel     371  // Move to next row of the bit image
#define ti_bicr      372  // Move to beginning of same row
#define ti_colornm   373  // Give name for color #1
#define ti_defbi     374  // Define rectangular bit image region
#define ti_endbi     375  // End a bit-image region
#define ti_setcolor  376  // Change to ribbon color #1
#define ti_slines    377  // Set page length to #1 lines
#define ti_dispc     378  // Display PC character #1
#define ti_smpch     379  // Enter PC character display mode
#define ti_rmpch     380  // Exit PC character display mode
#define ti_smsc      381  // Enter PC scancode mode
#define ti_rmsc      382  // Exit PC scancode mode
#define ti_pctrm     383  // PC terminal options
#define ti_scesc     384  // Escape for scancode emulation
#define ti_scesa     385  // Alternate escape for scancode emulation
#define ti_ehhlm     386  // Enter horizontal highlight mode
#define ti_elhlm     387  // Enter left highlight mode
#define ti_elohlm    388  // Enter low highlight mode
#define ti_erhlm     389  // Enter right highlight mode
#define ti_ethlm     390  // Enter top highlight mode
#define ti_evhlm     391  // Enter vertical highlight mode
#define ti_sgr1      392  // Define second set of video attributes #1-#6
#define ti_slength   393  // Set page length to #1 hundredth of an inch (some implementations use sL for termcap).
#define ti_OTi2      394  // secondary initialization string
#define ti_OTrs      395  // terminal reset string
#define ti_OTnl      396  // use to move down
#define ti_OTbc      397  // move left, if not ^H
#define ti_OTko      398  // list of self-mapped keycaps
#define ti_OTma      399  // map motion-keys for vi version 2
#define ti_OTG2      400  // single upper left
#define ti_OTG3      401  // single lower left
#define ti_OTG1      402  // single upper right
#define ti_OTG4      403  // single lower right
#define ti_OTGR      404  // tee pointing right
#define ti_OTGL      405  // tee pointing left
#define ti_OTGU      406  // tee pointing up
#define ti_OTGD      407  // tee pointing down
#define ti_OTGH      408  // single horizontal line
#define ti_OTGV      409  // single vertical line
#define ti_OTGC      410  // single intersection
#define ti_meml      411  // lock memory above cursor
#define ti_memu      412  // unlock memory
#define ti_box1      413  // box characters primary set
