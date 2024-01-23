/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef  __LC3_DEC_PORTABLE_H__
#define  __LC3_DEC_PORTABLE_H__

#ifdef MTK_BT_A2DP_LC3_USE_PIC

#include "lc3_codec_api.h"

typedef uint32_t (*LC3_Enc_Get_MemSize_t)(void);
typedef LC3_ERR_T(*LC3_Enc_Init_t)(void *p_lc3_mem_ext, uint16_t bits, uint32_t SampleRate, uint16_t nChannels, uint32_t BitRate, uint16_t frame_ms, uint16_t delay);
typedef LC3_ERR_T(*LC3_Enc_Prcs_t)(void *p_lc3_mem_ext, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes);
typedef LC3_ERR_T(*LC3_Enc_Set_BitRate_t)(void *p_lc3_mem_ext, uint32_t BitRate);

typedef uint32_t (*LC3_Dec_Get_MemSize_t)(LC3_DEC_MODE_T mode);
typedef void (*LC3_Dec_Get_Param_t)(void *p_lc3_mem_ext, uint32_t *nSamples, uint32_t *delay);
typedef LC3_ERR_T(*LC3_Dec_Init_t)(void *p_lc3_mem_ext, uint32_t bits, uint32_t SampleRate, uint32_t nChannels, LC3_DEC_MODE_T mode, LC3_PLC_MODE_T plcMeth, uint16_t frame_ms, uint16_t delay);
typedef LC3_ERR_T(*LC3_Dec_Prcs_t)(void *p_lc3_mem_ext, uint8_t *bytes, uint8_t *BufO, uint32_t nBytes, uint16_t packet_lost_st, uint32_t bfi_ext);

typedef uint32_t (*LC3_Get_Version_t)(void);

/* Referrence to the APIs of LC3 library after preloader load done */
extern void *g_lc3_codec_export_parameters[];

#define LC3_Enc_Get_MemSize ((LC3_Enc_Get_MemSize_t)g_lc3_codec_export_parameters[0])
#define LC3_Enc_Init ((LC3_Enc_Init_t)g_lc3_codec_export_parameters[1])
#define LC3_Enc_Prcs ((LC3_Enc_Prcs_t)g_lc3_codec_export_parameters[2])
#define LC3_Enc_Set_BitRate ((LC3_Enc_Set_BitRate_t)g_lc3_codec_export_parameters[3])

#define LC3_Dec_Get_MemSize ((LC3_Dec_Get_MemSize_t)g_lc3_codec_export_parameters[4])
#define LC3_Dec_Get_Param ((LC3_Dec_Get_Param_t)g_lc3_codec_export_parameters[5])
#define LC3_Dec_Init ((LC3_Dec_Init_t)g_lc3_codec_export_parameters[6])
#define LC3_Dec_Prcs ((LC3_Dec_Prcs_t)g_lc3_codec_export_parameters[7])

#define LC3_Get_Version ((LC3_Get_Version_t)g_lc3_codec_export_parameters[8])

typedef void (*fix_fft_Init_7P5_MS_t)(void);
typedef void (*fix_fft15_7P5_MS_t)(void);
typedef void (*fix_fft30_7P5_MS_t)(void);
typedef void (*fix_fft40_7P5_MS_t)(void);
typedef void (*FFT4N_7P5_MS_t)(void);
typedef void (*FFT8N_7P5_MS_t)(void);
typedef void (*FFT12N_7P5_MS_t)(void);

typedef void (*fix_fft_Init_10_MS_t)(void);
typedef void (*fix_fft10_10_MS_t)(void);
typedef void (*fix_fft15_10_MS_t)(void);
typedef void (*fix_fft20_10_MS_t)(void);
typedef void (*fix_fft30_10_MS_t)(void);
typedef void (*fix_fft40_10_MS_t)(void);
typedef void (*FFT8N_10_MS_t)(void);

#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
typedef LC3_ERR_T(*LC3PLUSN_Enc_Prcs_t)(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes, Multi_FFT *FFTx);
typedef LC3_ERR_T(*LC3PLUSN_Dec_Prcs_t)(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufO, int32_t nBytes, int16_t packet_lost_st, int32_t bfi_ext, Multi_FFT *FFTx);
typedef void (*LC3PLUSN_Dec_Get_Param_t)(void *p_lc3i_ptr, uint32_t *nSamples, int32_t *delay);
#else
typedef LC3_ERR_T(*LC3I_Enc_Prcs_t)(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes, Multi_FFT *FFTx);
typedef LC3_ERR_T(*LC3I_Dec_Prcs_t)(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufO, int32_t nBytes, int16_t packet_lost_st, int32_t bfi_ext, Multi_FFT *FFTx);
typedef void (*LC3I_Dec_Get_Param_t)(void *p_lc3i_ptr, uint32_t *nSamples, int32_t *delay);
#endif

extern void *g_lc3i_plc_std_export_parameters[];
#define processPLCmain_Std_fx ((processPLCmain_Std_fx_t)g_lc3i_plc_std_export_parameters[0])

extern void *g_lc3i_fft7_5ms_export_parameters[];
#define fix_fft_Init_7P5_MS ((fix_fft_Init_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[0])
#define fix_fft15_7P5_MS ((fix_fft15_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[1])
#define fix_fft30_7P5_MS ((fix_fft30_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[2])
#define fix_fft40_7P5_MS ((fix_fft40_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[3])
#define FFT4N_7P5_MS ((FFT4N_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[4])
#define FFT8N_7P5_MS ((FFT8N_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[5])
#define FFT12N_7P5_MS ((FFT12N_7P5_MS_t)g_lc3i_fft7_5ms_export_parameters[6])

extern void *g_lc3i_fft10ms_export_parameters[];
#define fix_fft_Init_10MS ((fix_fft_Init_10_MS_t)g_lc3i_fft10ms_export_parameters[0])
#define fix_fft10_10_MS ((fix_fft10_10_MS_t)g_lc3i_fft10ms_export_parameters[1])
#define fix_fft15_10_MS ((fix_fft15_10_MS_t)g_lc3i_fft10ms_export_parameters[2])
#define fix_fft20_10_MS ((fix_fft20_10_MS_t)g_lc3i_fft10ms_export_parameters[3])
#define fix_fft30_10_MS ((fix_fft30_10_MS_t)g_lc3i_fft10ms_export_parameters[4])
#define fix_fft40_10_MS ((fix_fft40_10_MS_t)g_lc3i_fft10ms_export_parameters[5])
#define FFT8N_10_MS ((FFT8N_10_MS_t)g_lc3i_fft10ms_export_parameters[6])

extern void *g_lc3i_codec_export_parameters[];
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
#define LC3PLUSN_Enc_Prcs ((LC3PLUSN_Enc_Prcs_t)g_lc3i_codec_export_parameters[0])
#define LC3PLUSN_Dec_Prcs ((LC3PLUSN_Dec_Prcs_t)g_lc3i_codec_export_parameters[1])
#define LC3PLUSN_Dec_Get_Param ((LC3PLUSN_Dec_Get_Param_t)g_lc3i_codec_export_parameters[2])
#else
#define LC3I_Enc_Prcs ((LC3I_Enc_Prcs_t)g_lc3i_codec_export_parameters[0])
#define LC3I_Dec_Prcs ((LC3I_Dec_Prcs_t)g_lc3i_codec_export_parameters[1])
#define LC3I_Dec_Get_Param ((LC3I_Dec_Get_Param_t)g_lc3i_codec_export_parameters[2])
#endif

extern void lc3i_fft7_5ms_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage);
extern uint32_t lc3i_fft7_5ms_library_unload(void);
extern void lc3i_fft10ms_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage);
extern uint32_t lc3i_fft10ms_library_unload(void);
extern uint32_t lc3i_codec_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage);
extern uint32_t lc3i_codec_library_unload(void);

extern uint32_t LC3I_Get_Version(void);
extern void LC3I_Set_ROM_Start(void *lc3_addr);

#endif

#endif

