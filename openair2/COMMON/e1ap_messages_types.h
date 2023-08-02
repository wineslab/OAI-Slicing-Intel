/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Author and copyright: Laurent Thomas, open-cells.com
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef E1AP_MESSAGES_TYPES_H
#define E1AP_MESSAGES_TYPES_H

#include "common/ngran_types.h"
#include "f1ap_messages_types.h"
#include "ngap_messages_types.h"

#define E1AP_MAX_NUM_TRANSAC_IDS 4
#define E1AP_MAX_NUM_PLMNS 4
#define E1AP_MAX_NUM_CELL_GROUPS 4
#define E1AP_MAX_NUM_QOS_FLOWS 4
#define E1AP_MAX_NUM_NGRAN_DRB 4
#define E1AP_MAX_NUM_PDU_SESSIONS 4
#define E1AP_MAX_NUM_DRBS 4
#define E1AP_MAX_NUM_DRBS 4
#define E1AP_MAX_NUM_UP_PARAM 4

#define E1AP_SETUP_REQ(mSGpTR)                            (mSGpTR)->ittiMsg.e1ap_setup_req
#define E1AP_SETUP_RESP(mSGpTR)                           (mSGpTR)->ittiMsg.e1ap_setup_resp
#define E1AP_BEARER_CONTEXT_SETUP_REQ(mSGpTR)             (mSGpTR)->ittiMsg.e1ap_bearer_setup_req
#define E1AP_BEARER_CONTEXT_SETUP_RESP(mSGpTR)            (mSGpTR)->ittiMsg.e1ap_bearer_setup_resp
#define E1AP_BEARER_CONTEXT_MODIFICATION_REQ(mSGpTR)      (mSGpTR)->ittiMsg.e1ap_bearer_setup_req

typedef f1ap_net_ip_address_t e1ap_net_ip_address_t;

typedef struct PLMN_ID_s {
  int mcc;
  int mnc;
  int mnc_digit_length;
} PLMN_ID_t;

typedef struct e1ap_setup_req_s {
  uint64_t              gNB_cu_up_id;
  char *                gNB_cu_up_name;
  int                   assoc_id;
  uint64_t              transac_id;
  int                   supported_plmns; 
  PLMN_ID_t             plmns[E1AP_MAX_NUM_PLMNS];
  uint16_t              sctp_in_streams;
  uint16_t              sctp_out_streams;
  uint16_t              default_sctp_stream_id;
  net_ip_address_t CUUP_e1_ip_address;
  net_ip_address_t CUCP_e1_ip_address;
  long                  cn_support;
  uint16_t remotePortF1U;
  char* localAddressF1U;
  uint16_t localPortF1U;
  char* localAddressN3;
  uint16_t localPortN3;
  uint16_t remotePortN3;
} e1ap_setup_req_t;

typedef struct e1ap_setup_resp_s {
  long transac_id;
} e1ap_setup_resp_t;

typedef struct cell_group_s {
  long id;
} cell_group_t;

typedef struct up_params_s {
  in_addr_t tlAddress;
  long teId;
  int cell_group_id;
} up_params_t;

typedef struct drb_to_setup_s {
  long drbId;
  long pDCP_SN_Size_UL;
  long pDCP_SN_Size_DL;
  long rLC_Mode;
  long qci;
  long qosPriorityLevel;
  long pre_emptionCapability;
  long pre_emptionVulnerability;
  in_addr_t tlAddress;
  long teId;
  int numCellGroups;
  cell_group_t cellGroupList[E1AP_MAX_NUM_CELL_GROUPS];
} drb_to_setup_t;

typedef struct qos_flow_to_setup_s {
  long id;
  fiveQI_type_t fiveQI_type;
  long fiveQI;
  long qoSPriorityLevel;
  long packetDelayBudget;
  long packetError_scalar;
  long packetError_exponent;
  long priorityLevel;
  long pre_emptionCapability;
  long pre_emptionVulnerability;
} qos_flow_to_setup_t;

typedef struct DRB_nGRAN_to_setup_s {
  long id;
  long defaultDRB;
  long sDAP_Header_UL;
  long sDAP_Header_DL;
  long pDCP_SN_Size_UL;
  long pDCP_SN_Size_DL;
  long discardTimer;
  long reorderingTimer;
  long rLC_Mode;
  in_addr_t tlAddress;
  int teId;
  int numDlUpParam;
  up_params_t DlUpParamList[E1AP_MAX_NUM_UP_PARAM];
  int numCellGroups;
  cell_group_t cellGroupList[E1AP_MAX_NUM_CELL_GROUPS];
  int numQosFlow2Setup;
  qos_flow_to_setup_t qosFlows[E1AP_MAX_NUM_QOS_FLOWS];
} DRB_nGRAN_to_setup_t;

typedef struct pdu_session_to_setup_s {
  long sessionId;
  long sessionType;
  uint8_t sst;
  uint8_t sd[3];
  long integrityProtectionIndication;
  long confidentialityProtectionIndication;
  in_addr_t tlAddress;
  in_addr_t tlAddress_dl;
  int32_t teId;
  int32_t teId_dl;
  int tl_port;
  int tl_port_dl;
  long numDRB2Setup;
  DRB_nGRAN_to_setup_t DRBnGRanList[E1AP_MAX_NUM_NGRAN_DRB];
  long numDRB2Modify;
  DRB_nGRAN_to_setup_t DRBnGRanModList[E1AP_MAX_NUM_NGRAN_DRB];
} pdu_session_to_setup_t;

typedef struct e1ap_bearer_setup_req_s {
  uint32_t gNB_cu_cp_ue_id;
  uint32_t gNB_cu_up_ue_id;
  rnti_t   rnti;
  uint64_t cipheringAlgorithm;
  uint64_t integrityProtectionAlgorithm;
  char     encryptionKey[128];
  char     integrityProtectionKey[128];
  long     ueDlAggMaxBitRate;
  PLMN_ID_t servingPLMNid;
  long activityNotificationLevel;
  int numDRBs;
  drb_to_setup_t DRBList[E1AP_MAX_NUM_DRBS];
  int numPDUSessions;
  pdu_session_to_setup_t pduSession[E1AP_MAX_NUM_PDU_SESSIONS];
  int numPDUSessionsMod;
  pdu_session_to_setup_t pduSessionMod[E1AP_MAX_NUM_PDU_SESSIONS];
} e1ap_bearer_setup_req_t;

typedef struct e1ap_bearer_release_cmd_s {
  uint32_t gNB_cu_cp_ue_id;
  uint32_t gNB_cu_up_ue_id;
  long cause_type;
  long cause;
} e1ap_bearer_release_cmd_t;

typedef struct drb_setup_s {
  int drbId;
  in_addr_t tlAddress;
  int teId;
  int numUpParam;
  up_params_t UpParamList[E1AP_MAX_NUM_UP_PARAM];
} drb_setup_t;

typedef struct qos_flow_setup_s {
  long id;
} qos_flow_setup_t;

typedef struct DRB_nGRAN_setup_s {
  long id;
  int numUpParam;
  up_params_t UpParamList[E1AP_MAX_NUM_UP_PARAM];
  int numQosFlowSetup;
  qos_flow_setup_t qosFlows[E1AP_MAX_NUM_QOS_FLOWS];
} DRB_nGRAN_setup_t;

typedef struct DRB_nGRAN_failed_s {
  long id;
  long cause_type;
  long cause;
} DRB_nGRAN_failed_t;

typedef struct pdu_session_setup_s {
  long id;
  uint8_t sst;
  uint8_t sd[3];
  in_addr_t tlAddress;
  long teId;
  int numDRBSetup;
  DRB_nGRAN_setup_t DRBnGRanList[E1AP_MAX_NUM_NGRAN_DRB];
  int numDRBFailed;
  DRB_nGRAN_failed_t DRBnGRanFailedList[E1AP_MAX_NUM_NGRAN_DRB];
} pdu_session_setup_t;

typedef struct e1ap_bearer_setup_resp_s {
  uint64_t gNB_cu_cp_ue_id;
  uint64_t gNB_cu_up_ue_id;
  int numDRBs;
  drb_setup_t DRBList[E1AP_MAX_NUM_DRBS];
  int numPDUSessions;
  pdu_session_setup_t pduSession[E1AP_MAX_NUM_PDU_SESSIONS];
} e1ap_bearer_setup_resp_t;

#endif /* E1AP_MESSAGES_TYPES_H */
