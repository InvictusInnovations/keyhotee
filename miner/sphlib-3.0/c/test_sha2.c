/* $Id: test_sha2.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the SHA-224 and SHA-256 hash functions.
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

#include "sph_sha2.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(SHA-224, sha224, 28)
TEST_DIGEST_INTERNAL_BITS(SHA-224, sha224, 28)
TEST_DIGEST_INTERNAL(SHA-256, sha256, 32)
TEST_DIGEST_INTERNAL_BITS(SHA-256, sha256, 32)

static void
test_sha2(void)
{
	test_sha224_internal("abc",
   "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7");
	test_sha224_internal("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlm"
		"nomnopnopq",
   "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525");

	KAT_MILLION_A(SHA-224, sha224, 28,
		"20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67");

	test_sha256_internal("abc",
   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	test_sha256_internal("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlm"
		"nomnopnopq",
   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

	KAT_MILLION_A(SHA-256, sha256, 32,
   "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

	test_sha224_internal_bits("68", 3,
		"e3b048552c3c387bcab37f6eb06bb79b96a4aee5ff27f51531a9551c");
	test_sha224_internal_bits("07", 0,
		"00ecd5f138422b8ad74c9799fd826c531bad2fcabc7450bee2aa8c2a");
	test_sha224_internal_bits("f07006f25a0bea68cd76a29587c28da0", 5,
		"1b01db6cb4a9e43ded1516beb3db0b87b6d1ea43187462c608137150");
	test_sha224_internal_bits("18804005dd4fbd1556299d6f9d93df62", 0,
		"df90d78aa78821c99b40ba4c966921accd8ffb1e98ac388e56191db1");
	test_sha224_internal_bits("a2be6e463281090294d9ce94826569423a3a305e"
		"d5e2116cd4a4c987fc0657006491b149ccd4b51130ac62b19dc248c744"
		"543d20cd3952dced1f06cc3b18b91f3f55633ecc3085f4907060d2e0", 5,
		"54bea6eab8195a2eb0a7906a4b4a876666300eefbd1f3b8474f9cd57");

	test_sha256_internal_bits("68", 3,
		"d6d3e02a31a84a8caa9718ed6c2057be"
		"09db45e7823eb5079ce7a573a3760f95");
	test_sha256_internal_bits("19", 0,
		"68aa2e2ee5dff96e3355e6c7ee373e3d"
		"6a4e17f75f9518d843709c0c9bc3e3d4");
	test_sha256_internal_bits("be2746c6db52765fdb2f88700f9a7360", 5,
		"77ec1dc89c821ff2a1279089fa091b35"
		"b8cd960bcaf7de01c6a7680756beb972");
	test_sha256_internal_bits("e3d72570dcdd787ce3887ab2cd684652", 0,
		"175ee69b02ba9b58e2b0a5fd13819cea"
		"573f3940a94f825128cf4209beabb4e8");
	test_sha256_internal_bits("3e740371c810c2b99fc04e804907ef7cf26be28b"
		"57cb58a3e2f3c007166e49c12e9ba34c0104069129ea7615642545703a"
		"2bd901e16eb0e05deba014ebff6406a07d54364eff742da779b0b3a0", 5,
		"3e9ad6468bbbad2ac3c2cdc292e018ba"
		"5fd70b960cf1679777fce708fdb066e9");
}

UTEST_MAIN("SHA-224 / SHA-256", test_sha2)
