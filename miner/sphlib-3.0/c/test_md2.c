/* $Id: test_md2.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the MD2 hash function.
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

#include "sph_md2.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(MD2, md2, 16)

static void
test_md2(void)
{
	test_md2_internal("", "8350e5a3e24c153df2275c9f80692773");
	test_md2_internal("a", "32ec01ec4a6dac72c0ab96fb34c0b5d1");
	test_md2_internal("abc", "da853b0d3f88d99b30283a69e6ded6bb");
	test_md2_internal("message digest", "ab4f496bfb2a530b219ff33031fe06b0");
	test_md2_internal("abcdefghijklmnopqrstuvwxyz",
		"4e8ddff3650292ab5a4108c3aa47940b");
	test_md2_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstu"
		"vwxyz0123456789", "da33def2a42df13975352846c30338cd");
	test_md2_internal("1234567890123456789012345678901234567890123456789"
		"0123456789012345678901234567890",
		"d5976f79d83d3a0dc9806c3c66f3efd8");

	KAT_MILLION_A(MD2, md2, 16,
		"8c0a09ff1216ecaf95c8130953c62efd");
}

UTEST_MAIN("MD2", test_md2)
