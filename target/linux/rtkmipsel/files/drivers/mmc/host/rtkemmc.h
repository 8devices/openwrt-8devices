/*
 *  Copyright (C) 2010 Realtek Semiconductors, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RTKEMMC_H
#define __RTKEMMC_H

#include "reg_mmc.h"
#include "../mmc_debug.h"	//liao

//debug
//#define MMC_DBG
#ifdef MMC_DBG
#define MMCPRINTF(fmt, args...)   printk(fmt,## args)
#else
#define MMCPRINTF(fmt, args...)
#endif

/* cmd1 sector mode */
#define MMC_SECTOR_ADDR         0x40000000
/*
 * Clock rates
 */
#define RTKSD_CLOCKRATE_MAX			48000000
#define RTKSD_BASE_DIV_MAX			0x100

/*
 * pad driving
 * */
#define MMC_IOS_GET_PAD_DRV     0x1
#define MMC_IOS_SET_PAD_DRV     0x2
#define MMC_IOS_RESTORE_PAD_DRV 0x4

#define RBUS_BASE_VIRT			(0xFE000000)
//#define GET_MAPPED_RBUS_ADDR(x)               (x - 0x18000000 + RBUS_BASE_VIRT)
#define GET_MAPPED_RBUS_ADDR(x)		(x)
#define SYS_muxpad0				(SYS_REG_BASE + 0x360)	//mux for nf

#define cr_readb(offset)        (*(volatile unsigned char *)GET_MAPPED_RBUS_ADDR(offset))
#if 0
#define cr_writeb(val, offset)  do {							\
				(*(volatile unsigned char *) (offset)) = val;\
                                asm ("DMB");						\
				} while(0);
#else
#define cr_writeb(val, offset)  do {							\
				(*(volatile unsigned char *) GET_MAPPED_RBUS_ADDR(offset)) = val;           \
				} while(0)
//#define cr_writeb(val, offset)  ((*(volatile unsigned char *) (offset)) = val)
#endif
#define cr_readl(offset)        (*(volatile unsigned int *)GET_MAPPED_RBUS_ADDR(offset))
#define cr_writel(val, offset)  do {							\
				(*(volatile unsigned int *) GET_MAPPED_RBUS_ADDR(offset)) = val;            \
				} while(0)

#define CLEAR_CR_CARD_STATUS(reg_addr)      \
do {                                \
    cr_writel(0xffffffff, reg_addr);    \
} while (0)

#define CLEAR_ALL_CR_CARD_STATUS(io_base)        \
do {                                \
    int i = 0;  \
    for(i=0; i<5; i++)  \
        CLEAR_CR_CARD_STATUS(io_base+20+ (i*4));  \
} while (0)

/*  ===== macros and funcitons for Realtek CR ===== */
/* for SD/MMC usage */
#define ON                      0
#define OFF                     1
#define GPIO_OUT    1
#define GPIO_IN     0
#define GPIO_HIGH   1
#define GPIO_LOW    0

/* MMC configure1, for SD_CONFIGURE1 */
#define SD30_FIFO_RST               (1<<4)
#define SD1_R0                      (SD30_FIFO_RST)

/* MMC configure3 , for SD_CONFIGURE3 */
#define SD30_CLK_STOP               (1<<4)
#define SD2_R0                      (RESP_CHK_EN | ADDR_BYTE_MODE)

/* MMC configure2, for SD_CONFIGURE2, response type */
#define SD_R0                   (RESP_TYPE_NON|CRC7_CHK_DIS)
#define SD_R1                   (RESP_TYPE_6B)
#define SD_R1b                  (RESP_TYPE_6B|WAIT_BUSY_EN)
#define SD_R2                   (RESP_TYPE_17B)
#define SD_R3                   (RESP_TYPE_6B)
#define SD_R4                   (RESP_TYPE_6B)
#define SD_R5                   (RESP_TYPE_6B)
#define SD_R6                   (RESP_TYPE_6B)
#define SD_R7                   (RESP_TYPE_6B)
#define SD_R_NO                 (0xFF)

/* rtflags */
#define RTKCR_FCARD_DETECTED    (1<<0)	/* Card is detected */
#define RTKCR_FOPEN_LOG         (1<<1)	/* open command log */
#define RTKCR_USER_PARTITION    (1<<2)	/* card is working on normal partition */

#define RTKCR_FCARD_POWER       (1<<4)	/* Card is power on */
#define RTKCR_FHOST_POWER       (1<<5)	/* Host is power on */

struct rtksd_host {
	struct mmc_host *mmc;	/* MMC structure */
	u32 rtflags;		/* Driver states */
	u8 ins_event;
	u8 cmd_opcode;

#define EVENT_NON		    0x00
#define EVENT_INSER		    0x01
#define EVENT_REMOV		    0x02
#define EVENT_USER		    0x10

	u8 reset_event;
	struct mmc_request *mrq;	/* Current request */
	u8 wp;
	struct rtk_host_ops *ops;
	struct semaphore sem;
	struct semaphore sem_op_end;

	void __iomem *base;
	u32 base_io;
	spinlock_t lock;
	unsigned int ns_per_clk;
	struct delayed_work cmd_work;
	struct tasklet_struct req_end_tasklet;

	struct timer_list timer;
	struct timer_list plug_timer;
	struct completion *int_waiting;
	struct device *dev;
	struct resource *res;
	int irq;
	u8 *tmp_buf;
	dma_addr_t tmp_buf_phy_addr;
	dma_addr_t paddr;
#ifdef MONI_MEM_TRASH
	u8 *cmp_buf;
	dma_addr_t phy_addr;
#endif
	//int                 gpio_card_detect;
	//int                 gpio_write_protect;
	u32 test_count;
	u32 int_status;
	u32 int_status_old;
	u32 sd_status1;
	u32 sd_status2;
	u32 bus_status;
	u32 dma_trans;
	u32 sd_trans;
	u32 gpio_isr_info;
	u32 tmout;
};

struct rtk_host_ops {
	irqreturn_t(*func_irq) (int irq, void *dev);
	int (*re_init_proc) (struct mmc_card * card);
	int (*card_det) (struct rtksd_host * sdport);
	void (*card_power) (struct rtksd_host * sdport, u8 status);
	void (*chk_card_insert) (struct rtksd_host * rtkhost);
	void (*set_crt_muxpad) (struct rtksd_host * rtkhost);
	void (*set_clk) (struct rtksd_host * rtkhost, u32 mmc_clk);
	void (*reset_card) (struct rtksd_host * rtkhost);
	void (*reset_host) (struct rtksd_host * rtkhost);
	void (*bus_speed_down) (struct rtksd_host * sdport);
	 u32(*get_cmdcode) (u32 opcode);
	 u32(*get_r1_type) (u32 opcode);
	 u32(*chk_cmdcode) (struct mmc_command * cmd);
	 u32(*chk_r1_type) (struct mmc_command * cmd);
	 u32(*backup_regs) (struct rtksd_host * sdport);
	 u32(*restore_regs) (struct rtksd_host * sdport);
};

struct ms_cmd_pkt {
	struct rtksd_host *sdport;
	struct ms_command *mscmd;
	struct mmc_data *data;

	unsigned char *dma_buffer;	//data transfer address
	u16 cmdcode;		//rtk ms operation code
	u16 trans_bytes;	//register transfer bytes
	u16 block_count;	//data block transfer count
};

struct sd_cmd_pkt {
	struct mmc_host *mmc;	/* MMC structure */
	struct rtksd_host *sdport;
	struct mmc_command *cmd;	/* cmd->opcode; cmd->arg; cmd->resp; cmd->data */
	struct mmc_data *data;
	unsigned char *dma_buffer;
	u16 byte_count;
	u16 block_count;

	u32 flags;
	s8 rsp_para1;
	s8 rsp_para2;
	s8 rsp_para3;
	u8 rsp_len;
	u32 timeout;
};

struct backupRegs {
	u32 sys_pll_emmc3;
	u32 pll_emmc1;
	u8 card_select;
	u8 sample_point_ctl;
	u8 push_point_ctl;
	u8 sd_configure1;
	u8 sd_configure2;
	u8 sd_configure3;
};

/* *** Realtek CRT register &&& */

/* CRT_SYS_CLKSEL setting bit *** */
//#define EMMC_CLKSEL_SHT        (12)
//#define SD_CLKSEL_SHT          (20)

#define EMMC_CLKSEL_MASK        (0x07<<EMMC_CLKSEL_SHT)
#define SD_CLKSEL_MASK          (0x07<<SD_CLKSEL_SHT)
#define EMMC_CLOCK_SPEED_GAP    (0x03<<EMMC_CLKSEL_SHT)
#define SD_CLOCK_SPEED_GAP      (0x03<<SD_CLKSEL_SHT)

static const char *const clk_tlb_B[8] = {
	"12MHz",
	"16MHz",
	"20MHz",
	"25MHz",
	"33MHz",
	"49MHz"
};

/* Magellan_ISO_arch_spec.doc; PMM_Magellan_PinMux.doc */
// iso gpio pinmux map
static const u8 st_gpio_map[36] = {
	0x34, 0x34, 0x34, 0x34, 0x30, 0x30, 0x30, 0x30,	//0:7
	0x2c, 0x2c, 0x2c, 0x2c, 0x28, 0x28, 0xff, 0xff,	//8:15
	0xff, 0xff, 0xff, 0xff, 0xff, 0x28, 0x28, 0x24,	//16:23
	0xff, 0xff, 0x24, 0x24, 0x24, 0xff, 0xff, 0xff,	//24:31
	0xff, 0xff, 0xff, 0x20	//32:35
};

// iso gpio pinmux bit operation map
static const u8 st_gpio_bit_map[36] = {
	0x04, 0x0C, 0x14, 0x1C, 0x04, 0x0C, 0x14, 0x1C,	//0:7
	0x04, 0x0C, 0x14, 0x1C, 0x04, 0x0C, 0xff, 0xff,	//8:15
	0xff, 0xff, 0xff, 0xff, 0xff, 0x14, 0x1C, 0x04,	//16:23
	0xff, 0xff, 0x0C, 0x14, 0x1C, 0xff, 0xff, 0xff,	//24:31
	0xff, 0xff, 0xff, 0x04	//32:35
};

//#define MAX_CMD_RETRY_COUNT 4
#define MAX_CMD_RETRY_COUNT 2
#define MAX_ISO_GPIO_CNT 35
#define MAX_MIS_GPIO_CNT 169
static const u8 mis_gpio_bit_map[4] = {
	0x1C, 0x14, 0x0C, 0x04	//0:3
};

#define CARD_SWITCHCLOCK_25MHZ_B    (0x00UL)	//(0x05)
#define CARD_SWITCHCLOCK_33MHZ_B    (0x01UL)	//(0x06)
#define CARD_SWITCHCLOCK_40MHZ_B    (0x02UL)	//(0x00)
#define CARD_SWITCHCLOCK_50MHZ_B    (0x03UL)	//(0x01)
#define CARD_SWITCHCLOCK_66MHZ_B    (0x04UL)	//(0x02)
#define CARD_SWITCHCLOCK_98MH_B     (0x05UL)	//(0x03)
#define CLOCK_SPEED_GAP          CARD_SWITCHCLOCK_50MHZ_B
#define LOW_SPEED_LMT               CARD_SWITCHCLOCK_33MHZ_B

static const u32 map_clk_to_reg[6] = { 0x05, 0x06, 0x00, 0x01, 0x02, 0x03 };

static const u32 map_reg_to_clk[8] =
    { 0x02, 0x03, 0x04, 0x05, 0xFF, 0x00, 0x01, 0xFF };

#define CARD_SWITCHCLOCK_60MHZ  (0x00UL)	//(0x00<<12)
#define CARD_SWITCHCLOCK_80MHZ  (0x01UL)	//(0x01<<12)
#define CARD_SWITCHCLOCK_98MHZ  (0x02UL)	//(0x02<<12)
#define CARD_SWITCHCLOCK_98MHZS (0x03UL)	//(0x03<<12)
#define CARD_SWITCHCLOCK_30MHZ  (0x04UL)	//(0x04<<12)
#define CARD_SWITCHCLOCK_40MHZ  (0x05UL)	//(0x05<<12)
#define CARD_SWITCHCLOCK_49MHZ  (0x06UL)	//(0x06<<12)
#define CARD_SWITCHCLOCK_49MHZS (0x07UL)	//(0x07<<12)

/* CRT_SYS_CLKSEL setting bit &&& */

/* CRT_SYS_SRST2 setting bit *** */
#define RSTN_CR (0x01<<23)	//for eMMC
/* CRT_SYS_SRST2 setting bit &&& */

/* CRT_SYS_SRST3 setting bit *** */
#define RSTN_SD (0x01<<16)	//for card reader
/* CRT_SYS_SRST3 setting bit &&& */

/* CRT_SYS_CLKEN2 setting bit *** */
#define CLKEN_CR (0x01<<23)	//for eMMC
/* CRT_SYS_CLKEN2 setting bit &&& */

/* CRT_SYS_CLKEN3 setting bit *** */
#define CLKEN_SD (0x01<<16)	//for card reader
/* CRT_SYS_CLKEN3 setting bit &&& */

#define CR_PINMUX_CR_MASK       0xFFFFC000

#define MUX_SD_DAT2             (0x01<<26)
#define MUX_MS_CLK              (0x02<<26)
#define MUX_SD_DAT1             (0x01<<24)
#define MUX_MS_BS               (0x02<<24)
#define MUX_SD_DT               (0x01<<20)
#define MUX_OTP_ATE_FAIL        (0x02<<20)
#define MUX_SD_WP               (0x01<<18)
#define MUX_MS_DT               (0x02<<18)
#define MUX_SD_CLK              (0x01<<16)
#define MUX_MS_DAT0             (0x02<<16)
#define MUX_SD_CMD              (0x01<<14)
#define MUX_MS_DAT2             (0x02<<14)

#define MUX_PAD5_DEFAULT         0x15554000

#define CR_PMUX_CMD_MASK        ~(0x03<<14)	//mask 15,14
#define CR_PMUX_CLK_MASK        ~(0x03<<16)	//mask 16,17
#define CR_PMUX_MOD_MASK        ~(0x0f<<18)	//mask 18,19,20,21
//#define DETECT_MODE             MUX_MS_DT|MUX_SD_DT
#define DETECT_MODE             MUX_SD_DT

#define MUX_PAD5_SD_MODE        0x15554000

#define RCA_SHIFTER             16
#define NORMAL_READ_BUF_SIZE    512	//no matter FPGA & QA

#define SD_CARD_WP              (1<<5)
//#define MS_CARD_EXIST           (1<<3)
#define SD_CARD_EXIST           (1<<2)

/* ======================================================= */
// see "Main_Magellan_PinMux.doc"
#define PINMUX_REG_BASE		0xb8000800
#define GPIO_MUXCFG_0		(PINMUX_REG_BASE + 0x00)	//0xb8000800
#define GPIO_MUXCFG_1		(PINMUX_REG_BASE + 0x04)	//0xb8000804
#define GPIO_MUXCFG_2		(PINMUX_REG_BASE + 0x08)	//0xb8000808
#define GPIO_MUXCFG_3		(PINMUX_REG_BASE + 0x0c)	//0xb800080C
#define GPIO_MUXCFG_4		(PINMUX_REG_BASE + 0x10)	//0xb8000810
#define GPIO_MUXCFG_5		(PINMUX_REG_BASE + 0x14)	//0xb8000814
#define GPIO_MUXCFG_6		(PINMUX_REG_BASE + 0x18)	//0xb8000818

#define GPIO_MUXCFG_13		(PINMUX_REG_BASE + 0x34)	//0xb8000834
#define GPIO_MUXCFG_14		(PINMUX_REG_BASE + 0x38)	//0xb8000838
#define GPIO_MUXCFG_15		(PINMUX_REG_BASE + 0x3C)	//0xb800083C
#define GPIO_MUXCFG_16		(PINMUX_REG_BASE + 0x40)	//0xb8000840
#define GPIO_MUXCFG_17		(PINMUX_REG_BASE + 0x44)	//0xb8000844
#define GPIO_MUXCFG_18		(PINMUX_REG_BASE + 0x48)	//0xb8000848

/*======================================================== */

/* move from c file *** */
#define BYTE_CNT            0x200
#define RTK_NORMAL_SPEED    0x00
#define RTK_HIGH_SPEED      0x01
#define RTK_1_BITS          0x00
#define RTK_4_BITS          0x10
#define RTK_BITS_MASK       0x30
#define RTK_SPEED_MASK      0x01
#define RTK_PHASE_MASK      0x06

#define R_W_CMD             2	//read/write command
#define INN_CMD             1	//command work chip inside
#define UIN_CMD             0	//no interrupt rtk command

#define RTK_FAIL            3	/* DMA error & cmd parser error */
#define RTK_RMOV            2	/* card removed */
#define RTK_TOUT            1	/* time out include DMA finish & cmd parser finish */
#define RTK_SUCC            0

#define CR_TRANS_OK         0x0
#define CR_TRANSFER_TO      0x1
#define CR_BUF_FULL_TO      0x2
#define CR_DMA_FAIL         0x3
#define CR_TRANSFER_FAIL    0x4

/* send status event */
#define STATE_IDLE          0
#define STATE_READY         1
#define STATE_IDENT         2
#define STATE_STBY          3
#define STATE_TRAN          4
#define STATE_DATA          5
#define STATE_RCV           6
#define STATE_PRG           7
#define STATE_DIS           8

#define GPIO_HI             0x78
#define GPIO_LO             0x87

#define LVL_HI             0x01
#define LVL_LO             0x00

#define POW_CHECK 0
#define POW_FORCE 1

#define rtkcr_get_int_sta(io_base,status_addr)                           \
            do {                                                         \
                if (io_base == CR_BASE_ADDR)                             \
                    *(u32 *)status_addr = cr_readl(io_base+CR_SD_ISR);   \
                else                                                     \
                    *(u32 *)status_addr = cr_readl(io_base+EMMC_SD_ISR); \
            } while (0);

#define rtkcr_get_sd_sta(io_base,status_addr1,status_addr2,bus_status)       \
            do {                                                             \
                    *(u32 *)status_addr1 = cr_readb(io_base+SD_STATUS1);     \
                    *(u32 *)status_addr2 = cr_readb(io_base+SD_STATUS2);     \
                    *(u32 *)bus_status = cr_readb(io_base+SD_BUS_STATUS);    \
            } while (0);

#define rtkcr_get_sd_trans(io_base,status_addr) \
            *(u32 *)status_addr = cr_readb(io_base+SD_TRANSFER)
#define rtkcr_get_dma_trans(io_base,status_addr) \
            *(u32 *)status_addr = cr_readb(io_base+CR_DMA_CTL3)

#define rtkcr_clr_int_sta(io_base)		writel(0x00000006, io_base + CR_SD_ISR);
#define rtkcr_hold_int_dec(io_base)	writel(0x00000006, io_base + EMMC_SD_ISREN);
#define rtkcr_en_int(io_base)			writel(0x00000007, io_base + EMMC_SD_ISREN);
//#define rtkcr_hold_int_dec(io_base)
//#define rtkcr_en_int(io_base)
//#define rtkcr_clr_int_sta(io_base)  writel(0x00000006, io_base + CR_SD_ISR);
/*
#define rtkcr_clr_int_sta(io_base)                                                                              \
            do {                                                                                                \
                if (io_base == CR_BASE_ADDR)                                                                    \
                    cr_writel( (cr_readl(io_base+CR_SD_ISR)|(ISRSTA_INT1|ISRSTA_INT2))|CLR_WRITE_DATA ,io_base+CR_SD_ISR); \
                else if (io_base == EM_BASE_ADDR)                                                               \
                    cr_writel( (cr_readl(io_base+EMMC_SD_ISR)|(ISRSTA_INT1|ISRSTA_INT2))|CLR_WRITE_DATA ,io_base+EMMC_SD_ISR); \
                else                                                                                            \
                    cr_writel( (cr_readl(io_base+SDIO_SD_ISR)|(SDIO_ISRSTA_INT1|SDIO_ISRSTA_INT2|SDIO_ISRSTA_INT3|SDIO_ISRSTA_INT4))|CLR_WRITE_DATA ,io_base+SDIO_SD_ISR); \
            } while(0);

#define rtkcr_hold_int_dec(io_base)    \
            do {                                                                                                \
                if (io_base == CR_BASE_ADDR)                                                                    \
                    cr_writel( (cr_readl(io_base+CR_SD_ISREN)|(ISRSTA_INT1EN|ISRSTA_INT2EN))|CLR_WRITE_DATA ,io_base+CR_SD_ISREN); \
                else if (io_base == EM_BASE_ADDR)                                                               \
                    cr_writel( (cr_readl(io_base+EMMC_SD_ISREN)|(ISRSTA_INT1EN|ISRSTA_INT2EN))|CLR_WRITE_DATA ,io_base+EMMC_SD_ISREN); \
                else                                                                                            \
                    cr_writel( (cr_readl(io_base+SDIO_SD_ISREN)|SDIO_ISRSTA_INT1EN)|CLR_WRITE_DATA ,io_base+SDIO_SD_ISREN); \
            } while(0);

#define rtkcr_en_int(io_base)  \
            do {                                                                                                \
                if (io_base == CR_BASE_ADDR)                                                                    \
                    cr_writel( (cr_readl(io_base+CR_SD_ISREN)|(ISRSTA_INT1EN|ISRSTA_INT2EN))|WRITE_DATA ,io_base+CR_SD_ISREN); \
                else if (io_base == EM_BASE_ADDR)                                                               \
                    cr_writel( (cr_readl(io_base+EMMC_SD_ISREN)|(ISRSTA_INT1EN|ISRSTA_INT2EN))|WRITE_DATA ,io_base+EMMC_SD_ISREN); \
                else                                                                                            \
                    cr_writel( (cr_readl(io_base+SDIO_SD_ISREN)|SDIO_ISRSTA_INT1EN)|WRITE_DATA ,io_base+SDIO_SD_ISREN); \
            } while(0);
*/
#define  rtkcr_mdelay(x)  \
            set_current_state(TASK_INTERRUPTIBLE); \
            schedule_timeout(msecs_to_jiffies(x))

#define INT_BLOCK_R_GAP 0x200
#define INT_BLOCK_W_GAP 5

static const char *const state_tlb[9] = {
	"STATE_IDLE",
	"STATE_READY",
	"STATE_IDENT",
	"STATE_STBY",
	"STATE_TRAN",
	"STATE_DATA",
	"STATE_RCV",
	"STATE_PRG",
	"STATE_DIS"
};

static const char *const bit_tlb[4] = {
	"1bit",
	"4bits",
	"8bits",
	"unknow"
};

static const char *const clk_tlb[8] = {
	"30MHz",
	"40MHz",
	"49MHz",
	"49MHz",
	"15MHz",
	"20MHz",
	"24MHz",
	"24MHz"
};

static const u32 const clk_2_hz[8] = {
	10000000,
	12000000,
	15000000,
	20000000,
	24000000,
	30000000,
	40000000,
	48000000
};

/* data read cmd */
static const u8 const opcode_r_type[16] = {
	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0
};

/* data write cmd */
static const u8 const opcode_w_type[16] = {
	1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
};

/* data xfer cmd */
static const u8 const opcode_d_type[16] = {
	1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0
};

static const char *const cmdcode_tlb[16] = {
	"N_W",			/* 0 */
	"AW3",			/* 1 */
	"AW4",			/* 2 */
	"UNK",			/* 3 */
	"UNK",			/* 4 */
	"AR3",			/* 5 */
	"AR4",			/* 6 */
	"UNK",			/* 7 */
	"SGR",			/* 8 */
	"AW1",			/* 9 */
	"AW2",			/* 10 */
	"UNK",			/* 11 */
	"N_R",			/* 12 */
	"AR1",			/* 13 */
	"AR2",			/* 14 */
	"UNK",			/* 15 */
};

#define card_sta_err_mask ((1<<31)|(1<<30)|(1<<29)|(1<<28)|(1<<27)|(1<<26)|(1<<24)|(1<<23)|(1<<22)|(1<<21)|(1<<20)|(1<<19)|(1<<18)|(1<<17)|(1<<16)|(1<<15)|(1<<13)|(1<<7))

/* Only ADTC type cmd use */
static const unsigned char rtk_sd_cmdcode[64][2] = {
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//0~3
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0},	//4~7
	{EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_AUTOREAD1, SD_R1},	//8~11
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_NORMALREAD, SD_R1}, {EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0},	//12~15
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_AUTOREAD2, SD_R1}, {EMMC_AUTOREAD1, SD_R1}, {EMMC_NORMALWRITE, SD_R1},	//16~19
	{EMMC_AUTOWRITE1, SD_R1}, {EMMC_TUNING, SD_R0}, {EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0},	//20~23
	{EMMC_AUTOWRITE2, SD_R1}, {EMMC_AUTOWRITE1, SD_R1}, {EMMC_NORMALWRITE, SD_R1}, {EMMC_NORMALWRITE, SD_R1},	//24~27
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_NORMALREAD, SD_R1}, {EMMC_NORMALREAD, SD_R1},	//28~31
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//32~35
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//36~39
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0},	//40~43
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//44~47
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_NORMALREAD, SD_R1},	//48~51
	{EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//52~55
	{EMMC_AUTOREAD2, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0},	//56~59
	{EMMC_AUTOWRITE2, SD_R1}, {EMMC_NORMALREAD, SD_R1}, {EMMC_CMD_UNKNOW, SD_R0}, {EMMC_CMD_UNKNOW, SD_R0}	//60~63
};

/* remove from c file &&& */

/* rtk function definition */
#define ENABLE_EMMC_INT_MODE
/* rtk function definition */
int error_handling(struct rtksd_host *sdport, unsigned int cmd_idx,
		   unsigned int bIgnore);
int rtkcr_send_cmd25(struct rtksd_host *sdport);
int rtkcr_send_cmd18(struct rtksd_host *sdport);
//int rtkcr_send_cmd8(struct rtksd_host *sdport,unsigned int bIgnore);
int rtkcr_send_cmd8(struct rtksd_host *sdport, unsigned int bIgnore);
int polling_to_tran_state(struct rtksd_host *sdport, unsigned int cmd_idx,
			  int bIgnore);
void host_card_stop(struct rtksd_host *sdport);
static int mmc_Select_SDR50_Push_Sample(struct rtksd_host *sdport);

#endif
