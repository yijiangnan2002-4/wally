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

#ifndef _VIVID_PASSTHRU_INTERFACE_H_
#define _VIVID_PASSTHRU_INTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include "dsp_sdk.h"
#include "dsp_utilities.h"
#include "dsp_para_ld_nr_nvkey_struct.h"
#include "dsp_para_vivid_pt_afc_nvkey_struct.h"

/* Public define -------------------------------------------------------------*/
#define VIVID_PASSTHRU_BLOCK_SIZE     10

#if defined(AIR_HEARTHROUGH_VIVID_PT_USE_PIC)
#define VIVID_PASSTHRU_AFC_SIZE       2128
#define VIVID_PASSTHRU_SCRATCH_SIZE   144
#else
#define VIVID_PASSTHRU_AFC_SIZE       2080
#define VIVID_PASSTHRU_SCRATCH_SIZE   48
#endif
#define VIVID_PASSTHRU_LDNR_SIZE      34896

#define VIVID_PASSTHRU_GET_ALIGN_SIZE(SIZE)            ((SIZE + 15) & 0x7FFFFFF0)

#define VIVID_PASSTHRU_AFC_MASTER_MEM_POSITION      0
#define VIVID_PASSTHRU_LDNR_MEM_POSITION            VIVID_PASSTHRU_AFC_MASTER_MEM_POSITION  + VIVID_PASSTHRU_GET_ALIGN_SIZE(VIVID_PASSTHRU_AFC_SIZE)
#define VIVID_PASSTHRU_SCRATCH_MEM_POSITION         VIVID_PASSTHRU_LDNR_MEM_POSITION        + VIVID_PASSTHRU_GET_ALIGN_SIZE(VIVID_PASSTHRU_LDNR_SIZE)
#define VIVID_PASSTHRU_ALG_TOTAL_MEM                VIVID_PASSTHRU_SCRATCH_MEM_POSITION     + VIVID_PASSTHRU_GET_ALIGN_SIZE(VIVID_PASSTHRU_SCRATCH_SIZE)

#define DSP_VIVID_PASSTHRU_MEMSIZE    (sizeof(vivid_pt_instance) + VIVID_PASSTHRU_ALG_TOTAL_MEM)

/* Public typedef ------------------------------------------------------------*/
typedef VOID (*VIVID_PT_LDNR_BG_ENTRY)(VOID);

typedef struct {
    S8  gse_frac_index;
    U8  reserved;
    U8  gse_shift;
    U8  max_gse_frac_index;
    S32 ref_energy_sum;
    S32 count;
} PACKED vivid_pt_vptafc_acs_t;

typedef struct {
    U8  mute_state;
    U8  Reserved[3];
    S32 input_energy_sum;
    S32 feedback_energy_sum;
    S32 detect_times;
} PACKED vivid_pt_vptafc_hs_t;

typedef struct {
    S32 current;
    S32 target;
    U32 state;
    S32 times;
    S32 step;
} PACKED vivid_pt_mic_smooth_t;

typedef struct {
	S32 hs_input_pwr;
	S32 hs_feedback_pwr;
	S32 hs_ref_pwr;
	S32 hs_gain;
} vivid_pt_vptafc_state_t;

typedef struct {
    DSP_PARA_AT_LD_STRU         ld_nr;
    DSP_PARA_VIVID_PT_AFC_STRU  afc;
} PACKED vivid_pt_nvkey_t;

#define VIVID_PT_NVKEY_SIZE    (sizeof(vivid_pt_nvkey_t))

typedef struct {
    U32                     memory_check;
    U8                      nvkey_ready;
    U8                      init_done;
    U8                      vpt_afc_hs_debug;
    U8                      vpt_afc_acs_debug;
    U8                      bypass_enable;
    U8                      reversed[3];
    vivid_pt_vptafc_state_t *vpt_afc_state;
    vivid_pt_vptafc_hs_t    vpt_afc_hs_state;
    vivid_pt_vptafc_acs_t   vpt_afc_acs_state;
    vivid_pt_mic_smooth_t   mic_out_smooth_ctl;
    U32                     ldnr_block_count;
    U32                     ldnr_background_process_ready;
    U32                     delay_block_count;
    S32                     inbuf_backup[VIVID_PASSTHRU_BLOCK_SIZE];
    vivid_pt_nvkey_t        nvkey;
    DSP_ALIGN16 U32         scratch_memory;
} vivid_pt_instance, *vivid_pt_instance_ptr;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern VIVID_PT_LDNR_BG_ENTRY vivid_pt_ldnr_bg_entry;

/* Public functions ----------------------------------------------------------*/
void stream_function_vivid_passthru_set_runtime_config(U8 event, U32 param);
void stream_function_vivid_passthru_set_afc_switch(U32 param);
void stream_function_vivid_passthru_set_ldnr_switch(U32 param);
void stream_function_vivid_passthru_mute(bool is_mute);
void stream_function_vivid_passthru_set_bypass_switch(U8 param);
void stream_function_vivid_passthru_deinitialize(void);
bool stream_function_vivid_passthru_load_nvkey(U16 nvkey_id, void *nvkey);
bool stream_function_vivid_passthru_initialize(void *para);
bool stream_function_vivid_passthru_process(void *para);
bool stream_function_vivid_passthru_post_proc_initialize(void *para);
bool stream_function_vivid_passthru_post_proc_process(void *para);



#endif /* _VIVID_PASSTHRU_INTERFACE_H_ */
