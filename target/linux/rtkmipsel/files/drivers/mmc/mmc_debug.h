#ifndef _MMC_RTK_DEBUG_H
#define _MMC_RTK_DEBUG_H

//#define MMC_DEBUG//////////////////////
#ifdef MMC_DEBUG
    #define mmcinfo(fmt, args...) \
               printk(KERN_INFO "mmc:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcinfo(fmt, args...)
#endif

//#define MMC_CORE//////////////////////
#ifdef MMC_CORE
    #define mmcore(fmt, args...) \
           printk(KERN_INFO "mmcore:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcore(fmt, args...)
#endif

//#define MMC_CARD//////////////////////
#ifdef MMC_CARD
    #define mmcard(fmt, args...) \
           printk(KERN_INFO "mmcard:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcard(fmt, args...)
#endif

//#define MMC_RTK//////////////////////
#ifdef MMC_RTK
    #define mmcrtk(fmt, args...) \
           printk(KERN_INFO "mmcrtk:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcrtk(fmt, args...)
#endif

//#define MMC_SPEC//////////////////////
#ifdef MMC_SPEC
    #define mmcspec(fmt, args...) \
           printk(KERN_INFO "mmcspec:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcspec(fmt, args...)
#endif

//#define MMC_MSG1//////////////////////
#ifdef MMC_MSG1
    #define mmcmsg1(fmt, args...) \
           printk(KERN_INFO "mmcmsg1:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcmsg1(fmt, args...)
#endif

//#define MMC_MSG2//////////////////////
#ifdef MMC_MSG2
    #define mmcmsg2(fmt, args...) \
           printk(KERN_INFO "mmcmsg2:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcmsg2(fmt, args...)
#endif

//#define MMC_MSG3//////////////////////
#ifdef MMC_MSG3
    #define mmcmsg3(fmt, args...) \
           printk(KERN_INFO "" fmt, ## args)
           //printk(KERN_INFO "mmcmsg3:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define mmcmsg3(fmt, args...)
#endif

//#define MMC_TRH//////////////////////
#ifdef MMC_TRH
    #define trhmsg(fmt, args...) \
           printk(KERN_INFO "" fmt,## args)
           //printk(KERN_INFO "trhmsg:%s(%d):" fmt, __func__ ,__LINE__,## args)
#else
    #define trhmsg(fmt, args...)
#endif

//#define SHOW_CSD
//#define SHOW_EXT_CSD
//#define SHOW_SWITCH_DATA
//#define SHOW_CID
//#define SHOW_SCR

//#define SHOW_MS_PRD
//#define SHOW_MMC_PRD
//#define  SHOW_INT_STATUS

//#define GPIO_CTL
//#define TEST_POWER_RESCYCLE

//#define OPEN_TEST_PORT

//#define WR_WORK_AROUND_EN
//#define WR_WORK_AROUND_EN_TEST
//#define HW_ISSUE_RESET
//#define MONI_MEM_TRASH
//#define END_DELAY_EN
//#define FPGA_TEST_CASE

#endif
