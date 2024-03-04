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

#ifndef _HEARING_AID_PORTABLE_H_
#define _HEARING_AID_PORTABLE_H_

#if defined(AIR_HEARTHROUGH_HA_ENABLE) && defined(AIR_HEARTHROUGH_HA_USE_PIC)

/* Includes ------------------------------------------------------------------*/
#include "hearing_aid_interface.h"

/* Public define -------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
typedef void (*f_ha_afc_process)(void* afc, int* pIO, int blockSize);
typedef void (*f_ha_afc_put_references)(void* afc, const int* pRef, const int blockSize);
typedef void (*f_ha_afc_fs_process)(void* fs_st, int* io_data);
typedef void (*f_ha_afc_fs_pic_set_extern_functions)(void* ha_biquad_pic, void* ha_ndm_pic);
typedef int (*f_ha_inr_detect)(void* inr, int* iData, int length);
typedef void (*f_ha_inr_process)(void* inr, int* iData, int length, int wind_is_detected);
typedef void (*f_ha_inr_pic_set_extern_functions)(void* ha_biquad_pic);
typedef void (*f_ha_ola_analysis)(void* ola_st, const int* in, int* out, const int length);
typedef void (*f_ha_ola_synthesis)(void* ola_st, const int* in, int* out, const int scale, const int length);
typedef void (*f_ha_fft_fft)(const int* in, int* out, int* scratch, int n_fft);
typedef int (*f_ha_fft_ifft)(int* in, int* out, int* scratch, int n_fft, int* out_block_maxabs);
typedef void (*f_ha_nr_process)(void* pNRst, int* pFft);
typedef void (*f_ha_beamforming_process)(void* bfm_st, int* mainFFT, int* refFFT, void* scratch);
typedef int (*f_ha_drc_process)(void* drc_st, int* fft, void* scratch);
typedef void (*f_ha_drc_pic_set_extern_functions)(void* ha_math_pic);
typedef void (*f_ha_drc_analyse)(void* drc_st, int* fft, void* scratch);
typedef int (*f_ha_drc_apply)(void* drc_st, int* fft, void* scratch);
typedef int (*f_ha_wnr_calculateGain)(void* wnr_st, const int* p_main_fft, const int* p_ref_fft);
typedef void (*f_ha_wnr_applyGain)(void* wnr_st, int* p_main_fft);
typedef int (*f_ha_math_db_to_amplitude)(const int x, int* out_frac_y);
typedef void (*f_ha_calib_anc_ffmic_gain_decouple)(const void* st, const int* in, int* out, int sample_count);
typedef void (*f_ha_calib_2mic_alignment_apply)(const void* st, int* p_mic1, int* p_mic2, int sample_count);
typedef void (*f_ha_calib_mic_frequency_domain_analyze)(void* st, int* fft);
typedef void (*f_ha_calib_mic_timedomain_analyze)(void* st, const int* in, const int count);
typedef long (*f_scl_cosine32x32)(long x);
typedef long (*f_scl_sine32x32)(long x);
typedef long (*f_scl_sqrt64x32)(long long x);
typedef int (*f_ha_tln_process)(int* data, int left_shift, int max_abs, int max_limit, int length);
typedef void (*f_ha_biquad_frame_process)(void* ha_biquad_st, const int* input, int* output, int length);

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern void *g_hearing_aid_afc_export_parameters[];
extern void *g_hearing_aid_afc_fs_export_parameters[];
extern void *g_hearing_aid_inr_export_parameters[];
extern void *g_hearing_aid_ola_export_parameters[];
extern void *g_hearing_aid_fft_export_parameters[];
extern void *g_hearing_aid_nr_export_parameters[];
extern void *g_hearing_aid_bfm_export_parameters[];
extern void *g_hearing_aid_drc_export_parameters[];
extern void *g_hearing_aid_wnr_export_parameters[];
extern void *g_hearing_aid_math_export_parameters[];
extern void *g_hearing_aid_math_export_pointer;
extern void *g_hearing_aid_calib_export_parameters[];
extern void *g_hearing_aid_ndm_export_parameters[];
extern void *g_hearing_aid_ndm_export_pointer;
extern void *g_hearing_aid_tln_export_parameters[];
extern void *g_hearing_aid_biquad_export_parameters[];
extern void *g_hearing_aid_biquad_export_pointer;

/* Public functions ----------------------------------------------------------*/
#define ha_afc_process                              ((f_ha_afc_process)g_hearing_aid_afc_export_parameters[0])
#define ha_afc_put_references                       ((f_ha_afc_put_references)g_hearing_aid_afc_export_parameters[1])
#define ha_afc_fs_process                           ((f_ha_afc_fs_process)g_hearing_aid_afc_fs_export_parameters[0])
#define ha_afc_fs_pic_set_extern_functions          ((f_ha_afc_fs_pic_set_extern_functions)g_hearing_aid_afc_fs_export_parameters[1])
#define ha_inr_detect                               ((f_ha_inr_detect)g_hearing_aid_inr_export_parameters[0])
#define ha_inr_process                              ((f_ha_inr_process)g_hearing_aid_inr_export_parameters[1])
#define ha_inr_pic_set_extern_functions             ((f_ha_inr_pic_set_extern_functions)g_hearing_aid_inr_export_parameters[2])
#define ha_ola_analysis                             ((f_ha_ola_analysis)g_hearing_aid_ola_export_parameters[0])
#define ha_ola_synthesis                            ((f_ha_ola_synthesis)g_hearing_aid_ola_export_parameters[1])
#define ha_fft_fft                                  ((f_ha_fft_fft)g_hearing_aid_fft_export_parameters[0])
#define ha_fft_ifft                                 ((f_ha_fft_ifft)g_hearing_aid_fft_export_parameters[1])
#define ha_nr_process                               ((f_ha_nr_process)g_hearing_aid_nr_export_parameters[0])
#define ha_beamforming_process                      ((f_ha_beamforming_process)g_hearing_aid_bfm_export_parameters[0])
#define ha_drc_pic_set_extern_functions             ((f_ha_drc_pic_set_extern_functions)g_hearing_aid_drc_export_parameters[0])
#define ha_drc_process                              ((f_ha_drc_process)g_hearing_aid_drc_export_parameters[1])
#define ha_drc_analyse                              ((f_ha_drc_analyse)g_hearing_aid_drc_export_parameters[2])
#define ha_drc_apply                                ((f_ha_drc_apply)g_hearing_aid_drc_export_parameters[3])
#define ha_wnr_calculateGain                        ((f_ha_wnr_calculateGain)g_hearing_aid_wnr_export_parameters[0])
#define ha_wnr_applyGain                            ((f_ha_wnr_applyGain)g_hearing_aid_wnr_export_parameters[1])
#define ha_math_pic_p                               ((void*)g_hearing_aid_math_export_pointer)
#define ha_math_db_to_amplitude                     ((f_ha_math_db_to_amplitude)g_hearing_aid_math_export_parameters[3])
#define ha_calib_anc_ffmic_gain_decouple            ((f_ha_calib_anc_ffmic_gain_decouple)g_hearing_aid_calib_export_parameters[0])
#define ha_calib_2mic_alignment_apply               ((f_ha_calib_2mic_alignment_apply)g_hearing_aid_calib_export_parameters[1])
#define ha_calib_mic_frequency_domain_analyze       ((f_ha_calib_mic_frequency_domain_analyze)g_hearing_aid_calib_export_parameters[2])
#define ha_calib_mic_timedomain_analyze             ((f_ha_calib_mic_timedomain_analyze)g_hearing_aid_calib_export_parameters[3])
#define ha_ndm_pic_p                                ((void*)g_hearing_aid_ndm_export_pointer)
#define scl_cosine32x32                             ((f_scl_cosine32x32)g_hearing_aid_ndm_export_parameters[0])
#define scl_sine32x32                               ((f_scl_sine32x32)g_hearing_aid_ndm_export_parameters[1])
#define scl_sqrt64x32                               ((f_scl_sqrt64x32)g_hearing_aid_ndm_export_parameters[2])
#define ha_tln_process                              ((f_ha_tln_process)g_hearing_aid_tln_export_parameters[0])
#define ha_biquad_pic_p                             ((void*)g_hearing_aid_biquad_export_pointer)
#define ha_biquad_frame_process                     ((f_ha_biquad_frame_process)g_hearing_aid_biquad_export_parameters[0])

uint32_t hearing_aid_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage);
uint32_t hearing_aid_library_unload(void);

#endif /* AIR_HEARTHROUGH_HA_ENABLE && AIR_HEARTHROUGH_HA_USE_PIC */

#endif /* _HEARING_AID_PORTABLE_H_ */

