/**************************openwrt *********************************************/
#ifdef CONFIG_WRT_BARRIER_BREAKER

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE //mark_boot
#define CONFIG_RTK_BOOTINFO_SUPPORT 1
#endif
#define RTK_USERDEFINE_FLASH_BANK_SIZE 0x2800000

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT
#include "rtk_bootinfo.c"
#define FLASH_BOOTINFO_OFFSET	0x300000
#define FLASH_MEM_MAP_ADDR_W	0x0
BOOTINFO_T bootinfo_ram;
#endif

//#define SQUASHFS_MAGIC_LE 		0x68737173
/* for little endian */
#define SQUASHFS_MAGIC_LE		0x73717368
#define FLASH_PROC_DIR_NAME 	"flash"
#define HWPART_NAME 			"hwpart"

unsigned int HW_SETTING_OFFSET = 0x0;
static unsigned int  RTK_LINUX_PART_OFFSET=0x800000;
static struct proc_dir_entry *flash_proc_dir;
static struct proc_dir_entry *hwpart_proc_entry;
static unsigned int  RTK_FLASH_BANK_SIZE = RTK_USERDEFINE_FLASH_BANK_SIZE;

static struct mtd_partition rtl8196_parts1[] = {
      {name: "boot", offset: 0, size:0x500000,},
      {name: "setting", offset: 0x500000, size:0x300000,},
      //{name: "ubifs", offset: 0x100000,	   size:0x200000,},
      {name: "linux", offset: 0x800000,    size:0x600000,}, //0x130000+ rootfs
      {name: "rootfs", offset: 0xe00000, size:0x1A00000,},
      //{name: "rootfs2", offset: 0x3600000,	   size:0x2800000,},
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
      //{name: "boot_backup", offset: 0x400000, size:0x30000,},
      {name: "linux_backup", offset: (CONFIG_RTK_DUALIMAGE_FLASH_OFFSET+0x500000),size:0x2300000,},
#endif
};

#define RTK_ADD_PROC(name, show_fct) \
	static int name##_open(struct inode *inode, struct file *file) \
	{ return single_open(file, show_fct, inode->i_private); }      \
	static const struct file_operations name##_fops = { \
		.owner = THIS_MODULE, \
		.open = name##_open, \
		.llseek = seq_lseek, \
		.read = seq_read, \
		.write = name##_write, \
		.release = single_release \
	};

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT

#if 1//def CONFIG_MTD_NAND
struct mtd_info* rtk_get_nand_mtd(void);
int rtk_nand_erase(struct mtd_info *mtd, loff_t  offs,size_t len);
int rtk_nand_read(struct mtd_info *mtd, uint8_t *buf, loff_t  offs,size_t len);
int rtk_nand_write(struct mtd_info *mtd, loff_t  offs, size_t len,uint8_t *buf);
#endif

void rtk_flash_write_data(unsigned int flash_addr,unsigned int len,unsigned char* data)
{
#if 0//def CONFIG_RTL819X_SPI_FLASH
	extern unsigned int ComSrlCmd_ComWriteData(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)	;
	ComSrlCmd_ComWriteData(0,flash_addr,len,data);
#endif
#if 1//def CONFIG_MTD_NAND
	struct mtd_info* mtd = rtk_get_nand_mtd();
	if(rtk_nand_erase(mtd,flash_addr,len) != 0)
		return;
	/* len must page aligned */
	rtk_nand_write(mtd,flash_addr,mtd->writesize,data);
#endif
}

static void  rtk_init_bootinfo(BOOTINFO_P boot)
{
#if 0//def CONFIG_RTL819X_SPI_FLASH
	rtk_read_bootinfo_from_flash(FLASH_MEM_MAP_ADDR+FLASH_BOOTINFO_OFFSET,boot);
#endif
#if 1//def CONFIG_MTD_NAND
	struct mtd_info* mtd = rtk_get_nand_mtd();
	rtk_nand_read(mtd,boot,FLASH_BOOTINFO_OFFSET,sizeof(BOOTINFO_T));
#endif

	 if(!rtk_check_bootinfo(boot)){ // if not valid bootinfo header use default setting
		printk("do reset \n");
		rtk_reset_bootinfo(boot);
	 }
}

#define BOOTBANK_NAME "bootbank"
#define BOOTMAXCNT_NAME "bootmaxcnt"
#define BOOTCNT_NAME "bootcnt"
#define BOOTMODE_NAME "bootmode"

static struct proc_dir_entry *bootbank_proc_entry;
static struct proc_dir_entry *bootcnt_proc_entry;
static struct proc_dir_entry *bootcntmax_proc_entry;
static struct proc_dir_entry *bootmode_proc_entry;

static int bootbank_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootbank;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootbank);

		if ((num != 1) || ( bootbank > 1) ) {
			printk("Invalid bootbank!\n");
			return num;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootbank = bootbank ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootcnt_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootcnt;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootcnt);

		if ((num != 1) || ( bootcnt > RTK_MAX_VALID_BOOTCNT) ) {
			printk("Invalid bootbank!\n");
			return num;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootcnt = bootcnt ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootcntmax_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootmaxcnt;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootmaxcnt);

		if ((num != 1) || ( bootmaxcnt > RTK_MAX_VALID_BOOTCNT) ) {
			printk("Invalid bootbank!\n");
			return num;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootmaxcnt = bootmaxcnt ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

static int bootmode_proc_write(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char tmp[32];
	unsigned int bootmode;
	unsigned int num;
	BOOTINFO_P bootinfo_p;

	if (buffer && !copy_from_user(tmp, buffer, 32)) {
		num = sscanf(tmp, "%d", &bootmode);

		if ((num != 1)) {
			printk("Invalid bootmode!\n");
			return num;
		}
		bootinfo_p = 	&bootinfo_ram;
		bootinfo_p->data.field.bootmode = bootmode ;

		rtk_write_bootinfo_to_flash(FLASH_MEM_MAP_ADDR_W+FLASH_BOOTINFO_OFFSET,&bootinfo_ram,rtk_flash_write_data);
	}

	return count;

}

#ifdef CONFIG_WRT_BARRIER_BREAKER
static int bootbank_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootbank);
	return pos;
}

static ssize_t bootbank_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootbank_proc_write(file, userbuf,count, off);
}

static int bootcnt_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootcnt);
	return pos;
}

static ssize_t bootcnt_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootcnt_proc_write(file, userbuf,count, off);
}

static int bootcntmax_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootmaxcnt);
	return pos;
}

static ssize_t bootcntmax_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootcntmax_proc_write(file, userbuf,count, off);
}

static int bootmode_show(struct seq_file *s, void *p)
{
	int pos = 0;
	BOOTINFO_P bootinfo_p;
	bootinfo_p = 	&bootinfo_ram;

	pos += seq_printf(s, "%d", bootinfo_p->data.field.bootmode);
	return pos;
}

static ssize_t bootmode_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return bootmode_proc_write(file, userbuf,count, off);
}

//add related proc fops here
RTK_ADD_PROC(bootbank,bootbank_show);
RTK_ADD_PROC(bootcnt,bootcnt_show);
RTK_ADD_PROC(bootcntmax,bootcntmax_show);
RTK_ADD_PROC(bootmode,bootmode_show);

#else //old proc files
static int bootbank_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootbank);
		return len;

}
static int bootcnt_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootcnt);
		return len;

}

static int bootcntmax_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootmaxcnt);
		return len;

}



static int bootmode_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int 	len;
		BOOTINFO_P bootinfo_p;
		bootinfo_p = 	&bootinfo_ram;

		len = sprintf(page, "%d",bootinfo_p->data.field.bootmode);
		return len;

}
#endif


static void  rtk_init_bootinfo_proc(struct proc_dir_entry *flash_proc_dir)
{
	rtk_init_bootinfo(&bootinfo_ram);

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootbank_proc_entry = proc_create(BOOTBANK_NAME,0,flash_proc_dir,&bootbank_fops);
#else
	bootbank_proc_entry = create_proc_entry(BOOTBANK_NAME,0,flash_proc_dir);
	bootbank_proc_entry->read_proc = bootbank_proc_read;
	bootbank_proc_entry->write_proc= bootbank_proc_write;
#endif

	if(!bootbank_proc_entry)
		return;

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootcnt_proc_entry = proc_create(BOOTCNT_NAME,0,flash_proc_dir,&bootcnt_fops);
#else
	bootcnt_proc_entry = create_proc_entry(BOOTCNT_NAME,0,flash_proc_dir);
	bootcnt_proc_entry->read_proc = bootcnt_proc_read;
	bootcnt_proc_entry->write_proc= bootcnt_proc_write;
#endif
	if(!bootcnt_proc_entry)
		return;

#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootcntmax_proc_entry = proc_create(BOOTMAXCNT_NAME,0,flash_proc_dir,&bootcntmax_fops);
#else
	bootcntmax_proc_entry = create_proc_entry(BOOTMAXCNT_NAME,0,flash_proc_dir);
	bootcntmax_proc_entry->read_proc = bootcntmax_proc_read;
	bootcntmax_proc_entry->write_proc= bootcntmax_proc_write;
#endif
	if(!bootcntmax_proc_entry)
		return;
#ifdef CONFIG_WRT_BARRIER_BREAKER
	bootmode_proc_entry = proc_create(BOOTMODE_NAME,0,flash_proc_dir,&bootmode_fops);
#else
	bootmode_proc_entry = create_proc_entry(BOOTMODE_NAME,0,flash_proc_dir);
	bootmode_proc_entry->read_proc = bootmode_proc_read;
	bootmode_proc_entry->write_proc=  bootmode_proc_write;
#endif
	if(!bootmode_proc_entry)
		return;
}
#endif


static int hwpart_show(struct seq_file *s, void *p)
{
	int pos = 0;

	pos += seq_printf(s, "%x", HW_SETTING_OFFSET);
	return pos;
}

static ssize_t hwpart_write(struct file * file, const char __user * userbuf,
                     size_t count, loff_t * off)
{
        return 0;
}

RTK_ADD_PROC(hwpart,hwpart_show);

static void  rtk_init_flash_proc(void)
{
	flash_proc_dir = proc_mkdir(FLASH_PROC_DIR_NAME,NULL);

	if(!flash_proc_dir)
		return;

	//init hwpart proc
	hwpart_proc_entry = proc_create(HWPART_NAME,0,flash_proc_dir,&hwpart_fops);

#ifdef CONFIG_RTK_BOOTINFO_SUPPORT
	rtk_init_bootinfo_proc(flash_proc_dir);
#endif
}

/* read one page */
static int rtkn_scan_read_data(struct mtd_info *mtd, uint8_t *buf, loff_t offs)
{
	size_t retlen;
	size_t len;

	len = mtd->writesize;
	return mtd_read(mtd, offs, len, &retlen, buf);
}

void detect_nand_flash_map(struct mtd_info *mtd, struct mtd_partition *rtl819x_parts)
{
	struct squashfs_super_block *sb;
	uint32_t mtd_size;
	uint32_t active_bank_offset=0;
	unsigned int page, block;
	unsigned char* data_buf = NULL;
	int offset;

	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int page_size = mtd->writesize,ppb = mtd->erasesize/mtd->writesize;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
	int bootbank=0;
	uint32_t back_bank_offset = 0;
	BOOTINFO_P bootinfo_p = &bootinfo_ram;
#endif

	if(RTK_FLASH_BANK_SIZE != 0)
		mtd_size = RTK_FLASH_BANK_SIZE;
	else
		mtd_size = (uint32_t)mtd->size;

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
   mtd_size = CONFIG_RTK_DUALIMAGE_FLASH_OFFSET;
   back_bank_offset = mtd_size;
   bootbank = bootinfo_p->data.field.bootbank;

	if(bootbank == 1 )
	{
		active_bank_offset = mtd_size;
		back_bank_offset = 0;
	}
#endif

	data_buf=kmalloc(page_size,GFP_KERNEL);
	if(!data_buf){
		printk("malloc data_buf fail\n");
		return;
	}
	offset =RTK_LINUX_PART_OFFSET;

	while ((offset + page_size) < mtd_size) {
		page = ((int) (offset + active_bank_offset)) >> this->page_shift;
		block = page/ppb;

		if(rtkn_scan_read_data(mtd,data_buf,page*page_size)){
			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			if(data_buf)
				kfree(data_buf);
			return;
		}

		if (*((__u32 *)data_buf) == SQUASHFS_MAGIC_LE) {

			sb = (struct squashfs_super_block *)data_buf;
			rtl819x_parts[3].offset = offset + active_bank_offset;								//rootfs
			rtl819x_parts[3].size =  (mtd_size+active_bank_offset) - rtl819x_parts[3].offset;

			rtl819x_parts[2].offset = active_bank_offset + rtl819x_parts[2].offset;				//linux
			rtl819x_parts[2].size =  rtl819x_parts[3].offset - rtl819x_parts[2].offset;

			rtl819x_parts[1].offset = active_bank_offset + rtl819x_parts[1].offset;				//hw_setting

			break;
		}
		offset += page_size;
	}

#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
	rtl819x_parts[4].offset = back_bank_offset + RTK_LINUX_PART_OFFSET;
	rtl819x_parts[4].size =  (mtd_size+back_bank_offset) - rtl819x_parts[4].offset;
#endif

	if(data_buf)
		kfree(data_buf);
	return;
}

//read the kerenl command to get the linux part info
static int __init get_linux_part(char *str)
{
	unsigned int  linux_part=0;

	sscanf(str, "0x%x", &linux_part);
	RTK_LINUX_PART_OFFSET = linux_part;
	return 1;
}
__setup("linuxpart=", get_linux_part);

static int __init get_hw_part(char *str)
{
	unsigned int  hw_part=0;

	sscanf(str, "0x%x", &hw_part);
	HW_SETTING_OFFSET = hw_part;
	//printk("get_hw_part = %x \n",HW_SETTING_OFFSET);
	return 1;
}
__setup("hwpart=", get_hw_part);

EXPORT_SYMBOL(HW_SETTING_OFFSET);
#endif
