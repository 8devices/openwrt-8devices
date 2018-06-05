/*
 * SHEIPA SPI controller driver
 *
 * Author: Realtek PSP Group
 *
 * Copyright 2015, Realtek Semiconductor Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DW_COMMON_H
#define _DW_COMMON_H

/* SPIC config */
#define ps_DW_APB_SPI_FLASH_BASE BSP_PS_I_SSI_BASE

#define ps_CC_SPI_FLASH_NUM_SLAVES            1
#define ps_CC_SPI_FLASH_HAS_DMA               0
#define ps_CC_SPI_FLASH_DMA_TX_SGL_STATUS     1
#define ps_CC_SPI_FLASH_DMA_RX_SGL_STATUS     1
#define ps_CC_SPI_FLASH_RX_FIFO_DEPTH         0x20
#define ps_CC_SPI_FLASH_TX_FIFO_DEPTH         0x20
#define ps_CC_SPI_FLASH_RX_ABW                6
#define ps_CC_SPI_FLASH_TX_ABW                6
#define ps_CC_SPI_FLASH_INTR_POL              1
#define ps_CC_SPI_FLASH_INTR_IO               1
#define ps_CC_SPI_FLASH_INDIVIDUAL            0
#define ps_CC_SPI_FLASH_ID                    0x0
#define ps_CC_SPI_FLASH_HC_FRF                0
#define ps_CC_SPI_FLASH_DFLT_FRF              0x0
#define ps_CC_SPI_FLASH_DFLT_SCPOL            0x0
#define ps_CC_SPI_FLASH_DFLT_SCPH             0x0
#define ps_CC_SPI_FLASH_CLK_PERIOD            400
#define ps_CC_SPI_FLASH_VERSION_ID            0x0

/* DW error code*/
#define DW_EPERM            1	/* operation not permitted */
#define DW_EIO              5	/* I/O error */
#define DW_ENXIO            6	/* no such device or address */
#define DW_ENOMEM           12	/* out of memory */
#define DW_EACCES           13	/* permission denied */
#define DW_EBUSY            16	/* device or resource busy */
#define DW_ENODEV           19	/* no such device */
#define DW_EINVAL           22	/* invalid argument */
#define DW_ENOSPC           28	/* no space left on device */
#define DW_ENOSYS           38	/* function not implemented/supported */
#define DW_ECHRNG           44	/* channel number out of range */
#define DW_ENODATA          61	/* no data available */
#define DW_ETIME            62	/* timer expired */
#define DW_EPROTO           71	/* protocol error */

/* Constant definitions for various bits of a 32-bit word. */
#define DW_BIT0     0x00000001
#define DW_BIT1     0x00000002
#define DW_BIT2     0x00000004
#define DW_BIT3     0x00000008
#define DW_BIT4     0x00000010
#define DW_BIT5     0x00000020
#define DW_BIT6     0x00000040
#define DW_BIT7     0x00000080
#define DW_BIT8     0x00000100
#define DW_BIT9     0x00000200
#define DW_BIT10    0x00000400
#define DW_BIT11    0x00000800
#define DW_BIT12    0x00001000
#define DW_BIT13    0x00002000
#define DW_BIT14    0x00004000
#define DW_BIT15    0x00008000
#define DW_BIT16    0x00010000
#define DW_BIT17    0x00020000
#define DW_BIT18    0x00040000
#define DW_BIT19    0x00080000
#define DW_BIT20    0x00100000
#define DW_BIT21    0x00200000
#define DW_BIT22    0x00400000
#define DW_BIT23    0x00800000
#define DW_BIT24    0x01000000
#define DW_BIT25    0x02000000
#define DW_BIT26    0x04000000
#define DW_BIT27    0x08000000
#define DW_BIT28    0x10000000
#define DW_BIT29    0x20000000
#define DW_BIT30    0x40000000
#define DW_BIT31    0x80000000
#define DW_BITS_ALL 0xFFFFFFFF

/* Returns the width of the specified bit-field. */
#define DW_BIT_WIDTH(__bfws)    ((uint32_t) (bfw ## __bfws))

/* Returns the offset of the specified bit-field. */
#define DW_BIT_OFFSET(__bfws)   ((uint32_t) (bfo ## __bfws))

/* Returns a mask with the bits to be addressed set and all others cleared. */
#define DW_BITS_MASK(__bfws, __bits) ((uint32_t) (( __bfws) == 32) ?  \
        0x0 : ( ((0xffffffff)>>(32- __bits)) <<(__bfws)))

#define DW_BIT_MASK(__bfws) ((uint32_t) (( __bfws) == 32) ?  \
        0x0 : ( 0x1 <<(__bfws)))

#define DW_BIT_MASK_WIDTH(__bfws, __bits) ((uint32_t) (( __bfws) == 32) ?  \
        0xFFFFFFFF : (((1<< (__bits)) - 1) << (__bfws)))

/* Clear the specified bits. */
#define DW_BITS_CLEAR(__datum, __bfws, __bits)                      \
    ((__datum) = ((uint32_t) (__datum) & ~DW_BITS_MASK(__bfws, __bits)))

#define DW_BIT_CLEAR(__datum, __bfws)                               \
    ((__datum) = ((uint32_t) (__datum) & ~DW_BIT_MASK(__bfws)))

/*
 * Returns the relevant bits masked from the data word, still at their
 * original offset.
 */
#define DW_BIT_GET_UNSHIFTED(__datum, __bfws)                       \
    ((uint32_t) ((__datum) & DW_BIT_MASK(__bfws)))

/*
 * Returns the relevant bits masked from the data word shifted to bit
 * zero (i.e. access the specifed bits from a word of data as an
 * integer value).
 */
#define DW_BIT_GET(__datum, __bfws)                                 \
    ((uint32_t) (((__datum) & DW_BIT_MASK(__bfws)) >>               \
                 (__bfws)))

/*
 * Place the specified value into the specified bits of a word of data
 * (first the data is read, and the non-specified bits are re-written).
 */
#define DW_BITS_SET(__datum, __bfws, __bits)                          \
    ((__datum) = ((uint32_t) (__datum) & ~DW_BITS_MASK(__bfws, __bits)) |    \
            (DW_BITS_MASK(__bfws, __bits)))

#define DW_BIT_SET(__datum, __bfws)                          \
    ((__datum) = ((uint32_t) (__datum) & ~DW_BIT_MASK(__bfws)) |    \
            (DW_BIT_MASK(__bfws)))

#define DW_BITS_SET_VAL(__datum, __bfws, __val, bit_num)                          \
    ((__datum) = ((uint32_t) (__datum) & ~DW_BIT_MASK_WIDTH(__bfws, bit_num)) |    \
            ((__val << (__bfws)) & DW_BIT_MASK_WIDTH(__bfws, bit_num)))

/*
 * Place the specified value into the specified bits of a word of data
 * without reading first - for sensitive interrupt type registers
 */
#define DW_BIT_SET_NOREAD(__datum, __bfws, __val)                   \
    ((uint32_t) ((__datum) = (((__val) << (bfo ## __bfws)) &        \
                              DW_BIT_MASK(__bfws))))

/* Shift the specified value into the desired bits. */
#define DW_BIT_BUILD(__bfws, __val)                                 \
    ((uint32_t) (((__val) << (bfo ## __bfws)) & DW_BIT_MASK(__bfws)))

#endif
