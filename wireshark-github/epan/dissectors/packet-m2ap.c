/* Do not modify this file. Changes will be overwritten.                      */
/* Generated automatically by the ASN.1 to Wireshark dissector compiler       */
/* packet-m2ap.c                                                              */
/* asn2wrs.py -p m2ap -c ./m2ap.cnf -s ./packet-m2ap-template -D . -O ../.. M2AP-CommonDataTypes.asn M2AP-Constants.asn M2AP-Containers.asn M2AP-IEs.asn M2AP-PDU-Contents.asn M2AP-PDU-Descriptions.asn */

/* Input file: packet-m2ap-template.c */

#line 1 "./asn1/m2ap/packet-m2ap-template.c"
/* packet-m2ap.c
 * Routines for M2 Application Protocol packet dissection
 * Copyright 2016, Pascal Quantin <pascal.quantin@gmail.com>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Reference: 3GPP TS 36.443 v13.3.0
 */

#include "config.h"

#include <epan/packet.h>
#include <epan/sctpppids.h>
#include <epan/asn1.h>
#include <epan/expert.h>

#include "packet-per.h"
#include "packet-e212.h"

#define PNAME  "M2 Application Protocol"
#define PSNAME "M2AP"
#define PFNAME "m2ap"

void proto_register_m2ap(void);
void proto_reg_handoff_m2ap(void);

/* M2AP uses port 36443 as recommended by IANA. */
#define M2AP_PORT 36443


/*--- Included file: packet-m2ap-val.h ---*/
#line 1 "./asn1/m2ap/packet-m2ap-val.h"
#define maxPrivateIEs                  65535
#define maxProtocolExtensions          65535
#define maxProtocolIEs                 65535
#define maxnoofMBSFNareas              256
#define maxnoofMBSFN_Allocations       8
#define maxnoofPMCHsperMBSFNarea       15
#define maxnoofCells                   256
#define maxnoofMBMSServiceAreasPerCell 256
#define maxnoofSessionsPerPMCH         29
#define maxnooferrors                  256
#define maxNrOfIndividualM2ConnectionsToReset 256
#define maxnoofCountingService         16
#define maxnoofCellsforMBMS            4096

typedef enum _ProcedureCode_enum {
  id_sessionStart =   0,
  id_sessionStop =   1,
  id_mbmsSchedulingInformation =   2,
  id_errorIndication =   3,
  id_reset     =   4,
  id_m2Setup   =   5,
  id_eNBConfigurationUpdate =   6,
  id_mCEConfigurationUpdate =   7,
  id_privateMessage =   8,
  id_sessionUpdate =   9,
  id_mbmsServiceCounting =  10,
  id_mbmsServiceCountingResultsReport =  11,
  id_mbmsOverloadNotification =  12
} ProcedureCode_enum;

typedef enum _ProtocolIE_ID_enum {
  id_MCE_MBMS_M2AP_ID =   0,
  id_ENB_MBMS_M2AP_ID =   1,
  id_TMGI      =   2,
  id_MBMS_Session_ID =   3,
  id_MBMS_Service_Area =   6,
  id_TNL_Information =   7,
  id_CriticalityDiagnostics =   8,
  id_Cause     =   9,
  id_MBSFN_Area_Configuration_List =  10,
  id_PMCH_Configuration_List =  11,
  id_PMCH_Configuration_Item =  12,
  id_GlobalENB_ID =  13,
  id_ENBname   =  14,
  id_ENB_MBMS_Configuration_data_List =  15,
  id_ENB_MBMS_Configuration_data_Item =  16,
  id_GlobalMCE_ID =  17,
  id_MCEname   =  18,
  id_MCCHrelatedBCCH_ConfigPerMBSFNArea =  19,
  id_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item =  20,
  id_TimeToWait =  21,
  id_MBSFN_Subframe_Configuration_List =  22,
  id_MBSFN_Subframe_Configuration_Item =  23,
  id_Common_Subframe_Allocation_Period =  24,
  id_MCCH_Update_Time =  25,
  id_ENB_MBMS_Configuration_data_List_ConfigUpdate =  26,
  id_ENB_MBMS_Configuration_data_ConfigUpdate_Item =  27,
  id_MBMS_Service_associatedLogicalM2_ConnectionItem =  28,
  id_MBSFN_Area_ID =  29,
  id_ResetType =  30,
  id_MBMS_Service_associatedLogicalM2_ConnectionListResAck =  31,
  id_MBMS_Counting_Request_Session =  32,
  id_MBMS_Counting_Request_Session_Item =  33,
  id_MBMS_Counting_Result_List =  34,
  id_MBMS_Counting_Result_Item =  35,
  id_Modulation_Coding_Scheme2 =  36,
  id_MCH_Scheduling_PeriodExtended =  37,
  id_Alternative_TNL_Information =  38,
  id_Overload_Status_Per_PMCH_List =  39,
  id_PMCH_Overload_Status =  41,
  id_Active_MBMS_Session_List =  42,
  id_MBMS_Suspension_Notification_List =  43,
  id_MBMS_Suspension_Notification_Item =  44,
  id_SC_PTM_Information =  45
} ProtocolIE_ID_enum;

/*--- End of included file: packet-m2ap-val.h ---*/
#line 47 "./asn1/m2ap/packet-m2ap-template.c"

/* Initialize the protocol and registered fields */
static int proto_m2ap = -1;

static int hf_m2ap_IPAddress_v4 = -1;
static int hf_m2ap_IPAddress_v6 = -1;

/*--- Included file: packet-m2ap-hf.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-hf.c"
static int hf_m2ap_Cause_PDU = -1;                /* Cause */
static int hf_m2ap_CriticalityDiagnostics_PDU = -1;  /* CriticalityDiagnostics */
static int hf_m2ap_ENB_MBMS_Configuration_data_Item_PDU = -1;  /* ENB_MBMS_Configuration_data_Item */
static int hf_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item_PDU = -1;  /* ENB_MBMS_Configuration_data_ConfigUpdate_Item */
static int hf_m2ap_ENB_MBMS_M2AP_ID_PDU = -1;     /* ENB_MBMS_M2AP_ID */
static int hf_m2ap_ENBname_PDU = -1;              /* ENBname */
static int hf_m2ap_GlobalENB_ID_PDU = -1;         /* GlobalENB_ID */
static int hf_m2ap_GlobalMCE_ID_PDU = -1;         /* GlobalMCE_ID */
static int hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem_PDU = -1;  /* MBMS_Service_associatedLogicalM2_ConnectionItem */
static int hf_m2ap_MBMS_Service_Area_PDU = -1;    /* MBMS_Service_Area */
static int hf_m2ap_MBMS_Session_ID_PDU = -1;      /* MBMS_Session_ID */
static int hf_m2ap_MBSFN_Area_ID_PDU = -1;        /* MBSFN_Area_ID */
static int hf_m2ap_MBSFN_Subframe_Configuration_PDU = -1;  /* MBSFN_Subframe_Configuration */
static int hf_m2ap_MCCH_Update_Time_PDU = -1;     /* MCCH_Update_Time */
static int hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_PDU = -1;  /* MCCHrelatedBCCH_ConfigPerMBSFNArea_Item */
static int hf_m2ap_MCE_MBMS_M2AP_ID_PDU = -1;     /* MCE_MBMS_M2AP_ID */
static int hf_m2ap_MCEname_PDU = -1;              /* MCEname */
static int hf_m2ap_MCH_Scheduling_PeriodExtended_PDU = -1;  /* MCH_Scheduling_PeriodExtended */
static int hf_m2ap_Modulation_Coding_Scheme2_PDU = -1;  /* Modulation_Coding_Scheme2 */
static int hf_m2ap_Common_Subframe_Allocation_Period_PDU = -1;  /* Common_Subframe_Allocation_Period */
static int hf_m2ap_SC_PTM_Information_PDU = -1;   /* SC_PTM_Information */
static int hf_m2ap_TimeToWait_PDU = -1;           /* TimeToWait */
static int hf_m2ap_TMGI_PDU = -1;                 /* TMGI */
static int hf_m2ap_TNL_Information_PDU = -1;      /* TNL_Information */
static int hf_m2ap_SessionStartRequest_PDU = -1;  /* SessionStartRequest */
static int hf_m2ap_SessionStartResponse_PDU = -1;  /* SessionStartResponse */
static int hf_m2ap_SessionStartFailure_PDU = -1;  /* SessionStartFailure */
static int hf_m2ap_SessionStopRequest_PDU = -1;   /* SessionStopRequest */
static int hf_m2ap_SessionStopResponse_PDU = -1;  /* SessionStopResponse */
static int hf_m2ap_SessionUpdateRequest_PDU = -1;  /* SessionUpdateRequest */
static int hf_m2ap_SessionUpdateResponse_PDU = -1;  /* SessionUpdateResponse */
static int hf_m2ap_SessionUpdateFailure_PDU = -1;  /* SessionUpdateFailure */
static int hf_m2ap_MbmsSchedulingInformation_PDU = -1;  /* MbmsSchedulingInformation */
static int hf_m2ap_MBSFN_Area_Configuration_List_PDU = -1;  /* MBSFN_Area_Configuration_List */
static int hf_m2ap_PMCH_Configuration_List_PDU = -1;  /* PMCH_Configuration_List */
static int hf_m2ap_PMCH_Configuration_Item_PDU = -1;  /* PMCH_Configuration_Item */
static int hf_m2ap_MBSFN_Subframe_ConfigurationList_PDU = -1;  /* MBSFN_Subframe_ConfigurationList */
static int hf_m2ap_MBMS_Suspension_Notification_List_PDU = -1;  /* MBMS_Suspension_Notification_List */
static int hf_m2ap_MBMS_Suspension_Notification_Item_PDU = -1;  /* MBMS_Suspension_Notification_Item */
static int hf_m2ap_MbmsSchedulingInformationResponse_PDU = -1;  /* MbmsSchedulingInformationResponse */
static int hf_m2ap_M2SetupRequest_PDU = -1;       /* M2SetupRequest */
static int hf_m2ap_ENB_MBMS_Configuration_data_List_PDU = -1;  /* ENB_MBMS_Configuration_data_List */
static int hf_m2ap_M2SetupResponse_PDU = -1;      /* M2SetupResponse */
static int hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_PDU = -1;  /* MCCHrelatedBCCH_ConfigPerMBSFNArea */
static int hf_m2ap_M2SetupFailure_PDU = -1;       /* M2SetupFailure */
static int hf_m2ap_ENBConfigurationUpdate_PDU = -1;  /* ENBConfigurationUpdate */
static int hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_PDU = -1;  /* ENB_MBMS_Configuration_data_List_ConfigUpdate */
static int hf_m2ap_ENBConfigurationUpdateAcknowledge_PDU = -1;  /* ENBConfigurationUpdateAcknowledge */
static int hf_m2ap_ENBConfigurationUpdateFailure_PDU = -1;  /* ENBConfigurationUpdateFailure */
static int hf_m2ap_MCEConfigurationUpdate_PDU = -1;  /* MCEConfigurationUpdate */
static int hf_m2ap_MCEConfigurationUpdateAcknowledge_PDU = -1;  /* MCEConfigurationUpdateAcknowledge */
static int hf_m2ap_MCEConfigurationUpdateFailure_PDU = -1;  /* MCEConfigurationUpdateFailure */
static int hf_m2ap_ErrorIndication_PDU = -1;      /* ErrorIndication */
static int hf_m2ap_Reset_PDU = -1;                /* Reset */
static int hf_m2ap_ResetType_PDU = -1;            /* ResetType */
static int hf_m2ap_ResetAcknowledge_PDU = -1;     /* ResetAcknowledge */
static int hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_PDU = -1;  /* MBMS_Service_associatedLogicalM2_ConnectionListResAck */
static int hf_m2ap_PrivateMessage_PDU = -1;       /* PrivateMessage */
static int hf_m2ap_MbmsServiceCountingRequest_PDU = -1;  /* MbmsServiceCountingRequest */
static int hf_m2ap_MBMS_Counting_Request_Session_PDU = -1;  /* MBMS_Counting_Request_Session */
static int hf_m2ap_MBMS_Counting_Request_SessionIE_PDU = -1;  /* MBMS_Counting_Request_SessionIE */
static int hf_m2ap_MbmsServiceCountingResponse_PDU = -1;  /* MbmsServiceCountingResponse */
static int hf_m2ap_MbmsServiceCountingFailure_PDU = -1;  /* MbmsServiceCountingFailure */
static int hf_m2ap_MbmsServiceCountingResultsReport_PDU = -1;  /* MbmsServiceCountingResultsReport */
static int hf_m2ap_MBMS_Counting_Result_List_PDU = -1;  /* MBMS_Counting_Result_List */
static int hf_m2ap_MBMS_Counting_Result_PDU = -1;  /* MBMS_Counting_Result */
static int hf_m2ap_MbmsOverloadNotification_PDU = -1;  /* MbmsOverloadNotification */
static int hf_m2ap_Overload_Status_Per_PMCH_List_PDU = -1;  /* Overload_Status_Per_PMCH_List */
static int hf_m2ap_PMCH_Overload_Status_PDU = -1;  /* PMCH_Overload_Status */
static int hf_m2ap_Active_MBMS_Session_List_PDU = -1;  /* Active_MBMS_Session_List */
static int hf_m2ap_M2AP_PDU_PDU = -1;             /* M2AP_PDU */
static int hf_m2ap_local = -1;                    /* INTEGER_0_maxPrivateIEs */
static int hf_m2ap_global = -1;                   /* OBJECT_IDENTIFIER */
static int hf_m2ap_ProtocolIE_Container_item = -1;  /* ProtocolIE_Field */
static int hf_m2ap_id = -1;                       /* ProtocolIE_ID */
static int hf_m2ap_criticality = -1;              /* Criticality */
static int hf_m2ap_ie_field_value = -1;           /* T_ie_field_value */
static int hf_m2ap_ProtocolExtensionContainer_item = -1;  /* ProtocolExtensionField */
static int hf_m2ap_ext_id = -1;                   /* ProtocolIE_ID */
static int hf_m2ap_extensionValue = -1;           /* T_extensionValue */
static int hf_m2ap_PrivateIE_Container_item = -1;  /* PrivateIE_Field */
static int hf_m2ap_private_id = -1;               /* PrivateIE_ID */
static int hf_m2ap_private_value = -1;            /* T_private_value */
static int hf_m2ap_priorityLevel = -1;            /* PriorityLevel */
static int hf_m2ap_pre_emptionCapability = -1;    /* Pre_emptionCapability */
static int hf_m2ap_pre_emptionVulnerability = -1;  /* Pre_emptionVulnerability */
static int hf_m2ap_iE_Extensions = -1;            /* ProtocolExtensionContainer */
static int hf_m2ap_radioNetwork = -1;             /* CauseRadioNetwork */
static int hf_m2ap_transport = -1;                /* CauseTransport */
static int hf_m2ap_nAS = -1;                      /* CauseNAS */
static int hf_m2ap_protocol = -1;                 /* CauseProtocol */
static int hf_m2ap_misc = -1;                     /* CauseMisc */
static int hf_m2ap_eCGI = -1;                     /* ECGI */
static int hf_m2ap_cellReservationInfo = -1;      /* T_cellReservationInfo */
static int hf_m2ap_Cell_Information_List_item = -1;  /* Cell_Information */
static int hf_m2ap_procedureCode = -1;            /* ProcedureCode */
static int hf_m2ap_triggeringMessage = -1;        /* TriggeringMessage */
static int hf_m2ap_procedureCriticality = -1;     /* Criticality */
static int hf_m2ap_iEsCriticalityDiagnostics = -1;  /* CriticalityDiagnostics_IE_List */
static int hf_m2ap_CriticalityDiagnostics_IE_List_item = -1;  /* CriticalityDiagnostics_IE_List_item */
static int hf_m2ap_iECriticality = -1;            /* Criticality */
static int hf_m2ap_iE_ID = -1;                    /* ProtocolIE_ID */
static int hf_m2ap_typeOfError = -1;              /* TypeOfError */
static int hf_m2ap_pLMN_Identity = -1;            /* PLMN_Identity */
static int hf_m2ap_eUTRANcellIdentifier = -1;     /* EUTRANCellIdentifier */
static int hf_m2ap_macro_eNB_ID = -1;             /* BIT_STRING_SIZE_20 */
static int hf_m2ap_mbsfnSynchronisationArea = -1;  /* MBSFN_SynchronisationArea_ID */
static int hf_m2ap_mbmsServiceAreaList = -1;      /* MBMS_Service_Area_ID_List */
static int hf_m2ap_mBMSConfigData = -1;           /* ENB_MBMS_Configuration_data_Item */
static int hf_m2ap_mBMS_E_RAB_MaximumBitrateDL = -1;  /* BitRate */
static int hf_m2ap_mBMS_E_RAB_GuaranteedBitrateDL = -1;  /* BitRate */
static int hf_m2ap_eNB_ID = -1;                   /* ENB_ID */
static int hf_m2ap_mCE_ID = -1;                   /* MCE_ID */
static int hf_m2ap_MBMS_Cell_List_item = -1;      /* ECGI */
static int hf_m2ap_qCI = -1;                      /* QCI */
static int hf_m2ap_gbrQosInformation = -1;        /* GBR_QosInformation */
static int hf_m2ap_allocationAndRetentionPriority = -1;  /* AllocationAndRetentionPriority */
static int hf_m2ap_eNB_MBMS_M2AP_ID = -1;         /* ENB_MBMS_M2AP_ID */
static int hf_m2ap_mCE_MBMS_M2AP_ID = -1;         /* MCE_MBMS_M2AP_ID */
static int hf_m2ap_MBMS_Service_Area_ID_List_item = -1;  /* MBMS_Service_Area */
static int hf_m2ap_MBMSsessionListPerPMCH_Item_item = -1;  /* MBMSsessionListPerPMCH_Item_item */
static int hf_m2ap_tmgi = -1;                     /* TMGI */
static int hf_m2ap_lcid = -1;                     /* LCID */
static int hf_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item = -1;  /* MBMSsessionsToBeSuspendedListPerPMCH_Item_item */
static int hf_m2ap_radioframeAllocationPeriod = -1;  /* T_radioframeAllocationPeriod */
static int hf_m2ap_radioframeAllocationOffset = -1;  /* INTEGER_0_7 */
static int hf_m2ap_subframeAllocation = -1;       /* T_subframeAllocation */
static int hf_m2ap_oneFrame = -1;                 /* BIT_STRING_SIZE_6 */
static int hf_m2ap_fourFrames = -1;               /* BIT_STRING_SIZE_24 */
static int hf_m2ap_mbsfnArea = -1;                /* MBSFN_Area_ID */
static int hf_m2ap_pdcchLength = -1;              /* T_pdcchLength */
static int hf_m2ap_repetitionPeriod = -1;         /* T_repetitionPeriod */
static int hf_m2ap_offset = -1;                   /* INTEGER_0_10 */
static int hf_m2ap_modificationPeriod = -1;       /* T_modificationPeriod */
static int hf_m2ap_subframeAllocationInfo = -1;   /* BIT_STRING_SIZE_6 */
static int hf_m2ap_modulationAndCodingScheme = -1;  /* T_modulationAndCodingScheme */
static int hf_m2ap_cellInformationList = -1;      /* Cell_Information_List */
static int hf_m2ap_allocatedSubframesEnd = -1;    /* AllocatedSubframesEnd */
static int hf_m2ap_dataMCS = -1;                  /* INTEGER_0_28 */
static int hf_m2ap_mchSchedulingPeriod = -1;      /* MCH_Scheduling_Period */
static int hf_m2ap_mbmsCellList = -1;             /* MBMS_Cell_List */
static int hf_m2ap_mbms_E_RAB_QoS_Parameters = -1;  /* MBMS_E_RAB_QoS_Parameters */
static int hf_m2ap_pLMNidentity = -1;             /* PLMN_Identity */
static int hf_m2ap_serviceID = -1;                /* OCTET_STRING_SIZE_3 */
static int hf_m2ap_iPMCAddress = -1;              /* IPAddress */
static int hf_m2ap_iPSourceAddress = -1;          /* IPAddress */
static int hf_m2ap_gTP_TEID = -1;                 /* GTP_TEID */
static int hf_m2ap_protocolIEs = -1;              /* ProtocolIE_Container */
static int hf_m2ap_MBSFN_Area_Configuration_List_item = -1;  /* ProtocolIE_Container */
static int hf_m2ap_PMCH_Configuration_List_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_pmch_Configuration = -1;       /* PMCH_Configuration */
static int hf_m2ap_mbms_Session_List = -1;        /* MBMSsessionListPerPMCH_Item */
static int hf_m2ap_MBSFN_Subframe_ConfigurationList_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_MBMS_Suspension_Notification_List_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_sfn = -1;                      /* SFN */
static int hf_m2ap_mbms_Sessions_To_Be_Suspended_List = -1;  /* MBMSsessionsToBeSuspendedListPerPMCH_Item */
static int hf_m2ap_ENB_MBMS_Configuration_data_List_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_m2_Interface = -1;             /* ResetAll */
static int hf_m2ap_partOfM2_Interface = -1;       /* MBMS_Service_associatedLogicalM2_ConnectionListRes */
static int hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_item = -1;  /* ProtocolIE_Single_Container */
static int hf_m2ap_privateIEs = -1;               /* PrivateIE_Container */
static int hf_m2ap_MBMS_Counting_Request_Session_item = -1;  /* ProtocolIE_Container */
static int hf_m2ap_MBMS_Counting_Result_List_item = -1;  /* ProtocolIE_Container */
static int hf_m2ap_countingResult = -1;           /* CountingResult */
static int hf_m2ap_Overload_Status_Per_PMCH_List_item = -1;  /* ProtocolIE_Container */
static int hf_m2ap_Active_MBMS_Session_List_item = -1;  /* ProtocolIE_Container */
static int hf_m2ap_initiatingMessage = -1;        /* InitiatingMessage */
static int hf_m2ap_successfulOutcome = -1;        /* SuccessfulOutcome */
static int hf_m2ap_unsuccessfulOutcome = -1;      /* UnsuccessfulOutcome */
static int hf_m2ap_initiatingMessage_value = -1;  /* InitiatingMessage_value */
static int hf_m2ap_successfulOutcome_value = -1;  /* SuccessfulOutcome_value */
static int hf_m2ap_unsuccessfulOutcome_value = -1;  /* UnsuccessfulOutcome_value */

/*--- End of included file: packet-m2ap-hf.c ---*/
#line 54 "./asn1/m2ap/packet-m2ap-template.c"

/* Initialize the subtree pointers */
static int ett_m2ap = -1;
static int ett_m2ap_PLMN_Identity = -1;
static int ett_m2ap_IPAddress = -1;

/*--- Included file: packet-m2ap-ett.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-ett.c"
static gint ett_m2ap_PrivateIE_ID = -1;
static gint ett_m2ap_ProtocolIE_Container = -1;
static gint ett_m2ap_ProtocolIE_Field = -1;
static gint ett_m2ap_ProtocolExtensionContainer = -1;
static gint ett_m2ap_ProtocolExtensionField = -1;
static gint ett_m2ap_PrivateIE_Container = -1;
static gint ett_m2ap_PrivateIE_Field = -1;
static gint ett_m2ap_AllocationAndRetentionPriority = -1;
static gint ett_m2ap_Cause = -1;
static gint ett_m2ap_Cell_Information = -1;
static gint ett_m2ap_Cell_Information_List = -1;
static gint ett_m2ap_CriticalityDiagnostics = -1;
static gint ett_m2ap_CriticalityDiagnostics_IE_List = -1;
static gint ett_m2ap_CriticalityDiagnostics_IE_List_item = -1;
static gint ett_m2ap_ECGI = -1;
static gint ett_m2ap_ENB_ID = -1;
static gint ett_m2ap_ENB_MBMS_Configuration_data_Item = -1;
static gint ett_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item = -1;
static gint ett_m2ap_GBR_QosInformation = -1;
static gint ett_m2ap_GlobalENB_ID = -1;
static gint ett_m2ap_GlobalMCE_ID = -1;
static gint ett_m2ap_MBMS_Cell_List = -1;
static gint ett_m2ap_MBMS_E_RAB_QoS_Parameters = -1;
static gint ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem = -1;
static gint ett_m2ap_MBMS_Service_Area_ID_List = -1;
static gint ett_m2ap_MBMSsessionListPerPMCH_Item = -1;
static gint ett_m2ap_MBMSsessionListPerPMCH_Item_item = -1;
static gint ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item = -1;
static gint ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item = -1;
static gint ett_m2ap_MBSFN_Subframe_Configuration = -1;
static gint ett_m2ap_T_subframeAllocation = -1;
static gint ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item = -1;
static gint ett_m2ap_PMCH_Configuration = -1;
static gint ett_m2ap_SC_PTM_Information = -1;
static gint ett_m2ap_TMGI = -1;
static gint ett_m2ap_TNL_Information = -1;
static gint ett_m2ap_SessionStartRequest = -1;
static gint ett_m2ap_SessionStartResponse = -1;
static gint ett_m2ap_SessionStartFailure = -1;
static gint ett_m2ap_SessionStopRequest = -1;
static gint ett_m2ap_SessionStopResponse = -1;
static gint ett_m2ap_SessionUpdateRequest = -1;
static gint ett_m2ap_SessionUpdateResponse = -1;
static gint ett_m2ap_SessionUpdateFailure = -1;
static gint ett_m2ap_MbmsSchedulingInformation = -1;
static gint ett_m2ap_MBSFN_Area_Configuration_List = -1;
static gint ett_m2ap_PMCH_Configuration_List = -1;
static gint ett_m2ap_PMCH_Configuration_Item = -1;
static gint ett_m2ap_MBSFN_Subframe_ConfigurationList = -1;
static gint ett_m2ap_MBMS_Suspension_Notification_List = -1;
static gint ett_m2ap_MBMS_Suspension_Notification_Item = -1;
static gint ett_m2ap_MbmsSchedulingInformationResponse = -1;
static gint ett_m2ap_M2SetupRequest = -1;
static gint ett_m2ap_ENB_MBMS_Configuration_data_List = -1;
static gint ett_m2ap_M2SetupResponse = -1;
static gint ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea = -1;
static gint ett_m2ap_M2SetupFailure = -1;
static gint ett_m2ap_ENBConfigurationUpdate = -1;
static gint ett_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate = -1;
static gint ett_m2ap_ENBConfigurationUpdateAcknowledge = -1;
static gint ett_m2ap_ENBConfigurationUpdateFailure = -1;
static gint ett_m2ap_MCEConfigurationUpdate = -1;
static gint ett_m2ap_MCEConfigurationUpdateAcknowledge = -1;
static gint ett_m2ap_MCEConfigurationUpdateFailure = -1;
static gint ett_m2ap_ErrorIndication = -1;
static gint ett_m2ap_Reset = -1;
static gint ett_m2ap_ResetType = -1;
static gint ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes = -1;
static gint ett_m2ap_ResetAcknowledge = -1;
static gint ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck = -1;
static gint ett_m2ap_PrivateMessage = -1;
static gint ett_m2ap_MbmsServiceCountingRequest = -1;
static gint ett_m2ap_MBMS_Counting_Request_Session = -1;
static gint ett_m2ap_MBMS_Counting_Request_SessionIE = -1;
static gint ett_m2ap_MbmsServiceCountingResponse = -1;
static gint ett_m2ap_MbmsServiceCountingFailure = -1;
static gint ett_m2ap_MbmsServiceCountingResultsReport = -1;
static gint ett_m2ap_MBMS_Counting_Result_List = -1;
static gint ett_m2ap_MBMS_Counting_Result = -1;
static gint ett_m2ap_MbmsOverloadNotification = -1;
static gint ett_m2ap_Overload_Status_Per_PMCH_List = -1;
static gint ett_m2ap_Active_MBMS_Session_List = -1;
static gint ett_m2ap_M2AP_PDU = -1;
static gint ett_m2ap_InitiatingMessage = -1;
static gint ett_m2ap_SuccessfulOutcome = -1;
static gint ett_m2ap_UnsuccessfulOutcome = -1;

/*--- End of included file: packet-m2ap-ett.c ---*/
#line 60 "./asn1/m2ap/packet-m2ap-template.c"

static expert_field ei_m2ap_invalid_ip_address_len = EI_INIT;

enum{
  INITIATING_MESSAGE,
  SUCCESSFUL_OUTCOME,
  UNSUCCESSFUL_OUTCOME
};

/* Global variables */
static guint32 ProcedureCode;
static guint32 ProtocolIE_ID;
static guint32 message_type;
static dissector_handle_t m2ap_handle;

/* Dissector tables */
static dissector_table_t m2ap_ies_dissector_table;
static dissector_table_t m2ap_extension_dissector_table;
static dissector_table_t m2ap_proc_imsg_dissector_table;
static dissector_table_t m2ap_proc_sout_dissector_table;
static dissector_table_t m2ap_proc_uout_dissector_table;

static int dissect_ProtocolIEFieldValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_ProtocolExtensionFieldExtensionValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_InitiatingMessageValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_SuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_UnsuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);


/*--- Included file: packet-m2ap-fn.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-fn.c"

static const value_string m2ap_Criticality_vals[] = {
  {   0, "reject" },
  {   1, "ignore" },
  {   2, "notify" },
  { 0, NULL }
};


static int
dissect_m2ap_Criticality(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_m2ap_INTEGER_0_maxPrivateIEs(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, maxPrivateIEs, NULL, FALSE);

  return offset;
}



static int
dissect_m2ap_OBJECT_IDENTIFIER(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_object_identifier(tvb, offset, actx, tree, hf_index, NULL);

  return offset;
}


static const value_string m2ap_PrivateIE_ID_vals[] = {
  {   0, "local" },
  {   1, "global" },
  { 0, NULL }
};

static const per_choice_t PrivateIE_ID_choice[] = {
  {   0, &hf_m2ap_local          , ASN1_NO_EXTENSIONS     , dissect_m2ap_INTEGER_0_maxPrivateIEs },
  {   1, &hf_m2ap_global         , ASN1_NO_EXTENSIONS     , dissect_m2ap_OBJECT_IDENTIFIER },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_PrivateIE_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_PrivateIE_ID, PrivateIE_ID_choice,
                                 NULL);

  return offset;
}


static const value_string m2ap_ProcedureCode_vals[] = {
  { id_sessionStart, "id-sessionStart" },
  { id_sessionStop, "id-sessionStop" },
  { id_mbmsSchedulingInformation, "id-mbmsSchedulingInformation" },
  { id_errorIndication, "id-errorIndication" },
  { id_reset, "id-reset" },
  { id_m2Setup, "id-m2Setup" },
  { id_eNBConfigurationUpdate, "id-eNBConfigurationUpdate" },
  { id_mCEConfigurationUpdate, "id-mCEConfigurationUpdate" },
  { id_privateMessage, "id-privateMessage" },
  { id_sessionUpdate, "id-sessionUpdate" },
  { id_mbmsServiceCounting, "id-mbmsServiceCounting" },
  { id_mbmsServiceCountingResultsReport, "id-mbmsServiceCountingResultsReport" },
  { id_mbmsOverloadNotification, "id-mbmsOverloadNotification" },
  { 0, NULL }
};

static value_string_ext m2ap_ProcedureCode_vals_ext = VALUE_STRING_EXT_INIT(m2ap_ProcedureCode_vals);


static int
dissect_m2ap_ProcedureCode(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 255U, &ProcedureCode, FALSE);

  return offset;
}


static const value_string m2ap_ProtocolIE_ID_vals[] = {
  { id_MCE_MBMS_M2AP_ID, "id-MCE-MBMS-M2AP-ID" },
  { id_ENB_MBMS_M2AP_ID, "id-ENB-MBMS-M2AP-ID" },
  { id_TMGI, "id-TMGI" },
  { id_MBMS_Session_ID, "id-MBMS-Session-ID" },
  { id_MBMS_Service_Area, "id-MBMS-Service-Area" },
  { id_TNL_Information, "id-TNL-Information" },
  { id_CriticalityDiagnostics, "id-CriticalityDiagnostics" },
  { id_Cause, "id-Cause" },
  { id_MBSFN_Area_Configuration_List, "id-MBSFN-Area-Configuration-List" },
  { id_PMCH_Configuration_List, "id-PMCH-Configuration-List" },
  { id_PMCH_Configuration_Item, "id-PMCH-Configuration-Item" },
  { id_GlobalENB_ID, "id-GlobalENB-ID" },
  { id_ENBname, "id-ENBname" },
  { id_ENB_MBMS_Configuration_data_List, "id-ENB-MBMS-Configuration-data-List" },
  { id_ENB_MBMS_Configuration_data_Item, "id-ENB-MBMS-Configuration-data-Item" },
  { id_GlobalMCE_ID, "id-GlobalMCE-ID" },
  { id_MCEname, "id-MCEname" },
  { id_MCCHrelatedBCCH_ConfigPerMBSFNArea, "id-MCCHrelatedBCCH-ConfigPerMBSFNArea" },
  { id_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item, "id-MCCHrelatedBCCH-ConfigPerMBSFNArea-Item" },
  { id_TimeToWait, "id-TimeToWait" },
  { id_MBSFN_Subframe_Configuration_List, "id-MBSFN-Subframe-Configuration-List" },
  { id_MBSFN_Subframe_Configuration_Item, "id-MBSFN-Subframe-Configuration-Item" },
  { id_Common_Subframe_Allocation_Period, "id-Common-Subframe-Allocation-Period" },
  { id_MCCH_Update_Time, "id-MCCH-Update-Time" },
  { id_ENB_MBMS_Configuration_data_List_ConfigUpdate, "id-ENB-MBMS-Configuration-data-List-ConfigUpdate" },
  { id_ENB_MBMS_Configuration_data_ConfigUpdate_Item, "id-ENB-MBMS-Configuration-data-ConfigUpdate-Item" },
  { id_MBMS_Service_associatedLogicalM2_ConnectionItem, "id-MBMS-Service-associatedLogicalM2-ConnectionItem" },
  { id_MBSFN_Area_ID, "id-MBSFN-Area-ID" },
  { id_ResetType, "id-ResetType" },
  { id_MBMS_Service_associatedLogicalM2_ConnectionListResAck, "id-MBMS-Service-associatedLogicalM2-ConnectionListResAck" },
  { id_MBMS_Counting_Request_Session, "id-MBMS-Counting-Request-Session" },
  { id_MBMS_Counting_Request_Session_Item, "id-MBMS-Counting-Request-Session-Item" },
  { id_MBMS_Counting_Result_List, "id-MBMS-Counting-Result-List" },
  { id_MBMS_Counting_Result_Item, "id-MBMS-Counting-Result-Item" },
  { id_Modulation_Coding_Scheme2, "id-Modulation-Coding-Scheme2" },
  { id_MCH_Scheduling_PeriodExtended, "id-MCH-Scheduling-PeriodExtended" },
  { id_Alternative_TNL_Information, "id-Alternative-TNL-Information" },
  { id_Overload_Status_Per_PMCH_List, "id-Overload-Status-Per-PMCH-List" },
  { id_PMCH_Overload_Status, "id-PMCH-Overload-Status" },
  { id_Active_MBMS_Session_List, "id-Active-MBMS-Session-List" },
  { id_MBMS_Suspension_Notification_List, "id-MBMS-Suspension-Notification-List" },
  { id_MBMS_Suspension_Notification_Item, "id-MBMS-Suspension-Notification-Item" },
  { id_SC_PTM_Information, "id-SC-PTM-Information" },
  { 0, NULL }
};

static value_string_ext m2ap_ProtocolIE_ID_vals_ext = VALUE_STRING_EXT_INIT(m2ap_ProtocolIE_ID_vals);


static int
dissect_m2ap_ProtocolIE_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, maxProtocolIEs, &ProtocolIE_ID, FALSE);

#line 47 "./asn1/m2ap/m2ap.cnf"
  if (tree) {
    proto_item_append_text(proto_item_get_parent_nth(actx->created_item, 2), ": %s", val_to_str_ext(ProtocolIE_ID, &m2ap_ProtocolIE_ID_vals_ext, "unknown (%d)"));
  }

  return offset;
}


static const value_string m2ap_TriggeringMessage_vals[] = {
  {   0, "initiating-message" },
  {   1, "successful-outcome" },
  {   2, "unsuccessful-outcome" },
  { 0, NULL }
};


static int
dissect_m2ap_TriggeringMessage(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_m2ap_T_ie_field_value(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_open_type_pdu_new(tvb, offset, actx, tree, hf_index, dissect_ProtocolIEFieldValue);

  return offset;
}


static const per_sequence_t ProtocolIE_Field_sequence[] = {
  { &hf_m2ap_id             , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_ID },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_ie_field_value , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_T_ie_field_value },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ProtocolIE_Field(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ProtocolIE_Field, ProtocolIE_Field_sequence);

  return offset;
}


static const per_sequence_t ProtocolIE_Container_sequence_of[1] = {
  { &hf_m2ap_ProtocolIE_Container_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Field },
};

static int
dissect_m2ap_ProtocolIE_Container(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_ProtocolIE_Container, ProtocolIE_Container_sequence_of,
                                                  0, maxProtocolIEs, FALSE);

  return offset;
}



static int
dissect_m2ap_ProtocolIE_Single_Container(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_m2ap_ProtocolIE_Field(tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_m2ap_T_extensionValue(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_open_type_pdu_new(tvb, offset, actx, tree, hf_index, dissect_ProtocolExtensionFieldExtensionValue);

  return offset;
}


static const per_sequence_t ProtocolExtensionField_sequence[] = {
  { &hf_m2ap_ext_id         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_ID },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_extensionValue , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_T_extensionValue },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ProtocolExtensionField(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ProtocolExtensionField, ProtocolExtensionField_sequence);

  return offset;
}


static const per_sequence_t ProtocolExtensionContainer_sequence_of[1] = {
  { &hf_m2ap_ProtocolExtensionContainer_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolExtensionField },
};

static int
dissect_m2ap_ProtocolExtensionContainer(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_ProtocolExtensionContainer, ProtocolExtensionContainer_sequence_of,
                                                  1, maxProtocolExtensions, FALSE);

  return offset;
}



static int
dissect_m2ap_T_private_value(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_open_type(tvb, offset, actx, tree, hf_index, NULL);

  return offset;
}


static const per_sequence_t PrivateIE_Field_sequence[] = {
  { &hf_m2ap_private_id     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_PrivateIE_ID },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_private_value  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_T_private_value },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_PrivateIE_Field(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_PrivateIE_Field, PrivateIE_Field_sequence);

  return offset;
}


static const per_sequence_t PrivateIE_Container_sequence_of[1] = {
  { &hf_m2ap_PrivateIE_Container_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_PrivateIE_Field },
};

static int
dissect_m2ap_PrivateIE_Container(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_PrivateIE_Container, PrivateIE_Container_sequence_of,
                                                  1, maxPrivateIEs, FALSE);

  return offset;
}



static int
dissect_m2ap_AllocatedSubframesEnd(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 1535U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_PriorityLevel_vals[] = {
  {   0, "spare" },
  {   1, "highest" },
  {  14, "lowest" },
  {  15, "no-priority" },
  { 0, NULL }
};


static int
dissect_m2ap_PriorityLevel(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 15U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_Pre_emptionCapability_vals[] = {
  {   0, "shall-not-trigger-pre-emption" },
  {   1, "may-trigger-pre-emption" },
  { 0, NULL }
};


static int
dissect_m2ap_Pre_emptionCapability(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string m2ap_Pre_emptionVulnerability_vals[] = {
  {   0, "not-pre-emptable" },
  {   1, "pre-emptable" },
  { 0, NULL }
};


static int
dissect_m2ap_Pre_emptionVulnerability(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t AllocationAndRetentionPriority_sequence[] = {
  { &hf_m2ap_priorityLevel  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_PriorityLevel },
  { &hf_m2ap_pre_emptionCapability, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Pre_emptionCapability },
  { &hf_m2ap_pre_emptionVulnerability, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Pre_emptionVulnerability },
  { &hf_m2ap_iE_Extensions  , ASN1_NO_EXTENSIONS     , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_AllocationAndRetentionPriority(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_AllocationAndRetentionPriority, AllocationAndRetentionPriority_sequence);

  return offset;
}



static int
dissect_m2ap_BitRate(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer_64b(tvb, offset, actx, tree, hf_index,
                                                            0U, G_GUINT64_CONSTANT(10000000000), NULL, FALSE);

  return offset;
}


static const value_string m2ap_CauseRadioNetwork_vals[] = {
  {   0, "unknown-or-already-allocated-MCE-MBMS-M2AP-ID" },
  {   1, "unknown-or-already-allocated-eNB-MBMS-M2AP-ID" },
  {   2, "unknown-or-inconsistent-pair-of-MBMS-M2AP-IDs" },
  {   3, "radio-resources-not-available" },
  {   4, "interaction-with-other-procedure" },
  {   5, "unspecified" },
  {   6, "invalid-QoS-combination" },
  {   7, "not-supported-QCI-value" },
  { 0, NULL }
};


static int
dissect_m2ap_CauseRadioNetwork(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     6, NULL, TRUE, 2, NULL);

  return offset;
}


static const value_string m2ap_CauseTransport_vals[] = {
  {   0, "transport-resource-unavailable" },
  {   1, "unspecified" },
  { 0, NULL }
};


static int
dissect_m2ap_CauseTransport(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, TRUE, 0, NULL);

  return offset;
}


static const value_string m2ap_CauseNAS_vals[] = {
  {   0, "unspecified" },
  { 0, NULL }
};


static int
dissect_m2ap_CauseNAS(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     1, NULL, TRUE, 0, NULL);

  return offset;
}


static const value_string m2ap_CauseProtocol_vals[] = {
  {   0, "transfer-syntax-error" },
  {   1, "abstract-syntax-error-reject" },
  {   2, "abstract-syntax-error-ignore-and-notify" },
  {   3, "message-not-compatible-with-receiver-state" },
  {   4, "semantic-error" },
  {   5, "abstract-syntax-error-falsely-constructed-message" },
  {   6, "unspecified" },
  { 0, NULL }
};


static int
dissect_m2ap_CauseProtocol(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     7, NULL, TRUE, 0, NULL);

  return offset;
}


static const value_string m2ap_CauseMisc_vals[] = {
  {   0, "control-processing-overload" },
  {   1, "hardware-failure" },
  {   2, "om-intervention" },
  {   3, "unspecified" },
  { 0, NULL }
};


static int
dissect_m2ap_CauseMisc(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, TRUE, 0, NULL);

  return offset;
}


static const value_string m2ap_Cause_vals[] = {
  {   0, "radioNetwork" },
  {   1, "transport" },
  {   2, "nAS" },
  {   3, "protocol" },
  {   4, "misc" },
  { 0, NULL }
};

static const per_choice_t Cause_choice[] = {
  {   0, &hf_m2ap_radioNetwork   , ASN1_EXTENSION_ROOT    , dissect_m2ap_CauseRadioNetwork },
  {   1, &hf_m2ap_transport      , ASN1_EXTENSION_ROOT    , dissect_m2ap_CauseTransport },
  {   2, &hf_m2ap_nAS            , ASN1_EXTENSION_ROOT    , dissect_m2ap_CauseNAS },
  {   3, &hf_m2ap_protocol       , ASN1_EXTENSION_ROOT    , dissect_m2ap_CauseProtocol },
  {   4, &hf_m2ap_misc           , ASN1_EXTENSION_ROOT    , dissect_m2ap_CauseMisc },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_Cause(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_Cause, Cause_choice,
                                 NULL);

  return offset;
}



static int
dissect_m2ap_PLMN_Identity(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 70 "./asn1/m2ap/m2ap.cnf"
  tvbuff_t *parameter_tvb = NULL;
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       3, 3, FALSE, &parameter_tvb);

  if (parameter_tvb) {
    proto_tree *subtree = proto_item_add_subtree(actx->created_item, ett_m2ap_PLMN_Identity);
    dissect_e212_mcc_mnc(parameter_tvb, actx->pinfo, subtree, 0, E212_NONE, FALSE);
  }


  return offset;
}



static int
dissect_m2ap_EUTRANCellIdentifier(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_bit_string(tvb, offset, actx, tree, hf_index,
                                     28, 28, FALSE, NULL, NULL);

  return offset;
}


static const per_sequence_t ECGI_sequence[] = {
  { &hf_m2ap_pLMN_Identity  , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PLMN_Identity },
  { &hf_m2ap_eUTRANcellIdentifier, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_EUTRANCellIdentifier },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ECGI(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ECGI, ECGI_sequence);

  return offset;
}


static const value_string m2ap_T_cellReservationInfo_vals[] = {
  {   0, "reservedCell" },
  {   1, "nonReservedCell" },
  { 0, NULL }
};


static int
dissect_m2ap_T_cellReservationInfo(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, TRUE, 0, NULL);

  return offset;
}


static const per_sequence_t Cell_Information_sequence[] = {
  { &hf_m2ap_eCGI           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ECGI },
  { &hf_m2ap_cellReservationInfo, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_cellReservationInfo },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_Cell_Information(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_Cell_Information, Cell_Information_sequence);

  return offset;
}


static const per_sequence_t Cell_Information_List_sequence_of[1] = {
  { &hf_m2ap_Cell_Information_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Cell_Information },
};

static int
dissect_m2ap_Cell_Information_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_Cell_Information_List, Cell_Information_List_sequence_of,
                                                  1, maxnoofCells, FALSE);

  return offset;
}


static const value_string m2ap_TypeOfError_vals[] = {
  {   0, "not-understood" },
  {   1, "missing" },
  { 0, NULL }
};


static int
dissect_m2ap_TypeOfError(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, TRUE, 0, NULL);

  return offset;
}


static const per_sequence_t CriticalityDiagnostics_IE_List_item_sequence[] = {
  { &hf_m2ap_iECriticality  , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_iE_ID          , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_ID },
  { &hf_m2ap_typeOfError    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_TypeOfError },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_CriticalityDiagnostics_IE_List_item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_CriticalityDiagnostics_IE_List_item, CriticalityDiagnostics_IE_List_item_sequence);

  return offset;
}


static const per_sequence_t CriticalityDiagnostics_IE_List_sequence_of[1] = {
  { &hf_m2ap_CriticalityDiagnostics_IE_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_CriticalityDiagnostics_IE_List_item },
};

static int
dissect_m2ap_CriticalityDiagnostics_IE_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_CriticalityDiagnostics_IE_List, CriticalityDiagnostics_IE_List_sequence_of,
                                                  1, maxnooferrors, FALSE);

  return offset;
}


static const per_sequence_t CriticalityDiagnostics_sequence[] = {
  { &hf_m2ap_procedureCode  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProcedureCode },
  { &hf_m2ap_triggeringMessage, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_TriggeringMessage },
  { &hf_m2ap_procedureCriticality, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_Criticality },
  { &hf_m2ap_iEsCriticalityDiagnostics, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_CriticalityDiagnostics_IE_List },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_CriticalityDiagnostics(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_CriticalityDiagnostics, CriticalityDiagnostics_sequence);

  return offset;
}



static int
dissect_m2ap_BIT_STRING_SIZE_20(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_bit_string(tvb, offset, actx, tree, hf_index,
                                     20, 20, FALSE, NULL, NULL);

  return offset;
}


static const value_string m2ap_ENB_ID_vals[] = {
  {   0, "macro-eNB-ID" },
  { 0, NULL }
};

static const per_choice_t ENB_ID_choice[] = {
  {   0, &hf_m2ap_macro_eNB_ID   , ASN1_EXTENSION_ROOT    , dissect_m2ap_BIT_STRING_SIZE_20 },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_ENB_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_ENB_ID, ENB_ID_choice,
                                 NULL);

  return offset;
}



static int
dissect_m2ap_MBSFN_SynchronisationArea_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 65535U, NULL, FALSE);

  return offset;
}



static int
dissect_m2ap_MBMS_Service_Area(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       NO_BOUND, NO_BOUND, FALSE, NULL);

  return offset;
}


static const per_sequence_t MBMS_Service_Area_ID_List_sequence_of[1] = {
  { &hf_m2ap_MBMS_Service_Area_ID_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMS_Service_Area },
};

static int
dissect_m2ap_MBMS_Service_Area_ID_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Service_Area_ID_List, MBMS_Service_Area_ID_List_sequence_of,
                                                  1, maxnoofMBMSServiceAreasPerCell, FALSE);

  return offset;
}


static const per_sequence_t ENB_MBMS_Configuration_data_Item_sequence[] = {
  { &hf_m2ap_eCGI           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ECGI },
  { &hf_m2ap_mbsfnSynchronisationArea, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBSFN_SynchronisationArea_ID },
  { &hf_m2ap_mbmsServiceAreaList, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMS_Service_Area_ID_List },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ENB_MBMS_Configuration_data_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ENB_MBMS_Configuration_data_Item, ENB_MBMS_Configuration_data_Item_sequence);

  return offset;
}


static const value_string m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item_vals[] = {
  {   0, "mBMSConfigData" },
  {   1, "eCGI" },
  { 0, NULL }
};

static const per_choice_t ENB_MBMS_Configuration_data_ConfigUpdate_Item_choice[] = {
  {   0, &hf_m2ap_mBMSConfigData , ASN1_EXTENSION_ROOT    , dissect_m2ap_ENB_MBMS_Configuration_data_Item },
  {   1, &hf_m2ap_eCGI           , ASN1_EXTENSION_ROOT    , dissect_m2ap_ECGI },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item, ENB_MBMS_Configuration_data_ConfigUpdate_Item_choice,
                                 NULL);

  return offset;
}



static int
dissect_m2ap_ENB_MBMS_M2AP_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 65535U, NULL, FALSE);

  return offset;
}



static int
dissect_m2ap_ENBname(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 150, TRUE);

  return offset;
}


static const per_sequence_t GBR_QosInformation_sequence[] = {
  { &hf_m2ap_mBMS_E_RAB_MaximumBitrateDL, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_BitRate },
  { &hf_m2ap_mBMS_E_RAB_GuaranteedBitrateDL, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_BitRate },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_GBR_QosInformation(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_GBR_QosInformation, GBR_QosInformation_sequence);

  return offset;
}


static const per_sequence_t GlobalENB_ID_sequence[] = {
  { &hf_m2ap_pLMN_Identity  , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PLMN_Identity },
  { &hf_m2ap_eNB_ID         , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ENB_ID },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_GlobalENB_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_GlobalENB_ID, GlobalENB_ID_sequence);

  return offset;
}



static int
dissect_m2ap_MCE_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       2, 2, FALSE, NULL);

  return offset;
}


static const per_sequence_t GlobalMCE_ID_sequence[] = {
  { &hf_m2ap_pLMN_Identity  , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PLMN_Identity },
  { &hf_m2ap_mCE_ID         , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MCE_ID },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_GlobalMCE_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_GlobalMCE_ID, GlobalMCE_ID_sequence);

  return offset;
}



static int
dissect_m2ap_GTP_TEID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       4, 4, FALSE, NULL);

  return offset;
}



static int
dissect_m2ap_IPAddress(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 79 "./asn1/m2ap/m2ap.cnf"
  tvbuff_t *parameter_tvb = NULL;
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       4, 16, FALSE, &parameter_tvb);

  if (parameter_tvb) {
    gint tvb_len = tvb_reported_length(parameter_tvb);
    proto_tree *subtree = proto_item_add_subtree(actx->created_item, ett_m2ap_IPAddress);

    switch (tvb_len) {
      case 4:
        proto_tree_add_item(subtree, hf_m2ap_IPAddress_v4, parameter_tvb, 0, 4, ENC_BIG_ENDIAN);
        break;
      case 16:
        proto_tree_add_item(subtree, hf_m2ap_IPAddress_v6, parameter_tvb, 0, 16, ENC_NA);
        break;
      default:
        proto_tree_add_expert(subtree, actx->pinfo, &ei_m2ap_invalid_ip_address_len, parameter_tvb, 0, tvb_len);
        break;
      }
    }


  return offset;
}



static int
dissect_m2ap_LCID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 28U, NULL, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Cell_List_sequence_of[1] = {
  { &hf_m2ap_MBMS_Cell_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ECGI },
};

static int
dissect_m2ap_MBMS_Cell_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Cell_List, MBMS_Cell_List_sequence_of,
                                                  1, maxnoofCellsforMBMS, FALSE);

  return offset;
}



static int
dissect_m2ap_QCI(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 255U, NULL, FALSE);

  return offset;
}


static const per_sequence_t MBMS_E_RAB_QoS_Parameters_sequence[] = {
  { &hf_m2ap_qCI            , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_QCI },
  { &hf_m2ap_gbrQosInformation, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_GBR_QosInformation },
  { &hf_m2ap_allocationAndRetentionPriority, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_AllocationAndRetentionPriority },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMS_E_RAB_QoS_Parameters(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMS_E_RAB_QoS_Parameters, MBMS_E_RAB_QoS_Parameters_sequence);

  return offset;
}



static int
dissect_m2ap_MCE_MBMS_M2AP_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 16777215U, NULL, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Service_associatedLogicalM2_ConnectionItem_sequence[] = {
  { &hf_m2ap_eNB_MBMS_M2AP_ID, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ENB_MBMS_M2AP_ID },
  { &hf_m2ap_mCE_MBMS_M2AP_ID, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_MCE_MBMS_M2AP_ID },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem, MBMS_Service_associatedLogicalM2_ConnectionItem_sequence);

  return offset;
}



static int
dissect_m2ap_MBMS_Session_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       1, 1, FALSE, NULL);

  return offset;
}



static int
dissect_m2ap_OCTET_STRING_SIZE_3(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       3, 3, FALSE, NULL);

  return offset;
}


static const per_sequence_t TMGI_sequence[] = {
  { &hf_m2ap_pLMNidentity   , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PLMN_Identity },
  { &hf_m2ap_serviceID      , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_OCTET_STRING_SIZE_3 },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_TMGI(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_TMGI, TMGI_sequence);

  return offset;
}


static const per_sequence_t MBMSsessionListPerPMCH_Item_item_sequence[] = {
  { &hf_m2ap_tmgi           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_TMGI },
  { &hf_m2ap_lcid           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_LCID },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMSsessionListPerPMCH_Item_item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMSsessionListPerPMCH_Item_item, MBMSsessionListPerPMCH_Item_item_sequence);

  return offset;
}


static const per_sequence_t MBMSsessionListPerPMCH_Item_sequence_of[1] = {
  { &hf_m2ap_MBMSsessionListPerPMCH_Item_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMSsessionListPerPMCH_Item_item },
};

static int
dissect_m2ap_MBMSsessionListPerPMCH_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMSsessionListPerPMCH_Item, MBMSsessionListPerPMCH_Item_sequence_of,
                                                  1, maxnoofSessionsPerPMCH, FALSE);

  return offset;
}


static const per_sequence_t MBMSsessionsToBeSuspendedListPerPMCH_Item_item_sequence[] = {
  { &hf_m2ap_tmgi           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_TMGI },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item, MBMSsessionsToBeSuspendedListPerPMCH_Item_item_sequence);

  return offset;
}


static const per_sequence_t MBMSsessionsToBeSuspendedListPerPMCH_Item_sequence_of[1] = {
  { &hf_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item },
};

static int
dissect_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item, MBMSsessionsToBeSuspendedListPerPMCH_Item_sequence_of,
                                                  1, maxnoofSessionsPerPMCH, FALSE);

  return offset;
}



static int
dissect_m2ap_MBSFN_Area_ID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 255U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_T_radioframeAllocationPeriod_vals[] = {
  {   0, "n1" },
  {   1, "n2" },
  {   2, "n4" },
  {   3, "n8" },
  {   4, "n16" },
  {   5, "n32" },
  { 0, NULL }
};


static int
dissect_m2ap_T_radioframeAllocationPeriod(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     6, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_m2ap_INTEGER_0_7(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 7U, NULL, FALSE);

  return offset;
}



static int
dissect_m2ap_BIT_STRING_SIZE_6(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_bit_string(tvb, offset, actx, tree, hf_index,
                                     6, 6, FALSE, NULL, NULL);

  return offset;
}



static int
dissect_m2ap_BIT_STRING_SIZE_24(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_bit_string(tvb, offset, actx, tree, hf_index,
                                     24, 24, FALSE, NULL, NULL);

  return offset;
}


static const value_string m2ap_T_subframeAllocation_vals[] = {
  {   0, "oneFrame" },
  {   1, "fourFrames" },
  { 0, NULL }
};

static const per_choice_t T_subframeAllocation_choice[] = {
  {   0, &hf_m2ap_oneFrame       , ASN1_NO_EXTENSIONS     , dissect_m2ap_BIT_STRING_SIZE_6 },
  {   1, &hf_m2ap_fourFrames     , ASN1_NO_EXTENSIONS     , dissect_m2ap_BIT_STRING_SIZE_24 },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_T_subframeAllocation(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_T_subframeAllocation, T_subframeAllocation_choice,
                                 NULL);

  return offset;
}


static const per_sequence_t MBSFN_Subframe_Configuration_sequence[] = {
  { &hf_m2ap_radioframeAllocationPeriod, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_radioframeAllocationPeriod },
  { &hf_m2ap_radioframeAllocationOffset, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_INTEGER_0_7 },
  { &hf_m2ap_subframeAllocation, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_subframeAllocation },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBSFN_Subframe_Configuration(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBSFN_Subframe_Configuration, MBSFN_Subframe_Configuration_sequence);

  return offset;
}



static int
dissect_m2ap_MCCH_Update_Time(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 255U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_T_pdcchLength_vals[] = {
  {   0, "s1" },
  {   1, "s2" },
  { 0, NULL }
};


static int
dissect_m2ap_T_pdcchLength(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, TRUE, 0, NULL);

  return offset;
}


static const value_string m2ap_T_repetitionPeriod_vals[] = {
  {   0, "rf32" },
  {   1, "rf64" },
  {   2, "rf128" },
  {   3, "rf256" },
  { 0, NULL }
};


static int
dissect_m2ap_T_repetitionPeriod(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_m2ap_INTEGER_0_10(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 10U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_T_modificationPeriod_vals[] = {
  {   0, "rf512" },
  {   1, "rf1024" },
  { 0, NULL }
};


static int
dissect_m2ap_T_modificationPeriod(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string m2ap_T_modulationAndCodingScheme_vals[] = {
  {   0, "n2" },
  {   1, "n7" },
  {   2, "n13" },
  {   3, "n19" },
  { 0, NULL }
};


static int
dissect_m2ap_T_modulationAndCodingScheme(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_sequence[] = {
  { &hf_m2ap_mbsfnArea      , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBSFN_Area_ID },
  { &hf_m2ap_pdcchLength    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_pdcchLength },
  { &hf_m2ap_repetitionPeriod, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_repetitionPeriod },
  { &hf_m2ap_offset         , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_INTEGER_0_10 },
  { &hf_m2ap_modificationPeriod, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_modificationPeriod },
  { &hf_m2ap_subframeAllocationInfo, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_BIT_STRING_SIZE_6 },
  { &hf_m2ap_modulationAndCodingScheme, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_T_modulationAndCodingScheme },
  { &hf_m2ap_cellInformationList, ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_Cell_Information_List },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item, MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_sequence);

  return offset;
}



static int
dissect_m2ap_MCEname(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 150, TRUE);

  return offset;
}


static const value_string m2ap_MCH_Scheduling_Period_vals[] = {
  {   0, "rf8" },
  {   1, "rf16" },
  {   2, "rf32" },
  {   3, "rf64" },
  {   4, "rf128" },
  {   5, "rf256" },
  {   6, "rf512" },
  {   7, "rf1024" },
  { 0, NULL }
};


static int
dissect_m2ap_MCH_Scheduling_Period(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     8, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string m2ap_MCH_Scheduling_PeriodExtended_vals[] = {
  {   0, "rf4" },
  { 0, NULL }
};


static int
dissect_m2ap_MCH_Scheduling_PeriodExtended(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     1, NULL, TRUE, 0, NULL);

  return offset;
}



static int
dissect_m2ap_Modulation_Coding_Scheme2(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 27U, NULL, FALSE);

  return offset;
}



static int
dissect_m2ap_INTEGER_0_28(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 28U, NULL, FALSE);

  return offset;
}


static const per_sequence_t PMCH_Configuration_sequence[] = {
  { &hf_m2ap_allocatedSubframesEnd, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_AllocatedSubframesEnd },
  { &hf_m2ap_dataMCS        , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_INTEGER_0_28 },
  { &hf_m2ap_mchSchedulingPeriod, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MCH_Scheduling_Period },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_PMCH_Configuration(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_PMCH_Configuration, PMCH_Configuration_sequence);

  return offset;
}


static const value_string m2ap_Common_Subframe_Allocation_Period_vals[] = {
  {   0, "rf4" },
  {   1, "rf8" },
  {   2, "rf16" },
  {   3, "rf32" },
  {   4, "rf64" },
  {   5, "rf128" },
  {   6, "rf256" },
  { 0, NULL }
};


static int
dissect_m2ap_Common_Subframe_Allocation_Period(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     7, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t SC_PTM_Information_sequence[] = {
  { &hf_m2ap_mbmsCellList   , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMS_Cell_List },
  { &hf_m2ap_mbms_E_RAB_QoS_Parameters, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMS_E_RAB_QoS_Parameters },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SC_PTM_Information(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SC_PTM_Information, SC_PTM_Information_sequence);

  return offset;
}



static int
dissect_m2ap_SFN(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 1023U, NULL, FALSE);

  return offset;
}


static const value_string m2ap_TimeToWait_vals[] = {
  {   0, "v1s" },
  {   1, "v2s" },
  {   2, "v5s" },
  {   3, "v10s" },
  {   4, "v20s" },
  {   5, "v60s" },
  { 0, NULL }
};


static int
dissect_m2ap_TimeToWait(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     6, NULL, TRUE, 0, NULL);

  return offset;
}


static const per_sequence_t TNL_Information_sequence[] = {
  { &hf_m2ap_iPMCAddress    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_IPAddress },
  { &hf_m2ap_iPSourceAddress, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_IPAddress },
  { &hf_m2ap_gTP_TEID       , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_GTP_TEID },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_TNL_Information(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_TNL_Information, TNL_Information_sequence);

  return offset;
}


static const per_sequence_t SessionStartRequest_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionStartRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 103 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Start Request");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionStartRequest, SessionStartRequest_sequence);

  return offset;
}


static const per_sequence_t SessionStartResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionStartResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 105 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Start Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionStartResponse, SessionStartResponse_sequence);

  return offset;
}


static const per_sequence_t SessionStartFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionStartFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 107 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Start Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionStartFailure, SessionStartFailure_sequence);

  return offset;
}


static const per_sequence_t SessionStopRequest_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionStopRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 109 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Stop Request");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionStopRequest, SessionStopRequest_sequence);

  return offset;
}


static const per_sequence_t SessionStopResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionStopResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 111 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Stop Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionStopResponse, SessionStopResponse_sequence);

  return offset;
}


static const per_sequence_t SessionUpdateRequest_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionUpdateRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 113 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Update Request");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionUpdateRequest, SessionUpdateRequest_sequence);

  return offset;
}


static const per_sequence_t SessionUpdateResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionUpdateResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 115 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Update Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionUpdateResponse, SessionUpdateResponse_sequence);

  return offset;
}


static const per_sequence_t SessionUpdateFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SessionUpdateFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 117 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Session Update Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SessionUpdateFailure, SessionUpdateFailure_sequence);

  return offset;
}


static const per_sequence_t MbmsSchedulingInformation_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsSchedulingInformation(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 119 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Scheduling Information");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsSchedulingInformation, MbmsSchedulingInformation_sequence);

  return offset;
}


static const per_sequence_t MBSFN_Area_Configuration_List_sequence_of[1] = {
  { &hf_m2ap_MBSFN_Area_Configuration_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
};

static int
dissect_m2ap_MBSFN_Area_Configuration_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBSFN_Area_Configuration_List, MBSFN_Area_Configuration_List_sequence_of,
                                                  1, maxnoofMBSFNareas, FALSE);

  return offset;
}


static const per_sequence_t PMCH_Configuration_List_sequence_of[1] = {
  { &hf_m2ap_PMCH_Configuration_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_PMCH_Configuration_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_PMCH_Configuration_List, PMCH_Configuration_List_sequence_of,
                                                  0, maxnoofPMCHsperMBSFNarea, FALSE);

  return offset;
}


static const per_sequence_t PMCH_Configuration_Item_sequence[] = {
  { &hf_m2ap_pmch_Configuration, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PMCH_Configuration },
  { &hf_m2ap_mbms_Session_List, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMSsessionListPerPMCH_Item },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_PMCH_Configuration_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_PMCH_Configuration_Item, PMCH_Configuration_Item_sequence);

  return offset;
}


static const per_sequence_t MBSFN_Subframe_ConfigurationList_sequence_of[1] = {
  { &hf_m2ap_MBSFN_Subframe_ConfigurationList_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_MBSFN_Subframe_ConfigurationList(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBSFN_Subframe_ConfigurationList, MBSFN_Subframe_ConfigurationList_sequence_of,
                                                  1, maxnoofMBSFN_Allocations, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Suspension_Notification_List_sequence_of[1] = {
  { &hf_m2ap_MBMS_Suspension_Notification_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_MBMS_Suspension_Notification_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Suspension_Notification_List, MBMS_Suspension_Notification_List_sequence_of,
                                                  1, maxnoofPMCHsperMBSFNarea, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Suspension_Notification_Item_sequence[] = {
  { &hf_m2ap_sfn            , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_SFN },
  { &hf_m2ap_mbms_Sessions_To_Be_Suspended_List, ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMS_Suspension_Notification_Item(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMS_Suspension_Notification_Item, MBMS_Suspension_Notification_Item_sequence);

  return offset;
}


static const per_sequence_t MbmsSchedulingInformationResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsSchedulingInformationResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 121 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Scheduling Information Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsSchedulingInformationResponse, MbmsSchedulingInformationResponse_sequence);

  return offset;
}


static const per_sequence_t M2SetupRequest_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_M2SetupRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 123 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "M2 Setup Request");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_M2SetupRequest, M2SetupRequest_sequence);

  return offset;
}


static const per_sequence_t ENB_MBMS_Configuration_data_List_sequence_of[1] = {
  { &hf_m2ap_ENB_MBMS_Configuration_data_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_ENB_MBMS_Configuration_data_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_ENB_MBMS_Configuration_data_List, ENB_MBMS_Configuration_data_List_sequence_of,
                                                  1, maxnoofCells, FALSE);

  return offset;
}


static const per_sequence_t M2SetupResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_M2SetupResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 125 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "M2 Setup Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_M2SetupResponse, M2SetupResponse_sequence);

  return offset;
}


static const per_sequence_t MCCHrelatedBCCH_ConfigPerMBSFNArea_sequence_of[1] = {
  { &hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea, MCCHrelatedBCCH_ConfigPerMBSFNArea_sequence_of,
                                                  1, maxnoofMBSFNareas, FALSE);

  return offset;
}


static const per_sequence_t M2SetupFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_M2SetupFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 127 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "M2 Setup Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_M2SetupFailure, M2SetupFailure_sequence);

  return offset;
}


static const per_sequence_t ENBConfigurationUpdate_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ENBConfigurationUpdate(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 129 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "eNB Configuration Update");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ENBConfigurationUpdate, ENBConfigurationUpdate_sequence);

  return offset;
}


static const per_sequence_t ENB_MBMS_Configuration_data_List_ConfigUpdate_sequence_of[1] = {
  { &hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate, ENB_MBMS_Configuration_data_List_ConfigUpdate_sequence_of,
                                                  1, maxnoofCells, FALSE);

  return offset;
}


static const per_sequence_t ENBConfigurationUpdateAcknowledge_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ENBConfigurationUpdateAcknowledge(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 131 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "eNB Configuration Update Acknowledge");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ENBConfigurationUpdateAcknowledge, ENBConfigurationUpdateAcknowledge_sequence);

  return offset;
}


static const per_sequence_t ENBConfigurationUpdateFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ENBConfigurationUpdateFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 133 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "eNB Configuration Update Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ENBConfigurationUpdateFailure, ENBConfigurationUpdateFailure_sequence);

  return offset;
}


static const per_sequence_t MCEConfigurationUpdate_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MCEConfigurationUpdate(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 135 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MCE Configuration Update");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MCEConfigurationUpdate, MCEConfigurationUpdate_sequence);

  return offset;
}


static const per_sequence_t MCEConfigurationUpdateAcknowledge_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MCEConfigurationUpdateAcknowledge(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 137 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MCE Configuration Update Acknowledge");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MCEConfigurationUpdateAcknowledge, MCEConfigurationUpdateAcknowledge_sequence);

  return offset;
}


static const per_sequence_t MCEConfigurationUpdateFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MCEConfigurationUpdateFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 139 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MCE Configuration Update Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MCEConfigurationUpdateFailure, MCEConfigurationUpdateFailure_sequence);

  return offset;
}


static const per_sequence_t ErrorIndication_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ErrorIndication(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 141 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "Error Indication");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ErrorIndication, ErrorIndication_sequence);

  return offset;
}


static const per_sequence_t Reset_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_Reset(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 143 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "Reset");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_Reset, Reset_sequence);

  return offset;
}


static const value_string m2ap_ResetAll_vals[] = {
  {   0, "reset-all" },
  { 0, NULL }
};


static int
dissect_m2ap_ResetAll(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     1, NULL, TRUE, 0, NULL);

  return offset;
}


static const per_sequence_t MBMS_Service_associatedLogicalM2_ConnectionListRes_sequence_of[1] = {
  { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes, MBMS_Service_associatedLogicalM2_ConnectionListRes_sequence_of,
                                                  1, maxNrOfIndividualM2ConnectionsToReset, FALSE);

  return offset;
}


static const value_string m2ap_ResetType_vals[] = {
  {   0, "m2-Interface" },
  {   1, "partOfM2-Interface" },
  { 0, NULL }
};

static const per_choice_t ResetType_choice[] = {
  {   0, &hf_m2ap_m2_Interface   , ASN1_EXTENSION_ROOT    , dissect_m2ap_ResetAll },
  {   1, &hf_m2ap_partOfM2_Interface, ASN1_EXTENSION_ROOT    , dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_ResetType(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_ResetType, ResetType_choice,
                                 NULL);

  return offset;
}


static const per_sequence_t ResetAcknowledge_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_ResetAcknowledge(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 145 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "Reset Acknowledge");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_ResetAcknowledge, ResetAcknowledge_sequence);

  return offset;
}


static const per_sequence_t MBMS_Service_associatedLogicalM2_ConnectionListResAck_sequence_of[1] = {
  { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Single_Container },
};

static int
dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck, MBMS_Service_associatedLogicalM2_ConnectionListResAck_sequence_of,
                                                  1, maxNrOfIndividualM2ConnectionsToReset, FALSE);

  return offset;
}


static const per_sequence_t PrivateMessage_sequence[] = {
  { &hf_m2ap_privateIEs     , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_PrivateIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_PrivateMessage(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 147 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "Private Message");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_PrivateMessage, PrivateMessage_sequence);

  return offset;
}


static const per_sequence_t MbmsServiceCountingRequest_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsServiceCountingRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 149 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Service Counting Request");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsServiceCountingRequest, MbmsServiceCountingRequest_sequence);

  return offset;
}


static const per_sequence_t MBMS_Counting_Request_Session_sequence_of[1] = {
  { &hf_m2ap_MBMS_Counting_Request_Session_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
};

static int
dissect_m2ap_MBMS_Counting_Request_Session(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Counting_Request_Session, MBMS_Counting_Request_Session_sequence_of,
                                                  1, maxnoofCountingService, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Counting_Request_SessionIE_sequence[] = {
  { &hf_m2ap_tmgi           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_TMGI },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMS_Counting_Request_SessionIE(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMS_Counting_Request_SessionIE, MBMS_Counting_Request_SessionIE_sequence);

  return offset;
}


static const per_sequence_t MbmsServiceCountingResponse_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsServiceCountingResponse(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 151 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Service Counting Response");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsServiceCountingResponse, MbmsServiceCountingResponse_sequence);

  return offset;
}


static const per_sequence_t MbmsServiceCountingFailure_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsServiceCountingFailure(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 153 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Service Counting Failure");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsServiceCountingFailure, MbmsServiceCountingFailure_sequence);

  return offset;
}


static const per_sequence_t MbmsServiceCountingResultsReport_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsServiceCountingResultsReport(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 155 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Service Counting Results Report");

  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsServiceCountingResultsReport, MbmsServiceCountingResultsReport_sequence);

  return offset;
}


static const per_sequence_t MBMS_Counting_Result_List_sequence_of[1] = {
  { &hf_m2ap_MBMS_Counting_Result_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
};

static int
dissect_m2ap_MBMS_Counting_Result_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_MBMS_Counting_Result_List, MBMS_Counting_Result_List_sequence_of,
                                                  1, maxnoofCountingService, FALSE);

  return offset;
}



static int
dissect_m2ap_CountingResult(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 1023U, NULL, FALSE);

  return offset;
}


static const per_sequence_t MBMS_Counting_Result_sequence[] = {
  { &hf_m2ap_tmgi           , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_TMGI },
  { &hf_m2ap_countingResult , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_CountingResult },
  { &hf_m2ap_iE_Extensions  , ASN1_EXTENSION_ROOT    , ASN1_OPTIONAL    , dissect_m2ap_ProtocolExtensionContainer },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MBMS_Counting_Result(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MBMS_Counting_Result, MBMS_Counting_Result_sequence);

  return offset;
}


static const per_sequence_t MbmsOverloadNotification_sequence[] = {
  { &hf_m2ap_protocolIEs    , ASN1_EXTENSION_ROOT    , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_MbmsOverloadNotification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 157 "./asn1/m2ap/m2ap.cnf"
  col_set_str(actx->pinfo->cinfo, COL_INFO, "MBMS Overload Notification");


  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_MbmsOverloadNotification, MbmsOverloadNotification_sequence);

  return offset;
}


static const per_sequence_t Overload_Status_Per_PMCH_List_sequence_of[1] = {
  { &hf_m2ap_Overload_Status_Per_PMCH_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
};

static int
dissect_m2ap_Overload_Status_Per_PMCH_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_Overload_Status_Per_PMCH_List, Overload_Status_Per_PMCH_List_sequence_of,
                                                  1, maxnoofPMCHsperMBSFNarea, FALSE);

  return offset;
}


static const value_string m2ap_PMCH_Overload_Status_vals[] = {
  {   0, "normal" },
  {   1, "overload" },
  { 0, NULL }
};


static int
dissect_m2ap_PMCH_Overload_Status(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, TRUE, 0, NULL);

  return offset;
}


static const per_sequence_t Active_MBMS_Session_List_sequence_of[1] = {
  { &hf_m2ap_Active_MBMS_Session_List_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProtocolIE_Container },
};

static int
dissect_m2ap_Active_MBMS_Session_List(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_m2ap_Active_MBMS_Session_List, Active_MBMS_Session_List_sequence_of,
                                                  1, maxnoofSessionsPerPMCH, FALSE);

  return offset;
}



static int
dissect_m2ap_InitiatingMessage_value(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 59 "./asn1/m2ap/m2ap.cnf"
  message_type = INITIATING_MESSAGE;


  offset = dissect_per_open_type_pdu_new(tvb, offset, actx, tree, hf_index, dissect_InitiatingMessageValue);

  return offset;
}


static const per_sequence_t InitiatingMessage_sequence[] = {
  { &hf_m2ap_procedureCode  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProcedureCode },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_initiatingMessage_value, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_InitiatingMessage_value },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_InitiatingMessage(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_InitiatingMessage, InitiatingMessage_sequence);

  return offset;
}



static int
dissect_m2ap_SuccessfulOutcome_value(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 63 "./asn1/m2ap/m2ap.cnf"
  message_type = SUCCESSFUL_OUTCOME;


  offset = dissect_per_open_type_pdu_new(tvb, offset, actx, tree, hf_index, dissect_SuccessfulOutcomeValue);

  return offset;
}


static const per_sequence_t SuccessfulOutcome_sequence[] = {
  { &hf_m2ap_procedureCode  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProcedureCode },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_successfulOutcome_value, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_SuccessfulOutcome_value },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_SuccessfulOutcome(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_SuccessfulOutcome, SuccessfulOutcome_sequence);

  return offset;
}



static int
dissect_m2ap_UnsuccessfulOutcome_value(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 67 "./asn1/m2ap/m2ap.cnf"
  message_type = UNSUCCESSFUL_OUTCOME;


  offset = dissect_per_open_type_pdu_new(tvb, offset, actx, tree, hf_index, dissect_UnsuccessfulOutcomeValue);

  return offset;
}


static const per_sequence_t UnsuccessfulOutcome_sequence[] = {
  { &hf_m2ap_procedureCode  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_ProcedureCode },
  { &hf_m2ap_criticality    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_Criticality },
  { &hf_m2ap_unsuccessfulOutcome_value, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_m2ap_UnsuccessfulOutcome_value },
  { NULL, 0, 0, NULL }
};

static int
dissect_m2ap_UnsuccessfulOutcome(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_m2ap_UnsuccessfulOutcome, UnsuccessfulOutcome_sequence);

  return offset;
}


static const value_string m2ap_M2AP_PDU_vals[] = {
  {   0, "initiatingMessage" },
  {   1, "successfulOutcome" },
  {   2, "unsuccessfulOutcome" },
  { 0, NULL }
};

static const per_choice_t M2AP_PDU_choice[] = {
  {   0, &hf_m2ap_initiatingMessage, ASN1_EXTENSION_ROOT    , dissect_m2ap_InitiatingMessage },
  {   1, &hf_m2ap_successfulOutcome, ASN1_EXTENSION_ROOT    , dissect_m2ap_SuccessfulOutcome },
  {   2, &hf_m2ap_unsuccessfulOutcome, ASN1_EXTENSION_ROOT    , dissect_m2ap_UnsuccessfulOutcome },
  { 0, NULL, 0, NULL }
};

static int
dissect_m2ap_M2AP_PDU(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_m2ap_M2AP_PDU, M2AP_PDU_choice,
                                 NULL);

  return offset;
}

/*--- PDUs ---*/

static int dissect_Cause_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Cause(tvb, offset, &asn1_ctx, tree, hf_m2ap_Cause_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_CriticalityDiagnostics_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_CriticalityDiagnostics(tvb, offset, &asn1_ctx, tree, hf_m2ap_CriticalityDiagnostics_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENB_MBMS_Configuration_data_Item_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENB_MBMS_Configuration_data_Item(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENB_MBMS_Configuration_data_Item_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENB_MBMS_Configuration_data_ConfigUpdate_Item_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENB_MBMS_M2AP_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENB_MBMS_M2AP_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENB_MBMS_M2AP_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENBname_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENBname(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENBname_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_GlobalENB_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_GlobalENB_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_GlobalENB_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_GlobalMCE_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_GlobalMCE_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_GlobalMCE_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Service_associatedLogicalM2_ConnectionItem_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Service_Area_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Service_Area(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Service_Area_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Session_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Session_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Session_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBSFN_Area_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBSFN_Area_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBSFN_Area_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBSFN_Subframe_Configuration_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBSFN_Subframe_Configuration(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBSFN_Subframe_Configuration_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCCH_Update_Time_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCCH_Update_Time(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCCH_Update_Time_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCE_MBMS_M2AP_ID_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCE_MBMS_M2AP_ID(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCE_MBMS_M2AP_ID_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCEname_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCEname(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCEname_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCH_Scheduling_PeriodExtended_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCH_Scheduling_PeriodExtended(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCH_Scheduling_PeriodExtended_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_Modulation_Coding_Scheme2_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Modulation_Coding_Scheme2(tvb, offset, &asn1_ctx, tree, hf_m2ap_Modulation_Coding_Scheme2_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_Common_Subframe_Allocation_Period_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Common_Subframe_Allocation_Period(tvb, offset, &asn1_ctx, tree, hf_m2ap_Common_Subframe_Allocation_Period_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SC_PTM_Information_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SC_PTM_Information(tvb, offset, &asn1_ctx, tree, hf_m2ap_SC_PTM_Information_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_TimeToWait_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_TimeToWait(tvb, offset, &asn1_ctx, tree, hf_m2ap_TimeToWait_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_TMGI_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_TMGI(tvb, offset, &asn1_ctx, tree, hf_m2ap_TMGI_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_TNL_Information_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_TNL_Information(tvb, offset, &asn1_ctx, tree, hf_m2ap_TNL_Information_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionStartRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionStartRequest(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionStartRequest_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionStartResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionStartResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionStartResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionStartFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionStartFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionStartFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionStopRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionStopRequest(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionStopRequest_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionStopResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionStopResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionStopResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionUpdateRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionUpdateRequest(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionUpdateRequest_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionUpdateResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionUpdateResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionUpdateResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_SessionUpdateFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_SessionUpdateFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_SessionUpdateFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsSchedulingInformation_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsSchedulingInformation(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsSchedulingInformation_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBSFN_Area_Configuration_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBSFN_Area_Configuration_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBSFN_Area_Configuration_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_PMCH_Configuration_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_PMCH_Configuration_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_PMCH_Configuration_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_PMCH_Configuration_Item_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_PMCH_Configuration_Item(tvb, offset, &asn1_ctx, tree, hf_m2ap_PMCH_Configuration_Item_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBSFN_Subframe_ConfigurationList_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBSFN_Subframe_ConfigurationList(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBSFN_Subframe_ConfigurationList_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Suspension_Notification_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Suspension_Notification_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Suspension_Notification_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Suspension_Notification_Item_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Suspension_Notification_Item(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Suspension_Notification_Item_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsSchedulingInformationResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsSchedulingInformationResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsSchedulingInformationResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_M2SetupRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_M2SetupRequest(tvb, offset, &asn1_ctx, tree, hf_m2ap_M2SetupRequest_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENB_MBMS_Configuration_data_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENB_MBMS_Configuration_data_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENB_MBMS_Configuration_data_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_M2SetupResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_M2SetupResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_M2SetupResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCCHrelatedBCCH_ConfigPerMBSFNArea_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_M2SetupFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_M2SetupFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_M2SetupFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENBConfigurationUpdate_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENBConfigurationUpdate(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENBConfigurationUpdate_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENB_MBMS_Configuration_data_List_ConfigUpdate_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENBConfigurationUpdateAcknowledge_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENBConfigurationUpdateAcknowledge(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENBConfigurationUpdateAcknowledge_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ENBConfigurationUpdateFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ENBConfigurationUpdateFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_ENBConfigurationUpdateFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCEConfigurationUpdate_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCEConfigurationUpdate(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCEConfigurationUpdate_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCEConfigurationUpdateAcknowledge_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCEConfigurationUpdateAcknowledge(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCEConfigurationUpdateAcknowledge_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MCEConfigurationUpdateFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MCEConfigurationUpdateFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_MCEConfigurationUpdateFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ErrorIndication_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ErrorIndication(tvb, offset, &asn1_ctx, tree, hf_m2ap_ErrorIndication_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_Reset_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Reset(tvb, offset, &asn1_ctx, tree, hf_m2ap_Reset_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ResetType_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ResetType(tvb, offset, &asn1_ctx, tree, hf_m2ap_ResetType_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_ResetAcknowledge_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_ResetAcknowledge(tvb, offset, &asn1_ctx, tree, hf_m2ap_ResetAcknowledge_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Service_associatedLogicalM2_ConnectionListResAck_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_PrivateMessage_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_PrivateMessage(tvb, offset, &asn1_ctx, tree, hf_m2ap_PrivateMessage_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsServiceCountingRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsServiceCountingRequest(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsServiceCountingRequest_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Counting_Request_Session_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Counting_Request_Session(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Counting_Request_Session_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Counting_Request_SessionIE_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Counting_Request_SessionIE(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Counting_Request_SessionIE_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsServiceCountingResponse_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsServiceCountingResponse(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsServiceCountingResponse_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsServiceCountingFailure_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsServiceCountingFailure(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsServiceCountingFailure_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsServiceCountingResultsReport_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsServiceCountingResultsReport(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsServiceCountingResultsReport_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Counting_Result_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Counting_Result_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Counting_Result_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MBMS_Counting_Result_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MBMS_Counting_Result(tvb, offset, &asn1_ctx, tree, hf_m2ap_MBMS_Counting_Result_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_MbmsOverloadNotification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_MbmsOverloadNotification(tvb, offset, &asn1_ctx, tree, hf_m2ap_MbmsOverloadNotification_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_Overload_Status_Per_PMCH_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Overload_Status_Per_PMCH_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_Overload_Status_Per_PMCH_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_PMCH_Overload_Status_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_PMCH_Overload_Status(tvb, offset, &asn1_ctx, tree, hf_m2ap_PMCH_Overload_Status_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_Active_MBMS_Session_List_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_Active_MBMS_Session_List(tvb, offset, &asn1_ctx, tree, hf_m2ap_Active_MBMS_Session_List_PDU);
  offset += 7; offset >>= 3;
  return offset;
}
static int dissect_M2AP_PDU_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  offset = dissect_m2ap_M2AP_PDU(tvb, offset, &asn1_ctx, tree, hf_m2ap_M2AP_PDU_PDU);
  offset += 7; offset >>= 3;
  return offset;
}


/*--- End of included file: packet-m2ap-fn.c ---*/
#line 89 "./asn1/m2ap/packet-m2ap-template.c"

static int
dissect_ProtocolIEFieldValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  return (dissector_try_uint(m2ap_ies_dissector_table, ProtocolIE_ID, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int
dissect_ProtocolExtensionFieldExtensionValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  return (dissector_try_uint(m2ap_extension_dissector_table, ProtocolIE_ID, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int
dissect_InitiatingMessageValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  return (dissector_try_uint(m2ap_proc_imsg_dissector_table, ProcedureCode, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int
dissect_SuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  return (dissector_try_uint(m2ap_proc_sout_dissector_table, ProcedureCode, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int
dissect_UnsuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  return (dissector_try_uint(m2ap_proc_uout_dissector_table, ProcedureCode, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}


static int
dissect_m2ap(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
  proto_item *m2ap_item = NULL;
  proto_tree *m2ap_tree = NULL;

  /* make entry in the Protocol column on summary display */
  col_set_str(pinfo->cinfo, COL_PROTOCOL, PSNAME);
  col_clear_fence(pinfo->cinfo, COL_INFO);
  col_clear(pinfo->cinfo, COL_INFO);

  m2ap_item = proto_tree_add_item(tree, proto_m2ap, tvb, 0, -1, ENC_NA);
  m2ap_tree = proto_item_add_subtree(m2ap_item, ett_m2ap);

  dissect_M2AP_PDU_PDU(tvb, pinfo, m2ap_tree, NULL);

  return tvb_captured_length(tvb);
}

void
proto_register_m2ap(void) {

  /* List of fields */
  static hf_register_info hf[] = {
    { &hf_m2ap_IPAddress_v4,
      { "IPAddress", "m2ap.IPAddress_v4",
         FT_IPv4, BASE_NONE, NULL, 0,
         NULL, HFILL }
    },
    { &hf_m2ap_IPAddress_v6,
      { "IPAddress", "m2ap.IPAddress_v6",
         FT_IPv6, BASE_NONE, NULL, 0,
         NULL, HFILL }
    },

/*--- Included file: packet-m2ap-hfarr.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-hfarr.c"
    { &hf_m2ap_Cause_PDU,
      { "Cause", "m2ap.Cause",
        FT_UINT32, BASE_DEC, VALS(m2ap_Cause_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_CriticalityDiagnostics_PDU,
      { "CriticalityDiagnostics", "m2ap.CriticalityDiagnostics_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_Item_PDU,
      { "ENB-MBMS-Configuration-data-Item", "m2ap.ENB_MBMS_Configuration_data_Item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item_PDU,
      { "ENB-MBMS-Configuration-data-ConfigUpdate-Item", "m2ap.ENB_MBMS_Configuration_data_ConfigUpdate_Item",
        FT_UINT32, BASE_DEC, VALS(m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_M2AP_ID_PDU,
      { "ENB-MBMS-M2AP-ID", "m2ap.ENB_MBMS_M2AP_ID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENBname_PDU,
      { "ENBname", "m2ap.ENBname",
        FT_STRING, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_GlobalENB_ID_PDU,
      { "GlobalENB-ID", "m2ap.GlobalENB_ID_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_GlobalMCE_ID_PDU,
      { "GlobalMCE-ID", "m2ap.GlobalMCE_ID_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem_PDU,
      { "MBMS-Service-associatedLogicalM2-ConnectionItem", "m2ap.MBMS_Service_associatedLogicalM2_ConnectionItem_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Service_Area_PDU,
      { "MBMS-Service-Area", "m2ap.MBMS_Service_Area",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Session_ID_PDU,
      { "MBMS-Session-ID", "m2ap.MBMS_Session_ID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBSFN_Area_ID_PDU,
      { "MBSFN-Area-ID", "m2ap.MBSFN_Area_ID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBSFN_Subframe_Configuration_PDU,
      { "MBSFN-Subframe-Configuration", "m2ap.MBSFN_Subframe_Configuration_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCCH_Update_Time_PDU,
      { "MCCH-Update-Time", "m2ap.MCCH_Update_Time",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_PDU,
      { "MCCHrelatedBCCH-ConfigPerMBSFNArea-Item", "m2ap.MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCE_MBMS_M2AP_ID_PDU,
      { "MCE-MBMS-M2AP-ID", "m2ap.MCE_MBMS_M2AP_ID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCEname_PDU,
      { "MCEname", "m2ap.MCEname",
        FT_STRING, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCH_Scheduling_PeriodExtended_PDU,
      { "MCH-Scheduling-PeriodExtended", "m2ap.MCH_Scheduling_PeriodExtended",
        FT_UINT32, BASE_DEC, VALS(m2ap_MCH_Scheduling_PeriodExtended_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_Modulation_Coding_Scheme2_PDU,
      { "Modulation-Coding-Scheme2", "m2ap.Modulation_Coding_Scheme2",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_Common_Subframe_Allocation_Period_PDU,
      { "Common-Subframe-Allocation-Period", "m2ap.Common_Subframe_Allocation_Period",
        FT_UINT32, BASE_DEC, VALS(m2ap_Common_Subframe_Allocation_Period_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_SC_PTM_Information_PDU,
      { "SC-PTM-Information", "m2ap.SC_PTM_Information_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_TimeToWait_PDU,
      { "TimeToWait", "m2ap.TimeToWait",
        FT_UINT32, BASE_DEC, VALS(m2ap_TimeToWait_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_TMGI_PDU,
      { "TMGI", "m2ap.TMGI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_TNL_Information_PDU,
      { "TNL-Information", "m2ap.TNL_Information_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionStartRequest_PDU,
      { "SessionStartRequest", "m2ap.SessionStartRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionStartResponse_PDU,
      { "SessionStartResponse", "m2ap.SessionStartResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionStartFailure_PDU,
      { "SessionStartFailure", "m2ap.SessionStartFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionStopRequest_PDU,
      { "SessionStopRequest", "m2ap.SessionStopRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionStopResponse_PDU,
      { "SessionStopResponse", "m2ap.SessionStopResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionUpdateRequest_PDU,
      { "SessionUpdateRequest", "m2ap.SessionUpdateRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionUpdateResponse_PDU,
      { "SessionUpdateResponse", "m2ap.SessionUpdateResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_SessionUpdateFailure_PDU,
      { "SessionUpdateFailure", "m2ap.SessionUpdateFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsSchedulingInformation_PDU,
      { "MbmsSchedulingInformation", "m2ap.MbmsSchedulingInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBSFN_Area_Configuration_List_PDU,
      { "MBSFN-Area-Configuration-List", "m2ap.MBSFN_Area_Configuration_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PMCH_Configuration_List_PDU,
      { "PMCH-Configuration-List", "m2ap.PMCH_Configuration_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PMCH_Configuration_Item_PDU,
      { "PMCH-Configuration-Item", "m2ap.PMCH_Configuration_Item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBSFN_Subframe_ConfigurationList_PDU,
      { "MBSFN-Subframe-ConfigurationList", "m2ap.MBSFN_Subframe_ConfigurationList",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Suspension_Notification_List_PDU,
      { "MBMS-Suspension-Notification-List", "m2ap.MBMS_Suspension_Notification_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Suspension_Notification_Item_PDU,
      { "MBMS-Suspension-Notification-Item", "m2ap.MBMS_Suspension_Notification_Item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsSchedulingInformationResponse_PDU,
      { "MbmsSchedulingInformationResponse", "m2ap.MbmsSchedulingInformationResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_M2SetupRequest_PDU,
      { "M2SetupRequest", "m2ap.M2SetupRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_List_PDU,
      { "ENB-MBMS-Configuration-data-List", "m2ap.ENB_MBMS_Configuration_data_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_M2SetupResponse_PDU,
      { "M2SetupResponse", "m2ap.M2SetupResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_PDU,
      { "MCCHrelatedBCCH-ConfigPerMBSFNArea", "m2ap.MCCHrelatedBCCH_ConfigPerMBSFNArea",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_M2SetupFailure_PDU,
      { "M2SetupFailure", "m2ap.M2SetupFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENBConfigurationUpdate_PDU,
      { "ENBConfigurationUpdate", "m2ap.ENBConfigurationUpdate_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_PDU,
      { "ENB-MBMS-Configuration-data-List-ConfigUpdate", "m2ap.ENB_MBMS_Configuration_data_List_ConfigUpdate",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENBConfigurationUpdateAcknowledge_PDU,
      { "ENBConfigurationUpdateAcknowledge", "m2ap.ENBConfigurationUpdateAcknowledge_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENBConfigurationUpdateFailure_PDU,
      { "ENBConfigurationUpdateFailure", "m2ap.ENBConfigurationUpdateFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCEConfigurationUpdate_PDU,
      { "MCEConfigurationUpdate", "m2ap.MCEConfigurationUpdate_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCEConfigurationUpdateAcknowledge_PDU,
      { "MCEConfigurationUpdateAcknowledge", "m2ap.MCEConfigurationUpdateAcknowledge_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCEConfigurationUpdateFailure_PDU,
      { "MCEConfigurationUpdateFailure", "m2ap.MCEConfigurationUpdateFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ErrorIndication_PDU,
      { "ErrorIndication", "m2ap.ErrorIndication_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_Reset_PDU,
      { "Reset", "m2ap.Reset_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ResetType_PDU,
      { "ResetType", "m2ap.ResetType",
        FT_UINT32, BASE_DEC, VALS(m2ap_ResetType_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_ResetAcknowledge_PDU,
      { "ResetAcknowledge", "m2ap.ResetAcknowledge_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_PDU,
      { "MBMS-Service-associatedLogicalM2-ConnectionListResAck", "m2ap.MBMS_Service_associatedLogicalM2_ConnectionListResAck",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PrivateMessage_PDU,
      { "PrivateMessage", "m2ap.PrivateMessage_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsServiceCountingRequest_PDU,
      { "MbmsServiceCountingRequest", "m2ap.MbmsServiceCountingRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Counting_Request_Session_PDU,
      { "MBMS-Counting-Request-Session", "m2ap.MBMS_Counting_Request_Session",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Counting_Request_SessionIE_PDU,
      { "MBMS-Counting-Request-SessionIE", "m2ap.MBMS_Counting_Request_SessionIE_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsServiceCountingResponse_PDU,
      { "MbmsServiceCountingResponse", "m2ap.MbmsServiceCountingResponse_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsServiceCountingFailure_PDU,
      { "MbmsServiceCountingFailure", "m2ap.MbmsServiceCountingFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsServiceCountingResultsReport_PDU,
      { "MbmsServiceCountingResultsReport", "m2ap.MbmsServiceCountingResultsReport_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Counting_Result_List_PDU,
      { "MBMS-Counting-Result-List", "m2ap.MBMS_Counting_Result_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Counting_Result_PDU,
      { "MBMS-Counting-Result", "m2ap.MBMS_Counting_Result_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MbmsOverloadNotification_PDU,
      { "MbmsOverloadNotification", "m2ap.MbmsOverloadNotification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_Overload_Status_Per_PMCH_List_PDU,
      { "Overload-Status-Per-PMCH-List", "m2ap.Overload_Status_Per_PMCH_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PMCH_Overload_Status_PDU,
      { "PMCH-Overload-Status", "m2ap.PMCH_Overload_Status",
        FT_UINT32, BASE_DEC, VALS(m2ap_PMCH_Overload_Status_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_Active_MBMS_Session_List_PDU,
      { "Active-MBMS-Session-List", "m2ap.Active_MBMS_Session_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_M2AP_PDU_PDU,
      { "M2AP-PDU", "m2ap.M2AP_PDU",
        FT_UINT32, BASE_DEC, VALS(m2ap_M2AP_PDU_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_local,
      { "local", "m2ap.local",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_maxPrivateIEs", HFILL }},
    { &hf_m2ap_global,
      { "global", "m2ap.global",
        FT_OID, BASE_NONE, NULL, 0,
        "OBJECT_IDENTIFIER", HFILL }},
    { &hf_m2ap_ProtocolIE_Container_item,
      { "ProtocolIE-Field", "m2ap.ProtocolIE_Field_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_id,
      { "id", "m2ap.id",
        FT_UINT32, BASE_DEC|BASE_EXT_STRING, &m2ap_ProtocolIE_ID_vals_ext, 0,
        "ProtocolIE_ID", HFILL }},
    { &hf_m2ap_criticality,
      { "criticality", "m2ap.criticality",
        FT_UINT32, BASE_DEC, VALS(m2ap_Criticality_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_ie_field_value,
      { "value", "m2ap.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_ie_field_value", HFILL }},
    { &hf_m2ap_ProtocolExtensionContainer_item,
      { "ProtocolExtensionField", "m2ap.ProtocolExtensionField_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ext_id,
      { "id", "m2ap.id",
        FT_UINT32, BASE_DEC|BASE_EXT_STRING, &m2ap_ProtocolIE_ID_vals_ext, 0,
        "ProtocolIE_ID", HFILL }},
    { &hf_m2ap_extensionValue,
      { "extensionValue", "m2ap.extensionValue_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PrivateIE_Container_item,
      { "PrivateIE-Field", "m2ap.PrivateIE_Field_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_private_id,
      { "id", "m2ap.id",
        FT_UINT32, BASE_DEC, VALS(m2ap_PrivateIE_ID_vals), 0,
        "PrivateIE_ID", HFILL }},
    { &hf_m2ap_private_value,
      { "value", "m2ap.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_private_value", HFILL }},
    { &hf_m2ap_priorityLevel,
      { "priorityLevel", "m2ap.priorityLevel",
        FT_UINT32, BASE_DEC, VALS(m2ap_PriorityLevel_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_pre_emptionCapability,
      { "pre-emptionCapability", "m2ap.pre_emptionCapability",
        FT_UINT32, BASE_DEC, VALS(m2ap_Pre_emptionCapability_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_pre_emptionVulnerability,
      { "pre-emptionVulnerability", "m2ap.pre_emptionVulnerability",
        FT_UINT32, BASE_DEC, VALS(m2ap_Pre_emptionVulnerability_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_iE_Extensions,
      { "iE-Extensions", "m2ap.iE_Extensions",
        FT_UINT32, BASE_DEC, NULL, 0,
        "ProtocolExtensionContainer", HFILL }},
    { &hf_m2ap_radioNetwork,
      { "radioNetwork", "m2ap.radioNetwork",
        FT_UINT32, BASE_DEC, VALS(m2ap_CauseRadioNetwork_vals), 0,
        "CauseRadioNetwork", HFILL }},
    { &hf_m2ap_transport,
      { "transport", "m2ap.transport",
        FT_UINT32, BASE_DEC, VALS(m2ap_CauseTransport_vals), 0,
        "CauseTransport", HFILL }},
    { &hf_m2ap_nAS,
      { "nAS", "m2ap.nAS",
        FT_UINT32, BASE_DEC, VALS(m2ap_CauseNAS_vals), 0,
        "CauseNAS", HFILL }},
    { &hf_m2ap_protocol,
      { "protocol", "m2ap.protocol",
        FT_UINT32, BASE_DEC, VALS(m2ap_CauseProtocol_vals), 0,
        "CauseProtocol", HFILL }},
    { &hf_m2ap_misc,
      { "misc", "m2ap.misc",
        FT_UINT32, BASE_DEC, VALS(m2ap_CauseMisc_vals), 0,
        "CauseMisc", HFILL }},
    { &hf_m2ap_eCGI,
      { "eCGI", "m2ap.eCGI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_cellReservationInfo,
      { "cellReservationInfo", "m2ap.cellReservationInfo",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_cellReservationInfo_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_Cell_Information_List_item,
      { "Cell-Information", "m2ap.Cell_Information_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_procedureCode,
      { "procedureCode", "m2ap.procedureCode",
        FT_UINT32, BASE_DEC|BASE_EXT_STRING, &m2ap_ProcedureCode_vals_ext, 0,
        NULL, HFILL }},
    { &hf_m2ap_triggeringMessage,
      { "triggeringMessage", "m2ap.triggeringMessage",
        FT_UINT32, BASE_DEC, VALS(m2ap_TriggeringMessage_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_procedureCriticality,
      { "procedureCriticality", "m2ap.procedureCriticality",
        FT_UINT32, BASE_DEC, VALS(m2ap_Criticality_vals), 0,
        "Criticality", HFILL }},
    { &hf_m2ap_iEsCriticalityDiagnostics,
      { "iEsCriticalityDiagnostics", "m2ap.iEsCriticalityDiagnostics",
        FT_UINT32, BASE_DEC, NULL, 0,
        "CriticalityDiagnostics_IE_List", HFILL }},
    { &hf_m2ap_CriticalityDiagnostics_IE_List_item,
      { "CriticalityDiagnostics-IE-List item", "m2ap.CriticalityDiagnostics_IE_List_item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_iECriticality,
      { "iECriticality", "m2ap.iECriticality",
        FT_UINT32, BASE_DEC, VALS(m2ap_Criticality_vals), 0,
        "Criticality", HFILL }},
    { &hf_m2ap_iE_ID,
      { "iE-ID", "m2ap.iE_ID",
        FT_UINT32, BASE_DEC|BASE_EXT_STRING, &m2ap_ProtocolIE_ID_vals_ext, 0,
        "ProtocolIE_ID", HFILL }},
    { &hf_m2ap_typeOfError,
      { "typeOfError", "m2ap.typeOfError",
        FT_UINT32, BASE_DEC, VALS(m2ap_TypeOfError_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_pLMN_Identity,
      { "pLMN-Identity", "m2ap.pLMN_Identity",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_eUTRANcellIdentifier,
      { "eUTRANcellIdentifier", "m2ap.eUTRANcellIdentifier",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_macro_eNB_ID,
      { "macro-eNB-ID", "m2ap.macro_eNB_ID",
        FT_BYTES, BASE_NONE, NULL, 0,
        "BIT_STRING_SIZE_20", HFILL }},
    { &hf_m2ap_mbsfnSynchronisationArea,
      { "mbsfnSynchronisationArea", "m2ap.mbsfnSynchronisationArea",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBSFN_SynchronisationArea_ID", HFILL }},
    { &hf_m2ap_mbmsServiceAreaList,
      { "mbmsServiceAreaList", "m2ap.mbmsServiceAreaList",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBMS_Service_Area_ID_List", HFILL }},
    { &hf_m2ap_mBMSConfigData,
      { "mBMSConfigData", "m2ap.mBMSConfigData_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "ENB_MBMS_Configuration_data_Item", HFILL }},
    { &hf_m2ap_mBMS_E_RAB_MaximumBitrateDL,
      { "mBMS-E-RAB-MaximumBitrateDL", "m2ap.mBMS_E_RAB_MaximumBitrateDL",
        FT_UINT64, BASE_DEC|BASE_UNIT_STRING, &units_bit_sec, 0,
        "BitRate", HFILL }},
    { &hf_m2ap_mBMS_E_RAB_GuaranteedBitrateDL,
      { "mBMS-E-RAB-GuaranteedBitrateDL", "m2ap.mBMS_E_RAB_GuaranteedBitrateDL",
        FT_UINT64, BASE_DEC|BASE_UNIT_STRING, &units_bit_sec, 0,
        "BitRate", HFILL }},
    { &hf_m2ap_eNB_ID,
      { "eNB-ID", "m2ap.eNB_ID",
        FT_UINT32, BASE_DEC, VALS(m2ap_ENB_ID_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_mCE_ID,
      { "mCE-ID", "m2ap.mCE_ID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Cell_List_item,
      { "ECGI", "m2ap.ECGI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_qCI,
      { "qCI", "m2ap.qCI",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_gbrQosInformation,
      { "gbrQosInformation", "m2ap.gbrQosInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GBR_QosInformation", HFILL }},
    { &hf_m2ap_allocationAndRetentionPriority,
      { "allocationAndRetentionPriority", "m2ap.allocationAndRetentionPriority_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_eNB_MBMS_M2AP_ID,
      { "eNB-MBMS-M2AP-ID", "m2ap.eNB_MBMS_M2AP_ID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_mCE_MBMS_M2AP_ID,
      { "mCE-MBMS-M2AP-ID", "m2ap.mCE_MBMS_M2AP_ID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Service_Area_ID_List_item,
      { "MBMS-Service-Area", "m2ap.MBMS_Service_Area",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMSsessionListPerPMCH_Item_item,
      { "MBMSsessionListPerPMCH-Item item", "m2ap.MBMSsessionListPerPMCH_Item_item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_tmgi,
      { "tmgi", "m2ap.tmgi_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_lcid,
      { "lcid", "m2ap.lcid",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item,
      { "MBMSsessionsToBeSuspendedListPerPMCH-Item item", "m2ap.MBMSsessionsToBeSuspendedListPerPMCH_Item_item_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_radioframeAllocationPeriod,
      { "radioframeAllocationPeriod", "m2ap.radioframeAllocationPeriod",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_radioframeAllocationPeriod_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_radioframeAllocationOffset,
      { "radioframeAllocationOffset", "m2ap.radioframeAllocationOffset",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_7", HFILL }},
    { &hf_m2ap_subframeAllocation,
      { "subframeAllocation", "m2ap.subframeAllocation",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_subframeAllocation_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_oneFrame,
      { "oneFrame", "m2ap.oneFrame",
        FT_BYTES, BASE_NONE, NULL, 0,
        "BIT_STRING_SIZE_6", HFILL }},
    { &hf_m2ap_fourFrames,
      { "fourFrames", "m2ap.fourFrames",
        FT_BYTES, BASE_NONE, NULL, 0,
        "BIT_STRING_SIZE_24", HFILL }},
    { &hf_m2ap_mbsfnArea,
      { "mbsfnArea", "m2ap.mbsfnArea",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBSFN_Area_ID", HFILL }},
    { &hf_m2ap_pdcchLength,
      { "pdcchLength", "m2ap.pdcchLength",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_pdcchLength_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_repetitionPeriod,
      { "repetitionPeriod", "m2ap.repetitionPeriod",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_repetitionPeriod_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_offset,
      { "offset", "m2ap.offset",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_10", HFILL }},
    { &hf_m2ap_modificationPeriod,
      { "modificationPeriod", "m2ap.modificationPeriod",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_modificationPeriod_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_subframeAllocationInfo,
      { "subframeAllocationInfo", "m2ap.subframeAllocationInfo",
        FT_BYTES, BASE_NONE, NULL, 0,
        "BIT_STRING_SIZE_6", HFILL }},
    { &hf_m2ap_modulationAndCodingScheme,
      { "modulationAndCodingScheme", "m2ap.modulationAndCodingScheme",
        FT_UINT32, BASE_DEC, VALS(m2ap_T_modulationAndCodingScheme_vals), 0,
        NULL, HFILL }},
    { &hf_m2ap_cellInformationList,
      { "cellInformationList", "m2ap.cellInformationList",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Cell_Information_List", HFILL }},
    { &hf_m2ap_allocatedSubframesEnd,
      { "allocatedSubframesEnd", "m2ap.allocatedSubframesEnd",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_dataMCS,
      { "dataMCS", "m2ap.dataMCS",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_28", HFILL }},
    { &hf_m2ap_mchSchedulingPeriod,
      { "mchSchedulingPeriod", "m2ap.mchSchedulingPeriod",
        FT_UINT32, BASE_DEC, VALS(m2ap_MCH_Scheduling_Period_vals), 0,
        "MCH_Scheduling_Period", HFILL }},
    { &hf_m2ap_mbmsCellList,
      { "mbmsCellList", "m2ap.mbmsCellList",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBMS_Cell_List", HFILL }},
    { &hf_m2ap_mbms_E_RAB_QoS_Parameters,
      { "mbms-E-RAB-QoS-Parameters", "m2ap.mbms_E_RAB_QoS_Parameters_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_pLMNidentity,
      { "pLMNidentity", "m2ap.pLMNidentity",
        FT_BYTES, BASE_NONE, NULL, 0,
        "PLMN_Identity", HFILL }},
    { &hf_m2ap_serviceID,
      { "serviceID", "m2ap.serviceID",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_3", HFILL }},
    { &hf_m2ap_iPMCAddress,
      { "iPMCAddress", "m2ap.iPMCAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "IPAddress", HFILL }},
    { &hf_m2ap_iPSourceAddress,
      { "iPSourceAddress", "m2ap.iPSourceAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "IPAddress", HFILL }},
    { &hf_m2ap_gTP_TEID,
      { "gTP-TEID", "m2ap.gTP_TEID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_protocolIEs,
      { "protocolIEs", "m2ap.protocolIEs",
        FT_UINT32, BASE_DEC, NULL, 0,
        "ProtocolIE_Container", HFILL }},
    { &hf_m2ap_MBSFN_Area_Configuration_List_item,
      { "ProtocolIE-Container", "m2ap.ProtocolIE_Container",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_PMCH_Configuration_List_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_pmch_Configuration,
      { "pmch-Configuration", "m2ap.pmch_Configuration_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_mbms_Session_List,
      { "mbms-Session-List", "m2ap.mbms_Session_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBMSsessionListPerPMCH_Item", HFILL }},
    { &hf_m2ap_MBSFN_Subframe_ConfigurationList_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Suspension_Notification_List_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_sfn,
      { "sfn", "m2ap.sfn",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_mbms_Sessions_To_Be_Suspended_List,
      { "mbms-Sessions-To-Be-Suspended-List", "m2ap.mbms_Sessions_To_Be_Suspended_List",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBMSsessionsToBeSuspendedListPerPMCH_Item", HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_List_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_m2_Interface,
      { "m2-Interface", "m2ap.m2_Interface",
        FT_UINT32, BASE_DEC, VALS(m2ap_ResetAll_vals), 0,
        "ResetAll", HFILL }},
    { &hf_m2ap_partOfM2_Interface,
      { "partOfM2-Interface", "m2ap.partOfM2_Interface",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MBMS_Service_associatedLogicalM2_ConnectionListRes", HFILL }},
    { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck_item,
      { "ProtocolIE-Single-Container", "m2ap.ProtocolIE_Single_Container_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_privateIEs,
      { "privateIEs", "m2ap.privateIEs",
        FT_UINT32, BASE_DEC, NULL, 0,
        "PrivateIE_Container", HFILL }},
    { &hf_m2ap_MBMS_Counting_Request_Session_item,
      { "ProtocolIE-Container", "m2ap.ProtocolIE_Container",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_MBMS_Counting_Result_List_item,
      { "ProtocolIE-Container", "m2ap.ProtocolIE_Container",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_countingResult,
      { "countingResult", "m2ap.countingResult",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_Overload_Status_Per_PMCH_List_item,
      { "ProtocolIE-Container", "m2ap.ProtocolIE_Container",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_Active_MBMS_Session_List_item,
      { "ProtocolIE-Container", "m2ap.ProtocolIE_Container",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_initiatingMessage,
      { "initiatingMessage", "m2ap.initiatingMessage_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_successfulOutcome,
      { "successfulOutcome", "m2ap.successfulOutcome_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_unsuccessfulOutcome,
      { "unsuccessfulOutcome", "m2ap.unsuccessfulOutcome_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_m2ap_initiatingMessage_value,
      { "value", "m2ap.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "InitiatingMessage_value", HFILL }},
    { &hf_m2ap_successfulOutcome_value,
      { "value", "m2ap.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "SuccessfulOutcome_value", HFILL }},
    { &hf_m2ap_unsuccessfulOutcome_value,
      { "value", "m2ap.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "UnsuccessfulOutcome_value", HFILL }},

/*--- End of included file: packet-m2ap-hfarr.c ---*/
#line 156 "./asn1/m2ap/packet-m2ap-template.c"
  };

  /* List of subtrees */
  static gint *ett[] = {
    &ett_m2ap,
    &ett_m2ap_PLMN_Identity,
    &ett_m2ap_IPAddress,

/*--- Included file: packet-m2ap-ettarr.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-ettarr.c"
    &ett_m2ap_PrivateIE_ID,
    &ett_m2ap_ProtocolIE_Container,
    &ett_m2ap_ProtocolIE_Field,
    &ett_m2ap_ProtocolExtensionContainer,
    &ett_m2ap_ProtocolExtensionField,
    &ett_m2ap_PrivateIE_Container,
    &ett_m2ap_PrivateIE_Field,
    &ett_m2ap_AllocationAndRetentionPriority,
    &ett_m2ap_Cause,
    &ett_m2ap_Cell_Information,
    &ett_m2ap_Cell_Information_List,
    &ett_m2ap_CriticalityDiagnostics,
    &ett_m2ap_CriticalityDiagnostics_IE_List,
    &ett_m2ap_CriticalityDiagnostics_IE_List_item,
    &ett_m2ap_ECGI,
    &ett_m2ap_ENB_ID,
    &ett_m2ap_ENB_MBMS_Configuration_data_Item,
    &ett_m2ap_ENB_MBMS_Configuration_data_ConfigUpdate_Item,
    &ett_m2ap_GBR_QosInformation,
    &ett_m2ap_GlobalENB_ID,
    &ett_m2ap_GlobalMCE_ID,
    &ett_m2ap_MBMS_Cell_List,
    &ett_m2ap_MBMS_E_RAB_QoS_Parameters,
    &ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionItem,
    &ett_m2ap_MBMS_Service_Area_ID_List,
    &ett_m2ap_MBMSsessionListPerPMCH_Item,
    &ett_m2ap_MBMSsessionListPerPMCH_Item_item,
    &ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item,
    &ett_m2ap_MBMSsessionsToBeSuspendedListPerPMCH_Item_item,
    &ett_m2ap_MBSFN_Subframe_Configuration,
    &ett_m2ap_T_subframeAllocation,
    &ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item,
    &ett_m2ap_PMCH_Configuration,
    &ett_m2ap_SC_PTM_Information,
    &ett_m2ap_TMGI,
    &ett_m2ap_TNL_Information,
    &ett_m2ap_SessionStartRequest,
    &ett_m2ap_SessionStartResponse,
    &ett_m2ap_SessionStartFailure,
    &ett_m2ap_SessionStopRequest,
    &ett_m2ap_SessionStopResponse,
    &ett_m2ap_SessionUpdateRequest,
    &ett_m2ap_SessionUpdateResponse,
    &ett_m2ap_SessionUpdateFailure,
    &ett_m2ap_MbmsSchedulingInformation,
    &ett_m2ap_MBSFN_Area_Configuration_List,
    &ett_m2ap_PMCH_Configuration_List,
    &ett_m2ap_PMCH_Configuration_Item,
    &ett_m2ap_MBSFN_Subframe_ConfigurationList,
    &ett_m2ap_MBMS_Suspension_Notification_List,
    &ett_m2ap_MBMS_Suspension_Notification_Item,
    &ett_m2ap_MbmsSchedulingInformationResponse,
    &ett_m2ap_M2SetupRequest,
    &ett_m2ap_ENB_MBMS_Configuration_data_List,
    &ett_m2ap_M2SetupResponse,
    &ett_m2ap_MCCHrelatedBCCH_ConfigPerMBSFNArea,
    &ett_m2ap_M2SetupFailure,
    &ett_m2ap_ENBConfigurationUpdate,
    &ett_m2ap_ENB_MBMS_Configuration_data_List_ConfigUpdate,
    &ett_m2ap_ENBConfigurationUpdateAcknowledge,
    &ett_m2ap_ENBConfigurationUpdateFailure,
    &ett_m2ap_MCEConfigurationUpdate,
    &ett_m2ap_MCEConfigurationUpdateAcknowledge,
    &ett_m2ap_MCEConfigurationUpdateFailure,
    &ett_m2ap_ErrorIndication,
    &ett_m2ap_Reset,
    &ett_m2ap_ResetType,
    &ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListRes,
    &ett_m2ap_ResetAcknowledge,
    &ett_m2ap_MBMS_Service_associatedLogicalM2_ConnectionListResAck,
    &ett_m2ap_PrivateMessage,
    &ett_m2ap_MbmsServiceCountingRequest,
    &ett_m2ap_MBMS_Counting_Request_Session,
    &ett_m2ap_MBMS_Counting_Request_SessionIE,
    &ett_m2ap_MbmsServiceCountingResponse,
    &ett_m2ap_MbmsServiceCountingFailure,
    &ett_m2ap_MbmsServiceCountingResultsReport,
    &ett_m2ap_MBMS_Counting_Result_List,
    &ett_m2ap_MBMS_Counting_Result,
    &ett_m2ap_MbmsOverloadNotification,
    &ett_m2ap_Overload_Status_Per_PMCH_List,
    &ett_m2ap_Active_MBMS_Session_List,
    &ett_m2ap_M2AP_PDU,
    &ett_m2ap_InitiatingMessage,
    &ett_m2ap_SuccessfulOutcome,
    &ett_m2ap_UnsuccessfulOutcome,

/*--- End of included file: packet-m2ap-ettarr.c ---*/
#line 164 "./asn1/m2ap/packet-m2ap-template.c"
  };

  expert_module_t* expert_m2ap;

  static ei_register_info ei[] = {
    { &ei_m2ap_invalid_ip_address_len, { "m2ap.invalid_ip_address_len", PI_MALFORMED, PI_ERROR, "Invalid IP address length", EXPFILL }}
  };

  /* Register protocol */
  proto_m2ap = proto_register_protocol(PNAME, PSNAME, PFNAME);
  /* Register fields and subtrees */
  proto_register_field_array(proto_m2ap, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));
  expert_m2ap = expert_register_protocol(proto_m2ap);
  expert_register_field_array(expert_m2ap, ei, array_length(ei));
  /* Register dissector */
  m2ap_handle = register_dissector(PFNAME, dissect_m2ap, proto_m2ap);

  /* Register dissector tables */
  m2ap_ies_dissector_table = register_dissector_table("m2ap.ies", "M2AP-PROTOCOL-IES", proto_m2ap, FT_UINT32, BASE_DEC);
  m2ap_extension_dissector_table = register_dissector_table("m2ap.extension", "M2AP-PROTOCOL-EXTENSION", proto_m2ap, FT_UINT32, BASE_DEC);
  m2ap_proc_imsg_dissector_table = register_dissector_table("m2ap.proc.imsg", "M2AP-ELEMENTARY-PROCEDURE InitiatingMessage", proto_m2ap, FT_UINT32, BASE_DEC);
  m2ap_proc_sout_dissector_table = register_dissector_table("m2ap.proc.sout", "M2AP-ELEMENTARY-PROCEDURE SuccessfulOutcome", proto_m2ap, FT_UINT32, BASE_DEC);
  m2ap_proc_uout_dissector_table = register_dissector_table("m2ap.proc.uout", "M2AP-ELEMENTARY-PROCEDURE UnsuccessfulOutcome", proto_m2ap, FT_UINT32, BASE_DEC);
}

void
proto_reg_handoff_m2ap(void)
{
  dissector_add_uint("sctp.ppi", PROTO_3GPP_M2AP_PROTOCOL_ID, m2ap_handle);
  dissector_add_uint("sctp.port", M2AP_PORT, m2ap_handle);

/*--- Included file: packet-m2ap-dis-tab.c ---*/
#line 1 "./asn1/m2ap/packet-m2ap-dis-tab.c"
  dissector_add_uint("m2ap.ies", id_MCE_MBMS_M2AP_ID, create_dissector_handle(dissect_MCE_MBMS_M2AP_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENB_MBMS_M2AP_ID, create_dissector_handle(dissect_ENB_MBMS_M2AP_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_TMGI, create_dissector_handle(dissect_TMGI_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Session_ID, create_dissector_handle(dissect_MBMS_Session_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Service_Area, create_dissector_handle(dissect_MBMS_Service_Area_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_TNL_Information, create_dissector_handle(dissect_TNL_Information_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_CriticalityDiagnostics, create_dissector_handle(dissect_CriticalityDiagnostics_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_Cause, create_dissector_handle(dissect_Cause_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBSFN_Area_Configuration_List, create_dissector_handle(dissect_MBSFN_Area_Configuration_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_PMCH_Configuration_List, create_dissector_handle(dissect_PMCH_Configuration_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_PMCH_Configuration_Item, create_dissector_handle(dissect_PMCH_Configuration_Item_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_GlobalENB_ID, create_dissector_handle(dissect_GlobalENB_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENBname, create_dissector_handle(dissect_ENBname_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENB_MBMS_Configuration_data_List, create_dissector_handle(dissect_ENB_MBMS_Configuration_data_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENB_MBMS_Configuration_data_Item, create_dissector_handle(dissect_ENB_MBMS_Configuration_data_Item_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_GlobalMCE_ID, create_dissector_handle(dissect_GlobalMCE_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MCEname, create_dissector_handle(dissect_MCEname_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MCCHrelatedBCCH_ConfigPerMBSFNArea, create_dissector_handle(dissect_MCCHrelatedBCCH_ConfigPerMBSFNArea_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item, create_dissector_handle(dissect_MCCHrelatedBCCH_ConfigPerMBSFNArea_Item_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_TimeToWait, create_dissector_handle(dissect_TimeToWait_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBSFN_Subframe_Configuration_List, create_dissector_handle(dissect_MBSFN_Subframe_ConfigurationList_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBSFN_Subframe_Configuration_Item, create_dissector_handle(dissect_MBSFN_Subframe_Configuration_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_Common_Subframe_Allocation_Period, create_dissector_handle(dissect_Common_Subframe_Allocation_Period_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MCCH_Update_Time, create_dissector_handle(dissect_MCCH_Update_Time_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENB_MBMS_Configuration_data_List_ConfigUpdate, create_dissector_handle(dissect_ENB_MBMS_Configuration_data_List_ConfigUpdate_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ENB_MBMS_Configuration_data_ConfigUpdate_Item, create_dissector_handle(dissect_ENB_MBMS_Configuration_data_ConfigUpdate_Item_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Service_associatedLogicalM2_ConnectionItem, create_dissector_handle(dissect_MBMS_Service_associatedLogicalM2_ConnectionItem_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBSFN_Area_ID, create_dissector_handle(dissect_MBSFN_Area_ID_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_ResetType, create_dissector_handle(dissect_ResetType_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Service_associatedLogicalM2_ConnectionListResAck, create_dissector_handle(dissect_MBMS_Service_associatedLogicalM2_ConnectionListResAck_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Counting_Request_Session, create_dissector_handle(dissect_MBMS_Counting_Request_Session_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Counting_Request_Session_Item, create_dissector_handle(dissect_MBMS_Counting_Request_SessionIE_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Counting_Result_List, create_dissector_handle(dissect_MBMS_Counting_Result_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Counting_Result_Item, create_dissector_handle(dissect_MBMS_Counting_Result_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_Alternative_TNL_Information, create_dissector_handle(dissect_TNL_Information_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_Overload_Status_Per_PMCH_List, create_dissector_handle(dissect_Overload_Status_Per_PMCH_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_PMCH_Overload_Status, create_dissector_handle(dissect_PMCH_Overload_Status_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_Active_MBMS_Session_List, create_dissector_handle(dissect_Active_MBMS_Session_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Suspension_Notification_List, create_dissector_handle(dissect_MBMS_Suspension_Notification_List_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_MBMS_Suspension_Notification_Item, create_dissector_handle(dissect_MBMS_Suspension_Notification_Item_PDU, proto_m2ap));
  dissector_add_uint("m2ap.ies", id_SC_PTM_Information, create_dissector_handle(dissect_SC_PTM_Information_PDU, proto_m2ap));
  dissector_add_uint("m2ap.extension", id_Modulation_Coding_Scheme2, create_dissector_handle(dissect_Modulation_Coding_Scheme2_PDU, proto_m2ap));
  dissector_add_uint("m2ap.extension", id_MCH_Scheduling_PeriodExtended, create_dissector_handle(dissect_MCH_Scheduling_PeriodExtended_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_sessionStart, create_dissector_handle(dissect_SessionStartRequest_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_sessionStart, create_dissector_handle(dissect_SessionStartResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_sessionStart, create_dissector_handle(dissect_SessionStartFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_sessionStop, create_dissector_handle(dissect_SessionStopRequest_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_sessionStop, create_dissector_handle(dissect_SessionStopResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_sessionUpdate, create_dissector_handle(dissect_SessionUpdateRequest_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_sessionUpdate, create_dissector_handle(dissect_SessionUpdateResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_sessionUpdate, create_dissector_handle(dissect_SessionUpdateFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_mbmsSchedulingInformation, create_dissector_handle(dissect_MbmsSchedulingInformation_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_mbmsSchedulingInformation, create_dissector_handle(dissect_MbmsSchedulingInformationResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_m2Setup, create_dissector_handle(dissect_M2SetupRequest_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_m2Setup, create_dissector_handle(dissect_M2SetupResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_m2Setup, create_dissector_handle(dissect_M2SetupFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_eNBConfigurationUpdate, create_dissector_handle(dissect_ENBConfigurationUpdate_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_eNBConfigurationUpdate, create_dissector_handle(dissect_ENBConfigurationUpdateAcknowledge_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_eNBConfigurationUpdate, create_dissector_handle(dissect_ENBConfigurationUpdateFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_mCEConfigurationUpdate, create_dissector_handle(dissect_MCEConfigurationUpdate_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_mCEConfigurationUpdate, create_dissector_handle(dissect_MCEConfigurationUpdateAcknowledge_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_mCEConfigurationUpdate, create_dissector_handle(dissect_MCEConfigurationUpdateFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_errorIndication, create_dissector_handle(dissect_ErrorIndication_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_reset, create_dissector_handle(dissect_Reset_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_reset, create_dissector_handle(dissect_ResetAcknowledge_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_privateMessage, create_dissector_handle(dissect_PrivateMessage_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_mbmsServiceCounting, create_dissector_handle(dissect_MbmsServiceCountingRequest_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.sout", id_mbmsServiceCounting, create_dissector_handle(dissect_MbmsServiceCountingResponse_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.uout", id_mbmsServiceCounting, create_dissector_handle(dissect_MbmsServiceCountingFailure_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_mbmsServiceCountingResultsReport, create_dissector_handle(dissect_MbmsServiceCountingResultsReport_PDU, proto_m2ap));
  dissector_add_uint("m2ap.proc.imsg", id_mbmsOverloadNotification, create_dissector_handle(dissect_MbmsOverloadNotification_PDU, proto_m2ap));


/*--- End of included file: packet-m2ap-dis-tab.c ---*/
#line 196 "./asn1/m2ap/packet-m2ap-template.c"
}
