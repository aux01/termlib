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
 * See also:
 *   <https://pubs.opengroup.org/onlinepubs/007908799/xcurses/term.h.html>
 *
 */
#include <stdint.h>

/*
 * Read the terminfo database and set up the terminfo structures for the given
 * terminal name, or the TERM environment variable when term is NULL.
 *
 * Simplest invocation uses TERM and standard output:
 *     tb_setupterm(NULL, 1, NULL);
 *
 * Returns 0 on success, -1 if no terminal string is set, -2 if the terminfo
 * file cannot be found.
 *
 * Ncurses counterpart: setupterm().
 */
int tb_setupterm(char *term, int fd);

/*
 * Read terminfo defined capabilities for the current terminal.
 * Each function takes one of the capability name values defined in this file.
 *
 * Ncurses counterpart: tigetflag(), tigetnum(), tigetstr().
 * */
int   tb_getflag(int cap);
int   tb_getnum(int cap);
char *tb_getstr(int cap);

/*
 * Process terminfo parameterized string.
 * The c argument specifies the number of variadic arguments that follow.
 *
 * Returns a newly allocated processed string unless ps is NULL, in which
 * case NULL is returned. The returned string must be freed with free(3).
 */
char *tb_parmn(char *ps, int c, ...);

typedef struct tb_termtype {
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
} tb_termtype;

typedef struct tb_terminal {          /* describe an actual terminal */
    tb_termtype type;             /* terminal type description */
    short       fd;               /* file description being written to */
    char *      termname;         /* term name used in setupterm */
    char *      termdata;
} tb_terminal;

// Boolean capability names
#define tb_auto_left_margin               0
#define tb_auto_right_margin              1
#define tb_no_esc_ctlc                    2
#define tb_ceol_standout_glitch           3
#define tb_eat_newline_glitch             4
#define tb_erase_overstrike               5
#define tb_generic_type                   6
#define tb_hard_copy                      7
#define tb_has_meta_key                   8
#define tb_has_status_line                9
#define tb_insert_null_glitch             10
#define tb_memory_above                   11
#define tb_memory_below                   12
#define tb_move_insert_mode               13
#define tb_move_standout_mode             14
#define tb_over_strike                    15
#define tb_status_line_esc_ok             16
#define tb_dest_tabs_magic_smso           17
#define tb_tilde_glitch                   18
#define tb_transparent_underline          19
#define tb_xon_xoff                       20
#define tb_needs_xon_xoff                 21
#define tb_prtr_silent                    22
#define tb_hard_cursor                    23
#define tb_non_rev_rmcup                  24
#define tb_no_pad_char                    25
#define tb_non_dest_scroll_region         26
#define tb_can_change                     27
#define tb_back_color_erase               28
#define tb_hue_lightness_saturation       29
#define tb_col_addr_glitch                30
#define tb_cr_cancels_micro_mode          31
#define tb_has_print_wheel                32
#define tb_row_addr_glitch                33
#define tb_semi_auto_right_margin         34
#define tb_cpi_changes_res                35
#define tb_lpi_changes_res                36

// Numeric capability names
#define tb_columns                        0
#define tb_init_tabs                      1
#define tb_lines                          2
#define tb_lines_of_memory                3
#define tb_magic_cookie_glitch            4
#define tb_padding_baud_rate              5
#define tb_virtual_terminal               6
#define tb_width_status_line              7
#define tb_num_labels                     8
#define tb_label_height                   9
#define tb_label_width                    10
#define tb_max_attributes                 11
#define tb_maximum_windows                12
#define tb_max_colors                     13
#define tb_max_pairs                      14
#define tb_no_color_video                 15
#define tb_buffer_capacity                16
#define tb_dot_vert_spacing               17
#define tb_dot_horz_spacing               18
#define tb_max_micro_address              19
#define tb_max_micro_jump                 20
#define tb_micro_col_size                 21
#define tb_micro_line_size                22
#define tb_number_of_pins                 23
#define tb_output_res_char                24
#define tb_output_res_line                25
#define tb_output_res_horz_inch           26
#define tb_output_res_vert_inch           27
#define tb_print_rate                     28
#define tb_wide_char_size                 29
#define tb_buttons                        30
#define tb_bit_image_entwining            31
#define tb_bit_image_type                 32

// String capability names
#define tb_back_tab                       0
#define tb_bell                           1
#define tb_carriage_return                2
#define tb_change_scroll_region           3
#define tb_clear_all_tabs                 4
#define tb_clear_screen                   5
#define tb_clr_eol                        6
#define tb_clr_eos                        7
#define tb_column_address                 8
#define tb_command_character              9
#define tb_cursor_address                 10
#define tb_cursor_down                    11
#define tb_cursor_home                    12
#define tb_cursor_invisible               13
#define tb_cursor_left                    14
#define tb_cursor_mem_address             15
#define tb_cursor_normal                  16
#define tb_cursor_right                   17
#define tb_cursor_to_ll                   18
#define tb_cursor_up                      19
#define tb_cursor_visible                 20
#define tb_delete_character               21
#define tb_delete_line                    22
#define tb_dis_status_line                23
#define tb_down_half_line                 24
#define tb_enter_alt_charset_mode         25
#define tb_enter_blink_mode               26
#define tb_enter_bold_mode                27
#define tb_enter_ca_mode                  28
#define tb_enter_delete_mode              29
#define tb_enter_dim_mode                 30
#define tb_enter_insert_mode              31
#define tb_enter_secure_mode              32
#define tb_enter_protected_mode           33
#define tb_enter_reverse_mode             34
#define tb_enter_standout_mode            35
#define tb_enter_underline_mode           36
#define tb_erase_chars                    37
#define tb_exit_alt_charset_mode          38
#define tb_exit_attribute_mode            39
#define tb_exit_ca_mode                   40
#define tb_exit_delete_mode               41
#define tb_exit_insert_mode               42
#define tb_exit_standout_mode             43
#define tb_exit_underline_mode            44
#define tb_flash_screen                   45
#define tb_form_feed                      46
#define tb_from_status_line               47
#define tb_init_1string                   48
#define tb_init_2string                   49
#define tb_init_3string                   50
#define tb_init_file                      51
#define tb_insert_character               52
#define tb_insert_line                    53
#define tb_insert_padding                 54
#define tb_key_backspace                  55
#define tb_key_catab                      56
#define tb_key_clear                      57
#define tb_key_ctab                       58
#define tb_key_dc                         59
#define tb_key_dl                         60
#define tb_key_down                       61
#define tb_key_eic                        62
#define tb_key_eol                        63
#define tb_key_eos                        64
#define tb_key_f0                         65
#define tb_key_f1                         66
#define tb_key_f10                        67
#define tb_key_f2                         68
#define tb_key_f3                         69
#define tb_key_f4                         70
#define tb_key_f5                         71
#define tb_key_f6                         72
#define tb_key_f7                         73
#define tb_key_f8                         74
#define tb_key_f9                         75
#define tb_key_home                       76
#define tb_key_ic                         77
#define tb_key_il                         78
#define tb_key_left                       79
#define tb_key_ll                         80
#define tb_key_npage                      81
#define tb_key_ppage                      82
#define tb_key_right                      83
#define tb_key_sf                         84
#define tb_key_sr                         85
#define tb_key_stab                       86
#define tb_key_up                         87
#define tb_keypad_local                   88
#define tb_keypad_xmit                    89
#define tb_lab_f0                         90
#define tb_lab_f1                         91
#define tb_lab_f10                        92
#define tb_lab_f2                         93
#define tb_lab_f3                         94
#define tb_lab_f4                         95
#define tb_lab_f5                         96
#define tb_lab_f6                         97
#define tb_lab_f7                         98
#define tb_lab_f8                         99
#define tb_lab_f9                         100
#define tb_meta_off                       101
#define tb_meta_on                        102
#define tb_newline                        103
#define tb_pad_char                       104
#define tb_parm_dch                       105
#define tb_parm_delete_line               106
#define tb_parm_down_cursor               107
#define tb_parm_ich                       108
#define tb_parm_index                     109
#define tb_parm_insert_line               110
#define tb_parm_left_cursor               111
#define tb_parm_right_cursor              112
#define tb_parm_rindex                    113
#define tb_parm_up_cursor                 114
#define tb_pkey_key                       115
#define tb_pkey_local                     116
#define tb_pkey_xmit                      117
#define tb_print_screen                   118
#define tb_prtr_off                       119
#define tb_prtr_on                        120
#define tb_repeat_char                    121
#define tb_reset_1string                  122
#define tb_reset_2string                  123
#define tb_reset_3string                  124
#define tb_reset_file                     125
#define tb_restore_cursor                 126
#define tb_row_address                    127
#define tb_save_cursor                    128
#define tb_scroll_forward                 129
#define tb_scroll_reverse                 130
#define tb_set_attributes                 131
#define tb_set_tab                        132
#define tb_set_window                     133
#define tb_tab                            134
#define tb_to_status_line                 135
#define tb_underline_char                 136
#define tb_up_half_line                   137
#define tb_init_prog                      138
#define tb_key_a1                         139
#define tb_key_a3                         140
#define tb_key_b2                         141
#define tb_key_c1                         142
#define tb_key_c3                         143
#define tb_prtr_non                       144
#define tb_char_padding                   145
#define tb_acs_chars                      146
#define tb_plab_norm                      147
#define tb_key_btab                       148
#define tb_enter_xon_mode                 149
#define tb_exit_xon_mode                  150
#define tb_enter_am_mode                  151
#define tb_exit_am_mode                   152
#define tb_xon_character                  153
#define tb_xoff_character                 154
#define tb_ena_acs                        155
#define tb_label_on                       156
#define tb_label_off                      157
#define tb_key_beg                        158
#define tb_key_cancel                     159
#define tb_key_close                      160
#define tb_key_command                    161
#define tb_key_copy                       162
#define tb_key_create                     163
#define tb_key_end                        164
#define tb_key_enter                      165
#define tb_key_exit                       166
#define tb_key_find                       167
#define tb_key_help                       168
#define tb_key_mark                       169
#define tb_key_message                    170
#define tb_key_move                       171
#define tb_key_next                       172
#define tb_key_open                       173
#define tb_key_options                    174
#define tb_key_previous                   175
#define tb_key_print                      176
#define tb_key_redo                       177
#define tb_key_reference                  178
#define tb_key_refresh                    179
#define tb_key_replace                    180
#define tb_key_restart                    181
#define tb_key_resume                     182
#define tb_key_save                       183
#define tb_key_suspend                    184
#define tb_key_undo                       185
#define tb_key_sbeg                       186
#define tb_key_scancel                    187
#define tb_key_scommand                   188
#define tb_key_scopy                      189
#define tb_key_screate                    190
#define tb_key_sdc                        191
#define tb_key_sdl                        192
#define tb_key_select                     193
#define tb_key_send                       194
#define tb_key_seol                       195
#define tb_key_sexit                      196
#define tb_key_sfind                      197
#define tb_key_shelp                      198
#define tb_key_shome                      199
#define tb_key_sic                        200
#define tb_key_sleft                      201
#define tb_key_smessage                   202
#define tb_key_smove                      203
#define tb_key_snext                      204
#define tb_key_soptions                   205
#define tb_key_sprevious                  206
#define tb_key_sprint                     207
#define tb_key_sredo                      208
#define tb_key_sreplace                   209
#define tb_key_sright                     210
#define tb_key_srsume                     211
#define tb_key_ssave                      212
#define tb_key_ssuspend                   213
#define tb_key_sundo                      214
#define tb_req_for_input                  215
#define tb_key_f11                        216
#define tb_key_f12                        217
#define tb_key_f13                        218
#define tb_key_f14                        219
#define tb_key_f15                        220
#define tb_key_f16                        221
#define tb_key_f17                        222
#define tb_key_f18                        223
#define tb_key_f19                        224
#define tb_key_f20                        225
#define tb_key_f21                        226
#define tb_key_f22                        227
#define tb_key_f23                        228
#define tb_key_f24                        229
#define tb_key_f25                        230
#define tb_key_f26                        231
#define tb_key_f27                        232
#define tb_key_f28                        233
#define tb_key_f29                        234
#define tb_key_f30                        235
#define tb_key_f31                        236
#define tb_key_f32                        237
#define tb_key_f33                        238
#define tb_key_f34                        239
#define tb_key_f35                        240
#define tb_key_f36                        241
#define tb_key_f37                        242
#define tb_key_f38                        243
#define tb_key_f39                        244
#define tb_key_f40                        245
#define tb_key_f41                        246
#define tb_key_f42                        247
#define tb_key_f43                        248
#define tb_key_f44                        249
#define tb_key_f45                        250
#define tb_key_f46                        251
#define tb_key_f47                        252
#define tb_key_f48                        253
#define tb_key_f49                        254
#define tb_key_f50                        255
#define tb_key_f51                        256
#define tb_key_f52                        257
#define tb_key_f53                        258
#define tb_key_f54                        259
#define tb_key_f55                        260
#define tb_key_f56                        261
#define tb_key_f57                        262
#define tb_key_f58                        263
#define tb_key_f59                        264
#define tb_key_f60                        265
#define tb_key_f61                        266
#define tb_key_f62                        267
#define tb_key_f63                        268
#define tb_clr_bol                        269
#define tb_clear_margins                  270
#define tb_set_left_margin                271
#define tb_set_right_margin               272
#define tb_label_format                   273
#define tb_set_clock                      274
#define tb_display_clock                  275
#define tb_remove_clock                   276
#define tb_create_window                  277
#define tb_goto_window                    278
#define tb_hangup                         279
#define tb_dial_phone                     280
#define tb_quick_dial                     281
#define tb_tone                           282
#define tb_pulse                          283
#define tb_flash_hook                     284
#define tb_fixed_pause                    285
#define tb_wait_tone                      286
#define tb_user0                          287
#define tb_user1                          288
#define tb_user2                          289
#define tb_user3                          290
#define tb_user4                          291
#define tb_user5                          292
#define tb_user6                          293
#define tb_user7                          294
#define tb_user8                          295
#define tb_user9                          296
#define tb_orig_pair                      297
#define tb_orig_colors                    298
#define tb_initialize_color               299
#define tb_initialize_pair                300
#define tb_set_color_pair                 301
#define tb_set_foreground                 302
#define tb_set_background                 303
#define tb_change_char_pitch              304
#define tb_change_line_pitch              305
#define tb_change_res_horz                306
#define tb_change_res_vert                307
#define tb_define_char                    308
#define tb_enter_doublewide_mode          309
#define tb_enter_draft_quality            310
#define tb_enter_italics_mode             311
#define tb_enter_leftward_mode            312
#define tb_enter_micro_mode               313
#define tb_enter_near_letter_quality      314
#define tb_enter_normal_quality           315
#define tb_enter_shadow_mode              316
#define tb_enter_subscript_mode           317
#define tb_enter_superscript_mode         318
#define tb_enter_upward_mode              319
#define tb_exit_doublewide_mode           320
#define tb_exit_italics_mode              321
#define tb_exit_leftward_mode             322
#define tb_exit_micro_mode                323
#define tb_exit_shadow_mode               324
#define tb_exit_subscript_mode            325
#define tb_exit_superscript_mode          326
#define tb_exit_upward_mode               327
#define tb_micro_column_address           328
#define tb_micro_down                     329
#define tb_micro_left                     330
#define tb_micro_right                    331
#define tb_micro_row_address              332
#define tb_micro_up                       333
#define tb_order_of_pins                  334
#define tb_parm_down_micro                335
#define tb_parm_left_micro                336
#define tb_parm_right_micro               337
#define tb_parm_up_micro                  338
#define tb_select_char_set                339
#define tb_set_bottom_margin              340
#define tb_set_bottom_margin_parm         341
#define tb_set_left_margin_parm           342
#define tb_set_right_margin_parm          343
#define tb_set_top_margin                 344
#define tb_set_top_margin_parm            345
#define tb_start_bit_image                346
#define tb_start_char_set_def             347
#define tb_stop_bit_image                 348
#define tb_stop_char_set_def              349
#define tb_subscript_characters           350
#define tb_superscript_characters         351
#define tb_these_cause_cr                 352
#define tb_zero_motion                    353
#define tb_char_set_names                 354
#define tb_key_mouse                      355
#define tb_mouse_info                     356
#define tb_req_mouse_pos                  357
#define tb_get_mouse                      358
#define tb_set_a_foreground               359
#define tb_set_a_background               360
#define tb_pkey_plab                      361
#define tb_device_type                    362
#define tb_code_set_init                  363
#define tb_set0_des_seq                   364
#define tb_set1_des_seq                   365
#define tb_set2_des_seq                   366
#define tb_set3_des_seq                   367
#define tb_set_lr_margin                  368
#define tb_set_tb_margin                  369
#define tb_bit_image_repeat               370
#define tb_bit_image_newline              371
#define tb_bit_image_carriage_return      372
#define tb_color_names                    373
#define tb_define_bit_image_region        374
#define tb_end_bit_image_region           375
#define tb_set_color_band                 376
#define tb_set_page_length                377
#define tb_display_pc_char                378
#define tb_enter_pc_charset_mode          379
#define tb_exit_pc_charset_mode           380
#define tb_enter_scancode_mode            381
#define tb_exit_scancode_mode             382
#define tb_pc_term_options                383
#define tb_scancode_escape                384
#define tb_alt_scancode_esc               385
#define tb_enter_horizontal_hl_mode       386
#define tb_enter_left_hl_mode             387
#define tb_enter_low_hl_mode              388
#define tb_enter_right_hl_mode            389
#define tb_enter_top_hl_mode              390
#define tb_enter_vertical_hl_mode         391
#define tb_set_a_attributes               392
#define tb_set_pglen_inch                 393
