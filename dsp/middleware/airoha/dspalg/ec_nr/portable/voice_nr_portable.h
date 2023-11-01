/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#ifndef  __VOICE_NR_PORTABLE_H__
#define  __VOICE_NR_PORTABLE_H__

#if defined(AIR_VOICE_NR_USE_PIC_ENABLE)
#define AIR_VOICE_NR_USE_HYBRID_PIC_ENABLE  /*EC IP with static lib for initial function*/
#endif


#if defined(AIR_BTA_IC_PREMIUM_G2)
#include "dsp_para_aec_nr_nvkey_struct.h"
void voice_nvkey_aec_nr_porting_layer(void *p_nvkey_aec, void *p_nvkey_nr, void *p_nvkey_inear, void *p_nvkey_aec_nr);
#endif

#include "types.h"
#include "dsp_para_cpd.h"
/* Rx NR */
#include "dsp_para_nb_rx_eq_nvkey_struct.h"
#include "dsp_para_wb_rx_eq_nvkey_struct.h"
#include "dsp_para_swb_rx_eq_nvkey_struct.h"

#include "dsp_para_aec_nvkey_struct.h"
#include "dsp_para_nr_nvkey_struct.h"
#include "dsp_para_inear_nvkey_struct.h"
#include "dsp_para_swb_tx_eq_nvkey_struct.h"
#include "dsp_para_wb_tx_eq_nvkey_struct.h"
#include "dsp_para_nb_tx_eq_nvkey_struct.h"
#include "dsp_para_aec_nr_swb_nvkey_struct.h"
#include "dsp_para_ast_eq_nvkey_struct.h"

#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
#include "dsp_para_inear_eq_nvkey_struct.h"
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE)
#include "dsp_para_reserved_nvkey_struct.h"
#include "dsp_para_wb_tx_eqv2_nvkey_struct.h"
#include "dsp_para_nb_tx_eqv2_nvkey_struct.h"
#endif

#if defined(AIR_VOICE_NR_USE_PIC_ENABLE)

extern void *g_ecnr_decoder_export_parameters[];
typedef void (*p_NB_RX_NR_init)(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);                           /*EC IP with static lib for initial function*/
typedef void (*p_WB_RX_NR_init)(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);                           /*EC IP with static lib for initial function*/
typedef void (*p_SWB_RX_NR_init)(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_ecnr_swb_NvKey, void *p_rxeq_NvKey);  /*EC IP with static lib for initial function*/

typedef void (*p_Voice_NB_RX_Prcs)(S16 *NR);
typedef void (*p_Voice_WB_RX_Prcs)(S16 *NR);
typedef void (*p_Voice_SWB_RX_Prcs)(S16 *NR);
typedef int (*p_dummy_EC_extNR_func)(void); /*Used for dbg*/

#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
/* Version */
typedef int (*p_get_ECNR_SVN)(void);
/* Buffer */
typedef int (*p_get_aec_nr_memsize)(void);
/* Reference gain */
typedef void (*p_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_IE_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_BAND5_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_EC_PreLim_Coef_READBACK)(S16 *coef);
typedef S16  (*p_DaulMIC_power_Coef_READBACK)(void);
typedef void (*p_BAND5_PreLim_Coef_READBACK)(S16 *coef);
typedef void (*p_AEC_NR_Write_FLASH)();
typedef int  (*p_TWO_MIC_WB_Write_FLASH)(S16 *bufou);
typedef void (*p_EQ_update)();
typedef int  (*p_get_txnr_pitch_last)();
/* EC-NR-POSTEC-EQ */
typedef void (*p_Voice_WB_TX_Init)(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey);
typedef void (*p_Voice_WB_TX_Prcs)(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *NR, ECNR_OUT *PAR);
typedef void (*p_Voice_WB_TX_Inear_Init_V2)(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
typedef void (*p_Voice_WB_TX_Inear_Prcs_V2)(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *NR, ECNR_OUT *PAR);
typedef void (*p_Voice_SWB_TX_Init)(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey);
typedef void (*p_Voice_SWB_TX_Prcs)(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *NR, ECNR_OUT *PAR);
typedef void (*p_Voice_SWB_TX_Inear_Init_V2)(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
typedef void (*p_Voice_SWB_TX_Inear_Prcs_V2)(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *NR, ECNR_OUT *PAR);
typedef void (*p_Assign_EC_RXNR_scratch_memory)(void *p_ecnr_mem_ext);

int get_ECNR_Init_SVN(void);
#if !defined(AIR_BTA_IC_PREMIUM_G3)
int get_aec_nr_memsize(void);
void Voice_WB_TX_Init(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey);
void NB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);
void WB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);
void Voice_WB_TX_Inear_Init_V2(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
void SWB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_ecnr_swb_NvKey, void *p_rxeq_NvKey);
void Voice_SWB_TX_Init(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey);
void Voice_SWB_TX_Inear_Init_V2(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
#endif

//#define get_aec_nr_memsize          ((p_get_aec_nr_memsize)g_ecnr_decoder_export_parameters[0])
//#define Voice_WB_TX_Init            ((p_Voice_WB_TX_Init)g_ecnr_decoder_export_parameters[1])
#define Voice_WB_TX_Prcs            ((p_Voice_WB_TX_Prcs)g_ecnr_decoder_export_parameters[2])
#define EC_REF_GAIN_READBACK        ((p_EC_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[3])
//#define NB_RX_NR_init               ((p_NB_RX_NR_init)g_ecnr_decoder_export_parameters[4])
#define Voice_NB_RX_Prcs            ((p_Voice_NB_RX_Prcs)g_ecnr_decoder_export_parameters[5])
//#define WB_RX_NR_init               ((p_WB_RX_NR_init)g_ecnr_decoder_export_parameters[6])
#define Voice_WB_RX_Prcs            ((p_Voice_WB_RX_Prcs)g_ecnr_decoder_export_parameters[7])
#define EC_PreLim_Coef_READBACK     ((p_EC_PreLim_Coef_READBACK)g_ecnr_decoder_export_parameters[8])
#define AEC_NR_Write_FLASH          ((p_AEC_NR_Write_FLASH)g_ecnr_decoder_export_parameters[9])
#define EQ_update                   ((p_EQ_update)g_ecnr_decoder_export_parameters[10])
#define get_txnr_pitch_last         ((p_get_txnr_pitch_last)g_ecnr_decoder_export_parameters[11])
#define DaulMIC_power_Coef_READBACK ((p_DaulMIC_power_Coef_READBACK)g_ecnr_decoder_export_parameters[12])
#define TWO_MIC_WB_Write_FLASH      ((p_TWO_MIC_WB_Write_FLASH)g_ecnr_decoder_export_parameters[13])
#define IE_EC_REF_GAIN_READBACK     ((p_IE_EC_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[14])
//#define Voice_WB_TX_Inear_Init_V2   ((p_Voice_WB_TX_Inear_Init_V2)g_ecnr_decoder_export_parameters[15])
#define Voice_WB_TX_Inear_Prcs_V2   ((p_Voice_WB_TX_Inear_Prcs_V2)g_ecnr_decoder_export_parameters[16])
#define get_ECNR_SVN                ((p_get_ECNR_SVN)g_ecnr_decoder_export_parameters[17])
//#define SWB_RX_NR_init              ((p_SWB_RX_NR_init)g_ecnr_decoder_export_parameters[18])
#define Voice_SWB_RX_Prcs           ((p_Voice_SWB_RX_Prcs)g_ecnr_decoder_export_parameters[19])
//#define Voice_SWB_TX_Init           ((p_Voice_SWB_TX_Init)g_ecnr_decoder_export_parameters[20])
#define Voice_SWB_TX_Prcs           ((p_Voice_SWB_TX_Prcs)g_ecnr_decoder_export_parameters[21])
//#define Voice_SWB_TX_Inear_Init_V2  ((p_Voice_SWB_TX_Inear_Init_V2)g_ecnr_decoder_export_parameters[22])
#define Voice_SWB_TX_Inear_Prcs_V2  ((p_Voice_SWB_TX_Inear_Prcs_V2)g_ecnr_decoder_export_parameters[23])
#define BAND5_EC_REF_GAIN_READBACK  ((p_BAND5_EC_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[24])
#define BAND5_PreLim_Coef_READBACK  ((p_BAND5_PreLim_Coef_READBACK)g_ecnr_decoder_export_parameters[25])
#define Assign_EC_RXNR_scratch_memory ((p_Assign_EC_RXNR_scratch_memory)g_ecnr_decoder_export_parameters[26])

#if defined(AIR_BTA_IC_PREMIUM_G3)
#define get_aec_nr_memsize          ((p_get_aec_nr_memsize)g_ecnr_decoder_export_parameters[0])
#define Voice_WB_TX_Init            ((p_Voice_WB_TX_Init)g_ecnr_decoder_export_parameters[1])
#define NB_RX_NR_init               ((p_NB_RX_NR_init)g_ecnr_decoder_export_parameters[4])
#define WB_RX_NR_init               ((p_WB_RX_NR_init)g_ecnr_decoder_export_parameters[6])
#define Voice_WB_TX_Inear_Init_V2   ((p_Voice_WB_TX_Inear_Init_V2)g_ecnr_decoder_export_parameters[15])
#define SWB_RX_NR_init              ((p_SWB_RX_NR_init)g_ecnr_decoder_export_parameters[18])
#define Voice_SWB_TX_Init           ((p_Voice_SWB_TX_Init)g_ecnr_decoder_export_parameters[20])
#define Voice_SWB_TX_Inear_Init_V2  ((p_Voice_SWB_TX_Inear_Init_V2)g_ecnr_decoder_export_parameters[22])
#endif

#endif
#if defined(AIR_3RD_PARTY_NR_ENABLE)
/* Version */
typedef int (*p_get_ECNR_SVN)(void);
#ifndef AIR_VOICE_NR_USE_HYBRID_PIC_ENABLE
/* Buffer */
typedef int (*p_get_aec_nr_memsize)(void);
typedef int (*p_get_aec_nr_inear_memsize)(void);
#endif
/* Reference gain */
typedef void (*p_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_EC_Inear_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_BAND5_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_EC_PreLim_Coef_READBACK)(S16 *coef);
typedef S16 (*p_DaulMIC_power_Coef_READBACK)(void);
typedef void (*p_BAND5_PreLim_Coef_READBACK)(S16 *coef);
typedef S16 (*p_PF_GAIN_FB_READBACK)(void);
typedef S16 (*p_WIND_INFO_READBACK)(void);
typedef void (*p_Assign_EC_RXNR_scratch_memory)(void *p_ecnr_mem_ext);
typedef void (*p_RX_EQ_update)(void *p_rxeq_NvKey, int mode);

/* EC */
#ifndef AIR_VOICE_NR_USE_HYBRID_PIC_ENABLE
typedef void (*p_Voice_EC_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain);       /*EC IP with static lib for initial function*/
typedef void (*p_Voice_EC_Inear_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain); /*EC IP with static lib for initial function*/
typedef void (*p_Voice_SWB_EC_Init)(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);      /*EC IP with static lib for initial function*/
typedef void (*p_Voice_SWB_EC_Inear_Init)(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);/*EC IP with static lib for initial function*/
#endif
typedef void (*p_Voice_EC_Prcs)(S16 *MIC1, S16 *MIC2, S16 *Ref, S16 *EC_out1, S16 *EC_out2);
typedef void (*p_Voice_EC_Inear_Prcs)(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *EC_out1, S16 *EC_out2, S16 *EC_outB);
typedef void (*p_Voice_SWB_EC_Prcs)(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *EC_out1, S16 *EC_out2);
typedef void (*p_Voice_SWB_EC_Inear_Prcs)(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *EC_out1, S16 *EC_out2, S16 *EC_outB);
/* NR */
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
#include "igo_nr.h"
#endif
/* POSTEC */
typedef void (*p_Voice_PostEC_Prcs)(S16 *NR_out, S16 *out, ECNR_OUT *PAR);
typedef void (*p_Voice_SWB_PostEC_Prcs)(S16 *NR_out, S16 *out, ECNR_OUT *PAR);
/* Tx EQ */
typedef int (*p_p_get_fir_wbeq_memsize)(void);
typedef void (*p_p_Voice_TxEQ_init)(void *handle, void *eq_nvkey);
typedef void (*p_p_Voice_TxEQ_Prcs)(S16 *buf, void *handle);
typedef void (*p_p_Voice_TxEQ_Prcs_Length)(S16 *buf, void *handle, int len);
/* Rx EQ */
typedef void (*p_RX_EQ_update)(void *p_rxeq_NvKey, int mode);

int get_ECNR_Init_SVN(void);
int get_aec_nr_memsize(void);                                                                                                           /*EC IP with static lib for initial function*/
void Voice_EC_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain);          /*EC IP with static lib for initial function*/
void NB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);                                                         /*EC IP with static lib for initial function*/
void WB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);                                                         /*EC IP with static lib for initial function*/
int get_aec_nr_inear_memsize(void);                                                                                                     /*EC IP with static lib for initial function*/
void Voice_EC_Inear_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain);    /*EC IP with static lib for initial function*/
void Voice_SWB_EC_Init(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);         /*EC IP with static lib for initial function*/
void Voice_SWB_EC_Inear_Init(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);   /*EC IP with static lib for initial function*/
void SWB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_ecnr_swb_NvKey, void *p_rxeq_NvKey);                                /*EC IP with static lib for initial function*/

//#define get_aec_nr_memsize                ((p_get_aec_nr_memsize)g_ecnr_decoder_export_parameters[0])
//#define Voice_EC_Init                     ((p_Voice_EC_Init)g_ecnr_decoder_export_parameters[1])
#define Voice_EC_Prcs                     ((p_Voice_EC_Prcs)g_ecnr_decoder_export_parameters[2])
#define Voice_PostEC_Prcs                 ((p_Voice_PostEC_Prcs)g_ecnr_decoder_export_parameters[3])
#define EC_REF_GAIN_READBACK              ((p_EC_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[4])
//#define NB_RX_NR_init                     ((p_NB_RX_NR_init)g_ecnr_decoder_export_parameters[5])
#define Voice_NB_RX_Prcs                  ((p_Voice_NB_RX_Prcs)g_ecnr_decoder_export_parameters[6])
//#define WB_RX_NR_init                     ((p_WB_RX_NR_init)g_ecnr_decoder_export_parameters[7])
#define Voice_WB_RX_Prcs                  ((p_Voice_WB_RX_Prcs)g_ecnr_decoder_export_parameters[8])
//#define get_aec_nr_inear_memsize          ((p_get_aec_nr_inear_memsize)g_ecnr_decoder_export_parameters[9])
//#define Voice_EC_Inear_Init               ((p_Voice_EC_Inear_Init)g_ecnr_decoder_export_parameters[10])
#define Voice_EC_Inear_Prcs               ((p_Voice_EC_Inear_Prcs)g_ecnr_decoder_export_parameters[11])
#define EC_Inear_REF_GAIN_READBACK        ((p_EC_Inear_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[12])
#define get_ECNR_SVN                      ((p_get_ECNR_SVN)g_ecnr_decoder_export_parameters[13])
#define EC_PreLim_Coef_READBACK           ((p_EC_PreLim_Coef_READBACK)g_ecnr_decoder_export_parameters[14])
#define DaulMIC_power_Coef_READBACK       ((p_DaulMIC_power_Coef_READBACK)g_ecnr_decoder_export_parameters[15])
//#define Voice_SWB_EC_Init                 ((p_Voice_SWB_EC_Init)g_ecnr_decoder_export_parameters[16])
#define Voice_SWB_EC_Prcs                 ((p_Voice_SWB_EC_Prcs)g_ecnr_decoder_export_parameters[17])
//#define Voice_SWB_EC_Inear_Init           ((p_Voice_SWB_EC_Inear_Init)g_ecnr_decoder_export_parameters[18])
#define Voice_SWB_EC_Inear_Prcs           ((p_Voice_SWB_EC_Inear_Prcs)g_ecnr_decoder_export_parameters[19])
#define Voice_SWB_PostEC_Prcs             ((p_Voice_SWB_PostEC_Prcs)g_ecnr_decoder_export_parameters[20])
//#define SWB_RX_NR_init                    ((p_SWB_RX_NR_init)g_ecnr_decoder_export_parameters[21])
#define Voice_SWB_RX_Prcs                 ((p_Voice_SWB_RX_Prcs)g_ecnr_decoder_export_parameters[22])
#define BAND5_EC_REF_GAIN_READBACK        ((p_BAND5_EC_REF_GAIN_READBACK)g_ecnr_decoder_export_parameters[23])
#define BAND5_PreLim_Coef_READBACK        ((p_BAND5_PreLim_Coef_READBACK)g_ecnr_decoder_export_parameters[24])
#define PF_GAIN_FB_READBACK               ((p_PF_GAIN_FB_READBACK)g_ecnr_decoder_export_parameters[25])
#define WIND_INFO_READBACK               ((p_WIND_INFO_READBACK)g_ecnr_decoder_export_parameters[26])
#define Assign_EC_RXNR_scratch_memory     ((p_Assign_EC_RXNR_scratch_memory)g_ecnr_decoder_export_parameters[27])
#define RX_EQ_update                      ((p_RX_EQ_update)g_ecnr_decoder_export_parameters[28])

#ifndef AIR_BTA_IC_STEREO_HIGH_G3
extern void *g_tx_eq_export_parameters[];
#endif

typedef int (*p_get_fir_wbeq_memsize)(void);
typedef void (*p_Voice_TxEQ_init)(void *handle, void *eq_nvkey);
typedef void (*p_Voice_TxEQ_Prcs)(S16 *buf, void *handle);
typedef void (*p_Voice_TxEQ_Prcs_Length)(S16 *buf, void *handle, int len);

#define get_fir_wbeq_memsize    ((p_get_fir_wbeq_memsize)g_tx_eq_export_parameters[0])
#define Voice_TxEQ_init         ((p_Voice_TxEQ_init)g_tx_eq_export_parameters[1])
#define Voice_TxEQ_Prcs         ((p_Voice_TxEQ_Prcs)g_tx_eq_export_parameters[2])
#define Voice_TxEQ_Prcs_Length  ((p_Voice_TxEQ_Prcs_Length)g_tx_eq_export_parameters[3])

#endif

#if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
/* FallBand */
extern void *g_ecfb_decoder_export_parameters[];
typedef int (*p_get_aec_nr_memsize_fb)(void);
typedef void (*p_Voice_FB_EC_Init)(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);
typedef void (*p_Voice_FB_EC_Prcs)(S32 *MIC1, S32 *REF, S32 *EC_out1);
typedef void (*p_Voice_FB_PostEC_Prcs)(S32 *NR_out, S32 *out, ECNR_OUT *PAR);
typedef void (*p_EC_REF_GAIN_READBACK_FB)(S16 *gain);
typedef int (*p_get_ECNR_SVN_FB)(void);
typedef void (*p_BAND5_EC_REF_GAIN_READBACK_FB)(S16 *gain);

#define get_aec_nr_memsize_fb            ((p_get_aec_nr_memsize_fb)g_ecfb_decoder_export_parameters[0])
#define Voice_FB_EC_Init                 ((p_Voice_FB_EC_Init)g_ecfb_decoder_export_parameters[1])
#define Voice_FB_EC_1M_Prcs              ((p_Voice_FB_EC_Prcs)g_ecfb_decoder_export_parameters[2])
#define Voice_FB_PostEC_Prcs             ((p_Voice_FB_PostEC_Prcs)g_ecfb_decoder_export_parameters[3])
#define EC_REF_GAIN_READBACK_FB          ((p_EC_REF_GAIN_READBACK_FB)g_ecfb_decoder_export_parameters[4])
#define get_ECNR_SVN_FB                  ((p_get_ECNR_SVN_FB)g_ecfb_decoder_export_parameters[5])
#define BAND5_EC_REF_GAIN_READBACK_FB    ((p_BAND5_EC_REF_GAIN_READBACK_FB)g_ecfb_decoder_export_parameters[6])
#endif


#else  /*AIR_VOICE_NR_USE_PIC_ENABLE = n */
void NB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);
void WB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_rxeq_NvKey);
void SWB_RX_NR_init(void *p_ecnr_mem_ext, void *p_nr_NvKey, void *p_ecnr_swb_NvKey, void *p_rxeq_NvKey);
void Voice_NB_RX_Prcs(S16 *NR);
void Voice_WB_RX_Prcs(S16 *NR);
void Voice_SWB_RX_Prcs(S16 *NR);
#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
/* Version */
int get_ECNR_SVN(void);

/* Buffer */
int get_aec_nr_memsize(void);
/* Reference gain */
void EC_REF_GAIN_READBACK(S16 *gain);
void IE_EC_REF_GAIN_READBACK(S16 *gain);
void BAND5_EC_REF_GAIN_READBACK(S16 *gain);
void EC_PreLim_Coef_READBACK(S16 *coef);
S16 DaulMIC_power_Coef_READBACK(void);
void BAND5_PreLim_Coef_READBACK(S16 *coef);
void AEC_NR_Write_FLASH();
int TWO_MIC_WB_Write_FLASH(S16 *bufou);
void EQ_update();
int get_txnr_pitch_last();
/* EC-NR-POSTEC-EQ */
void Voice_WB_TX_Init(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey);
void Voice_WB_TX_Prcs(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *NR, ECNR_OUT *PAR);
void Voice_WB_TX_Inear_Init_V2(int NB_mode, S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
void Voice_WB_TX_Inear_Prcs_V2(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *NR, ECNR_OUT *PAR);
void Voice_SWB_TX_Init(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey);
void Voice_SWB_TX_Prcs(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *NR, ECNR_OUT *PAR);
void Voice_SWB_TX_Inear_Init_V2(S16 *bufin, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey, void *p_txeq_NvKey, void *p_InEarEQ_NvKey, void *p_ast_eq_nvkey);
void Voice_SWB_TX_Inear_Prcs_V2(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *NR, ECNR_OUT *PAR);
#endif
#if defined(AIR_3RD_PARTY_NR_ENABLE)
/* Version */
int get_ECNR_SVN(void);
/* Buffer */
int get_aec_nr_memsize(void);
int get_aec_nr_inear_memsize(void);
/* Reference gain */
void EC_REF_GAIN_READBACK(S16 *gain);
void EC_Inear_REF_GAIN_READBACK(S16 *gain);
void BAND5_EC_REF_GAIN_READBACK(S16 *gain);
void EC_PreLim_Coef_READBACK(S16 *coef);
S16 DaulMIC_power_Coef_READBACK(void);
void BAND5_PreLim_Coef_READBACK(S16 *coef);
S16 PF_GAIN_FB_READBACK(void);
/* EC */
void Voice_EC_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain);
void Voice_EC_Prcs(S16 *MIC1, S16 *MIC2, S16 *Ref, S16 *EC_out1, S16 *EC_out2);
void Voice_EC_Inear_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, S16 *NR_gain);
void Voice_EC_Inear_Prcs(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *EC_out1, S16 *EC_out2, S16 *EC_outB);
void Voice_SWB_EC_Init(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);
void Voice_SWB_EC_Prcs(S16 *MIC1, S16 *MIC2, S16 *REF, S16 *EC_out1, S16 *EC_out2);
void Voice_SWB_EC_Inear_Init(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);
void Voice_SWB_EC_Inear_Prcs(S16 *MIC1, S16 *MIC2, S16 *MICB, S16 *REF, S16 *EC_out1, S16 *EC_out2, S16 *EC_outB);
/* NR */
/* 3rd party NR don't offer static library, so the API declare is NOT needed here */
/* POSTEC */
void Voice_PostEC_Prcs(S16 *NR_out, S16 *out, ECNR_OUT *PAR);
void Voice_SWB_PostEC_Prcs(S16 *NR_out, S16 *out, ECNR_OUT *PAR);
/* Tx EQ */
int get_fir_wbeq_memsize(void);
void Voice_TxEQ_init(void *handle, void *eq_nvkey);
void Voice_TxEQ_Prcs(S16 *buf, void *handle);
void Voice_TxEQ_Prcs_Length(S16 *buf, void *handle, int len);
/* Rx EQ */
void RX_EQ_update(void *p_rxeq_NvKey, int mode);

#endif
#endif

/* EC */
#if defined(AIR_3RD_PARTY_NR_ENABLE) && defined(AIR_ECNR_PREV_PART_ENABLE)

#if defined(AIR_ECNR_PREV_PART_USE_PIC_ENABLE)

typedef int (*p_prev_ec_get_ec120_memsize)(void);
typedef void (*p_prev_ec_Voice_EC120_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
typedef void (*p_prev_ec_Voice_EC120_Prcs)(S16* MIC1, S16* MIC2, S16* Ref ,S16* EC_out1, S16 *EC_out2);
typedef void (*p_prev_ec_Voice_PostEC120_Prcs)(S16* NR_out, S16 *out, ECNR_OUT *PAR);
typedef void (*p_prev_ec_EC_REF_GAIN_READBACK)(S16 *gain);
typedef int (*p_prev_ec_get_ec120_inear_memsize)(void);
typedef void (*p_prev_ec_Voice_EC120_Inear_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
typedef void (*p_prev_ec_Voice_EC120_Inear_Prcs)(S16* MIC1, S16* MIC2, S16* MICB, S16* REF ,S16* EC_out1, S16 *EC_out2, S16 *EC_outB);
typedef void (*p_prev_ec_EC_Inear_REF_GAIN_READBACK)(S16 *gain);
typedef int (*p_prev_ec_get_ECNR_SVN)(void);
typedef void (*p_prev_ec_EC_PreLim_Coef_READBACK)(S16 *coef);
typedef S16 (*p_prev_ec_DaulMIC_power_Coef_READBACK)(void);
typedef U8 (*p_prev_ec_PostEC120_Info)(ECNR_OUT *PAR);
typedef void (*p_prev_ec_Voice_EC80_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
typedef void (*p_prev_ec_Voice_EC80_Inear_Init)(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
#ifdef AIR_BTA_IC_PREMIUM_G3
typedef void (*p_prev_ec_Voice_SWB_EC120_Init)(void *p_ecnr_mem_ext, void *p_ecnr_NvKey, void *p_ecnr_swb_NvKey);
typedef void (*p_prev_ec_Voice_SWB_EC120_Inear_Init)(void *p_ecnr_mem_ext, void *p_ecnr_NvKey, void *p_ecnr_swb_NvKey);
typedef void (*p_prev_ec_Voice_SWB_EC120_Prcs)(S16* MIC1, S16* MIC2, S16* REF ,S16* EC_out1, S16 *EC_out2);
typedef void (*p_prev_ec_Voice_SWB_EC120_Inear_Prcs)(S16* MIC1, S16* MIC2, S16* MICB, S16* REF ,S16* EC_out1, S16 *EC_out2, S16 *EC_outB);
typedef void (*p_prev_ec_Voice_SWB_PostEC120_Prcs)(S16* NR_out, S16 *out, ECNR_OUT *PAR);
typedef void (*p_prev_ec_BAND5_EC_REF_GAIN_READBACK)(S16 *gain);
typedef void (*p_prev_ec_BAND5_PreLim_Coef_READBACK)(S16 *coef);
typedef void (*p_prev_ec_Voice_SWB_EC80_Init)(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);
typedef void (*p_prev_ec_Voice_SWB_EC80_Inear_Init)(void *p_ecnr_mem_ext, void *p_aec_NvKey, void *p_nr_NvKey, void *p_inear_NvKey, void *p_ecnr_swb_NvKey);
#endif

extern void *g_ecnr_prev_decoder_export_parameters[];

#define prev_ec_get_ec120_memsize          ((p_prev_ec_get_ec120_memsize)g_ecnr_prev_decoder_export_parameters[0])
#define prev_ec_Voice_EC120_Init           ((p_prev_ec_Voice_EC120_Init)g_ecnr_prev_decoder_export_parameters[1])
#define prev_ec_Voice_EC120_Prcs           ((p_prev_ec_Voice_EC120_Prcs)g_ecnr_prev_decoder_export_parameters[2])
#define prev_ec_Voice_PostEC120_Prcs       ((p_prev_ec_Voice_PostEC120_Prcs)g_ecnr_prev_decoder_export_parameters[3])
#define prev_ec_EC_REF_GAIN_READBACK       ((p_prev_ec_EC_REF_GAIN_READBACK)g_ecnr_prev_decoder_export_parameters[4])
#define prev_ec_get_ECNR_SVN               ((p_prev_ec_get_ECNR_SVN)g_ecnr_prev_decoder_export_parameters[5])
#define prev_ec_get_ec120_inear_memsize    ((p_prev_ec_get_ec120_inear_memsize)g_ecnr_prev_decoder_export_parameters[6])
#define prev_ec_Voice_EC120_Inear_Init     ((p_prev_ec_Voice_EC120_Inear_Init)g_ecnr_prev_decoder_export_parameters[7])
#define prev_ec_Voice_EC120_Inear_Prcs     ((p_prev_ec_Voice_EC120_Inear_Prcs)g_ecnr_prev_decoder_export_parameters[8])
#define prev_ec_EC_Inear_REF_GAIN_READBACK ((p_prev_ec_EC_Inear_REF_GAIN_READBACK)g_ecnr_prev_decoder_export_parameters[9])
#define prev_ec_EC_PreLim_Coef_READBACK    ((p_prev_ec_EC_PreLim_Coef_READBACK)g_ecnr_prev_decoder_export_parameters[10])
#define prev_ec_DaulMIC_power_Coef_READBACK ((p_prev_ec_DaulMIC_power_Coef_READBACK)g_ecnr_prev_decoder_export_parameters[11])
#define prev_ec_PostEC120_Info             ((p_prev_ec_PostEC120_Info)g_ecnr_prev_decoder_export_parameters[12])
#ifdef AIR_BTA_IC_PREMIUM_G2
#define prev_ec_Voice_EC80_Init            ((p_prev_ec_Voice_EC80_Init)g_ecnr_prev_decoder_export_parameters[13])
#define prev_ec_Voice_EC80_Inear_Init      ((p_prev_ec_Voice_EC80_Inear_Init)g_ecnr_prev_decoder_export_parameters[14])
#elif defined(AIR_BTA_IC_PREMIUM_G3)
#define prev_ec_Voice_SWB_EC120_Init        ((p_prev_ec_Voice_SWB_EC120_Init)g_ecnr_prev_decoder_export_parameters[13])
#define prev_ec_Voice_SWB_EC120_Inear_Init  ((p_prev_ec_Voice_SWB_EC120_Inear_Init)g_ecnr_prev_decoder_export_parameters[14])
#define prev_ec_Voice_SWB_EC120_Prcs        ((p_prev_ec_Voice_SWB_EC120_Prcs)g_ecnr_prev_decoder_export_parameters[15])
#define prev_ec_Voice_SWB_EC120_Inear_Prcs  ((p_prev_ec_Voice_SWB_EC120_Inear_Prcs)g_ecnr_prev_decoder_export_parameters[16])
#define prev_ec_Voice_SWB_PostEC120_Prcs    ((p_prev_ec_Voice_SWB_PostEC120_Prcs)g_ecnr_prev_decoder_export_parameters[17])
#define prev_ec_BAND5_EC_REF_GAIN_READBACK  ((p_prev_ec_BAND5_EC_REF_GAIN_READBACK)g_ecnr_prev_decoder_export_parameters[18])
#define prev_ec_BAND5_PreLim_Coef_READBACK  ((p_prev_ec_BAND5_PreLim_Coef_READBACK)g_ecnr_prev_decoder_export_parameters[19])
#define prev_ec_Voice_EC80_Init             ((p_prev_ec_Voice_EC80_Init)g_ecnr_prev_decoder_export_parameters[20])
#define prev_ec_Voice_EC80_Inear_Init       ((p_prev_ec_Voice_EC80_Inear_Init)g_ecnr_prev_decoder_export_parameters[21])
#define prev_ec_Voice_SWB_EC80_Init         ((p_prev_ec_Voice_SWB_EC80_Init)g_ecnr_prev_decoder_export_parameters[22])
#define prev_ec_Voice_SWB_EC80_Inear_Init   ((p_prev_ec_Voice_SWB_EC80_Inear_Init)g_ecnr_prev_decoder_export_parameters[23])
#endif

#else

int get_ec120_memsize(void);
void Voice_EC120_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
void Voice_EC120_Prcs(S16* MIC1, S16* MIC2, S16* Ref ,S16* EC_out1, S16 *EC_out2);
void Voice_PostEC120_Prcs(S16* NR_out, S16 *out, ECNR_OUT *PAR);
void EC_REF_GAIN_READBACK(S16 *gain);
int get_ec120_inear_memsize(void);
void Voice_EC120_Inear_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
void Voice_EC120_Inear_Prcs(S16* MIC1, S16* MIC2, S16* MICB, S16* REF ,S16* EC_out1, S16 *EC_out2, S16 *EC_outB);
void EC_Inear_REF_GAIN_READBACK(S16 *gain);
int get_ECNR_SVN(void);
void EC_PreLim_Coef_READBACK(S16 *coef);
S16 DaulMIC_power_Coef_READBACK(void);
U8 PostEC120_Info(ECNR_OUT *PAR);
void Voice_EC80_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);
void Voice_EC80_Inear_Init(int NB_mode, void *p_ecnr_mem_ext, void *p_ecnr_NvKey, S16 *NR_gain);

#endif
#endif


/* POSTEC */
#if defined(AIR_3RD_PARTY_NR_ENABLE) && defined(AIR_ECNR_POST_PART_ENABLE)

#if defined(AIR_ECNR_POST_PART_USE_PIC_ENABLE)

typedef int (*p_get_post_ec_memsize)(void);
typedef void (*p_EXT_POST_EC_Init)(void *pec_handle, void *p_ecnr_NvKey);
typedef void (*p_EXT_POST_EC_PRCS)(void *pec_handle, S16* NR_out, S16 *out, U8 PEC_GAIN, ECNR_OUT *PAR);

extern void *g_ecnr_post_decoder_export_parameters[];

#define get_post_ec_memsize         ((p_get_post_ec_memsize)g_ecnr_post_decoder_export_parameters[0])
#define EXT_POST_EC_Init            ((p_EXT_POST_EC_Init)g_ecnr_post_decoder_export_parameters[1])
#define EXT_POST_EC_PRCS            ((p_EXT_POST_EC_PRCS)g_ecnr_post_decoder_export_parameters[2])

#else

int get_post_ec_memsize(void);
void EXT_POST_EC_Init(void *pec_handle, void *p_ecnr_NvKey);
void EXT_POST_EC_PRCS(void *pec_handle, S16* NR_out, S16 *out, U8 PEC_GAIN, ECNR_OUT *PAR);

#endif

#endif

#endif /* __VOICE_NR_PORTABLE_H__ */
