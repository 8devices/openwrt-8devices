
#ifndef __INC_HALCOMPHYREG_BB_H
#define __INC_HALCOMPHYREG_BB_H

#if IS_RTL88XX_AC



//
// BB-PHY register PMAC 0x100 PHY 0x800 - 0xEFF
// 1. PMAC duplicate register due to connection: RF_Mode, TRxRN, NumOf L-STF
// 2. 0x800/0x900/0xA00/0xC00/0xD00/0xE00
// 3. RF register 0x00-2E
// 4. Bit Mask for BB/RF register
// 5. Other defintion for BB/RF R/W
//

//============Programming guide Start=====================
/*
    BB REG define rules 
    1. For all reg define, it should be prefixed by " REG_BB"


    RF REG define rules 
    1. For all reg define, it should be prefixed by " REG_RF"

*/
/**** page 8 ****/
#define    REG_BB_CCKENABLE_AC                           0x808 // OFDM/CCK block enable
#define    REG_BB_RXPATH_AC                              0x808    // Rx antenna
#define    REG_BB_TXPATH_AC                              0x80c    // Tx antenna
#define    REG_BB_AGC_TABLE_AC                           0x82c   // AGC tabel select
#define    REG_BB_PWED_TH_AC                             0x830   
#define    REG_BB_BWINDICATION_AC                        0x834
#define    REG_BB_CCAONSEC_AC                            0x838
#define    REG_BB_L1_Weight_Jaguar                       0x840
#define    REG_BB_L1PEAKTH_AC                            0x848
#define    REG_BB_FC_AREA_AC                             0x860   // fc_area 
#define    REG_BB_FPGA0_XB_RFINTERFACEOE_AC              0x864
#define    REG_BB_FPGA0_XAB_RFINTERFACESW_AC             0x870    // RF Interface Software Control
#define    REG_BB_FPGA0_XCD_RFINTERFACESW_AC             0x874
#define    REG_BB_FPGA0_XAB_RFPARAMETER_AC               0x878    // RF Parameter
#define    REG_BB_FPGA0_XCD_RFPARAMETER_AC               0x87c
#define    REG_BB_FPGA0_ANALOGPARAMETER1_AC              0x880    // Crystal cap setting RF-R/W protection for parameter4??
#define    REG_BB_FPGA0_ANALOGPARAMETER2_AC              0x884
#define    REG_BB_FPGA0_ANALOGPARAMETER3_AC              0x888
#define    REG_BB_FPGA0_ADDACLOCKEN_AC                   0x888    // enable ad/da clock1 for dual-phy
#define    REG_BB_FPGA0_ANALOGPARAMETER4_AC              0x88c
#define    REG_BB_EDCCA_AC                               0x8a4 // EDCCA
#define    REG_BB_RFMOD_AC                               0x8ac    //RF mode 
#define    REG_BB_HSSIREAD_AC                            0x8b0  // RF read addr
#define    REG_BB_FC_ENABL_VHT_AC                        0x8c0
#define    REG_BB_VHTLEN_USE_LSIG_AC                     0x8c3    // Use LSIG for VHT length
#define    REG_BB_ADC_BUF_CLK_AC                         0x8c4  
#define    REG_BB_ADC_BUF_CLK_2_AC                       0x8c8  

/**** For 8814 Rx spur issue ****/
#if (IS_RTL8814A_SERIES || IS_RTL8822B_SERIES)
#define    rNBI_Setting_Jaguar                           0x87c
#define    rCSI_Mask_Setting1_Jaguar                     0x874
#define    rCSI_Fix_Mask7_Jaguar                         0x89c
#define    rCSI_Fix_Mask6_Jaguar                         0x898
#define    rCSI_Fix_Mask0_Jaguar                         0x880
#define    rCSI_Fix_Mask1_Jaguar                         0x884

// Block & Path enable
#define    rOFDMCCKEN_Jaguar                             0x808 // OFDM/CCK block enable
#define    bOFDMEN_Jaguar                                0x20000000
#define    bCCKEN_Jaguar                                 0x10000000
#define    rADC_Buf_160_Clk_Jaguar                       0x8c4
#endif

/**** page 9 ****/
#define    REG_BB_SINGLETONE_CONTTX_AC                   0x914
#define    REG_BB_AGC_TABLE_AC_V2                        0x958
#define	REG_BB_TX_PATH_SEL_1					0x93c
#define	REG_BB_TX_PATH_SEL_2					0x940


/**** page a ****/
#define    REG_BB_CCK_SYSTEM_AC                          0xa00   // for cck sideband
#define    REG_BB_CCK_RX_AC                              0xa04    // for cck rx path selection
#define    REG_BB_CCK_CCA_AC                             0xa08    // cca threshold
// CCK TX scaling
#define    REG_BB_CCK_TXFILTER1_AC                       0xa20
#define    REG_BB_CCK_TXFILTER2_AC                       0xa24
#define    REG_BB_CCK_TXFILTER3_MSK_AC                   0xa28
#define    REG_BB_CCK_FALSEALARM_AC                      0xa5c // counter for cck false alarm

/**** page b ****/
// RSSI Dump
#define    REG_BB_A_RSSIDUMP_AC                          0xBF0
#define    REG_BB_B_RSSIDUMP_AC                          0xBF1
#define    REG_BB_S1_RXEVMDUMP_AC                        0xBF4 
#define    REG_BB_S2_RXEVMDUMP_AC                        0xBF5
#define    REG_BB_A_RXSNRDUMP_AC                         0xBF6
#define    REG_BB_B_RXSNRDUMP_AC                         0xBF7
#define    REG_BB_A_CFOSHORTDUMP_AC                      0xBF8 
#define    REG_BB_B_CFOSHORTDUMP_AC                      0xBFA
#define    REG_BB_A_CFOLONGDUMP_AC                       0xBEC
#define    REG_BB_B_CFOLONGDUMP_AC                       0xBEE
// YN: mask the following register definition temporarily
//#define    REG_BB_PDP_ANTA_AC                          0xb00  
//#define    REG_BB_PDP_ANTA_4_AC                        0xb04
//#define    REG_BB_CONFIG_PMPD_ANTA_AC                  0xb28
//#define    REG_BB_CONFIG_ANTA_AC                       0xb68
//#define    REG_BB_CONFIG_ANTB_AC                       0xb6c
//#define    REG_BB_PDP_ANTB_AC                          0xb70
//#define    REG_BB_PDP_ANTB_4_AC                        0xb74
//#define    REG_BB_CONFIG_PMPD_ANTB_AC                  0xb98
//#define    REG_BB_APK_AC                               0xbd8

/**** page c ****/
#define    REG_BB_A_RXIQC_AB_AC                           0xc10  //RxIQ imblance matrix coeff. A & B
#define    REG_BB_A_RXIQC_CD_AC                           0xc14  //RxIQ imblance matrix coeff. C & D
#define    REG_BB_AGC_TABLE_AC_V1                         0xc1c

// TX AGC 
#define    REG_BB_TXAGC_A_CCK11_CCK1_AC                   0xc20
#define    REG_BB_TXAGC_A_OFDM18_OFDM6_AC                 0xc24
#define    REG_BB_TXAGC_A_OFDM54_OFDM24_AC                0xc28
#define    REG_BB_TXAGC_A_MCS3_MCS0_AC                    0xc2c
#define    REG_BB_TXAGC_A_MCS7_MCS4_AC                    0xc30
#define    REG_BB_TXAGC_A_MCS11_MCS8_AC                   0xc34
#define    REG_BB_TXAGC_A_MCS15_MCS12_AC                  0xc38
#define    REG_BB_TXAGC_A_MCS19_MCS16_AC                  0xcd8
#define    REG_BB_TXAGC_A_MCS23_MCS20_AC				  0xcdc
#define    REG_BB_TXAGC_A_NSS1INDEX3_NSS1INDEX0_AC        0xc3c
#define    REG_BB_TXAGC_A_NSS1INDEX7_NSS1INDEX4_AC        0xc40
#define    REG_BB_TXAGC_A_NSS2INDEX1_NSS1INDEX8_AC        0xc44
#define    REG_BB_TXAGC_A_NSS2INDEX5_NSS2INDEX2_AC        0xc48
#define    REG_BB_TXAGC_A_NSS2INDEX9_NSS2INDEX6_AC        0xc4c
#define    REG_BB_TXAGC_A_NSS3INDEX3_NSS3INDEX0_AC        0xce0
#define    REG_BB_TXAGC_A_NSS3INDEX7_NSS3INDEX4_AC        0xce4
#define    REG_BB_TXAGC_A_NSS3INDEX9_NSS3INDEX8_AC        0xce8

// DIG-related
#define    REG_BB_A_IGI_AC                                0xc50    // Initial Gain for path-A
// AFE-related
#define    REG_BB_A_AFEPWR1_AC                            0xc60 // dynamic AFE power control
#define    REG_BB_A_AFEPWR2_AC                            0xc64 // dynamic AFE power control
#define    REG_BB_A_RX_WAITCCA_TX_CCKRFON_AC              0xc68
#define    REG_BB_A_TX_CCKBBON_OFDMRFON_AC                0xc6c
#define    REG_BB_A_TX_OFDMBBON_TX2RX_AC                  0xc70
#define    REG_BB_A_TX2TX_RXCCK_AC                        0xc74
#define    REG_BB_A_RX_OFDM_WAITRIFS_AC                   0xc78
#define    REG_BB_A_RX2RX_BT_AC                           0xc7c
#define    REG_BB_A_SLEEP_NAV_AC                          0xc80
#define    REG_BB_A_PMPD_AC                               0xc84
#define    REG_BB_A_LSSIWRITE_AC                          0xc90 // RF write addr
#define    REG_BB_A_RFE_PINMUX_AC                         0xcb0  // Path_A RFE cotrol pinmux
#define    REG_BB_A_RFE_AC                                0xcb8  // Path_A RFE cotrol

/**** page d ****/
#define    REG_BB_A_PIREAD_AC                             0xd04 // RF readback with PI
#define    REG_BB_B_PIREAD_AC                             0xd44 // RF readback with PI
#define    REG_BB_C_PIREAD_AC                             0xd84 // RF readback with PI
#define    REG_BB_D_PIREAD_AC                             0xdc4 // RF readback with PI
#define    REG_BB_A_SIREAD_AC                             0xd08 // RF readback with SI
#define    REG_BB_B_SIREAD_AC                             0xd48 // RF readback with SI
#define    REG_BB_C_SIREAD_AC                             0xd88 // RF readback with SI
#define    REG_BB_D_SIREAD_AC                             0xdc8 // RF readback with SI


/**** page e ****/
// RXIQC
#define    REG_BB_B_TXSCALE_AC                            0xe1c  // Path_B TX scaling factor
#define    REG_BB_B_RXIQC_AB_AC                           0xe10  //RxIQ imblance matrix coeff. A & B
#define    REG_BB_B_RXIQC_CD_AC                           0xe14  //RxIQ imblance matrix coeff. C & D

// TX AGC 
#define    REG_BB_TXAGC_B_CCK11_CCK1_AC                   0xe20
#define    REG_BB_TXAGC_B_OFDM18_OFDM6_AC                 0xe24
#define    REG_BB_TXAGC_B_OFDM54_OFDM24_AC                0xe28
#define    REG_BB_TXAGC_B_MCS3_MCS0_AC                    0xe2c
#define    REG_BB_TXAGC_B_MCS7_MCS4_AC                    0xe30
#define    REG_BB_TXAGC_B_MCS11_MCS8_AC                   0xe34
#define    REG_BB_TXAGC_B_MCS15_MCS12_AC                  0xe38
#define    REG_BB_TXAGC_B_MCS19_MCS16_AC                  0xed8
#define    REG_BB_TXAGC_B_MCS23_MCS20_AC				  0xedc
#define    REG_BB_TXAGC_B_NSS1INDEX3_NSS1INDEX0_AC        0xe3c
#define    REG_BB_TXAGC_B_NSS1INDEX7_NSS1INDEX4_AC        0xe40
#define    REG_BB_TXAGC_B_NSS2INDEX1_NSS1INDEX8_AC        0xe44
#define    REG_BB_TXAGC_B_NSS2INDEX5_NSS2INDEX2_AC        0xe48
#define    REG_BB_TXAGC_B_NSS2INDEX9_NSS2INDEX6_AC        0xe4c
#define    REG_BB_TXAGC_B_NSS3INDEX3_NSS3INDEX0_AC        0xee0
#define    REG_BB_TXAGC_B_NSS3INDEX7_NSS3INDEX4_AC        0xee4
#define    REG_BB_TXAGC_B_NSS3INDEX9_NSS3INDEX8_AC        0xee8

// DIG-related
#define    REG_BB_B_IGI_AC                                0xe50    // Initial Gain for path-B
#define    REG_BB_B_LSSIWRITE_AC                          0xe90 // RF write addr

// Misc functions
#define    REG_BB_B_RFE_PINMUX_AC                         0xeb0 // Path_B RFE control pinmux
#define    REG_BB_B_RFE_AC                                0xeb8 // Path_B RFE control

/**** page f ****/
// DIG-related
#define    REG_BB_OFDM_FALSEALARM1_AC                     0xf48  // counter for break
#define    REG_BB_OFDM_FALSEALARM2_AC                     0xf4c  // counter for spoofing
// Report-related                                     
#define    REG_BB_OFDM_SHORTCFOAB_AC                      0xf60  
#define    REG_BB_OFDM_LONGCFOAB_AC                       0xf64
#define    REG_BB_OFDM_ENDCFOAB_AC                        0xf70
#define    REG_BB_OFDM_AGCREPORT_AC                       0xf84
#define    REG_BB_OFDM_RXSNR_AC                           0xf88
#define    REG_BB_OFDM_RXEVMCSI_AC                        0xf8c
#define    REG_BB_OFDM_SIGREPORT_AC                       0xf90

/**** page 18 ****/
// TX AGC 
#define    REG_BB_TXAGC_C_CCK11_CCK1_AC                   0x1820
#define    REG_BB_TXAGC_C_OFDM18_OFDM6_AC                 0x1824
#define    REG_BB_TXAGC_C_OFDM54_OFDM24_AC                0x1828
#define    REG_BB_TXAGC_C_MCS3_MCS0_AC                    0x182c
#define    REG_BB_TXAGC_C_MCS7_MCS4_AC                    0x1830
#define    REG_BB_TXAGC_C_MCS11_MCS8_AC                   0x1834
#define    REG_BB_TXAGC_C_MCS15_MCS12_AC                  0x1838
#define    REG_BB_TXAGC_C_MCS19_MCS16_AC                  0x18d8
#define    REG_BB_TXAGC_C_MCS23_MCS20_AC				  0x18dc
#define    REG_BB_TXAGC_C_NSS1INDEX3_NSS1INDEX0_AC        0x183c
#define    REG_BB_TXAGC_C_NSS1INDEX7_NSS1INDEX4_AC        0x1840
#define    REG_BB_TXAGC_C_NSS2INDEX1_NSS1INDEX8_AC        0x1844
#define    REG_BB_TXAGC_C_NSS2INDEX5_NSS2INDEX2_AC        0x1848
#define    REG_BB_TXAGC_C_NSS2INDEX9_NSS2INDEX6_AC        0x184c
#define    REG_BB_TXAGC_C_NSS3INDEX3_NSS3INDEX0_AC        0x18e0
#define    REG_BB_TXAGC_C_NSS3INDEX7_NSS3INDEX4_AC        0x18e4
#define    REG_BB_TXAGC_C_NSS3INDEX9_NSS3INDEX8_AC        0x18e8

#define    REG_BB_C_LSSIWRITE_AC                          0x1890 // RF write addr
#define    REG_BB_C_RFE_PINMUX_AC                         0x18b4 // Path_C RFE control pinmux

/**** page 19 ****/
//TX BeamForming
#define	REG_BB_TXBF_ANT_SET_BF1				0x19ac
#define	REG_BB_TXBF_ANT_SET_BF0				0x19b4


/**** page 1a ****/
// TX AGC 
#define    REG_BB_TXAGC_D_CCK11_CCK1_AC                   0x1a20
#define    REG_BB_TXAGC_D_OFDM18_OFDM6_AC                 0x1a24
#define    REG_BB_TXAGC_D_OFDM54_OFDM24_AC                0x1a28
#define    REG_BB_TXAGC_D_MCS3_MCS0_AC                    0x1a2c
#define    REG_BB_TXAGC_D_MCS7_MCS4_AC                    0x1a30
#define    REG_BB_TXAGC_D_MCS11_MCS8_AC                   0x1a34
#define    REG_BB_TXAGC_D_MCS15_MCS12_AC                  0x1a38
#define    REG_BB_TXAGC_D_MCS19_MCS16_AC                  0x1ad8
#define    REG_BB_TXAGC_D_MCS23_MCS20_AC				  0x1adc
#define    REG_BB_TXAGC_D_NSS1INDEX3_NSS1INDEX0_AC        0x1a3c
#define    REG_BB_TXAGC_D_NSS1INDEX7_NSS1INDEX4_AC        0x1a40
#define    REG_BB_TXAGC_D_NSS2INDEX1_NSS1INDEX8_AC        0x1a44
#define    REG_BB_TXAGC_D_NSS2INDEX5_NSS2INDEX2_AC        0x1a48
#define    REG_BB_TXAGC_D_NSS2INDEX9_NSS2INDEX6_AC        0x1a4c
#define    REG_BB_TXAGC_D_NSS3INDEX3_NSS3INDEX0_AC        0x1ae0
#define    REG_BB_TXAGC_D_NSS3INDEX7_NSS3INDEX4_AC        0x1ae4
#define    REG_BB_TXAGC_D_NSS3INDEX9_NSS3INDEX8_AC        0x1ae8

#define    REG_BB_D_LSSIWRITE_AC                          0x1a90 // RF write addr
#define    REG_BB_D_RFE_PINMUX_AC                         0x1ab4 // Path_D RFE control pinmux


//////////////////////////////////////




// RF Register
//
#define    REG_RF_AC_AC                                    0x00     
#define    REG_RF_RF_TOP_AC                                0x07     
#define    REG_RF_TXLOK_AC                                 0x08     
#define    REG_RF_TXAPK_AC                                 0x0B
#define    REG_RF_CHNLBW_AC                                0x18    // RF channel and BW switch
#define    REG_RF_TXLCTANK_AC                              0x54
#define    REG_RF_APK_AC                                   0x63
#define    REG_BRF_CHNLBW_MOD_AG_AC                        0x70300
#define    REG_BRF_CHNLBW_BW_AC                            0xc00
#define    REG_RF_RCK1_AC                                  0x1c    
#define    REG_RF_RCK2_AC                                  0x1d
#define    REG_RF_RCK3_AC                                  0x1e
#define    REG_RF_LCK_AC                                   0xB4
#define    REG_BB_BRFREGOFFSETMASK_AC                      0xfffff

// TX AGC V2 - 8814A MP
#define REG_BB_TXAGC_V2_BASE_ADDR      0x1998

/*
Example:
0x1998 = 0x12801151
0x1998[7:0]=0x51: TX_RATE VHT 4ss mcs7
0x1998[9:8]=path_idx=1: path B (path A/B/C/D -> 0/1/2/3)
0x1998[12]�FDy3�FG?n1, |b8813!LS|3!��@�FDI.
0x1998[23]=1: write
0x1998[31:24]=0x12: txagc codeword
*/

#define BIT_SHIFT_BB_TXAGC_V2_TX_RATE   0
#define BIT_MASK_BB_TXAGC_V2_TX_RATE    0xff
#define BIT_BB_TXAGC_V2_TX_RATE(x)      (((x) & BIT_MASK_BB_TXAGC_V2_TX_RATE) << BIT_SHIFT_BB_TXAGC_V2_TX_RATE)

#define BIT_SHIFT_BB_TXAGC_V2_PATH_IDX  8
#define BIT_MASK_BB_TXAGC_V2_PATH_IDX   0x3
#define BIT_BB_TXAGC_V2_PATH_IDX(x)     (((x) & BIT_MASK_BB_TXAGC_V2_PATH_IDX) << BIT_SHIFT_BB_TXAGC_V2_PATH_IDX)

#define BIT_SHIFT_BB_TXAGC_V2_NONE      12
#define BIT_MASK_BB_TXAGC_V2_NONE       0x1
#define BIT_BB_TXAGC_V2_NONE(x)         (((x) & BIT_MASK_BB_TXAGC_V2_NONE) << BIT_SHIFT_BB_TXAGC_V2_NONE)

#define BIT_SHIFT_BB_TXAGC_V2_WRITE     23
#define BIT_MASK_BB_TXAGC_V2_WRITE      0x1
#define BIT_BB_TXAGC_V2_WRITE(x)        (((x) & BIT_MASK_BB_TXAGC_V2_WRITE) << BIT_SHIFT_BB_TXAGC_V2_WRITE)

#define BIT_SHIFT_BB_TXAGC_V2_CODEWORD  24
#define BIT_MASK_BB_TXAGC_V2_CODEWORD   0xff
#define BIT_BB_TXAGC_V2_CODEWORD(x)     (((x) & BIT_MASK_BB_TXAGC_V2_CODEWORD) << BIT_SHIFT_BB_TXAGC_V2_CODEWORD)

#define FILL_REG_BB_TXAGC_V2(RATE, PATH_IDX, NONE_VALUE, WRITE_VALUE, DEF_POWER)    (BIT_BB_TXAGC_V2_TX_RATE(RATE) | BIT_BB_TXAGC_V2_PATH_IDX(PATH_IDX) | BIT_BB_TXAGC_V2_NONE(NONE_VALUE) | BIT_BB_TXAGC_V2_WRITE(WRITE_VALUE) | BIT_BB_TXAGC_V2_CODEWORD(DEF_POWER))

#define TX_AGC_CCK_1M           0x00
#define TX_AGC_CCK_2M           0x01
#define TX_AGC_CCK_5_5M         0x02
#define TX_AGC_CCK_11M          0x03
#define TX_AGC_OFDM_6M          0x04
#define TX_AGC_OFDM_9M          0x05
#define TX_AGC_OFDM_12M         0x06
#define TX_AGC_OFDM_18M         0x07
#define TX_AGC_OFDM_24M         0x08
#define TX_AGC_OFDM_36M         0x09
#define TX_AGC_OFDM_48M         0x0A
#define TX_AGC_OFDM_54M         0x0B

#define TX_AGC_HT_NSS1_MCS0        0x0C
#define TX_AGC_HT_NSS1_MCS1        0x0D
#define TX_AGC_HT_NSS1_MCS2        0x0E
#define TX_AGC_HT_NSS1_MCS3        0x0F
#define TX_AGC_HT_NSS1_MCS4        0x10
#define TX_AGC_HT_NSS1_MCS5        0x11
#define TX_AGC_HT_NSS1_MCS6        0x12
#define TX_AGC_HT_NSS1_MCS7        0x13
#define TX_AGC_HT_NSS2_MCS8        0x14
#define TX_AGC_HT_NSS2_MCS9        0x15
#define TX_AGC_HT_NSS2_MCS10       0x16
#define TX_AGC_HT_NSS2_MCS11       0x17
#define TX_AGC_HT_NSS2_MCS12       0x18
#define TX_AGC_HT_NSS2_MCS13       0x19
#define TX_AGC_HT_NSS2_MCS14       0x1A
#define TX_AGC_HT_NSS2_MCS15       0x1B
#define TX_AGC_HT_NSS3_MCS16       0x1C
#define TX_AGC_HT_NSS3_MCS17       0x1D
#define TX_AGC_HT_NSS3_MCS18       0x1E
#define TX_AGC_HT_NSS3_MCS19       0x1F
#define TX_AGC_HT_NSS3_MCS20       0x20
#define TX_AGC_HT_NSS3_MCS21       0x21
#define TX_AGC_HT_NSS3_MCS22       0x22
#define TX_AGC_HT_NSS3_MCS23       0x23
#define TX_AGC_HT_NSS4_MCS24       0x24
#define TX_AGC_HT_NSS4_MCS25       0x25
#define TX_AGC_HT_NSS4_MCS26       0x26
#define TX_AGC_HT_NSS4_MCS27       0x27
#define TX_AGC_HT_NSS4_MCS28       0x28
#define TX_AGC_HT_NSS4_MCS29       0x29
#define TX_AGC_HT_NSS4_MCS30       0x2A
#define TX_AGC_HT_NSS4_MCS31       0x2B

#define TX_AGC_VHT_NSS1_MCS0       0x2C
#define TX_AGC_VHT_NSS1_MCS1       0x2D
#define TX_AGC_VHT_NSS1_MCS2       0x2E
#define TX_AGC_VHT_NSS1_MCS3       0x2F
#define TX_AGC_VHT_NSS1_MCS4       0x30
#define TX_AGC_VHT_NSS1_MCS5       0x31
#define TX_AGC_VHT_NSS1_MCS6       0x32
#define TX_AGC_VHT_NSS1_MCS7       0x33
#define TX_AGC_VHT_NSS1_MCS8       0x34
#define TX_AGC_VHT_NSS1_MCS9       0x35
#define TX_AGC_VHT_NSS2_MCS0       0x36
#define TX_AGC_VHT_NSS2_MCS1       0x37
#define TX_AGC_VHT_NSS2_MCS2       0x38
#define TX_AGC_VHT_NSS2_MCS3       0x39
#define TX_AGC_VHT_NSS2_MCS4       0x3A
#define TX_AGC_VHT_NSS2_MCS5       0x3B
#define TX_AGC_VHT_NSS2_MCS6       0x3C
#define TX_AGC_VHT_NSS2_MCS7       0x3D
#define TX_AGC_VHT_NSS2_MCS8       0x3E
#define TX_AGC_VHT_NSS2_MCS9       0x3F
#define TX_AGC_VHT_NSS3_MCS0       0x40
#define TX_AGC_VHT_NSS3_MCS1       0x41
#define TX_AGC_VHT_NSS3_MCS2       0x42
#define TX_AGC_VHT_NSS3_MCS3       0x43
#define TX_AGC_VHT_NSS3_MCS4       0x44
#define TX_AGC_VHT_NSS3_MCS5       0x45
#define TX_AGC_VHT_NSS3_MCS6       0x46
#define TX_AGC_VHT_NSS3_MCS7       0x47
#define TX_AGC_VHT_NSS3_MCS8       0x48
#define TX_AGC_VHT_NSS3_MCS9       0x49
#define TX_AGC_VHT_NSS4_MCS0       0x4A
#define TX_AGC_VHT_NSS4_MCS1       0x4B
#define TX_AGC_VHT_NSS4_MCS2       0x4C
#define TX_AGC_VHT_NSS4_MCS3       0x4D
#define TX_AGC_VHT_NSS4_MCS4       0x4E
#define TX_AGC_VHT_NSS4_MCS5       0x4F
#define TX_AGC_VHT_NSS4_MCS6       0x50
#define TX_AGC_VHT_NSS4_MCs7       0x51
#define TX_AGC_VHT_NSS4_MCS8       0x52
#define TX_AGC_VHT_NSS4_MCS9       0x53

//eric-8814
#define		RF_SYN_G1					0x25	// RF TX Power control
#define		RF_SYN_G2					0x26	// RF TX Power control
#define		RF_SYN_G3					0x27	// RF TX Power control
#define		RF_SYN_G4					0x28	// RF TX Power control
#define		RF_SYN_G5					0x29	// RF TX Power control
#define		RF_SYN_G6					0x2A	// RF TX Power control
#define		RF_SYN_G7					0x2B	// RF TX Power control
#define		RF_SYN_G8					0x2C	// RF TX Power control

#define		RF_RCK_OS					0x30	// RF TX PA control
#define		RF_TXPA_G1					0x31	// RF TX PA control
#define		RF_TXPA_G2					0x32	// RF TX PA control
#define		RF_TXPA_G3					0x33	// RF TX PA control
#define		RF_TX_BIAS_A				0x35
#define		RF_TX_BIAS_D				0x36
#define		RF_LOBF_9					0x38
#define		RF_RXRF_A3					0x3C	//	
#define		RF_TRSW						0x3F

#define		RF_TXRF_A2					0x41
#define		RF_TXPA_G4					0x46	
#define		RF_TXPA_A4					0x4B	
#define 	RF_0x52						0x52
#define		RF_WE_LUT					0xEF	


#endif //#if IS_RTL88XX_AC


#if IS_RTL88XX_N

//
// 2. BB Register Page8(0x800)
//
#define    REG_BB_FPGA0_RFMOD_N                     0x800    //RF mode & CCK TxSC // RF BW Setting??
#define    REG_BB_FPGA0_TXINFO_N                    0x804    // Status report??
#define    REG_BB_FPGA0_PSDFUNCTION_N               0x808
#define    REG_BB_FPGA0_TXGAINSTAGE_N               0x80c    // Set TX PWR init gain?
#define    REG_BB_FPGA0_RFTIMING1_N                 0x810    // Useless now
#define    REG_BB_FPGA0_RFTIMING2_N                 0x814
#define    REG_BB_PRIMESC_N                         0x818

#define    REG_BB_FPGA0_XA_HSSIPARAMETER1_N         0x820    // RF 3 wire register
#define    REG_BB_FPGA0_XA_HSSIPARAMETER2_N         0x824
#define    REG_BB_FPGA0_XB_HSSIPARAMETER1_N         0x828
#define    REG_BB_FPGA0_XB_HSSIPARAMETER2_N         0x82c
#define    REG_BB_TXAGC_B_RATE18_06_N               0x830
#define    REG_BB_TXAGC_B_RATE54_24_N               0x834
#define    REG_BB_TXAGC_B_CCK5_1_MCS32_N            0x838
#define    REG_BB_TXAGC_B_MCS03_MCS00_N             0x83c
#define    REG_BB_FPGA0_XA_LSSIPARAMETER_N          0x840
#define    REG_BB_FPGA0_XB_LSSIPARAMETER_N          0x844
#define    REG_BB_TXAGC_B_MCS07_MCS04_N             0x848
#define    REG_BB_TXAGC_B_MCS11_MCS08_N             0x84c
#define    REG_BB_FPGA0_RFWAKEUPPARAMETER_N         0x850    // Useless now
#define    REG_BB_FPGA0_RFSLEEPUPPARAMETER_N        0x854
#define    REG_BB_FPGA0_XAB_SWITCHCONTROL_N         0x858    // RF Channel switch
#define    REG_BB_FPGA0_XCD_SWITCHCONTROL_N         0x85c
#define    REG_BB_FPGA0_XA_RFINTERFACEOE_N          0x860    // RF Channel switch
#define    REG_BB_FPGA0_XB_RFINTERFACEOE_N          0x864
#define    REG_BB_TXAGC_B_MCS15_MCS12_N             0x868
#define    REG_BB_TXAGC_A_CCK11_2_B_CCK11_N         0x86c
#define    REG_BB_FPGA0_XAB_RFINTERFACESW_N         0x870    // RF Interface Software Control
#define    REG_BB_FPGA0_XCD_RFINTERFACESW_N         0x874
#define    REG_BB_FPGA0_XAB_RFPARAMETER_N           0x878    // RF Parameter
#define    REG_BB_FPGA0_XCD_RFPARAMETER_N           0x87c
#define    REG_BB_FPGA0_ANALOGPARAMETER1_N          0x880    // Crystal cap setting RF-R/W protection for parameter4??
#define    REG_BB_FPGA0_ANALOGPARAMETER2_N          0x884
#define    REG_BB_FPGA0_ANALOGPARAMETER3_N          0x888    // Useless now
#define    REG_BB_FPGA0_ANALOGPARAMETER4_N          0x88c
#define    REG_BB_FPGA0_XA_LSSIREADBACK_N           0x8a0    // Tranceiver LSSI Readback
#define    REG_BB_FPGA0_XB_LSSIREADBACK_N           0x8a4

#define    REG_BB_FPGA0_PSDREPORT_N                0x8b4    // Useless now
#define    REG_BB_TRANSCEIVERA_HSPI_READBACK_N     0x8b8
#define    REG_BB_TRANSCEIVERB_HSPI_READBACK_N     0x8bc
#define    REG_BB_FPGA0_XAB_RFINTERFACERB_N        0x8e0    // Useless now // RF Interface Readback Value



//
// 3. BB Register Page9(0x900)
//
#define    REG_BB_FPGA1_RFMOD_N                   0x900    //RF mode & OFDM TxSC // RF BW Setting??
#define    REG_BB_FPGA1_TXBLOCK_N                 0x904    // Useless now
#define    REG_BB_FPGA1_DEBUGSELECT_N             0x908    // Useless now
#define    REG_BB_FPGA1_TXINFO_N                  0x90c    // Useless now // Status report??


//
// 4. BB Register PageA(0xA00)
//
// Set Control channel to upper or lower. These settings are required only for 40MHz
#define    REG_BB_CCK0_SYSTEM_N                  0xa00
#define    REG_BB_CCK0_AFESETTING_N              0xa04    // Disable init gain now // Select RX path by RSSI
#define    REG_BB_CCK0_CCA_N                     0xa08    // Disable init gain now // Init gain
#define    REG_BB_CCK0_RXAGC1_N                  0xa0c     //AGC default value, saturation level // Antenna Diversity, RX AGC, LNA Threshold, RX LNA Threshold useless now. Not the same as 90 series
#define    REG_BB_CCK0_RXAGC2_N                  0xa10     //AGC & DAGC
#define    REG_BB_CCK0_RXHP_N                    0xa14
#define    REG_BB_CCK0_DSPPARAMETER1_N           0xa18    //Timing recovery & Channel estimation threshold
#define    REG_BB_CCK0_DSPPARAMETER2_N           0xa1c    //SQ threshold
#define    REG_BB_CCK0_TXFILTER1_N               0xa20
#define    REG_BB_CCK0_TXFILTER2_N               0xa24
#define    REG_BB_CCK0_DEBUGPORT_N               0xa28    //debug port and Tx filter3
#define    REG_BB_CCK0_FALSEALARMREPORT_N        0xa2c    //0xa2d    useless now 0xa30-a4f channel report
#define    REG_BB_CCK0_TRSSIREPORT_N             0xa50
#define    REG_BB_CCK0_RXREPORT_N                0xa54  //0xa57
#define    REG_BB_CCK0_FACOUNTERLOWER_N          0xa5c  //0xa5b
#define    REG_BB_CCK0_FACOUNTERUPPER_N          0xa58  //0xa5c


//
// 5. BB Register PageC(0xC00)
//
#define    REG_BB_OFDM0_LSTF_N                   0xc00
#define    REG_BB_OFDM0_TRXPATHENABLE_N          0xc04
#define    REG_BB_OFDM0_TRMUXPAR_N               0xc08
#define    REG_BB_OFDM0_TRSWISOLATION_N          0xc0c
#define    REG_BB_OFDM0_XARXAFE_N                0xc10  //RxIQ DC offset, Rx digital filter, DC notch filter
#define    REG_BB_OFDM0_XARXIQIMBALANCE_N        0xc14  //RxIQ imblance matrix
#define    REG_BB_OFDM0_XBRXAFE_N                0xc18
#define    REG_BB_OFDM0_XBRXIQIMBALANCE_N        0xc1c
/*                                             
#define    REG_BB_OFDM0_XCRXAFE_N                0xc20
#define    REG_BB_OFDM0_XCRXIQIMBALANCE_N        0xc24
#define    REG_BB_OFDM0_XDRXAFE_N                0xc28
#define    REG_BB_OFDM0_XDRXIQIMBALANCE_N        0xc2c
*/                                             
#define    REG_BB_OFDM0_RXDETECTOR1_N            0xc30  //PD,BW & SBD    // DM tune init gain
#define    REG_BB_OFDM0_RXDETECTOR2_N            0xc34  //SBD & Fame Sync.
#define    REG_BB_OFDM0_RXDETECTOR3_N            0xc38  //Frame Sync.
#define    REG_BB_OFDM0_RXDETECTOR4_N            0xc3c  //PD, SBD, Frame Sync & Short-GI
#define    REG_BB_OFDM0_RXDSP_N                  0xc40  //Rx Sync Path
#define    REG_BB_OFDM0_CFOANDDAGC_N             0xc44  //CFO & DAGC
#define    REG_BB_OFDM0_CCADROPTHRESHOLD_N       0xc48 //CCA Drop threshold
#define    REG_BB_OFDM0_ECCATHRESHOLD_N          0xc4c // energy CCA
#define    REG_BB_OFDM0_XAAGCCORE1_N             0xc50    // DIG
#define    REG_BB_OFDM0_XAAGCCORE2_N             0xc54
#define    REG_BB_OFDM0_XBAGCCORE1_N             0xc58
#define    REG_BB_OFDM0_XBAGCCORE2_N             0xc5c
/*                                             
#define    REG_BB_OFDM0_XCAGCCORE1_N             0xc60
#define    REG_BB_OFDM0_XCAGCCORE2_N             0xc64
#define    REG_BB_OFDM0_XDAGCCORE1_N             0xc68
#define    REG_BB_OFDM0_XDAGCCORE2_N             0xc6c
*/
#define    REG_BB_OFDM0_AGCPARAMETER1_N          0xc70
#define    REG_BB_OFDM0_AGCPARAMETER2_N          0xc74
#define    REG_BB_OFDM0_AGCRSSITABLE_N           0xc78
#define    REG_BB_OFDM0_HTSTFAGC_N               0xc7c
#define    REG_BB_OFDM0_XATXIQIMBALANC_N         0xc80    // TX PWR TRACK and DIG
#define    REG_BB_OFDM0_XATXAFE_N                0xc84
#define    REG_BB_OFDM0_XBTXIQIMBALANCE_N        0xc88
#define    REG_BB_OFDM0_XBTXAFE_N                0xc8c


#define    REG_BB_OFDM0_XCTXIQIMBALANCE_N        0xc90
#define    REG_BB_OFDM0_XCTXAFE_N                0xc94
#define    REG_BB_OFDM0_XDTXIQIMBALANCE_N        0xc98
#define    REG_BB_OFDM0_XDTXAFE_N                0xc9c
#define    REG_BB_OFDM0_RXIQEXTANTA_N            0xca0


#define    REG_BB_OFDM0_RXHPPARAMETER_N          0xce0
#define    REG_BB_OFDM0_TXPSEUDONOISEWGT_N       0xce4
#define    REG_BB_OFDM0_FRAMESYNC_N              0xcf0
#define    REG_BB_OFDM0_DFSREPORT_N              0xcf4
#define    REG_BB_OFDM0_TXCOEFF1_N               0xca4
#define    REG_BB_OFDM0_TXCOEFF2_N               0xca8
#define    REG_BB_OFDM0_TXCOEFF3_N               0xcac
#define    REG_BB_OFDM0_TXCOEFF4_N               0xcb0
#define    REG_BB_OFDM0_TXCOEFF5_N               0xcb4
#define    REG_BB_OFDM0_TXCOEFF6_N               0xcb8


//
// 6. BB Register PageD(0xD00)
//
#define    REG_BB_OFDM1_LSTF_N                   0xd00
#define    REG_BB_OFDM1_TRXPATHENABLE_N          0xd04
#define    REG_BB_OFDM1_CFO_N                    0xd08    // No setting now
#define    REG_BB_OFDM1_CSI1_N                   0xd10
#define    REG_BB_OFDM1_SBD_N                    0xd14
#define    REG_BB_OFDM1_CSI2_N                   0xd18
#define    REG_BB_OFDM1_CFOTRACKING_N            0xd2c
#define    REG_BB_OFDM1_TRXMESAURE1_N            0xd34
#define    REG_BB_OFDM1_INTFDET_N                0xd3c
#define    REG_BB_OFDM1_PSEUDONOISESTATEAB_N     0xd50
#define    REG_BB_OFDM1_PSEUDONOISESTATECD_N     0xd54
#define    REG_BB_OFDM1_RXPSEUDONOISEWGT_N       0xd58
#define    REG_BB_OFDM_PHYCOUNTER1_N             0xda0  //cca, parity fail
#define    REG_BB_OFDM_PHYCOUNTER2_N             0xda4  //rate illegal, crc8 fail
#define    REG_BB_OFDM_PHYCOUNTER3_N             0xda8  //MCS not support
#define    REG_BB_OFDM_SHORTCFOAB_N              0xdac    // No setting now
#define    REG_BB_OFDM_SHORTCFOCD_N              0xdb0
#define    REG_BB_OFDM_LONGCFOAB_N               0xdb4
#define    REG_BB_OFDM_LONGCFOCD_N               0xdb8
#define    REG_BB_OFDM_TAILCFOAB_N               0xdbc
#define    REG_BB_OFDM_TAILCFOCD_N               0xdc0
#define    REG_BB_OFDM_PWMEASURE1_N              0xdc4
#define    REG_BB_OFDM_PWMEASURE2_N              0xdc8
#define    REG_BB_OFDM_BWREPORT_N                0xdcc
#define    REG_BB_OFDM_AGCREPORT_N               0xdd0
#define    REG_BB_OFDM_RXSNR_N                   0xdd4
#define    REG_BB_OFDM_RXEVMCSI_N                0xdd8
#define    REG_BB_OFDM_SIGREPORT_N               0xddc


//
// 6. BB Register PageE(0xE00)
//
#define    REG_BB_TXAGC_A_RATE18_06_N           0xe00
#define    REG_BB_TXAGC_A_RATE54_24_N           0xe04
#define    REG_BB_TXAGC_A_CCK1_MCS32_N          0xe08
#define    REG_BB_TXAGC_A_MCS03_MCS00_N         0xe10
#define    REG_BB_TXAGC_A_MCS07_MCS04_N         0xe14
#define    REG_BB_TXAGC_A_MCS11_MCS08_N         0xe18
#define    REG_BB_TXAGC_A_MCS15_MCS12_N         0xe1c


//
// 7. RF Register 0x00-0x2E
//
//Zebra1
#define    REG_RF_ZEBRA1_HSSIENABLE_N            0x0    // Useless now
#define    REG_RF_ZEBRA1_TRXENABLE1_N            0x1
#define    REG_RF_ZEBRA1_TRXENABLE2_N            0x2
#define    REG_RF_ZEBRA1_AGC_N                   0x4
#define    REG_RF_ZEBRA1_CHARGEPUMP_N            0x5
#define    REG_RF_ZEBRA1_CHANNEL_N               0x7    // RF channel switch
#define    REG_RF_ZEBRA1_TXGAIN_N                0x8    // Useless now
#define    REG_RF_ZEBRA1_TXLPF_N                 0x9
#define    REG_RF_ZEBRA1_RXLPF_N                 0xb
#define    REG_RF_ZEBRA1_RXHPFCORNER_N           0xc
#define    REG_RF_RFCHANNEL_N                    0x18    // RF channel switch    For T65 RF 0222d
#define    REG_RF_CSI_MASK_N                     0x42

//Zebra4
#define    REG_RF_GLOBALCTRL_N                    0    // Useless now
#define    REG_RF_RTL8256_TXLPF_N                19
#define    REG_RF_RTL8256_RXLPF_N                11

//RTL8258
#define    REG_RF_RTL8258_RSSILPF_N              0xa
#define    REG_RF_RTL8258_TXLPF_N                0x11    // Useless now
#define    REG_RF_RTL8258_RXLPF_N                0x13

// 92E temp
#define		RF_SYN_G1					0x25	// RF TX Power control
#define		RF_SYN_G2					0x26	// RF TX Power control
#define		RF_SYN_G3					0x27	// RF TX Power control
#define		RF_SYN_G4					0x28	// RF TX Power control
#define		RF_SYN_G5					0x29	// RF TX Power control
#define		RF_SYN_G6					0x2A	// RF TX Power control
#define		RF_SYN_G7					0x2B	// RF TX Power control
#define		RF_SYN_G8					0x2C	// RF TX Power control

#define		RF_RCK_OS					0x30	// RF TX PA control
#define		RF_TXPA_G1					0x31	// RF TX PA control
#define		RF_TXPA_G2					0x32	// RF TX PA control
#define		RF_TXPA_G3					0x33	// RF TX PA control
#define		RF_TX_BIAS_A				0x35
#define		RF_TX_BIAS_D				0x36
#define		RF_LOBF_9					0x38
#define		RF_RXRF_A3					0x3C	//	
#define		RF_TRSW						0x3F

#define		RF_TXRF_A2					0x41
#define		RF_TXPA_G4					0x46	
#define		RF_TXPA_A4					0x4B	
#define 	RF_0x52						0x52
#define		RF_WE_LUT					0xEF

#define REG_BFMEE_SEL_8192E			0x0714
#define	REG_SND_PTCL_CTRL_8192E		0x0718
#define REG_BFMER0_INFO_8192E		0x06E4
#define REG_BFMER1_INFO_8192E		0x06EC
#define REG_CSI_RPT_PARAM_BW20_92E  0x06F4
#define REG_CSI_RPT_PARAM_BW40_92E  0x06F8

#endif //#if IS_RTL88XX_N
#endif //#ifndef __INC_HALCOMPHYREG_BB_H


