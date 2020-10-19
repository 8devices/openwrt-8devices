/* packet-x2ap.c
 * Routines for dissecting Evolved Universal Terrestrial Radio Access Network (EUTRAN);
 * X2 Application Protocol (X2AP);
 * 3GPP TS 36.423 packet dissection
 * Copyright 2007-2014, Anders Broman <anders.broman@ericsson.com>
 * Copyright 2016, Pascal Quantin <pacal.quantin@gmail.com>
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
 * Ref:
 * 3GPP TS 36.423 V13.6.0 (2017-01)
 */

#include "config.h"

#include <epan/packet.h>
#include <epan/tfs.h>
#include <epan/asn1.h>
#include <epan/prefs.h>
#include <epan/sctpppids.h>
#include <epan/proto_data.h>

#include "packet-per.h"
#include "packet-e212.h"
#include "packet-lte-rrc.h"

#ifdef _MSC_VER
/* disable: "warning C4146: unary minus operator applied to unsigned type, result still unsigned" */
#pragma warning(disable:4146)
#endif

#define PNAME  "EUTRAN X2 Application Protocol (X2AP)"
#define PSNAME "X2AP"
#define PFNAME "x2ap"

void proto_register_x2ap(void);

/* Dissector will use SCTP PPID 27 or SCTP port. IANA assigned port = 36422 */
#define SCTP_PORT_X2AP	36422

#include "packet-x2ap-val.h"

/* Initialize the protocol and registered fields */
static int proto_x2ap = -1;
static int hf_x2ap_transportLayerAddressIPv4 = -1;
static int hf_x2ap_transportLayerAddressIPv6 = -1;
static int hf_x2ap_ReportCharacteristics_PRBPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_TNLLoadIndPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_HWLoadIndPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_CompositeAvailableCapacityPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_ABSStatusPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_RSRPMeasurementReportPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_CSIReportPeriodic = -1;
static int hf_x2ap_ReportCharacteristics_Reserved = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_PRBPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_TNLLoadIndPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_HWLoadIndPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_CompositeAvailableCapacityPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_ABSStatusPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_RSRPMeasurementReportPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_CSIReportPeriodic = -1;
static int hf_x2ap_measurementFailedReportCharacteristics_Reserved = -1;
static int hf_x2ap_eUTRANTraceID_TraceID = -1;
static int hf_x2ap_eUTRANTraceID_TraceRecordingSessionReference = -1;
static int hf_x2ap_interfacesToTrace_S1_MME = -1;
static int hf_x2ap_interfacesToTrace_X2 = -1;
static int hf_x2ap_interfacesToTrace_Uu = -1;
static int hf_x2ap_interfacesToTrace_Reserved = -1;
static int hf_x2ap_traceCollectionEntityIPAddress_IPv4 = -1;
static int hf_x2ap_traceCollectionEntityIPAddress_IPv6 = -1;
static int hf_x2ap_encryptionAlgorithms_EEA1 = -1;
static int hf_x2ap_encryptionAlgorithms_EEA2 = -1;
static int hf_x2ap_encryptionAlgorithms_EEA3 = -1;
static int hf_x2ap_encryptionAlgorithms_Reserved = -1;
static int hf_x2ap_integrityProtectionAlgorithms_EIA1 = -1;
static int hf_x2ap_integrityProtectionAlgorithms_EIA2 = -1;
static int hf_x2ap_integrityProtectionAlgorithms_EIA3 = -1;
static int hf_x2ap_integrityProtectionAlgorithms_Reserved = -1;
static int hf_x2ap_measurementsToActivate_M1 = -1;
static int hf_x2ap_measurementsToActivate_M2 = -1;
static int hf_x2ap_measurementsToActivate_M3 = -1;
static int hf_x2ap_measurementsToActivate_M4 = -1;
static int hf_x2ap_measurementsToActivate_M5 = -1;
static int hf_x2ap_measurementsToActivate_LoggingM1FromEventTriggered = -1;
static int hf_x2ap_measurementsToActivate_M6 = -1;
static int hf_x2ap_measurementsToActivate_M7 = -1;
static int hf_x2ap_MDT_Location_Info_GNSS = -1;
static int hf_x2ap_MDT_Location_Info_E_CID = -1;
static int hf_x2ap_MDT_Location_Info_Reserved = -1;
static int hf_x2ap_MDT_transmissionModes_tm1 = -1;
static int hf_x2ap_MDT_transmissionModes_tm2 = -1;
static int hf_x2ap_MDT_transmissionModes_tm3 = -1;
static int hf_x2ap_MDT_transmissionModes_tm4 = -1;
static int hf_x2ap_MDT_transmissionModes_tm6 = -1;
static int hf_x2ap_MDT_transmissionModes_tm8 = -1;
static int hf_x2ap_MDT_transmissionModes_tm9 = -1;
static int hf_x2ap_MDT_transmissionModes_tm10 = -1;
#include "packet-x2ap-hf.c"

/* Initialize the subtree pointers */
static int ett_x2ap = -1;
static int ett_x2ap_TransportLayerAddress = -1;
static int ett_x2ap_PLMN_Identity = -1;
static int ett_x2ap_TargeteNBtoSource_eNBTransparentContainer = -1;
static int ett_x2ap_RRC_Context = -1;
static int ett_x2ap_UE_HistoryInformationFromTheUE = -1;
static int ett_x2ap_ReportCharacteristics = -1;
static int ett_x2ap_measurementFailedReportCharacteristics = -1;
static int ett_x2ap_UE_RLF_Report_Container = -1;
static int ett_x2ap_UE_RLF_Report_Container_for_extended_bands = -1;
static int ett_x2ap_MeNBtoSeNBContainer = -1;
static int ett_x2ap_SeNBtoMeNBContainer = -1;
static int ett_x2ap_EUTRANTraceID = -1;
static int ett_x2ap_InterfacesToTrace = -1;
static int ett_x2ap_TraceCollectionEntityIPAddress = -1;
static int ett_x2ap_EncryptionAlgorithms = -1;
static int ett_x2ap_IntegrityProtectionAlgorithms = -1;
static int ett_x2ap_MeasurementsToActivate = -1;
static int ett_x2ap_MDT_Location_Info = -1;
static int ett_x2ap_transmissionModes = -1;
static int ett_x2ap_X2AP_Message = -1;
#include "packet-x2ap-ett.c"

struct x2ap_private_data {
  guint32 procedure_code;
  guint32 protocol_ie_id;
};

enum {
  X2AP_RRC_CONTEXT_LTE,
  X2AP_RRC_CONTEXT_NBIOT
};

static const enum_val_t x2ap_rrc_context_vals[] = {
  {"lte", "LTE", X2AP_RRC_CONTEXT_LTE},
  {"nb-iot","NB-IoT", X2AP_RRC_CONTEXT_NBIOT},
  {NULL, NULL, -1}
};

/* Global variables */
static guint gbl_x2apSctpPort=SCTP_PORT_X2AP;
static gint g_x2ap_dissect_rrc_context_as = X2AP_RRC_CONTEXT_LTE;

/* Dissector tables */
static dissector_table_t x2ap_ies_dissector_table;
static dissector_table_t x2ap_extension_dissector_table;
static dissector_table_t x2ap_proc_imsg_dissector_table;
static dissector_table_t x2ap_proc_sout_dissector_table;
static dissector_table_t x2ap_proc_uout_dissector_table;

static int dissect_ProtocolIEFieldValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_ProtocolExtensionFieldExtensionValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_InitiatingMessageValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_SuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_UnsuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *);
static int dissect_X2AP_PDU_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_);
void proto_reg_handoff_x2ap(void);

static dissector_handle_t x2ap_handle;

static const true_false_string x2ap_tfs_failed_succeeded = {
  "Failed",
  "Succeeded"
};

static const true_false_string x2ap_tfs_interfacesToTrace = {
  "Should be traced",
  "Should not be traced"
};

static const true_false_string x2ap_tfs_activate_do_not_activate = {
  "Activate",
  "Do not activate"
};

static void
x2ap_Time_UE_StayedInCell_EnhancedGranularity_fmt(gchar *s, guint32 v)
{
  g_snprintf(s, ITEM_LABEL_LENGTH, "%.1fs", ((float)v)/10);
}

static void
x2ap_handoverTriggerChange_fmt(gchar *s, guint32 v)
{
  g_snprintf(s, ITEM_LABEL_LENGTH, "%.1fdB (%d)", ((float)v)/2, (gint32)v);
}

static void
x2ap_Threshold_RSRP_fmt(gchar *s, guint32 v)
{
  g_snprintf(s, ITEM_LABEL_LENGTH, "%ddBm (%u)", (gint32)v-140, v);
}

static void
x2ap_Threshold_RSRQ_fmt(gchar *s, guint32 v)
{
  g_snprintf(s, ITEM_LABEL_LENGTH, "%.1fdB (%u)", ((float)v/2)-20, v);
}

static struct x2ap_private_data*
x2ap_get_private_data(packet_info *pinfo)
{
  struct x2ap_private_data *x2ap_data = (struct x2ap_private_data*)p_get_proto_data(pinfo->pool, pinfo, proto_x2ap, 0);
  if (!x2ap_data) {
    x2ap_data = wmem_new0(pinfo->pool, struct x2ap_private_data);
    p_add_proto_data(pinfo->pool, pinfo, proto_x2ap, 0, x2ap_data);
  }
  return x2ap_data;
}

#include "packet-x2ap-fn.c"

static int dissect_ProtocolIEFieldValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  struct x2ap_private_data *x2ap_data = x2ap_get_private_data(pinfo);

  return (dissector_try_uint(x2ap_ies_dissector_table, x2ap_data->protocol_ie_id, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int dissect_ProtocolExtensionFieldExtensionValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  struct x2ap_private_data *x2ap_data = x2ap_get_private_data(pinfo);

  return (dissector_try_uint(x2ap_extension_dissector_table, x2ap_data->protocol_ie_id, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int dissect_InitiatingMessageValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  struct x2ap_private_data *x2ap_data = x2ap_get_private_data(pinfo);

  return (dissector_try_uint(x2ap_proc_imsg_dissector_table, x2ap_data->procedure_code, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int dissect_SuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  struct x2ap_private_data *x2ap_data = x2ap_get_private_data(pinfo);

  return (dissector_try_uint(x2ap_proc_sout_dissector_table, x2ap_data->procedure_code, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int dissect_UnsuccessfulOutcomeValue(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  struct x2ap_private_data *x2ap_data = x2ap_get_private_data(pinfo);

  return (dissector_try_uint(x2ap_proc_uout_dissector_table, x2ap_data->procedure_code, tvb, pinfo, tree)) ? tvb_captured_length(tvb) : 0;
}

static int
dissect_x2ap(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
  proto_item *x2ap_item;
  proto_tree *x2ap_tree;

  /* make entry in the Protocol column on summary display */
  col_set_str(pinfo->cinfo, COL_PROTOCOL, "X2AP");
  col_clear_fence(pinfo->cinfo, COL_INFO);
  col_clear(pinfo->cinfo, COL_INFO);

  /* create the x2ap protocol tree */
  x2ap_item = proto_tree_add_item(tree, proto_x2ap, tvb, 0, -1, ENC_NA);
  x2ap_tree = proto_item_add_subtree(x2ap_item, ett_x2ap);

  return dissect_X2AP_PDU_PDU(tvb, pinfo, x2ap_tree, data);
}

/*--- proto_register_x2ap -------------------------------------------*/
void proto_register_x2ap(void) {

  /* List of fields */

  static hf_register_info hf[] = {
    { &hf_x2ap_transportLayerAddressIPv4,
      { "transportLayerAddress(IPv4)", "x2ap.transportLayerAddressIPv4",
        FT_IPv4, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_transportLayerAddressIPv6,
      { "transportLayerAddress(IPv6)", "x2ap.transportLayerAddressIPv6",
        FT_IPv6, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_PRBPeriodic,
      { "PRBPeriodic", "x2ap.ReportCharacteristics.PRBPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x80000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_TNLLoadIndPeriodic,
      { "TNLLoadIndPeriodic", "x2ap.ReportCharacteristics.TNLLoadIndPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x40000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_HWLoadIndPeriodic,
      { "HWLoadIndPeriodic", "x2ap.ReportCharacteristics.HWLoadIndPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x20000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_CompositeAvailableCapacityPeriodic,
      { "CompositeAvailableCapacityPeriodic", "x2ap.ReportCharacteristics.CompositeAvailableCapacityPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x10000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_ABSStatusPeriodic,
      { "ABSStatusPeriodic", "x2ap.ReportCharacteristics.ABSStatusPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x08000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_RSRPMeasurementReportPeriodic,
      { "RSRPMeasurementReportPeriodic", "x2ap.ReportCharacteristics.RSRPMeasurementReportPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x04000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_CSIReportPeriodic,
      { "CSIReportPeriodic", "x2ap.ReportCharacteristics.CSIReportPeriodic",
        FT_BOOLEAN, 32, TFS(&tfs_requested_not_requested), 0x02000000,
        NULL, HFILL }},
    { &hf_x2ap_ReportCharacteristics_Reserved,
      { "Reserved", "x2ap.ReportCharacteristics.Reserved",
        FT_UINT32, BASE_HEX, NULL, 0x01ffffff,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_PRBPeriodic,
      { "PRBPeriodic", "x2ap.measurementFailedReportCharacteristics.PRBPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x80000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_TNLLoadIndPeriodic,
      { "TNLLoadIndPeriodic", "x2ap.measurementFailedReportCharacteristics.TNLLoadIndPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x40000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_HWLoadIndPeriodic,
      { "HWLoadIndPeriodic", "x2ap.measurementFailedReportCharacteristics.HWLoadIndPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x20000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_CompositeAvailableCapacityPeriodic,
      { "CompositeAvailableCapacityPeriodic", "x2ap.measurementFailedReportCharacteristics.CompositeAvailableCapacityPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x10000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_ABSStatusPeriodic,
      { "ABSStatusPeriodic", "x2ap.measurementFailedReportCharacteristics.ABSStatusPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x08000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_RSRPMeasurementReportPeriodic,
      { "RSRPMeasurementReportPeriodic", "x2ap.measurementFailedReportCharacteristics.RSRPMeasurementReportPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x04000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_CSIReportPeriodic,
      { "CSIReportPeriodic", "x2ap.measurementFailedReportCharacteristics.CSIReportPeriodic",
        FT_BOOLEAN, 32, TFS(&x2ap_tfs_failed_succeeded), 0x02000000,
        NULL, HFILL }},
    { &hf_x2ap_measurementFailedReportCharacteristics_Reserved,
      { "Reserved", "x2ap.measurementFailedReportCharacteristics.Reserved",
        FT_UINT32, BASE_HEX, NULL, 0x01ffffff,
        NULL, HFILL }},
    { &hf_x2ap_eUTRANTraceID_TraceID,
      { "TraceID", "x2ap.eUTRANTraceID.TraceID",
        FT_UINT24, BASE_HEX, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_eUTRANTraceID_TraceRecordingSessionReference,
      { "TraceRecordingSessionReference", "x2ap.eUTRANTraceID.TraceRecordingSessionReference",
        FT_UINT16, BASE_HEX, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_interfacesToTrace_S1_MME,
      { "S1-MME", "x2ap.interfacesToTrace.S1_MME",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_interfacesToTrace), 0x80,
        NULL, HFILL }},
    { &hf_x2ap_interfacesToTrace_X2,
      { "X2", "x2ap.interfacesToTrace.X2",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_interfacesToTrace), 0x40,
        NULL, HFILL }},
    { &hf_x2ap_interfacesToTrace_Uu,
      { "Uu", "x2ap.interfacesToTrace.Uu",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_interfacesToTrace), 0x20,
        NULL, HFILL }},
    { &hf_x2ap_interfacesToTrace_Reserved,
      { "Reserved", "x2ap.interfacesToTrace.Reserved",
        FT_UINT8, BASE_HEX, NULL, 0x1f,
        NULL, HFILL }},
    { &hf_x2ap_traceCollectionEntityIPAddress_IPv4,
      { "IPv4", "x2ap.traceCollectionEntityIPAddress.IPv4",
        FT_IPv4, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_traceCollectionEntityIPAddress_IPv6,
      { "IPv6", "x2ap.traceCollectionEntityIPAddress.IPv6",
        FT_IPv6, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_x2ap_encryptionAlgorithms_EEA1,
      { "128-EEA1", "x2ap.encryptionAlgorithms.EEA1",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x8000,
        NULL, HFILL }},
    { &hf_x2ap_encryptionAlgorithms_EEA2,
      { "128-EEA2", "x2ap.encryptionAlgorithms.EEA2",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x4000,
        NULL, HFILL }},
    { &hf_x2ap_encryptionAlgorithms_EEA3,
      { "128-EEA3", "x2ap.encryptionAlgorithms.EEA3",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x2000,
        NULL, HFILL }},
    { &hf_x2ap_encryptionAlgorithms_Reserved,
      { "Reserved", "x2ap.encryptionAlgorithms.Reserved",
        FT_UINT16, BASE_HEX, NULL, 0x1fff,
        NULL, HFILL }},
    { &hf_x2ap_integrityProtectionAlgorithms_EIA1,
      { "128-EIA1", "x2ap.integrityProtectionAlgorithms.EIA1",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x8000,
        NULL, HFILL }},
    { &hf_x2ap_integrityProtectionAlgorithms_EIA2,
      { "128-EIA2", "x2ap.integrityProtectionAlgorithms.EIA2",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x4000,
        NULL, HFILL }},
    { &hf_x2ap_integrityProtectionAlgorithms_EIA3,
      { "128-EIA3", "x2ap.integrityProtectionAlgorithms.EIA3",
        FT_BOOLEAN, 16, TFS(&tfs_supported_not_supported), 0x2000,
        NULL, HFILL }},
    { &hf_x2ap_integrityProtectionAlgorithms_Reserved,
      { "Reserved", "x2ap.integrityProtectionAlgorithms.Reserved",
        FT_UINT16, BASE_HEX, NULL, 0x1fff,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M1,
      { "M1", "x2ap.measurementsToActivate.M1",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x80,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M2,
      { "M2", "x2ap.measurementsToActivate.M2",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x40,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M3,
      { "M3", "x2ap.measurementsToActivate.M3",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x20,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M4,
      { "M4", "x2ap.measurementsToActivate.M4",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x10,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M5,
      { "M5", "x2ap.measurementsToActivate.M5",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x08,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_LoggingM1FromEventTriggered,
      { "LoggingOfM1FromEventTriggeredMeasurementReports", "x2ap.measurementsToActivate.LoggingM1FromEventTriggered",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x04,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M6,
      { "M6", "x2ap.measurementsToActivate.M6",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x02,
        NULL, HFILL }},
    { &hf_x2ap_measurementsToActivate_M7,
      { "M7", "x2ap.measurementsToActivate.M7",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x01,
        NULL, HFILL }},
    { &hf_x2ap_MDT_Location_Info_GNSS,
      { "GNSS", "x2ap.MDT_Location_Info.GNSS",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x80,
        NULL, HFILL }},
    { &hf_x2ap_MDT_Location_Info_E_CID,
      { "E-CID", "x2ap.MDT_Location_Info.E_CID",
        FT_BOOLEAN, 8, TFS(&x2ap_tfs_activate_do_not_activate), 0x40,
        NULL, HFILL }},
    { &hf_x2ap_MDT_Location_Info_Reserved,
      { "Reserved", "x2ap.MDT_Location_Info.Reserved",
        FT_UINT8, BASE_HEX, NULL, 0x3f,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm1,
      { "TM1", "x2ap.MDT_Location_Info.transmissionModes.tm1",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x80,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm2,
      { "TM2", "x2ap.MDT_Location_Info.transmissionModes.tm2",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x40,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm3,
      { "TM3", "x2ap.MDT_Location_Info.transmissionModes.tm3",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x20,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm4,
      { "TM4", "x2ap.MDT_Location_Info.transmissionModes.tm4",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x10,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm6,
      { "TM6", "x2ap.MDT_Location_Info.transmissionModes.tm6",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x08,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm8,
      { "TM8", "x2ap.MDT_Location_Info.transmissionModes.tm8",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x04,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm9,
      { "TM9", "x2ap.MDT_Location_Info.transmissionModes.tm9",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x02,
        NULL, HFILL }},
    { &hf_x2ap_MDT_transmissionModes_tm10,
      { "TM10", "x2ap.MDT_Location_Info.transmissionModes.tm10",
        FT_BOOLEAN, 8, TFS(&tfs_yes_no), 0x01,
        NULL, HFILL }},
#include "packet-x2ap-hfarr.c"
  };

  /* List of subtrees */
  static gint *ett[] = {
    &ett_x2ap,
    &ett_x2ap_TransportLayerAddress,
    &ett_x2ap_PLMN_Identity,
    &ett_x2ap_TargeteNBtoSource_eNBTransparentContainer,
    &ett_x2ap_RRC_Context,
    &ett_x2ap_UE_HistoryInformationFromTheUE,
    &ett_x2ap_ReportCharacteristics,
    &ett_x2ap_measurementFailedReportCharacteristics,
    &ett_x2ap_UE_RLF_Report_Container,
    &ett_x2ap_UE_RLF_Report_Container_for_extended_bands,
    &ett_x2ap_MeNBtoSeNBContainer,
    &ett_x2ap_SeNBtoMeNBContainer,
    &ett_x2ap_EUTRANTraceID,
    &ett_x2ap_InterfacesToTrace,
    &ett_x2ap_TraceCollectionEntityIPAddress,
    &ett_x2ap_EncryptionAlgorithms,
    &ett_x2ap_IntegrityProtectionAlgorithms,
    &ett_x2ap_MeasurementsToActivate,
    &ett_x2ap_MDT_Location_Info,
    &ett_x2ap_transmissionModes,
    &ett_x2ap_X2AP_Message,
#include "packet-x2ap-ettarr.c"
  };

  module_t *x2ap_module;

  /* Register protocol */
  proto_x2ap = proto_register_protocol(PNAME, PSNAME, PFNAME);
  /* Register fields and subtrees */
  proto_register_field_array(proto_x2ap, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));

  /* Register dissector */
  x2ap_handle = register_dissector("x2ap", dissect_x2ap, proto_x2ap);

  /* Register dissector tables */
  x2ap_ies_dissector_table = register_dissector_table("x2ap.ies", "X2AP-PROTOCOL-IES", proto_x2ap, FT_UINT32, BASE_DEC);
  x2ap_extension_dissector_table = register_dissector_table("x2ap.extension", "X2AP-PROTOCOL-EXTENSION", proto_x2ap, FT_UINT32, BASE_DEC);
  x2ap_proc_imsg_dissector_table = register_dissector_table("x2ap.proc.imsg", "X2AP-ELEMENTARY-PROCEDURE InitiatingMessage", proto_x2ap, FT_UINT32, BASE_DEC);
  x2ap_proc_sout_dissector_table = register_dissector_table("x2ap.proc.sout", "X2AP-ELEMENTARY-PROCEDURE SuccessfulOutcome", proto_x2ap, FT_UINT32, BASE_DEC);
  x2ap_proc_uout_dissector_table = register_dissector_table("x2ap.proc.uout", "X2AP-ELEMENTARY-PROCEDURE UnsuccessfulOutcome", proto_x2ap, FT_UINT32, BASE_DEC);

  /* Register configuration options for ports */
  x2ap_module = prefs_register_protocol(proto_x2ap, proto_reg_handoff_x2ap);

  prefs_register_uint_preference(x2ap_module, "sctp.port",
                                 "X2AP SCTP Port",
                                 "Set the SCTP port for X2AP messages",
                                 10,
                                 &gbl_x2apSctpPort);
  prefs_register_enum_preference(x2ap_module, "dissect_rrc_context_as", "Dissect RRC Context as",
                                 "Select whether RRC Context should be dissected as legacy LTE or NB-IOT",
                                 &g_x2ap_dissect_rrc_context_as, x2ap_rrc_context_vals, FALSE);
}


/*--- proto_reg_handoff_x2ap ---------------------------------------*/
void
proto_reg_handoff_x2ap(void)
{
  static gboolean Initialized=FALSE;
  static guint SctpPort;

  if (!Initialized) {
    dissector_add_for_decode_as("sctp.port", x2ap_handle);
    dissector_add_uint("sctp.ppi", X2AP_PAYLOAD_PROTOCOL_ID, x2ap_handle);
    Initialized=TRUE;
#include "packet-x2ap-dis-tab.c"
  } else {
    if (SctpPort != 0) {
      dissector_delete_uint("sctp.port", SctpPort, x2ap_handle);
    }
  }

  SctpPort=gbl_x2apSctpPort;
  if (SctpPort != 0) {
    dissector_add_uint("sctp.port", SctpPort, x2ap_handle);
  }
}


