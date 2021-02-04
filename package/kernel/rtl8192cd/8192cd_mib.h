/*
 *  Header files of SNMP MIB module
 *
 *  $Id: 8192cd_mib.h,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_MIB_H_
#define _8192CD_MIB_H_

#define MAX_MIB_TREE_NUM	580
#define MAX_OID_LEVEL	10

#define PRIV_OFFSET(field)	((int)(long *)&(((struct wifi_mib *)0)->field))
#define PRIV_SIZE(field)	sizeof(((struct wifi_mib *)0)->field)
#define SNMP_OFFSET(field)	((int)(long *)&(((struct mib_snmp *)0)->field))
#define SNMP_SIZE(field)	sizeof(((struct mib_snmp *)0)->field)

enum { MIB_TYPE_VAL=1, MIB_TYPE_PRIV, MIB_TYPE_SNMP, MIB_TYPE_SNMP1 };
enum { VAL_OCTET, VAL_STR, VAL_NULL };

struct mib_entry {
	int id;
	struct mib_entry *next_level;
	struct mib_entry *next_node;
	struct mib_val *val;
};

struct mib_val {
	int oid[MAX_OID_LEVEL];
	char *name;
	int type;
	int offset;
	int size;
};

struct mib_snmp {
	unsigned char dot11AuthenticationAlgorithmsEnableOpen;
	unsigned char dot11AuthenticationAlgorithmsEnableShared;
	unsigned char dot11WEPDefaultKey1[13];
	unsigned char dot11WEPDefaultKey2[13];
	unsigned char dot11WEPDefaultKey3[13];
	unsigned char dot11WEPDefaultKey4[13];
	unsigned char dot11WEPDefaultKeyLen;
	unsigned char dot11SupportedDataRatesNum;
	unsigned int dot11SupportedDataRatesSet[12];
	unsigned char dot11OperationalRateSet[12];
	int dot11DesiredBSSType;
	int dot11DisassociateReason;
	unsigned char dot11DisassociateStation[6];
	int dot11DeauthenticateReason;
	unsigned char dot11DeauthenticateStation[6];
	int dot11AuthenticateFailStatus;
	unsigned char dot11AuthenticateFailStation[6];
	int dot11PrivacyInvoked;
	int dot11WEPICVErrorCount;
	int dot11WEPExcludedCount;
	int dot11TransmittedFragmentCount;
	int dot11MulticastTransmittedFrameCount;
	int dot11FailedCount;
	int dot11RetryCount;
	int dot11MultipleRetryCount;
	int dot11FrameDuplicateCount;
	int dot11RTSSuccessCount;
	int dot11RTSFailureCount;
	int dot11ACKFailureCount;
	int dot11ReceivedFragmentCount;
	int dot11MulticastReceivedFrameCount;
	int dot11FCSErrorCount;
	int dot11TransmittedFrameCount;
	int dot11WEPUndecryptableCount;
	int dot11CurrentRegDomain;
	int dot11CurrentTxAntenna;
	int dot11DataRate;
	int dot11PowerScale;

	struct mib_entry mib_list[MAX_MIB_TREE_NUM];
	int tree_used_index;
	struct mib_entry *mib_tree;
};
#endif // _8192CD_MIB_H_

