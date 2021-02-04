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
#if (RTL8822B_SUPPORT == 1)
#ifndef __INC_MP_BB_HW_IMG_8822B_H
#define __INC_MP_BB_HW_IMG_8822B_H


/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_AGC_TAB(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_AGC_TAB(void);

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG(void);

/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG(void);

/******************************************************************************
*                           PHY_REG_PG_type12.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type12(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type12(void);

/******************************************************************************
*                           PHY_REG_PG_type15.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type15(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type15(void);

/******************************************************************************
*                           PHY_REG_PG_type16.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type16(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type16(void);

/******************************************************************************
*                           PHY_REG_PG_type17.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type17(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type17(void);

/******************************************************************************
*                           PHY_REG_PG_type2.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type2(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type2(void);

/******************************************************************************
*                           PHY_REG_PG_type3.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type3(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type3(void);

/******************************************************************************
*                           PHY_REG_PG_type4.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type4(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type4(void);

/******************************************************************************
*                           PHY_REG_PG_type5.TXT
******************************************************************************/

void
ODM_ReadAndConfig_MP_8822B_PHY_REG_PG_type5(/* TC: Test Chip, MP: MP Chip*/
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte ODM_GetVersion_MP_8822B_PHY_REG_PG_type5(void);

#endif
#endif /* end of HWIMG_SUPPORT*/

