/* $Id: utest.h 192 2010-05-25 22:33:34Z tp $ */
/**
 * Functions for unit tests.
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
 * @file     utest.h
 * @author   Thomas Pornin <thomas.pornin@cryptolog.com>
 */

#ifndef UTEST_H__
#define UTEST_H__

#include <stddef.h>

/**
 * Set the current name (used for success and failure reports). The
 * provided pointer is recorded as-is; the pointed-to string must not be
 * modified afterwards, until a new name is set.
 *
 * @param name   the current name
 */
void utest_setname(char *name);

/**
 * Fail with an explicit message. Message format and argument use the same
 * conventions than <code>printf()</code>. This function does not return.
 *
 * @param fmt   the explicit message (format)
 */
void fail(char *fmt, ...);

/** @hideinitializer
 * Assert an expression to be true. If the expression does not evaluate
 * to a boolean "true" value, then the test fails with an explicit message
 * which contains the string representation of the expression (the program
 * is exited).
 *
 * @param expr   the expression to test
 */
#define ASSERT(expr)   do { \
		if (!(expr)) \
			fail("assertion failed (%s:%ld): %s", \
				__FILE__, (unsigned long)__LINE__, #expr); \
	} while (0)

/**
 * Convert an hexadecimal string into bytes. The string characters are
 * read in sequence; non-hex digits are skipped silently. And hexadecimal
 * digit is either a decimal digit or a letter between A and F, inclusive
 * (both uppercase and lowercase letters are accepted). If the number of
 * hexadecimal digits in the string is odd, then the function fails: the
 * program exists with an explicit message. The destination buffer MUST be
 * wide enough to accomodate the resulting byte stream.
 *
 * @param dst   the destination buffer
 * @param src   the source string
 * @return  the message length (in bytes)
 */
size_t utest_strtobin(void *dst, char *src);

/**
 * Compare to arrays of bytes for equality. Returned value is 1 if the
 * two arrays are equal, 0 otherwise.
 *
 * @param d1    the first array
 * @param d2    the second array
 * @param len   the common array length (in bytes)
 * @return  1 on equality, 0 otherwise
 */
int utest_byteequal(void *d1, void *d2, size_t len);

/**
 * Print out some bytes in hexadecimal. Uppercase letters are used, with
 * no leading, trailing or separating extra character.
 *
 * @param src   the buffer address
 * @param len   the buffer length (in bytes)
 */
void utest_printarray(void *src, size_t len);

/**
 * Report success. A message is printed, which contains the current test
 * name.
 */
void utest_success(void);

/** @hideinitializer
 * This macro defines a <code>main()</code> function which runs the
 * provided <code>tfun()</code> function (no parameter) and then reports
 * test success. The current test name is set to <code>name</code>, which
 * must evaluate to a string pointer (a literal string is fine).
 *
 * @param name   the current test name (string)
 * @param tfun   the test function
 */
#define UTEST_MAIN(name, tfun) \
int main(void) \
{ \
	utest_setname(name); \
	tfun(); \
	utest_success(); \
	return 0; \
}

/**
 * Get a pointer to one of the NIST short messages, defined for the SHA-3
 * test vectors. Those messages are indexed by bit length, from 0 to
 * 2047, inclusive.
 *
 * @param blen   the message length (in bits)
 * @return  the test message
 */
const void *utest_nist_data(unsigned blen);

#endif
