/*
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This file was derived from: inlude/asm-mips/mach-generic/mangle-port.h
 *      Copyright (C) 2003, 2004 Ralf Baechle
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef __ASM_MACH_ATH79_MANGLE_PORT_H
#define __ASM_MACH_ATH79_MANGLE_PORT_H

#ifdef CONFIG_PCI
extern unsigned long (ath79_pci_swizzle_b)(unsigned long port);
extern unsigned long (ath79_pci_swizzle_w)(unsigned long port);
#else
#define ath79_pci_swizzle_b(port) (port)
#define ath79_pci_swizzle_w(port) (port)
#endif

#define __swizzle_addr_b(port)	ath79_pci_swizzle_b(port)
#define __swizzle_addr_w(port)	ath79_pci_swizzle_w(port)
#define __swizzle_addr_l(port)	(port)
#define __swizzle_addr_q(port)	(port)

# define ioswabb(a, x)           (x)
# define __mem_ioswabb(a, x)     (x)
# define ioswabw(a, x)           (x)
# define __mem_ioswabw(a, x)     cpu_to_le16(x)
# define ioswabl(a, x)           (x)
# define __mem_ioswabl(a, x)     cpu_to_le32(x)
# define ioswabq(a, x)           (x)
# define __mem_ioswabq(a, x)     cpu_to_le64(x)

#endif /* __ASM_MACH_ATH79_MANGLE_PORT_H */
