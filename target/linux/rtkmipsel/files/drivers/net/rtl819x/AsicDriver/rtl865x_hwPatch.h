/*
 *
 *	The header file collects the information about the IC's different Version/Sub-version.
 *		=> It's used by DRIVER to determine how to implement features for different ICs.
 *
 */

#ifndef RTL865x_hwPatch_H
#define RTL865x_hwPatch_H

/* Chip Version */
#define RTL865X_CHIP_VER_RTL865XB	0x01
#define RTL865X_CHIP_VER_RTL865XC	0x02
#define RTL865X_CHIP_VER_RTL8196B	0x03
#define RTL865X_CHIP_VER_RTL8196C	0x04

/* Revision ID of each chip */
#define RTL865X_CHIP_REV_A		0x00
#define RTL865X_CHIP_REV_B		0x01
#define RTL865X_CHIP_REV_C		0x02
#define RTL865X_CHIP_REV_D		0x03
#define RTL865X_CHIP_REV_E		0x04

#if 1
#define RTL819X_TLU_CHECK		1
#else

/* RTL865xC */
/* ================================== */
/* Rev-A: It would cause HW process when dynamically stop HW table lookup. */
#define  RTL819X_TLU_CHECK		(	(( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL865XC	/* CHIP */ ) && \
										( RtkHomeGatewayChipRevisionID >= RTL865X_CHIP_REV_B		/* REV */ ))	|| \
										( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL8196B		/* CHIP */ ) ||\
										( RtkHomeGatewayChipNameID == RTL865X_CHIP_VER_RTL8196C		/* CHIP */ ))
										
#endif	

#endif	

