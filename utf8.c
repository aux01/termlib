/*
 *
 * utf8.c - Standalone utf8 char identification and codepoint conversion lib
 * Copyright (c) 2020, Auxrelius I <aux01@aux.life>
 *
 * Based originally on Termbox library utf8.c.
 * Copyright (C) 2010-2013 nsf <no.smile.face@gmail.com>
 *
 * See utf8.h for documentation and usage.
 *
 *
 */

#include <stddef.h>            // size_t
#include <stdint.h>            // uint32_t

#include "utf8.h"

static const uint8_t utf8_length[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x00
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x20
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x40
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x60
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x80
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0xa0
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xc0
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1  // 0xe0
};

static const uint8_t utf8_mask[6] = {
	0x7F,
	0x1F,
	0x0F,
	0x07,
	0x03,
	0x01,
};

int utf8_seq_len(char c)
{
	return utf8_length[(uint8_t)c];
}

int utf8_seq_to_codepoint(uint32_t *codepoint, const char *seq, size_t sz)
{
	if (sz < 1)
		return 0;

	int len = utf8_seq_len(seq[0]);
	if (len > (int)sz)
		return 0;

	uint8_t  mask = utf8_mask[len-1];
	uint32_t res = seq[0] & mask;
	for (int i = 1; i < len; ++i) {
		res <<= 6;
		res |= seq[i] & 0x3f;
	}

	*codepoint = res;
	return len;
}

int utf8_codepoint_to_seq(char *seq, uint32_t c)
{
	int len = 0;
	int first;

	if (c < 0x80) {
		first = 0;
		len = 1;
	} else if (c < 0x800) {
		first = 0xc0;
		len = 2;
	} else if (c < 0x10000) {
		first = 0xe0;
		len = 3;
	} else if (c < 0x200000) {
		first = 0xf0;
		len = 4;
	} else if (c < 0x4000000) {
		first = 0xf8;
		len = 5;
	} else {
		first = 0xfc;
		len = 6;
	}

	for (int i = len - 1; i > 0; --i) {
		seq[i] = (c & 0x3f) | 0x80;
		c >>= 6;
	}
	seq[0] = c | first;

	return len;
}
