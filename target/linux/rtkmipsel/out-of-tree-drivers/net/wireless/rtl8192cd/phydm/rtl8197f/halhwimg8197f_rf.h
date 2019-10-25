/****************************************************************************** 
* 
* Copyright(c) 2007 - 2017 Realtek Corporation. 
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of version 2 of the GNU General Public License as 
* published by the Free Software Foundation. 
* 
* This program is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
* more details. 
* 
******************************************************************************/

/*Image2HeaderVersion: R2 1.2.1 non coding style*/
#if (RTL8197F_SUPPORT == 1)
#ifndef __INC_MP_RF_HW_IMG_8197F_H
#define __INC_MP_RF_HW_IMG_8197F_H


/******************************************************************************
*                           RadioA.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_RadioA(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_RadioA(void);

/******************************************************************************
*                           RadioB.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_RadioB(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_RadioB(void);

/******************************************************************************
*                           TxPowerTrack.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack(void);

/******************************************************************************
*                           TxPowerTrack_Type0.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type0(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type0(void);

/******************************************************************************
*                           TxPowerTrack_Type1.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type1(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type1(void);

/******************************************************************************
*                           TxPowerTrack_Type2.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type2(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type2(void);

/******************************************************************************
*                           TxPowerTrack_Type3.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type3(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type3(void);

/******************************************************************************
*                           TxPowerTrack_Type4.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type4(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type4(void);

/******************************************************************************
*                           TxPowerTrack_Type5.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type5(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type5(void);

/******************************************************************************
*                           TxPowerTrack_Type6.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type6(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type6(void);

/******************************************************************************
*                           TxPowerTrack_Type7.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8197F_TxPowerTrack_Type7(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8197F_TxPowerTrack_Type7(void);

#endif
#endif /* end of HWIMG_SUPPORT*/

