/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995, 96, 97, 98, 99, 2001 by Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2001 Thiemo Seufer.
 * Copyright (C) 2002 Maciej W. Rozycki
 * Copyright (C) 2014 Imagination Technologies Ltd.
 */
#ifndef _ASM_CHECKSUM_H
#define _ASM_CHECKSUM_H

#ifdef CONFIG_GENERIC_CSUM
#include <asm-generic/checksum.h>
#else

#include <linux/in6.h>

#include <asm/uaccess.h>

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
__wsum csum_partial(const void *buff, int len, __wsum sum);

__wsum __csum_partial_copy_kernel(const void *src, void *dst,
				  int len, __wsum sum, int *err_ptr);

__wsum __csum_partial_copy_from_user(const void *src, void *dst,
				     int len, __wsum sum, int *err_ptr);
__wsum __csum_partial_copy_to_user(const void *src, void *dst,
				   int len, __wsum sum, int *err_ptr);
/*
 * this is a new version of the above that records errors it finds in *errp,
 * but continues and zeros the rest of the buffer.
 */
static inline
__wsum csum_partial_copy_from_user(const void __user *src, void *dst, int len,
				   __wsum sum, int *err_ptr)
{
	might_fault();
	if (segment_eq(get_fs(), get_ds()))
		return __csum_partial_copy_kernel((__force void *)src, dst,
						  len, sum, err_ptr);
	else
		return __csum_partial_copy_from_user((__force void *)src, dst,
						     len, sum, err_ptr);
}

#define _HAVE_ARCH_COPY_AND_CSUM_FROM_USER
static inline
__wsum csum_and_copy_from_user(const void __user *src, void *dst,
			       int len, __wsum sum, int *err_ptr)
{
	if (access_ok(VERIFY_READ, src, len))
		return csum_partial_copy_from_user(src, dst, len, sum,
						   err_ptr);
	if (len)
		*err_ptr = -EFAULT;

	return sum;
}

/*
 * Copy and checksum to user
 */
#define HAVE_CSUM_COPY_USER
static inline
__wsum csum_and_copy_to_user(const void *src, void __user *dst, int len,
			     __wsum sum, int *err_ptr)
{
	might_fault();
	if (access_ok(VERIFY_WRITE, dst, len)) {
		if (segment_eq(get_fs(), get_ds()))
			return __csum_partial_copy_kernel(src,
							  (__force void *)dst,
							  len, sum, err_ptr);
		else
			return __csum_partial_copy_to_user(src,
							   (__force void *)dst,
							   len, sum, err_ptr);
	}
	if (len)
		*err_ptr = -EFAULT;

	return (__force __wsum)-1; /* invalid checksum */
}

/*
 * the same as csum_partial, but copies from user space (but on MIPS
 * we have just one address space, so this is identical to the above)
 */
__wsum csum_partial_copy_nocheck(const void *src, void *dst,
				       int len, __wsum sum);
#define csum_partial_copy_nocheck csum_partial_copy_nocheck

/*
 *	Fold a partial checksum without adding pseudo headers
 */
static inline __sum16 csum_fold(__wsum csum)
{
	u32 sum = (__force u32)csum;;

	sum += (sum << 16);
	csum = (sum < csum);
	sum >>= 16;
	sum += csum;

	return (__force __sum16)~sum;
}
#define csum_fold csum_fold

/*
 *	This is a version of ip_compute_csum() optimized for IP headers,
 *	which always checksum on 4 octet boundaries.
 *
 *	By Jorge Cwik <jorge@laser.satlink.net>, adapted for linux by
 *	Arnt Gulbrandsen.
 */
static inline __sum16 ip_fast_csum(const void *iph, unsigned int ihl)
{
	const unsigned int *word = iph;
	const unsigned int *stop = word + ihl;
	unsigned int csum;
	int carry;
	unsigned int w;

	csum = net_hdr_word(word++);

	w = net_hdr_word(word++);
	csum += w;
	carry = (csum < w);
	csum += carry;

	w = net_hdr_word(word++);
	csum += w;
	carry = (csum < w);
	csum += carry;

	w = net_hdr_word(word++);
	csum += w;
	carry = (csum < w);
	csum += carry;

	do {
		w = net_hdr_word(word++);
		csum += w;
		carry = (csum < w);
		csum += carry;
	} while (word != stop);

	return csum_fold(csum);
}
#define ip_fast_csum ip_fast_csum

static inline __wsum csum_tcpudp_nofold(__be32 saddr,
	__be32 daddr, unsigned short len, unsigned short proto,
	__wsum sum)
{
	__asm__(
	"	.set	push		# csum_tcpudp_nofold\n"
	"	.set	noat		\n"
#ifdef CONFIG_32BIT
	"	addu	%0, %2		\n"
	"	sltu	$1, %0, %2	\n"
	"	addu	%0, $1		\n"

	"	addu	%0, %3		\n"
	"	sltu	$1, %0, %3	\n"
	"	addu	%0, $1		\n"

	"	addu	%0, %4		\n"
	"	sltu	$1, %0, %4	\n"
	"	addu	%0, $1		\n"
#endif
#ifdef CONFIG_64BIT
	"	daddu	%0, %2		\n"
	"	daddu	%0, %3		\n"
	"	daddu	%0, %4		\n"
	"	dsll32	$1, %0, 0	\n"
	"	daddu	%0, $1		\n"
	"	sltu	$1, %0, $1	\n"
	"	dsra32	%0, %0, 0	\n"
	"	addu	%0, $1		\n"
#endif
	"	.set	pop"
	: "=r" (sum)
	: "0" ((__force unsigned long)daddr),
	  "r" ((__force unsigned long)saddr),
#ifdef __MIPSEL__
	  "r" ((proto + len) << 8),
#else
	  "r" (proto + len),
#endif
	  "r" ((__force unsigned long)sum));

	return sum;
}
#define csum_tcpudp_nofold csum_tcpudp_nofold

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */
static inline __sum16 ip_compute_csum(const void *buff, int len)
{
	return csum_fold(csum_partial(buff, len, 0));
}

#include <asm-generic/checksum.h>
#endif /* CONFIG_GENERIC_CSUM */

#endif /* _ASM_CHECKSUM_H */
