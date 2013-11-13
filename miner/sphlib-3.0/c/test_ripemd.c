/* $Id: test_ripemd.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the RIPEMD hash functions (original RIPEMD, RIPEMD-128
 * and RIPEMD-160).
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

#include "sph_ripemd.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(RIPEMD, ripemd, 16)
TEST_DIGEST_INTERNAL(RIPEMD-128, ripemd128, 16)
TEST_DIGEST_INTERNAL(RIPEMD-160, ripemd160, 20)

static void
test_ripemd(void)
{
	test_ripemd_internal("",
		"9f73aa9b372a9dacfb86a6108852e2d9");
	test_ripemd_internal("a",
		"486f74f790bc95ef7963cd2382b4bbc9");
	test_ripemd_internal("abc",
		"3f14bad4c2f9b0ea805e5485d3d6882d");
	test_ripemd_internal("message digest",
		"5f5c7ebe1abbb3c7036482942d5f9d49");
	test_ripemd_internal("abcdefghijklmnopqrstuvwxyz",
		"ff6e1547494251a1cca6f005a6eaa2b4");
	test_ripemd_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789",
		"ff418a5aed3763d8f2ddf88a29e62486");
	test_ripemd_internal("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890",
		"dfd6b45f60fe79bbbde87c6bfc6580a5");

	TEST_COLLISION(RIPEMD, ripemd, 16,
		"8eaf9f5779f5ec09ba6a4a5711354178a410b4a29f6c2fad2c20560b"
		"1179754de7aade0bf291bc787d6dbc47b1d1bd9a15205da4ff047181"
		"a8584726a54e0661",
		"8eaf9f5779f5ec09ba6a4a5711355178a410b4a29f6c2fad2c20560b"
		"1179754de7aade0bf291bc787d6dc0c7b1d1bd9a15205da4ff047181"
		"a8584726a54e06e1");

	TEST_COLLISION(RIPEMD, ripemd, 16,
		"8eaf9f5779f5ec09ba6a4a5711354178a410b4a29f6c2fad2c20560b"
		"1179754de7aade0bf291bc787d6dbc47b1d1bd9a15205da4ff04a5a0"
		"a8588db1b6660ce7",
		"8eaf9f5779f5ec09ba6a4a5711355178a410b4a29f6c2fad2c20560b"
		"1179754de7aade0bf291bc787d6dc0c7b1d1bd9a15205da4ff04a5a0"
		"a8588db1b6660c67");

	test_ripemd128_internal("",
		"cdf26213a150dc3ecb610f18f6b38b46");
	test_ripemd128_internal("a",
		"86be7afa339d0fc7cfc785e72f578d33");
	test_ripemd128_internal("abc",
		"c14a12199c66e4ba84636b0f69144c77");
	test_ripemd128_internal("message digest",
		"9e327b3d6e523062afc1132d7df9d1b8");
	test_ripemd128_internal("abcdefghijklmnopqrstuvwxyz",
		"fd2aa607f71dc8f510714922b371834e");
	test_ripemd128_internal(
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"a1aa0689d0fafa2ddc22e88b49133a06");
	test_ripemd128_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789",
		"d1e959eb179c911faea4624c60c5c702");
	test_ripemd128_internal("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890",
		"3f45ef194732c2dbb2c4a2c769795fa3");

	KAT_MILLION_A(RIPEMD-128, ripemd128, 16,
		"4a7f5723f954eba1216c9d8f6320431f");

	test_ripemd160_internal("",
		"9c1185a5c5e9fc54612808977ee8f548b2258d31");
	test_ripemd160_internal("a",
		"0bdc9d2d256b3ee9daae347be6f4dc835a467ffe");
	test_ripemd160_internal("abc",
		"8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
	test_ripemd160_internal("message digest",
		"5d0689ef49d2fae572b881b123a85ffa21595f36");
	test_ripemd160_internal("abcdefghijklmnopqrstuvwxyz",
		"f71c27109c692c1b56bbdceb5b9d2865b3708dbc");
	test_ripemd160_internal(
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"12a053384a9c0c88e405a06c27dcf49ada62eb2b");
	test_ripemd160_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789",
		"b0e20b6e3116640286ed3a87a5713079b21f5189");
	test_ripemd160_internal("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890",
		"9b752e45573d4b39f4dbd3323cab82bf63326bfb");

	KAT_MILLION_A(RIPEMD-160, ripemd160, 20,
		"52783243c1697bdbe16d37f97f68f08325dc1528");
}

UTEST_MAIN("RIPEMD (orignal, -128, -160)", test_ripemd)
