#ifndef DRIVERS_MMC_HOST_RTK_SDMMC_H_
#define DRIVERS_MMC_HOST_RTK_SDMMC_H_



#ifdef CONFIG_MMC_RTK_SDMMC_DEBUG
#define rtk_sdmmc_debug(fmt, args...)        printk(KERN_DEBUG fmt, ##args)
#else
#define rtk_sdmmc_debug(fmt, args...)
#endif

struct rtk_sdmmc_host {

    struct mmc_host *mmc;
    struct mmc_request *mrq;
    struct mmc_command *rtk_sdmmc_cmd12;
    struct rtk_sdmmc_host_ops *ops;
    volatile u32 rtflags;

    u8 ins_event;
    u8 cmd_opcode;
    u8 reset_event;
    u8 wp;

    void __iomem *sdmmc;
    void __iomem *pll;
    void __iomem *sysbrdg;
    void __iomem *emmc;
    void __iomem *sdio;

    spinlock_t lock;

    struct tasklet_struct req_end_tasklet;
    struct timer_list timer;
    struct timer_list plug_timer;
    struct timer_list rtk_sdmmc_stop_cmd; //CMD25_WO_STOP_COMMAND

    struct completion *int_waiting;
    struct device *dev;
    int irq;

    dma_addr_t paddr;

    u32 sdmmc_gpio;
    u32 power_status;
    u32 int_status_old;
    u32 timeout;

    unsigned int magic_num;
};

struct sdmmc_cmd_pkt {
    struct mmc_host *mmc; /* MMC structure */
    struct rtk_sdmmc_host *rtk_host;
    struct mmc_command *cmd;
    struct mmc_data *data;
    u32 dma_buffer;
    u16 byte_count;
    u16 block_count;

    u32 flags;
    s8 rsp_para1;
    s8 rsp_para2;
    s8 rsp_para3;
    u8 rsp_len;
    u32 timeout;
};

struct rtk_sdmmc_host_ops {
    irqreturn_t (*func_irq)(int irq, void *dev);
    int (*re_init_proc)(struct mmc_card *card);
    int (*card_det)(struct rtk_sdmmc_host *rtk_host);
    void (*card_power)(struct rtk_sdmmc_host *rtk_host,u8 status);
    void (*chk_card_insert)(struct rtk_sdmmc_host *rtk_host);
    void (*set_crt_muxpad)(struct rtk_sdmmc_host *rtk_host);
    void (*set_clk)(struct rtk_sdmmc_host *rtk_host,u32 mmc_clk);
    void (*reset_card)(struct rtk_sdmmc_host *rtk_host);
    void (*reset_host)(struct rtk_sdmmc_host *rtk_host);
    void (*bus_speed_down)(struct rtk_sdmmc_host *rtk_host);
    u32 (*get_cmdcode)(u32 opcode );
    u32 (*get_r1_type)(u32 opcode );
    u8 (*chk_cmdcode)(struct mmc_command* cmd);
    u32 (*chk_r1_type)(struct mmc_command* cmd);
};

enum sdmmc_clock_speed {
    SDMMC_CLOCK_200KHZ = 0,
    SDMMC_CLOCK_400KHZ = 1,
    SDMMC_CLOCK_6200KHZ = 2,
    SDMMC_CLOCK_20000KHZ = 3,
    SDMMC_CLOCK_25000KHZ = 4,
    SDMMC_CLOCK_50000KHZ = 5,
    SDMMC_CLOCK_208000KHZ = 6,
    SDMMC_CLOCK_100000KHZ = 7,
};

static const char *const bit_tlb[4] = {
    "1bit",
    "4bits",
    "8bits",
    "unknow"
};

struct timing_phase_path{
    int start;
    int end;
    int mid;
    int len;
};

#define MMC_SECTOR_ADDR            0x40000000

#define EVENT_NON                  0x0000
#define EVENT_INSER                0x0001
#define EVENT_REMOV                0x0002
#define EVENT_USER                 0x0010

#define BYTE_CNT                   0x0200
#define RTK_NORMAL_SPEED           0x0000
#define RTK_HIGH_SPEED             0x0001

#define ON                         0
#define OFF                        1
#define GPIO_OUT                   1
#define GPIO_IN                    0
#define GPIO_HIGH                  1
#define GPIO_LOW                   0

/* MMC configure1, for SD_CONFIGURE1 */
#define SD30_FIFO_RST              (1 << 4)
#define SD1_R0                     (SDCLK_DIV | SDCLK_DIV_256 | RST_RDWR_FIFO)

/* MMC configure2, for SD_CONFIGURE2, response type */
#define SD_R0                      (RESP_TYPE_NON | CRC7_CHK_DIS)
#define SD_R1                      RESP_TYPE_6B
#define SD_R1b                     RESP_TYPE_6B
#define SD_R2                      RESP_TYPE_17B
#define SD_R3                      (RESP_TYPE_6B | CRC7_CHK_DIS)
#define SD_R4                      RESP_TYPE_6B
#define SD_R5                      RESP_TYPE_6B
#define SD_R6                      RESP_TYPE_6B
#define SD_R7                      RESP_TYPE_6B
#define SD_R_NO                    0xFF

static const unsigned char rtk_sdmmc_cmdcode[60][2] = {
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, //0~3
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, //4~7
    {SD_CMD_UNKNOW ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R1 }, //8~11
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, //12~15
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_AUTOREAD2  ,SD_R1 }, {SD_AUTOREAD1  ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R1 }, //16~19
    {SD_AUTOWRITE1 ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, //20~23
#ifndef CMD25_USE_SD_AUTOWRITE2
    {SD_AUTOWRITE2 ,SD_R1 }, {SD_AUTOWRITE1 ,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, //24~27
#else
    {SD_AUTOWRITE2 ,SD_R1 }, {SD_AUTOWRITE2 ,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, {SD_NORMALWRITE,SD_R1 }, //24~27
#endif
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_NORMALREAD ,SD_R1 }, //28~31
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, //32~35
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, //36~39
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, //40~43
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, //44~47
    {SD_AUTOREAD2 ,SD_R1 }, {SD_AUTOWRITE2 ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_NORMALREAD ,SD_R1 }, //48~51
    {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_CMD_UNKNOW ,SD_R0 }, //52~55
    {SD_AUTOREAD2 ,SD_R1 }, {SD_CMD_UNKNOW ,SD_R0 }, {SD_AUTOREAD2  ,SD_R1 }, {SD_AUTOWRITE2 ,SD_R1 }  //56~59
};

/* MMC configure3 , for SD_CONFIGURE3 */
#define SD30_CLK_STOP              (1 << 4)
#define SD2_R0                     (RESP_CHK_EN | ADDR_BYTE_MODE)

#define RTKCR_FCARD_DETECTED       (0x01 << 0) /* Card is detected */
#define RTKCR_FCARD_SELECTED       (0x01 << 1) /* Card is detected */
#define RTKCR_USER_PARTITION       (0x01 << 2) /* card is working on normal partition */

#define MMC_ERR_NONE               0
#define MMC_ERR_TIMEOUT            1
#define MMC_ERR_BADCRC             2
#define MMC_ERR_RMOVE              3
#define MMC_ERR_FAILED             4
#define MMC_ERR_INVALID            5

#define RCA_SHIFTER                16

#define CR_TRANS_OK                0x00
#define CR_TRANSFER_TO             0x01
#define CR_BUF_FULL_TO             0x02
#define CR_DMA_FAIL                0x03
#define CR_TRANSFER_FAIL           0x04

#define RTK_FAIL                   3 /* DMA error & cmd parser error */
#define RTK_RMOV                   2 /* card removed */
#define RTK_TOUT                   1 /* time out include DMA finish & cmd parser finish */
#define RTK_SUCC                   0

#define MMC_EXT_READ_SINGLE        48
#define MMC_EXT_WRITE_SINGLE       49
#define MMC_EXT_READ_MULTIPLE      58
#define MMC_EXT_WRITE_MULTIPLE     59

#endif
