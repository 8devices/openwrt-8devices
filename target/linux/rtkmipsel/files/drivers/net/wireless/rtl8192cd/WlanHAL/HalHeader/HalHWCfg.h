#ifndef __HAL_HW_CFG_H__
#define __HAL_HW_CFG_H__

/*-------------------------Modification Log-----------------------------------
    20120416 KaiYuan Add BIT Define
-------------------------Modification Log-----------------------------------*/


/*--------------------------Define -------------------------------------------*/
#ifndef BIT
    #define BIT(x)		(1 << (x))
#endif
#define SUPPORT_CHIP_8723A      BIT1
#define SUPPORT_CHIP_8188E      BIT2
#define SUPPORT_CHIP_8881A      BIT3
#define SUPPORT_CHIP_8812A      BIT4
#define SUPPORT_CHIP_8821A      BIT5
#define SUPPORT_CHIP_8723B      BIT6
#define SUPPORT_CHIP_8192E      BIT7
#define SUPPORT_CHIP_8814A      BIT8

//Marco Utility
#define SUPPORT_CHIP_ALL	(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E| \
                                SUPPORT_CHIP_8881A|SUPPORT_CHIP_8812A| \
                                SUPPORT_CHIP_8821A|SUPPORT_CHIP_8723B| \
                                SUPPORT_CHIP_8192E | SUPPORT_CHIP_8814A)

//Compile Option
//#define CONFIG_WLANREG_SUPPORT	(SUPPORT_CHIP_8192E|SUPPORT_CHIP_8881A | SUPPORT_CHIP_8814A)


#endif//__HAL_HW_CFG_H__



