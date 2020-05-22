/*
 *
 * utf8.h - Standalone utf8 char identification and codepoint conversion lib
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * Based originally on Termbox library utf8.c.
 * Copyright (C) 2010-2013 nsf <no.smile.face@gmail.com>
 *
 * The utf8 library includes just enough utf8 features for basic terminal
 * input and output processing.
 *
 * The following headers must be included before utf8.h:
 *
 *     #include <stddef.h>
 *     #include <stdint.h>
 *
 *
 */

#pragma once

/*
 * Returns the length in bytes of a utf8 character sequence with ch as the
 * first byte.
 */
int utf8_seq_len(char ch);

/* Convert the utf8 byte sequence pointed to by seq to a 32bit integer unicode
 * codepoint. No more than len bytes will be consumed from the seq array.
 *
 * Returns the number of bytes read from the seq buffer on success.
 * Returns 0 when not enough bytes are available in the seq buffer.
 */
int utf8_seq_to_codepoint(uint32_t *codepoint, const char *seq, size_t len);

/* Convert a 32bit integer unicode codepoint to a utf8 byte sequence and write
 * to the char buffer pointed to by seq. No null terminator character is
 * written to the buffer.
 *
 * The buffer should be allocated to be at least six bytes unless the utf8 byte
 * length of the codepoint is determined to be less beforehand.
 *
 * Returns the number of bytes written to the seq buffer: 1..6.
 */
int utf8_codepoint_to_seq(char *seq, uint32_t codepoint);
