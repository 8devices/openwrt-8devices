#define TAG_LEN					4
#define RTK_BOOTINFO_SIGN "hwbt"
#define RTK_MAX_VALID_BOOTCNT 16

#define BOOT_NORMAL_MODE 0
#define BOOT_DUALIMAGE_TOGGLE_MODE 1
#define BOOT_DEFAULT_MAXCNT 3
/* Config file header */

typedef struct bootinfo_header {
	unsigned char tag[TAG_LEN] ;  // Tag + version
	unsigned int len ;
} BOOTINFO_HEADER_T, *BOOTINFO_HEADER_Tp;

typedef union bootinfo_data{
		unsigned int 	val;
        struct {
			unsigned char	bootbank;
			unsigned char	bootmaxcnt;
			unsigned char	bootcnt;
			unsigned char	bootmode;
        } field;
} BOOTINFO_DATA_T, *BOOTINFO_DATA_P;

typedef struct bootinfo {
	BOOTINFO_HEADER_T header;
	BOOTINFO_DATA_T data;
}BOOTINFO_T, *BOOTINFO_P;


void  rtk_reset_bootinfo(BOOTINFO_P boot)
{
	memcpy(&boot->header.tag,RTK_BOOTINFO_SIGN,TAG_LEN);
	boot->header.len= sizeof(BOOTINFO_DATA_T);
	boot->data.field.bootmaxcnt=BOOT_DEFAULT_MAXCNT;
	boot->data.field.bootbank=0;
	boot->data.field.bootcnt=0;
#ifdef CONFIG_RTK_BOOTINFO_DUALIMAGE
	boot->data.field.bootmode=BOOT_DUALIMAGE_TOGGLE_MODE;
#else
	boot->data.field.bootmode=BOOT_NORMAL_MODE;
#endif
}


int  rtk_check_bootinfo(BOOTINFO_P boot)
{

     if(!memcmp(boot->header.tag, RTK_BOOTINFO_SIGN, TAG_LEN)) //ok
		return 1;

	return 0; //fail
}


void rtk_read_bootinfo_from_flash(unsigned int bootinfo_addr,BOOTINFO_P bootinfo_ram_p)
{

	memset((char *)bootinfo_ram_p,0,sizeof(BOOTINFO_T));

	//flash(spi...etc)  can be read directly
	memcpy((char *)bootinfo_ram_p,(char *)bootinfo_addr,sizeof(BOOTINFO_T));
#if 0
	printk("rtk_read_bootinfo_from_flash \n");
	printk("header=%2x%2x%2x%2x len=%x\n",bootinfo_ram_p->header.tag[0],bootinfo_ram_p->header.tag[1]
		,bootinfo_ram_p->header.tag[2],bootinfo_ram_p->header.tag[3],bootinfo_ram_p->header.len);

	printk("bank=%d , cnt=%d,maxcnt=%d \n",bootinfo_ram_p->data.field.bootbank,bootinfo_ram_p->data.field.bootcnt,
		bootinfo_ram_p->data.field.bootmaxcnt);
#endif

}

void rtk_write_bootinfo_to_flash(unsigned int bootinfo_addr,BOOTINFO_P bootinfo_ram_p,
	                     void (*write_func)(unsigned int ,unsigned int ,unsigned char*) )
{
#if 0
	printk("rtk_read_bootinfo_from_flash \n");
	printk("header=%2x%2x%2x%2x len=%x\n",bootinfo_ram_p->header.tag[0],bootinfo_ram_p->header.tag[1]
		,bootinfo_ram_p->header.tag[2],bootinfo_ram_p->header.tag[3],bootinfo_ram_p->header.len);

	printk("bank=%d , cnt=%d,maxcnt=%d , val=%8x \n",bootinfo_ram_p->data.field.bootbank,bootinfo_ram_p->data.field.bootcnt,
		bootinfo_ram_p->data.field.bootmaxcnt,bootinfo_ram_p->data.val);
#endif
	//flash(spi...etc) need flash write api
	write_func(bootinfo_addr,sizeof(BOOTINFO_T),(char *)bootinfo_ram_p);
}
