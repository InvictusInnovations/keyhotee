/* $Id: test_tiger.c 216 2010-06-08 09:46:57Z tp $ */
/*
 * Unit tests for the Tiger and Tiger2 hash functions.
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

#include "sph_tiger.h"
#include "test_digest_helper.c"

#if SPH_64

TEST_DIGEST_INTERNAL(Tiger, tiger, 24)
TEST_DIGEST_INTERNAL(Tiger2, tiger2, 24)

static void
test_tiger(void)
{
	test_tiger_internal("",
		"3293AC630C13F0245F92BBB1766E16167A4E58492DDE73F3");
	test_tiger_internal("a",
		"77BEFBEF2E7EF8AB2EC8F93BF587A7FC613E247F5F247809");
	test_tiger_internal("abc",
		"2AAB1484E8C158F2BFB8C5FF41B57A525129131C957B5F93");
	test_tiger_internal("message digest",
		"D981F8CB78201A950DCF3048751E441C517FCA1AA55A29F6");
	test_tiger_internal("abcdefghijklmnopqrstuvwxyz",
		"1714A472EEE57D30040412BFCC55032A0B11602FF37BEEE9");
	test_tiger_internal(
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"0F7BF9A19B9C58F2B7610DF7E84F0AC3A71C631E7B53F78E");
	test_tiger_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789",
		"8DCEA680A17583EE502BA38A3C368651890FFBCCDC49A8CC");
	test_tiger_internal("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890",
		"1C14795529FD9F207A958F84C52F11E887FA0CABDFD91BFD");

	KAT_MILLION_A(Tiger, tiger, 24,
		"6DB0E2729CBEAD93D715C6A7D36302E9B3CEE0D2BC314B41");

	test_tiger2_internal("",
		"4441BE75F6018773C206C22745374B924AA8313FEF919F41");
	test_tiger2_internal("a",
		"67E6AE8E9E968999F70A23E72AEAA9251CBC7C78A7916636");
	test_tiger2_internal("abc",
		"F68D7BC5AF4B43A06E048D7829560D4A9415658BB0B1F3BF");
	test_tiger2_internal("message digest",
		"E29419A1B5FA259DE8005E7DE75078EA81A542EF2552462D");
	test_tiger2_internal("abcdefghijklmnopqrstuvwxyz",
		"F5B6B6A78C405C8547E91CD8624CB8BE83FC804A474488FD");
	test_tiger2_internal(
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"A6737F3997E8FBB63D20D2DF88F86376B5FE2D5CE36646A9");
	test_tiger2_internal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789",
		"EA9AB6228CEE7B51B77544FCA6066C8CBB5BBAE6319505CD");
	test_tiger2_internal("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890",
		"D85278115329EBAA0EEC85ECDC5396FDA8AA3A5820942FFF");

	KAT_MILLION_A(Tiger2, tiger2, 24,
		"E068281F060F551628CC5715B9D0226796914D45F7717CF4");
}

UTEST_MAIN("Tiger / Tiger2", test_tiger)

#else

#include <stdio.h>

int
main(void)
{
	fprintf(stderr, "warning: Tiger is not supported (no 64-bit type)\n");
	return 0;
}

#endif
