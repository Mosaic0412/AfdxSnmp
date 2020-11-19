/*	Id: inet_ntop.c,v 1.4 2001/04/17 07:53:47 lukem Exp 	*/
/*	$NetBSD: inet_ntop.c,v 1.9 2000/01/22 22:19:16 mycroft Exp $	*/

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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <types.h>

#include "inet_ntop.h"

#ifndef IN6ADDRSZ
#define	IN6ADDRSZ	16
#endif

#ifndef INT16SZ
#define	INT16SZ		2
#endif

# define SPRINTF(x) ((size_t)sprintf x)

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static const char *inet_ntop4(const u_char *src, char *dst, size_t size);

/* char *
 * inet_ntop(af, src, dst, size)
 *	convert a network format address to presentation format.
 * return:
 *	pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *	Paul Vixie, 1996.
 */
const char *
netsnmp_inet_ntop(int af, const void *src, char *dst, size_t size)
{

	switch (af) {
	case AF_INET:
		return (inet_ntop4(src, dst, size));
	default:
		errno = EAFNOSUPPORT;
		return (NULL);
	}
	/* NOTREACHED */
}

/* const char *
 * inet_ntop4(src, dst, size)
 *	format an IPv4 address, more or less like inet_ntoa()
 * return:
 *	`dst' (as a const)
 * notes:
 *	(1) uses no statics
 *	(2) takes a u_char* not an in_addr as input
 * author:
 *	Paul Vixie, 1996.
 */
static const char *
inet_ntop4(const u_char *src, char *dst, size_t size)
{
	static const char fmt[] = "%u.%u.%u.%u";
	char tmp[sizeof "255.255.255.255"];

	if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size) {
		errno = ENOSPC;
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}
