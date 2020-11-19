/*	Id: inet_pton.c,v 1.5 2001/04/13 15:24:35 lukem Exp 	*/
/*	$NetBSD: inet_pton.c,v 1.16 2000/02/07 18:51:02 itojun Exp $	*/

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <net-snmp-config.h>

#include <ctype.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <types.h>
#include "inet_pton.h"

#ifndef IN6ADDRSZ
#define	IN6ADDRSZ	16
#endif

#ifndef INT16SZ
#define	INT16SZ		2
#endif

#ifndef INADDRSZ
#define	INADDRSZ	4
#endif

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int	inet_pton4(const char *src, u_char *dst, int pton);

/* int
 * inet_pton(af, src, dst)
 *	convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * return:
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *	Paul Vixie, 1996.
 */
int
netsnmp_inet_pton(int af, const char *src, void *dst)
{

	switch (af) {
	case AF_INET:
		return (inet_pton4(src, dst, 1));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
	/* NOTREACHED */
}

/* int
 * inet_pton4(src, dst, pton)
 *	when last arg is 0: inet_aton(). with hexadecimal, octal and shorthand.
 *	when last arg is 1: inet_pton(). decimal dotted-quad only.
 * return:
 *	1 if `src' is a valid input, else 0.
 * notice:
 *	does not touch `dst' unless it's returning 1.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton4(const char *src, u_char *dst, int pton)
{
	u_int val;
	u_int digit;
	int base, n;
	unsigned char c;
	u_int parts[4];
	register u_int *pp = parts;

	c = *src;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++src;
			if (c == 'x' || c == 'X')
				base = 16, c = *++src;
			else if (isdigit(c) && c != '9')
				base = 8;
		}
		/* inet_pton() takes decimal only */
		if (pton && base != 10)
			return (0);
		for (;;) {
			if (isdigit(c)) {
				digit = c - '0';
				if ((int)digit >= base)
					break;
				val = (val * base) + digit;
				c = *++src;
			} else if (base == 16 && isxdigit(c)) {
				digit = c + 10 - (islower(c) ? 'a' : 'A');
				if (digit >= 16)
					break;
				val = (val << 4) | digit;
				c = *++src;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 *	a	(with a treated as 32 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++src;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && !isspace(c))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	/* inet_pton() takes dotted-quad only.  it does not take shorthand. */
	if (pton && n != 4)
		return (0);
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (parts[0] > 0xff || val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if ((parts[0] | parts[1]) > 0xff || val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((parts[0] | parts[1] | parts[2] | val) > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (dst) {
		val = htonl(val);
		memcpy(dst, &val, INADDRSZ);
	}
	return (1);
}