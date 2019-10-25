/*****************************************************************************
 *	Copyright(c) 2009,  RealTEK Technology Inc. All Right Reserved.
 *
 * Module:	__INC_HAL8812REG_H
 *
 *
 * Note:	1. Define Mac register address and corresponding bit mask map
 *			
 *
 * Export:	Constants, macro, functions(API), global variables(None).
 *
 * Abbrev:	
 *
 * History:
 *		Data		Who		Remark 
 * 
 *****************************************************************************/

#ifdef RTK_AC_SUPPORT

#define BEAMFORM_MAX_ANT_SUPP	1
#define BEAMFORM_SOUNDING_DIMENSIONS	1


//=== VHT capability info field ===

#define MAX_MPDU_LENGTH_S	0
#define MAX_MPDU_LENGTH_E	1

#define CHL_WIDTH_S	2
#define CHL_WIDTH_E	3  

#define RX_LDPC_S	4   
#define RX_LDPC_E	4

#define SHORT_GI80M_S	5 
#define SHORT_GI80M_E	5

#define SHORT_GI160M_S	6
#define SHORT_GI160M_E	6

#define TX_STBC_S	7
#define TX_STBC_E	7

#define RX_STBC_S	8
#define RX_STBC_E	10

#define SU_BFER_S	11  
#define SU_BFER_E	11

#define SU_BFEE_S	12
#define SU_BFEE_E	12

#define MAX_ANT_SUPP_S	13
#define MAX_ANT_SUPP_E	15

#define SOUNDING_DIMENSIONS_S	16    
#define SOUNDING_DIMENSIONS_E	18

#define MU_BFER_S	19
#define MU_BFER_E	19

#define MU_BFEE_S	20
#define MU_BFEE_E	20

#define TXOP_PS_S	21
#define TXOP_PS_E 	21

#define HTC_VHT_S	22   
#define HTC_VHT_E	22 

#define MAX_RXAMPDU_FACTOR_S	23
#define MAX_RXAMPDU_FACTOR_E	25

#define LINK_ADAPTION_S	26
#define LINK_ADAPTION_E	27

#define RX_ANT_PC_S	28      
#define RX_ANT_PC_E 28

#define TX_ANT_PC_S	29 
#define TX_ANT_PC_E 29  

//30 - 31 Reserved 


//=== VHT supported mcs set field ===

#define MCS_RX_MAP_S	0
#define MCS_RX_MAP_E	15

#define MCS_RX_HIGHEST_RATE_S	16 
#define MCS_RX_HIGHEST_RATE_E	28

#define MCS_TX_MAP_S	0  //32-32
#define MCS_TX_MAP_E	15 //47-32

#define MCS_TX_HIGHEST_RATE_S	16	//48-32
#define MCS_TX_HIGHEST_RATE_E	28	//60-32


#endif
