#ifndef	__PHYDM_IQK_8821C_H__
#define    __PHYDM_IQK_8821C_H__

#if (RTL8821C_SUPPORT == 1)


/*--------------------------Define Parameters-------------------------------*/
#define		MAC_REG_NUM_8821C 2
#define		BB_REG_NUM_8821C 9
#define		RF_REG_NUM_8821C 5
#define		NUM_8821C 1

#define	LOK_delay_8821C 1
#define	IQK_delay_8821C 1
#define	WBIQK_delay_8821C 1

#define TXIQK 0
#define RXIQK 1

/*---------------------------End Define Parameters-------------------------------*/


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
VOID 
DoIQK_8821C(
	PVOID	pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	);
#else
VOID 
DoIQK_8821C(
	PVOID		pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	);
#endif

VOID	
PHY_IQCalibrate_8821C(	
	IN	PVOID		pDM_VOID,
	IN	BOOLEAN		reset
	);

#else	/* (RTL8821C_SUPPORT == 0)*/

#define PHY_IQCalibrate_8821C(_pDM_VOID, _reset)

#endif	/* RTL8821C_SUPPORT */

 #endif	/* #ifndef __PHYDM_IQK_8821C_H__*/

