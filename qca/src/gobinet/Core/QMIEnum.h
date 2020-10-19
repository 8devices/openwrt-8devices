/*===========================================================================
FILE:
   QMIEnum.h

DESCRIPTION:
   QMI protocol enumerations and related methods

PUBLIC ENUMERATIONS AND METHODS:
   eQMIService
   eQMIMessageCTL
   eQMIMessageWDS
   eQMIMessageDMS
   eQMIMessageNAS
   eQMIMessageWMS
   eQMIMessagePDS
   eQMIMessageAUTH
   eQMIMessageAT
   eQMIMessageVoice
   eQMIMessageUIM
   eQMIMessagePBM
   eQMIMessageSAR
   eQMIMessageRMTFS
   eQMIMessageWDA
   eQMIMessageQCMAP
   eQMIMessageCOEX
   eQMIMessagePDC
   eQMIMessageSTX
   eQMIMessageRFRPE
   eQMIMessageCAT
   eQMIMessageRMS
   eQMIMessageOMA
   eQMIResultCode
   eQMIErrorCode
   eQMICallEndReason
   eIPAddressType
   eSMSMessageMode

Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived from 
      this software without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Invalid QMI transaction ID
const ULONG INVALID_QMI_TRANSACTION_ID = 0;

// QMI DMS PRL size constants
const ULONG QMI_DMS_MAX_PRL_SIZE = 16384;
const ULONG QMI_DMS_MAX_PRL_BLOCK = 256;

/*=========================================================================*/
// eQMIService Enumeration
//    QMI Service Type Enumeration
/*=========================================================================*/
enum eQMIService
{
   eQMI_SVC_ENUM_BEGIN = -1, 

   eQMI_SVC_CONTROL,       // 000 Control service
   eQMI_SVC_WDS,           // 001 Wireless data service
   eQMI_SVC_DMS,           // 002 Device management service
   eQMI_SVC_NAS,           // 003 Network access service
   eQMI_SVC_QOS,           // 004 Quality of service, err, service 
   eQMI_SVC_WMS,           // 005 Wireless messaging service
   eQMI_SVC_PDS,           // 006 Position determination service
   eQMI_SVC_AUTH,          // 007 Authentication service
   eQMI_SVC_AT,            // 008 AT command processor service
   eQMI_SVC_VOICE,         // 009 Voice service
   eQMI_SVC_CAT2,          // 010 Card application toolkit service (new)
   eQMI_SVC_UIM,           // 011 UIM service
   eQMI_SVC_PBM,           // 012 Phonebook service
   eQMI_SVC_QCHAT,         // 013 QCHAT Service
   eQMI_SVC_RMTFS,         // 014 Remote file system service
   eQMI_SVC_TEST,          // 015 Test service
   eQMI_SVC_LOC,           // 016 Location service 
   eQMI_SVC_SAR,           // 017 Specific absorption rate service
   eQMI_SVC_IMSS,          // 018 IMS settings service
   eQMI_SVC_ADC,           // 019 Analog to digital converter driver service
   eQMI_SVC_CSD,           // 020 Core sound driver service
   eQMI_SVC_MFS,           // 021 Modem embedded file system service
   eQMI_SVC_TIME,          // 022 Time service
   eQMI_SVC_TS,            // 023 Thermal sensors service
   eQMI_SVC_TMD,           // 024 Thermal mitigation device service
   eQMI_SVC_SAP,           // 025 Service access proxy service
   eQMI_SVC_WDA,           // 026 Wireless data administrative service
   eQMI_SVC_TSYNC,         // 027 TSYNC control service 
   eQMI_SVC_RFSA,          // 028 Remote file system access service
   eQMI_SVC_CSVT,          // 029 Circuit switched videotelephony service
   eQMI_SVC_QCMAP,         // 030 Qualcomm mobile access point service
   eQMI_SVC_IMSP,          // 031 IMS presence service
   eQMI_SVC_IMSVT,         // 032 IMS videotelephony service
   eQMI_SVC_IMSA,          // 033 IMS application service
   eQMI_SVC_COEX,          // 034 Coexistence service
   eQMI_SVC_RESERVED_35,   // 035 Reserved
   eQMI_SVC_PDC,           // 036 Persistent device configuration service
   eQMI_SVC_RESERVED_37,   // 037 Reserved
   eQMI_SVC_STX,           // 038 Simultaneous transmit service
   eQMI_SVC_BIT,           // 039 Bearer independent transport service
   eQMI_SVC_IMSRTP,        // 040 IMS RTP service
   eQMI_SVC_RFRPE,         // 041 RF radiated performance enhancement service
   eQMI_SVC_DSD,           // 042 Data system determination service
   eQMI_SVC_SSCTL,         // 043 Subsystem control service

   eQMI_SVC_CAT = 224,     // 224 Card application toolkit service
   eQMI_SVC_RMS,           // 225 Remote management service
   eQMI_SVC_OMA,           // 226 Open mobile alliance dev mgmt service

   eQMI_SVC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIService validity check

PARAMETERS:
   svc         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIService svc )
{
   bool retVal = false;
   if ( (svc > eQMI_SVC_ENUM_BEGIN && svc <= eQMI_SVC_SSCTL)
   ||   (svc >= eQMI_SVC_CAT && svc < eQMI_SVC_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageCTL Enumeration
//    QMI Control Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageCTL
{
   eQMI_CTL_ENUM_BEGIN = -1, 

   eQMI_CTL_SET_INSTANCE_ID = 32,   // 32 Set the unique link instance ID
   eQMI_CTL_GET_VERSION_INFO,       // 33 Get supported service version info
   eQMI_CTL_GET_CLIENT_ID,          // 34 Get a unique client ID 
   eQMI_CTL_RELEASE_CLIENT_ID,      // 35 Release the unique client ID 
   eQMI_CTL_REVOKE_CLIENT_ID_IND,   // 36 Indication of client ID revocation
   eQMI_CTL_INVALID_CLIENT_ID,      // 37 Indication of invalid client ID
   eQMI_CTL_SET_DATA_FORMAT,        // 38 Set host driver data format 
   eQMI_CTL_SYNC,                   // 39 Synchronize client/server
   eQMI_CTL_SYNC_IND = 39,          // 39 Synchronize indication
   eQMI_CTL_SET_EVENT,              // 40 Set event report conditions
   eQMI_CTL_EVENT_IND = 40,         // 40 Event report indication
   eQMI_CTL_SET_POWER_SAVE_CFG,     // 41 Set power save config
   eQMI_CTL_SET_POWER_SAVE_MODE,    // 42 Set power save mode
   eQMI_CTL_GET_POWER_SAVE_MODE,    // 43 Get power save mode

   eQMI_CTL_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageCTL validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageCTL msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_CTL_SET_INSTANCE_ID && msgID < eQMI_CTL_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageWDS Enumeration
//    QMI WDS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageWDS
{
   eQMI_WDS_ENUM_BEGIN = -1, 

   eQMI_WDS_RESET,                // 000 Reset WDS service state variables
   eQMI_WDS_SET_EVENT,            // 001 Set connection state report conditions
   eQMI_WDS_EVENT_IND = 1,        // 001 Connection state report indication
   eQMI_WDS_ABORT,                // 002 Abort previously issued WDS command
   eQMI_WDS_SET_INDICATION,       // 003 Set indication conditions

   eQMI_WDS_GET_MESSAGES = 30,    // 030 Get supported messages
   eQMI_WDS_GET_FIELDS,           // 031 Get supported fields
   eQMI_WDS_START_NET,            // 032 Start WDS network interface
   eQMI_WDS_STOP_NET,             // 033 Stop WDS network interface
   eQMI_WDS_GET_PKT_STATUS,       // 034 Get packet data connection status
   eQMI_WDS_PKT_STATUS_IND = 34,  // 034 Packet data connection status indication
   eQMI_WDS_GET_RATES,            // 035 Get current bit rates of the connection
   eQMI_WDS_GET_STATISTICS,       // 036 Get the packet data transfer statistics
   eQMI_WDS_G0_DORMANT,           // 037 Go dormant
   eQMI_WDS_G0_ACTIVE,            // 038 Go active
   eQMI_WDS_CREATE_PROFILE,       // 039 Create profile with specified settings
   eQMI_WDS_MODIFY_PROFILE,       // 040 Modify profile with specified settings
   eQMI_WDS_DELETE_PROFILE,       // 041 Delete the specified profile 
   eQMI_WDS_GET_PROFILE_LIST,     // 042 Get all profiles
   eQMI_WDS_GET_PROFILE,          // 043 Get the specified profile
   eQMI_WDS_GET_DEFAULTS,         // 044 Get the default data session settings 
   eQMI_WDS_GET_SETTINGS,         // 045 Get the runtime data session settings 
   eQMI_WDS_SET_MIP,              // 046 Get the mobile IP setting 
   eQMI_WDS_GET_MIP,              // 047 Set the mobile IP setting 
   eQMI_WDS_GET_DORMANCY,         // 048 Get the dormancy status

   eQMI_WDS_GET_AUTOCONNECT = 52, // 052 Get the NDIS autoconnect setting
   eQMI_WDS_GET_DURATION,         // 053 Get the duration of data session
   eQMI_WDS_GET_MODEM_STATUS,     // 054 Get the modem status
   eQMI_WDS_MODEM_IND = 54,       // 054 Modem status indication
   eQMI_WDS_GET_DATA_BEARER,      // 055 Get the data bearer type
   eQMI_WDS_GET_MODEM_INFO,       // 056 Get the modem info
   eQMI_WDS_MODEM_INFO_IND = 56,  // 056 Modem info indication

   eQMI_WDS_GET_ACTIVE_MIP = 60,  // 060 Get the active mobile IP profile
   eQMI_WDS_SET_ACTIVE_MIP,       // 061 Set the active mobile IP profile
   eQMI_WDS_GET_MIP_PROFILE,      // 062 Get mobile IP profile settings
   eQMI_WDS_SET_MIP_PROFILE,      // 063 Set mobile IP profile settings
   eQMI_WDS_GET_MIP_PARAMS,       // 064 Get mobile IP parameters
   eQMI_WDS_SET_MIP_PARAMS,       // 065 Set mobile IP parameters
   eQMI_WDS_GET_LAST_MIP_STATUS,  // 066 Get last mobile IP status
   eQMI_WDS_GET_AAA_AUTH_STATUS,  // 067 Get AN-AAA authentication status
   eQMI_WDS_GET_CUR_DATA_BEARER,  // 068 Get current data bearer
   eQMI_WDS_GET_CALL_LIST,        // 069 Get the call history list
   eQMI_WDS_GET_CALL_ENTRY,       // 070 Get an entry from the call history list
   eQMI_WDS_CLEAR_CALL_LIST,      // 071 Clear the call history list
   eQMI_WDS_GET_CALL_LIST_MAX,    // 072 Get maximum size of call history list
   eQMI_WDS_GET_DEFAULT_PROF_NUM, // 073 Get default profile number
   eQMI_WDS_SET_DEFAULT_PROF_NUM, // 074 Set default profile number
   eQMI_WDS_RESET_PROFILE,        // 075 Reset profile
   eQMI_WDS_RESET_PROF_PARAM,     // 076 Reset profile param to invalid
   eQMI_WDS_SET_IP_FAMILY,        // 077 Set the client IP family preference
   eQMI_WDS_SET_FMC_TUNNEL,       // 078 Set FMC tunnel parameters
   eQMI_WDS_CLEAR_FMC_TUNNEL,     // 079 Clear FMC tunnel parameters
   eQMI_WDS_GET_FMC_TUNNEL,       // 080 Get FMC tunnel parameters
   eQMI_WDS_SET_AUTOCONNECT,      // 081 Set the NDIS autoconnect setting
   eQMI_WDS_GET_DNS,              // 082 Get the DNS setting
   eQMI_WDS_SET_DNS,              // 083 Set the DNS setting
   eQMI_WDS_GET_PRE_DORMANCY,     // 084 Get the CDMA pre-dormancy settings
   eQMI_WDS_SET_CAM_TIMER,        // 085 Set the CAM timer
   eQMI_WDS_GET_CAM_TIMER,        // 086 Get the CAM timer
   eQMI_WDS_SET_SCRM,             // 087 Set SCRM status 
   eQMI_WDS_GET_SCRM,             // 088 Get SCRM status
   eQMI_WDS_SET_RDUD,             // 089 Set RDUD status 
   eQMI_WDS_GET_RDUD,             // 090 Get RDUD status 
   eQMI_WDS_GET_SIPMIP_CALL_TYPE, // 091 Set SIP/MIP call type 
   eQMI_WDS_SET_PM_PERIOD,        // 092 Set EV-DO page monitor period
   eQMI_WDS_PM_PERIOD_IND = 92,   // 092 EV-DO page monitor period indication
   eQMI_WDS_SET_FORCE_LONG_SLEEP, // 093 Set EV-DO force long sleep feature
   eQMI_WDS_GET_PM_PERIOD,        // 094 Get EV-DO page monitor period
   eQMI_WDS_GET_CALL_THROTTLE,    // 095 Get call throttle info
   eQMI_WDS_GET_NSAPI,            // 096 Get NSAPI
   eQMI_WDS_SET_DUN_CTRL_PREF,    // 097 Set DUN control preference
   eQMI_WDS_GET_DUN_CTRL_INFO,    // 098 Set DUN control info
   eQMI_WDS_SET_DUN_CTRL_EVENT,   // 099 Set DUN control event preference
   eQMI_WDS_DUN_CTRL_IND = 99,    // 099 DUN control event report indication
   eQMI_WDS_PENDING_DUN_CTRL,     // 100 Control pending DUN call
   eQMI_WDS_TMGI_ACTIVATE,        // 101 Activate eMBMS TMGI  
   eQMI_WDS_TMGI_ACT_IND = 101,   // 101 eMBMS TMGI activate indication  
   eQMI_WDS_TMGI_DEACTIVATE,      // 102 Activate eMBMS TMGI  
   eQMI_WDS_TMGI_DEACT_IND = 102, // 102 eMBMS TMGI activate indication  
   eQMI_WDS_TMGI_LIST_QUERY,      // 103 Query for eMBMS TMGI list  
   eQMI_WDS_TMGI_LIST_IND,        // 104 eMBMS TMGI list query indication  
   eQMI_WDS_GET_PREF_DATA_SYS,    // 105 Get preferred data system
   eQMI_WDS_GET_LAST_DATA_STATUS, // 106 Get last data call status
   eQMI_WDS_GET_CURR_DATA_SYS,    // 107 Get current data systems status
   eQMI_WDS_GET_PDN_THROTTLE,     // 108 Get PDN throttle info

   eQMI_WDS_GET_LTE_ATTACH = 133, // 133 Get LTE attach parameters
   eQMI_WDS_RESET_PKT_STATS,      // 134 Reset packet statistics
   eQMI_WDS_GET_FLOW_CTRL_STATUS, // 135 Get flow control status
   eQMI_WDS_TMGI_SWITCH,          // 136 Activate/deactivate eMBMS TMGI  
   eQMI_WDS_TMGI_SWITCH_IND = 136,// 136 eMBMS TMGI activate/deactivate ind
   eQMI_WDS_BIND_DATA_PORT,       // 137 Bind data port
   eQMI_WDS_SET_PDN_FILTER,       // 138 Set additional PDN filter
   eQMI_WDS_REMOVE_PDN_FILTER,    // 139 Remove PDN filter
   eQMI_WDS_EX_IP_CFG_IND,        // 140 Extend IP config indication
   eQMI_WDS_CFG_REV_IP_CONN_EVT,  // 141 Configure reverse IP connection event
   eQMI_WDS_REV_IP_TRANSPORT_IND, // 142 Reverse IP transport connection ind
   eQMI_WDS_GET_IPSEC_SA_CFG,     // 143 Get IPSec static SA config
   eQMI_WDS_REV_IP_TPORT_CFG_DONE,// 144 Reverse IP transport config complete
   eQMI_WDS_GET_EX_DATA_BEARER,   // 145 Get extended data bearer
   eQMI_WDS_GET_LTE_MAX_ATTACH,   // 146 Get LTE maximum attach PDN number
   eQMI_WDS_SET_LTE_ATTACH_PDNS,  // 147 Set LTE attach PDN list
   eQMI_WDS_GET_LTE_ATTACH_PDNS,  // 148 Set LTE attach PDN list
   eQMI_WDS_LTE_ATTACH_PDNS_IND,  // 149 LTE attach PDN list indication
   eQMI_WDS_SET_LTE_DATA_RETRY,   // 150 Set LTE data retry
   eQMI_WDS_GET_LTE_DATA_RETRY,   // 151 Get LTE data retry
   eQMI_WDS_SET_LTE_ATTACH_TYPE,  // 152 Set LTE attach type
   eQMI_WDS_GET_LTE_ATTACH_TYPE,  // 153 Get LTE attach type
   eQMI_WDS_REV_IP_FILTER_IND,    // 154 Reverse IP transport filter ind
   eQMI_WDS_HANDOFF_INFO_IND,     // 155 Handoff info indication

   eQMI_WDS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageWDS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageWDS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_WDS_ENUM_BEGIN && msgID <= eQMI_WDS_SET_INDICATION)
   ||   (msgID >= eQMI_WDS_GET_MESSAGES && msgID <= eQMI_WDS_GET_DORMANCY)
   ||   (msgID >= eQMI_WDS_GET_AUTOCONNECT && msgID <= eQMI_WDS_MODEM_INFO_IND)
   ||   (msgID >= eQMI_WDS_GET_ACTIVE_MIP && msgID <= eQMI_WDS_GET_PDN_THROTTLE)
   ||   (msgID >= eQMI_WDS_GET_LTE_ATTACH && msgID < eQMI_WDS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageDMS Enumeration
//    QMI DMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageDMS
{
   eQMI_DMS_ENUM_BEGIN = -1, 

   eQMI_DMS_RESET,               // 00 Reset DMS service state variables
   eQMI_DMS_SET_EVENT,           // 01 Set connection state report conditions
   eQMI_DMS_EVENT_IND = 1,       // 01 Connection state report indication

   eQMI_DMS_GET_MESSAGES = 30,   // 30 Get supported messages
   eQMI_DMS_GET_FIELDS,          // 31 Get supported fields
   eQMI_DMS_GET_CAPS,            // 32 Get the device capabilities
   eQMI_DMS_GET_MANUFACTURER,    // 33 Get the device manfacturer
   eQMI_DMS_GET_MODEL_ID,        // 34 Get the device model ID
   eQMI_DMS_GET_REV_ID,          // 35 Get the device revision ID
   eQMI_DMS_GET_NUMBER,          // 36 Get the assigned voice number
   eQMI_DMS_GET_IDS,             // 37 Get the ESN/IMEI/MEID
   eQMI_DMS_GET_POWER_STATE,     // 38 Get the get power state
   eQMI_DMS_UIM_SET_PIN_PROT,    // 39 UIM - Set PIN protection
   eQMI_DMS_UIM_PIN_VERIFY,      // 40 UIM - Verify PIN 
   eQMI_DMS_UIM_PIN_UNBLOCK,     // 41 UIM - Unblock PIN
   eQMI_DMS_UIM_PIN_CHANGE,      // 42 UIM - Change PIN
   eQMI_DMS_UIM_GET_PIN_STATUS,  // 43 UIM - Get PIN status
   eQMI_DMS_GET_MSM_ID = 44,     // 44 Get MSM ID
   eQMI_DMS_GET_OPERATING_MODE,  // 45 Get the operating mode
   eQMI_DMS_SET_OPERATING_MODE,  // 46 Set the operating mode
   eQMI_DMS_GET_TIME,            // 47 Get timestamp from the device
   eQMI_DMS_GET_PRL_VERSION,     // 48 Get the PRL version
   eQMI_DMS_GET_ACTIVATED_STATE, // 49 Get the activation state 
   eQMI_DMS_ACTIVATE_AUTOMATIC,  // 50 Perform an automatic activation
   eQMI_DMS_ACTIVATE_MANUAL,     // 51 Perform a manual activation
   eQMI_DMS_GET_USER_LOCK_STATE, // 52 Get the lock state
   eQMI_DMS_SET_USER_LOCK_STATE, // 53 Set the lock state
   eQMI_DMS_SET_USER_LOCK_CODE,  // 54 Set the lock PIN
   eQMI_DMS_READ_USER_DATA,      // 55 Read user data
   eQMI_DMS_WRITE_USER_DATA,     // 56 Write user data
   eQMI_DMS_READ_ERI_FILE,       // 57 Read the enhanced roaming indicator file
   eQMI_DMS_FACTORY_DEFAULTS,    // 58 Reset to factory defaults
   eQMI_DMS_VALIDATE_SPC,        // 59 Validate service programming code
   eQMI_DMS_UIM_GET_ICCID,       // 60 Get UIM ICCID
   eQMI_DMS_GET_FIRWARE_ID,      // 61 Get firmware ID
   eQMI_DMS_SET_FIRMWARE_ID,     // 62 Set firmware ID
   eQMI_DMS_GET_HOST_LOCK_ID,    // 63 Get host lock ID
   eQMI_DMS_UIM_GET_CK_STATUS,   // 64 UIM - Get control key status
   eQMI_DMS_UIM_SET_CK_PROT,     // 65 UIM - Set control key protection
   eQMI_DMS_UIM_UNBLOCK_CK,      // 66 UIM - Unblock facility control key
   eQMI_DMS_GET_IMSI,            // 67 Get the IMSI
   eQMI_DMS_UIM_GET_STATE,       // 68 UIM - Get the UIM state
   eQMI_DMS_GET_BAND_CAPS,       // 69 Get the device band capabilities
   eQMI_DMS_GET_FACTORY_ID,      // 70 Get the device factory ID
   eQMI_DMS_GET_FIRMWARE_PREF,   // 71 Get firmware preference 
   eQMI_DMS_SET_FIRMWARE_PREF,   // 72 Set firmware preference 
   eQMI_DMS_LIST_FIRMWARE,       // 73 List all stored firmware
   eQMI_DMS_DELETE_FIRMWARE,     // 74 Delete specified stored firmware
   eQMI_DMS_SET_TIME,            // 75 Set device time
   eQMI_DMS_GET_FIRMWARE_INFO,   // 76 Get stored firmware info
   eQMI_DMS_GET_ALT_NET_CFG,     // 77 Get alternate network config
   eQMI_DMS_SET_ALT_NET_CFG,     // 78 Set alternate network config
   eQMI_DMS_GET_IMG_DLOAD_MODE,  // 79 Get next image download mode
   eQMI_DMS_SET_IMG_DLOAD_MODE,  // 80 Set next image download mode
   eQMI_DMS_GET_SW_VERSION,      // 81 Get software version
   eQMI_DMS_SET_SPC,             // 82 Set SPC
   eQMI_DMS_GET_CURRENT_PRL_INFO,// 83 Get current PRL info
   eQMI_DMS_BIND_SUBSCRIPTION,   // 84 Bind subscription
   eQMI_DMS_GET_SUBSCRIPTION,    // 85 Get bound subscription

   eQMI_DMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageDMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageDMS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_DMS_ENUM_BEGIN && msgID <= eQMI_DMS_EVENT_IND)
   ||   (msgID >= eQMI_DMS_GET_MESSAGES && msgID < eQMI_DMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageNAS Enumeration
//    QMI NAS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageNAS
{
   eQMI_NAS_ENUM_BEGIN = -1, 

   eQMI_NAS_RESET,               // 000 Reset NAS service state variables
   eQMI_NAS_ABORT,               // 001 Abort previously issued NAS command
   eQMI_NAS_SET_EVENT,           // 002 Set NAS state report conditions
   eQMI_NAS_EVENT_IND = 2,       // 002 Connection state report indication
   eQMI_NAS_SET_REG_EVENT,       // 003 Set NAS registration report conditions

   eQMI_NAS_GET_MESSAGES = 30,   // 030 Get supported messages
   eQMI_NAS_GET_FIELDS,          // 031 Get supported fields
   eQMI_NAS_GET_RSSI,            // 032 Get the signal strength
   eQMI_NAS_SCAN_NETS,           // 033 Scan for visible network
   eQMI_NAS_REGISTER_NET,        // 034 Initiate a network registration
   eQMI_NAS_ATTACH_DETACH,       // 035 Initiate an attach or detach action
   eQMI_NAS_GET_SS_INFO,         // 036 Get info about current serving system
   eQMI_NAS_SS_INFO_IND = 36,    // 036 Current serving system info indication
   eQMI_NAS_GET_HOME_INFO,       // 037 Get info about home network
   eQMI_NAS_GET_NET_PREF_LIST,   // 038 Get the list of preferred networks
   eQMI_NAS_SET_NET_PREF_LIST,   // 039 Set the list of preferred networks
   eQMI_NAS_GET_NET_BAN_LIST,    // 040 Get the list of forbidden networks
   eQMI_NAS_SET_NET_BAN_LIST,    // 041 Set the list of forbidden networks
   eQMI_NAS_SET_TECH_PREF,       // 042 Set the technology preference
   eQMI_NAS_GET_TECH_PREF,       // 043 Get the technology preference
   eQMI_NAS_GET_ACCOLC,          // 044 Get the Access Overload Class
   eQMI_NAS_SET_ACCOLC,          // 045 Set the Access Overload Class 
   eQMI_NAS_GET_SYSPREF,         // 046 Get the CDMA system preference 
   eQMI_NAS_GET_NET_PARAMS,      // 047 Get various network parameters 
   eQMI_NAS_SET_NET_PARAMS,      // 048 Set various network parameters 
   eQMI_NAS_GET_RF_INFO,         // 049 Get the SS radio/band channel info
   eQMI_NAS_GET_AAA_AUTH_STATUS, // 050 Get AN-AAA authentication status
   eQMI_NAS_SET_SYS_SELECT_PREF, // 051 Set system selection preference
   eQMI_NAS_GET_SYS_SELECT_PREF, // 052 Get system selection preference
   eQMI_NAS_SYS_SELECT_IND = 52, // 052 System selection pref indication

   eQMI_NAS_SET_DDTM_PREF = 55,  // 055 Set DDTM preference
   eQMI_NAS_GET_DDTM_PREF,       // 056 Get DDTM preference
   eQMI_NAS_DDTM_IND = 56,       // 056 DDTM preference indication
   eQMI_NAS_GET_OPERATER_NAME,   // 057 Get operator name data
   eQMI_NAS_OPERATER_NAME_IND,   // 058 Operator name data indication
   eQMI_NAS_GET_PLMN_MODE,       // 059 Get PLMN mode bit from CSP
   eQMI_NAS_PLMN_MODE_IND,       // 060 CSP PLMN mode bit indication
   eQMI_NAS_UPDATE_AKEY,         // 061 Update the A-KEY
   eQMI_NAS_GET_3GPP2_SUBS_INFO, // 062 Get 3GPP2 subscription info
   eQMI_NAS_SET_3GPP2_SUBS_INFO, // 063 Set 3GPP2 subscription info
   eQMI_NAS_MOB_CAI_REV,         // 064 Get mobile CAI revision information
   eQMI_NAS_GET_RTRE_CONFIG,     // 065 Get RTRE configuration information
   eQMI_NAS_SET_RTRE_CONFIG,     // 066 Set RTRE configuration information
   eQMI_NAS_GET_CELL_LOC_INFO,   // 067 Get cell location information
   eQMI_NAS_GET_PLMN_NAME,       // 068 Get operator name for specified network
   eQMI_NAS_BIND_SUBS,           // 069 Bind client to a specific subscription
   eQMI_NAS_MANAGED_ROAMING_IND, // 070 Managed roaming indication
   eQMI_NAS_DSB_PREF_IND,        // 071 Dual standby preference indication
   eQMI_NAS_SUBS_INFO_IND,       // 072 Subscription info indication
   eQMI_NAS_GET_MODE_PREF,       // 073 Get mode preference

   eQMI_NAS_SET_DSB_PREF = 75,   // 075 Set dual standby preference
   eQMI_NAS_NETWORK_TIME_IND,    // 076 Network time indication
   eQMI_NAS_GET_SYSTEM_INFO,     // 077 Get system info
   eQMI_NAS_SYSTEM_INFO_IND,     // 078 System info indication
   eQMI_NAS_GET_SIGNAL_INFO,     // 079 Get signal info
   eQMI_NAS_CFG_SIGNAL_INFO,     // 080 Configure signal info report
   eQMI_NAS_SIGNAL_INFO_IND,     // 081 Signal info indication
   eQMI_NAS_GET_ERROR_RATE,      // 082 Get error rate info
   eQMI_NAS_ERROR_RATE_IND,      // 083 Error rate indication
   eQMI_NAS_EVDO_SESSION_IND,    // 084 CDMA 1xEV-DO session close indication
   eQMI_NAS_EVDO_UATI_IND,       // 085 CDMA 1xEV-DO UATI update indication
   eQMI_NAS_GET_EVDO_SUBTYPE,    // 086 Get CDMA 1xEV-DO protocol subtype
   eQMI_NAS_GET_EVDO_COLOR_CODE, // 087 Get CDMA 1xEV-DO color code
   eQMI_NAS_GET_ACQ_SYS_MODE,    // 088 Get current acquisition system mode
   eQMI_NAS_SET_RX_DIVERSITY,    // 089 Set the RX diversity
   eQMI_NAS_GET_RX_TX_INFO,      // 090 Get detailed RX/TX information
   eQMI_NAS_UPDATE_AKEY_EXT,     // 091 Update the A-KEY (extended)
   eQMI_NAS_GET_DSB_PREF,        // 092 Get dual standby preference
   eQMI_NAS_DETACH_LTE,          // 093 Detach the current LTE system
   eQMI_NAS_BLOCK_LTE_PLMN,      // 094 Block LTE PLMN
   eQMI_NAS_UNBLOCK_LTE_PLMN,    // 095 Unblock LTE PLMN
   eQMI_NAS_RESET_LTE_PLMN_BLK,  // 096 Reset LTE PLMN blocking
   eQMI_NAS_CUR_PLMN_NAME_IND,   // 097 Current PLMN name indication
   eQMI_NAS_CONFIG_EMBMS,        // 098 Configure eMBMS
   eQMI_NAS_GET_EMBMS_STATUS,    // 099 Get eMBMS status
   eQMI_NAS_EMBMS_STATUS_IND,    // 100 eMBMS status indication
   eQMI_NAS_GET_CDMA_POS_INFO,   // 101 Get CDMA position info
   eQMI_NAS_RF_BAND_INFO_IND,    // 102 RF band info indication
   eQMI_NAS_FORCE_NET_SEARCH,    // 103 Force network search
   eQMI_NAS_NET_REJECT_IND,      // 104 Network reject indication
   eQMI_NAS_GET_MANAGED_ROAM,    // 105 Get managed roaming configuration
   eQMI_NAS_RTRE_CONFIG_IND,     // 106 RTRE configuration indication
   eQMI_NAS_GET_CENTRALIZED_EOM, // 107 Get centralized EONS support
   eQMI_NAS_CFG_SIGNAL_INFO2,    // 108 Configure signal info report (V2)
   eQMI_NAS_GET_TDS_CELL_INFO,   // 109 Get TD-SCDMA cell/position info
   eQMI_NAS_SET_HPLMN_IRAT_TIMER,// 110 Set HPLMN IRAT search timer
   eQMI_NAS_GET_EMBMS_SIQ_QUAL,  // 111 Get eMBMS signal quality
   eQMI_NAS_LIMIT_SYS_INFO,      // 112 Limit system info indications
   eQMI_NAS_GET_SYS_INFO_LIMITS, // 113 Get system info indication limits
   eQMI_NAS_UPDATE_IMS_STATUS,   // 114 Update IMS status
   eQMI_NAS_GET_IMS_PREFERENCE,  // 115 Get IMS preference status
   eQMI_NAS_IMS_PREFERENCE_IND,  // 116 IMS preference status indication
   eQMI_NAS_CFG_IND_PLMN_NAME,   // 117 Configure indication for PLMN name 
   eQMI_NAS_CDMA_AVOID_SYSTEM,   // 118 CDMA avoid system
   eQMI_NAS_GET_AVOIDED_SYSTEMS, // 119 Get CDMA avoided system list
   eQMI_NAS_SET_HPLMN_SRCH_TIMER,// 120 Set HPLMN search timer
   eQMI_NAS_GET_HPLMN_SRCH_TIMER,// 121 Get HPLMN search timer
   eQMI_NAS_SET_E911_STATE,      // 122 Set E911 state
   eQMI_NAS_E911_STATE_IND,      // 123 E911 state ready indication
   eQMI_NAS_GET_SUBS_INFO,       // 124 Get subscription info
   eQMI_NAS_GET_NET_TIME,        // 125 Get network time
   eQMI_NAS_GET_LTE_SIB16_TIME,  // 126 Get LTE SIB16 network time
   eQMI_NAS_LTE_SIB16_TIME_IND,  // 127 LTE SIB16 network time indication

   eQMI_NAS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageNAS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageNAS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_NAS_ENUM_BEGIN && msgID <= eQMI_NAS_SET_REG_EVENT)
   ||   (msgID >= eQMI_NAS_GET_MESSAGES && msgID <= eQMI_NAS_SYS_SELECT_IND)
   ||   (msgID >= eQMI_NAS_SET_DDTM_PREF && msgID <= eQMI_NAS_GET_MODE_PREF)
   ||   (msgID >= eQMI_NAS_SET_DSB_PREF && msgID < eQMI_NAS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageWMS Enumeration
//    QMI WMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageWMS
{
   eQMI_WMS_ENUM_BEGIN = -1, 

   eQMI_WMS_RESET,                  // 00 Reset WMS service state variables
   eQMI_WMS_SET_EVENT,              // 01 Set new message report conditions
   eQMI_WMS_EVENT_IND = 1,          // 01 New message report indication

   eQMI_WMS_GET_MESSAGES = 30,      // 030 Get supported messages
   eQMI_WMS_GET_FIELDS,             // 031 Get supported fields
   eQMI_WMS_RAW_SEND,               // 32 Send a raw message
   eQMI_WMS_RAW_WRITE,              // 33 Write a raw message to the device
   eQMI_WMS_RAW_READ,               // 34 Read a raw message from the device
   eQMI_WMS_MODIFY_TAG,             // 35 Modify message tag on the device
   eQMI_WMS_DELETE,                 // 36 Delete message by index/tag/memory

   eQMI_WMS_GET_MSG_PROTOCOL = 48,  // 48 Get the current message protocol
   eQMI_WMS_GET_MSG_LIST,           // 49 Get list of messages from the device
   eQMI_WMS_SET_ROUTES,             // 50 Set routes for message memory storage
   eQMI_WMS_GET_ROUTES,             // 51 Get routes for message memory storage
   eQMI_WMS_GET_SMSC_ADDR,          // 52 Get SMSC address
   eQMI_WMS_SET_SMSC_ADDR,          // 53 Set SMSC address
   eQMI_WMS_GET_MSG_LIST_MAX,       // 54 Get maximum size of SMS storage
   eQMI_WMS_SEND_ACK,               // 55 Send ACK
   eQMI_WMS_SET_RETRY_PERIOD,       // 56 Set retry period
   eQMI_WMS_SET_RETRY_INTERVAL,     // 57 Set retry interval
   eQMI_WMS_SET_DC_DISCO_TIMER,     // 58 Set DC auto-disconnect timer
   eQMI_WMS_SET_MEMORY_STATUS,      // 59 Set memory storage status
   eQMI_WMS_SET_BC_ACTIVATION,      // 60 Set broadcast activation
   eQMI_WMS_SET_BC_CONFIG,          // 61 Set broadcast config
   eQMI_WMS_GET_BC_CONFIG,          // 62 Get broadcast config
   eQMI_WMS_MEMORY_FULL_IND,        // 63 Memory full indication
   eQMI_WMS_GET_DOMAIN_PREF,        // 64 Get domain preference
   eQMI_WMS_SET_DOMAIN_PREF,        // 65 Set domain preference
   eQMI_WMS_MEMORY_SEND,            // 66 Send message from memory store
   eQMI_WMS_GET_MSG_WAITING,        // 67 Get message waiting info
   eQMI_WMS_MSG_WAITING_IND,        // 68 Message waiting indication
   eQMI_WMS_SET_PRIMARY_CLIENT,     // 69 Set client as primary client
   eQMI_WMS_SMSC_ADDR_IND,          // 70 SMSC address indication
   eQMI_WMS_INDICATOR_REG,          // 71 Register for indicators
   eQMI_WMS_GET_TRANSPORT_INFO,     // 72 Get transport layer info
   eQMI_WMS_TRANSPORT_INFO_IND,     // 73 Transport layer info indication
   eQMI_WMS_GET_NW_REG_INFO,        // 74 Get network registration info
   eQMI_WMS_NW_REG_INFO_IND,        // 75 Network registration info indication
   eQMI_WMS_BIND_SUBSCRIPTION,      // 76 Bind client to a subscription
   eQMI_WMS_GET_INDICATOR_REG,      // 77 Get indicator registration
   eQMI_WMS_GET_SMS_PARAMETERS,     // 78 Get SMS EF-SMSP parameters
   eQMI_WMS_SET_SMS_PARAMETERS,     // 79 Set SMS EF-SMSP parameters
   eQMI_WMS_CALL_STATUS_IND,        // 80 Call status indication
   eQMI_WMS_GET_DOMAIN_PREF_CFG,    // 81 Get domain pref config
   eQMI_WMS_SET_DOMAIN_PREF_CFG,    // 82 Set domain pref config
   eQMI_WMS_GET_RETRY_PERIOD,       // 83 Get retry period
   eQMI_WMS_GET_RETRY_INTERVAL,     // 84 Get retry interval
   eQMI_WMS_GET_DC_DISCO_TIMER,     // 85 Get DC auto-disconnect timer
   eQMI_WMS_GET_MEMORY_STATUS,      // 86 Get memory storage status
   eQMI_WMS_GET_PRIMARY_CLIENT,     // 87 Get primary cleint
   eQMI_WMS_GET_SUBSCR_BINDING,     // 88 Get client subscription binding
   eQMI_WMS_ASYNC_RAW_SEND,         // 89 Asynchronously send a raw message
   eQMI_WMS_ASYNC_RAW_SEND_IND = 89,// 89 Asynchronous send indication
   eQMI_WMS_ASYNC_SEND_ACK,         // 90 Asynchronously send ACK
   eQMI_WMS_ASYNC_SEND_ACK_IND = 90,// 90 Asynchronou send ACK indication
   eQMI_WMS_ASYNC_MEMORY_SEND,      // 91 Async send msg from memory store
   eQMI_WMS_ASYNC_MEM_SEND_IND = 91,// 91 Async memory store send indication
   eQMI_WMS_GET_SERVICE_READY,      // 92 Get service ready status
   eQMI_WMS_SERVICE_READY_IND,      // 93 Service ready status indication
   eQMI_WMS_BC_CONFIG_IND,          // 94 Broadcast config indication
   eQMI_WMS_SET_MSG_WAITING,        // 95 Set message waiting info

   eQMI_WMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageWMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageWMS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_WMS_ENUM_BEGIN && msgID <= eQMI_WMS_EVENT_IND)
   ||   (msgID >= eQMI_WMS_GET_MESSAGES && msgID <= eQMI_WMS_DELETE)
   ||   (msgID >= eQMI_WMS_GET_MSG_PROTOCOL && msgID < eQMI_WMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessagePDS Enumeration
//    QMI PDS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessagePDS
{
   eQMI_PDS_ENUM_BEGIN = -1, 

   eQMI_PDS_RESET,                // 000 Reset PDS service state variables
   eQMI_PDS_SET_EVENT,            // 001 Set PDS report conditions
   eQMI_PDS_EVENT_IND = 1,        // 001 PDS report indication

   eQMI_PDS_GET_STATE = 32,       // 032 Return PDS service state
   eQMI_PDS_STATE_IND = 32,       // 032 PDS service state indication
   eQMI_PDS_SET_STATE,            // 033 Set PDS service state
   eQMI_PDS_START_SESSION,        // 034 Start a PDS tracking session
   eQMI_PDS_GET_SESSION_INFO,     // 035 Get PDS tracking session info
   eQMI_PDS_FIX_POSITION,         // 036 Manual tracking session position
   eQMI_PDS_END_SESSION,          // 037 End a PDS tracking session
   eQMI_PDS_GET_NMEA_CFG,         // 038 Get NMEA sentence config
   eQMI_PDS_SET_NMEA_CFG,         // 039 Set NMEA sentence config
   eQMI_PDS_INJECT_TIME,          // 040 Inject a time reference
   eQMI_PDS_GET_DEFAULTS,         // 041 Get default tracking session config
   eQMI_PDS_SET_DEFAULTS,         // 042 Set default tracking session config
   eQMI_PDS_GET_XTRA_PARAMS,      // 043 Get the GPS XTRA parameters 
   eQMI_PDS_SET_XTRA_PARAMS,      // 044 Set the GPS XTRA parameters 
   eQMI_PDS_FORCE_XTRA_DL,        // 045 Force a GPS XTRA database download
   eQMI_PDS_GET_AGPS_CONFIG,      // 046 Get the AGPS mode configuration
   eQMI_PDS_SET_AGPS_CONFIG,      // 047 Set the AGPS mode configuration
   eQMI_PDS_GET_SVC_AUTOTRACK,    // 048 Get the service auto-tracking state
   eQMI_PDS_SET_SVC_AUTOTRACK,    // 049 Set the service auto-tracking state
   eQMI_PDS_GET_COM_AUTOTRACK,    // 050 Get COM port auto-tracking config
   eQMI_PDS_SET_COM_AUTOTRACK,    // 051 Set COM port auto-tracking config
   eQMI_PDS_RESET_DATA,           // 052 Reset PDS service data
   eQMI_PDS_SINGLE_FIX,           // 053 Request single position fix
   eQMI_PDS_GET_VERSION,          // 054 Get PDS service version
   eQMI_PDS_INJECT_XTRA,          // 055 Inject XTRA data
   eQMI_PDS_INJECT_POSITION,      // 056 Inject position data
   eQMI_PDS_INJECT_WIFI,          // 057 Inject Wi-Fi obtained data
   eQMI_PDS_GET_SBAS_CONFIG,      // 058 Get SBAS config
   eQMI_PDS_SET_SBAS_CONFIG,      // 059 Set SBAS config
   eQMI_PDS_SEND_NI_RESPONSE,     // 060 Send network initiated response
   eQMI_PDS_INJECT_ABS_TIME,      // 061 Inject absolute time
   eQMI_PDS_INJECT_EFS,           // 062 Inject EFS data
   eQMI_PDS_GET_DPO_CONFIG,       // 063 Get DPO config
   eQMI_PDS_SET_DPO_CONFIG,       // 064 Set DPO config
   eQMI_PDS_GET_ODP_CONFIG,       // 065 Get ODP config
   eQMI_PDS_SET_ODP_CONFIG,       // 066 Set ODP config
   eQMI_PDS_CANCEL_SINGLE_FIX,    // 067 Cancel single position fix
   eQMI_PDS_GET_GPS_STATE,        // 068 Get GPS state
   eQMI_PDS_SET_PPM_EVT_REPORT,   // 069 Set PPM event report  
   eQMI_PDS_SET_SPI_REPORT,       // 070 Set SPI streaming reporting
   eQMI_PDS_SET_SPI_RPT_IND = 70, // 070 Set SPI streaming indication
   eQMI_PDS_SET_SPI_STATUS,       // 071 Set SPI status
   eQMI_PDS_SET_PPM_REPORT,       // 072 Set PPM reporting state
   eQMI_PDS_SET_PPM_RPT_IND = 72, // 072 Set PPM reporting state indication
   eQMI_PDS_FORCE_RECEIVER_OFF,   // 073 Force receiver off

   eQMI_PDS_GET_METHODS = 80,     // 080 Get GPS position methods state
   eQMI_PDS_SET_METHODS,          // 081 Set GPS position methods state
   eQMI_PDS_INJECT_SENSOR,        // 082 Inject sensor data
   eQMI_PDS_INJECT_TIME_SYNC,     // 083 Inject time sync data
   eQMI_PDS_GET_SENSOR_CFG,       // 084 Get sensor config
   eQMI_PDS_SET_SENSOR_CFG,       // 085 Set sensor config
   eQMI_PDS_GET_NAV_CFG,          // 086 Get navigation config
   eQMI_PDS_SET_NAV_CFG,          // 087 Set navigation config

   eQMI_PDS_SET_WLAN_BLANK = 90,  // 090 Set WLAN blanking
   eQMI_PDS_SET_LBS_SC_RPT,       // 091 Set LBS security challenge reporting
   eQMI_PDS_LBS_SC_IND = 91,      // 091 LBS security challenge indication
   eQMI_PDS_SET_LBS_SC,           // 092 Set LBS security challenge
   eQMI_PDS_GET_LBS_ENCRYPT_CFG,  // 093 Get LBS security encryption config
   eQMI_PDS_SET_LBS_UPDATE_RATE,  // 094 Set LBS security update rate
   eQMI_PDS_SET_CELLDB_CONTROL,   // 095 Set cell database control
   eQMI_PDS_READY_IND,            // 096 Ready indication
   eQMI_PDS_INJECT_MOTION_DATA,   // 097 Inject motion data 
   eQMI_PDS_SET_GNSS_ERR_REPORT,  // 098 Set GNSS error recovery report 
   eQMI_PDS_GNSS_ERR_IND = 98,    // 098 GNSS error recovery report indication
   eQMI_PDS_RESET_SERVICE,        // 099 Reset location service
   eQMI_PDS_INJECT_TEST_DATA,     // 100 Inject test data
   eQMI_PDS_SET_GNSS_RF_CFG,      // 101 Set GNSS RF config
   eQMI_PDS_INJECT_COG_DATA,      // 102 Inject course over ground data
   eQMI_PDS_INJECT_SUPL_CERT,     // 103 Inject SUPL certificate
   eQMI_PDS_DELETE_SUPL_CERT ,    // 104 Delete SUPL certificate
   eQMI_PDS_GET_IS801_OVER_SUPL,  // 105 Get IS-801 Over SUPL indicator
   eQMI_PDS_SET_IS801_OVER_SUPL,  // 106 Set IS-801 Over SUPL indicator
   eQMI_PDS_GET_SUPL_HASH_ALG,    // 107 Get SUPL hash algorithm
   eQMI_PDS_SET_SUPL_HASH_ALG,    // 108 Set SUPL hash algorithm
   eQMI_PDS_GET_SUPL_MAX_VERSION, // 109 Get SUPL maximum version
   eQMI_PDS_SET_SUPL_MAX_VERSION, // 110 Set SUPL maximum version
   eQMI_PDS_GET_SUPL_SECURITY,    // 111 Get SUPL security
   eQMI_PDS_SET_SUPL_SECURITY,    // 112 Set SUPL security
   eQMI_PDS_GET_SUPL_TLS_VERSION, // 113 Get SUPL TLS version
   eQMI_PDS_SET_SUPL_TLS_VERSION, // 114 Set SUPL TLS version
   eQMI_PDS_GET_AGNSS_POS_MODES,  // 115 Get AGNSS positioning modes
   eQMI_PDS_SET_AGNSS_POS_MODES,  // 116 Set AGNSS positioning modes
   eQMI_PDS_GET_EMERGENCY_PROTO,  // 117 Get emergency protocol config
   eQMI_PDS_SET_EMERGENCY_PROTO,  // 118 Set emergency protocol config
   eQMI_PDS_GET_APN_PROFILES,     // 119 Get APN profiles
   eQMI_PDS_SET_APN_PROFILES,     // 120 Set APN profiles
   eQMI_PDS_GET_HOME_SUPL_ADDR,   // 121 Get home SUPL address
   eQMI_PDS_SET_HOME_SUPL_ADDR,   // 122 Set home SUPL address
   eQMI_PDS_INJECT_VEHICLE_DATA,  // 123 Inject vehicle sensor data
   eQMI_PDS_INJECT_PEDO_DATA,     // 124 Inject pedometer data

   eQMI_PDS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessagePDS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessagePDS msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_PDS_ENUM_BEGIN && msgID <= eQMI_PDS_EVENT_IND)
   ||   (msgID >= eQMI_PDS_GET_STATE && msgID <= eQMI_PDS_FORCE_RECEIVER_OFF)
   ||   (msgID >= eQMI_PDS_GET_METHODS && msgID <= eQMI_PDS_SET_NAV_CFG)
   ||   (msgID >= eQMI_PDS_SET_WLAN_BLANK && msgID < eQMI_PDS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageAUTH Enumeration
//    QMI Authentication Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageAUTH
{
   eQMI_AUTH_ENUM_BEGIN = -1, 

   eQMI_AUTH_START_EAP = 32,        // 32 Start the EAP session
   eQMI_AUTH_SEND_EAP,              // 33 Send and receive EAP packets
   eQMI_AUTH_EAP_RESULT_IND,        // 34 EAP session result indication
   eQMI_AUTH_GET_EAP_KEYS,          // 35 Get the EAP session keys
   eQMI_AUTH_END_EAP,               // 36 End the EAP session
   eQMI_AUTH_RUN_AKA,               // 37 Runs the AKA algorithm
   eQMI_AUTH_AKA_RESULT_IND,        // 38 AKA algorithm result indication

   eQMI_AUTH_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageAUTH validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageAUTH msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_AUTH_START_EAP && msgID < eQMI_AUTH_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageAT Enumeration
//    QMI AT Command Processor Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageAT
{
   eQMI_AT_ENUM_BEGIN = -1, 

   eQMI_AT_RESET,                   // 00 Reset AT service state variables

   eQMI_AT_REG_COMMANDS = 32,       // 32 Register AT commands to be forwarded
   eQMI_AT_COMMAND_IND,             // 33 AT command indication
   eQMI_AT_SEND_RESPONSE,           // 34 Send the response to an AT command
   eQMI_AT_ABORT_IND,               // 35 AT command abort indication

   eQMI_AT_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageAT validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageAT msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_AT_RESET)
   ||   (msgID >= eQMI_AT_REG_COMMANDS && msgID < eQMI_AT_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};


/*=========================================================================*/
// eQMIMessageVoice Enumeration
//    QMI Voice Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageVoice
{
   eQMI_VOICE_ENUM_BEGIN = -1, 

   eQMI_VOICE_INDICATION_REG = 3,    // 03 Set indication registration state

   eQMI_VOICE_GET_MESSAGES = 30,     // 30 Get supported messages
   eQMI_VOICE_GET_FIELDS,            // 31 Get supported fields
   eQMI_VOICE_CALL_ORIGINATE,        // 32 Originate a voice call
   eQMI_VOICE_CALL_END,              // 33 End a voice call
   eQMI_VOICE_CALL_ANSWER,           // 34 Answer incoming voice call

   eQMI_VOICE_GET_CALL_INFO = 36,    // 36 Get call information
   eQMI_VOICE_OTASP_STATUS_IND,      // 37 OTASP/OTAPA event indication
   eQMI_VOICE_INFO_REC_IND,          // 38 New info record indication
   eQMI_VOICE_SEND_FLASH,            // 39 Send a simple flash
   eQMI_VOICE_BURST_DTMF,            // 40 Send a burst DTMF
   eQMI_VOICE_START_CONT_DTMF,       // 41 Starts a continuous DTMF
   eQMI_VOICE_STOP_CONT_DTMF,        // 42 Stops a continuous DTMF
   eQMI_VOICE_DTMF_IND,              // 43 DTMF event indication
   eQMI_VOICE_SET_PRIVACY_PREF,      // 44 Set privacy preference
   eQMI_VOICE_PRIVACY_IND,           // 45 Privacy change indication
   eQMI_VOICE_ALL_STATUS_IND,        // 46 Voice all call status indication
   eQMI_VOICE_GET_ALL_STATUS,        // 47 Get voice all call status

   eQMI_VOICE_MANAGE_CALLS = 49,     // 49 Manage calls
   eQMI_VOICE_SUPS_NOTIFICATION_IND, // 50 Supplementary service notifications
   eQMI_VOICE_SET_SUPS_SERVICE,      // 51 Manage supplementary service
   eQMI_VOICE_GET_CALL_WAITING,      // 52 Query sup service call waiting
   eQMI_VOICE_GET_CALL_BARRING,      // 53 Query sup service call barring
   eQMI_VOICE_GET_CLIP,              // 54 Query sup service CLIP
   eQMI_VOICE_GET_CLIR,              // 55 Query sup service CLIR
   eQMI_VOICE_GET_CALL_FWDING,       // 56 Query sup service call forwarding
   eQMI_VOICE_SET_CALL_BARRING_PWD,  // 57 Set call barring password
   eQMI_VOICE_ORIG_USSD,             // 58 Initiate USSD operation then wait
   eQMI_VOICE_ANSWER_USSD,           // 59 Answer USSD request
   eQMI_VOICE_CANCEL_USSD,           // 60 Cancel USSD operation
   eQMI_VOICE_USSD_RELEASE_IND,      // 61 USSD release indication
   eQMI_VOICE_USSD_IND,              // 62 USSD request/notification indication
   eQMI_VOICE_UUS_IND,               // 63 UUS information indication
   eQMI_VOICE_SET_CONFIG,            // 64 Set config
   eQMI_VOICE_GET_CONFIG,            // 65 Get config
   eQMI_VOICE_SUPS_IND,              // 66 Sup service request indication
   eQMI_VOICE_ASYNC_ORIG_USSD,       // 67 Initiate USSD operation
   eQMI_VOICE_ASYNC_USSD_IND = 67,   // 67 USSD request/notification indication
   eQMI_VOICE_BIND_SUBSCRIPTION,     // 68 Bind subscription
   eQMI_VOICE_ALS_SET_LINE_SW,       // 69 ALS set line switching
   eQMI_VOICE_ALS_SELECT_LINE,       // 70 ALS select line
   eQMI_VOICE_AOC_RESET_ACM,         // 71 AOC reset ACM
   eQMI_VOICE_AOC_SET_ACM_MAX,       // 72 ACM set ACM maximum
   eQMI_VOICE_AOC_GET_CM_INFO,       // 73 AOC get call meter info
   eQMI_VOICE_AOC_LOW_FUNDS_IND,     // 74 AOC low funds indication
   eQMI_VOICE_GET_COLP,              // 75 Get COLP info
   eQMI_VOICE_GET_COLR,              // 76 Get COLR info
   eQMI_VOICE_GET_CNAP,              // 77 Get CNAP info
   eQMI_VOICE_MANAGE_IP_CALLS,       // 78 Manage VoIP calls
   eQMI_VOICE_ALS_GET_LINE_SW,       // 79 ALS get line switching
   eQMI_VOICE_ALS_GET_LINE_SEL,      // 80 ALS get selected line
   eQMI_VOICE_MODIFIED_IND,          // 81 Call modified indication
   eQMI_VOICE_MODIFY_ACCEPT_IND,     // 82 Call modify accept indication
   eQMI_VOICE_SPEECH_CODEC_IND,      // 83 Speech codec info indication
   eQMI_VOICE_HANDOVER_IND,          // 84 Handover indication
   eQMI_VOICE_CONFERENCE_INFO_IND,   // 85 Conference info indication
   eQMI_VOICE_CONFERENCE_JOIN_IND,   // 86 Conference join indication
   eQMI_VOICE_CONFERENCE_UPDATE_IND, // 87 Conference update indication
   eQMI_VOICE_EXT_BURST_TYPE_IND,    // 88 Extended burst type indication
   eQMI_VOICE_MT_PAGE_MISS_IND,      // 89 MT page miss indication
   eQMI_VOICE_CC_RESULT_INFO_IND,    // 90 Call control result info indication

   eQMI_VOICE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageVoice validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageVoice msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_VOICE_INDICATION_REG)
   ||   (msgID >= eQMI_VOICE_GET_MESSAGES && msgID <= eQMI_VOICE_CALL_ANSWER)
   ||   (msgID >= eQMI_VOICE_GET_CALL_INFO && msgID <= eQMI_VOICE_GET_ALL_STATUS)
   ||   (msgID >= eQMI_VOICE_MANAGE_CALLS && msgID < eQMI_VOICE_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageUIM Enumeration
//    QMI UIM Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageUIM
{
   eQMI_UIM_ENUM_BEGIN = -1, 

   eQMI_UIM_RESET,                     // 00 Reset

   eQMI_UIM_GET_MESSAGES = 30,            // 30 Get supported messages
   eQMI_UIM_GET_FIELDS,                   // 31 Get supported fields
   eQMI_UIM_READ_TRANSPARENT,             // 32 Read data
   eQMI_UIM_READ_TRANSPARENT_IND = 32,    // 32 Read data indication
   eQMI_UIM_READ_RECORD,                  // 33 Read one or more records
   eQMI_UIM_READ_RECORD_IND = 33,         // 33 Read records indication
   eQMI_UIM_WRITE_TRANSPARENT,            // 34 Write data
   eQMI_UIM_WRITE_TRANSPARENT_IND = 34,   // 34 Write data indication
   eQMI_UIM_WRITE_RECORD,                 // 35 Write a record
   eQMI_UIM_WRITE_RECORD_IND = 35,        // 35 Write a record indication
   eQMI_UIM_GET_FILE_ATTRIBUTES,          // 36 Get file attributes
   eQMI_UIM_GET_FILE_ATTRIBUTES_IND = 36, // 36 Get file attributes indication
   eQMI_UIM_SET_PIN_PROTECTION,           // 37 Set PIN protection
   eQMI_UIM_SET_PIN_PROTECTION_IND = 37,  // 37 Set PIN protection indication
   eQMI_UIM_VERITFY_PIN,                  // 38 Verify PIN 
   eQMI_UIM_VERITFY_PIN_IND = 38,         // 38 Verify PIN indication
   eQMI_UIM_UNBLOCK_PIN,                  // 39 Unblock PIN
   eQMI_UIM_UNBLOCK_PIN_IND = 39,         // 39 Unblock PIN indication
   eQMI_UIM_CHANGE_PIN,                   // 40 Change PIN
   eQMI_UIM_CHANGE_PIN_IND = 40,          // 40 Change PIN indication
   eQMI_UIM_DEPERSONALIZATION,            // 41 Depersonalization
   eQMI_UIM_REFRESH_REGISTER,             // 42 Refresh register
   eQMI_UIM_REFRESH_OK,                   // 43 Validate refresh
   eQMI_UIM_REFRESH_COMPLETE,             // 44 Complete refresh
   eQMI_UIM_GET_LAST_REFRESH_EVENT,       // 45 Get last refresh event
   eQMI_UIM_EVENT_REGISTRATION,           // 46 Register for indications
   eQMI_UIM_GET_CARD_STATUS,              // 47 Get card status
   eQMI_UIM_POWER_DOWN,                   // 48 Power down
   eQMI_UIM_POWER_UP,                     // 49 Power up
   eQMI_UIM_CARD_STATUS_IND,              // 50 Card status indication
   eQMI_UIM_REFRESH_IND,                  // 51 Refresh indication
   eQMI_UIM_AUTHENTICATE,                 // 52 Authenticate
   eQMI_UIM_AUTHENTICATE_IND = 52,        // 52 Authenticate indication
   eQMI_UIM_CLOSE_SESSION,                // 53 Close session
   eQMI_UIM_GET_SERVICE_STATUS,           // 54 Get service status
   eQMI_UIM_SET_SERVICE_STATUS,           // 55 Set service status
   eQMI_UIM_CHANGE_PROVISIONING,          // 56 Change provisioning   
   eQMI_UIM_GET_LABEL,                    // 57 Get label 
   eQMI_UIM_GET_CONFIG,                   // 58 Get configuration
   eQMI_UIM_SEND_ADPU,                    // 59 Send ADPU
   eQMI_UIM_SEND_ADPU_IND = 59,           // 59 Send ADPU indication
   eQMI_UIM_SAP_CONNECTION,               // 60 SAP connection
   eQMI_UIM_SAP_REQUEST,                  // 61 SAP request
   eQMI_UIM_SAP_CONNECTION_IND,           // 62 SAP connection indication
   eQMI_UIM_LOGICAL_CHANNEL,              // 63 Logical channel
   eQMI_UIM_SUBSCRIPTION_OK,              // 64 OK to publish subscription?
   eQMI_UIM_GET_ATR,                      // 65 Get ATR
   eQMI_UIM_SESSION_CLOSED_IND,           // 67 Session closed indication
   eQMI_UIM_REGISTER_REFRESH_ALL,         // 68 Register for all refresh 
   eQMI_UIM_SET_FILE_STATUS,              // 69 Set file status 

   eQMI_UIM_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageUIM validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageUIM msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_UIM_RESET)
   ||   (msgID >= eQMI_UIM_GET_MESSAGES && msgID < eQMI_UIM_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessagePBM Enumeration
//    QMI PBM Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessagePBM
{
   eQMI_PBM_ENUM_BEGIN = -1, 

   eQMI_PBM_INDICATION_REG = 1,     // 01 Set indication registration state
   eQMI_PBM_GET_CAPABILITIES,       // 02 Get phonebook capabilities by type
   eQMI_PBM_GET_ALL_CAPABILITIES,   // 03 Get all phonebook capabilities
   eQMI_PBM_READ_RECORDS,           // 04 Read phonebook records
   eQMI_PBM_READ_RECORD_IND = 4,    // 04 Read phonebook record indication
   eQMI_PBM_WRITE_RECORD,           // 05 Add/modify a phonebook record
   eQMI_PBM_DELETE_RECORD,          // 06 Delete a phonebook record
   eQMI_PBM_DELETE_ALL_RECORDS,     // 07 Delete all phonebook records
   eQMI_PBM_SEARCH_RECORDS,         // 08 Search phonebook records
   eQMI_PBM_RECORD_UPDATE_IND,      // 09 Phonebook record update indication
   eQMI_PBM_REFRESH_IND,            // 10 Phonebook refresh indication
   eQMI_PBM_READY_IND,              // 11 Phonebook ready indication
   eQMI_PBM_EMERGENCY_LIST_IND,     // 12 Phonebook emergency list indication
   eQMI_PBM_ALL_READY_IND,          // 13 All phonebooks ready indication
   eQMI_PBM_GET_EMERGENCY_LIST,     // 14 Get phonebook emergency list
   eQMI_PBM_GET_ALL_GROUPS,         // 15 Get all phonebook groups
   eQMI_PBM_SET_GROUP_INFO,         // 16 Set phonebook group info
   eQMI_PBM_GET_STATE,              // 17 Get phonebook state
   eQMI_PBM_READ_ALL_HIDDEN_RECS,   // 18 Read all hidden phonebook records
   eQMI_PBM_HIDDEN_REC_STATUS_IND,  // 19 Hidden record status indication
   eQMI_PBM_GET_NEXT_EMPTY_REC_ID,  // 20 Get next empty record ID
   eQMI_PBM_GET_NEXT_REC_ID,        // 21 Get next non-empty record ID
   eQMI_PBM_GET_AAS_LIST,           // 22 Get AAS list
   eQMI_PBM_SET_AAS,                // 23 Add/modify/delete AAS entry
   eQMI_PBM_UPDATE_AAS_IND,         // 24 AAS change indication
   eQMI_PBM_UPDATE_GAS_IND,         // 25 GAS change indication
   eQMI_PBM_BIND_SUBSCRIPTION,      // 26 Bind client to specific subscription
   eQMI_PBM_GET_SUBSCRIPTION,       // 27 Get subscription client is bound to
   eQMI_PBM_SET_CAPS_READ_OP,       // 28 ADN set capabilities read operation
   eQMI_PBM_SET_CAPS_READ_IND = 28, // 28 ADN set caps read op indication
   eQMI_PBM_EX_READ_RECORDS,        // 29 Read phonebook records
   eQMI_PBM_EX_READ_RECORD_IND = 29,// 29 Read phonebook record indication
   eQMI_PBM_EX_WRITE_RECORD,        // 30 Add/modify a phonebook record
   eQMI_PBM_EX_SEARCH_RECORDS,      // 31 Search phonebook records
   eQMI_PBM_EX_READ_ALL_HIDDEN_RECS,// 32 Read all hidden phonebook records
   eQMI_PBM_SIM_INIT_DONE_IND,      // 33 SIM initialization done indication
   eQMI_PBM_READ_UNDC_RECS,         // 34 Read undecoded phonebook records
   eQMI_PBM_READ_UNDC_RECS_IND = 34,// 34 Read undecoded phonebook record ind

   eQMI_PBM_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessagePBM validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessagePBM msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_PBM_INDICATION_REG && msgID < eQMI_PBM_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageLOC Enumeration
//    QMI LOC Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageLOC
{
   eQMI_LOC_ENUM_BEGIN = -1, 

   eQMI_LOC_GET_MESSAGES = 30,      // 030 Get supported messages
   eQMI_LOC_GET_FIELDS,             // 031 Get supported fields
   eQMI_LOC_CLIENT_REVISION,        // 032 Send client revision to service
   eQMI_LOC_REGISTER_EVENTS,        // 033 Register for events/indications
   eQMI_LOC_START,                  // 034 Start GPS session
   eQMI_LOC_STOP,                   // 035 Start GPS session
   eQMI_LOC_POSITION_REPORT_IND,    // 036 Position report indication
   eQMI_LOC_GNSS_SAT_INFO_IND,      // 037 GNSS satellite info indication
   eQMI_LOC_NMEA_IND,               // 038 NMEA sentence indication
   eQMI_LOC_NETWORK_REQ_IND,        // 039 Network initiated request indication
   eQMI_LOC_INJECT_TIME_REQ_IND,    // 040 Inject time request indication
   eQMI_LOC_INJECT_ORBITS_REQ_IND,  // 041 Inject predicted orbits req ind
   eQMI_LOC_INJECT_POS_REQ_IND,     // 042 Inject position request indication
   eQMI_LOC_ENGINE_STATE_IND,       // 043 Engine state indication
   eQMI_LOC_FIX_SESSION_STATE_IND,  // 044 Fi session state indication
   eQMI_LOC_WIFI_REQ_IND,           // 045 Wi-Fi request indication
   eQMI_LOC_SENSOR_DATA_IND,        // 046 Sensor streaming ready status ind
   eQMI_LOC_INJECT_TIME_SYNC_IND,   // 047 Inject time sync data indication
   eQMI_LOC_SPI_STREAM_REQ_IND,     // 048 SPI streaming reports req indication
   eQMI_LOC_SVR_CONNECTION_REQ_IND, // 049 Server connection req indication
   eQMI_LOC_GET_REV_REQ,            // 050 Get service revision
   eQMI_LOC_GET_REV_REQ_IND = 50,   // 050 Get service revision indication
   eQMI_LOC_GET_FIX_CRIT,           // 051 Get fix criteria
   eQMI_LOC_GET_FIX_CRIT_IND = 51,  // 051 Get fix criteria indication
   eQMI_LOC_NI_USER_RSP,            // 052 Network initiated user response
   eQMI_LOC_NI_USER_RSP_IND = 52,   // 052 Network initiated user response ind
   eQMI_LOC_INJECT_ORBITS,          // 053 Inject predicted orbits data
   eQMI_LOC_INJECT_ORBITS_IND = 53, // 053 Inject predicted orbits indication
   eQMI_LOC_GET_ORBIT_SRC,          // 054 Get predicted orbits data source
   eQMI_LOC_GET_ORBIT_SRC_IND = 54, // 054 Get predicted orbits data source ind
   eQMI_LOC_GET_ORBIT_VLD,          // 055 Get predicted orbits data validity
   eQMI_LOC_GET_ORBIT_VLD_IND = 55, // 055 Get predicted orbits validity ind
   eQMI_LOC_INJECT_UTC,             // 056 Inject UTC time
   eQMI_LOC_INJECT_UTC_IND = 56,    // 056 Inject UTC time indication
   eQMI_LOC_INJECT_POS,             // 057 Inject position
   eQMI_LOC_INJECT_POS_IND = 57,    // 057 Inject position indication
   eQMI_LOC_SET_ENG_LOCK,           // 058 Set engine lock
   eQMI_LOC_SET_ENG_LOCK_IND = 58,  // 058 Set engine lock indication
   eQMI_LOC_GET_ENG_LOCK,           // 059 Get engine lock
   eQMI_LOC_GET_ENG_LOCK_IND = 59,  // 059 Get engine lock indication
   eQMI_LOC_SET_SBAS_CFG,           // 060 Set SBAS config
   eQMI_LOC_SET_SBAS_CFG_IND = 60,  // 060 Set SBAS config indication
   eQMI_LOC_GET_SBAS_CFG,           // 061 Get SBAS config
   eQMI_LOC_GET_SBAS_CFG_IND = 61,  // 061 Get SBAS config indication
   eQMI_LOC_SET_NMEA_TYPS,          // 062 Set NMEA sentence types
   eQMI_LOC_SET_NMEA_TYPS_IND = 62, // 062 Set NMEA sentence types indication
   eQMI_LOC_GET_NMEA_TYPS,          // 063 Get NMEA sentence types
   eQMI_LOC_GET_NMEA_TYPS_IND = 63, // 063 Get NMEA sentence types indication
   eQMI_LOC_SET_LPM_CFG,            // 064 Set low power mode config
   eQMI_LOC_SET_LPM_CFG_IND = 64,   // 064 Set low power mode config indication
   eQMI_LOC_GET_LPM_CFG,            // 065 Get low power mode config
   eQMI_LOC_GET_LPM_CFG_IND = 65,   // 065 Get low power mode config indication
   eQMI_LOC_SET_SERVER,             // 066 Set A-GPS server
   eQMI_LOC_SET_SERVER_IND = 66,    // 066 Set A-GPS server indication
   eQMI_LOC_GET_SERVER,             // 067 Set A-GPS server
   eQMI_LOC_GET_SERVER_IND = 67,    // 067 Set A-GPS server indication
   eQMI_LOC_DEL_ASST_DATA,          // 068 Delete assistance data
   eQMI_LOC_DEL_ASST_DATA_IND = 68, // 068 Delete assistance data indication
   eQMI_LOC_SET_XTRA_T,             // 069 Set XTRA_T session control 
   eQMI_LOC_SET_XTRA_T_IND = 69,    // 069 Set XTRA_T session control indication
   eQMI_LOC_GET_XTRA_T,             // 070 Get XTRA_T session control 
   eQMI_LOC_GET_XTRA_T_IND = 70,    // 070 Get XTRA_T session control indication
   eQMI_LOC_INJECT_WIFI,            // 071 Inject Wi-Fi info
   eQMI_LOC_INJECT_WIFI_IND = 71,   // 071 Inject Wi-Fi info indication
   eQMI_LOC_NOTIFY_WIFI,            // 072 Notify server of Wi-Fi status
   eQMI_LOC_NOTIFY_WIFI_IND = 72,   // 072 Notify server of Wi-Fi status ind
   eQMI_LOC_GET_REG_EVENTS,         // 073 Get registered event status
   eQMI_LOC_GET_REG_EVENTS_IND = 73,// 073 Get registered event status ind
   eQMI_LOC_SET_OP_MODE,            // 074 Set operation mode
   eQMI_LOC_SET_OP_MODE_IND = 74,   // 074 Set operation mode indication
   eQMI_LOC_GET_OP_MODE,            // 075 Get operation mode
   eQMI_LOC_GET_OP_MODE_IND = 75,   // 075 Get operation mode indication
   eQMI_LOC_SET_SPI_STATUS,         // 076 Set SPI status
   eQMI_LOC_SET_SPI_STATUS_IND = 76,// 076 Set SPI status indication
   eQMI_LOC_INJECT_SENSOR,          // 077 Inject sensor data
   eQMI_LOC_INJECT_SENSOR_IND = 77, // 077 Inject sensor data indication
   eQMI_LOC_INJ_TIME_SYNC,          // 078 Inject time sync data
   eQMI_LOC_INJ_TIME_SYNC_IND = 78, // 078 Inject time sync data indication
   eQMI_LOC_SET_CRADLE,             // 079 Set cradle mount config
   eQMI_LOC_SET_CRADLE_IND = 79,    // 079 Set cradle mount config indication
   eQMI_LOC_GET_CRADLE,             // 080 Get cradle mount config
   eQMI_LOC_GET_CRADLE_IND = 80,    // 080 Get cradle mount config indication
   eQMI_LOC_SET_EXT_POWER,          // 081 Set external power config
   eQMI_LOC_SET_EXT_POWER_IND = 81, // 081 Set external power config indication
   eQMI_LOC_GET_EXT_POWER,          // 082 Get external power config
   eQMI_LOC_GET_EXT_POWER_IND = 82, // 082 Get external power config indication
   eQMI_LOC_INFORM_CONN,            // 083 Inform service of connection status
   eQMI_LOC_INFORM_CONN_IND = 83,   // 083 Inform connection status indication
   eQMI_LOC_SET_PROTO_CFG,          // 084 Set protocol config
   eQMI_LOC_SET_PROTO_CFG_IND = 84, // 084 Set protocol config indication
   eQMI_LOC_GET_PROTO_CFG,          // 085 Get protocol config
   eQMI_LOC_GET_PROTO_CFG_IND = 85, // 085 Get protocol config indication
   eQMI_LOC_SET_SENSOR_CFG,         // 086 Set sensor control config
   eQMI_LOC_SET_SENSOR_CFG_IND = 86,// 086 Set sensor control config indication
   eQMI_LOC_GET_SENSOR_CFG,         // 087 Get sensor control config
   eQMI_LOC_GET_SENSOR_CFG_IND = 87,// 087 Get sensor control config indication
   eQMI_LOC_SET_SENSOR_PRP,         // 088 Set sensor properties
   eQMI_LOC_SET_SENSOR_PRP_IND = 88,// 088 Set sensor properties indication
   eQMI_LOC_GET_SENSOR_PRP,         // 089 Get sensor properties
   eQMI_LOC_GET_SENSOR_PRP_IND = 89,// 089 Get sensor properties indication
   eQMI_LOC_SET_SENSOR_PRF,         // 090 Set sensor performance control
   eQMI_LOC_SET_SENSOR_PRF_IND = 90,// 090 Set sensor performance control ind
   eQMI_LOC_GET_SENSOR_PRF,         // 091 Get sensor performance control
   eQMI_LOC_GET_SENSOR_PRF_IND = 91,// 091 Get sensor performance control ind
   eQMI_LOC_INJ_SUPL_CERT,          // 092 Inject SUPL certificate
   eQMI_LOC_INJ_SUPL_CERT_IND = 92, // 092 Inject SUPL certificate indication
   eQMI_LOC_DEL_SUPL_CERT,          // 093 Delete SUPL certificate
   eQMI_LOC_DEL_SUPL_CERT_IND = 93, // 093 Delete SUPL certificate indication
   eQMI_LOC_SET_ENGINE_CFG,         // 094 Set position engine config
   eQMI_LOC_SET_ENGINE_CFG_IND = 94,// 094 Set position engine config ind
   eQMI_LOC_GET_ENGINE_CFG,         // 095 Get position engine config
   eQMI_LOC_GET_ENGINE_CFG_IND = 95,// 095 Get position engine config ind
   eQMI_LOC_GEOFENCE_NOTIF_IND,     // 096 Geofence notification indication
   eQMI_LOC_GEOFENCE_ALERT_IND,     // 097 Geofence alert indication
   eQMI_LOC_GEOFENCE_BREACH_IND,    // 098 Geofence breach indication
   eQMI_LOC_ADD_GEOFENCE,           // 099 Add circular geofence
   eQMI_LOC_ADD_GEOFENCE_IND = 99,  // 099 Add circular geofence indication
   eQMI_LOC_DEL_GEOFENCE,           // 100 Delete geofence
   eQMI_LOC_DEL_GEOFENCE_IND = 100, // 100 Delete geofence indication
   eQMI_LOC_QRY_GEOFENCE,           // 101 Query geofence
   eQMI_LOC_QRY_GEOFENCE_IND = 101, // 101 Query geofence indication
   eQMI_LOC_EDIT_GEOFENCE,          // 102 Edit geofence 
   eQMI_LOC_EDIT_GEOFENCE_IND = 102,// 102 Edit geofence indication
   eQMI_LOC_GET_BEST_POS,           // 103 Get best available position
   eQMI_LOC_GET_BEST_POS_IND = 103, // 103 Get best available position ind
   eQMI_LOC_INJ_MOTION,             // 104 Inject motion data
   eQMI_LOC_INJ_MOTION_IND = 104,   // 104 Inject motion data indication
   eQMI_LOC_GET_NI_GF_IDS,          // 105 Get NI geofence ID list
   eQMI_LOC_GET_NI_GF_IDS_IND = 105,// 105 Get NI geofence ID list indication
   eQMI_LOC_INJ_GSM_CELL,           // 106 Inject GSM cell info
   eQMI_LOC_INJ_GSM_CELL_IND = 106, // 106 Inject GSM cell info indication
   eQMI_LOC_INJ_NI_MSG,             // 107 Inject network initiated message
   eQMI_LOC_INJ_NI_MSG_IND = 107,   // 107 Inject NI message indication
   eQMI_LOC_WWAN_OOS,               // 108 Notify WWAN is now OOS
   eQMI_LOC_WWAN_OOS_IND = 108,     // 108 Notify WWAN is now OOS indication
   eQMI_PEDOMETER_CTRL_IND,         // 109 Pedometer control indication
   eQMI_MOTION_DATA_CTRL_IND,       // 110 Motion data control indication
   eQMI_LOC_INJ_PEDOMETER,          // 111 Inject pedometer data
   eQMI_LOC_INJ_PEDOMETER_IND = 111,// 111 Inject pedometer data indication
   eQMI_LOC_INJ_WCDMA_CI,           // 112 Inject WCDMA cell info
   eQMI_LOC_INJ_WCDMA_CI_IND = 112, // 112 Inject WCDMA cell info indication
   eQMI_LOC_INJ_TDSCDMA,            // 113 Inject TD-SCDMA cell info
   eQMI_LOC_INJ_TDSCDMA_IND = 113,  // 113 Inject TD-SCDMA cell info indication
   eQMI_LOC_INJ_SUBS_ID,            // 114 Inject subscriber ID
   eQMI_LOC_INJ_SUBS_ID_IND = 114,  // 114 Inject subscriber ID indication

   eQMI_LOC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageLOC validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageLOC msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_LOC_GET_MESSAGES && msgID < eQMI_LOC_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageSAR Enumeration
//    QMI SAR Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageSAR
{
   eQMI_SAR_ENUM_BEGIN = -1, 

   eQMI_SAR_SET_RF_SAR_STATE = 1,   // 01 Set the RF SAR state
   eQMI_SAR_GET_RF_SAR_STATE,       // 02 Get the RF SAR state

   eQMI_SAR_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageSAR validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageSAR msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_SAR_SET_RF_SAR_STATE && msgID < eQMI_SAR_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageRMTFS Enumeration
//    QMI Remote Filesystem Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageRMTFS
{
   eQMI_RMTFS_ENUM_BEGIN = -1, 

   eQMI_RMTFS_OPEN = 1,             // 01 Request an open 
   eQMI_RMTFS_CLOSE,                // 02 Request a close
   eQMI_RMTFS_IO_VECTOR,            // 03 Request an I/O vector operation
   eQMI_RMTFS_ALLOC_BUFFER,         // 04 Allocate a buffer
   eQMI_RMTFS_GET_DEV_ERROR,        // 05 Return device error for last request
   eQMI_RMTFS_FORCE_SYNC_IND,       // 06 Force sync indication

   eQMI_RMTFS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageRMTFS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageRMTFS msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_RMTFS_OPEN && msgID < eQMI_RMTFS_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageWDA Enumeration
//    QMI Qualcomm Wireless Data Addministrative Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageWDA
{
   eQMI_WDA_ENUM_BEGIN = -1, 

   eQMI_WDA_GET_MESSAGES = 30,             // 30 Get supported messages
   eQMI_WDA_GET_FIELDS,                    // 31 Get supported fields
   eQMI_WDA_SET_DATA_FORMAT,               // 32 
   eQMI_WDA_GET_DATA_FORMAT,               // 33 
   eQMI_WDA_ENABLE_PKT_FILTER,             // 34
   eQMI_WDA_DISABLE_PKT_FILTER,            // 35
   eQMI_WDA_GET_PKT_FILTER_STATE,          // 36
   eQMI_WDA_ADD_PKT_FILTER_RULE,           // 37
   eQMI_WDA_DELETE_PKT_FILTER_RULE,        // 38
   eQMI_WDA_GET_PKT_FILTER_RULE_HANDLES,   // 39
   eQMI_WDA_GET_PKT_FILTER_RULE,           // 40
   eQMI_WDA_SET_LOOPBACK_STATE,            // 41
   eQMI_WDA_GET_LOOPBACK_STATE,            // 42

   eQMI_WDA_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageWDA validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageWDA msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_WDA_GET_MESSAGES && msgID < eQMI_WDA_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageQCMAP Enumeration
//    QMI Qualcomm Mobile Access Point Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageQCMAP
{
   eQMI_QCMAP_ENUM_BEGIN = -1, 

   eQMI_QCMAP_ENABLE = 32,                   // 32 
   eQMI_QCMAP_DISABLE,                       // 33 
   eQMI_QCMAP_BRING_UP_WWAN,                 // 34
   eQMI_QCMAP_BRING_UP_WWAN_IND = 34,        // 34
   eQMI_QCMAP_TEAR_DOWN_WWAN,                // 35
   eQMI_QCMAP_TEAR_DOWN_WWAN_IND = 35,       // 35
   eQMI_QCMAP_GET_WWAN_STATUS,               // 36
   eQMI_QCMAP_GET_IPSEC_VPN_PASSTHROUGH,     // 37
   eQMI_QCMAP_SET_IPSEC_VPN_PASSTHROUGH,     // 38
   eQMI_QCMAP_GET_PPTP_VPN_PASSTHROUGH,      // 39
   eQMI_QCMAP_SET_PPTP_VPN_PASSTHROUGH,      // 40
   eQMI_QCMAP_GET_L2TP_VPN_PASSTHROUGH,      // 41
   eQMI_QCMAP_SET_L2TP_VPN_PASSTHROUGH,      // 42
   eQMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TO,      // 43
   eQMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TO,      // 44
   eQMI_QCMAP_ADD_SNAT_ENTRY,                // 45
   eQMI_QCMAP_DELETE_SNAT_ENTRY,             // 46
   eQMI_QCMAP_GET_SNAT_ENTRIES,              // 47
   eQMI_QCMAP_SET_DMZ,                       // 48
   eQMI_QCMAP_DELETE_DMZ,                    // 49
   eQMI_QCMAP_GET_DMZ,                       // 50
   eQMI_QCMAP_GET_WWAN_CONFIG,               // 51
   eQMI_QCMAP_ENABLE_FIREWALL_SETTING,       // 52
   eQMI_QCMAP_GET_FIREWALL_SETTING,          // 53
   eQMI_QCMAP_DISABLE_FIREWALL_SETTING,      // 54
   eQMI_QCMAP_ADD_FIREWALL_CONFIG,           // 55
   eQMI_QCMAP_GET_FIREWALL_CONFIG,           // 56
   eQMI_QCMAP_DELETE_FIREWALL_CONFIG,        // 57
   eQMI_QCMAP_WWAN_STATUS_IND_REG,           // 58
   eQMI_QCMAP_STATION_MODE_ENABLE,           // 59
   eQMI_QCMAP_STATION_MODE_DISABLE,          // 60
   eQMI_QCMAP_GET_STATION_MODE,              // 61
   eQMI_QCMAP_WWAN_STATUS_IND,               // 62
   eQMI_QCMAP_ADD_EXT_FIREWALL_CONFIG,       // 63
   eQMI_QCMAP_GET_EXT_FIREWALL_CONFIG,       // 64
   eQMI_QCMAP_GET_FIREWALL_HANDLES,          // 65
   eQMI_QCMAP_CHANGE_NAT_TYPE,               // 66
   eQMI_QCMAP_GET_NAT_TYPE,                  // 67

   eQMI_QCMAP_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageQCMAP validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageQCMAP msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_QCMAP_ENABLE && msgID < eQMI_QCMAP_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageCOEX Enumeration
//    QMI COEX Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageCOEX
{
   eQMI_COEX_ENUM_BEGIN = -1, 

   eQMI_COEX_RESET,                    // 00 Reset the PDC service

   eQMI_COEX_REG_INDICATIONS = 32,     // 32 Register for indications
   eQMI_COEX_WWAN_STATE_IND,           // 33 WWAN state indication
   eQMI_COEX_GET_WWAN_STATE,           // 34 Get WWAN state
   eQMI_COEX_SET_WLAN_STATE,           // 35 Set WLAN state
   eQMI_COEX_GET_WLAN_SCAN_STATE,      // 36 Get WLAN scan state
   eQMI_COEX_GET_WLAN_CONN_STATE,      // 37 Get WLAN connection state
   eQMI_COEX_SET_POLICY,               // 38 Set policy
   eQMI_COEX_GET_POLICY,               // 39 Get policy

   eQMI_COEX_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageCOEX validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageCOEX msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_COEX_RESET)
   ||   (msgID >= eQMI_COEX_REG_INDICATIONS && msgID < eQMI_COEX_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessagePDC Enumeration
//    QMI PDC Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessagePDC
{
   eQMI_PDC_ENUM_BEGIN = -1, 

   eQMI_PDC_RESET,                     // 00 Reset the PDC service

   eQMI_PDC_REG_INDICATIONS = 32,      // 32 Register for indications
   eQMI_PDC_CFG_CHANGE_IND,            // 33 Config change indication
   eQMI_PDC_GET_CFG,                   // 34 Get selected config 
   eQMI_PDC_GET_CFG_IND = 34,          // 34 Get selected config indication
   eQMI_PDC_SET_CFG,                   // 35 Set selected config 
   eQMI_PDC_SET_CFG_IND = 35,          // 35 Set selected config indication
   eQMI_PDC_LIST_CFGS,                 // 36 List configs
   eQMI_PDC_LIST_CFGS_IND = 36,        // 36 List configs indication
   eQMI_PDC_DEL_CFG,                   // 37 Delete config 
   eQMI_PDC_DEL_CFG_IND = 37,          // 37 Delete config indication
   eQMI_PDC_LOAD_CFG,                  // 38 Load config 
   eQMI_PDC_LOAD_CFG_IND = 38,         // 38 Load config indication
   eQMI_PDC_ACTIVATE_CFG,              // 39 Activate config 
   eQMI_PDC_ACTIVATE_CFG_IND = 39,     // 39 Activate config indication
   eQMI_PDC_GET_CFG_INFO,              // 40 Get config info
   eQMI_PDC_GET_CFG_INFO_IND = 40,     // 40 Get config info indication
   eQMI_PDC_GET_CFG_LIMITS,            // 41 Get config limits
   eQMI_PDC_GET_CFG_LIMITS_IND = 41,   // 41 Get config limits indication
   eQMI_PDC_GET_DEF_CFG_INFO,          // 42 Get default config info
   eQMI_PDC_GET_DEF_CFG_INFO_IND = 42, // 42 Get default config info indication
   eQMI_PDC_DEACTIVATE_CFG,            // 43 Deactivate active config
   eQMI_PDC_DEACTIVATE_CFG_IND = 43,   // 43 Deactivate active config ind

   eQMI_PDC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessagePDC validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessagePDC msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_PDC_RESET)
   ||   (msgID >= eQMI_PDC_REG_INDICATIONS && msgID < eQMI_PDC_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageSTX Enumeration
//    QMI STX Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageSTX
{
   eQMI_STX_ENUM_BEGIN = -1, 

   eQMI_STX_TECH_TX_STATUS = 32,       // 32 Technology TX status
   eQMI_STX_UPDATE_TX_POWER_LIMIT,     // 33 Update TX power limit
   eQMI_STX_TX_STATE_UPDATE_IND,       // 34 TX state update indication
   eQMI_STX_TX_STATE_CLEAR_IND,        // 34 TX state clear indication
   eQMI_STX_SET_TECH_POWER_LUT,        // 36 Set technology power LUT
   eQMI_STX_TX_POWER_LIMIT_IND,        // 37 TX power limit state

   eQMI_STX_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageSTX validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageSTX msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_STX_TECH_TX_STATUS && msgID < eQMI_STX_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageRFRPE Enumeration
//    QMI RFRPE Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageRFRPE
{
   eQMI_RFRPE_ENUM_BEGIN = -1, 

   eQMI_RFRPE_SET_RFM_SCENARIO = 32,   // 32 Set RFM scenario
   eQMI_RFRPE_GET_RFM_SCENARIO,        // 33 Get RFM scenario
   eQMI_RFRPE_GET_TABLE_REVISION,      // 34 Get provisioned table revision

   eQMI_RFRPE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageRFRPE validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageRFRPE msgID )
{
   bool retVal = false;
   if (msgID >= eQMI_RFRPE_SET_RFM_SCENARIO && msgID < eQMI_RFRPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageCAT Enumeration
//    QMI CAT Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageCAT
{
   eQMI_CAT_ENUM_BEGIN = -1, 

   eQMI_CAT_RESET,                  // 00 Reset CAT service state variables
   eQMI_CAT_SET_EVENT,              // 01 Set new message report conditions
   eQMI_CAT_EVENT_IND = 1,          // 01 New message report indication

   eQMI_CAT_GET_MESSAGES = 30,      // 30 Get supported messages
   eQMI_CAT_GET_FIELDS,             // 31 Get supported fields
   eQMI_CAT_GET_STATE,              // 32 Get service state information
   eQMI_CAT_SEND_TERMINAL,          // 33 Send a terminal response
   eQMI_CAT_SEND_ENVELOPE,          // 34 Send an envelope command
   eQMI_CAT_GET_EVENT,              // 35 Get last message report
   eQMI_CAT_SEND_DECODED_TERMINAL,  // 36 Send a decoded terminal response
   eQMI_CAT_SEND_DECODED_ENVELOPE,  // 37 Send a decoded envelope command
   eQMI_CAT_EVENT_CONFIRMATION,     // 38 Event confirmation
   eQMI_CAT_SCWS_OPEN_CHANNEL,      // 39 Open a channel to a SCWS
   eQMI_CAT_SCWS_OPEN_IND = 39,     // 39 SCWS open channel indication
   eQMI_CAT_SCWS_CLOSE_CHANNEL,     // 40 Close a channel to a SCWS
   eQMI_CAT_SCWS_CLOSE_IND = 40,    // 40 SCWS close channel indication
   eQMI_CAT_SCWS_SEND_DATA,         // 41 Send data to a SCWS
   eQMI_CAT_SCWS_SEND_IND = 41,     // 41 SCWS send data indication
   eQMI_CAT_SCWS_DATA_AVAILABLE,    // 42 Indicate that data is available
   eQMI_CAT_SCWS_CHANNEL_STATUS,    // 43 Provide channel status
   eQMI_CAT_GET_TERMINAL_PROFILE,   // 44 Get current modem terminal profile
   eQMI_CAT_SET_CONFIG,             // 45 Set configuration
   eQMI_CAT_GET_CONFIG,             // 46 Get configuration

   eQMI_CAT_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageCAT validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageCAT msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_CAT_ENUM_BEGIN && msgID <= eQMI_CAT_EVENT_IND)
   ||   (msgID >= eQMI_CAT_GET_MESSAGES && msgID < eQMI_CAT_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageRMS Enumeration
//    QMI RMS Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageRMS
{
   eQMI_RMS_ENUM_BEGIN = -1, 

   eQMI_RMS_RESET,                  // 00 Reset RMS service state variables

   eQMI_RMS_GET_SMS_WAKE = 32,      // 32 Get SMS wake settings
   eQMI_RMS_SET_SMS_WAKE,           // 33 Set SMS wake settings

   eQMI_RMS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageRMS validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageRMS msgID )
{
   bool retVal = false;
   if ( (msgID == eQMI_RMS_RESET)
   ||   (msgID >= eQMI_RMS_GET_SMS_WAKE && msgID < eQMI_RMS_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIMessageOMA Enumeration
//    QMI OMA-DM Service Type Message ID Enumeration
/*=========================================================================*/
enum eQMIMessageOMA
{
   eQMI_OMA_ENUM_BEGIN = -1, 

   eQMI_OMA_RESET,                  // 00 Reset OMA service state variables
   eQMI_OMA_SET_EVENT,              // 01 Set OMA report conditions
   eQMI_OMA_EVENT_IND = 1,          // 01 OMA report indication

   eQMI_OMA_START_SESSION = 32,     // 32 Start client inititated session
   eQMI_OMA_CANCEL_SESSION,         // 33 Cancel session
   eQMI_OMA_GET_SESSION_INFO,       // 34 Get session information
   eQMI_OMA_SEND_SELECTION,         // 35 Send selection for net inititated msg
   eQMI_OMA_GET_FEATURES,           // 36 Get feature settings
   eQMI_OMA_SET_FEATURES,           // 37 Set feature settings

   eQMI_OMA_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIMessageOMA validity check

PARAMETERS:
   msgID         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIMessageOMA msgID )
{
   bool retVal = false;
   if ( (msgID > eQMI_OMA_ENUM_BEGIN && msgID <= eQMI_OMA_EVENT_IND)
   ||   (msgID >= eQMI_OMA_START_SESSION && msgID < eQMI_OMA_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIResultCode Enumeration
//    QMI Result Code Enumeration
/*=========================================================================*/
enum eQMIResultCode
{
   eQMI_RC_ENUM_BEGIN = -1, 

   eQMI_RC_SUCCESS,           // 00 Success
   eQMI_RC_ERROR,             // 01 Error

   eQMI_RC_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIResultCode validity check

PARAMETERS:
   rc          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIResultCode rc )
{
   bool retVal = false;
   if (rc > eQMI_RC_ENUM_BEGIN && rc < eQMI_RC_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMIErrorCode Enumeration
//    QMI Error Code Enumeration
/*=========================================================================*/
enum eQMIErrorCode
{
   eQMI_ERR_ENUM_BEGIN = -1, 

   eQMI_ERR_NONE,                            // 00
   eQMI_ERR_MALFORMED_MSG,                   // 01
   eQMI_ERR_NO_MEMORY,                       // 02
   eQMI_ERR_INTERNAL,                        // 03    
   eQMI_ERR_ABORTED,                         // 04
   eQMI_ERR_CLIENT_IDS_EXHAUSTED,            // 05
   eQMI_ERR_UNABORTABLE_TRANSACTION,         // 06
   eQMI_ERR_INVALID_CLIENT_ID,               // 07
   eQMI_ERR_NO_THRESHOLDS,                   // 08
   eQMI_ERR_INVALID_HANDLE,                  // 09
   eQMI_ERR_INVALID_PROFILE,                 // 10
   eQMI_ERR_INVALID_PIN_ID,                  // 11
   eQMI_ERR_INCORRECT_PIN,                   // 12
   eQMI_ERR_NO_NETWORK_FOUND,                // 13
   eQMI_ERR_CALL_FAILED,                     // 14
   eQMI_ERR_OUT_OF_CALL,                     // 15
   eQMI_ERR_NOT_PROVISIONED,                 // 16
   eQMI_ERR_MISSING_ARG,                     // 17
   eQMI_ERR_18,                              // 18
   eQMI_ERR_ARG_TOO_LONG,                    // 19
   eQMI_ERR_20,                              // 20
   eQMI_ERR_21,                              // 21
   eQMI_ERR_INVALID_TX_ID,                   // 22
   eQMI_ERR_DEVICE_IN_USE,                   // 23
   eQMI_ERR_OP_NETWORK_UNSUPPORTED,          // 24
   eQMI_ERR_OP_DEVICE_UNSUPPORTED,           // 25
   eQMI_ERR_NO_EFFECT,                       // 26
   eQMI_ERR_NO_FREE_PROFILE,                 // 27
   eQMI_ERR_INVALID_PDP_TYPE,                // 28
   eQMI_ERR_INVALID_TECH_PREF,               // 29
   eQMI_ERR_INVALID_PROFILE_TYPE,            // 30
   eQMI_ERR_INVALID_SERVICE_TYPE,            // 31
   eQMI_ERR_INVALID_REGISTER_ACTION,         // 32
   eQMI_ERR_INVALID_PS_ATTACH_ACTION,        // 33
   eQMI_ERR_AUTHENTICATION_FAILED,           // 34
   eQMI_ERR_PIN_BLOCKED,                     // 35
   eQMI_ERR_PIN_ALWAYS_BLOCKED,              // 36
   eQMI_ERR_UIM_UNINITIALIZED,               // 37
   eQMI_ERR_MAX_QOS_REQUESTS_IN_USE,         // 38
   eQMI_ERR_INCORRECT_FLOW_FILTER,           // 39
   eQMI_ERR_NETWORK_QOS_UNAWARE,             // 40
   eQMI_ERR_INVALID_QOS_ID,                  // 41
   eQMI_ERR_REQUESTED_NUM_UNSUPPORTED,       // 42
   eQMI_ERR_INTERFACE_NOT_FOUND,             // 43
   eQMI_ERR_FLOW_SUSPENDED,                  // 44
   eQMI_ERR_INVALID_DATA_FORMAT,             // 45
   eQMI_ERR_GENERAL,                         // 46
   eQMI_ERR_UNKNOWN,                         // 47
   eQMI_ERR_INVALID_ARG,                     // 48
   eQMI_ERR_INVALID_INDEX,                   // 49
   eQMI_ERR_NO_ENTRY,                        // 50
   eQMI_ERR_DEVICE_STORAGE_FULL,             // 51
   eQMI_ERR_DEVICE_NOT_READY,                // 52
   eQMI_ERR_NETWORK_NOT_READY,               // 53
   eQMI_ERR_WMS_CAUSE_CODE,                  // 54
   eQMI_ERR_WMS_MESSAGE_NOT_SENT,            // 55
   eQMI_ERR_WMS_MESSAGE_DELIVERY_FAILURE,    // 56
   eQMI_ERR_WMS_INVALID_MESSAGE_ID,          // 57
   eQMI_ERR_WMS_ENCODING,                    // 58
   eQMI_ERR_AUTHENTICATION_LOCK,             // 59
   eQMI_ERR_INVALID_TRANSITION,              // 60
   eQMI_ERR_61,                              // 61
   eQMI_ERR_62,                              // 62
   eQMI_ERR_63,                              // 63
   eQMI_ERR_64,                              // 64
   eQMI_ERR_SESSION_INACTIVE,                // 65
   eQMI_ERR_SESSION_INVALID,                 // 66
   eQMI_ERR_SESSION_OWNERSHIP,               // 67
   eQMI_ERR_INSUFFICIENT_RESOURCES,          // 68
   eQMI_ERR_DISABLED,                        // 69
   eQMI_ERR_INVALID_OPERATION,               // 70
   eQMI_ERR_INVALID_QMI_CMD,                 // 71
   eQMI_ERR_WMS_TPDU_TYPE,                   // 72
   eQMI_ERR_WMS_SMSC_ADDR,                   // 73
   eQMI_ERR_INFO_UNAVAILABLE,                // 74
   eQMI_ERR_SEGMENT_TOO_LONG,                // 75
   eQMI_ERR_SEGMENT_ORDER,                   // 76
   eQMI_ERR_BUNDLING_NOT_SUPPORTED,          // 77
   eQMI_ERR_78,                              // 78
   eQMI_ERR_POLICY_MISMATCH,                 // 79
   eQMI_ERR_SIM_FILE_NOT_FOUND,              // 80
   eQMI_ERR_EXTENDED_EXTERNAL,               // 81
   eQMI_ERR_ACCESS_DENIED,                   // 82
   eQMI_ERR_HARDWARE_RESTRICTED,             // 83
   eQMI_ERR_ACK_NOT_SENT,                    // 84

   eQMI_ERR_INCOMPATIBLE_STATE = 90,         // 90
   eQMI_ERR_FDN_RESTRICT,                    // 91
   eQMI_ERR_SUPS_FAILURE_CAUSE,              // 92
   eQMI_ERR_NO_RADIO,                        // 93
   eQMI_ERR_NOT_SUPPORTED,                   // 94

   eQMI_ERR_CARD_CALL_CONTROL_FAILED = 96,   // 96
   eQMI_ERR_NETWORK_ABORTED,                 // 97

   eQMI_ERR_CAT_EVT_REG_FAILED,              // 61441
   eQMI_ERR_CAT_INVALID_TR,                  // 61442
   eQMI_ERR_CAT_INVALID_ENV_CMD,             // 61443
   eQMI_ERR_CAT_ENV_CMD_BUSY,                // 61444
   eQMI_ERR_CAT_ENV_CMD_FAIL,                // 61445

   eQMI_ERR_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMIErrorCode validity check

PARAMETERS:
   ec          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMIErrorCode ec )
{
   bool retVal = false;
   if ( (ec > eQMI_ERR_ENUM_BEGIN && ec <= eQMI_ERR_ACK_NOT_SENT)
   ||   (ec >= eQMI_ERR_INCOMPATIBLE_STATE && ec <= eQMI_ERR_NOT_SUPPORTED)
   ||   (ec == eQMI_ERR_CARD_CALL_CONTROL_FAILED)
   ||   (ec == eQMI_ERR_NETWORK_ABORTED)
   ||   (ec >= eQMI_ERR_CAT_EVT_REG_FAILED && ec < eQMI_ERR_ENUM_END) )
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eQMICallEndReason Enumeration
//    QMI Call End Reason Enumeration
/*=========================================================================*/
enum eQMICallEndReason
{
    eQMI_CALL_END_REASON_BEGIN = -1,

   // General 
   eQMI_CALL_END_REASON_UNSPECIFIED = 1,           // 1
   eQMI_CALL_END_REASON_CLIENT_END,                // 2
   eQMI_CALL_END_REASON_NO_SRV,                    // 3
   eQMI_CALL_END_REASON_FADE,                      // 4
   eQMI_CALL_END_REASON_REL_NORMAL,                // 5
   eQMI_CALL_END_REASON_ACC_IN_PROG,               // 6
   eQMI_CALL_END_REASON_ACC_FAIL,                  // 7
   eQMI_CALL_END_REASON_REDIR_OR_HANDOFF,          // 8
   eQMI_CALL_END_REASON_CLOSE_IN_PROGRESS,         // 9
   eQMI_CALL_END_REASON_AUTH_FAILED,               // 10
   eQMI_CALL_END_REASON_INTERNAL,                  // 11

   // CDMA
   eQMI_CALL_END_REASON_CDMA_LOCK = 500,           // 500
   eQMI_CALL_END_REASON_INTERCEPT,                 // 501
   eQMI_CALL_END_REASON_REORDER,                   // 502
   eQMI_CALL_END_REASON_REL_SO_REJ,                // 503
   eQMI_CALL_END_REASON_INCOM_CALL,                // 504
   eQMI_CALL_END_REASON_ALERT_STOP,                // 505
   eQMI_CALL_END_REASON_ACTIVATION,                // 506
   eQMI_CALL_END_REASON_MAX_ACCESS_PROBE,          // 507
   eQMI_CALL_END_REASON_CCS_NOT_SUPPORTED_BY_BS,   // 508
   eQMI_CALL_END_REASON_NO_RESPONSE_FROM_BS,       // 509
   eQMI_CALL_END_REASON_REJECTED_BY_BS,            // 510
   eQMI_CALL_END_REASON_INCOMPATIBLE,              // 511
   eQMI_CALL_END_REASON_ALREADY_IN_TC,             // 512
   eQMI_CALL_END_REASON_USER_CALL_ORIG_DURING_GPS, // 513
   eQMI_CALL_END_REASON_USER_CALL_ORIG_DURING_SMS, // 514
   eQMI_CALL_END_REASON_NO_CDMA_SRV,               // 515

   // GSM/WCDMA
   eQMI_CALL_END_REASON_CONF_FAILED = 1000,        // 1000
   eQMI_CALL_END_REASON_INCOM_REJ,                 // 1001
   eQMI_CALL_END_REASON_NO_GW_SRV,                 // 1002
   eQMI_CALL_END_REASON_NETWORK_END,               // 1003
   eQMI_CALL_END_REASON_LLC_SNDCP_FAILURE,         // 1004
   eQMI_CALL_END_REASON_INSUFFICIENT_RESOURCES,    // 1005
   eQMI_CALL_END_REASON_OPTION_TEMP_OOO,           // 1006
   eQMI_CALL_END_REASON_NSAPI_ALREADY_USED,        // 1007
   eQMI_CALL_END_REASON_REGULAR_DEACTIVATION,      // 1008
   eQMI_CALL_END_REASON_NETWORK_FAILURE,           // 1009
   eQMI_CALL_END_REASON_UMTS_REATTACH_REQ,         // 1010
   eQMI_CALL_END_REASON_UMTS_PROTOCOL_ERROR,       // 1011
   eQMI_CALL_END_REASON_OPERATOR_BARRING,          // 1012
   eQMI_CALL_END_REASON_UNKNOWN_APN,               // 1013
   eQMI_CALL_END_REASON_UNKNOWN_PDP,               // 1014
   eQMI_CALL_END_REASON_GGSN_REJECT,               // 1015
   eQMI_CALL_END_REASON_ACTIVATION_REJECT,         // 1016
   eQMI_CALL_END_REASON_OPTION_NOT_SUPPORTED,      // 1017
   eQMI_CALL_END_REASON_OPTION_UNSUBSCRIBED,       // 1018
   eQMI_CALL_END_REASON_QOS_NOT_ACCEPTED,          // 1019
   eQMI_CALL_END_REASON_TFT_SEMANTIC_ERROR,        // 1020
   eQMI_CALL_END_REASON_TFT_SYNTAX_ERROR,          // 1021
   eQMI_CALL_END_REASON_UNKNOWN_PDP_CONTEXT,       // 1022
   eQMI_CALL_END_REASON_FILTER_SEMANTIC_ERROR,     // 1023
   eQMI_CALL_END_REASON_FILTER_SYNTAX_ERROR,       // 1024
   eQMI_CALL_END_REASON_PDP_WITHOUT_ACTIVE_TFT,    // 1025
   eQMI_CALL_END_REASON_INVALID_TRANSACTION_ID,    // 1026
   eQMI_CALL_END_REASON_MESSAGE_SEMANTIC_ERROR,    // 1027
   eQMI_CALL_END_REASON_INVALID_MANDATORY_INFO,    // 1028
   eQMI_CALL_END_REASON_TYPE_UNSUPPORTED,          // 1029
   eQMI_CALL_END_REASON_MSG_TYPE_WRONG_FOR_STATE,  // 1030
   eQMI_CALL_END_REASON_UNKNOWN_INFO_ELEMENT,      // 1031
   eQMI_CALL_END_REASON_CONDITIONAL_IE_ERROR,      // 1032
   eQMI_CALL_END_REASON_MSG_WRONG_FOR_PROTOCOL,    // 1033
   eQMI_CALL_END_REASON_APN_TYPE_CONFLICT,         // 1034
   eQMI_CALL_END_REASON_NO_GPRS_CONTEXT,           // 1035
   eQMI_CALL_END_REASON_FEATURE_NOT_SUPPORTED,     // 1036

   // CDMA 1xEV-DO (HDR)
   eQMI_CALL_END_REASON_CD_GEN_OR_BUSY = 1500,     // 1500
   eQMI_CALL_END_REASON_CD_BILL_OR_AUTH,           // 1501
   eQMI_CALL_END_REASON_CHG_HDR,                   // 1502
   eQMI_CALL_END_REASON_EXIT_HDR,                  // 1503
   eQMI_CALL_END_REASON_HDR_NO_SESSION ,           // 1504
   eQMI_CALL_END_REASON_HDR_ORIG_DURING_GPS_FIX,   // 1505
   eQMI_CALL_END_REASON_HDR_CS_TIMEOUT ,           // 1506
   eQMI_CALL_END_REASON_HDR_RELEASED_BY_CM,        // 1507

   eQMI_CALL_END_REASON_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eQMICallEndReason validity check

PARAMETERS:
   err         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eQMICallEndReason err )
{
   if ( (err >= eQMI_CALL_END_REASON_UNSPECIFIED) 
   &&   (err <= eQMI_CALL_END_REASON_INTERNAL) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CDMA_LOCK)
   &&   (err <= eQMI_CALL_END_REASON_NO_CDMA_SRV) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CONF_FAILED)
   &&   (err <= eQMI_CALL_END_REASON_FEATURE_NOT_SUPPORTED) )
   {
      return true;
   }

   if ( (err >= eQMI_CALL_END_REASON_CD_GEN_OR_BUSY)
   &&   (err <= eQMI_CALL_END_REASON_HDR_RELEASED_BY_CM) )
   {
      return true;
   }

   return false;
};

/*=========================================================================*/
// eIPAddressType Enumeration
//    IPv4 or IPv6
/*=========================================================================*/
enum eIPAddressType
{
   eIP_ADDRESS_ENUM_BEGIN = -1,

   eIP_ADDRESS_V4 = 4,        // IPv4
   eIP_ADDRESS_V6 = 6,        // IPv6

   eIP_ADDRESS_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eIPAddressType validity check

PARAMETERS:
   type        [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eIPAddressType type )
{
   if (type == eIP_ADDRESS_V4 || type == eIP_ADDRESS_V6)
   {
      return true;
   }

   return false;
}

/*=========================================================================*/
// eSMSMessageMode Enumeration
//    CDMA or GW
/*=========================================================================*/
enum eSMSMessageMode
{
   eSMS_MESSAGE_MODE_BEGIN = -1,

   eSMS_MESSAGE_MODE_CDMA,    // 0 CDMA
   eSMS_MESSAGE_MODE_GW,      // 1 GW

   eSMS_MESSAGE_MODE_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eSMSMessageMode validity check

PARAMETERS:
   mode        [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eSMSMessageMode mode )
{
   if (mode == eSMS_MESSAGE_MODE_CDMA || mode == eSMS_MESSAGE_MODE_GW)
   {
      return true;
   }

   return false;
}

/*=========================================================================*/
// eNetworkMode Enumeration
//    UMTS or CDMA
/*=========================================================================*/
enum eNetworkMode
{
   eNETWORK_MODE_BEGIN = -1,

   eNETWORK_MODE_UMTS,           // 0 UMTS
   eNETWORK_MODE_CDMA,           // 1 CDMA

   eNETWORK_MODE_UNSET = 0xff,   // 0xff unknown/unset

   eNETWORK_MODE_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eNetworkMode validity check

PARAMETERS:
   mode        [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eNetworkMode mode )
{
   if (mode == eNETWORK_MODE_UMTS 
   ||  mode == eNETWORK_MODE_CDMA
   ||  mode == eNETWORK_MODE_UNSET)
   {
      return true;
   }

   return false;
}