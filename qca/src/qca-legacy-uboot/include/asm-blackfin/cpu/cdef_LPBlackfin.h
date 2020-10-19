/*
 * cdef_LPBlackfin.h
 *
 * This file is subject to the terms and conditions of the GNU Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Non-GPL License also available as part of VisualDSP++
 *
 * http://www.analog.com/processors/resources/crosscore/visualDspDevSoftware.html
 *
 * (c) Copyright 2001-2005 Analog Devices, Inc. All rights reserved
 *
 * This file under source code control, please send bugs or changes to:
 * dsptools.support@analog.com
 *
 */

#ifndef _CDEF_LPBLACKFIN_H
#define _CDEF_LPBLACKFIN_H

/*
 * #if !defined(__ADSPLPBLACKFIN__)
 * #warning cdef_LPBlackfin.h should only be included for 532 compatible chips.
 * #endif
 */
#include <asm/cpu/def_LPBlackfin.h>

/* Cache & SRAM Memory */
#define pSRAM_BASE_ADDRESS ((volatile void **)SRAM_BASE_ADDRESS)
#define pDMEM_CONTROL ((volatile unsigned long *)DMEM_CONTROL)
#define pDCPLB_STATUS ((volatile unsigned long *)DCPLB_STATUS)
#define pDCPLB_FAULT_ADDR ((volatile void **)DCPLB_FAULT_ADDR)

/* #define MMR_TIMEOUT 0xFFE00010 */	/* Memory-Mapped Register Timeout Register */
#define pDCPLB_ADDR0 ((volatile void **)DCPLB_ADDR0)
#define pDCPLB_ADDR1 ((volatile void **)DCPLB_ADDR1)
#define pDCPLB_ADDR2 ((volatile void **)DCPLB_ADDR2)
#define pDCPLB_ADDR3 ((volatile void **)DCPLB_ADDR3)
#define pDCPLB_ADDR4 ((volatile void **)DCPLB_ADDR4)
#define pDCPLB_ADDR5 ((volatile void **)DCPLB_ADDR5)
#define pDCPLB_ADDR6 ((volatile void **)DCPLB_ADDR6)
#define pDCPLB_ADDR7 ((volatile void **)DCPLB_ADDR7)
#define pDCPLB_ADDR8 ((volatile void **)DCPLB_ADDR8)
#define pDCPLB_ADDR9 ((volatile void **)DCPLB_ADDR9)
#define pDCPLB_ADDR10 ((volatile void **)DCPLB_ADDR10)
#define pDCPLB_ADDR11 ((volatile void **)DCPLB_ADDR11)
#define pDCPLB_ADDR12 ((volatile void **)DCPLB_ADDR12)
#define pDCPLB_ADDR13 ((volatile void **)DCPLB_ADDR13)
#define pDCPLB_ADDR14 ((volatile void **)DCPLB_ADDR14)
#define pDCPLB_ADDR15 ((volatile void **)DCPLB_ADDR15)
#define pDCPLB_DATA0 ((volatile unsigned long *)DCPLB_DATA0)
#define pDCPLB_DATA1 ((volatile unsigned long *)DCPLB_DATA1)
#define pDCPLB_DATA2 ((volatile unsigned long *)DCPLB_DATA2)
#define pDCPLB_DATA3 ((volatile unsigned long *)DCPLB_DATA3)
#define pDCPLB_DATA4 ((volatile unsigned long *)DCPLB_DATA4)
#define pDCPLB_DATA5 ((volatile unsigned long *)DCPLB_DATA5)
#define pDCPLB_DATA6 ((volatile unsigned long *)DCPLB_DATA6)
#define pDCPLB_DATA7 ((volatile unsigned long *)DCPLB_DATA7)
#define pDCPLB_DATA8 ((volatile unsigned long *)DCPLB_DATA8)
#define pDCPLB_DATA9 ((volatile unsigned long *)DCPLB_DATA9)
#define pDCPLB_DATA10 ((volatile unsigned long *)DCPLB_DATA10)
#define pDCPLB_DATA11 ((volatile unsigned long *)DCPLB_DATA11)
#define pDCPLB_DATA12 ((volatile unsigned long *)DCPLB_DATA12)
#define pDCPLB_DATA13 ((volatile unsigned long *)DCPLB_DATA13)
#define pDCPLB_DATA14 ((volatile unsigned long *)DCPLB_DATA14)
#define pDCPLB_DATA15 ((volatile unsigned long *)DCPLB_DATA15)
#define pDTEST_COMMAND ((volatile unsigned long *)DTEST_COMMAND)

/* #define DTEST_INDEX            0xFFE00304 */ 	/* Data Test Index Register */
#define pDTEST_DATA0 ((volatile unsigned long *)DTEST_DATA0)
#define pDTEST_DATA1 ((volatile unsigned long *)DTEST_DATA1)

/*
 * # define DTEST_DATA2	0xFFE00408   Data Test Data Register
 * #define DTEST_DATA3	0xFFE0040C   Data Test Data Register
 */
#define pIMEM_CONTROL ((volatile unsigned long *)IMEM_CONTROL)
#define pICPLB_STATUS ((volatile unsigned long *)ICPLB_STATUS)
#define pICPLB_FAULT_ADDR ((volatile void **)ICPLB_FAULT_ADDR)
#define pICPLB_ADDR0 ((volatile void **)ICPLB_ADDR0)
#define pICPLB_ADDR1 ((volatile void **)ICPLB_ADDR1)
#define pICPLB_ADDR2 ((volatile void **)ICPLB_ADDR2)
#define pICPLB_ADDR3 ((volatile void **)ICPLB_ADDR3)
#define pICPLB_ADDR4 ((volatile void **)ICPLB_ADDR4)
#define pICPLB_ADDR5 ((volatile void **)ICPLB_ADDR5)
#define pICPLB_ADDR6 ((volatile void **)ICPLB_ADDR6)
#define pICPLB_ADDR7 ((volatile void **)ICPLB_ADDR7)
#define pICPLB_ADDR8 ((volatile void **)ICPLB_ADDR8)
#define pICPLB_ADDR9 ((volatile void **)ICPLB_ADDR9)
#define pICPLB_ADDR10 ((volatile void **)ICPLB_ADDR10)
#define pICPLB_ADDR11 ((volatile void **)ICPLB_ADDR11)
#define pICPLB_ADDR12 ((volatile void **)ICPLB_ADDR12)
#define pICPLB_ADDR13 ((volatile void **)ICPLB_ADDR13)
#define pICPLB_ADDR14 ((volatile void **)ICPLB_ADDR14)
#define pICPLB_ADDR15 ((volatile void **)ICPLB_ADDR15)
#define pICPLB_DATA0 ((volatile unsigned long *)ICPLB_DATA0)
#define pICPLB_DATA1 ((volatile unsigned long *)ICPLB_DATA1)
#define pICPLB_DATA2 ((volatile unsigned long *)ICPLB_DATA2)
#define pICPLB_DATA3 ((volatile unsigned long *)ICPLB_DATA3)
#define pICPLB_DATA4 ((volatile unsigned long *)ICPLB_DATA4)
#define pICPLB_DATA5 ((volatile unsigned long *)ICPLB_DATA5)
#define pICPLB_DATA6 ((volatile unsigned long *)ICPLB_DATA6)
#define pICPLB_DATA7 ((volatile unsigned long *)ICPLB_DATA7)
#define pICPLB_DATA8 ((volatile unsigned long *)ICPLB_DATA8)
#define pICPLB_DATA9 ((volatile unsigned long *)ICPLB_DATA9)
#define pICPLB_DATA10 ((volatile unsigned long *)ICPLB_DATA10)
#define pICPLB_DATA11 ((volatile unsigned long *)ICPLB_DATA11)
#define pICPLB_DATA12 ((volatile unsigned long *)ICPLB_DATA12)
#define pICPLB_DATA13 ((volatile unsigned long *)ICPLB_DATA13)
#define pICPLB_DATA14 ((volatile unsigned long *)ICPLB_DATA14)
#define pICPLB_DATA15 ((volatile unsigned long *)ICPLB_DATA15)
#define pITEST_COMMAND ((volatile unsigned long *)ITEST_COMMAND)

/* #define ITEST_INDEX 0xFFE01304 */	/* Instruction Test Index Register */
#define pITEST_DATA0 ((volatile unsigned long *)ITEST_DATA0)
#define pITEST_DATA1 ((volatile unsigned long *)ITEST_DATA1)

/* Event/Interrupt Registers */
#define pEVT0 ((volatile void **)EVT0)
#define pEVT1 ((volatile void **)EVT1)
#define pEVT2 ((volatile void **)EVT2)
#define pEVT3 ((volatile void **)EVT3)
#define pEVT4 ((volatile void **)EVT4)
#define pEVT5 ((volatile void **)EVT5)
#define pEVT6 ((volatile void **)EVT6)
#define pEVT7 ((volatile void **)EVT7)
#define pEVT8 ((volatile void **)EVT8)
#define pEVT9 ((volatile void **)EVT9)
#define pEVT10 ((volatile void **)EVT10)
#define pEVT11 ((volatile void **)EVT11)
#define pEVT12 ((volatile void **)EVT12)
#define pEVT13 ((volatile void **)EVT13)
#define pEVT14 ((volatile void **)EVT14)
#define pEVT15 ((volatile void **)EVT15)
#define pIMASK ((volatile unsigned long *)IMASK)
#define pIPEND ((volatile unsigned long *)IPEND)
#define pILAT ((volatile unsigned long *)ILAT)

/* Core Timer Registers */
#define pTCNTL ((volatile unsigned long *)TCNTL)
#define pTPERIOD ((volatile unsigned long *)TPERIOD)
#define pTSCALE ((volatile unsigned long *)TSCALE)
#define pTCOUNT ((volatile unsigned long *)TCOUNT)

/* Debug/MP/Emulation Registers */
#define pDSPID ((volatile unsigned long *)DSPID)
#define pDBGCTL ((volatile unsigned long *)DBGCTL)
#define pDBGSTAT ((volatile unsigned long *)DBGSTAT)
#define pEMUDAT ((volatile unsigned long *)EMUDAT)

/* Trace Buffer Registers */
#define pTBUFCTL ((volatile unsigned long *)TBUFCTL)
#define pTBUFSTAT ((volatile unsigned long *)TBUFSTAT)
#define pTBUF ((volatile void **)TBUF)

/* Watch Point Control Registers */
#define pWPIACTL ((volatile unsigned long *)WPIACTL)
#define pWPIA0 ((volatile void **)WPIA0)
#define pWPIA1 ((volatile void **)WPIA1)
#define pWPIA2 ((volatile void **)WPIA2)
#define pWPIA3 ((volatile void **)WPIA3)
#define pWPIA4 ((volatile void **)WPIA4)
#define pWPIA5 ((volatile void **)WPIA5)
#define pWPIACNT0 ((volatile unsigned long *)WPIACNT0)
#define pWPIACNT1 ((volatile unsigned long *)WPIACNT1)
#define pWPIACNT2 ((volatile unsigned long *)WPIACNT2)
#define pWPIACNT3 ((volatile unsigned long *)WPIACNT3)
#define pWPIACNT4 ((volatile unsigned long *)WPIACNT4)
#define pWPIACNT5 ((volatile unsigned long *)WPIACNT5)
#define pWPDACTL ((volatile unsigned long *)WPDACTL)
#define pWPDA0 ((volatile void **)WPDA0)
#define pWPDA1 ((volatile void **)WPDA1)
#define pWPDACNT0 ((volatile unsigned long *)WPDACNT0)
#define pWPDACNT1 ((volatile unsigned long *)WPDACNT1)
#define pWPSTAT ((volatile unsigned long *)WPSTAT)

/* Performance Monitor Registers */
#define pPFCTL ((volatile unsigned long *)PFCTL)
#define pPFCNTR0 ((volatile unsigned long *)PFCNTR0)
#define pPFCNTR1 ((volatile unsigned long *)PFCNTR1)

/* #define IPRIO 0xFFE02110 */ /* Core Interrupt Priority Register */

#endif	/* _CDEF_LPBLACKFIN_H */
