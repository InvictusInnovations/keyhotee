/* $Id: test_md4.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the MD4 hash function.
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

#include "sph_md4.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(MD4, md4, 16)

static void
test_md4(void)
{
	test_md4_internal("", "31d6cfe0d16ae931b73c59d7e0c089c0");
	test_md4_internal("a", "bde52cb31de33e46245e05fbdbd6fb24");
	test_md4_internal("abc", "a448017aaf21d8525fc10ae87aa6729d");
	test_md4_internal("message digest", "d9130a8164549fe818874806e1c7014b");
	test_md4_internal("abcdefghijklmnopqrstuvwxyz",
		"d79e1c308aa5bbcdeea8ed63df412da9");
	test_md4_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstu"
		"vwxyz0123456789", "043f8582f241db351ce627e153e7f0e4");
	test_md4_internal("1234567890123456789012345678901234567890123456789"
		"0123456789012345678901234567890",
		"e33b4ddc9c38f2199c3e7b164fcc0536");

	KAT_MILLION_A(MD4, md4, 16,
		"bbce80cc6bb65e5c6745e30d4eeca9a4");

	TEST_COLLISION(MD4, md4, 16,
		"839c7a4d7a92cb5678a5d5b9eea5a7573c8a74deb366c3dc20a083b69"
		"f5d2a3bb3719dc69891e9f95e809fd7e8b23ba6318edd45e51fe39708"
		"bf9427e9c3e8b9",
		"839c7a4d7a92cbd678a5d529eea5a7573c8a74deb366c3dc20a083b69"
		"f5d2a3bb3719dc69891e9f95e809fd7e8b23ba6318edc45e51fe39708"
		"bf9427e9c3e8b9");
	TEST_COLLISION(MD4, md4, 16,
		"839c7a4d7a92cb5678a5d5b9eea5a7573c8a74deb366c3dc20a083b69"
		"f5d2a3bb3719dc69891e9f95e809fd7e8b23ba6318edd45e51fe39740"
		"c213f769cfb8a7",
		"839c7a4d7a92cbd678a5d529eea5a7573c8a74deb366c3dc20a083b69"
		"f5d2a3bb3719dc69891e9f95e809fd7e8b23ba6318edc45e51fe39740"
		"c213f769cfb8a7");
}

UTEST_MAIN("MD4", test_md4)
