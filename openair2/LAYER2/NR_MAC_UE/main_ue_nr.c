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
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/* \file main_ue_nr.c
 * \brief top init of Layer 2
 * \author R. Knopp, K.H. HSU
 * \date 2018
 * \version 0.1
 * \company Eurecom / NTUST
 * \email: knopp@eurecom.fr, kai-hsiang.hsu@eurecom.fr
 * \note
 * \warning
 */

//#include "defs.h"
#include "mac_proto.h"
#include "radio/COMMON/common_lib.h"
//#undef MALLOC
#include "assertions.h"
#include "executables/softmodem-common.h"
#include "openair2/LAYER2/nr_pdcp/nr_pdcp.h"
#include "openair2/LAYER2/nr_pdcp/nr_pdcp_oai_api.h"
#include "nr_rlc/nr_rlc_oai_api.h"
#include "RRC/NR/MESSAGES/asn1_msg.h"
#include <pthread.h>

static NR_UE_MAC_INST_t *nr_ue_mac_inst; 

NR_UE_MAC_INST_t * nr_l2_init_ue(NR_UE_RRC_INST_t* rrc_inst) {

    //LOG_I(MAC, "[MAIN] MAC_INIT_GLOBAL_PARAM IN...\n");

    //LOG_I(MAC, "[MAIN] init UE MAC functions \n");
    
    //init mac here
    nr_ue_mac_inst = (NR_UE_MAC_INST_t *)calloc(sizeof(NR_UE_MAC_INST_t), NB_NR_UE_MAC_INST);

    for (int j = 0; j < NB_NR_UE_MAC_INST; j++)
      nr_ue_init_mac(j);

    if (rrc_inst && rrc_inst->scell_group_config) {

      nr_rrc_mac_config_req_ue(0,0,0,NULL,NULL,NULL,rrc_inst->scell_group_config);
      AssertFatal(rlc_module_init(0) == 0, "%s: Could not initialize RLC layer\n", __FUNCTION__);
      if (IS_SOFTMODEM_NOS1){
        // get default noS1 configuration
        NR_RadioBearerConfig_t *rbconfig = NULL;
        NR_RLC_BearerConfig_t *rlc_rbconfig = NULL;
        fill_nr_noS1_bearer_config(&rbconfig, &rlc_rbconfig);
        struct NR_CellGroupConfig__rlc_BearerToAddModList rlc_bearer_list = {
          .list = { .array = &rlc_rbconfig, .count = 1, .size = 1, }
        };

        // set up PDCP, RLC, MAC
        nr_pdcp_layer_init();
        nr_pdcp_add_drbs(ENB_FLAG_NO, nr_ue_mac_inst->crnti, 0, rbconfig->drb_ToAddModList, 0, NULL, NULL, &rlc_bearer_list);
        nr_rlc_add_drb(nr_ue_mac_inst->crnti, rbconfig->drb_ToAddModList->list.array[0]->drb_Identity, rlc_rbconfig, NULL);
        nr_ue_mac_inst->logicalChannelBearer_exist[0] = true;

        // free memory
        free_nr_noS1_bearer_config(&rbconfig, &rlc_rbconfig);
      }
      // Allocate memory for ul_config_request in the mac instance. This is now a pointer and will
      // point to a list of structures (one for each UL slot) to store PUSCH scheduling parameters
      // received from UL DCI.
      if (nr_ue_mac_inst->scc) {
        NR_TDD_UL_DL_ConfigCommon_t *tdd = nr_ue_mac_inst->scc->tdd_UL_DL_ConfigurationCommon;
        int num_slots_ul = tdd ? tdd->pattern1.nrofUplinkSlots : nr_slots_per_frame[*nr_ue_mac_inst->scc->ssbSubcarrierSpacing];
        if (tdd && nr_ue_mac_inst->scc->tdd_UL_DL_ConfigurationCommon->pattern1.nrofUplinkSymbols > 0) {
          num_slots_ul++;
        }
        LOG_D(NR_MAC, "In %s: Initializing ul_config_request. num_slots_ul = %d\n", __FUNCTION__, num_slots_ul);
        nr_ue_mac_inst->ul_config_request = (fapi_nr_ul_config_request_t *)calloc(num_slots_ul, sizeof(fapi_nr_ul_config_request_t));
        for (int i=0; i<num_slots_ul; i++)
          pthread_mutex_init(&(nr_ue_mac_inst->ul_config_request[i].mutex_ul_config), NULL);
      }
    }
    else {
      LOG_I(MAC,"Running without CellGroupConfig\n");
      nr_rrc_mac_config_req_ue(0,0,0,NULL,NULL,NULL,NULL);
      if(get_softmodem_params()->sa == 1) {
        AssertFatal(rlc_module_init(0) == 0, "%s: Could not initialize RLC layer\n", __FUNCTION__);
      }
    }

    return (nr_ue_mac_inst);
}

NR_UE_MAC_INST_t *get_mac_inst(module_id_t module_id){
    return &nr_ue_mac_inst[(int)module_id];
}
