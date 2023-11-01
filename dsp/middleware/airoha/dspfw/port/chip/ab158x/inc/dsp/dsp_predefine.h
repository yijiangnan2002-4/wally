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

#ifndef _DSP_PREDEFINE_H_
#define _DSP_PREDEFINE_H_

#include "dsp_sdk.h"

#include "dsp_feature_interface.h"
#include "dsp_stream_connect.h"
#include "dsp_audio_process.h"
#ifdef AIR_BT_HFP_CVSD_ENABLE
#include "cvsd_enc_interface.h"
#include "cvsd_dec_interface.h"
#endif
#ifdef AIR_BT_HFP_MSBC_ENABLE
#include "msbc_enc_interface.h"
#include "msbc_dec_interface.h"
#endif
#include "dprt_rt.h"
#ifdef MTK_BT_A2DP_SBC_ENABLE
#include "sbc_interface.h"
#endif /* MTK_BT_A2DP_SBC_ENABLE */
#ifdef MTK_BT_A2DP_AAC_ENABLE
#include "aac_dec_interface.h"
#endif /* MTK_BT_A2DP_AAC_ENABLE */
#include "compander_interface.h"

#ifdef AIR_VOICE_NR_ENABLE
#include "voice_nr_interface.h"
#include "voice_nr_portable.h"
#ifdef AIR_HFP_DNN_PATH_ENABLE
#include "dnn_nr_interface.h"
#endif
#endif

#include "clk_skew.h"
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#include "peq_interface.h"
#endif
#ifdef MTK_LINEIN_INS_ENABLE
#include "ins_interface.h"
#endif
#ifdef MTK_WWE_ENABLE
#include "wwe_interface.h"
#endif

#include "dsp_vendor_codec_sdk.h"

#include "ch_select_interface.h"
#include "mute_smooth_interface.h"
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "leakage_compensation.h"
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#include "user_trigger_adaptive_ff.h"
#endif

#ifdef MTK_VOICE_AGC_ENABLE
#include "agc_interface.h"
#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
#include "gsensor_processing.h"
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
#include "audio_plc_interface.h"
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "lc3_enc_interface.h"
#include "lc3_dec_interface.h"
#endif

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
#include "audio_loopback_test_interface.h"
#endif

#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif
#ifdef AIR_CELT_ENC_ENABLE
#include "celt_enc_interface.h"
#endif
#ifdef AIR_SOFTWARE_SRC_ENABLE
#include "sw_src_interface.h"
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
#include "clk_skew_sw.h"
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
#include "sw_mixer_interface.h"
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
#include "sw_buffer_interface.h"
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
#include "wind_interface.h"
#endif
#ifdef AIR_AFC_ENABLE
#include "afc_interface.h"
#endif
#ifdef AIR_LD_NR_ENABLE
#include "ld_nr_interface.h"
#endif
#ifdef AIR_AT_AGC_ENABLE
#include "at_agc_interface.h"
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
#include "lc3_enc_branch_interface.h"
#include "lc3_dec_interface_v2.h"
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#ifdef AIR_FIXED_RATIO_SRC
#include "src_fixed_ratio_interface.h"
#endif



////////////////////////////////////////////////////////////////////////////////
// CONSTANT DEFINITIONS ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define DSP_MEMORY_COMMON_SUBCOMPONENT_ID   (SubComponent_Max+1)
#define DSP_MEMORY_NO_USE                   (SubComponent_No_Used_Memory)

#define DSP_COMPONENT_NO_USE                (Component_No_Used_Memory)

////////////////////////////////////////////////////////////////////////////////
// Undefine Subcomponent type/////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*A2DP component */
#ifndef MTK_BT_A2DP_VENDOR_ENABLE
#define SubComponent_A2DP_VENDOR            DSP_MEMORY_NO_USE
#endif

#ifndef MTK_AUDIO_PLC_ENABLE
#define SubComponent_A2DP_PLC               DSP_MEMORY_NO_USE
#endif

#ifndef MTK_PEQ_ENABLE
#define SubComponent_A2DP_PEQ               DSP_MEMORY_NO_USE
#define SubComponent_A2DP_PEQ2              DSP_MEMORY_NO_USE
#define SubComponent_A2DP_DRC               DSP_MEMORY_NO_USE
#endif

/*HFP component */
#ifndef AIR_BT_HFP_MSBC_ENABLE
#define SubComponent_HFP_MSBC_DEC           DSP_MEMORY_NO_USE
#define SubComponent_HFP_MSBC_ENC           DSP_MEMORY_NO_USE
#endif

#ifndef AIR_VOICE_PLC_ENABLE
#define SubComponent_HFP_PLC                DSP_MEMORY_NO_USE
#endif

#ifndef AIR_VOICE_NR_ENABLE
#define SubComponent_HFP_RX_NR              DSP_MEMORY_NO_USE
#define SubComponent_HFP_TX_NR              DSP_MEMORY_NO_USE
#endif

#ifndef AIR_VOICE_DRC_ENABLE
#define SubComponent_HFP_RX_DRC             DSP_MEMORY_NO_USE
#define SubComponent_HFP_TX_DRC             DSP_MEMORY_NO_USE
#endif

#ifndef AIR_3RD_PARTY_NR_ENABLE
#define SubComponent_HFP_TX_EQ              DSP_MEMORY_NO_USE
#endif

#ifndef AIR_BT_HFP_ENABLE
#define SubComponent_HFP_RX_FW              DSP_MEMORY_NO_USE
#define SubComponent_HFP_TX_FW              DSP_MEMORY_NO_USE
#define SubComponent_HFP_RX_STREAM_IN       DSP_MEMORY_NO_USE
#define SubComponent_HFP_RX_STREAM_OUT      DSP_MEMORY_NO_USE
#define SubComponent_HFP_TX_STREAM_IN       DSP_MEMORY_NO_USE
#endif

/*Le Audio/Le Call component */
#ifndef AIR_BT_CODEC_BLE_ENABLED
#define SubComponent_LEAUDIO_DL_FW          DSP_MEMORY_NO_USE
#define SubComponent_LEAUDIO_DL_STREAM_IN   DSP_MEMORY_NO_USE
#define SubComponent_LEAUDIO_DL_STREAM_OUT  DSP_MEMORY_NO_USE

#define SubComponent_LECALL_UL_FW           DSP_MEMORY_NO_USE
#define SubComponent_LECALL_UL_STREAM_IN    DSP_MEMORY_NO_USE
#define SubComponent_LECALL_DL_FW           DSP_MEMORY_NO_USE
#define SubComponent_LECALL_DL_STREAM_IN    DSP_MEMORY_NO_USE
#define SubComponent_LECALL_DL_STREAM_OUT   DSP_MEMORY_NO_USE
#endif

/* MultiMic Functions*/
#ifndef AIR_MULTI_MIC_STREAM_ENABLE
#define SubComponent_ESA_FW                 DSP_MEMORY_NO_USE
#define SubComponent_ESA_STREAM_IN          DSP_MEMORY_NO_USE
#define SubComponent_ESA_FUNC_F             DSP_MEMORY_NO_USE


#define SubComponent_MULTIMIC_FW1           DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_FW1_STREAM_IN DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_GAIN          DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_FUNC_D        DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_FUNC_E        DSP_MEMORY_NO_USE


#define SubComponent_MULTIMIC_FUNC_A        DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_FUNC_B        DSP_MEMORY_NO_USE
#define SubComponent_MULTIMIC_FUNC_C        DSP_MEMORY_NO_USE
#endif

#ifndef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
#define SubComponent_TDM_FW                 DSP_MEMORY_NO_USE
#define SubComponent_TDM_STREAM_IN          DSP_MEMORY_NO_USE
#define SubComponent_TDM_STREAM_OUT         DSP_MEMORY_NO_USE
#endif

////////////////////////////////////////////////////////////////////////////////
// PreDefine Size Macro  ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * A2DP
 **/
#define DSP_PREDEFINE_A2DP_SBC_IRAM                 4397
#define DSP_PREDEFINE_A2DP_SBC_DRAM                 9520    // DSP_SBC_CODEC_MEMSIZE + 5376
#define DSP_PREDEFINE_A2DP_SBC_SYSRAM               0

#define DSP_PREDEFINE_A2DP_AAC_IRAM                 15788
#define DSP_PREDEFINE_A2DP_AAC_DRAM                 64624   //DSP_AAC_DECODER_MEMSIZE + 23856
#define DSP_PREDEFINE_A2DP_AAC_SYSRAM               0

#define DSP_PREDEFINE_A2DP_VENDOR_IRAM              14685
#define DSP_PREDEFINE_A2DP_VENDOR_DRAM              30768   //VENDOR_HANDLE_SIZE + 14168
#define DSP_PREDEFINE_A2DP_VENDOR_SYSRAM            0

/**
 * HFP
 **/
#define DSP_PREDEFINE_HFP_MSBC_DEC_IRAM             8800
#define DSP_PREDEFINE_HFP_MSBC_DEC_DRAM             9552
#define DSP_PREDEFINE_HFP_MSBC_DEC_SYSRAM           0

#define DSP_PREDEFINE_HFP_PLC_IRAM                  9800
#define DSP_PREDEFINE_HFP_PLC_DRAM                  6944
#define DSP_PREDEFINE_HFP_PLC_SYSRAM                0

#define DSP_PREDEFINE_HFP_ECNR_LIB_IRAM             62000
#define DSP_PREDEFINE_HFP_ECNR_LIB_DRAM             258000
#define DSP_PREDEFINE_HFP_ECNR_LIB_SYSRAM           0

#define DSP_PREDEFINE_HFP_DRC_LIB_IRAM              7200
#define DSP_PREDEFINE_HFP_DRC_LIB_DRAM              2000
#define DSP_PREDEFINE_HFP_DRC_LIB_SYSRAM            0

#define DSP_PREDEFINE_HFP_CLKSKEW_LIB_IRAM          6700
#define DSP_PREDEFINE_HFP_CLKSKEW_LIB_DRAM          1200
#define DSP_PREDEFINE_HFP_CLKSKEW_LIB_SYSRAM        0

#define DSP_PREDEFINE_HFP_MSBC_ENC_IRAM             5000
#define DSP_PREDEFINE_HFP_MSBC_ENC_DRAM             3500
#define DSP_PREDEFINE_HFP_MSBC_ENC_SYSRAM           0

#define DSP_PREDEFINE_HFP_ECNR_DL_IRAM              0
#define DSP_PREDEFINE_HFP_ECNR_DL_DRAM              88352
#define DSP_PREDEFINE_HFP_ECNR_DL_SYSRAM            0

#define DSP_PREDEFINE_HFP_TX_EQ_IRAM                0
#define DSP_PREDEFINE_HFP_TX_EQ_DRAM                0
#define DSP_PREDEFINE_HFP_TX_EQ_SYSRAM              0

#define DSP_PREDEFINE_HFP_VOICE_DRC_DL_IRAM         0
#define DSP_PREDEFINE_HFP_VOICE_DRC_DL_DRAM         672
#define DSP_PREDEFINE_HFP_VOICE_DRC_DL_SYSRAM       0

#define DSP_PREDEFINE_HFP_CLKSKEW_DL_IRAM           0
#define DSP_PREDEFINE_HFP_CLKSKEW_DL_DRAM           1300
#define DSP_PREDEFINE_HFP_CLKSKEW_DL_SYSRAM         0

#define DSP_PREDEFINE_HFP_RX_FW_IRAM                0
#define DSP_PREDEFINE_HFP_RX_FW_DRAM                160
#define DSP_PREDEFINE_HFP_RX_FW_SYSRAM              0

#define DSP_PREDEFINE_HFP_TX_FW_IRAM                0
#define DSP_PREDEFINE_HFP_TX_FW_DRAM                160
#define DSP_PREDEFINE_HFP_TX_FW_SYSRAM              0

#define DSP_PREDEFINE_HFP_RX_STREAM_IN_IRAM         0
#define DSP_PREDEFINE_HFP_RX_STREAM_IN_DRAM         1024
#define DSP_PREDEFINE_HFP_RX_STREAM_IN_SYSRAM       0

#define DSP_PREDEFINE_HFP_RX_STREAM_OUT_IRAM        0
#define DSP_PREDEFINE_HFP_RX_STREAM_OUT_DRAM        1024
#define DSP_PREDEFINE_HFP_RX_STREAM_OUT_SYSRAM      0

#define DSP_PREDEFINE_HFP_TX_STREAM_IN_IRAM         0
#define DSP_PREDEFINE_HFP_TX_STREAM_IN_DRAM         2112
#define DSP_PREDEFINE_HFP_TX_STREAM_IN_SYSRAM       0

#define DSP_PREDEFINE_HFP_CLKSKEW_UL_IRAM           0
#define DSP_PREDEFINE_HFP_CLKSKEW_UL_DRAM           1800
#define DSP_PREDEFINE_HFP_CLKSKEW_UL_SYSRAM         0

#define DSP_PREDEFINE_HFP_MIC_SW_GAIN_IRAM          0
#define DSP_PREDEFINE_HFP_MIC_SW_GAIN_DRAM          64
#define DSP_PREDEFINE_HFP_MIC_SW_GAIN_SYSRAM        0

#define DSP_PREDEFINE_HFP_VOICE_DRC_UL_IRAM         0
#define DSP_PREDEFINE_HFP_VOICE_DRC_UL_DRAM         672
#define DSP_PREDEFINE_HFP_VOICE_DRC_UL_SYSRAM       0

#endif /* _DSP_PREDEFINE_H_ */

