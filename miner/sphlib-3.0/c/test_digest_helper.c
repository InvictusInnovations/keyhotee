/* $Id: test_digest_helper.c 192 2010-05-25 22:33:34Z tp $ */
/*
 * Helper code for hash function unit tests. This code is meant to be
 * included by another file which then uses the macros defined herein.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2007-2010  Projet RNRT SAPHIR
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@cryptolog.com>
 */

#include <stdlib.h>
#include <string.h>
#include "utest.h"

/*
 * This macro defines a function which verifies a test vector. The input
 * data is an ASCII string; the reference string is the hexadecimal
 * representation of the expected binary output. Hashing is done twice,
 * in order to exercise functions with misaligned input, as well as
 * proper state reinitialization. The announced output length is also
 * checked against the output size macro.
 */
#define TEST_DIGEST_INTERNAL(Name, cname, blen) \
static void \
test_ ## cname ## _internal(char *data, char *refres) \
{ \
	sph_ ## cname ## _context mc; \
	unsigned char res[blen], ref[blen]; \
	size_t dbuf_len; \
	unsigned char *dbuf; \
 \
	if (((blen) * 8) != SPH_SIZE_ ## cname) \
		fail("wrong output size (%u, exp: %u)", \
			(unsigned)(blen) * 8, (unsigned)SPH_SIZE_ ## cname); \
	dbuf_len = strlen(data); \
	sph_ ## cname ## _init(&mc); \
	sph_ ## cname(&mc, (unsigned char *)data, dbuf_len); \
	sph_ ## cname ## _close(&mc, res); \
	utest_strtobin(ref, refres); \
	ASSERT(utest_byteequal(res, ref, blen)); \
	dbuf = malloc(1 + dbuf_len); \
	if (dbuf == NULL) { \
		fail("malloc() failed (length = %lu)", \
			(unsigned long)(1 + dbuf_len)); \
	} \
	memset(res, 0, sizeof res); \
	memcpy(1 + dbuf, data, dbuf_len); \
	sph_ ## cname(&mc, dbuf + 1, dbuf_len); \
	sph_ ## cname ## _close(&mc, res); \
	ASSERT(utest_byteequal(res, ref, blen)); \
	free(dbuf); \
}

/*
 * This macro defines a function which verifies a test vector. The input
 * data is an hexadecimal string, but 0 to 7 bits in the last byte may
 * be ignored. The reference string is the hexadecimal representation of
 * the expected binary output. This function exercises the support for
 * partial bytes (i.e. an input message bit length which is not a
 * multiple of eight).
 *
 * Hashing is done twice, in order to exercise functions with misaligned
 * input, as well as proper state reinitialization. The announced output
 * length is also checked against the output size macro.
 */
#define TEST_DIGEST_INTERNAL_BITS(Name, cname, blen) \
static void \
test_ ## cname ## _internal_bits(char *data, unsigned ignored, char *refres) \
{ \
	sph_ ## cname ## _context mc; \
	unsigned char res[blen], ref[blen]; \
	size_t dbuf_len; \
	unsigned char *dbuf; \
 \
	if (((blen) * 8) != SPH_SIZE_ ## cname) \
		fail("wrong output size (%u, exp: %u)", \
			(unsigned)(blen) * 8, (unsigned)SPH_SIZE_ ## cname); \
	dbuf_len = 1 + 2 * strlen(data); \
	dbuf = malloc(1 + 2 * strlen(data)); \
	if (dbuf == NULL) { \
		fail("malloc() failed (length = %lu)", \
			(unsigned long)dbuf_len); \
	} \
	dbuf_len = utest_strtobin(dbuf, data); \
	sph_ ## cname ## _init(&mc); \
	if (ignored == 0) { \
		sph_ ## cname(&mc, dbuf, dbuf_len); \
		sph_ ## cname ## _close(&mc, res); \
	} else { \
		sph_ ## cname(&mc, dbuf, dbuf_len - 1); \
		sph_ ## cname ## _addbits_and_close(&mc, \
			dbuf[dbuf_len - 1], 8 - ignored, res); \
	} \
	utest_strtobin(ref, refres); \
	ASSERT(utest_byteequal(res, ref, blen)); \
	memset(res, 0, sizeof res); \
	memmove(1 + dbuf, dbuf, dbuf_len); \
	if (ignored == 0) { \
		sph_ ## cname(&mc, dbuf + 1, dbuf_len); \
		sph_ ## cname ## _close(&mc, res); \
	} else { \
		sph_ ## cname(&mc, dbuf + 1, dbuf_len - 1); \
		sph_ ## cname ## _addbits_and_close(&mc, \
			dbuf[dbuf_len], 8 - ignored, res); \
	} \
	ASSERT(utest_byteequal(res, ref, blen)); \
	free(dbuf); \
}

/*
 * This macro defines a function which is similar to the one defined
 * by TEST_DIGEST_INTERNAL_BITS, except that it uses the NIST messages
 * for SHA-3 test vectors, indexed by message size (in bits, from 0 to
 * 2047, inclusive).
 */
#define TEST_DIGEST_NIST(Name, cname, blen) \
static void \
test_ ## cname ## _nist(unsigned u, char *refres) \
{ \
	sph_ ## cname ## _context mc; \
	unsigned char res[blen], ref[blen]; \
	size_t dbuf_len; \
	unsigned char dbuf[260]; \
	unsigned extra; \
 \
	if (((blen) * 8) != SPH_SIZE_ ## cname) \
		fail("wrong output size (%u, exp: %u)", \
			(unsigned)(blen) * 8, (unsigned)SPH_SIZE_ ## cname); \
	dbuf_len = (u + 7) / 8; \
	extra = u & 7; \
	memcpy(dbuf, utest_nist_data(u), dbuf_len); \
	sph_ ## cname ## _init(&mc); \
	if (extra == 0) { \
		sph_ ## cname(&mc, dbuf, dbuf_len); \
		sph_ ## cname ## _close(&mc, res); \
	} else { \
		sph_ ## cname(&mc, dbuf, dbuf_len - 1); \
		sph_ ## cname ## _addbits_and_close(&mc, \
			dbuf[dbuf_len - 1], extra, res); \
	} \
	utest_strtobin(ref, refres); \
	ASSERT(utest_byteequal(res, ref, blen)); \
	memset(res, 0, sizeof res); \
	memmove(1 + dbuf, dbuf, dbuf_len); \
	if (extra == 0) { \
		sph_ ## cname(&mc, dbuf + 1, dbuf_len); \
		sph_ ## cname ## _close(&mc, res); \
	} else { \
		sph_ ## cname(&mc, dbuf + 1, dbuf_len - 1); \
		sph_ ## cname ## _addbits_and_close(&mc, \
			dbuf[dbuf_len], extra, res); \
	} \
	ASSERT(utest_byteequal(res, ref, blen)); \
}

/*
 * This macro hashes a 1000000-byte message where all bytes are equal to
 * 0x61 (the 'a' lowercase letter, in ASCII). Output is compared to the
 * provided reference string (hexadecimal representation of the expected
 * binary output). Hashing is done twice, in order to exercise functions
 * with misaligned input.
 */
#define KAT_MILLION_A(Name, cname, blen, refres)   do { \
		sph_ ## cname ## _context sc; \
		unsigned char buf[1001], res[blen], ref[blen]; \
		int i; \
 \
		memset(buf, 'a', sizeof buf); \
		sph_ ## cname ## _init(&sc); \
		for (i = 0; i < 1000; i ++) \
			sph_ ## cname(&sc, buf, 1000); \
		sph_ ## cname ## _close(&sc, res); \
		utest_strtobin(ref, refres); \
		ASSERT(utest_byteequal(res, ref, blen)); \
		for (i = 0; i < 1000; i ++) \
			sph_ ## cname(&sc, buf + 1, 1000); \
		sph_ ## cname ## _close(&sc, res); \
		ASSERT(utest_byteequal(res, ref, blen)); \
	} while (0)

/*
 * This macro hashes two (distinct) messages which are provided as
 * hexadecimal strings, and verifies that they yield the same output.
 * This is used to check published collisions for those algorithms
 * which have been successfully attacked. Both messages must fit in
 * 4 kilobytes each.
 */
#define TEST_COLLISION(Name, cname, blen, str1, str2)   do { \
		sph_ ## cname ## _context sc; \
		unsigned char msg1[4096], msg2[4096]; \
		unsigned char res1[blen], res2[blen]; \
		size_t msg1_len, msg2_len; \
		msg1_len = utest_strtobin(msg1, str1); \
		msg2_len = utest_strtobin(msg2, str2); \
		ASSERT((msg1_len != msg2_len) \
			|| !utest_byteequal(msg1, msg2, msg1_len)); \
		sph_ ## cname ## _init(&sc); \
		sph_ ## cname(&sc, msg1, msg1_len); \
		sph_ ## cname ## _close(&sc, res1); \
		sph_ ## cname(&sc, msg2, msg2_len); \
		sph_ ## cname ## _close(&sc, res2); \
		ASSERT(utest_byteequal(res1, res2, blen)); \
	} while (0)
