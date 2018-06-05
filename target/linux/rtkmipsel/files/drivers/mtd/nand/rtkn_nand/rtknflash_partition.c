
#ifndef CONFIG_WRT_BARRIER_BREAKER

#if defined(CONFIG_RTK_NAND_FLASH_STORAGE)
#define NAND_FLASH_SIZE		(0x6400000)		// max value= %90 * flash size
#endif

#ifndef __UBOOT__
#if 0//def CONFIG_MTD_CMDLINE_PARTS
static const char *probes[] = { "cmdlinepart", NULL };
#else
#if !defined(CONFIG_RTK_NAND_FLASH_STORAGE)
#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
	{
            name: 			"boot",
            size:           0x500000,
            offset:         0x00000000,
    },
    {
            name:           "setting",
            size:			0x300000,
            offset:			0x500000,
    },
    {
            name: 			"linux",
            size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET - 0x800000),
            offset:         0x800000,
    },
    {
            name: 			"rootfs",
            size:           (CONFIG_RTL_FLASH_SIZE - CONFIG_RTL_ROOT_IMAGE_OFFSET),
            offset:         CONFIG_RTL_ROOT_IMAGE_OFFSET,
    }
    /* add new partition */
#if 0
    ,
    {
			name:			"test",
			size:			0x200000,
			offset:			CONFIG_RTL_FLASH_SIZE-0x200000,
    }
    ,
    {
			name:			"test2",
			size:			0x200000,
			offset:			CONFIG_RTL_FLASH_SIZE-0x200000,
    }
#endif
};
#else //!CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
static struct mtd_partition rtl8196_parts1[] = {
    {
            name: 			"boot",
            size:           0x500000,
            offset:         0x00000000,
    },
    {
            name:           "setting",
            size:        	0x300000,
            offset:         0x500000,
    },
    {
            name: 			"linux",
            size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0x800000),
            offset:         0x800000,
    },
    {
            name:           "rootfs",
            size:        	(CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET-CONFIG_RTL_ROOT_IMAGE_OFFSET),
            offset:         CONFIG_RTL_ROOT_IMAGE_OFFSET,
    },
    {
            name:           "setting2",
            size:        	0x300000,
            offset:         (CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET+0x500000),
    },
    {
            name: 			"linux2",
            size:           (CONFIG_RTL_ROOT_IMAGE_OFFSET-0x800000),
            offset:         (CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET+0x800000),
    },
    {
            name:           "rootfs2",
            size:        	(CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET-CONFIG_RTL_ROOT_IMAGE_OFFSET),
            offset:         (CONFIG_RTL_FLASH_DUAL_IMAGE_OFFSET+CONFIG_RTL_ROOT_IMAGE_OFFSET),
    }

};
#endif
#else
	//defined CONFIG_RTK_NAND_FLASH_STORAGE
	static struct mtd_partition rtl8196_parts1[] = {
	{
            name: 			"boot",
            size:           0x500000,
            offset:         0x00000000,
    }
    ,
	{
            name: 			"storage",
            size:           (NAND_FLASH_SIZE-0x500000),
            offset:         0x500000,
    }
};
#endif
#endif
#endif

#if !defined(CONFIG_RTK_NAND_FLASH_STORAGE)
#if !defined(CONFIG_MTD_RTKXX_PARTS)
static int detect_rootfs_flash_map(void)
{
	int i;
	unsigned int size = 0;
	int dualpartition  = 0;

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE

	for(i = 0; i < ARRAY_SIZE(rtl8196_parts1);i++){
		if(!strcmp(rtl8196_parts1[i].name,"setting2"))
			dualpartition = i;
	}
#else
	dualpartition = ARRAY_SIZE(rtl8196_parts1);
#endif

	for(i = dualpartition - 1; i >= 0;i--){
		if(strcmp(rtl8196_parts1[i].name,"rootfs")){
			size += rtl8196_parts1[i].size;
			rtl8196_parts1[i].offset = CONFIG_RTL_FLASH_SIZE - size;
		}else{
			if(rtl8196_parts1[i].size < size){
				printk("donnot have space for rootfs partition\n");
				return -1;
			}
			rtl8196_parts1[i].size = rtl8196_parts1[i].size - size;
			break;
		}
	}

#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
	for(i = ARRAY_SIZE(rtl8196_parts1) - 1; i >= dualpartition;i--){
		if(strcmp(rtl8196_parts1[i].name,"rootfs2")){
			size += rtl8196_parts1[i].size;
			rtl8196_parts1[i].offset = CONFIG_RTL_FLASH_SIZE - size;
		}else{
			if(rtl8196_parts1[i].size < size){
				printk("donnot have space for rootfs2 partition\n");
				return -1;
			}
			rtl8196_parts1[i].size = rtl8196_parts1[i].size - size;
			break;
		}
	}
#endif

	return 0;
}
#endif
#endif

#endif
