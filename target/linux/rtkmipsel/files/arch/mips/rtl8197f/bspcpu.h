/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspcpu.h
 *     bsp cpu and memory configuration file
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#ifndef _BSPCPU_H_
#define _BSPCPU_H_

/*
 * scache => L2 cache
 * dcache => D cache
 * icache => I cache
 */
#define cpu_icache_size     (64 << 10)
#define cpu_dcache_size     (32 << 10)
#define cpu_scache_size     0
#define cpu_icache_line     32
#define cpu_dcache_line     32
#define cpu_scache_line     0
#define cpu_tlb_entry       64

#define cpu_mem_size        (32 << 20)

#define cpu_imem_size       0
#define cpu_dmem_size       0
#define cpu_smem_size       0

#endif /* _BSPCPU_H_ */
