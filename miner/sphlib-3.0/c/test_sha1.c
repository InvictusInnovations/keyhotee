/* $Id: test_sha1.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the SHA-1 hash function.
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

#include "sph_sha1.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(SHA-1, sha1, 20)
TEST_DIGEST_INTERNAL_BITS(SHA-1, sha1, 20)

static void
test_sha1(void)
{
	test_sha1_internal("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
	test_sha1_internal("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlm"
		"nomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1");

	KAT_MILLION_A(SHA-1, sha1, 20,
		"34aa973cd4c4daa4f61eeb2bdbad27316534016f");

	test_sha1_internal_bits("98", 3,
		"29826b003b906e660eff4027ce98af3531ac75ba");
	test_sha1_internal_bits("5e", 0,
		"5e6f80a34a9798cafc6a5db96cc57ba4c4db59c2");
	test_sha1_internal_bits("49b2aec2594bbe3a3b117542d94ac880", 5,
		"6239781e03729919c01955b3ffa8acb60b988340");
	test_sha1_internal_bits("9a7dfdf1ecead06ed646aa55fe757146", 0,
		"82abff6605dbe1c17def12a394fa22a82b544a35");
	test_sha1_internal_bits("65f932995ba4ce2cb1b4a2e71ae70220aacec8962"
		"dd4499cbd7c887a94eaaa101ea5aabc529b4e7e43665a5af2cd03fe67"
		"8ea6a5005bba3b082204c28b9109f469dac92aaab3aa7c11a1b32ae0", 5,
		"8c5b2a5ddae5a97fc7f9d85661c672adbf7933d4");
}

UTEST_MAIN("SHA-1", test_sha1)
