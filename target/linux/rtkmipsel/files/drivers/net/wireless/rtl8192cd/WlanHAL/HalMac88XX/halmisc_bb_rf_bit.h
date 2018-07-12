#ifndef __HALMAC_BB_RF_BIT_H__
#define __HALMAC_BB_RF_BIT_H__


/*-------------------------Modification Log-----------------------------------

-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/

//3 ============Programming guide Start=====================
/*
    BB BIT define rules 
    1. For all bit define, it should be prefixed by "BIT_BB"
    2. For all bit mask, it should be prefixed by "BIT_MASK_BB"
    3. For all bit shift, it should be prefixed by "BIT_SHIFT_BB"
    4. For other case, prefix is not needed

    RF BIT define rules 
    1. For all bit define, it should be prefixed by "BIT_RF"
    2. For all bit mask, it should be prefixed by "BIT_MASK_RF"
    3. For all bit shift, it should be prefixed by "BIT_SHIFT_RF"
    4. For other case, prefix is not needed    

    SET BIT define rules
    1. Setting byte mask for BB or RF, it should be prefixed by "BIT_SET"

    Suffixed BIT define rules
    1. For AC chip, it should be suffixed by "_AC"
    2. For N chip, it should be suffixed by "_N"
    3. For Common bit define, it should be suffixed by "_COMMON"    
    
*/
#if 1//IS_RTL88XX_AC
/**** page 8 ****/

//REG_BB_RCCKENABLE_AC  offset:0x808
#define    BIT_MASK_BB_BOFDMEN_AC        0x20000000
#define    BIT_MASK_BB_BCCKEN_AC         0x10000000
#define    BIT_MASK_BB_BRXPATH_AC        0xff

//REG_BB_RTXPATH_AC     offset:0x80c
#define    BIT_MASK_BB_BTXPATH_AC        0x0fffffff

//REG_BB_RAGC_TABLE_AC  offset:0x82c   // AGC tabel select
#define    BIT_MASK_BB_BAGC_TABLE_AC                           0x3

//REG_BB_CCAONSEC_AC    offset:0x838
#define    BIT_MASK_BB_CCA_AC                                  0xf

//REG_BB_REDCCA_AC      offset:0x8a4 // EDCCA
#define    BIT_MASK_BB_BEDCCA_AC                               0xffff

//REG_BB_RHSSIREAD_AC   offset:0x8b0  // RF read addr
#define    BIT_MASK_BB_BHSSIREAD_ADDR_AC                       0xff

/**** page 9 ****/
//REG_BB_AGC_TABLE_AC_V2                        0x958
#define     BIT_MASK_BB_BAGC_TABLE_AC_V2                        0x0000001f


/**** page a ****/
//REG_BB_RCCK_SYSTEM_AC offset: 0xa00   // for cck sideband
#define    BIT_MASK_BB_BCCK_SYSTEM_AC                          0x10 
//REG_BB_RCCK_RX_AC     offset:  0xa04    // for cck rx path selection
#define    BIT_MASK_BB_BCCK_RX_AC                              0x0c000000 
//REG_BB_RCCK_CCA_AC    offset:  0xa08    // cca threshold
#define    BIT_MASK_BB_BCCK_CCA_AC                             0x00ff0000

// REG_BB_RCCK_TXFILTER1_AC       offset:  0xa20
#define    BIT_MASK_BB_BCCK_TXFILTER1_C0_AC                    0x00ff0000
#define    BIT_MASK_BB_BCCK_TXFILTER1_C1_AC                    0xff000000
//REG_BB_RCCK_TXFILTER2_AC         offset:               0xa24
#define     BIT_MASK_BB_BCCK_TXFILTER2_C2_AC                    0x000000ff
#define     BIT_MASK_BB_BCCK_TXFILTER2_C3_AC                    0x0000ff00
#define     BIT_MASK_BB_BCCK_TXFILTER2_C4_AC                    0x00ff0000
#define     BIT_MASK_BB_BCCK_TXFILTER2_C5_AC                    0xff000000
//REG_BB_RCCK_TXFILTER3_AC    offset:                0xa28
#define     BIT_MASK_BB_BCCK_TXFILTER3_C6_AC                    0x000000ff
#define     BIT_MASK_BB_BCCK_TXFILTER3_C7_AC                    0x0000ff00
//REG_BB_RCCK_FALSEALARM_AC           offset:            0xa5c // counter for cck false alarm
#define     BIT_MASK_BB_B_FALSEALARM_AC                         0xffff
/**** page b ****/
/**** page c ****/
//REG_BB_AGC_TABLE_AC_V1                            0xc1c
#define     BIT_MASK_BB_BAGC_TABLE_AC_V1                        0x00000f00
//REG_BB_A_LSSIWRITE_AC offset :0xc90
#define     BIT_MASK_BB_LSSIWRITE_AC                               0xf0000000
/**** page d ****/
/**** page e ****/


#endif //#if IS_RTL88XX_AC


#if 1//IS_RTL88XX_N

//
// 2. Page8(0x800)
//
#define    BIT_MASK_BB_RFMOD_N_N                         0x1    // Reg 0x800 rFPGA0_RFMOD
#define    BIT_MASK_BB_JAPANMODE_N                       0x2
#define    BIT_MASK_BB_CCKTXSC_N                            0x30
#define    BIT_MASK_BB_CCKEN_N                           0x1000000
#define    BIT_MASK_BB_OFDMEN_N                          0x2000000
#define    BIT_MASK_BB_OFDMRXADCPHASE_N                     0x10000    // Useless now
#define    BIT_MASK_BB_OFDMTXDACPHASE_N                     0x40000

#define    BIT_MASK_BB_XATXAGC_N                         0x3f
#define    BIT_MASK_BB_XBTXAGC_N                         0xf00    // Reg 80c rFPGA0_TxGainStage
/*
#define    BIT_MASK_BB_XCTXAGC_N                         0xf000
#define    BIT_MASK_BB_XDTXAGC_N                         0xf0000
*/

#define    BIT_MASK_BB_PASTART_N                         0xf0000000    // Useless now
#define    BIT_MASK_BB_TRSTART_N                         0x00f00000
#define    BIT_MASK_BB_RFSTART_N                         0x0000f000
#define    BIT_MASK_BB_BBSTART_N                         0x000000f0
#define    BIT_MASK_BB_BBCCKSTART_N                      0x0000000f
#define    BIT_MASK_BB_PAEND_N                           0xf          //Reg0x814
#define    BIT_MASK_BB_TREND_N                           0x0f000000
#define    BIT_MASK_BB_RFEND_N                           0x000f0000
#define    BIT_MASK_BB_CCAMASK_N                         0x000000f0   //T2R
#define    BIT_MASK_BB_R2RCCAMASK_N                      0x00000f00
#define    BIT_MASK_BB_HSSI_R2TDELAY_N                   0xf8000000
#define    BIT_MASK_BB_HSSI_T2RDELAY_N                   0xf80000
#define    BIT_MASK_BB_CONTTXHSSI_N                      0x400     //chane gain at continue Tx
#define    BIT_MASK_BB_IGFROMCCK_N                       0x200
#define    BIT_MASK_BB_AGCADDRESS_N                      0x3f
#define    BIT_MASK_BB_RXHPTX_N                          0x7000
#define    BIT_MASK_BB_RXHPT2R_N                         0x38000
#define    BIT_MASK_BB_RXHPCCKINI_N                      0xc0000
#define    BIT_MASK_BB_AGCTXCODE_N                       0xc00000
#define    BIT_MASK_BB_AGCRXCODE_N                       0x300000

#define    BIT_MASK_BB_3WIREDATALENGTH_N                 0x800    // Reg 0x820~84f rFPGA0_XA_HSSIParameter1
#define    BIT_MASK_BB_3WIREADDRESSLENGTH_N              0x400

#define    BIT_MASK_BB_3WIRERFPOWERDOWN_N                0x1    // Useless now
#define    BIT_MASK_BB_5GPAPEPOLARITY_N                  0x40000000
#define    BIT_MASK_BB_2GPAPEPOLARITY_N                  0x80000000
#define    BIT_MASK_BB_RFSW_TXDEFAULTANT_N               0x3
#define    BIT_MASK_BB_RFSW_TXOPTIONANT_N                0x30
#define    BIT_MASK_BB_RFSW_RXDEFAULTANT_N               0x300
#define    BIT_MASK_BB_RFSW_RXOPTIONANT_N                0x3000
#define    BIT_MASK_BB_RFSI_3WIREDATA_N                  0x1
#define    BIT_MASK_BB_RFSI_3WIRECLOCK_N                 0x2
#define    BIT_MASK_BB_RFSI_3WIRELOAD_N                  0x4
#define    BIT_MASK_BB_RFSI_3WIRERW_N                    0x8
#define    BIT_MASK_BB_RFSI_3WIRE_N                      0xf

#define    BIT_MASK_BB_RFSI_RFENV_N                      0x10    // Reg 0x870 rFPGA0_XAB_RFInterfaceSW
#define    BIT_MASK_BB_RFSI_TRSW_N                       0x20    // Useless now
#define    BIT_MASK_BB_RFSI_TRSWB_N                      0x40
#define    BIT_MASK_BB_RFSI_ANTSW_N                      0x100
#define    BIT_MASK_BB_RFSI_ANTSWB_N                     0x200
#define    BIT_MASK_BB_RFSI_PAPE_N                       0x400
#define    BIT_MASK_BB_RFSI_PAPE5G_N                     0x800
#define    BIT_MASK_BB_BANDSELECT_N                      0x1
#define    BIT_MASK_BB_HTSIG2_GI_N                       0x80
#define    BIT_MASK_BB_HTSIG2_SMOOTHING_N                0x01
#define    BIT_MASK_BB_HTSIG2_SOUNDING_N                 0x02
#define    BIT_MASK_BB_HTSIG2_AGGREATON_N                0x08
#define    BIT_MASK_BB_HTSIG2_STBC_N                     0x30
#define    BIT_MASK_BB_HTSIG2_ADVCODING_N                0x40
#define    BIT_MASK_BB_HTSIG2_NUMOFHTLTF_N               0x300
#define    BIT_MASK_BB_HTSIG2_CRC8_N                     0x3fc
#define    BIT_MASK_BB_HTSIG1_MCS_N                      0x7f
#define    BIT_MASK_BB_HTSIG1_BANDWIDTH_N                0x80
#define    BIT_MASK_BB_HTSIG1_HTLENGTH_N                 0xffff
#define    BIT_MASK_BB_LSIG_RATE_N                       0xf
#define    BIT_MASK_BB_LSIG_RESERVED_N                   0x10
#define    BIT_MASK_BB_LSIG_LENGTH_N                     0x1fffe
#define    BIT_MASK_BB_LSIG_PARITY_N                     0x20
#define    BIT_MASK_BB_CCKRXPHASE_N                      0x4

//#define    BIT_MASK_BB_LSSIREADADDRESS                 0x3f000000   //LSSI "Read" Address    // Reg 0x824 rFPGA0_XA_HSSIParameter2
#define    BIT_MASK_BB_LSSIREADADDRESS_N                 0x7f800000   // T65 RF
#define    BIT_MASK_BB_LSSIREADEDGE_N                    0x80000000   //LSSI "Read" edge signal

//#define    BIT_MASK_BB_LSSIREADBACKDATA_N              0xfff        // Reg 0x8a0 rFPGA0_XA_LSSIReadBack
#define    BIT_MASK_BB_LSSIREADBACKDATA_N                0xfffff        // T65 RF

#define    BIT_MASK_BB_LSSIREADOKFLAG_N                  0x1000    // Useless now
#define    BIT_MASK_BB_CCKSAMPLERATE_N                   0x8       //0: 44MHz, 1:88MHz
#define    BIT_MASK_BB_REGULATOR0STANDBY_N               0x1
#define    BIT_MASK_BB_REGULATORPLLSTANDBY_N             0x2
#define    BIT_MASK_BB_REGULATOR1STANDBY_N               0x4
#define    BIT_MASK_BB_PLLPOWERUP_N                      0x8
#define    BIT_MASK_BB_DPLLPOWERUP_N                     0x10
#define    BIT_MASK_BB_DA10POWERUP_N                     0x20
#define    BIT_MASK_BB_AD7POWERUP_N                      0x200
#define    BIT_MASK_BB_DA6POWERUP_N                      0x2000
#define    BIT_MASK_BB_XTALPOWERUP_N                     0x4000
#define    BIT_MASK_BB_40MDCLKPOWERUP_N                  0x8000
#define    BIT_MASK_BB_DA6DEBUGMODE_N                    0x20000
#define    BIT_MASK_BB_DA6SWING_N                        0x380000

#define    BIT_MASK_BB_ADCLKPHASE_N                      0x4000000    // Reg 0x880 rFPGA0_AnalogParameter1 20/40 CCK support switch 40/80 BB MHZ

#define    BIT_MASK_BB_80MCLKDELAY_N                     0x18000000    // Useless
#define    BIT_MASK_BB_AFEWATCHDOGENABLE_N               0x20000000

#define    BIT_MASK_BB_XTALCAP01_N                       0xc0000000    // Reg 0x884 rFPGA0_AnalogParameter2 Crystal cap
#define    BIT_MASK_BB_XTALCAP23_N                       0x3
#define    BIT_MASK_BB_XTALCAP92X_N                      0x0f000000

#define    BIT_MASK_BB_INTDIFCLKENABLE_N                 0x400    // Useless
#define    BIT_MASK_BB_EXTSIGCLKENABLE_N                 0x800
#define    BIT_MASK_BB_BANDGAPMBIASPOWERUP_N             0x10000
#define    BIT_MASK_BB_AD11SHGAIN_N                      0xc0000
#define    BIT_MASK_BB_AD11INPUTRANGE_N                  0x700000
#define    BIT_MASK_BB_AD11OPCURRENT_N                   0x3800000
#define    BIT_MASK_BB_IPATHLOOPBACK_N                   0x4000000
#define    BIT_MASK_BB_QPATHLOOPBACK_N                   0x8000000
#define    BIT_MASK_BB_AFELOOPBACK_N                     0x10000000
#define    BIT_MASK_BB_DA10SWING_N                       0x7e0
#define    BIT_MASK_BB_DA10REVERSE_N                     0x800
#define    BIT_MASK_BB_DACLKSOURCE_N                     0x1000
#define    BIT_MASK_BB_AD7INPUTRANGE_N                   0x6000
#define    BIT_MASK_BB_AD7GAIN_N                         0x38000
#define    BIT_MASK_BB_AD7OUTPUTCMMODE_N                 0x40000
#define    BIT_MASK_BB_AD7INPUTCMMODE_N                  0x380000
#define    BIT_MASK_BB_AD7CURRENT_N                      0xc00000
#define    BIT_MASK_BB_REGULATORADJUST_N                 0x7000000
#define    BIT_MASK_BB_AD11POWERUPATTX_N                 0x1
#define    BIT_MASK_BB_DA10PSATTX_N                      0x10
#define    BIT_MASK_BB_AD11POWERUPATRX_N                 0x100
#define    BIT_MASK_BB_DA10PSATRX_N                      0x1000
#define    BIT_MASK_BB_CCKRXAGCFORMAT_N                  0x200
#define    BIT_MASK_BB_PSDFFTSAMPLEPPOINT_N              0xc000
#define    BIT_MASK_BB_PSDAVERAGENUM_N                   0x3000
#define    BIT_MASK_BB_IQPATHCONTROL_N                   0xc00
#define    BIT_MASK_BB_PSDFREQ_N                         0x3ff
#define    BIT_MASK_BB_PSDANTENNAPATH_N                  0x30
#define    BIT_MASK_BB_PSDIQSWITCH_N                     0x40
#define    BIT_MASK_BB_PSDRXTRIGGER_N                    0x400000
#define    BIT_MASK_BB_PSDTXTRIGGER_N                    0x80000000
#define    BIT_MASK_BB_PSDSINETONESCALE_N                0x7f000000
#define    BIT_MASK_BB_PSDREPORT_N                       0xffff


//
// 3. Page9(0x900)
//
#define    BIT_MASK_BB_OFDMTXSC_N                       0x30000000    // Useless
#define    BIT_MASK_BB_CCKTXON_N                        0x1
#define    BIT_MASK_BB_OFDMTXON_N                       0x2
#define    BIT_MASK_BB_DEBUGPAGE_N                      0xfff  //reset debug page and also HWord, LWord
#define    BIT_MASK_BB_DEBUGITEM_N                      0xff   //reset debug page and LWord
#define    BIT_MASK_BB_ANTL_N                           0x10
#define    BIT_MASK_BB_ANTNONHT_N                       0x100
#define    BIT_MASK_BB_ANTHT1_N                         0x1000
#define    BIT_MASK_BB_ANTHT2_N                         0x10000
#define    BIT_MASK_BB_ANTHT1S1_N                       0x100000
#define    BIT_MASK_BB_ANTNONHTS1_N                     0x1000000


//
// 4. PageA(0xA00)
//
#define    BIT_MASK_BB_CCKBBMODE_N                      0x3    // Useless
#define    BIT_MASK_BB_CCKTXPOWERSAVING_N               0x80
#define    BIT_MASK_BB_CCKRXPOWERSAVING_N               0x40
#define    BIT_MASK_BB_CCKSIDEBAND_N                    0x10    // Reg 0xa00 rCCK0_System 20/40 switch
#define    BIT_MASK_BB_CCKSCRAMBLE_N                    0x8    // Useless
#define    BIT_MASK_BB_CCKANTDIVERSITY_N                0x8000
#define    BIT_MASK_BB_CCKCARRIERRECOVERY_N             0x4000
#define    BIT_MASK_BB_CCKTXRATE_N                      0x3000
#define    BIT_MASK_BB_CCKDCCANCEL_N                    0x0800
#define    BIT_MASK_BB_CCKISICANCEL_N                   0x0400
#define    BIT_MASK_BB_CCKMATCHFILTER_N                 0x0200
#define    BIT_MASK_BB_CCKEQUALIZER_N                   0x0100
#define    BIT_MASK_BB_CCKPREAMBLEDETECT_N              0x800000
#define    BIT_MASK_BB_CCKFASTFALSECCA_N                0x400000
#define    BIT_MASK_BB_CCKCHESTSTART_N                  0x300000
#define    BIT_MASK_BB_CCKCCACOUNT_N                    0x080000
#define    BIT_MASK_BB_CCKCS_LIM_N                      0x070000
#define    BIT_MASK_BB_CCKBISTMODE_N                    0x80000000
#define    BIT_MASK_BB_CCKCCAMASK_N                     0x40000000
#define    BIT_MASK_BB_CCKTXDACPHASE_N                  0x4
#define    BIT_MASK_BB_CCKRXADCPHASE_N                  0x20000000   //r_rx_clk
#define    BIT_MASK_BB_CCKR_CP_MODE0_N                  0x0100
#define    BIT_MASK_BB_CCKTXDCOFFSET_N                  0xf0
#define    BIT_MASK_BB_CCKRXDCOFFSET_N                  0xf
#define    BIT_MASK_BB_CCKCCAMODE_N                     0xc000
#define    BIT_MASK_BB_CCKFALSECS_LIM_N                 0x3f00
#define    BIT_MASK_BB_CCKCS_RATIO_N                    0xc00000
#define    BIT_MASK_BB_CCKCORGBIT_SEL_N                 0x300000
#define    BIT_MASK_BB_CCKPD_LIM_N                      0x0f0000
#define    BIT_MASK_BB_CCKNEWCCA_N                      0x80000000
#define    BIT_MASK_BB_CCKRXHPOFIG_N                    0x8000
#define    BIT_MASK_BB_CCKRXIG_N                        0x7f00
#define    BIT_MASK_BB_CCKLNAPOLARITY_N                 0x800000
#define    BIT_MASK_BB_CCKRX1STGAIN_N                   0x7f0000
#define    BIT_MASK_BB_CCKRFEXTEND_N                    0x20000000 //CCK Rx Iinital gain polarity
#define    BIT_MASK_BB_CCKRXAGCSATLEVEL_N               0x1f000000
#define    BIT_MASK_BB_CCKRXAGCSATCOUNT_N               0xe0
#define    BIT_MASK_BB_CCKRXRFSETTLE_N                  0x1f       //AGCsamp_dly
#define    BIT_MASK_BB_CCKFIXEDRXAGC_N                  0x8000
//#define bCCKRxAGCFormat                  0x4000   //remove to HSSI register 0x824
#define    BIT_MASK_BB_CCKANTENNAPOLARITY_N             0x2000
#define    BIT_MASK_BB_CCKTXFILTERTYPE_N                0x0c00
#define    BIT_MASK_BB_CCKRXAGCREPORTTYPE_N             0x0300
#define    BIT_MASK_BB_CCKRXDAGCEN_N                    0x80000000
#define    BIT_MASK_BB_CCKRXDAGCPERIOD_N                0x20000000
#define    BIT_MASK_BB_CCKRXDAGCSATLEVEL_N              0x1f000000
#define    BIT_MASK_BB_CCKTIMINGRECOVERY_N              0x800000
#define    BIT_MASK_BB_CCKTXC0_N                        0x3f0000
#define    BIT_MASK_BB_CCKTXC1_N                        0x3f000000
#define    BIT_MASK_BB_CCKTXC2_N                        0x3f
#define    BIT_MASK_BB_CCKTXC3_N                        0x3f00
#define    BIT_MASK_BB_CCKTXC4_N                        0x3f0000
#define    BIT_MASK_BB_CCKTXC5_N                        0x3f000000
#define    BIT_MASK_BB_CCKTXC6_N                        0x3f
#define    BIT_MASK_BB_CCKTXC7_N                        0x3f00
#define    BIT_MASK_BB_CCKDEBUGPORT_N                   0xff0000
#define    BIT_MASK_BB_CCKDACDEBUG_N                    0x0f000000
#define    BIT_MASK_BB_CCKFALSEALARMENABLE_N            0x8000
#define    BIT_MASK_BB_CCKFALSEALARMREAD_N              0x4000
#define    BIT_MASK_BB_CCKTRSSI_N                       0x7f
#define    BIT_MASK_BB_CCKRXAGCREPORT_N                 0xfe
#define    BIT_MASK_BB_CCKRXREPORT_ANTSEL_N             0x80000000
#define    BIT_MASK_BB_CCKRXREPORT_MFOFF_N              0x40000000
#define    BIT_MASK_BB_CCKRXRXREPORT_SQLOSS_N           0x20000000
#define    BIT_MASK_BB_CCKRXREPORT_PKTLOSS_N            0x10000000
#define    BIT_MASK_BB_CCKRXREPORT_LOCKEDBIT_N          0x08000000
#define    BIT_MASK_BB_CCKRXREPORT_RATEERROR_N          0x04000000
#define    BIT_MASK_BB_CCKRXREPORT_RXRATE_N             0x03000000
#define    BIT_MASK_BB_CCKRXFACOUNTERLOWER_N            0xff
#define    BIT_MASK_BB_CCKRXFACOUNTERUPPER_N            0xff000000
#define    BIT_MASK_BB_CCKRXHPAGCSTART_N                0xe000
#define    BIT_MASK_BB_CCKRXHPAGCFINAL_N                0x1c00
#define    BIT_MASK_BB_CCKRXFALSEALARMENABLE_N          0x8000
#define    BIT_MASK_BB_CCKFACOUNTERFREEZE_N             0x4000
#define    BIT_MASK_BB_CCKTXPATHSEL_N                   0x10000000
#define    BIT_MASK_BB_CCKDEFAULTRXPATH_N               0xc000000
#define    BIT_MASK_BB_CCKOPTIONRXPATH_N                0x3000000


//
// 5. PageC(0xC00)
//
#define    BIT_MASK_BB_NUMOFSTF_N                      0x3    // Useless
#define    BIT_MASK_BB_SHIFT_L_N                       0xc0
#define    BIT_MASK_BB_GI_TH_N                         0xc
#define    BIT_MASK_BB_RXPATHA_N                       0x1
#define    BIT_MASK_BB_RXPATHB_N                       0x2
#define    BIT_MASK_BB_RXPATHC_N                       0x4
#define    BIT_MASK_BB_RXPATHD_N                       0x8
#define    BIT_MASK_BB_TXPATHA_N                       0x1
#define    BIT_MASK_BB_TXPATHB_N                       0x2
#define    BIT_MASK_BB_TXPATHC_N                       0x4
#define    BIT_MASK_BB_TXPATHD_N                       0x8
#define    BIT_MASK_BB_TRSSIFREQ_N                     0x200
#define    BIT_MASK_BB_ADCBACKOFF_N                    0x3000
#define    BIT_MASK_BB_DFIRBACKOFF_N                   0xc000
#define    BIT_MASK_BB_TRSSILATCHPHASE_N               0x10000
#define    BIT_MASK_BB_RXIDCOFFSET_N                   0xff
#define    BIT_MASK_BB_RXQDCOFFSET_N                   0xff00
#define    BIT_MASK_BB_RXDFIRMODE_N                    0x1800000
#define    BIT_MASK_BB_RXDCNFTYPE_N                    0xe000000
#define    BIT_MASK_BB_RXIQIMB_A_N                     0x3ff
#define    BIT_MASK_BB_RXIQIMB_B_N                     0xfc00
#define    BIT_MASK_BB_RXIQIMB_C_N                     0x3f0000
#define    BIT_MASK_BB_RXIQIMB_D_N                     0xffc00000
#define    BIT_MASK_BB_DC_DC_NOTCH_N                   0x60000
#define    BIT_MASK_BB_RXNBINOTCH_N                    0x1f000000
#define    BIT_MASK_BB_PD_TH_N                         0xf
#define    BIT_MASK_BB_PD_TH_OPT2_N                    0xc000
#define    BIT_MASK_BB_PWED_TH_N                       0x700
#define    BIT_MASK_BB_IFMF_WIN_L_N                    0x800
#define    BIT_MASK_BB_PD_OPTION_N                     0x1000
#define    BIT_MASK_BB_MF_WIN_L_N                      0xe000
#define    BIT_MASK_BB_BW_SEARCH_L_N                   0x30000
#define    BIT_MASK_BB_WIN_ENH_L_N                     0xc0000
#define    BIT_MASK_BB_BW_TH_N                         0x700000
#define    BIT_MASK_BB_ED_TH2_N                        0x3800000
#define    BIT_MASK_BB_BW_OPTION_N                     0x4000000
#define    BIT_MASK_BB_RATIO_TH_N                      0x18000000
#define    BIT_MASK_BB_WINDOW_L_N                      0xe0000000
#define    BIT_MASK_BB_SBD_OPTION_N                    0x1
#define    BIT_MASK_BB_FRAME_TH_N                      0x1c
#define    BIT_MASK_BB_FS_OPTION_N                     0x60
#define    BIT_MASK_BB_DC_SLOPE_CHECK_N                0x80
#define    BIT_MASK_BB_FGUARD_COUNTER_DC_L_N           0xe00
#define    BIT_MASK_BB_FRAME_WEIGHT_SHORT_N            0x7000
#define    BIT_MASK_BB_SUB_TUNE_N                      0xe00000
#define    BIT_MASK_BB_FRAME_DC_LENGTH_N               0xe000000
#define    BIT_MASK_BB_SBD_START_OFFSET_N              0x30000000
#define    BIT_MASK_BB_FRAME_TH_2_N                    0x7
#define    BIT_MASK_BB_FRAME_GI2_TH_N                  0x38
#define    BIT_MASK_BB_GI2_SYNC_EN_N                   0x40
#define    BIT_MASK_BB_SARCH_SHORT_EARLY_N             0x300
#define    BIT_MASK_BB_SARCH_SHORT_LATE_N              0xc00
#define    BIT_MASK_BB_SARCH_GI2_LATE_N                0x70000
#define    BIT_MASK_BB_CFOANTSUM_N                     0x1
#define    BIT_MASK_BB_CFOACC_N                        0x2
#define    BIT_MASK_BB_CFOSTARTOFFSET_N                0xc
#define    BIT_MASK_BB_CFOLOOKBACK_N                   0x70
#define    BIT_MASK_BB_CFOSUMWEIGHT_N                  0x80
#define    BIT_MASK_BB_DAGCENABLE_N                    0x10000
#define    BIT_MASK_BB_TXIQIMB_A_N                     0x3ff
#define    BIT_MASK_BB_TXIQIMB_B_N                     0xfc00
#define    BIT_MASK_BB_TXIQIMB_C_N                     0x3f0000
#define    BIT_MASK_BB_TXIQIMB_D_N                     0xffc00000
#define    BIT_MASK_BB_TXIDCOFFSET_N                   0xff
#define    BIT_MASK_BB_TXQDCOFFSET_N                   0xff00
#define    BIT_MASK_BB_TXDFIRMODE_N                    0x10000
#define    BIT_MASK_BB_TXPESUDONOISEON_N               0x4000000
#define    BIT_MASK_BB_TXPESUDONOISE_A_N               0xff
#define    BIT_MASK_BB_TXPESUDONOISE_B_N               0xff00
#define    BIT_MASK_BB_TXPESUDONOISE_C_N               0xff0000
#define    BIT_MASK_BB_TXPESUDONOISE_D_N               0xff000000
#define    BIT_MASK_BB_CCADROPOPTION_N                 0x20000
#define    BIT_MASK_BB_CCADROPTHRES_N                  0xfff00000
#define    BIT_MASK_BB_EDCCA_H_N                       0xf
#define    BIT_MASK_BB_EDCCA_L_N                       0xf0
#define    BIT_MASK_BB_LAMBDA_ED_N                     0x300
#define    BIT_MASK_BB_RXINITIALGAIN_N                 0x7f
#define    BIT_MASK_BB_RXANTDIVEN_N                    0x80
#define    BIT_MASK_BB_RXAGCADDRESSFORLNA_N            0x7f00
#define    BIT_MASK_BB_RXHIGHPOWERFLOW_N               0x8000
#define    BIT_MASK_BB_RXAGCFREEZETHRES_N              0xc0000
#define    BIT_MASK_BB_RXFREEZESTEP_AGC1_N             0x300000
#define    BIT_MASK_BB_RXFREEZESTEP_AGC2_N             0xc00000
#define    BIT_MASK_BB_RXFREEZESTEP_AGC3_N             0x3000000
#define    BIT_MASK_BB_RXFREEZESTEP_AGC0_N             0xc000000
#define    BIT_MASK_BB_RXRSSI_CMP_EN_N                 0x10000000
#define    BIT_MASK_BB_RXQUICKAGCEN_N                  0x20000000
#define    BIT_MASK_BB_RXAGCFREEZETHRESMODE_N          0x40000000
#define    BIT_MASK_BB_RXOVERFLOWCHECKTYPE_N           0x80000000
#define    BIT_MASK_BB_RXAGCSHIFT_N                    0x7f
#define    BIT_MASK_BB_TRSW_TRI_ONLY_N                 0x80
#define    BIT_MASK_BB_POWERTHRES_N                    0x300
#define    BIT_MASK_BB_RXAGCEN_N                       0x1
#define    BIT_MASK_BB_RXAGCTOGETHEREN_N               0x2
#define    BIT_MASK_BB_RXAGCMIN_N                      0x4
#define    BIT_MASK_BB_RXHP_INI_N                      0x7
#define    BIT_MASK_BB_RXHP_TRLNA_N                    0x70
#define    BIT_MASK_BB_RXHP_RSSI_N                     0x700
#define    BIT_MASK_BB_RXHP_BBP1_N                     0x7000
#define    BIT_MASK_BB_RXHP_BBP2_N                     0x70000
#define    BIT_MASK_BB_RXHP_BBP3_N                     0x700000
#define    BIT_MASK_BB_RSSI_H_N                        0x7f0000     //the threshold for high power
#define    BIT_MASK_BB_RSSI_GEN_N                      0x7f000000   //the threshold for ant diversity
#define    BIT_MASK_BB_RXSETTLE_TRSW_N                 0x7
#define    BIT_MASK_BB_RXSETTLE_LNA_N                  0x38
#define    BIT_MASK_BB_RXSETTLE_RSSI_N                 0x1c0
#define    BIT_MASK_BB_RXSETTLE_BBP_N                  0xe00
#define    BIT_MASK_BB_RXSETTLE_RXHP_N                 0x7000
#define    BIT_MASK_BB_RXSETTLE_ANTSW_RSSI_N           0x38000
#define    BIT_MASK_BB_RXSETTLE_ANTSW_N                0xc0000
#define    BIT_MASK_BB_RXPROCESSTIME_DAGC_N            0x300000
#define    BIT_MASK_BB_RXSETTLE_HSSI_N                 0x400000
#define    BIT_MASK_BB_RXPROCESSTIME_BBPPW_N           0x800000
#define    BIT_MASK_BB_RXANTENNAPOWERSHIFT_N           0x3000000
#define    BIT_MASK_BB_RSSITABLESELECT_N               0xc000000
#define    BIT_MASK_BB_RXHP_FINAL_N                    0x7000000
#define    BIT_MASK_BB_RXHTSETTLE_BBP_N                0x7
#define    BIT_MASK_BB_RXHTSETTLE_HSSI_N               0x8
#define    BIT_MASK_BB_RXHTSETTLE_RXHP_N               0x70
#define    BIT_MASK_BB_RXHTSETTLE_BBPPW_N              0x80
#define    BIT_MASK_BB_RXHTSETTLE_IDLE_N               0x300
#define    BIT_MASK_BB_RXHTSETTLE_RESERVED_N           0x1c00
#define    BIT_MASK_BB_RXHTRXHPEN_N                    0x8000
#define    BIT_MASK_BB_RXHTAGCFREEZETHRES_N            0x30000
#define    BIT_MASK_BB_RXHTAGCTOGETHEREN_N             0x40000
#define    BIT_MASK_BB_RXHTAGCMIN_N                    0x80000
#define    BIT_MASK_BB_RXHTAGCEN_N                     0x100000
#define    BIT_MASK_BB_RXHTDAGCEN_N                    0x200000
#define    BIT_MASK_BB_RXHTRXHP_BBP_N                  0x1c00000
#define    BIT_MASK_BB_RXHTRXHP_FINAL_N                0xe0000000
#define    BIT_MASK_BB_RXPWRATIOTH_N                   0x3
#define    BIT_MASK_BB_RXPWRATIOEN_N                   0x4
#define    BIT_MASK_BB_RXMFHOLD_N                      0x3800
#define    BIT_MASK_BB_RXPD_DELAY_TH1_N                0x38
#define    BIT_MASK_BB_RXPD_DELAY_TH2_N                0x1c0
#define    BIT_MASK_BB_RXPD_DC_COUNT_MAX_N             0x600
//#define bRxMF_Hold               0x3800
#define    BIT_MASK_BB_RxPD_Delay_TH_N                0x8000
#define    BIT_MASK_BB_RxProcess_Delay_N              0xf0000
#define    BIT_MASK_BB_RxSearchrange_GI2_Early_N      0x700000
#define    BIT_MASK_BB_RxFrame_Guard_Counter_L_N      0x3800000
#define    BIT_MASK_BB_RxSGI_Guard_L_N                0xc000000
#define    BIT_MASK_BB_RxSGI_Search_L_N               0x30000000
#define    BIT_MASK_BB_RxSGI_TH_N                     0xc0000000
#define    BIT_MASK_BB_DFSCnt0_N                      0xff
#define    BIT_MASK_BB_DFSCnt1_N                      0xff00
#define    BIT_MASK_BB_DFSFlag_N                      0xf0000
#define    BIT_MASK_BB_MFWeightSum_N                  0x300000
#define    BIT_MASK_BB_MinIdxTH_N                     0x7f000000
#define    BIT_MASK_BB_DAFormat_N                     0x40000
#define    BIT_MASK_BB_TxChEmuEnable_N                0x01000000
#define    BIT_MASK_BB_TRSWIsolation_A_N              0x7f
#define    BIT_MASK_BB_TRSWIsolation_B_N              0x7f00
/*  
#define    BIT_MASK_BB_TRSWIsolation_C_N              0x7f0000
#define    BIT_MASK_BB_TRSWIsolation_D_N              0x7f000000
*/
#define    BIT_MASK_BB_ExtLNAGain_N                   0x7c00


//
// 6. PageE(0xE00)
//
#define    BIT_MASK_BB_STBCEN_N                       0x4    // Useless
#define    BIT_MASK_BB_ANTENNAMAPPING_N               0x10
#define    BIT_MASK_BB_NSS_N                          0x20
#define    BIT_MASK_BB_CFOANTSUMD_N                   0x200
#define    BIT_MASK_BB_OFDMDISPWSAVTX_N               0x400000
#define    BIT_MASK_BB_PHYCOUNTERRESET_N              0x8000000
#define    BIT_MASK_BB_CFOREPORTGET_N                 0x4000000
#define    BIT_MASK_BB_OFDMCONTINUETX_N               0x10000000
#define    BIT_MASK_BB_OFDMSINGLECARRIER_N            0x20000000
#define    BIT_MASK_BB_OFDMSINGLETONE_N               0x40000000
/*
#define bRxPath1                 0x01
#define bRxPath2                 0x02
#define bRxPath3                 0x04
#define bRxPath4                 0x08
#define bTxPath1                 0x10
#define bTxPath2                 0x20
*/
#define    BIT_MASK_BB_HTDETECT_N                    0x100
#define    BIT_MASK_BB_CFOEN_N                       0x10000
#define    BIT_MASK_BB_CFOVALUE_N                    0xfff00000
#define    BIT_MASK_BB_SIGTONE_RE_N                  0x3f
#define    BIT_MASK_BB_SIGTONE_IM_N                  0x7f00
#define    BIT_MASK_BB_COUNTER_CCA_N                 0xffff
#define    BIT_MASK_BB_COUNTER_PARITYFAIL_N          0xffff0000
#define    BIT_MASK_BB_COUNTER_RATEILLEGAL_N         0xffff
#define    BIT_MASK_BB_COUNTER_CRC8FAIL_N            0xffff0000
#define    BIT_MASK_BB_COUNTER_MCSNOSUPPORT_N        0xffff
#define    BIT_MASK_BB_COUNTER_FASTSYNC_N            0xffff
#define    BIT_MASK_BB_SHORTCFO_N                    0xfff
#define    BIT_MASK_BB_SHORTCFOTLENGTH_N             12   //total
#define    BIT_MASK_BB_SHORTCFOFLENGTH_N             11   //fraction
#define    BIT_MASK_BB_LONGCFO_N                     0x7ff
#define    BIT_MASK_BB_LONGCFOTLENGTH_N              11
#define    BIT_MASK_BB_LONGCFOFLENGTH_N              11
#define    BIT_MASK_BB_TAILCFO_N                     0x1fff
#define    BIT_MASK_BB_TAILCFOTLENGTH_N              13
#define    BIT_MASK_BB_TAILCFOFLENGTH_N              12
#define    BIT_MASK_BB_MAX_EN_PWDB_N                 0xffff
#define    BIT_MASK_BB_CC_POWER_DB_N                 0xffff0000
#define    BIT_MASK_BB_NOISE_PWDB_N                  0xffff
#define    BIT_MASK_BB_POWERMEASTLENGTH_N            10
#define    BIT_MASK_BB_POWERMEASFLENGTH_N            3
#define    BIT_MASK_BB_RX_HT_BW_N                    0x1
#define    BIT_MASK_BB_RXSC_N                        0x6
#define    BIT_MASK_BB_RX_HT_N                       0x8
#define    BIT_MASK_BB_NB_INTF_DET_ON_N              0x1
#define    BIT_MASK_BB_INTF_WIN_LEN_CFG_N            0x30
#define    BIT_MASK_BB_NB_INTF_TH_CFG_N              0x1c0
#define    BIT_MASK_BB_RFGAIN_N                      0x3f
#define    BIT_MASK_BB_TABLESEL_N                    0x40
#define    BIT_MASK_BB_TRSW_N                        0x80
#define    BIT_MASK_BB_RXSNR_A_N                     0xff
#define    BIT_MASK_BB_RXSNR_B_N                     0xff00
/*
#define    BIT_MASK_BB_RXSNR_C_N                     0xff0000
#define    BIT_MASK_BB_RXSNR_D_N                     0xff000000
*/
#define    BIT_MASK_BB_SNREVMTLENGTH_N               8
#define    BIT_MASK_BB_SNREVMFLENGTH_N               1
#define    BIT_MASK_BB_CSI1ST_N                      0xff
#define    BIT_MASK_BB_CSI2ND_N                      0xff00
#define    BIT_MASK_BB_RXEVM1ST_N                    0xff0000
#define    BIT_MASK_BB_RXEVM2ND_N                    0xff000000
#define    BIT_MASK_BB_SIGEVM_N                      0xff
#define    BIT_MASK_BB_PWDB_N                        0xff00
#define    BIT_MASK_BB_SGIEN_N                       0x10000

#define    BIT_MASK_BB_SFACTORQAM1_N                 0xf    // Useless
#define    BIT_MASK_BB_SFACTORQAM2_N                 0xf0
#define    BIT_MASK_BB_SFACTORQAM3_N                 0xf00
#define    BIT_MASK_BB_SFACTORQAM4_N                 0xf000
#define    BIT_MASK_BB_SFACTORQAM5_N                 0xf0000
#define    BIT_MASK_BB_SFACTORQAM6_N                 0xf0000
#define    BIT_MASK_BB_SFACTORQAM7_N                 0xf00000
#define    BIT_MASK_BB_SFACTORQAM8_N                 0xf000000
#define    BIT_MASK_BB_SFACTORQAM9_N                 0xf0000000
#define    BIT_MASK_BB_CSISCHEME_N                   0x100000

#define    BIT_MASK_BB_NOISELVLTOPSET_N              0x3    // Useless
#define    BIT_MASK_BB_CHSMOOTH_N                    0x4
#define    BIT_MASK_BB_CHSMOOTHCFG1_N                0x38
#define    BIT_MASK_BB_CHSMOOTHCFG2_N                0x1c0
#define    BIT_MASK_BB_CHSMOOTHCFG3_N                0xe00
#define    BIT_MASK_BB_CHSMOOTHCFG4_N                0x7000
#define    BIT_MASK_BB_MRCMODE_N                     0x800000
#define    BIT_MASK_BB_THEVMCFG_N                    0x7000000

#define    BIT_MASK_BB_LOOPFITTYPE_N                 0x1    // Useless
#define    BIT_MASK_BB_UPDCFO_N                      0x40
#define    BIT_MASK_BB_UPDCFOOFFDATA_N               0x80
#define    BIT_MASK_BB_ADVUPDCFO_N                   0x100
#define    BIT_MASK_BB_ADVTIMECTRL_N                 0x800
#define    BIT_MASK_BB_UPDCLKO_N                     0x1000
#define    BIT_MASK_BB_FC_N                          0x6000
#define    BIT_MASK_BB_TRACKINGMODE_N                0x8000
#define    BIT_MASK_BB_PHCMPENABLE_N                 0x10000
#define    BIT_MASK_BB_UPDCLKOLTF_N                  0x20000
#define    BIT_MASK_BB_COMCHCFO_N                    0x40000
#define    BIT_MASK_BB_CSIESTIMODE_N                 0x80000
#define    BIT_MASK_BB_ADVUPDEQZ_N                   0x100000
#define    BIT_MASK_BB_UCHCFG_N                      0x7000000
#define    BIT_MASK_BB_UPDEQZ_N                      0x8000000

#define    BIT_MASK_BB_TXAGCRATE18_06_N              0x7f7f7f7f    // Useless
#define    BIT_MASK_BB_TXAGCRATE54_24_N              0x7f7f7f7f
#define    BIT_MASK_BB_TXAGCRATEMCS32_N              0x7f
#define    BIT_MASK_BB_TXAGCRATECCK_N                0x7f00
#define    BIT_MASK_BB_TXAGCRATEMCS3_MCS0_N          0x7f7f7f7f
#define    BIT_MASK_BB_TXAGCRATEMCS7_MCS4_N          0x7f7f7f7f
#define    BIT_MASK_BB_TXAGCRATEMCS11_MCS8_N         0x7f7f7f7f
#define    BIT_MASK_BB_TXAGCRATEMCS15_MCS12_N        0x7f7f7f7f

//Rx Pseduo noise
#define    BIT_MASK_BB_RXPESUDONOISEON_N            0x20000000    // Useless
#define    BIT_MASK_BB_RXPESUDONOISE_A_N            0xff
#define    BIT_MASK_BB_RXPESUDONOISE_B_N            0xff00
/*
#define    BIT_MASK_BB_RXPESUDONOISE_C_N            0xff0000
#define    BIT_MASK_BB_RXPESUDONOISE_D_N            0xff000000
*/
#define    BIT_MASK_BB_PESUDONOISESTATE_A_N        0xffff
#define    BIT_MASK_BB_PESUDONOISESTATE_B_N        0xffff0000
/*
#define    BIT_MASK_BB_PESUDONOISESTATE_C_N        0xffff
#define    BIT_MASK_BB_PESUDONOISESTATE_D_N        0xffff0000
*/


//
//7. RF Register
//
//Zebra1
#define    BIT_MASK_RF_ZEBRA1_HSSIENABLE_N         0x8        // Useless
#define    BIT_MASK_RF_ZEBRA1_TRXCONTROL_N         0xc00
#define    BIT_MASK_RF_ZEBRA1_TRXGAINSETTING_N     0x07f
#define    BIT_MASK_RF_ZEBRA1_RXCORNER_N           0xc00
#define    BIT_MASK_RF_ZEBRA1_TXCHARGEPUMP_N       0x38
#define    BIT_MASK_RF_ZEBRA1_RXCHARGEPUMP_N       0x7
#define    BIT_MASK_RF_ZEBRA1_CHANNELNUM_N         0xf80
#define    BIT_MASK_RF_ZEBRA1_TXLPFBW_N            0x400
#define    BIT_MASK_RF_ZEBRA1_RXLPFBW_N            0x600

//Zebra4
#define    BIT_MASK_RF_RTL8256REGMODECTRL1_N       0x100    // Useless
#define    BIT_MASK_RF_RTL8256REGMODECTRL0_N       0x40
#define    BIT_MASK_RF_RTL8256_TXLPFBW_N           0x18
#define    BIT_MASK_RF_RTL8256_RXLPFBW_N           0x600
                    
//RTL8258           
#define    BIT_MASK_RF_RTL8258_TXLPFBW_N           0xc    // Useless
#define    BIT_MASK_RF_RTL8258_RXLPFBW_N           0xc00
#define    BIT_MASK_RF_RTL8258_RSSILPFBW_N         0xc0



#endif //#if IS_RTL88XX_N

// Other Common setting bit define 

//byte endable for sb_write
#define    BIT_MASK_SET_BYTE0_COMMON                     0x1    // Useless
#define    BIT_MASK_SET_BYTE1_COMMON                     0x2
#define    BIT_MASK_SET_BYTE2_COMMON                     0x4
#define    BIT_MASK_SET_BYTE3_COMMON                     0x8
#define    BIT_MASK_SET_WORD0_COMMON                     0x3
#define    BIT_MASK_SET_WORD1_COMMON                     0xc
#define    BIT_MASK_SET_DWORD_COMMON                     0xf

//for PutRegsetting & GetRegSetting BitMask
#define    BIT_MASK_SET_MASKBYTE0_COMMON                 0xff    // Reg 0xc50 rOFDM0_XAAGCCore~0xC6f
#define    BIT_MASK_SET_MASKBYTE1_COMMON                 0xff00
#define    BIT_MASK_SET_MASKBYTE2_COMMON                 0xff0000
#define    BIT_MASK_SET_MASKBYTE3_COMMON                 0xff000000
#define    BIT_MASK_SET_MASKHWORD_COMMON                 0xffff0000
#define    BIT_MASK_SET_MASKLWORD_COMMON                 0x0000ffff
#define    BIT_MASK_SET_MASKDWORD_COMMON                 0xffffffff
#define    BIT_MASK_SET_MASKH4BITS_COMMON                0xf0000000
#define    BIT_MASK_SET_MASKH3BYTES_COMMON               0xffffff00    
#define    BIT_MASK_SET_MASKOFDM_D_COMMON                0xffc00000
#define    BIT_MASK_SET_MASKCCK_COMMON                   0x3f3f3f3f

//for PutRFRegsetting & GetRFRegSetting BitMask
#define    BIT_MASK_SET_MASK12BITS_COMMON                0xfff    // RF Reg mask bits
#define    BIT_MASK_SET_MASK20BITS_COMMON                0xfffff    // RF Reg mask bits T65 RF


#endif//__INC_HALCOMPHYBIT_H


