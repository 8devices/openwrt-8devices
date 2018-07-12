/*
 *  SNMP MIB module
 *
 *  $Id: 8192cd_mib.c,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#define _8192CD_MIB_C_

#ifdef __KERNEL__
#include <linux/module.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"

#ifdef SUPPORT_SNMP_MIB

#if defined(__KERNEL__) || defined(__OSK__)
#include "./ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#include "./8192cd_mib.h"

extern int _convert_2_pwr_dot(char *s, int base);
extern int _atoi(char *s, int base);

struct mib_val dot11_mib_table[]={
	//dot11StationConfigTable
	{ {1, 1, 1, 1, 1, -1}, "dot11StationID", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.hwaddr), PRIV_SIZE(dot11OperationEntry.hwaddr)},
	{ {1, 1, 1, 2, 1, -1}, "dot11MediumOccupancyLimit ", MIB_TYPE_VAL, 100, 1},
	{ {1, 1, 1, 3, 1, -1}, "dot11CFPollable", MIB_TYPE_VAL, 0, 1},
	{ {1, 1, 1, 4, 1, -1}, "dot11CFPPeriod", MIB_TYPE_VAL, 0, 1},
	{ {1, 1, 1, 5, 1, -1}, "dot11CFPMaxDuration", MIB_TYPE_VAL, 0, 1},
	{ {1, 1, 1, 6, 1, -1}, "dot11AuthenticationResponseTimeOut", MIB_TYPE_VAL, REAUTH_TO*10, 4},
	{ {1, 1, 1, 7, 1, -1}, "dot11PrivacyOptionImplemented", MIB_TYPE_VAL, 1, 1},
	{ {1, 1, 1, 8, 1, -1}, "dot11PowerManagementMode", MIB_TYPE_VAL, 1, 1},
	{ {1, 1, 1, 9, 1, -1}, "dot11DesiredSSID", MIB_TYPE_PRIV, PRIV_OFFSET(dot11StationConfigEntry.dot11DesiredSSID), 0},
	{ {1, 1, 1, 10, 1, -1}, "dot11DesiredBSSType", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DesiredBSSType), SNMP_SIZE(dot11DesiredBSSType)},
	{ {1, 1, 1, 11, 1, -1}, "dot11OperationalRateSet", MIB_TYPE_SNMP1, SNMP_OFFSET(dot11OperationalRateSet), SNMP_OFFSET(dot11SupportedDataRatesNum)},
	{ {1, 1, 1, 12, 1, -1}, "dot11BeaconPeriod", MIB_TYPE_PRIV, PRIV_OFFSET(dot11StationConfigEntry.dot11BeaconPeriod), PRIV_SIZE(dot11StationConfigEntry.dot11BeaconPeriod)},
	{ {1, 1, 1, 13, 1, -1}, "dot11DTIMPeriod", MIB_TYPE_PRIV, PRIV_OFFSET(dot11StationConfigEntry.dot11DTIMPeriod), PRIV_SIZE(dot11StationConfigEntry.dot11DTIMPeriod)},
	{ {1, 1, 1, 14, 1, -1}, "dot11AssociationResponseTimeOut", MIB_TYPE_VAL, REASSOC_TO*10, 4},
	{ {1, 1, 1, 15, 1, -1}, "dot11DisassociateReason", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DisassociateReason), SNMP_SIZE(dot11DisassociateReason)},
	{ {1, 1, 1, 16, 1, -1}, "dot11DisassociateStation", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DisassociateStation), SNMP_SIZE(dot11DisassociateStation)},
	{ {1, 1, 1, 17, 1, -1}, "dot11DeauthenticateReason", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DeauthenticateReason), SNMP_SIZE(dot11DeauthenticateReason)},
	{ {1, 1, 1, 18, 1, -1}, "dot11DeauthenticateStation", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DeauthenticateStation), SNMP_SIZE(dot11DeauthenticateStation)},
	{ {1, 1, 1, 19, 1, -1}, "dot11AuthenticateFailStatus", MIB_TYPE_SNMP, SNMP_OFFSET(dot11AuthenticateFailStatus), SNMP_SIZE(dot11AuthenticateFailStatus)},
	{ {1, 1, 1, 20, 1, -1}, "dot11AuthenticateFailStation", MIB_TYPE_SNMP, SNMP_OFFSET(dot11AuthenticateFailStation), SNMP_SIZE(dot11AuthenticateFailStation)},
	{ {1, 1, 1, 24, 1, -1}, "dot11RegDomain", MIB_TYPE_PRIV, PRIV_OFFSET(dot11StationConfigEntry.dot11RegDomain), PRIV_SIZE(dot11StationConfigEntry.dot11RegDomain)},
	{ {1, 1, 1, 25, 1, -1}, "dot11DataRate", MIB_TYPE_SNMP, SNMP_OFFSET(dot11DataRate), SNMP_SIZE(dot11DataRate)},
	{ {1, 1, 1, 26, 1, -1}, "dot11ProtectionDisabled", MIB_TYPE_PRIV, PRIV_OFFSET(dot11StationConfigEntry.protectionDisabled), PRIV_SIZE(dot11StationConfigEntry.protectionDisabled)},
	{ {1, 1, 1, 27, 1, -1}, "dot11nSTBC", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nSTBC), PRIV_SIZE(dot11nConfigEntry.dot11nSTBC)},
	{ {1, 1, 1, 28, 1, -1}, "dot11nCoexist", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nCoexist), PRIV_SIZE(dot11nConfigEntry.dot11nCoexist)},
	{ {1, 1, 1, 29, 1, -1}, "dot11nUse40M", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nUse40M), PRIV_SIZE(dot11nConfigEntry.dot11nUse40M)},
	{ {1, 1, 1, 30, 1, -1}, "dot11n2ndChOffset", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11n2ndChOffset), PRIV_SIZE(dot11nConfigEntry.dot11n2ndChOffset)},
	{ {1, 1, 1, 31, 1, -1}, "dot11nShortGIfor20M", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nShortGIfor20M), PRIV_SIZE(dot11nConfigEntry.dot11nShortGIfor20M)},
	{ {1, 1, 1, 32, 1, -1}, "dot11nShortGIfor40M", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nShortGIfor40M), PRIV_SIZE(dot11nConfigEntry.dot11nShortGIfor40M)},
	{ {1, 1, 1, 33, 1, -1}, "dot11nAMPDU", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nAMPDU), PRIV_SIZE(dot11nConfigEntry.dot11nAMPDU)},
	{ {1, 1, 1, 34, 1, -1}, "dot11nAMSDU", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nAMSDU), PRIV_SIZE(dot11nConfigEntry.dot11nAMSDU)},
	{ {1, 1, 1, 35, 1, -1}, "dot11nLDPC", MIB_TYPE_PRIV, PRIV_OFFSET(dot11nConfigEntry.dot11nLDPC), PRIV_SIZE(dot11nConfigEntry.dot11nLDPC)},
	//dot11AuthenticationAlgorithmsTable
	{ {1, 2, 1, 1, 1, -1}, "dot11AuthenticationAlgorithmsIndex", MIB_TYPE_VAL, 1, 1},
	{ {1, 2, 1, 2, 1, -1}, "dot11AuthenticationAlgorithm", MIB_TYPE_VAL, 1, 1},
	{ {1, 2, 1, 3, 1, -1}, "dot11AuthenticationAlgorithmsEnable", MIB_TYPE_SNMP, SNMP_OFFSET(dot11AuthenticationAlgorithmsEnableOpen), SNMP_SIZE(dot11AuthenticationAlgorithmsEnableOpen)},
	{ {1, 2, 1, 1, 2, -1}, "dot11AuthenticationAlgorithmsIndex", MIB_TYPE_VAL, 2, 1},
	{ {1, 2, 1, 2, 2, -1}, "dot11AuthenticationAlgorithm", MIB_TYPE_VAL, 2, 1},
	{ {1, 2, 1, 3, 2, -1}, "dot11AuthenticationAlgorithmsEnable", MIB_TYPE_SNMP, SNMP_OFFSET(dot11AuthenticationAlgorithmsEnableShared), SNMP_SIZE(dot11AuthenticationAlgorithmsEnableShared)},

	//dot11WEPDefaultKeysTable
	{ {1, 3, 1, 1, 1, -1}, "dot11WEPDefaultKeyIndex", MIB_TYPE_VAL, 1, 1},
	{ {1, 3, 1, 2, 1, -1}, "dot11WEPDefaultKeyValue", MIB_TYPE_SNMP1, SNMP_OFFSET(dot11WEPDefaultKey1), SNMP_SIZE(dot11WEPDefaultKeyLen)},
	{ {1, 3, 1, 1, 2, -1}, "dot11WEPDefaultKeyIndex", MIB_TYPE_VAL, 2, 1},
	{ {1, 3, 1, 2, 2, -1}, "dot11WEPDefaultKeyValue", MIB_TYPE_SNMP1, SNMP_OFFSET(dot11WEPDefaultKey2), SNMP_SIZE(dot11WEPDefaultKeyLen)},
	{ {1, 3, 1, 1, 3, -1}, "dot11WEPDefaultKeyIndex", MIB_TYPE_VAL, 3, 1},
	{ {1, 3, 1, 2, 3, -1}, "dot11WEPDefaultKeyValue", MIB_TYPE_SNMP1, SNMP_OFFSET(dot11WEPDefaultKey3), SNMP_SIZE(dot11WEPDefaultKeyLen)},
	{ {1, 3, 1, 1, 4, -1}, "dot11WEPDefaultKeyIndex", MIB_TYPE_VAL, 4, 1},
	{ {1, 3, 1, 2, 4, -1}, "dot11WEPDefaultKeyValue", MIB_TYPE_SNMP1, SNMP_OFFSET(dot11WEPDefaultKey4), SNMP_SIZE(dot11WEPDefaultKeyLen)},

	//dot11WEPKeyMappingsTable
	{ {1, 4, 1, 1, 1, -1}, "dot11WEPKeyMappingIndex", MIB_TYPE_VAL, 1, 1},
	{ {1, 4, 1, 2, 1, -1}, "dot11WEPKeyMappingAddress", 0},
	{ {1, 4, 1, 3, 1, -1}, "dot11WEPKeyMappingWEPOn", 0},
	{ {1, 4, 1, 4, 1, -1}, "dot11WEPKeyMappingValue", 0},
	{ {1, 4, 1, 5, 1, -1}, "dot11WEPKeyMappingStatus", 0},

	//dot11PrivacyTable
	{ {1, 5, 1, 1, 1, -1}, "dot11PrivacyInvoked", MIB_TYPE_SNMP, SNMP_OFFSET(dot11PrivacyInvoked), SNMP_SIZE(dot11PrivacyInvoked)},
	{ {1, 5, 1, 2, 1, -1}, "dot11WEPDefaultKeyID", MIB_TYPE_PRIV, PRIV_OFFSET(dot1180211AuthEntry.dot11PrivacyKeyIndex), PRIV_SIZE(dot1180211AuthEntry.dot11PrivacyKeyIndex)},
	{ {1, 5, 1, 3, 1, -1}, "dot11WEPKeyMappingLength", MIB_TYPE_VAL, 10, 1},
	{ {1, 5, 1, 4, 1, -1}, "dot11ExcludeUnencrypted", MIB_TYPE_SNMP, SNMP_OFFSET(dot11PrivacyInvoked), SNMP_SIZE(dot11PrivacyInvoked)},
	{ {1, 5, 1, 5, 1, -1}, "dot11WEPICVErrorCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11WEPICVErrorCount), SNMP_SIZE(dot11WEPICVErrorCount)},
	{ {1, 5, 1, 6, 1, -1}, "dot11WEPExcludedCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11WEPExcludedCount), SNMP_SIZE(dot11WEPExcludedCount)},

	{ {1, 8, 1, 1, 1, -1}, "dot11WPAKeyIndex", MIB_TYPE_VAL, 1, 1},
	{ {1, 8, 1, 2, 1, -1}, "dot11WPAKeyFormat", MIB_TYPE_VAL, 1, 1},
	{ {1, 8, 1, 3, 1, -1}, "dot11WPAKeyValue", MIB_TYPE_VAL, 1, 1},
	{ {1, 9, 1, 1, 1, -1}, "dot11WPA2KeyIndex", MIB_TYPE_VAL, 1, 1},
	{ {1, 9, 1, 2, 1, -1}, "dot11WPA2KeyFormat", MIB_TYPE_VAL, 1, 1},
	{ {1, 9, 1, 3, 1, -1}, "dot11WPA2KeyValue", MIB_TYPE_VAL, 1, 1},
	{ {1, 10, 1, 1, 1, -1}, "dot11RadiusServerIPAddress", MIB_TYPE_VAL, 1, 1},
	{ {1, 10, 1, 2, 1, -1}, "dot11RadiusServerPort", MIB_TYPE_VAL, 1, 1},
	{ {1, 10, 1, 3, 1, -1}, "dot11RadiusServerPassword", MIB_TYPE_VAL, 1, 1},
	{ {1, 10, 1, 4, 1, -1}, "dot11RadiusServerEnable", MIB_TYPE_VAL, 1, 1},	
	//dot11OperationTable
	{ {2, 1, 1, 1, 1, -1}, "dot11MACAddress", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.hwaddr), PRIV_SIZE(dot11OperationEntry.hwaddr)},
	{ {2, 1, 1, 2, 1, -1}, "dot11RTSThreshold", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.dot11RTSThreshold), PRIV_SIZE(dot11OperationEntry.dot11RTSThreshold)},
	{ {2, 1, 1, 3, 1, -1}, "dot11ShortRetryLimit", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.dot11ShortRetryLimit), PRIV_SIZE(dot11OperationEntry.dot11ShortRetryLimit)},
	{ {2, 1, 1, 4, 1, -1}, "dot11LongRetryLimit", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.dot11LongRetryLimit), PRIV_SIZE(dot11OperationEntry.dot11LongRetryLimit)},
	{ {2, 1, 1, 5, 1, -1}, "dot11FragmentationThreshold", MIB_TYPE_PRIV, PRIV_OFFSET(dot11OperationEntry.dot11FragmentationThreshold), PRIV_SIZE(dot11OperationEntry.dot11FragmentationThreshold)},
	{ {2, 1, 1, 6, 1, -1}, "dot11MaxTransmitMSDULifetime", MIB_TYPE_VAL, 512, 1},
	{ {2, 1, 1, 7, 1, -1}, "dot11MaxReceiveLifetime", MIB_TYPE_VAL, FRAG_TO*10, 4},
	{ {2, 1, 1, 8, 1, -1}, "dot11ManufacturerID", MIB_TYPE_VAL, (int)"Realtek", 0},
	{ {2, 1, 1, 9, 1, -1}, "dot11ProductID", MIB_TYPE_VAL, (int)"RTL8185/RTL8186", 0},
	{ {2, 1, 1, 10, 1, -1}, "dot11IappEnable", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.iapp_enable), PRIV_SIZE(dot11OperationEntry.iapp_enable)},
	{ {2, 1, 1, 11, 1, -1}, "dot11BlockRelay", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.block_relay), PRIV_SIZE(dot11OperationEntry.block_relay)},
	{ {2, 1, 1, 12, 1, -1}, "dot11WIFISpecific", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.wifi_specific), PRIV_SIZE(dot11OperationEntry.wifi_specific)},
	{ {2, 1, 1, 13, 1, -1}, "dot11QosEnable", MIB_TYPE_PRIV,PRIV_OFFSET(dot11QosEntry.dot11QosEnable), PRIV_SIZE(dot11QosEntry.dot11QosEnable)},
	{ {2, 1, 1, 14, 1, -1}, "dot11NetworkType", MIB_TYPE_PRIV,PRIV_OFFSET(dot11BssType.net_work_type), PRIV_SIZE(dot11BssType.net_work_type)},
	{ {2, 1, 1, 15, 1, -1}, "dot11LedType", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.ledtype), PRIV_SIZE(dot11OperationEntry.ledtype)},						
	{ {2, 1, 1, 16, 1, -1}, "dot11OperationMode", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.opmode), PRIV_SIZE(dot11OperationEntry.opmode)},	
	{ {2, 1, 1, 17, 1, -1}, "dot11BroadCastSSID", MIB_TYPE_PRIV,PRIV_OFFSET(miscEntry.show_hidden_bss), PRIV_SIZE(miscEntry.show_hidden_bss)},
	{ {2, 1, 1, 18, 1, -1}, "dot11GuestAccess", MIB_TYPE_PRIV,PRIV_OFFSET(dot11OperationEntry.guest_access), PRIV_SIZE(dot11OperationEntry.guest_access)},	

	//dot11CountersTable
	{ {2, 2, 1, 1, 1, -1}, "dot11TransmittedFragmentCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11TransmittedFragmentCount), SNMP_SIZE(dot11TransmittedFragmentCount)},
	{ {2, 2, 1, 2, 1, -1}, "dot11MulticastTransmittedFrameCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11MulticastTransmittedFrameCount), SNMP_SIZE(dot11MulticastTransmittedFrameCount)},
	{ {2, 2, 1, 3, 1, -1}, "dot11FailedCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11FailedCount), SNMP_SIZE(dot11FailedCount)},
	{ {2, 2, 1, 4, 1, -1}, "dot11RetryCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11RetryCount), SNMP_SIZE(dot11RetryCount)},
	{ {2, 2, 1, 5, 1, -1}, "dot11MultipleRetryCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11MultipleRetryCount), SNMP_SIZE(dot11MultipleRetryCount)},
	{ {2, 2, 1, 6, 1, -1}, "dot11FrameDuplicateCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11FrameDuplicateCount), SNMP_SIZE(dot11FrameDuplicateCount)},
	{ {2, 2, 1, 7, 1, -1}, "dot11RTSSuccessCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11RTSSuccessCount), SNMP_SIZE(dot11RTSSuccessCount)},
	{ {2, 2, 1, 8, 1, -1}, "dot11RTSFailureCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11RTSFailureCount), SNMP_SIZE(dot11RTSFailureCount)},
	{ {2, 2, 1, 9, 1, -1}, "dot11ACKFailureCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11ACKFailureCount), SNMP_SIZE(dot11ACKFailureCount)},
	{ {2, 2, 1, 10, 1, -1}, "dot11ReceivedFragmentCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11ReceivedFragmentCount), SNMP_SIZE(dot11ReceivedFragmentCount)},
	{ {2, 2, 1, 11, 1, -1}, "dot11MulticastReceivedFrameCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11MulticastReceivedFrameCount), SNMP_SIZE(dot11MulticastReceivedFrameCount)},
	{ {2, 2, 1, 12, 1, -1}, "dot11FCSErrorCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11FCSErrorCount), SNMP_SIZE(dot11FCSErrorCount)},
	{ {2, 2, 1, 13, 1, -1}, "dot11TransmittedFrameCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11TransmittedFrameCount), SNMP_SIZE(dot11TransmittedFrameCount)},
	{ {2, 2, 1, 14, 1, -1}, "dot11WEPUndecryptableCount", MIB_TYPE_SNMP, SNMP_OFFSET(dot11WEPUndecryptableCount), SNMP_SIZE(dot11WEPUndecryptableCount)},

	//dot11GroupAddressesTable
	{ {2, 3, 1, 1, 1, -1}, "dot11GroupAddressesIndex", MIB_TYPE_VAL, 1, 1},
	{ {2, 3, 1, 2, 1, -1}, "dot11Address", 0},
	{ {2, 3, 1, 3, 1, -1}, "dot11GroupAddressesStatus", 0},

	{ {3, 1, 1, 0, -1}, "dot11ResourceTypeIDName", MIB_TYPE_VAL, (int)"8185", 0},

	//dot11ResourceInfoTable
	{ {3, 1, 2, 1, 1, 1, -1}, "dot11manufacturerOUI", MIB_TYPE_VAL, (int)"\x00\xE0\x4C", 3},
	{ {3, 1, 2, 1, 2, 1, -1}, "dot11manufacturerName", MIB_TYPE_VAL, (int)"Realtek", 0},
	{ {3, 1, 2, 1, 3, 1, -1}, "dot11manufacturerProductName", MIB_TYPE_VAL, (int)"Wireless 802.11 b/g", 0},
	{ {3, 1, 2, 1, 4, 1, -1}, "dot11manufacturerProductVersion", MIB_TYPE_VAL, (int)"v1.00", 0},

	//dot11PhyOperationTable
	{ {4, 1, 1, 1, 1, -1}, "dot11PHYType", MIB_TYPE_VAL, 2, 1},
	{ {4, 1, 1, 2, 1, -1}, "dot11CurrentRegDomain", MIB_TYPE_SNMP, SNMP_OFFSET(dot11CurrentRegDomain), SNMP_SIZE(dot11CurrentRegDomain)},
	{ {4, 1, 1, 3, 1, -1}, "dot11TempType", MIB_TYPE_VAL, 1, 1},
	{ {4, 1, 1, 4, 1, -1}, "dot11ShortPreamble",MIB_TYPE_PRIV, PRIV_OFFSET(dot11RFEntry.shortpreamble), PRIV_SIZE(dot11RFEntry.shortpreamble)},	
	{ {4, 1, 1, 5, 1, -1}, "dot11PhyBandSelect",MIB_TYPE_PRIV, PRIV_OFFSET(dot11RFEntry.phyBandSelect), PRIV_SIZE(dot11RFEntry.phyBandSelect)},	
	{ {4, 1, 1, 6, 1, -1}, "dot11TrSwitch",MIB_TYPE_PRIV, PRIV_OFFSET(dot11RFEntry.trswitch), PRIV_SIZE(dot11RFEntry.trswitch)},	
	{ {4, 1, 1, 7, 1, -1}, "dot11PowerScale", MIB_TYPE_SNMP, SNMP_OFFSET(dot11PowerScale), SNMP_SIZE(dot11PowerScale)},

	//dot11PhyAntennaTable
	{ {4, 2, 1, 1, 1, -1}, "dot11CurrentTxAntenna", MIB_TYPE_SNMP, SNMP_OFFSET(dot11CurrentTxAntenna), SNMP_SIZE(dot11CurrentTxAntenna)},
	{ {4, 2, 1, 2, 1, -1}, "dot11DiversitySupport", MIB_TYPE_VAL, 2, 1},
	{ {4, 2, 1, 3, 1, -1}, "dot11CurrentRxAntenna", MIB_TYPE_SNMP, SNMP_OFFSET(dot11CurrentTxAntenna), SNMP_SIZE(dot11CurrentTxAntenna)},

	//dot11PhyTxPowerTable
	{ {4, 3, 1, 1, 1, -1}, "dot11NumberSupportedPowerLevels", MIB_TYPE_VAL, 4, 1},
	{ {4, 3, 1, 2, 1, -1}, "dot11TxPowerLevel1", MIB_TYPE_VAL, 25, 1},
	{ {4, 3, 1, 3, 1, -1}, "dot11TxPowerLevel2", MIB_TYPE_VAL, 12, 1},
	{ {4, 3, 1, 4, 1, -1}, "dot11TxPowerLevel3", MIB_TYPE_VAL, 6, 1},
	{ {4, 3, 1, 5, 1, -1}, "dot11TxPowerLevel4", MIB_TYPE_VAL, 3, 1},
	{ {4, 3, 1, 6, 1, -1}, "dot11TxPowerLevel5", MIB_TYPE_VAL, 0, 1},
	{ {4, 3, 1, 7, 1, -1}, "dot11TxPowerLevel6", MIB_TYPE_VAL, 0, 1},
	{ {4, 3, 1, 8, 1, -1}, "dot11TxPowerLevel7", MIB_TYPE_VAL, 0, 1},
	{ {4, 3, 1, 9, 1, -1}, "dot11TxPowerLevel8", MIB_TYPE_VAL, 0, 1},
	{ {4, 3, 1, 10, 1, -1}, "dot11CurrentTxPowerLevel", 0}, // NCTU

	//dot11PhyDSSSTable
	{ {4, 5, 1, 1, 1, -1}, "dot11CurrentChannel", MIB_TYPE_PRIV, PRIV_OFFSET(dot11RFEntry.dot11channel), PRIV_SIZE(dot11RFEntry.dot11channel)},
	{ {4, 5, 1, 2, 1, -1}, "dot11CCAModeSupported", MIB_TYPE_VAL, 4, 1},
	{ {4, 5, 1, 3, 1, -1}, "dot11CurrentCCAMode", MIB_TYPE_VAL, 4, 1},
	{ {4, 5, 1, 4, 1, -1}, "dot11EDThreshold", MIB_TYPE_VAL, 0, 1},

	//dot11RegDomainsSupportedTable
	{ {4, 7, 1, 1, 1, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 1, 1},
	{ {4, 7, 1, 2, 1, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 16, 1},
	{ {4, 7, 1, 1, 2, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 2, 1},
	{ {4, 7, 1, 2, 2, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 32, 1},
	{ {4, 7, 1, 1, 3, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 3, 1},
	{ {4, 7, 1, 2, 3, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 48, 1},
	{ {4, 7, 1, 1, 4, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 4, 1},
	{ {4, 7, 1, 2, 4, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 49, 1},
	{ {4, 7, 1, 1, 5, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 5, 1},
	{ {4, 7, 1, 2, 5, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 50, 1},
	{ {4, 7, 1, 1, 6, -1}, "dot11RegDomainsSupportIndex", MIB_TYPE_VAL, 6, 1},
	{ {4, 7, 1, 2, 6, -1}, "dot11RegDomainsSupportValue", MIB_TYPE_VAL, 64, 1},

	//dot11AntennasListTable
	{ {4, 8, 1, 1, 1, -1}, "dot11AntennaListIndex", MIB_TYPE_VAL, 1, 1},
	{ {4, 8, 1, 2, 1, -1}, "dot11SupportedTxAntenna", MIB_TYPE_VAL, 1, 1},
	{ {4, 8, 1, 3, 1, -1}, "dot11SupportedRxAntenna", MIB_TYPE_VAL, 1, 1},
	{ {4, 8, 1, 4, 1, -1}, "dot11DiversitySelectionRx", MIB_TYPE_VAL, 1, 1},

	//dot11SupportedDataRatesTxTable
	{ {4, 9, 1, 1, 1, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 1, 1},
	{ {4, 9, 1, 2, 1, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[0]), 4},
	{ {4, 9, 1, 1, 2, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 2, 1},
	{ {4, 9, 1, 2, 2, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[1]), 4},
	{ {4, 9, 1, 1, 3, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 3, 1},
	{ {4, 9, 1, 2, 3, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[2]), 4},
	{ {4, 9, 1, 1, 4, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 4, 1},
	{ {4, 9, 1, 2, 4, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[3]), 4},
	{ {4, 9, 1, 1, 5, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 5, 1},
	{ {4, 9, 1, 2, 5, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[4]), 4},
	{ {4, 9, 1, 1, 6, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 6, 1},
	{ {4, 9, 1, 2, 6, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[5]), 4},
	{ {4, 9, 1, 1, 7, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 7, 1},
	{ {4, 9, 1, 2, 7, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[6]), 4},
	{ {4, 9, 1, 1, 8, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 8, 1},
	{ {4, 9, 1, 2, 8, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[7]), 4},
	{ {4, 9, 1, 1, 9, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 9, 1},
	{ {4, 9, 1, 2, 9, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[8]), 4},
	{ {4, 9, 1, 1, 10, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 10, 1},
	{ {4, 9, 1, 2, 10, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[9]), 4},
	{ {4, 9, 1, 1, 11, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 11, 1},
	{ {4, 9, 1, 2, 11, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[10]), 4},
	{ {4, 9, 1, 1, 12, -1}, "dot11SupportedDataRatesTxIndex", MIB_TYPE_VAL, 12, 1},
	{ {4, 9, 1, 2, 12, -1}, "dot11SupportedDataRatesTxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[11]), 4},

	//dot11SupportedDataRatesRxTable
 	{ {4, 10, 1, 1, 1, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 1, 1},
 	{ {4, 10, 1, 2, 1, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[0]), 4},
 	{ {4, 10, 1, 1, 2, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 2, 1},
 	{ {4, 10, 1, 2, 2, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[1]), 4},
 	{ {4, 10, 1, 1, 3, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 3, 1},
 	{ {4, 10, 1, 2, 3, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[2]), 4},
 	{ {4, 10, 1, 1, 4, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 4, 1},
 	{ {4, 10, 1, 2, 4, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[3]), 4},
 	{ {4, 10, 1, 1, 5, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 5, 1},
 	{ {4, 10, 1, 2, 5, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[4]), 4},
 	{ {4, 10, 1, 1, 6, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 6, 1},
 	{ {4, 10, 1, 2, 6, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[5]), 4},
 	{ {4, 10, 1, 1, 7, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 7, 1},
 	{ {4, 10, 1, 2, 7, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[6]), 4},
 	{ {4, 10, 1, 1, 8, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 8, 1},
 	{ {4, 10, 1, 2, 8, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[7]), 4},
 	{ {4, 10, 1, 1, 9, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 9, 1},
 	{ {4, 10, 1, 2, 9, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[8]), 4},
 	{ {4, 10, 1, 1, 10, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 10, 1},
 	{ {4, 10, 1, 2, 10, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[9]), 4},
 	{ {4, 10, 1, 1, 11, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 11, 1},
 	{ {4, 10, 1, 2, 11, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[10]), 4},
 	{ {4, 10, 1, 1, 12, -1}, "dot11SupportedDataRatesRxIndex", MIB_TYPE_VAL, 12, 1},
 	{ {4, 10, 1, 2, 12, -1}, "dot11SupportedDataRatesRxValue", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesSet[11]), 4},

	// Number of table entry, Realtek proprietary
	{ {6, 1, -1}, "dot11StationConfigEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 2, -1}, "dot11AuthenticationAlgorithmsEntryNum", MIB_TYPE_VAL, 2, 1},
	{ {6, 3, -1}, "dot11WEPDefaultKeysEntryNum", MIB_TYPE_VAL, 4, 1},
	{ {6, 4, -1}, "dot11WEPKeyMappingsEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 5, -1}, "dot11PrivacyEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 6, -1}, "dot11OperationEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 7, -1}, "dot11CountersEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 8, -1}, "dot11GroupAddressesEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 9, -1}, "dot11ResourceInfoTableNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 10, -1}, "dot11PhyOperationEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 11, -1}, "dot11PhyAntennaEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 12, -1}, "dot11PhyTxPowerEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 13, -1}, "dot11PhyDSSSEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 14, -1}, "dot11RegDomainsSupportEntryNum", MIB_TYPE_VAL, 6, 1},
	{ {6, 15, -1}, "dot11AntennasListEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 16, -1}, "dot11AntennasListEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 17, -1}, "dot11SupportedDataRatesTxEntryNum", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesNum), SNMP_SIZE(dot11SupportedDataRatesNum)},
	{ {6, 18, -1}, "dot11SupportedDataRatesRxEntryNum", MIB_TYPE_SNMP, SNMP_OFFSET(dot11SupportedDataRatesNum), SNMP_SIZE(dot11SupportedDataRatesNum)},
	{ {6, 19, -1}, "dot11MultiDomainCapabilityEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 20, -1}, "dot11WPAKeysEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 21, -1}, "dot11WPA2KeysEntryNum", MIB_TYPE_VAL, 1, 1},
	{ {6, 22, -1}, "dot11RadiusServerNum", MIB_TYPE_VAL, 1, 1},

	{ {-1}, NULL, 0}
};


static int add_mib_entry(struct rtl8192cd_priv *priv, struct mib_val *ent)
{
	int i=0, round = 0;
	struct mib_entry **mib=&priv->snmp_mib.mib_tree, *pre_mib=NULL;

	while(1) {
		if (ent->oid[i] == -1)
			break;
		if (*mib == NULL) {
			if (priv->snmp_mib.tree_used_index == MAX_MIB_TREE_NUM) {
				printk("Exceed max mib tree number\n");
				return -1;
			}
			*mib = &priv->snmp_mib.mib_list[priv->snmp_mib.tree_used_index++];
		}

		if ((*mib)->id == -1 || ent->oid[i] == (*mib)->id) {
			if ((*mib)->id == -1)
				(*mib)->id = ent->oid[i];
			i++;
			pre_mib = *mib;
			mib = &((*mib)->next_level);
		}
		else
			mib = &((*mib)->next_node);

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	if (pre_mib == NULL) {
		printk("build mib tree error, no parent mib found!\n");
		return -1;
	}
	if (pre_mib->val) {
		printk("build mib tree error, mib value has been assigned!\n");
		return -1;
	}
	pre_mib->val = ent;
	return 0;
}

static void name2oid(char *name, int *oid)
{
	int i=0;
	char *src=name, *ptr;
	while (*src) {
		ptr = strstr(src, ".");
		if (ptr)
			*ptr = '\0';
		oid[i++] = _atoi(src, 10);

		if (ptr==NULL)
			break;
		src = ++ptr;
	}
	oid[i] = -1;
}

static struct mib_val *search_mib(struct rtl8192cd_priv *priv, char *id)
{
	int i=0;
	struct mib_entry *mib=priv->snmp_mib.mib_tree, *pre_mib=NULL;
	int oid[100];

	name2oid(id, oid);

	while (oid[i] != -1) {
		if (mib == NULL || mib->id == -1)
			return NULL;
		if (mib->id == oid[i]) {
			pre_mib = mib;
			mib = mib->next_level;
			i++;
		}
		else {
			pre_mib = NULL;
			mib = mib->next_node;
		}
	}

	if (pre_mib && pre_mib->val)
		return pre_mib->val;
	else
		return NULL;
}

static void build_tree(struct rtl8192cd_priv *priv)
{
	int i=0;
	struct mib_val *tbl = dot11_mib_table;

	for (i=0; i<MAX_MIB_TREE_NUM; i++)
		priv->snmp_mib.mib_list[i].id = -1;

	i=0;
	while (tbl[i].name) {
		if (add_mib_entry(priv, &tbl[i]) < 0) {
			printk("Add mib entry failed [%s]!\n", tbl[i].name);
			return;
		}
		i++;
	}

//	printk("used mib num = %d\n", priv->snmp_mib.tree_used_index);

}

int mib_get(struct rtl8192cd_priv *priv, char *oid, unsigned char *data, int *pLen)
{
 	int iVal;
 	unsigned char bVal, *pVal, *pVal_type=data++;
 	struct mib_val *mib;

	mib = search_mib(priv, oid);
	if (mib == NULL) {
		printk("search mib failed [oid=%s]!\n", oid);
		return 0;
	}

	*pVal_type = VAL_OCTET;

	switch (mib->type) {
		case MIB_TYPE_PRIV:
			if (mib->size)
				*pLen = mib->size;
			else {
				*pLen = strlen(((unsigned char *)priv->pmib)+mib->offset)+1;
				*pVal_type = VAL_STR;
			}
			memcpy(data, ((unsigned char *)priv->pmib)+mib->offset, *pLen);
			break;

		case MIB_TYPE_SNMP:
		case MIB_TYPE_SNMP1:
			if (mib->type == MIB_TYPE_SNMP1) {
				bVal = *(((unsigned char *)&priv->snmp_mib)+mib->size);
				*pLen = (int)bVal;
			}
			else if (mib->size > 0)
				*pLen = mib->size;
			else if (mib->size == 0) {
				*pLen = strlen(((unsigned char *)&priv->snmp_mib)+mib->offset) + 1;
				*pVal_type = VAL_STR;
			}
			memcpy(data, ((unsigned char *)&priv->snmp_mib)+mib->offset, *pLen);
			break;

		case MIB_TYPE_VAL:
			if (mib->size == 0) {
				pVal = (unsigned char *)mib->offset;
				*pLen = strlen(pVal) + 1;
				memcpy(data, pVal, *pLen);
				*pVal_type = VAL_STR;
			} else if (mib->size == 1) {
				bVal = (unsigned char)mib->offset;
				pVal = &bVal;
				*pLen  = sizeof(bVal);
			}
			else {
				iVal = mib->offset;
				pVal = (unsigned char *)&iVal;
				*pLen = sizeof(iVal);
			}
			memcpy(data, pVal, *pLen);
			break;

		default:
//			printk("Invalid mib type [%d, %s]\n", mib->type, mib->name);
			*pVal_type = VAL_NULL;
			*pLen = 0;
			break;
	}

#if 0
	{
		int i;
		printk("name=%s, val_type=%d, len=%d", mib->name, *pVal_type, *pLen);
		if (*pVal_type == VAL_STR)
			printk(", val=%s\n", data);
		else {
			printk(", val=");
			for (i=0; i<*pLen; i++) {
				printk("%02x ", data[i]);
			}
			printk("\n");
		}
	}
#endif
	*pLen += 1;
	return 1;
}

// build tree and mapping mib value from priv to dot11
void mib_init(struct rtl8192cd_priv *priv)
{
	int val;

	memset(&priv->snmp_mib, '\0', sizeof(struct mib_snmp));
	build_tree(priv);

	if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
		SNMP_MIB_ASSIGN(dot11DesiredBSSType, 1); // infra
	else
		SNMP_MIB_ASSIGN(dot11DesiredBSSType, 2); // ad-hoc

	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm > 0)
		SNMP_MIB_ASSIGN(dot11PrivacyInvoked, 1);

	if (priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm == 1) // shared key
		SNMP_MIB_ASSIGN(dot11AuthenticationAlgorithmsEnableShared, 1);
	else
		SNMP_MIB_ASSIGN(dot11AuthenticationAlgorithmsEnableOpen, 1);

	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) {
		SNMP_MIB_COPY(dot11WEPDefaultKey1, &priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], 5);
		SNMP_MIB_COPY(dot11WEPDefaultKey2,	&priv->pmib->dot11DefaultKeysTable.keytype[1].skey[0], 5);
		SNMP_MIB_COPY(dot11WEPDefaultKey3,	&priv->pmib->dot11DefaultKeysTable.keytype[2].skey[0], 5);
		SNMP_MIB_COPY(dot11WEPDefaultKey4, &priv->pmib->dot11DefaultKeysTable.keytype[3].skey[0], 5);
		SNMP_MIB_ASSIGN(dot11WEPDefaultKeyLen, 5);
	}
	else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) {
		SNMP_MIB_COPY(dot11WEPDefaultKey1, &priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], 13);
		SNMP_MIB_COPY(dot11WEPDefaultKey2,	&priv->pmib->dot11DefaultKeysTable.keytype[1].skey[0], 13);
		SNMP_MIB_COPY(dot11WEPDefaultKey3,	&priv->pmib->dot11DefaultKeysTable.keytype[2].skey[0], 13);
		SNMP_MIB_COPY(dot11WEPDefaultKey4, &priv->pmib->dot11DefaultKeysTable.keytype[3].skey[0], 13);
		SNMP_MIB_ASSIGN(dot11WEPDefaultKeyLen, 13);
	}
	get_oper_rate(priv);

	switch(priv->pmib->dot11StationConfigEntry.dot11RegDomain) {
		case DOMAIN_FCC:
			val = 16;
			break;
		case DOMAIN_IC:
			val = 32;
			break;
		case DOMAIN_ETSI:
			val = 48;
			break;
		case DOMAIN_SPAIN:
			val = 49;
			break;
		case DOMAIN_FRANCE:
			val = 50;
			break;
		case DOMAIN_MKK:
		case DOMAIN_MKK1:
		case DOMAIN_MKK2:
		case DOMAIN_MKK3:
			val = 64;
			break;
		default:
			printk("Invalid dot11RegDomain [%d]!\n", priv->pmib->dot11StationConfigEntry.dot11RegDomain);
			val = 16;
			break;
	}
	SNMP_MIB_ASSIGN(dot11CurrentRegDomain, val);
 	SNMP_MIB_ASSIGN(dot11CurrentTxAntenna, priv->pmib->dot11RFEntry.defaultAntennaB+1);
}
#endif // SUPPORT_SNMP_MIB

