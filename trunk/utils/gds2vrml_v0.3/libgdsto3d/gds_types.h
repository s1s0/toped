/*
 * File: gds_types.h
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _GDS_TYPES_H
#define _GDS_TYPES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/*
 * We need byte swapping functions.
 * If these exist for a particular platform, use the (presumably) optimised
 * versions provided. Otherwise, use the network functions htonl() and htons().
 */

#if defined(linux)
#	include <endian.h>
#	include <byteswap.h>
#	define endian_swap_long(A) bswap_32((A))
#	define endian_swap_short(A) bswap_16((A))
#elif defined(WIN32)
//#	define __LITTLE_ENDIAN 1234
//#	define __BIG_ENDIAN 4321
//#	define __BYTE_ORDER __LITTLE_ENDIAN
//	long endian_swap_long(long value);
//	short endian_swap_short(short value);
#ifdef HAVE_WINSOCK2_H
#	include <winsock2.h>
#	define endian_swap_long(A) htonl((A))
#	define endian_swap_short(A) htons((A))
#endif
#else
	/* Generic includes for Unix-alike systems */
	/* Includes at least Solaris, FreeBSD, HP-UX */
#	include <arpa/nameser_compat.h>
#	include <netinet/in.h>
#	define endian_swap_long(A) htonl((A))
#	define endian_swap_short(A) htons((A))
#endif

#endif

