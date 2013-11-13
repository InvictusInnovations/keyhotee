/* $Id: test_sha0.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the SHA-0 hash function.
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

#include "sph_sha0.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(SHA-0, sha0, 20)

static void
test_sha0(void)
{
	test_sha0_internal("abc", "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880");
	test_sha0_internal("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlm"
		"nomnopnopq", "d2516ee1acfa5baf33dfc1c471e438449ef134c8");

	KAT_MILLION_A(SHA-0, sha0, 20,
		"3232affa48628a26653b5aaa44541fd90d690603");
}

UTEST_MAIN("SHA-0", test_sha0)
