/* Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_DMA_MAPPING_FAST_H
#define __LINUX_DMA_MAPPING_FAST_H

#define CONFIG_IOMMU_IO_PGTABLE_FAST

#include <linux/iommu.h>
#include <linux/io-pgtable-fast.h>

struct dma_iommu_mapping;

struct dma_fast_smmu_mapping {
	struct device		*dev;
	struct iommu_domain	*domain;
	dma_addr_t	 base;
	size_t		 size;
	size_t		 num_4k_pages;

	unsigned int	bitmap_size;
	unsigned long	*bitmap;
	unsigned long	next_start;
	unsigned long	upcoming_stale_bit;
	bool		have_stale_tlbs;

	dma_addr_t	pgtbl_dma_handle;
	av8l_fast_iopte	*pgtbl_pmds;

	spinlock_t	lock;
	struct notifier_block notifier;
};

#ifdef CONFIG_IOMMU_IO_PGTABLE_FAST
int fast_smmu_attach_device(struct device *dev,
			    struct dma_iommu_mapping *mapping);
void fast_smmu_detach_device(struct device *dev,
			     struct dma_iommu_mapping *mapping);
extern const struct dma_map_ops fast_smmu_dma_ops;
#else
static inline int fast_smmu_attach_device(struct device *dev,
					  struct dma_iommu_mapping *mapping)
{
	return -ENODEV;
}

static inline void fast_smmu_detach_device(struct device *dev,
					   struct dma_iommu_mapping *mapping)
{
}
#endif

#endif /* __LINUX_DMA_MAPPING_FAST_H */
