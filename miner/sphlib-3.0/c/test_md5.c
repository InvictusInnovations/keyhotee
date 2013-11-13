/* $Id: test_md5.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the MD5 hash function.
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

#include "sph_md5.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(MD5, md5, 16)

static void
test_md5(void)
{
	test_md5_internal("", "d41d8cd98f00b204e9800998ecf8427e");
	test_md5_internal("a", "0cc175b9c0f1b6a831c399e269772661");
	test_md5_internal("abc", "900150983cd24fb0d6963f7d28e17f72");
	test_md5_internal("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
	test_md5_internal("abcdefghijklmnopqrstuvwxyz",
		"c3fcd3d76192e4007dfb496cca67e13b");
	test_md5_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstu"
		"vwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f");
	test_md5_internal("1234567890123456789012345678901234567890123456789"
		"0123456789012345678901234567890",
		"57edf4a22be3c955ac49da2e2107b67a");

	KAT_MILLION_A(MD5, md5, 16,
		"7707d6ae4e027c70eea2a935c2296f21");

	TEST_COLLISION(MD5, md5, 16,
		"d131dd02c5e6eec4693d9a0698aff95c2fcab58712467eab4004583eb8f"
		"b7f8955ad340609f4b30283e488832571415a085125e8f7cdc99fd91dbd"
		"f280373c5b960b1dd1dc417b9ce4d897f45a6555d535739ac7f0ebfd0c3"
		"029f166d109b18f75277f7930d55ceb22e8adba79cc155ced74cbdd5fc5"
		"d36db19b0ad835cca7e3",
		"d131dd02c5e6eec4693d9a0698aff95c2fcab50712467eab4004583eb8f"
		"b7f8955ad340609f4b30283e4888325f1415a085125e8f7cdc99fd91dbd"
		"7280373c5b960b1dd1dc417b9ce4d897f45a6555d535739a47f0ebfd0c3"
		"029f166d109b18f75277f7930d55ceb22e8adba794c155ced74cbdd5fc5"
		"d36db19b0a5835cca7e3");

	TEST_COLLISION(MD5, md5, 16,
		"d131dd02c5e6eec4693d9a0698aff95c2fcab58712467eab4004583eb8f"
		"b7f8955ad340609f4b30283e488832571415a085125e8f7cdc99fd91dbd"
		"f280373c5bd8823e3156348f5bae6dacd436c919c6dd53e2b487da03fd0"
		"2396306d248cda0e99f33420f577ee8ce54b67080a80d1ec69821bcb6a8"
		"839396f9652b6ff72a70",
		"d131dd02c5e6eec4693d9a0698aff95c2fcab50712467eab4004583eb8f"
		"b7f8955ad340609f4b30283e4888325f1415a085125e8f7cdc99fd91dbd"
		"7280373c5bd8823e3156348f5bae6dacd436c919c6dd53e23487da03fd0"
		"2396306d248cda0e99f33420f577ee8ce54b67080280d1ec69821bcb6a8"
		"839396f965ab6ff72a70");
}

UTEST_MAIN("MD5", test_md5)
