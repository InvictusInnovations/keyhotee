/* $Id: test_sha2big.c 216 2010-06-08 09:46:57Z tp $ */
/*
 * Unit tests for the SHA-384 and SHA-512 hash functions.
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

#if SPH_64

TEST_DIGEST_INTERNAL(SHA-384, sha384, 48)
TEST_DIGEST_INTERNAL_BITS(SHA-384, sha384, 48)
TEST_DIGEST_INTERNAL(SHA-512, sha512, 64)
TEST_DIGEST_INTERNAL_BITS(SHA-512, sha512, 64)

static void
test_sha3(void)
{
	test_sha384_internal("abc",
		"cb00753f45a35e8bb5a03d699ac65007272c32ab0eded163"
		"1a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7");
	test_sha384_internal(
		"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
		"hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
		"09330c33f71147e83d192fc782cd1b4753111b173b3b05d2"
		"2fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039");

	KAT_MILLION_A(SHA-384, sha384, 48,
		"9d0e1809716474cb086e834e310a4a1ced149e9c00f24852"
		"7972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985");

	test_sha512_internal("abc",
   "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
   "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
	test_sha512_internal(
		"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
		"hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
   "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
   "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909");

	KAT_MILLION_A(SHA-512, sha512, 64,
   "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
   "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b");

	test_sha384_internal_bits("10", 3,
		"8d17be79e32b6718e07d8a603eb84ba0478f7fcfd1bb9399"
		"5f7d1149e09143ac1ffcfc56820e469f3878d957a15a3fe4");
	test_sha384_internal_bits("b9", 0,
		"bc8089a19007c0b14195f4ecc74094fec64f01f90929282c2fb392881578208ad466828b1c6c283d2722cf0ad1ab6938");
	test_sha384_internal_bits("8bc500c77ceed9879da989107ce0aaa0", 5,
		"d8c43b38e12e7c42a7c9b810299fd6a770bef30920f17532"
		"a898de62c7a07e4293449c0b5fa70109f0783211cfc4bce3");
	test_sha384_internal_bits("a41c497779c0375ff10a7f4e08591739", 0,
		"c9a68443a005812256b8ec76b00516f0dbb74fab26d66591"
		"3f194b6ffb0e91ea9967566b58109cbc675cc208e4c823f7");
	test_sha384_internal_bits("68f501792dea9796767022d93da71679309920f"
		"a1012aea357b2b1331d40a1d03c41c240b3c9a75b4892f4c0724b68c8"
		"75321ab8cfe5023bd375bc0f94bd89fe04f297105d7b82ffc0021aeb1"
		"ccb674f5244ea3497de26a4191c5f62e5e9a2d8082f0551f4a5306826"
		"e91cc006ce1bf60ff719d42fa521c871cd2394d96ef4468f21966b41f"
		"2ba80c26e83a9e0", 5,
		"5860e8de91c21578bb4174d227898a98e0b45c4c760f0095"
		"49495614daedc0775d92d11d9f8ce9b064eeac8dafc3a297");

	test_sha512_internal_bits("b0", 3,
		"d4ee29a9e90985446b913cf1d1376c836f4be2c1cf3c"
		"ada0720a6bf4857d886a7ecb3c4e4c0fa8c7f95214e4"
		"1dc1b0d21b22a84cc03bf8ce4845f34dd5bdbad4");
	test_sha512_internal_bits("d0", 0,
		"9992202938e882e73e20f6b69e68a0a7149090423d93"
		"c81bab3f21678d4aceeee50e4e8cafada4c85a54ea83"
		"06826c4ad6e74cece9631bfa8a549b4ab3fbba15");
	test_sha512_internal_bits("08ecb52ebae1f7422db62bcd54267080", 5,
		"ed8dc78e8b01b69750053dbb7a0a9eda0fb9e9d292b1"
		"ed715e80a7fe290a4e16664fd913e85854400c5af05e"
		"6dad316b7359b43e64f8bec3c1f237119986bbb6");
	test_sha512_internal_bits("8d4e3c0e3889191491816e9d98bff0a0", 0,
		"cb0b67a4b8712cd73c9aabc0b199e9269b20844afb75"
		"acbdd1c153c9828924c3ddedaafe669c5fdd0bc66f63"
		"0f6773988213eb1b16f517ad0de4b2f0c95c90f8");
	test_sha512_internal_bits("3addec85593216d1619aa02d9756970bfc70ace"
		"2744f7c6b2788151028f7b6a2550fd74a7e6e69c2c9b45fc454966dc3"
		"1d2e10da1f95ce02beb4bf8765574cbd6e8337ef420adc98c15cb6d5e"
		"4a0241ba0046d250e510231cac2046c991606ab4ee4145bee2ff4bb12"
		"3aab498d9d44794f99ccad89a9a1621259eda70a5b6dd4bdd87778c90"
		"43b9384f5490680", 5,
		"32ba76fc30eaa0208aeb50ffb5af1864fdbf17902a4d"
		"c0a682c61fcea6d92b783267b21080301837f59de79c"
		"6b337db2526f8a0a510e5e53cafed4355fe7c2f1");
}

UTEST_MAIN("SHA-384 / SHA-512", test_sha3)

#else

#include <stdio.h>

int
main(void)
{
	fprintf(stderr, "warning: SHA-384 and SHA-512"
		" are not supported (no 64-bit type)\n");
	return 0;
}

#endif
