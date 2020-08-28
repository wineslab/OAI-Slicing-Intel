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

/*! \file PHY/NR_UE_TRANSPORT/nr_ulsch.c
* \brief Top-level routines for transmission of the PUSCH TS 38.211 v 15.4.0
* \author Khalid Ahmed
* \date 2019
* \version 0.1
* \company Fraunhofer IIS
* \email: khalid.ahmed@iis.fraunhofer.de
* \note
* \warning
*/
#include <stdint.h>
#include "PHY/NR_REFSIG/dmrs_nr.h"
#include "PHY/NR_REFSIG/ptrs_nr.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_ue.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_proto_ue.h"
#include "PHY/MODULATION/nr_modulation.h"
#include "PHY/MODULATION/modulation_common.h"
#include "common/utils/assertions.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "PHY/NR_TRANSPORT/nr_transport_common_proto.h"
#include "PHY/NR_TRANSPORT/nr_sch_dmrs.h"
#include "PHY/defs_nr_common.h"
#include "PHY/TOOLS/tools_defs.h"
#include "executables/nr-softmodem.h"
#include "executables/softmodem-common.h"
#include "LAYER2/NR_MAC_UE/mac_proto.h"

//#define DEBUG_SCFDMA
//#define DEBUG_PUSCH_MAPPING
//#define DEBUG_MAC_PDU

//extern int32_t uplink_counter;

void nr_pusch_codeword_scrambling(uint8_t *in,
                         uint32_t size,
                         uint32_t Nid,
                         uint32_t n_RNTI,
                         uint32_t* out) {

  uint8_t reset, b_idx;
  uint32_t x1, x2, s=0, temp_out;

  reset = 1;
  x2 = (n_RNTI<<15) + Nid;

  for (int i=0; i<size; i++) {
    b_idx = i&0x1f;
    if (b_idx==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      reset = 0;
      if (i)
        out++;
    }
    if (in[i]==NR_PUSCH_x)
      *out ^= 1<<b_idx;
    else if (in[i]==NR_PUSCH_y){
      if (b_idx!=0)
        *out ^= (*out & (1<<(b_idx-1)))<<1;
      else{

        temp_out = *(out-1);
        *out ^= temp_out>>31;

      }
    }
    else
      *out ^= (((in[i])&1) ^ ((s>>b_idx)&1))<<b_idx;
    //printf("i %d b_idx %d in %d s 0x%08x out 0x%08x\n", i, b_idx, in[i], s, *out);
  }

}

void nr_ue_ulsch_procedures(PHY_VARS_NR_UE *UE,
                               unsigned char harq_pid,
                               uint32_t frame,
                               uint8_t slot,
                               uint8_t thread_id,
                               int gNB_id) {

  LOG_D(PHY,"nr_ue_ulsch_procedures hard_id %d %d.%d\n",harq_pid,frame,slot);

  uint32_t available_bits;
  uint8_t mod_order, cwd_index, l;
  uint32_t scrambled_output[NR_MAX_NB_CODEWORDS][NR_MAX_PDSCH_ENCODED_LENGTH>>5];
  int16_t **tx_layers;
  int32_t **txdataF;
  int8_t Wf[2], Wt[2], l_prime[2], delta;
  uint16_t rnti, code_rate, nb_rb;
  uint8_t nb_dmrs_re_per_rb,mcs, Nl;
  int ap, i;
  int sample_offsetF, N_RE_prime;

  NR_DL_FRAME_PARMS *frame_parms = &UE->frame_parms;
  NR_UE_PUSCH *pusch_ue = UE->pusch_vars[thread_id][gNB_id];
  // ptrs_UplinkConfig_t *ptrs_Uplink_Config = &UE->pusch_config.dmrs_UplinkConfig.ptrs_UplinkConfig;

  uint8_t  num_of_codewords = 1; // tmp assumption
  int      Nid_cell = 0;
  int      N_PRB_oh = 0; // higher layer (RRC) parameter xOverhead in PUSCH-ServingCellConfig
  uint16_t number_dmrs_symbols = 0;

  for (cwd_index = 0;cwd_index < num_of_codewords; cwd_index++) {

    NR_UE_ULSCH_t *ulsch_ue = UE->ulsch[thread_id][gNB_id][cwd_index];
    NR_UL_UE_HARQ_t *harq_process_ul_ue = ulsch_ue->harq_processes[harq_pid];

    int      start_symbol      = harq_process_ul_ue->pusch_pdu.start_symbol_index;
    uint16_t ul_dmrs_symb_pos  = harq_process_ul_ue->pusch_pdu.ul_dmrs_symb_pos;
    uint8_t  number_of_symbols = harq_process_ul_ue->pusch_pdu.nr_of_symbols;
    uint8_t  dmrs_type         = harq_process_ul_ue->pusch_pdu.dmrs_config_type;
    uint16_t start_rb          = harq_process_ul_ue->pusch_pdu.rb_start;
    uint16_t start_sc          = frame_parms->first_carrier_offset + start_rb*NR_NB_SC_PER_RB;

    if (start_sc >= frame_parms->ofdm_symbol_size)
      start_sc -= frame_parms->ofdm_symbol_size;

    for (i = start_symbol; i < start_symbol + number_of_symbols; i++) {

      if((ul_dmrs_symb_pos >> i) & 0x01)
        number_dmrs_symbols += 1;

    }

    rnti                  = harq_process_ul_ue->pusch_pdu.rnti;
    ulsch_ue->Nid_cell    = Nid_cell;

    nb_dmrs_re_per_rb = ((dmrs_type == pusch_dmrs_type1) ? 6:4)*harq_process_ul_ue->pusch_pdu.num_dmrs_cdm_grps_no_data;

    N_RE_prime = NR_NB_SC_PER_RB*number_of_symbols - nb_dmrs_re_per_rb*number_dmrs_symbols - N_PRB_oh;

    nb_rb = harq_process_ul_ue->pusch_pdu.rb_size;

    harq_process_ul_ue->num_of_mod_symbols = N_RE_prime*nb_rb*num_of_codewords;

    mcs            = harq_process_ul_ue->pusch_pdu.mcs_index;
    Nl             = harq_process_ul_ue->pusch_pdu.nrOfLayers;
    mod_order      = nr_get_Qm_ul(mcs, 0);
    code_rate      = nr_get_code_rate_ul(mcs, 0);

    harq_process_ul_ue->pusch_pdu.pusch_data.tb_size = nr_compute_tbs(mod_order,
                                             code_rate,
                                             nb_rb,
                                             number_of_symbols,
                                             nb_dmrs_re_per_rb*number_dmrs_symbols,
                                             0,
                                             0,
                                             harq_process_ul_ue->pusch_pdu.nrOfLayers);

    /////////////////////////ULSCH coding/////////////////////////
    ///////////

    unsigned int G = nr_get_G(nb_rb, number_of_symbols,
                              nb_dmrs_re_per_rb, number_dmrs_symbols, mod_order, Nl);
    nr_ulsch_encoding(ulsch_ue, frame_parms, harq_pid, G);

    ///////////
    ////////////////////////////////////////////////////////////////////

    /////////////////////////ULSCH scrambling/////////////////////////
    ///////////

    available_bits = G;

    memset(scrambled_output[cwd_index], 0, ((available_bits>>5)+1)*sizeof(uint32_t));

    nr_pusch_codeword_scrambling(ulsch_ue->g,
                                 available_bits,
                                 ulsch_ue->Nid_cell,
                                 rnti,
                                 scrambled_output[cwd_index]); // assume one codeword for the moment


    /////////////
    //////////////////////////////////////////////////////////////////////////

    /////////////////////////ULSCH modulation/////////////////////////
    ///////////

    nr_modulation(scrambled_output[cwd_index], // assume one codeword for the moment
                  available_bits,
                  mod_order,
                  (int16_t *)ulsch_ue->d_mod);


    // pusch_transform_precoding(ulsch_ue, frame_parms, harq_pid);

    ///////////
    ////////////////////////////////////////////////////////////////////////

  /////////////////////////DMRS Modulation/////////////////////////
  ///////////
  uint32_t ***pusch_dmrs = UE->nr_gold_pusch_dmrs[slot];
  uint16_t n_dmrs = (start_rb+nb_rb)*((dmrs_type == pusch_dmrs_type1) ? 6:4);
  int16_t mod_dmrs[n_dmrs<<1];
  ///////////
  ////////////////////////////////////////////////////////////////////////


  /////////////////////////PTRS parameters' initialization/////////////////////////
  ///////////

  int16_t mod_ptrs[nb_rb]; // assume maximum number of PTRS per pusch allocation
  uint8_t L_ptrs, K_ptrs = 0;
  uint16_t beta_ptrs = 1; // temp value until power control is implemented

  if (harq_process_ul_ue->pusch_pdu.pdu_bit_map & PUSCH_PDU_BITMAP_PUSCH_PTRS) {

    K_ptrs = (harq_process_ul_ue->pusch_pdu.pusch_ptrs.ptrs_freq_density)?4:2;
    L_ptrs = 1<<harq_process_ul_ue->pusch_pdu.pusch_ptrs.ptrs_time_density;

    ulsch_ue->ptrs_symbols = 0;

    set_ptrs_symb_idx(&ulsch_ue->ptrs_symbols,
                      number_of_symbols,
                      start_symbol,
                      L_ptrs,
                      ul_dmrs_symb_pos);
  }

  ///////////
  ////////////////////////////////////////////////////////////////////////////////

  /////////////////////////ULSCH layer mapping/////////////////////////
  ///////////

  tx_layers = (int16_t **)pusch_ue->txdataF_layers;

  nr_ue_layer_mapping(UE->ulsch[thread_id][gNB_id],
                      Nl,
                      available_bits/mod_order,
                      tx_layers);

  ///////////
  ////////////////////////////////////////////////////////////////////////


  //////////////////////// ULSCH transform precoding ////////////////////////
  ///////////

  l_prime[0] = 0; // single symbol ap 0

#ifdef NR_SC_FDMA
  uint32_t nb_re_pusch, nb_re_dmrs_per_rb;
  uint32_t y_offset = 0;

  for (l = start_symbol; l < start_symbol + number_of_symbols; l++) {

    if((ul_dmrs_symb_pos >> l) & 0x01)
      nb_re_dmrs_per_rb = nb_dmrs_re_per_rb;
    else
      nb_re_dmrs_per_rb = 0;
    
    nb_re_pusch = nb_rb * (NR_NB_SC_PER_RB - nb_re_dmrs_per_rb);

    nr_dft(&ulsch_ue->y[y_offset], &((int32_t*)tx_layers[0])[y_offset], nb_re_pusch);

    y_offset = y_offset + nb_re_pusch;
  }
#else
  memcpy(ulsch_ue->y, tx_layers[0], (available_bits/mod_order)*sizeof(int32_t));
#endif

  ///////////
  ////////////////////////////////////////////////////////////////////////



  /////////////////////////ULSCH RE mapping/////////////////////////
  ///////////

  txdataF = UE->common_vars.txdataF;

  for (ap=0; ap< Nl; ap++) {

    uint8_t k_prime = 0;
    uint16_t m = 0;

    // DMRS params for this ap
    get_Wt(Wt, ap, dmrs_type);
    get_Wf(Wf, ap, dmrs_type);
    delta = get_delta(ap, dmrs_type);

    for (l=start_symbol; l<start_symbol+number_of_symbols; l++) {

      uint16_t k = start_sc;
      uint16_t n = 0;
      uint8_t is_dmrs_sym = 0;
      uint8_t is_ptrs_sym = 0;
      uint16_t dmrs_idx = 0, ptrs_idx = 0;

      if ((ul_dmrs_symb_pos >> l) & 0x01) {
        is_dmrs_sym = 1;

        if (dmrs_type == pusch_dmrs_type1)
          dmrs_idx = start_rb*6;
        else
          dmrs_idx = start_rb*4;

        nr_modulation(pusch_dmrs[l][0], n_dmrs*2, DMRS_MOD_ORDER, mod_dmrs); // currently only codeword 0 is modulated. Qm = 2 as DMRS is QPSK modulated

      } else if (harq_process_ul_ue->pusch_pdu.pdu_bit_map & PUSCH_PDU_BITMAP_PUSCH_PTRS) {

        if(is_ptrs_symbol(l, ulsch_ue->ptrs_symbols)) {
          is_ptrs_sym = 1;
          nr_modulation(pusch_dmrs[l][0], nb_rb, DMRS_MOD_ORDER, mod_ptrs);
        }
      }

      for (i=0; i< nb_rb*NR_NB_SC_PER_RB; i++) {

        uint8_t is_dmrs = 0;
        uint8_t is_ptrs = 0;

        sample_offsetF = l*frame_parms->ofdm_symbol_size + k;

        if (is_dmrs_sym) {
          if (k == ((start_sc+get_dmrs_freq_idx_ul(n, k_prime, delta, dmrs_type))%frame_parms->ofdm_symbol_size))
            is_dmrs = 1;
        } else if (is_ptrs_sym) {
            is_ptrs = is_ptrs_subcarrier(k,
                                         rnti,
                                         ap,
                                         dmrs_type,
                                         K_ptrs,
                                         nb_rb,
                                         harq_process_ul_ue->pusch_pdu.pusch_ptrs.ptrs_ports_list[0].ptrs_re_offset,
                                         start_sc,
                                         frame_parms->ofdm_symbol_size);
        }

        if (is_dmrs == 1) {

          ((int16_t*)txdataF[ap])[(sample_offsetF)<<1] = (Wt[l_prime[0]]*Wf[k_prime]*AMP*mod_dmrs[dmrs_idx<<1]) >> 15;
          ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1] = (Wt[l_prime[0]]*Wf[k_prime]*AMP*mod_dmrs[(dmrs_idx<<1) + 1]) >> 15;

          #ifdef DEBUG_PUSCH_MAPPING
            printf("dmrs_idx %d\t l %d \t k %d \t k_prime %d \t n %d \t dmrs: %d %d\n",
            dmrs_idx, l, k, k_prime, n, ((int16_t*)txdataF[ap])[(sample_offsetF)<<1],
            ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1]);
          #endif


          dmrs_idx++;
          k_prime++;
          k_prime&=1;
          n+=(k_prime)?0:1;

        }  else if (is_ptrs == 1) {

          ((int16_t*)txdataF[ap])[(sample_offsetF)<<1] = (beta_ptrs*AMP*mod_ptrs[ptrs_idx<<1]) >> 15;
          ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1] = (beta_ptrs*AMP*mod_ptrs[(ptrs_idx<<1) + 1]) >> 15;

          ptrs_idx++;

        } else if (!is_dmrs_sym || allowed_xlsch_re_in_dmrs_symbol(k,start_sc,harq_process_ul_ue->pusch_pdu.num_dmrs_cdm_grps_no_data,dmrs_type)) {

          ((int16_t*)txdataF[ap])[(sample_offsetF)<<1]       = ((int16_t *) ulsch_ue->y)[m<<1];
          ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1] = ((int16_t *) ulsch_ue->y)[(m<<1) + 1];

        #ifdef DEBUG_PUSCH_MAPPING
          printf("m %d\t l %d \t k %d \t txdataF: %d %d\n",
          m, l, k, ((int16_t*)txdataF[ap])[(sample_offsetF)<<1],
          ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1]);
        #endif

          m++;

        } else {

          ((int16_t*)txdataF[ap])[(sample_offsetF)<<1]       = 0;
          ((int16_t*)txdataF[ap])[((sample_offsetF)<<1) + 1] = 0;

        }

        if (++k >= frame_parms->ofdm_symbol_size)
          k -= frame_parms->ofdm_symbol_size;
      }
    }
  }
  }

  NR_UL_UE_HARQ_t *harq_process_ulsch=NULL;
  harq_process_ulsch = UE->ulsch[thread_id][gNB_id][0]->harq_processes[harq_pid];
  harq_process_ulsch->status = SCH_IDLE;

  ///////////
  ////////////////////////////////////////////////////////////////////////

}


uint8_t nr_ue_pusch_common_procedures(PHY_VARS_NR_UE *UE,
                                      uint8_t harq_pid,
                                      uint8_t slot,
                                      uint8_t thread_id,
                                      uint8_t gNB_id,
                                      NR_DL_FRAME_PARMS *frame_parms) {

  int tx_offset, ap;
  int32_t **txdata;
  int32_t **txdataF;
  uint8_t Nl = UE->ulsch[thread_id][gNB_id][0]->harq_processes[harq_pid]->pusch_pdu.nrOfLayers; // cw 0

  /////////////////////////IFFT///////////////////////
  ///////////

  tx_offset = frame_parms->get_samples_slot_timestamp(slot, frame_parms, 0);

  // clear the transmit data array for the current subframe
  /*for (int aa=0; aa<UE->frame_parms.nb_antennas_tx; aa++) {
	  memset(&UE->common_vars.txdata[aa][tx_offset],0,UE->frame_parms.samples_per_slot*sizeof(int32_t));
	  //memset(&UE->common_vars.txdataF[aa][tx_offset],0,UE->frame_parms.samples_per_slot*sizeof(int32_t));
  }*/


  txdata = UE->common_vars.txdata;
  txdataF = UE->common_vars.txdataF;

  for (ap = 0; ap < Nl; ap++) {
    if (frame_parms->Ncp == 1) { // extended cyclic prefix
      PHY_ofdm_mod(txdataF[ap],
                   &txdata[ap][tx_offset],
                   frame_parms->ofdm_symbol_size,
                   12,
                   frame_parms->nb_prefix_samples,
                   CYCLIC_PREFIX);
    } else { // normal cyclic prefix
      nr_normal_prefix_mod(txdataF[ap],
                           &txdata[ap][tx_offset],
                           14,
                           frame_parms);
    }
  }

  ///////////
  ////////////////////////////////////////////////////
  return 0;
}
