/* Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
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

/*! \file RRC/NR/nr_rrc_defs.h
* \brief NR RRC struct definitions and function prototypes
* \author Navid Nikaein, Raymond Knopp, WEI-TAI CHEN
* \date 2010 - 2014, 2018
* \version 1.0
* \company Eurecom, NTSUT
* \email: navid.nikaein@eurecom.fr, raymond.knopp@eurecom.fr, kroempa@gmail.com
*/

#ifndef __OPENAIR_RRC_DEFS_NR_H__
#define __OPENAIR_RRC_DEFS_NR_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collection/tree.h"
#include "collection/linear_alloc.h"
#include "nr_rrc_types.h"

#include "common/ngran_types.h"
#include "common/platform_constants.h"
#include "COMMON/platform_types.h"
#include "mac_rrc_dl.h"
#include "cucp_cuup_if.h"

#include "NR_SIB1.h"
#include "NR_RRCReconfigurationComplete.h"
#include "NR_RRCReconfiguration.h"
#include "NR_RRCReestablishmentRequest.h"
#include "NR_BCCH-BCH-Message.h"
#include "NR_BCCH-DL-SCH-Message.h"
#include "NR_BCCH-BCH-Message.h"
#include "NR_PLMN-IdentityInfo.h"
#include "NR_MCC-MNC-Digit.h"
#include "NR_NG-5G-S-TMSI.h"

#include "NR_UE-NR-Capability.h"
#include "NR_UE-MRDC-Capability.h"
#include "NR_MeasResults.h"
#include "NR_CellGroupConfig.h"
#include "NR_ServingCellConfigCommon.h"
#include "NR_EstablishmentCause.h"
#include "NR_SIB1.h"
//-------------------

#include "intertask_interface.h"

/* TODO: be sure this include is correct.
 * It solves a problem of compilation of the RRH GW,
 * issue #186.
 */
  #include "as_message.h"

  #include "commonDef.h"

#define PROTOCOL_NR_RRC_CTXT_UE_FMT                PROTOCOL_CTXT_FMT
#define PROTOCOL_NR_RRC_CTXT_UE_ARGS(CTXT_Pp)      PROTOCOL_NR_CTXT_ARGS(CTXT_Pp)

#define PROTOCOL_NR_RRC_CTXT_FMT                   PROTOCOL_CTXT_FMT
#define PROTOCOL_NR_RRC_CTXT_ARGS(CTXT_Pp)         PROTOCOL_NR_CTXT_ARGS(CTXT_Pp)

// 3GPP TS 38.331 Section 12 Table 12.1-1: UE performance requirements for RRC procedures for UEs
#define NR_RRC_SETUP_DELAY_MS           10
#define NR_RRC_RECONFIGURATION_DELAY_MS 10
#define NR_RRC_BWP_SWITCHING_DELAY_MS   6

// 3GPP TS 38.133 - Section 8 - Table 8.2.1.2.7-2: Parameters which cause interruption other than SCS
// This table was recently added to 3GPP. It shows that changing the parameters locationAndBandwidth, nrofSRS-Ports or
// maxMIMO-Layers-r16 causes an interruption. This parameter is not yet being used in code, but has been placed here
// for future reference.
#define NR_OF_SRS_PORTS_SWITCHING_DELAY_MS 30

#define NR_UE_MODULE_INVALID ((module_id_t) ~0) // FIXME attention! depends on type uint8_t!!!
#define NR_UE_INDEX_INVALID  ((module_id_t) ~0) // FIXME attention! depends on type uint8_t!!! used to be -1

typedef enum {
  NR_RRC_OK=0,
  NR_RRC_ConnSetup_failed,
  NR_RRC_PHY_RESYNCH,
  NR_RRC_Handover_failed,
  NR_RRC_HO_STARTED
} NR_RRC_status_t;

typedef enum UE_STATE_NR_e {
  NR_RRC_INACTIVE=0,
  NR_RRC_IDLE,
  NR_RRC_SI_RECEIVED,
  NR_RRC_CONNECTED,
  NR_RRC_RECONFIGURED,
  NR_RRC_HO_EXECUTION
} NR_UE_STATE_t;


#define RRM_FREE(p)       if ( (p) != NULL) { free(p) ; p=NULL ; }
#define RRM_MALLOC(t,n)   (t *) malloc16( sizeof(t) * n )
#define RRM_CALLOC(t,n)   (t *) malloc16( sizeof(t) * n)
#define RRM_CALLOC2(t,s)  (t *) malloc16( s )

#define MAX_MEAS_OBJ                                  7
#define MAX_MEAS_CONFIG                               7
#define MAX_MEAS_ID                                   7

#define PAYLOAD_SIZE_MAX                              1024
#define RRC_BUF_SIZE                                  1024
#define UNDEF_SECURITY_MODE                           0xff
#define NO_SECURITY_MODE                              0x20

/* TS 36.331: RRC-TransactionIdentifier ::= INTEGER (0..3) */
#define NR_RRC_TRANSACTION_IDENTIFIER_NUMBER             3

typedef struct {
  unsigned short                                      transport_block_size;      /*!< \brief Minimum PDU size in bytes provided by RLC to MAC layer interface */
  unsigned short                                      max_transport_blocks;      /*!< \brief Maximum PDU size in bytes provided by RLC to MAC layer interface */
  unsigned long                                       Guaranteed_bit_rate;       /*!< \brief Guaranteed Bit Rate (average) to be offered by MAC layer scheduling*/
  unsigned long                                       Max_bit_rate;              /*!< \brief Maximum Bit Rate that can be offered by MAC layer scheduling*/
  uint8_t                                             Delay_class;               /*!< \brief Delay class offered by MAC layer scheduling*/
  uint8_t                                             Target_bler;               /*!< \brief Target Average Transport Block Error rate*/
  uint8_t                                             Lchan_t;                   /*!< \brief Logical Channel Type (BCCH,CCCH,DCCH,DTCH_B,DTCH,MRBCH)*/
} __attribute__ ((__packed__))  NR_LCHAN_DESC;

typedef struct UE_RRC_INFO_NR_s {
  NR_UE_STATE_t                                       State;
  uint8_t                                             SIB1systemInfoValueTag;
  uint32_t                                            SIStatus;
  uint32_t                                            SIcnt;
  uint8_t                                             MCCHStatus[8];             // MAX_MBSFN_AREA
  uint16_t                                            SIwindowsize;              //!< Corresponds to the SIB1 si-WindowLength parameter. The unit is ms. Possible values are (final): 1,2,5,10,15,20,40
  uint8_t                                             handoverTarget;
  //HO_STATE_t ho_state;
  uint16_t
  SIperiod;                  //!< Corresponds to the SIB1 si-Periodicity parameter (multiplied by 10). Possible values are (final): 80,160,320,640,1280,2560,5120
  unsigned short                                      UE_index;
  uint32_t                                            T300_active;
  uint32_t                                            T300_cnt;
  uint32_t                                            T304_active;
  uint32_t                                            T304_cnt;
  uint32_t                                            T310_active;
  uint32_t                                            T310_cnt;
  uint32_t                                            N310_cnt;
  uint32_t                                            N311_cnt;
  rnti_t                                              rnti;
} __attribute__ ((__packed__)) NR_UE_RRC_INFO;

typedef struct UE_S_TMSI_NR_s {
  bool                                                presence;
  uint16_t                                            amf_set_id;
  uint8_t                                             amf_pointer;
  uint32_t                                            fiveg_tmsi;
} __attribute__ ((__packed__)) NR_UE_S_TMSI;


typedef enum nr_e_rab_satus_e {
  NR_E_RAB_STATUS_NEW,
  NR_E_RAB_STATUS_DONE,           // from the gNB perspective
  NR_E_RAB_STATUS_ESTABLISHED,    // get the reconfigurationcomplete form UE
  NR_E_RAB_STATUS_FAILED,
} nr_e_rab_status_t;

typedef struct nr_e_rab_param_s {
  e_rab_t param;
  uint8_t status;
  uint8_t xid; // transaction_id
} __attribute__ ((__packed__)) nr_e_rab_param_t;


typedef struct HANDOVER_INFO_NR_s {
  uint8_t                                             ho_prepare;
  uint8_t                                             ho_complete;
  uint8_t                                             modid_s;            //module_idP of serving cell
  uint8_t                                             modid_t;            //module_idP of target cell
  uint8_t                                             ueid_s;             //UE index in serving cell
  uint8_t                                             ueid_t;             //UE index in target cell

  // NR not define at this moment
  //AS_Config_t                                       as_config;          /* these two parameters are taken from 36.331 section 10.2.2: HandoverPreparationInformation-r8-IEs */
  //AS_Context_t                                      as_context;         /* They are mandatory for HO */

  uint8_t                                             buf[RRC_BUF_SIZE];  /* ASN.1 encoded handoverCommandMessage */
  int                                                 size;               /* size of above message in bytes */
} NR_HANDOVER_INFO;


#define NR_RRC_HEADER_SIZE_MAX 64
#define NR_RRC_BUFFER_SIZE_MAX 1024

typedef struct {
  char                                                Payload[NR_RRC_BUFFER_SIZE_MAX];
  char                                                Header[NR_RRC_HEADER_SIZE_MAX];
  uint16_t                                            payload_size;
} NR_RRC_BUFFER;

#define NR_RRC_BUFFER_SIZE                            sizeof(RRC_BUFFER_NR)


typedef struct RB_INFO_NR_s {
  uint16_t                                            Rb_id;  //=Lchan_id
  NR_LCHAN_DESC Lchan_desc[2];
  //MAC_MEAS_REQ_ENTRY *Meas_entry; //may not needed for NB-IoT
} NR_RB_INFO;

typedef struct NR_SRB_INFO_s {
  uint16_t                                            Srb_id;         //=Lchan_id
  NR_RRC_BUFFER                                          Rx_buffer;
  NR_RRC_BUFFER                                          Tx_buffer;
  NR_LCHAN_DESC                                          Lchan_desc[2];
  unsigned int                                        Trans_id;
  uint8_t                                             Active;
} NR_SRB_INFO;


typedef struct RB_INFO_TABLE_ENTRY_NR_s {
  NR_RB_INFO                                          Rb_info;
  uint8_t                                             Active;
  uint32_t                                            Next_check_frame;
  uint8_t                                             status;
} NR_RB_INFO_TABLE_ENTRY;

typedef struct SRB_INFO_TABLE_ENTRY_NR_s {
  NR_SRB_INFO                                         Srb_info;
  uint8_t                                             Active;
  uint8_t                                             status;
  uint32_t                                            Next_check_frame;
} NR_SRB_INFO_TABLE_ENTRY;

typedef struct nr_rrc_guami_s {
  uint16_t mcc;
  uint16_t mnc;
  uint8_t  mnc_len;
  uint8_t  amf_region_id;
  uint16_t amf_set_id;
  uint8_t  amf_pointer;
} nr_rrc_guami_t;

typedef enum pdu_session_satus_e {
  PDU_SESSION_STATUS_NEW,
  PDU_SESSION_STATUS_DONE,
  PDU_SESSION_STATUS_ESTABLISHED,
  PDU_SESSION_STATUS_REESTABLISHED, // after HO
  PDU_SESSION_STATUS_TOMODIFY,      // ENDC NSA
  PDU_SESSION_STATUS_FAILED,
  PDU_SESSION_STATUS_TORELEASE  // to release DRB between eNB and UE
} pdu_session_status_t;

typedef struct pdu_session_param_s {
  pdusession_t param;
  uint8_t status;
  uint8_t xid; // transaction_id
  ngap_Cause_t cause;
  uint8_t cause_value;
} rrc_pdu_session_param_t;

typedef struct drb_s {
  int status;
  int defaultDRBid;
  int drb_id;
  int reestablishPDCP;
  int recoverPDCP;
  int daps_Config_r16;
  struct cnAssociation_s {
    int present;
    int eps_BearerIdentity;
    struct sdap_config_s {
      bool defaultDRB;
      int pdusession_id;
      int sdap_HeaderDL;
      int sdap_HeaderUL;
      int mappedQoS_FlowsToAdd[QOSFLOW_MAX_VALUE];
    } sdap_config;
  } cnAssociation;
  struct pdcp_config_s {
    int discardTimer;
    int pdcp_SN_SizeUL;
    int pdcp_SN_SizeDL;
    int t_Reordering;
    int integrityProtection;
    struct headerCompression_s {
      int NotUsed;
      int present;
    } headerCompression;
    struct ext1_s {
      int cipheringDisabled;
    } ext1;
  } pdcp_config;
} drb_t;

typedef struct nr_drb_nssai_map_s {
  int drb_id;
  ngap_allowed_NSSAI_t nssai;
} nr_drb_nssai_map_t;

typedef struct nr_drb_nssai_map_list_s {
  int num_drb2add;
  nr_drb_nssai_map_t drb_nssai_list[NGAP_MAX_DRBS_PER_UE];
} nr_drb_nssai_map_list_t;

typedef struct gNB_RRC_UE_s {
  uint8_t                            primaryCC_id;
  NR_SRB_ToAddModList_t             *SRB_configList;
  NR_SRB_ToAddModList_t             *SRB_configList2[NR_RRC_TRANSACTION_IDENTIFIER_NUMBER];
  NR_DRB_ToAddModList_t             *DRB_configList;
  NR_DRB_ToAddModList_t             *DRB_configList2[NR_RRC_TRANSACTION_IDENTIFIER_NUMBER];
  NR_DRB_ToReleaseList_t            *DRB_Release_configList2[NR_RRC_TRANSACTION_IDENTIFIER_NUMBER];
  drb_t                              established_drbs[NGAP_MAX_DRBS_PER_UE];
  uint8_t                            DRB_active[NGAP_MAX_DRBS_PER_UE];
  nr_drb_nssai_map_list_t           DRB_NSSAI_configList;
  nr_drb_nssai_map_list_t           DRB_NSSAI_configList2[NR_RRC_TRANSACTION_IDENTIFIER_NUMBER];

  NR_SRB_INFO_TABLE_ENTRY Srb[maxSRBs]; // 3gpp max is 3 SRBs, number 1..3, we waste the entry 0 for code simplicity
  NR_MeasConfig_t                   *measConfig;
  NR_HANDOVER_INFO                  *handover_info;
  NR_MeasResults_t                  *measResults;


  NR_UE_NR_Capability_t*             UE_Capability_nr;
  int                                UE_Capability_size;
  NR_UE_MRDC_Capability_t*           UE_Capability_MRDC;
  int                                UE_MRDC_Capability_size;

  NR_CellGroupConfig_t               *secondaryCellGroup;
  NR_CellGroupConfig_t               *masterCellGroup;
  NR_RRCReconfiguration_t            *reconfig;
  NR_RadioBearerConfig_t             *rb_config;

  /* Pointer to save spCellConfig during RRC Reestablishment procedures */
  NR_SpCellConfig_t                  *spCellConfigReestablishment;

  ImsiMobileIdentity_t               imsi;

  /* KgNB as derived from KASME received from EPC */
  uint8_t kgnb[32];
  int8_t  kgnb_ncc;
  uint8_t nh[32];
  int8_t  nh_ncc;

  /* Used integrity/ciphering algorithms */
  NR_CipheringAlgorithm_t            ciphering_algorithm;
  e_NR_IntegrityProtAlgorithm        integrity_algorithm;

  uint8_t                            StatusRrc;
  rnti_t                             rnti;
  uint64_t                           random_ue_identity;

  /* Information from UE RRC Setup Request */
  NR_UE_S_TMSI                       Initialue_identity_5g_s_TMSI;
  uint64_t                           ng_5G_S_TMSI_Part1;
  uint16_t                           ng_5G_S_TMSI_Part2;
  NR_EstablishmentCause_t            establishment_cause;

  /* Information from UE RRCReestablishmentRequest */
  NR_ReestablishmentCause_t          reestablishment_cause;

  /* UE id for initial connection to S1AP */
  uint16_t                           ue_initial_id;

  /* Information from S1AP initial_context_setup_req */
  uint32_t                           gNB_ue_s1ap_id :24;
  uint32_t                           gNB_ue_ngap_id;
  uint64_t amf_ue_ngap_id;
  nr_rrc_guami_t                     ue_guami;

  ngap_security_capabilities_t       security_capabilities;

  /* Total number of e_rab already setup in the list */
  uint8_t                           setup_e_rabs;
  /* Number of e_rab to be setup in the list */
  uint8_t                            nb_of_e_rabs;
  /* Number of e_rab to be modified in the list */
  uint8_t                            nb_of_modify_e_rabs;
  uint8_t                            nb_of_failed_e_rabs;
  nr_e_rab_param_t                   modify_e_rab[NB_RB_MAX];//[S1AP_MAX_E_RAB];
  /* Number of pdu session managed for the ue */
  uint8_t                            nb_of_pdusessions;
  /* Number of e_rab to be modified in the list */
  uint8_t                            nb_of_modify_pdusessions;
  uint8_t                            nb_of_failed_pdusessions;
  rrc_pdu_session_param_t modify_pdusession[NR_NB_RB_MAX];
  /* list of e_rab to be setup by RRC layers */
  /* list of pdu session to be setup by RRC layers */
  nr_e_rab_param_t                   e_rab[NB_RB_MAX];//[S1AP_MAX_E_RAB];
  rrc_pdu_session_param_t pduSession[NGAP_MAX_PDU_SESSION];
  //release e_rabs
  uint8_t                            nb_release_of_e_rabs;
  e_rab_failed_t                     e_rabs_release_failed[S1AP_MAX_E_RAB];
  uint8_t                            nb_release_of_pdusessions;
  pdusession_failed_t                pdusessions_release_failed[NGAP_MAX_PDUSESSION];
  // LG: For GTPV1 TUNNELS
  uint32_t                           gnb_gtp_teid[S1AP_MAX_E_RAB];
  transport_layer_addr_t             gnb_gtp_addrs[S1AP_MAX_E_RAB];
  rb_id_t                            gnb_gtp_ebi[S1AP_MAX_E_RAB];
  rb_id_t                            gnb_gtp_psi[S1AP_MAX_E_RAB];
  //GTPV1 F1-U TUNNELS
  uint32_t                           incoming_teid[S1AP_MAX_E_RAB];

  uint32_t                           ul_failure_timer;
  uint32_t                           ue_release_timer;
  uint32_t                           ue_release_timer_thres;
  uint32_t                           ue_release_timer_s1;
  uint32_t                           ue_release_timer_thres_s1;
  uint32_t                           ue_release_timer_ng;
  uint32_t                           ue_release_timer_thres_ng;
  uint32_t                           ue_release_timer_rrc;
  uint32_t                           ue_release_timer_thres_rrc;
  uint32_t                           ue_reestablishment_timer;
  uint32_t                           ue_reestablishment_timer_thres;
  uint8_t                            e_rab_release_command_flag;
  uint8_t                            pdu_session_release_command_flag;
  uint8_t                            established_pdu_sessions_flag;
  uint32_t                           ue_rrc_inactivity_timer;
  int8_t                             reestablishment_xid;
  uint32_t                           ue_reestablishment_counter;
  uint32_t                           ue_reconfiguration_after_reestablishment_counter;
  NR_CellGroupId_t                                      cellGroupId;
  struct NR_SpCellConfig                                *spCellConfig;
  struct NR_CellGroupConfig__sCellToAddModList          *sCellconfig;
  struct NR_CellGroupConfig__sCellToReleaseList         *sCellconfigRelease;
  struct NR_CellGroupConfig__rlc_BearerToAddModList     *rlc_BearerBonfig;
  struct NR_CellGroupConfig__rlc_BearerToReleaseList    *rlc_BearerRelease;
  struct NR_MAC_CellGroupConfig                         *mac_CellGroupConfig;
  struct NR_PhysicalCellGroupConfig                     *physicalCellGroupConfig;

  /* Nas Pdu */
  ngap_pdu_t nas_pdu;

} gNB_RRC_UE_t;

typedef struct rrc_gNB_ue_context_s {
  /* Tree related data */
  RB_ENTRY(rrc_gNB_ue_context_s) entries;
  /* UE id for initial connection to NGAP */
  struct gNB_RRC_UE_s   ue_context;
} rrc_gNB_ue_context_t;

typedef struct {

  uint8_t                                   *SIB1;
  uint16_t                                  sizeof_SIB1;

  uint8_t                                   *SIB23;
  uint8_t                                   sizeof_SIB23;

  int                                       physCellId;

  NR_BCCH_BCH_Message_t                    *mib;
  NR_SIB1_t                                *siblock1_DU;
  NR_SIB1_t                                *sib1;
  NR_SIB2_t                                *sib2;
  NR_SIB3_t                                *sib3;
  NR_BCCH_DL_SCH_Message_t                  systemInformation; // SIB23
  NR_BCCH_DL_SCH_Message_t                  *siblock1;
  NR_ServingCellConfigCommon_t              *servingcellconfigcommon;
  NR_CellGroupConfig_t                      *secondaryCellGroup[MAX_NR_RRC_UE_CONTEXTS];
  int                                       p_gNB;

} rrc_gNB_carrier_data_t;
//---------------------------------------------------


typedef struct {
  /* nea0 = 0, nea1 = 1, ... */
  int ciphering_algorithms[4];
  int ciphering_algorithms_count;

  /* nia0 = 0, nia1 = 1, ... */
  int integrity_algorithms[4];
  int integrity_algorithms_count;

  /* flags to enable/disable ciphering and integrity for DRBs */
  int do_drb_ciphering;
  int do_drb_integrity;
} nr_security_configuration_t;

typedef struct nr_mac_rrc_dl_if_s {
  /* TODO add other message types as necessary */
  dl_rrc_message_transfer_func_t dl_rrc_message_transfer;
} nr_mac_rrc_dl_if_t;

typedef struct cucp_cuup_if_s {
  cucp_cuup_bearer_context_setup_func_t bearer_context_setup;
  cucp_cuup_bearer_context_setup_func_t bearer_context_mod;
} cucp_cuup_if_t;

typedef struct nr_reestablish_rnti_map_s {
  ue_id_t ue_id;
  rnti_t c_rnti;
} nr_reestablish_rnti_map_t;

//---NR---(completely change)---------------------
typedef struct gNB_RRC_INST_s {

  ngran_node_t                                        node_type;
  uint32_t                                            node_id;
  char                                               *node_name;
  int                                                 module_id;
  eth_params_t                                        eth_params_s;
  rrc_gNB_carrier_data_t                              carrier;
  uid_allocator_t                                     uid_allocator;
  RB_HEAD(rrc_nr_ue_tree_s, rrc_gNB_ue_context_s) rrc_ue_head; // ue_context tree key search by rnti
  /// NR cell id
  uint64_t nr_cellid;

  // RRC configuration
  gNB_RrcConfigurationReq configuration;

  // gNB N3 GTPU instance
  instance_t e1_inst;

  // other PLMN parameters
  /// Mobile country code
  int mcc;
  /// Mobile network code
  int mnc;
  /// number of mnc digits
  int mnc_digit_length;

  // other RAN parameters
  int srb1_timer_poll_retransmit;
  int srb1_poll_pdu;
  int srb1_poll_byte;
  int srb1_max_retx_threshold;
  int srb1_timer_reordering;
  int srb1_timer_status_prohibit;
  int um_on_default_drb;
  int srs_enable[MAX_NUM_CCs];
  uint16_t sctp_in_streams;
  uint16_t sctp_out_streams;
  int cell_info_configured;
  pthread_mutex_t cell_info_mutex;

  char *uecap_file;

  // security configuration (preferred algorithms)
  nr_security_configuration_t security;

  nr_reestablish_rnti_map_t nr_reestablish_rnti_map[MAX_MOBILES_PER_GNB];

  nr_mac_rrc_dl_if_t mac_rrc;
  cucp_cuup_if_t cucp_cuup;

} gNB_RRC_INST;

#include "nr_rrc_proto.h" //should be put here otherwise compilation error

#endif
/** @} */
