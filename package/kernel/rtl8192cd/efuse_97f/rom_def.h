
#ifndef	__ROM_DEF_H__
#define __ROM_DEF_H__

/*** basic type ***/
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef VOID
#define VOID    void
#endif

#ifndef NULL
#define NULL	          ((void *)0)
#endif

#if 0
#ifndef BOOLEAN
typedef unsigned char		    BOOLEAN,*PBOOLEAN;
#endif 

#ifndef u1Byte
typedef unsigned char			u1Byte,*pu1Byte;
typedef unsigned short			u2Byte,*pu2Byte;
typedef unsigned int			u4Byte,*pu4Byte;
typedef unsigned long long		u8Byte,*pu8Byte;

typedef signed char				s1Byte,*ps1Byte;
typedef signed short			s2Byte,*ps2Byte;
typedef signed int				s4Byte,*ps4Byte;
typedef signed long long		s8Byte,*ps8Byte;
#endif
typedef unsigned long long		ULONG64,*PULONG64;
#endif 

#if 0
/* In ARM platform, system would use the type -- "char" as "unsigned char"*/
typedef signed char		    s1Byte,*ps1Byte;
typedef signed short		s2Byte,*ps2Byte;
typedef signed int			s4Byte,*ps4Byte;
typedef signed long long	s8Byte,*ps8Byte;

typedef unsigned long		UINT32,*pUINT32;
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed long			INT32;
typedef unsigned int		UINT;
typedef signed int			INT;
typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif

#define BIT(x)	(1 << (x))

/* Register Macro */
#ifndef REG32
#define REG32(reg)      (*(volatile u4Byte *)(reg))
#endif
#ifndef REG16
#define REG16(reg)      (*(volatile u2Byte *)(reg))
#endif
#ifndef REG8
#define REG8(reg)       (*(volatile u1Byte *)(reg))
#endif

#define TRUE        1
#define FALSE       0

typedef enum _ROM_STATUS_ 
{
    ROM_STATUS_FAIL     = -1,
    ROM_STATUS_SUCCESS  = 0
} ROM_STATUS, *PROM_STATUS;

typedef enum _IMG_TYPE_
{
    IMG_TYPE_INIT_RAM,
    IMG_TYPE_UBOOT
} IMG_TYPE, *PIMG_TYPE;

/*// the file header is reference by /branch-rtl819x-sdk-vXXX/users/goahead-2.1.1/LINUX/cvimg.c*/
#define UBOOT_SIGNATURE		((unsigned char *)"boot")
#define INIT_RAM_SIGNATURE	((unsigned char *)"iram")
#define SIG_LEN			    4
#define UBOOT_FILE_NAME     ("boot.bin")

typedef struct _IMG_HEADER_TYPE_
{
	unsigned char signature[SIG_LEN];
	unsigned long startAddr;
	unsigned long burnAddr;
	unsigned long len;
} IMG_HEADER_TYPE, *PIMG_HEADER_TYPE;


#define KEY_CODE_ESC            27
#define KEY_CODE_u              117
#define KEY_CODE_v              118
#define TIME_OUT                10000
#define IMAGE_OFFSET            0x40000 // 256 * 1024 bytes
#define UBOOT_IMAGE_BAKCUP_NUM  4  // We store uboot in external storage UBOOT_IMAGE_BAKCUP_NUM copies
#define HEAP_START_ADDR         0x80000000
#define HEAP_END_ADDR           0x80010000
#define ERASE_SPI_NOR_FLASH_START_ADDR  0xB0000000

#define DBG_MESSAGE			    0
#define DBG_SERIOUS				1
#define DBG_WARNING				2
#define DBG_TRACE				3

#define COMP_CONSOLE            BIT(0)
#define COMP_MONITOR            BIT(1)
#define COMP_EFUSE		        BIT(2)
#define COMP_DRAM               BIT(3)
#define COMP_EMMC               BIT(4)
#define COMP_SPI_NAND           BIT(5)
#define COMP_PARALLEL_NAND      BIT(6)
#define COMP_SWITCH             BIT(7)
#define COMP_SDCARD             BIT(8)
#define COMP_SPI_NOR            BIT(9)

extern u4Byte globalDebugComponents;
extern u1Byte globalDebugLevel;

#if 1
#define DBG_MSG(_comp_, _level_, _fmt_)
#else
#define DBG_MSG(_comp_, _level_, _fmt_) \
    if (((_comp_) & globalDebugComponents) && (_level_ <= globalDebugLevel))   \
    {   printf _fmt_;  }
#endif

#define printf      dprintf
#define GetChar()	dwc_serial_getc()
#define PutChar(x)	dwc_serial_putc(x)

typedef enum _ROM_PROGRESS_
{
    ROM_PROGRESS_UART_OK                = 0x1,
    ROM_PROGRESS_LOAD_EFUSE_TO_REG_OK   = 0x2,
    ROM_PROGRESS_LOAD_EFUSE_TO_REG_FAIL = 0x3,
    ROM_PROGRESS_LOAD_EFUSE_TO_MEM_OK   = 0x4,
    ROM_PROGRESS_LOAD_EFUSE_TO_MEM_FAIL = 0x5,
    ROM_PROGRESS_INIT_IP_OK             = 0x6,
    ROM_PROGRESS_INIT_IP_FAIL           = 0x7,
    ROM_PROGRESS_INIT_DDR_OK            = 0x8,
    ROM_PROGRESS_INIT_DDR_FAIL          = 0x9,
    ROM_PROGRESS_EXEC_FROM_SRAM_OK      = 0xA,
    ROM_PROGRESS_EXEC_FROM_SRAM_FAIL    = 0xB,
    ROM_PROGRESS_IMG_SIG_OK             = 0xC,
    ROM_PROGRESS_IMG_SIG_ERR            = 0xD,
    ROM_PROGRESS_IMG_CHKSUM_OK          = 0xE,
    ROM_PROGRESS_IMG_CHKSUM_ERR         = 0xF,
    ROM_PROGRESS_LOAD_IMG_OK            = 0x10,
    ROM_PROGRESS_LOAD_IMG_FAIL          = 0x11
} ROM_PROGRESS, *PROM_PROGRESS;


/*** function list ***/
extern u4Byte dwc_serial_init (void);
extern int dwc_serial_getc (void);
extern void dwc_serial_putc (const char c);
extern void dprintf(const char *fmt, ...);

u4Byte init_spi_nand_flash(void);
u4Byte init_parallel_nand_flash(void);
u2Byte verify_checksum(pu1Byte buf, u4Byte len);


//////////////////////////////////////////

// RTL8196F bus architecture
// LX0 slave: switch, NFBI, Parallel-NAND-Flash, Timer, GPIO, ISR, sys reg?, efuse, SPI-NAND-Flash, 
// LX1 slave: PCM, I2S, SD/MMC, IPSec, 
// LX2 slave: USB2 host, PCIe_RC, 
// LX0 master:  NFBI, Paralle-NAND-Flash, SPI-NAND-Flash, 
// LX1 master: PCM, I2S, SD/MMC, IPSec,
// LX2 master: switch
// AHB: GDMA, USB2 host, USB2 otg, 
// APB: SPI, I2C, GTimer, UART, WiFi, 
// TLP: PCIe_RC
// AXI3: WiFi

#define REG_SYS_UART            (BIT_ACTIVE_RT_UART0|BIT_ACTIVE_APB_BUS)
#define REG_SYS_SPI_NAND        (BIT_ACTIVE_SPINAND|BIT_ACTIVE_SPIFLSHECC)
#define REG_SYS_PARALLEL_NAND   (BIT_ACTIVE_NFCTRL)
#define REG_SYS_EMMC_SD_1       (BIT_ACTIVE_LX1ARB|BIT_ACTIVE_LX1)
#define REG_SYS_EMMC_SD_2       (BIT_ACTIVE_SD30|BIT_ACTIVE_SD30_PLL5M)
#define REG_SYS_USB_HOST_1      (BIT_ACTIVE_LX2ARB|BIT_ACTIVE_LX2)
#define REG_SYS_USB_HOST_2      (BIT_ACTIVE_USB2HOST)
#define REG_SYS_USB_OTG         (BIT_ACTIVE_USBOTG)
#define REG_SYS_SWITCH          (BIT_ACTIVE_LX2ARB|BIT_ACTIVE_LX2|BIT_SWITCH_RST_PROTECT|BIT_ACTIVE_SWCORE)

#define ENABLE_UART             REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | REG_SYS_UART;
#define ENABLE_SPI_NAND         REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | REG_SYS_SPI_NAND;
#define ENABLE_PARALLEL_NAND    REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | REG_SYS_PARALLEL_NAND;
#define ENABLE_EMMC_SD  \
    do { \
        REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) | REG_SYS_EMMC_SD_1;      \
        REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | REG_SYS_EMMC_SD_2;    \
    } while(0);
#define ENABLE_USB_HOST \
    do { \
        REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) | REG_SYS_USB_HOST_1;      \
        REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) | REG_SYS_USB_HOST_2;    \
    } while(0);
#define ENABLE_USB_OTG          REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) | REG_SYS_USB_OTG;
#define ENABLE_SWITCH           REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) | REG_SYS_SWITCH;

#define DISABLE_UART            REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) & ~REG_SYS_UART;
#define DISABLE_SPI_NAND        REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) & ~REG_SYS_SPI_NAND;
#define DISABLE_PARALLEL_NAND   REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) & ~REG_SYS_PARALLEL_NAND;
#define DISABLE_EMMC_SD  \
    do { \
        REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) & ~REG_SYS_EMMC_SD_1;      \
        REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) & ~REG_SYS_EMMC_SD_2;    \
    } while(0);
#define DISABLE_USB_HOST \
    do { \
        REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) & ~REG_SYS_USB_HOST_1;      \
        REG32(REG_CLK_MANAGE2) = REG32(REG_CLK_MANAGE2) & ~REG_SYS_USB_HOST_2;    \
    } while(0);
#define DISABLE_USB_OTG         REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) & ~REG_SYS_USB_OTG;
#define DISABLE_SWITCH          REG32(REG_CLK_MANAGE) = REG32(REG_CLK_MANAGE) & ~REG_SYS_SWITCH;


#define PRINT_DATA(_TitleString, _HexData, _HexDataLen)						\
{												\
	char		*szTitle = _TitleString;					\
	pu1Byte		pbtHexData = _HexData;							\
	u4Byte		u4bHexDataLen = _HexDataLen;						\
	u4Byte		__i;									\
	dprintf("%s", szTitle);								\
	for (__i=0;__i<u4bHexDataLen;__i++)								\
	{											\
		if ((__i & 15) == 0) 								\
		{										\
			dprintf("\n");								\
		}										\
		dprintf("%02X%s", pbtHexData[__i], ( ((__i&3)==3) ? "  " : " ") );			\
	}											\
	dprintf("\n");										\
}

#define member_size(type, member) sizeof(((type *)0)->member)

#endif // #ifndef	__ROM_DEF_H__

