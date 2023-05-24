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

/*! \file       nr_rlc_oai_api.h
 * \brief       Header file for nr_rlc_oai_api
 * \author      Guido Casati
 * \date        2020
 * \email:      guido.casati@iis.fraunhofe.de
 * \version     1.0
 * @ingroup     _rlc

 */

#include "NR_RLC-BearerConfig.h"
#include "NR_RLC-Config.h"
#include "NR_LogicalChannelIdentity.h"
#include "NR_RadioBearerConfig.h"
#include "NR_CellGroupConfig.h"
#include "openair2/RRC/NR/nr_rrc_proto.h"

struct NR_RLC_Config;
struct NR_LogicalChannelConfig;

void nr_rlc_add_srb(int rnti, int srb_id, const NR_RLC_BearerConfig_t *rlc_BearerConfig);
void nr_rlc_add_drb(int rnti, int drb_id, const NR_RLC_BearerConfig_t *rlc_BearerConfig, const nr_drb_nssai_map_list_t *drb_nssai_list);

void nr_rlc_remove_ue(int rnti);

int nr_rlc_get_available_tx_space(
  const rnti_t            rntiP,
  const logical_chan_id_t channel_idP);

void nr_rlc_activate_avg_time_to_tx(
  const rnti_t            rnti,
  const logical_chan_id_t channel_id,
  const bool              is_on);

void nr_rlc_srb_recv_sdu(const int rnti, const logical_chan_id_t channel_id, unsigned char *buf, int size);

struct gNB_MAC_INST_s;
void nr_rlc_activate_srb0(int rnti, struct gNB_MAC_INST_s *mac, void *rawUE,
                          void (*send_initial_ul_rrc_message)(
                                     struct gNB_MAC_INST_s *mac,
                                     int                    rnti,
                                     const uint8_t         *sdu,
                                     sdu_size_t             sdu_len,
                                     void                  *rawUE));

ngap_allowed_NSSAI_t mac_rlc_get_nssai(const rnti_t rntiP,
                                       const logical_chan_id_t channel_idP);
