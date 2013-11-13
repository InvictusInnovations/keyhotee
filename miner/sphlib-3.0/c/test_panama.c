/* $Id: test_panama.c 154 2010-04-26 17:00:24Z tp $ */
/*
 * Unit tests for the PANAMA hash function.
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

#include "sph_panama.h"
#include "test_digest_helper.c"

TEST_DIGEST_INTERNAL(PANAMA, panama, 32)

static void
test_panama(void)
{
	test_panama_internal("",
    "aa0cc954d757d7ac7779ca3342334ca471abd47d5952ac91ed837ecd5b16922b");
	test_panama_internal("T",
    "049d698307d8541f22870dfa0a551099d3d02bc6d57c610a06a4585ed8d35ff8");
	test_panama_internal("The quick brown fox jumps over the lazy dog",
    "5f5ca355b90ac622b0aa7e654ef5f27e9e75111415b48b8afe3add1c6b89cba1");

	KAT_MILLION_A(PANAMA, panama, 32,
    "af9c66fb6058e2232a5dfba063ee14b0f86f0e334e165812559435464dd9bb60");
}

UTEST_MAIN("PANAMA", test_panama)
