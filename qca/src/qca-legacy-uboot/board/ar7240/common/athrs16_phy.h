/* 
 * Copyright (c) 2014 Qualcomm Atheros, Inc.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#ifndef _ATHRS16_PHY_H
#define _ATHRS16_PHY_H

/*****************/
/* PHY Registers */
/*****************/
#define ATHR_PHY_CONTROL                 0
#define ATHR_PHY_STATUS                  1
#define ATHR_PHY_ID1                     2
#define ATHR_PHY_ID2                     3
#define ATHR_AUTONEG_ADVERT              4
#define ATHR_LINK_PARTNER_ABILITY        5
#define ATHR_AUTONEG_EXPANSION           6
#define ATHR_NEXT_PAGE_TRANSMIT          7
#define ATHR_LINK_PARTNER_NEXT_PAGE      8
#define ATHR_1000BASET_CONTROL           9
#define ATHR_1000BASET_STATUS            10
#define ATHR_PHY_SPEC_CONTROL            16
#define ATHR_PHY_SPEC_STATUS             17
#define ATHR_DEBUG_PORT_ADDRESS          29
#define ATHR_DEBUG_PORT_DATA             30

/* ATHR_PHY_CONTROL fields */
#define ATHR_CTRL_SOFTWARE_RESET                    0x8000
#define ATHR_CTRL_SPEED_LSB                         0x2000
#define ATHR_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ATHR_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define ATHR_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define ATHR_CTRL_SPEED_MSB                         0x0040

#define ATHR_RESET_DONE(phy_control)                   \
    (((phy_control) & (ATHR_CTRL_SOFTWARE_RESET)) == 0)
    
/* Phy status fields */
#define ATHR_STATUS_AUTO_NEG_DONE                   0x0020

#define ATHR_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (ATHR_STATUS_AUTO_NEG_DONE)) ==                    \
        (ATHR_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define ATHR_LINK_100BASETX_FULL_DUPLEX       0x0100
#define ATHR_LINK_100BASETX                   0x0080
#define ATHR_LINK_10BASETX_FULL_DUPLEX        0x0040
#define ATHR_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define ATHR_ADVERTISE_NEXT_PAGE              0x8000
#define ATHR_ADVERTISE_ASYM_PAUSE             0x0800
#define ATHR_ADVERTISE_PAUSE                  0x0400
#define ATHR_ADVERTISE_100FULL                0x0100
#define ATHR_ADVERTISE_100HALF                0x0080  
#define ATHR_ADVERTISE_10FULL                 0x0040  
#define ATHR_ADVERTISE_10HALF                 0x0020  

#define ATHR_ADVERTISE_ALL (ATHR_ADVERTISE_ASYM_PAUSE | ATHR_ADVERTISE_PAUSE | \
                            ATHR_ADVERTISE_10HALF | ATHR_ADVERTISE_10FULL | \
                            ATHR_ADVERTISE_100HALF | ATHR_ADVERTISE_100FULL)
                       
/* 1000BASET_CONTROL */
#define ATHR_ADVERTISE_1000FULL               0x0200

/* Phy Specific status fields */
#define ATHER_STATUS_LINK_MASK                0xC000
#define ATHER_STATUS_LINK_SHIFT               14
#define ATHER_STATUS_FULL_DEPLEX              0x2000
#define ATHR_STATUS_LINK_PASS                 0x0400 
#define ATHR_STATUS_RESOVLED                  0x0800

/*phy debug port  register */
#define ATHER_DEBUG_SERDES_REG                5

/* Serdes debug fields */
#define ATHER_SERDES_BEACON                   0x0100

/* S16 CSR Registers */

#define S16_PORT_STATUS_REGISTER0                0x0100 
#define S16_PORT_STATUS_REGISTER1                0x0200
#define S16_PORT_STATUS_REGISTER2                0x0300
#define S16_PORT_STATUS_REGISTER3                0x0400
#define S16_PORT_STATUS_REGISTER4                0x0500
#define S16_PORT_STATUS_REGISTER5                0x0600

#define S16_RATE_LIMIT_REGISTER0                 0x010C
#define S16_RATE_LIMIT_REGISTER1                 0x020C
#define S16_RATE_LIMIT_REGISTER2                 0x030C
#define S16_RATE_LIMIT_REGISTER3                 0x040C
#define S16_RATE_LIMIT_REGISTER4                 0x050C
#define S16_RATE_LIMIT_REGISTER5                 0x060C

#define S16_PORT_CONTROL_REGISTER0               0x0104
#define S16_PORT_CONTROL_REGISTER1               0x0204
#define S16_PORT_CONTROL_REGISTER2               0x0304
#define S16_PORT_CONTROL_REGISTER3               0x0404
#define S16_PORT_CONTROL_REGISTER4               0x0504
#define S16_PORT_CONTROL_REGISTER5               0x0604

#define S16_CPU_PORT_REGISTER                    0x0078
#define S16_MDIO_CTRL_REGISTER                   0x0098

#define S16_ARL_TBL_FUNC_REG0                0x0050
#define S16_ARL_TBL_FUNC_REG1                0x0054
#define S16_ARL_TBL_FUNC_REG2                0x0058
#define S16_FLD_MASK_REG                     0x002c
#define S16_ARL_TBL_CTRL_REG                 0x005c
#define S16_GLOBAL_INTR_REG                  0x10
#define S16_GLOBAL_INTR_MASK_REG             0x14
#define S16_PWR_ON_STRAP_REG                 0x8


#define S16_ENABLE_CPU_BROADCAST             (1 << 26)

#define S16_PHY_LINK_CHANGE_REG 		     0x4
#define S16_PHY_LINK_UP 		             0x400
#define S16_PHY_LINK_DOWN 		             0x800
#define S16_PHY_LINK_DUPLEX_CHANGE 		     0x2000
#define S16_PHY_LINK_SPEED_CHANGE		     0x4000
#define S16_PHY_LINK_INTRS			     (PHY_LINK_UP | PHY_LINK_DOWN | PHY_LINK_DUPLEX_CHANGE | PHY_LINK_SPEED_CHANGE)


#ifndef BOOL
#define BOOL    int
#endif

/*add feature define here*/
//#define FULL_FEATURE

#ifdef CONFIG_AR7242_S16_PHY
#undef HEADER_REG_CONF
#undef HEADER_EN
#endif

void athrs16_reg_init(void);
int athrs16_phy_is_up(int unit);
int athrs16_phy_is_fdx(int unit);
int athrs16_phy_speed(int unit);
BOOL athrs16_phy_setup(int unit);

#endif


