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

//-
#include <string.h>

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

#ifdef AIR_BT_CLK_SKEW_ENABLE
#include "clk_skew.h"
#include "clk_skew_init.h"
#endif
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

#ifdef AIR_BT_LE_LC3_ENABLE
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

#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
#include "lc3plus_enc_interface.h"
#include "lc3plus_dec_interface.h"
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */

#if defined(AIR_BT_LHDC_ENCODER_ENABLE)
#include "lhdc_enc_interface.h"
#endif /* AIR_BT_LHDC_ENCODER_ENABLE */

#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
#include "uld_enc_interface.h"
#endif

#ifdef AIR_AUDIO_ULD_DECODE_ENABLE
#include "uld_dec_interface.h"
#endif

#if defined(AIR_CELT_ENC_V2_ENABLE)
#include "celt_enc_interface_v2.h"
#endif /* AIR_CELT_ENC_V2_ENABLE */

#if defined(AIR_CELT_DEC_V2_ENABLE)
#include "celt_dec_interface_v2.h"
#endif /* AIR_CELT_DEC_V2_ENABLE */

#ifdef AIR_FIXED_RATIO_SRC
#include "src_fixed_ratio_interface.h"
#endif

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
#include "full_adapt_anc_api.h"
#endif

#ifdef AIR_HW_VIVID_PT_ENABLE
#include "hw_vivid_passthru_api.h"
#endif

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
#include "user_unaware_interface.h"
#endif

#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
#include "environment_detection_interface.h"
#endif

#ifdef AIR_ADAPTIVE_EQ_ENABLE
#include "adaptive_eq_interface.h"
#endif

#include "queue_buffer_interface.h"

#if defined(AIR_HEARING_AID_ENABLE)
#include "hearing_aid_interface.h"
#elif defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "hearthrough_psap_interface.h"
#endif
#if defined (AIR_HEARTHROUGH_VIVID_PT_ENABLE)
#include "vivid_passthru_interface.h"
#endif

#ifdef AIR_BT_A2DP_SBC_ENCODER_ENABLE
#include "sbc_encoder_interface.h"
#endif

#if defined(AIR_SURROUND_AUDIO_ENABLE)
#include "surround_audio.h"
#endif /* AIR_SURROUND_AUDIO_ENABLE */

#ifdef AIR_SOFTWARE_DRC_ENABLE
#include "compander_interface_sw.h"
#endif /* AIR_SOFTWARE_DRC_ENABLE */
#if defined(AIR_DCHS_MODE_ENABLE)
#include "stream_dchs.h"
#endif

#if defined(AIR_CUSTOMIZED_LLF_ENABLE)
#include "low_latency_framework_exsample.h"
#endif

#if defined(AIR_VOLUME_ESTIMATOR_ENABLE)
#include "volume_estimator_interface.h"
#endif

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
#include "lhdc_enc_interface.h"
#define STREAM_CODEC_ENCODER_VENDOR_INITIALIZE   (stream_codec_encoder_lhdc_initialize)
#define STREAM_CODEC_ENCODER_VENDOR_PROCESS      (stream_codec_encoder_lhdc_process)
#define STREAM_CODEC_ENCODER_VERNDOR_MEM_SIZE    (DSP_LHDC_ENCODER_MEMSIZE)
#else
#define STREAM_CODEC_ENCODER_VENDOR_INITIALIZE   (NULL)
#define STREAM_CODEC_ENCODER_VENDOR_PROCESS      (NULL)
#define STREAM_CODEC_ENCODER_VERNDOR_MEM_SIZE    (0)
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */

////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
typedef struct stream_codec_sample_instance_u {
    uint8_t scratch_memory[128];
    uint8_t output[512];
    bool memory_check;
    bool reset_check;
} stream_codec_sample_instance_t, *stream_codec_sample_instance_ptr_t;

typedef struct stream_function_sample_instance_u {
    uint32_t coefficient_size;
    uint8_t filter[32];
    int16_t buffer_l[512];
    int16_t buffer_r[512];
} stream_function_sample_instance_t, *stream_function_sample_instance_ptr_t;


////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CODEC_SAMPLE_MEMORY_SIZE        sizeof(stream_codec_sample_instance_t)
#define CODEC_OUTPUT_SIZE               1024

#define FUNCTION_SAMPLE_MEMORY_SIZE     sizeof(stream_function_sample_instance_t)




////////////////////////////////////////////////////////////////////////////////
// Function Prototypes /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool stream_codec_decoder_sample_initialize(void *para);
bool stream_codec_decoder_sample_process(void *para);
bool stream_function_sample_initialize(void *para);
bool stream_function_sample_process(void *para);
uint32_t stream_feature_sample_open(void *code_address, void *data_address, uint32_t *dram_pic_usage);
uint32_t stream_feature_sample_close(void);
uint32_t codec_decoder_sample_api(uint8_t *pattern_pointer, int16_t *l_pointer, int16_t *r_pointer, uint32_t input_length);
uint32_t function_sample_api(stream_function_sample_instance_ptr_t instance_ptr, int16_t *input_l_pointer, int16_t *input_r_pointer, int16_t *output_l_pointer, int16_t *output_r_pointer, uint32_t length);


////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*===========================================================================================*/
// Codec blocks
// CodecOutSize must have value
stream_feature_table_t stream_feature_table[DSP_FEATURE_MAX_NUM] = {
    /*====================================================================================    stream feature codec    ====================================================================================*/
    /*feature_type,                                              memory_size,          codec_output_size,                               *initialize_entry,                                  *process_entry*/
    {CODEC_PCM_COPY,                                                       0,                       2048,                      stream_pcm_copy_initialize,                         stream_pcm_copy_process},/*0x00 CODEC_PCM_COPY               */ /* Only copy in_ptr memory to out_ptr memory */
#ifdef AIR_BT_HFP_CVSD_ENABLE
    {CODEC_DECODER_CVSD,                            DSP_CVSD_DECODER_MEMSIZE,  480 + DSP_SIZE_FOR_CLK_SKEW,            stream_codec_decoder_cvsd_initialize,               stream_codec_decoder_cvsd_process}, /*0x01 CVSD_Decoder,    */ // 120(NB), 240(WB)
#else
    {CODEC_DECODER_CVSD,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x01 CVSD_Decoder,    */   // 120(NB), 240(WB)
#endif
#ifdef AIR_BT_HFP_MSBC_ENABLE
    {CODEC_DECODER_MSBC,                            DSP_MSBC_DECODER_MEMSIZE,  480 + DSP_SIZE_FOR_CLK_SKEW,            stream_codec_decoder_msbc_initialize,               stream_codec_decoder_msbc_process}, /*0x02 CODEC_DECODER_MSBC,      */
#else
    {CODEC_DECODER_MSBC,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x02 CODEC_DECODER_MSBC,      */
#endif
#ifdef MTK_BT_A2DP_SBC_ENABLE
    {CODEC_DECODER_SBC,                       DSP_SBC_CODEC_MEMSIZE, 4096 + 1024 + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_decoder_sbc_initialize,                stream_codec_decoder_sbc_process}, /*0x03 CODEC_DECODER_SBC,       */
#else
    {CODEC_DECODER_SBC,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x03 CODEC_DECODER_SBC,       */
#endif
#ifdef MTK_BT_A2DP_AAC_ENABLE
    {CODEC_DECODER_AAC,              DSP_AAC_DECODER_MEMSIZE + 20000, 4096 + 1024 + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_decoder_aac_initialize,                stream_codec_decoder_aac_process}, /*0x04 CODEC_DECODER_AAC,       */
#else
    {CODEC_DECODER_AAC,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x04 CODEC_DECODER_AAC,       */
#endif
    {CODEC_DECODER_MP3,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x05 _Reserved,       */
    {CODEC_DECODER_EC,                                                     0,                       0xFF,                                            NULL,                                            NULL},/*0x06 _Reserved,       */
    {CODEC_DECODER_UART,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x07 _Reserved,       */
#if defined(AIR_CELT_DEC_V2_ENABLE)
    {CODEC_DECODER_OPUS_V2,                                                0,                       0xFF,          stream_codec_decoder_celt_v2_initialize,             stream_codec_decoder_celt_v2_process},/*0x08 CODEC_DECODER_OPUS_V2,       */
#else
    {CODEC_DECODER_OPUS_V2,                                                0,                       0xFF,                                            NULL,                                            NULL},/*0x08 CODEC_DECODER_OPUS_V2,       */
#endif /* AIR_CELT_DEC_V2_ENABLE */
#ifdef MTK_CELT_DEC_ENABLE
    {CODEC_DECODER_CELT_HD,                             DSP_CELT_DEC_MEMSIZE, 4096 + DSP_SIZE_FOR_CLK_SKEW,            stream_codec_decoder_celt_initialize,               stream_codec_decoder_celt_process}, /*0x09 CODEC_DECODER_CELT_HD,   */
#else
    {CODEC_DECODER_CELT_HD,                                                0,                       0xFF,                                            NULL,                                            NULL},/*0x09 CODEC_DECODER_CELT_HD,   */
#endif
#ifdef MTK_BT_A2DP_VENDOR_ENABLE
    {CODEC_DECODER_VENDOR,                       VENDOR_HANDLE_SIZE, 4096 + 1024 + DSP_SIZE_FOR_CLK_SKEW,          STREAM_CODEC_DECODER_VENDOR_INITIALIZE,             STREAM_CODEC_DECODER_VENDOR_PROCESS}, /*0x0A _CODEC_DECODER_VENDOR,       */
#else
    {CODEC_DECODER_VENDOR,                                                 0,                       0xFF,                                            NULL,                                            NULL},/*0x0A _CODEC_DECODER_VENDOR,       */
#endif
    {CODEC_DECODER_VENDOR_1,                              VENDOR_1_HANDLE_SIZE, VENDOR_1_CODEC_OUT_SIZE + DSP_SIZE_FOR_CLK_SKEW,      STREAM_CODEC_DECODER_VENDOR_1_INITIALIZE,           STREAM_CODEC_DECODER_VENDOR_1_PROCESS}, /*0x0B _CODEC_DECODER_VENDOR,       */
    {CODEC_DECODER_VENDOR_2,                              VENDOR_2_HANDLE_SIZE, VENDOR_2_CODEC_OUT_SIZE + DSP_SIZE_FOR_CLK_SKEW,      STREAM_CODEC_DECODER_VENDOR_2_INITIALIZE,           STREAM_CODEC_DECODER_VENDOR_2_PROCESS}, /*0x0C _CODEC_DECODER_VENDOR,       */
#ifdef AIR_AUDIO_ULD_DECODE_ENABLE
    {CODEC_DECODER_ULD,                                                    0,                          0,             stream_codec_decoder_uld_initialize,                stream_codec_decoder_uld_process}, /*0x0D _CODEC_DECODER_ULD,       */
#else
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL}, /*0x0D _CODEC_DECODER_ULD,       */
#endif
#ifdef AIR_BT_LE_LC3_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    {CODEC_DECODER_LC3,                                                52976,  DSP_LC3_DECODER_OUT_BUF_SIZE + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_decoder_lc3_initialize,                stream_codec_decoder_lc3_process}, /*0x0E CODEC_DECODER_LC3,       */
#else
    {CODEC_DECODER_LC3,                                                    0,  DSP_LC3_DECODER_OUT_BUF_SIZE + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_decoder_lc3_initialize,                stream_codec_decoder_lc3_process}, /*0x0E CODEC_DECODER_LC3,       */
#endif
#elif defined(AIR_BT_CODEC_BLE_V2_ENABLED)
    {CODEC_DECODER_LC3,                                                    0,                       0xFF,          stream_codec_decoder_lc3_v2_initialize,             stream_codec_decoder_lc3_v2_process},/*0x0E CODEC_DECODER_LC3,       */
#else
    {CODEC_DECODER_LC3,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x0E CODEC_DECODER_LC3,       */
#endif
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    {CODEC_DECODER_LC3PLUS,                                                0,  DSP_LC3_PLUS_DECODER_OUT_BUF_SIZE + DSP_SIZE_FOR_CLK_SKEW,         stream_codec_decoder_lc3plus_initialize,            stream_codec_decoder_lc3plus_process},/*0x0F CODEC_DECODER_LC3PLUS,       */
#else
    {CODEC_DECODER_LC3PLUS,                                                0,                       0xFF,                                            NULL,                                            NULL},/*0x0F CODEC_DECODER_LC3PLUS,       */
#endif
#ifdef AIR_BT_HFP_CVSD_ENABLE
    {CODEC_ENCODER_CVSD,                            DSP_CVSD_ENCODER_MEMSIZE,                        240,            stream_codec_encoder_cvsd_initialize,               stream_codec_encoder_cvsd_process},/*0x10 CVSD_Encoder,    */   // 60(NB), 120(WB)
#else
    {CODEC_ENCODER_CVSD,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x10 CVSD_Encoder,    */   // 60(NB), 120(WB)
#endif
#ifdef AIR_BT_HFP_MSBC_ENABLE
    {CODEC_ENCODER_MSBC,                            DSP_MSBC_ENCODER_MEMSIZE,                        240,            stream_codec_encoder_msbc_initialize,               stream_codec_encoder_msbc_process},/*0x11 CODEC_ENCODER_MSBC,      */
#else
    {CODEC_ENCODER_MSBC,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x11 CODEC_ENCODER_MSBC,      */
#endif
#ifdef AIR_CELT_ENC_ENABLE
    {CODEC_ENCODER_OPUS,                                DSP_CELT_ENC_MEMSIZE,   DSP_CELT_ENC_OUTPUT_SIZE,            stream_codec_encoder_celt_initialize,               stream_codec_encoder_celt_process},/*0x12 CODEC_ENCODER_OPUS,       */
#else
    {CODEC_ENCODER_OPUS,                                                   0,                       0xFF,                                            NULL,                                            NULL},/*0x12 CODEC_ENCODER_OPUS,       */
#endif /* AIR_CELT_ENC_ENABLE */
#ifdef AIR_BT_LE_LC3_ENABLE
    {CODEC_ENCODER_LC3,                                                    0,  DSP_LC3_ENCODER_OUT_BUF_SIZE + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_encoder_lc3_initialize,                stream_codec_encoder_lc3_process}, /*0x13 CODEC_ENCODER_LC3,       */
    {CODEC_ENCODER_LC3_BRANCH,                       DSP_LC3_ENCODER_MEMSIZE,  DSP_LC3_ENCODER_OUT_BUF_SIZE + DSP_SIZE_FOR_CLK_SKEW,             stream_codec_encoder_lc3_initialize,         stream_codec_encoder_lc3_process_branch}, /*0x14 CODEC_ENCODER_LC3,       */
#elif defined(AIR_BT_CODEC_BLE_V2_ENABLED)
    {CODEC_ENCODER_LC3,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x13 CODEC_ENCODER_LC3,       */
    {CODEC_ENCODER_LC3_BRANCH,                                             0,                       0xFF,      stream_codec_encoder_lc3_branch_initialize,         stream_codec_encoder_lc3_branch_process},/*0x14 CODEC_ENCODER_LC3,       */
#else
    {CODEC_ENCODER_LC3,                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x13 CODEC_ENCODER_LC3,       */
    {CODEC_ENCODER_LC3_BRANCH,                                             0,                       0xFF,                                            NULL,                                            NULL},/*0x14 CODEC_ENCODER_LC3,       */
#endif
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    {CODEC_ENCODER_LC3PLUS,                                                0,                       0xFF,         stream_codec_encoder_lc3plus_initialize,            stream_codec_encoder_lc3plus_process},/*0x15 CODEC_ENCODER_LC3PLUS,       */
#else
    {CODEC_ENCODER_LC3PLUS,                                                0,                       0xFF,                                            NULL,                                            NULL},/*0x15 CODEC_ENCODER_LC3PLUS,       */
#endif
#if defined(AIR_CELT_ENC_V2_ENABLE)
    {CODEC_ENCODER_OPUS_V2,                                                0,                       0xFF,         stream_codec_encoder_celt_v2_initialize,            stream_codec_encoder_celt_v2_process},/*0x16 CODEC_ENCODER_OPUS_V2,       */
#else
    {CODEC_ENCODER_OPUS_V2,                                                0,                       0xFF,                                            NULL,                                            NULL},/*0x16 CODEC_ENCODER_OPUS_V2,       */
#endif /* AIR_CELT_ENC_V2_ENABLE */
#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
    {CODEC_ENCODER_ULD,                                                    0,                          0,             stream_codec_encoder_uld_initialize,                stream_codec_encoder_uld_process}, /*0x17 CODEC_ENCODER_ULD,       */
#else
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL}, /*0x17 CODEC_ENCODER_ULD,       */
#endif
#ifdef AIR_BT_A2DP_SBC_ENCODER_ENABLE
    {CODEC_ENCODER_SBC,                                 DSP_MEM_SBC_ECN_SIZE,/* 1652*/                 0,             stream_codec_encoder_sbc_initialize,                stream_codec_encoder_sbc_process},/*0x18 CODEC_ENCODER_SBC,       */
#else
    {CODEC_ENCODER_SBC,                                                    0,                          0,                                            NULL,                                            NULL},/*0x18 CODEC_ENCODER_SBC,       */
#endif
    {CODEC_ENCODER_VENDOR,             STREAM_CODEC_ENCODER_VERNDOR_MEM_SIZE,                          0,          STREAM_CODEC_ENCODER_VENDOR_INITIALIZE,             STREAM_CODEC_ENCODER_VENDOR_PROCESS},/*0x19 CODEC_ENCODER_VENDOR,       */
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x1A _Reserved,       */
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x1B _Reserved,       */
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x1C _Reserved,       */
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x1D _Reserved,       */
    {0,                                                                    0,                       0xFF,                                            NULL,                                            NULL},/*0x1E _Reserved,       */
    /*====================================================================================    stream feature function    ====================================================================================*/
    /*feature_type,                                              memory_size,      0(Must equal to zero),                               *initialize_entry,                                  *process_entry*/
#ifdef MTK_HWSRC_IN_STREAM
    {DSP_SRC,                                                DSP_SRC_MEMSIZE,                          0,                  stream_function_src_initialize,                     stream_function_src_process},/*0x1F  DSP_SRC,       */
#else
    {DSP_SRC,                                                              0,                          0,                                            NULL,                                            NULL},/*0x1F  DSP_SRC,       */
#endif
    {FUNC_END,                                                             0,                          0,                  stream_function_end_initialize,                     stream_function_end_process},/*0x20 FUNC_END,                */
#ifdef AIR_VOICE_DRC_ENABLE
    {FUNC_RX_WB_DRC,                                      DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_rx_wb_initialize,            stream_function_drc_voice_rx_process},/*0x21 FUNC_RX_WB_DRC,          */
    {FUNC_RX_NB_DRC,                                      DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_rx_nb_initialize,            stream_function_drc_voice_rx_process},/*0x22 FUNC_RX_NB_DRC,          */
    {FUNC_TX_WB_DRC,                                      DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_tx_wb_initialize,            stream_function_drc_voice_tx_process},/*0x23 FUNC_TX_WB_DRC,       */
    {FUNC_TX_NB_DRC,                                      DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_tx_nb_initialize,            stream_function_drc_voice_tx_process},/*0x24 FUNC_TX_NB_DRC,       */
#else
    {FUNC_RX_WB_DRC,                                                       0,                          0,                                            NULL,                                             NULL},/*0x21 FUNC_RX_WB_DRC,    */
    {FUNC_RX_NB_DRC,                                                       0,                          0,                                            NULL,                                             NULL},/*0x22 FUNC_RX_NB_DRC,       */
    {FUNC_TX_WB_DRC,                                                       0,                          0,                                            NULL,                                             NULL},/*0x23 FUNC_TX_WB_DRC,      */
    {FUNC_TX_NB_DRC,                                                       0,                          0,                                            NULL,                                             NULL},/*0x24 FUNC_TX_NB_DRC,       */
#endif
#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    {FUNC_RX_NR,                                            VOICE_NR_MEMSIZE,                          0,                   stream_function_nr_initialize,                      stream_function_nr_process},/*0x25 FUNC_RX_NR,                */
#else
    {FUNC_RX_NR,                                                           0,                          0,                   stream_function_nr_initialize,                      stream_function_nr_process},/*0x25 FUNC_RX_NR,                */
#endif
    {FUNC_TX_NR,                                                           0,                          0,               stream_function_aec_nr_initialize,                     stream_function_aec_process},/*0x26 FUNC_TX_NR,                */
#else
    {FUNC_RX_NR,                                                           0,                          0,                                            NULL,                                             NULL},/*0x25 FUNC_RX_NR,                */
    {FUNC_TX_NR,                                                           0,                          0,                                            NULL,                                             NULL},/*0x26 FUNC_TX_NR,                */
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
#if defined(AIR_UL_FIX_SAMPLING_RATE_48K)
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(5, 1440, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#else
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(4, 1440, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#endif
#elif defined(AIR_UL_FIX_SAMPLING_RATE_32K)
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(5, 960, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#else
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(4, 960, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#endif
#else
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(5, 480, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#else
    {FUNC_CLK_SKEW_HFP_UL,                     DSP_CLK_SKEW_MEMSIZE(4, 480, 0),                          0,    stream_function_clock_skew_hfp_ul_initialize,               stream_function_clock_skew_process}, /*0x27 FUNC_CLK_SKEW_HFP_UL,        */
#endif
#endif
#else
    {FUNC_CLK_SKEW_HFP_UL,                                                     0,                          0,                                            NULL,                                                NULL}, /*0x27 FUNC_CLK_SKEW_UL,        */
#endif//AIR_BT_CLK_SKEW_ENABLE
#ifdef AIR_BT_CLK_SKEW_ENABLE
    {FUNC_CLK_SKEW_A2DP_DL,                   DSP_CLK_SKEW_MEMSIZE(2, 8192, 1),                          0,      stream_function_clock_skew_a2dp_initialize,               stream_function_clock_skew_process}, /*0x28 FUNC_CLK_SKEW_A2DP_DL,        */
#else
    {FUNC_CLK_SKEW_A2DP_DL,                                                0,                          0,                                            NULL,                                                NULL}, /*0x28 FUNC_CLK_SKEW_A2DP_DL,        */
#endif//AIR_BT_CLK_SKEW_ENABLE
    {FUNC_MIC_SW_GAIN,                                                     8,                          0,                 stream_function_gain_initialize,                    stream_function_gain_process},/*0x29 FUNC_MIC_SW_GAIN,        */
#ifdef AIR_VOICE_PLC_ENABLE
    {FUNC_PLC,                                         DSP_VOICE_PLC_MEMSIZE,                          0,                  stream_function_plc_initialize,                     stream_function_plc_process},/*0x2A FUNC_PLC,                */
#else
    {FUNC_PLC,                                                             0,                          0,                                            NULL,                                            NULL},/*0x2A FUNC_PLC,   */
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
    {FUNC_CLK_SKEW_HFP_DL,                     DSP_CLK_SKEW_MEMSIZE(2, 480, 0),                          0,    stream_function_clock_skew_hfp_dl_initialize,              stream_function_clock_skew_process}, /*0x2B FUNC_CLK_SKEW_HFP_DL,   */
#else
    {FUNC_CLK_SKEW_HFP_DL,                                                 0,                          0,                                            NULL,                                                NULL}, /*0x2B FUNC_CLK_SKEW_DL,        */
#endif//AIR_BT_CLK_SKEW_ENABLE
    {FUNC_PROC_SIZE_CONV,                                                  0,                          0,       stream_function_size_converter_initialize,          stream_function_size_converter_process},/*0x2C FUNC_PROC_SIZE_CONV     */ /* Convert Processing size to give fixed given size*/
    {FUNC_JOINT,                                                           0,                          0,                                            NULL,                                            NULL},/*0x2D FUNC_JOINT,       */
    {FUNC_BRANCH,                                                          0,                          0,                                            NULL,                                            NULL},/*0x2E FUNC_BRANCH,       */
    {FUNC_MUTE,                                                            0,                          0,                                            NULL,                                            NULL},/*0x2F FUNC_MUTE,       */
#ifdef AIR_DRC_ENABLE
    {FUNC_DRC,                                            DRC_AU_MEMSIZE,                          0,            stream_function_drc_audio_initialize,               stream_function_drc_audio_process},/*0x30 FUNC_DRC,       */
#else
    {FUNC_DRC,                                                             0,                          0,                                            NULL,                                            NULL},/*0x30 FUNC_DRC,       */
#endif
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    {FUNC_PEQ,                                               DSP_PEQ_MEMSIZE,                          0,                  stream_function_peq_initialize,                     stream_function_peq_process},/*0x31 FUNC_PEQ,       */
#else
    {FUNC_PEQ,                                                             0,                          0,                                            NULL,                                            NULL},/*0x31 FUNC_PEQ,       */
#endif
    {FUNC_LPF,                                                             0,                          0,                                            NULL,                                            NULL},/*0x32 FUNC_LPF,       */
    {FUNC_CH_SEL,                                                          0,                          0, stream_function_channel_selector_initialize_a2dp,   stream_function_channel_selector_process_a2dp}, /*0x33 FUNC_CH_SEL,       */
#ifdef AIR_MUTE_SMOOTHER_ENABLE
    {FUNC_MUTE_SMOOTHER,                                                   0,                          0,          stream_function_mute_smooth_initialize,             stream_function_mute_smooth_process},/*0x34 FUNC_MUTE_SMOOTHER,       */
#else
    {FUNC_MUTE_SMOOTHER,                                                   0,                          0,                                            NULL,                                            NULL},/*0x34 FUNC_MUTE_SMOOTHER,       */
#endif
#ifdef MTK_PEQ_ENABLE
    {FUNC_PEQ2,                                              DSP_PEQ_MEMSIZE,                          0,                 stream_function_peq2_initialize,                    stream_function_peq2_process},/*0x35 FUNC_PEQ2,     */
#else
    {FUNC_PEQ2,                                                            0,                          0,                                            NULL,                                            NULL},/*0x35 FUNC_PEQ2,     */
#endif
#ifdef AIR_DRC_ENABLE
    {FUNC_DRC2,                                           DRC_AU_MEMSIZE,                          0,           stream_function_drc_audio2_initialize,              stream_function_drc_audio2_process},/*0x36 FUNC_DRC2,       */
#else
    {FUNC_DRC2,                                                            0,                          0,                                            NULL,                                            NULL},/*0x36 FUNC_DRC2,       */
#endif
    {FUNC_CH_SEL_HFP,                                                      0,                          0, stream_function_channel_selector_initialize_hfp,    stream_function_channel_selector_process_hfp},/*0x33 FUNC_CH_SEL,       */
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    {FUNC_LEAKAGE_COMPENSATION,              DSP_LEAKAGE_COMPENSATION_MEMSIZE,                          0, stream_function_leakage_compensation_initialize,    stream_function_leakage_compensation_process},/*0x3A  FUNC_LEAKAGE_COMPENSATION,       */
#else
    {FUNC_LEAKAGE_COMPENSATION,                                             0,                          0,                                            NULL,                                            NULL},/*0x3A  FUNC_LEAKAGE_COMPENSATION,       */
#endif
    {0,                                                                    0,                          0,                                            NULL,                                            NULL},/*0x39 _Reserved,       */
#ifdef MTK_WWE_ENABLE
    {FUNC_WWE_PREPROC,                                   WWE_PREPROC_MEMSIZE,                          0,    stream_function_wwe_preprocessing_initialize,       stream_function_wwe_preprocessing_process},/*0x3A FUNC_WWE_PREPROC,     */
    {FUNC_WWE_PROC,                                         WWE_PROC_MEMSIZE,                          0,       stream_function_wwe_processing_initialize,          stream_function_wwe_processing_process},/*0x3B FUNC_WWE_PROC,     */
#else
    {FUNC_WWE_PREPROC,                                                     0,                          0,                                            NULL,                                            NULL},/*0x3A FUNC_WWE_PREPROC,     */
    {FUNC_WWE_PROC,                                                        0,                          0,                                            NULL,                                            NULL},/*0x3B FUNC_WWE_PROC,     */
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    {FUNC_RX_WB_AGC,                                         DSP_AGC_MEMSIZE,                          0,               stream_function_rx_agc_initialize,                  stream_function_rx_agc_process},/*0x3C FUNC_RX_WB_AGC,       */
    {FUNC_RX_NB_AGC,                                         DSP_AGC_MEMSIZE,                          0,               stream_function_rx_agc_initialize,                  stream_function_rx_agc_process},/*0x3D FUNC_RX_WB_AGC,       */
    {FUNC_TX_AGC,                                            DSP_AGC_MEMSIZE,                          0,               stream_function_tx_agc_initialize,                  stream_function_tx_agc_process},/*0x3E FUNC_RX_WB_AGC,       */
#else
    {FUNC_RX_WB_AGC,                                                       0,                          0,                                            NULL,                                            NULL},/*0x3C FUNC_RX_WB_AGC,       */
    {FUNC_RX_NB_AGC,                                                       0,                          0,                                            NULL,                                            NULL},/*0x3D FUNC_RX_WB_AGC,       */
    {FUNC_TX_AGC,                                                          0,                          0,                                            NULL,                                            NULL},/*0x3E FUNC_RX_WB_AGC,       */
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
    {FUNC_GSENSOR_MOTION_DETECT,                                         512,                        512,               gsensor_motion_detect_interface_init,               gsensor_motion_detect_interface_process},/*0x3F FUNC_GSENSOR_MOTION_DETECT*/
#else
    {FUNC_GSENSOR_MOTION_DETECT,                                           0,                          0,                                            NULL,                                                     NULL},/*0x3F FUNC_GSENSOR_MOTION_DETECT*/
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
    {FUNC_AUDIO_PLC,                                       AUDIO_PLC_MEMSIZE,                         0,                                   Audio_PLC_Init,                               Audio_PLC_Process},/*0x40 FUNC_AUDIO_PLC,       */
#else
    {FUNC_AUDIO_PLC,                                                       0,                      0xFF,                                             NULL,                                            NULL},/*0x40 FUNC_AUDIO_PLC,       */
#endif
#ifdef MTK_LINEIN_INS_ENABLE
    {FUNC_INS,                                               DSP_INS_MEMSIZE,                          0,            stream_function_ins_audio_initialize,               stream_function_ins_audio_process},/*0x41 FUNC_INS,       */
#else
    {FUNC_INS,                                                             0,                          0,                                            NULL,                                            NULL},/*0x41 FUNC_INS,       */
#endif
#ifdef MTK_PEQ_ENABLE
    {FUNC_PEQ_INSTANT,                                              DSP_PEQ_MEMSIZE,                   0,                  stream_function_peq_initialize,             stream_function_instant_peq_process},/*0x42 FUNC_PEQ_INSTANT,     */
#else
    {FUNC_PEQ_INSTANT,                                                     0,                          0,                                            NULL,                                            NULL},/*0x42 FUNC_PEQ_INSTANT,     */
#endif
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    {FUNC_FIXED_SW_GAIN,                                                   0,                          0,           stream_function_fixed_gain_initialize,               stream_function_fixed_gain_process},/*0x43 FUNC_FIXED_SW_GAIN,    */
    {FUNC_FUNCTION_A,                                                      0,                          0,                                            NULL,                                             NULL},/*0x44 FUNC_FUNCTION_A,       */
    {FUNC_FUNCTION_B,                                                      0,                          0,                                            NULL,                                             NULL},/*0x45 FUNC_FUNCTION_B,       */
    {FUNC_FUNCTION_C,                                                      0,                          0,                                            NULL,                                             NULL},/*0x46 FUNC_FUNCTION_C,       */
    {FUNC_FUNCTION_D,                                                      0,                          0,                                            NULL,                                             NULL},/*0x47 FUNC_FUNCTION_D,       */
    {FUNC_FUNCTION_E,                                                      0,                          0,                                            NULL,                                             NULL},/*0x48 FUNC_FUNCTION_E,       */
    {FUNC_FUNCTION_F,                                                      0,                          0,                                            NULL,                                             NULL},/*0x49 FUNC_FUNCTION_F,       */
    {FUNC_FUNCTION_G,                                                      0,                          0,                                            NULL,                                             NULL},/*0x4A FUNC_FUNCTION_G,       */
    {FUNC_FUNCTION_H,                                                      0,                          0,                                            NULL,                                             NULL},/*0x4B FUNC_FUNCTION_H,       */
#else
    {FUNC_FIXED_SW_GAIN,                                                   0,                          0,                                            NULL,                                             NULL},/*0x43 FUNC_FIXED_SW_GAIN,    */
    {FUNC_FUNCTION_A,                                                      0,                          0,                                            NULL,                                             NULL},/*0x44 FUNC_FUNCTION_A,       */
    {FUNC_FUNCTION_B,                                                      0,                          0,                                            NULL,                                             NULL},/*0x45 FUNC_FUNCTION_B,       */
    {FUNC_FUNCTION_C,                                                      0,                          0,                                            NULL,                                             NULL},/*0x46 FUNC_FUNCTION_C,       */
    {FUNC_FUNCTION_D,                                                      0,                          0,                                            NULL,                                             NULL},/*0x47 FUNC_FUNCTION_D,       */
    {FUNC_FUNCTION_E,                                                      0,                          0,                                            NULL,                                             NULL},/*0x48 FUNC_FUNCTION_E,       */
    {FUNC_FUNCTION_F,                                                      0,                          0,                                            NULL,                                             NULL},/*0x49 FUNC_FUNCTION_F,       */
    {FUNC_FUNCTION_G,                                                      0,                          0,                                            NULL,                                             NULL},/*0x4A FUNC_FUNCTION_G,       */
    {FUNC_FUNCTION_H,                                                      0,                          0,                                            NULL,                                             NULL},/*0x4B FUNC_FUNCTION_H,       */
#endif
#ifdef AIR_VOICE_NR_ENABLE
#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)
    {FUNC_TX_EQ,                                                           0,                          0,                stream_function_tx_eq_initialize,                   stream_function_tx_eq_process},/*0x4C FUNC_TX_EQ,       */
#else
    {FUNC_TX_EQ,                                                           0,                          0,                                            NULL,                                            NULL},/*0x4C FUNC_TX_EQ,       */
#endif
#else
    {FUNC_TX_EQ,                                                           0,                          0,                                            NULL,                                            NULL},/*0x4C FUNC_TX_EQ,       */
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
    {FUNC_USER_TRIGGER_FF_SZ,                          USER_TRIGGER_SZ_MEMSIZE,                          0,       stream_function_user_trigger_ff_sz_initialize,        stream_function_user_trigger_ff_sz_process},/*0x4D  FUNC_USER_TRIGGER_FF_SZ,       */
    {FUNC_USER_TRIGGER_FF_PZ,                          USER_TRIGGER_PZ_MEMSIZE,                          0,       stream_function_user_trigger_ff_pz_initialize,        stream_function_user_trigger_ff_pz_process},/*0x4E  FUNC_USER_TRIGGER_FF_PZ,       */
    {FUNC_USER_TRIGGER_FF_PZ_FIR,                  USER_TRIGGER_PZ_FIR_MEMSIZE,                          0,       stream_function_user_trigger_ff_pz_fir_initialize,        stream_function_user_trigger_ff_pz_fir_process},/*0x4E  FUNC_USER_TRIGGER_FF_WZ,       */
#else
    {FUNC_USER_TRIGGER_FF_SZ,                                                0,                          0,                                             NULL,                                           NULL},/*0x4D  FUNC_USER_TRIGGER_FF_SZ,       */
    {FUNC_USER_TRIGGER_FF_PZ,                                                0,                          0,                                             NULL,                                           NULL},/*0x4E  FUNC_USER_TRIGGER_FF_PZ,       */
    {FUNC_USER_TRIGGER_FF_PZ_FIR,                                            0,                          0,                                             NULL,                                           NULL},/*0x4F  FUNC_USER_TRIGGER_FF_PZ,       */
#endif
#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
    {FUNC_AUDIO_LOOPBACK_TEST,               DSP_AUDIO_LOOPBACK_TEST_MEMSIZE,                          0,                        Audio_Loopback_Test_Init,                     Audio_Loopback_Test_Process},/*0x50 FUNC_AUDIO_LOOPBACK_TEST,       */
#else
    {FUNC_AUDIO_LOOPBACK_TEST,                                             0,                          0,                                            NULL,                                            NULL},/*0x50 FUNC_AUDIO_LOOPBACK_TEST,       */
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    {FUNC_DRC3,                                         DRC_AU_MEMSIZE,                          0,           stream_function_drc_audio3_initialize,               stream_function_drc_audio3_process},/*0x51 FUNC_DRC3,       */
#else
    {FUNC_DRC3,                                                          0,                          0,                                            NULL,                                             NULL},/*0x51 FUNC_DRC3,       */
#endif
#ifdef AIR_SOFTWARE_SRC_ENABLE
    {FUNC_SW_SRC,                                                          0,                         0,                stream_function_sw_src_initialize,                  stream_function_sw_src_process},/*0x52 FUNC_SW_SRC,       */
#else
    {FUNC_SW_SRC,                                                          0,                         0,                                             NULL,                                            NULL},/*0x52 FUNC_SW_SRC,       */
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    {FUNC_SW_CLK_SKEW,                                                     0,                         0,           stream_function_sw_clk_skew_initialize,             stream_function_sw_clk_skew_process},/*0x53 FUNC_SW_CLK_SKEW,       */
#else
    {FUNC_SW_CLK_SKEW,                                                     0,                         0,                                             NULL,                                            NULL},/*0x53 FUNC_SW_CLK_SKEW,       */
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    {FUNC_SW_GAIN,                                                         0,                         0,               stream_function_sw_gain_initialize,                 stream_function_sw_gain_process},/*0x54 FUNC_SW_GAIN,       */
#else
    {FUNC_SW_GAIN,                                                         0,                         0,                                             NULL,                                            NULL},/*0x54 FUNC_SW_GAIN,       */
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    {FUNC_SW_MIXER,                                                        0,                         0,              stream_function_sw_mixer_initialize,                stream_function_sw_mixer_process},/*0x55 FUNC_SW_MIXER,       */
#else
    {FUNC_SW_MIXER,                                                        0,                         0,                                             NULL,                                            NULL},/*0x55 FUNC_SW_MIXER,       */
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    {FUNC_SW_BUFFER,                                                       0,                         0,             stream_function_sw_buffer_initialize,               stream_function_sw_buffer_process},/*0x56 FUNC_SW_BUFFER,       */
#else
    {FUNC_SW_BUFFER,                                                       0,                         0,                                             NULL,                                            NULL},/*0x56 FUNC_SW_BUFFER,       */
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
#ifdef AIR_BT_CODEC_BLE_ENABLED
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    {FUNC_CLK_SKEW_BLE_MUSIC_DL,               DSP_CLK_SKEW_MEMSIZE(4, 1920, 0),                         0,   stream_function_clock_skew_le_music_initialize,              stream_function_clock_skew_process}, /*0x57 FUNC_CLK_SKEW_BLE_MUSIC_DL,   */
#else
    {FUNC_CLK_SKEW_BLE_MUSIC_DL,               DSP_CLK_SKEW_MEMSIZE(2, 1920, 0),                         0,   stream_function_clock_skew_le_music_initialize,              stream_function_clock_skew_process}, /*0x57 FUNC_CLK_SKEW_BLE_MUSIC_DL,   */
#endif
#else
    {FUNC_CLK_SKEW_BLE_MUSIC_DL,                                           0,                         0,                                             NULL,                                            NULL},/*0x57 FUNC_CLK_SKEW_BLE_MUSIC_DL,   */
#endif
#else
    {FUNC_CLK_SKEW_BLE_MUSIC_DL,                                           0,                         0,                                             NULL,                                            NULL},/*0x57 FUNC_CLK_SKEW_BLE_MUSIC_DL,   */
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    {FUNC_WIND_DETECT,                                WIND_DETECTION_MEMSIZE,                         0,                  stream_function_wind_initialize,                    stream_function_wind_process},/*0x58 FUNC_WIND_DETECT,   */
#else
    {FUNC_WIND_DETECT,                                                     0,                         0,                                             NULL,                                            NULL},/*0x58 FUNC_WIND_DETECT,   */
#endif
    {FUNC_CH_SEL_VP,                                                       0,                         0,  stream_function_channel_selector_initialize_vp,    stream_function_channel_selector_process_vp},/*0x59 FUNC_CH_SEL_VP,       */
#ifdef MTK_LINEIN_PEQ_ENABLE
    {FUNC_PEQ3,                                              DSP_PEQ_MEMSIZE,                          0,                 stream_function_peq3_initialize,                    stream_function_peq3_process},/*0x5A FUNC_PEQ3,     */
#else
    {FUNC_PEQ3,                                                            0,                          0,                                            NULL,                                            NULL},/*0x5A FUNC_PEQ3,     */
#endif
#ifdef AIR_AFC_ENABLE
    {FUNC_AFC,                                                             0,                         0,                   stream_function_afc_initialize,                     stream_function_afc_process},/*0x5B FUNC_AFC,           */
#else
    {FUNC_AFC,                                                             0,                         0,                                             NULL,                                            NULL},/*0x5B FUNC_AFC,           */
#endif
#ifdef AIR_LD_NR_ENABLE
    {FUNC_LD_NR,                                                           0,                         0,                 stream_function_ld_nr_initialize,                   stream_function_ld_nr_process},/*0x5C FUNC_LD_NR,         */
#else
    {FUNC_LD_NR,                                                           0,                         0,                                             NULL,                                            NULL},/*0x5C FUNC_LD_NR,         */
#endif
#ifdef AIR_AT_AGC_ENABLE
    {FUNC_AT_AGC,                                                          0,                         0,                stream_function_at_agc_initialize,                  stream_function_at_agc_process},/*0x5D FUNC_AT_AGC,        */
#else
    {FUNC_AT_AGC,                                                          0,                         0,                                             NULL,                                            NULL},/*0x5D FUNC_AT_AGC,        */
#endif
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE
    {FUNC_PEQ4,                                          (DSP_PEQ_MEMSIZE)*2,                          0,                 stream_function_peq4_initialize,                    stream_function_peq4_process},/*0x5E FUNC_PEQ3,     */
#else
    {FUNC_PEQ4,                                                            0,                          0,                                            NULL,                                            NULL},/*0x5E FUNC_PEQ3,     */
#endif
#ifdef AIR_VOICE_NR_ENABLE
#if defined(AIR_HFP_DNN_PATH_ENABLE) && defined(AIR_DNN_LIB_ENABLE)
    {FUNC_DNN_NR,                                            DNN_NR_MEMSIZE,                          0,               stream_function_dnn_nr_initialize,                   stream_function_dnn_nr_process},/*0x5F FUNC_DNN_NR,    */
#else
    {FUNC_DNN_NR,                                                         0,                          0,                                            NULL,                                             NULL},/*0x5F FUNC_DNN_NR,    */
#endif
#if defined(AIR_ECNR_EC_ENABLE) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE))
    {FUNC_EC120,                                             IGO_NR_MEMSIZE,                          0,               stream_function_ec_120_initialize,                   stream_function_ec120_process},/*0x60 FUNC_EC120,    */
#elif defined(AIR_ECNR_EC_ENABLE)
    {FUNC_EC120,                                                          0,                          0,               stream_function_ec_120_initialize,                   stream_function_ec120_process},/*0x60 FUNC_EC120,    */
#else
    {FUNC_EC120,                                                          0,                          0,                                            NULL,                                             NULL},/*0x60 FUNC_EC120,    */
#endif
#else
    {FUNC_DNN_NR,                                                         0,                          0,                                            NULL,                                             NULL},/*0x5F FUNC_DNN_NR,    */
    {FUNC_EC120,                                                          0,                          0,                                            NULL,                                             NULL},/*0x60 FUNC_EC120,     */
#endif
#ifdef AIR_FIXED_RATIO_SRC
    {FUNC_SRC_FIXED_RATIO,                                                0,                          0,      stream_function_src_fixed_ratio_initialize,          stream_function_src_fixed_ratio_process},/*0x61 FUNC_SRC_FIXED_RATIO,     */
#else
    {FUNC_SRC_FIXED_RATIO,                                                0,                          0,                                            NULL,                                             NULL},/*0x61 FUNC_SRC_FIXED_RATIO,     */
#endif
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    {FUNC_FIXED_SW_GAIN_TDM,                                               0,                          0,       stream_function_fixed_gain_tdm_initialize,           stream_function_fixed_gain_tdm_process},/*0x62 FUNC_FIXED_SW_GAIN_TDM,    */
#else
    {FUNC_FIXED_SW_GAIN_TDM,                                               0,                          0,                                            NULL,                                             NULL},/*0x62 FUNC_FIXED_SW_GAIN_TDM,    */
#endif
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    {FUNC_FULL_ADAPT_ANC,                                                 0,                          0,      stream_function_full_adapt_anc_initialize,          stream_function_full_adapt_anc_process},/*0x63 FUNC_FULL_ADAPT_ANC,     */
#else
    {FUNC_FULL_ADAPT_ANC,                                                 0,                          0,                                            NULL,                                             NULL},/*0x63 FUNC_FULL_ADAPT_ANC,     */
#endif
#if defined(AIR_ADAPTIVE_EQ_ENABLE) && defined(MTK_PEQ_ENABLE)
    {FUNC_ADAPTIVE_EQ,                                       DSP_PEQ_MEMSIZE,                          0,          stream_function_adaptive_eq_initialize,              stream_function_adaptive_eq_process},/*0x64 FUNC_ADAPTIVE_EQ,       */
#else
    {FUNC_ADAPTIVE_EQ,                                                     0,                          0,                                            NULL,                                            NULL},/*0x64 FUNC_ADAPTIVE_EQ,       */
#endif
#if defined(AIR_ADAPTIVE_EQ_ENABLE) && defined(MTK_PEQ_ENABLE)
    {FUNC_ADAPTIVE_EQ_DRC,                                                 0,                          0,      stream_function_adaptive_eq_drc_initialize,         stream_function_adaptive_eq_drc_process},/*0x65 FUNC_ADAPTIVE_EQ_DRC,       */
#else
    {FUNC_ADAPTIVE_EQ_DRC,                                                 0,                          0,                                            NULL,                                            NULL},/*0x65 FUNC_ADAPTIVE_EQ_DRC,       */
#endif
#ifdef AIR_VOICE_DRC_ENABLE
    {FUNC_RX_SWB_DRC,                                         DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_rx_swb_initialize,           stream_function_drc_voice_rx_process},/*0x66 FUNC_RX_WB_DRC,       */
    {FUNC_TX_SWB_DRC,                                         DRC_VO_MEMSIZE,                          0,      stream_function_drc_voice_tx_swb_initialize,           stream_function_drc_voice_tx_process},/*0x67 FUNC_TX_WB_DRC,       */
#else
    {FUNC_RX_SWB_DRC,                                                      0,                          0,                                             NULL,                                           NULL},/*0x66 FUNC_RX_SWB_DRC,      */
    {FUNC_TX_SWB_DRC,                                                      0,                          0,                                             NULL,                                           NULL},/*0x67 FUNC_TX_SWB_DRC,      */
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    {FUNC_RX_SWB_AGC,                                         DSP_AGC_MEMSIZE,                         0,                stream_function_rx_agc_initialize,                  stream_function_rx_agc_process},/*0x68 FUNC_RX_SWB_AGC,       */
#else
    {FUNC_RX_SWB_AGC,                                                      0,                          0,                                             NULL,                                            NULL},/*0x68 FUNC_RX_SWB_AGC,       */
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    {FUNC_USER_UNAWARE,                                                     0,                         0,                  stream_function_user_unaware_initialize,                    stream_function_user_unaware_process},/*0x69 FUNC_USER_UNAWARE,   */
#else
    {FUNC_USER_UNAWARE,                                                     0,                         0,                                             NULL,                                            NULL},/*0x69 FUNC_USER_UNAWARE,   */
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    {FUNC_ENVIRONMENT_DETECTION,               ENVIRONMENT_DETECTION_MEMSIZE,                         0,         stream_function_environment_detection_initialize,            stream_function_environment_detection_process},/*0x6A FUNC_ENVIRONMENT_DETECTION,   */
#else
    {FUNC_ENVIRONMENT_DETECTION,                                           0,                         0,                                             NULL,                                            NULL},/*0x6A FUNC_ENVIRONMENT_DETECTION,   */
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    {FUNC_ADAPTIVE_EQ_DETECT,                                 DSP_AEQ_MEMSIZE,                          0,      stream_function_adaptive_eq_detect_initialize,      stream_function_adaptive_eq_detect_process},/*0x6B FUNC_ADAPTIVE_EQ_DRC,       */
#else
    {FUNC_ADAPTIVE_EQ_DETECT,                                              0,                          0,                                            NULL,                                            NULL},/*0x6B FUNC_ADAPTIVE_EQ_DRC,       */
#endif
#ifdef AIR_ECNR_PREV_PART_ENABLE
    {FUNC_ECNR_PREV_PROCESS,                                                 0,                       0,            stream_function_ecnr_prev_initialize,                  stream_function_ecnr_prev_process},/*0x64 FUNC_ECNR_PREV_PROCESS,   */
#else
    {FUNC_ECNR_PREV_PROCESS,                                                 0,                       0,                                            NULL,                                               NULL},/*0x64 FUNC_ECNR_PREV_PROCESS,   */
#endif
#ifdef AIR_ECNR_POST_PART_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    {FUNC_ECNR_POST_PROCESS,                                 VOICE_NR_MEMSIZE,                        0,            stream_function_ecnr_post_initialize,                  stream_function_ecnr_post_process},/*0x65 FUNC_ECNR_POST_PROCESS,   */
#else
    {FUNC_ECNR_POST_PROCESS,                                                0,                        0,            stream_function_ecnr_post_initialize,                  stream_function_ecnr_post_process},/*0x65 FUNC_ECNR_POST_PROCESS,   */
#endif
#else
    {FUNC_ECNR_POST_PROCESS,                                                 0,                       0,                                            NULL,                                               NULL},/*0x65 FUNC_ECNR_POST_PROCESS,   */
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
#ifdef AIR_BT_CODEC_BLE_ENABLED
    {FUNC_CLK_SKEW_BLE_CALL_DL,               DSP_CLK_SKEW_MEMSIZE(2, 960, 0),                          0,   stream_function_clock_skew_le_call_dl_initialize,              stream_function_clock_skew_process}, /*0x6E FUNC_CLK_SKEW_BLE_CALL_DL,   */
#else
    {FUNC_CLK_SKEW_BLE_CALL_DL,                                           0,                          0,                                             NULL,                                            NULL},/*0x6E FUNC_CLK_SKEW_BLE_CALL_DL,   */
#endif
#else
    {FUNC_CLK_SKEW_BLE_CALL_DL,                                           0,                          0,                                             NULL,                                            NULL},/*0x6E FUNC_CLK_SKEW_BLE_CALL_DL,   */
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
#ifdef AIR_BT_CODEC_BLE_ENABLED
    {FUNC_CLK_SKEW_BLE_CALL_UL,               DSP_CLK_SKEW_MEMSIZE(4, 960, 0),                          0,   stream_function_clock_skew_le_call_ul_initialize,              stream_function_clock_skew_process}, /*0x6F FUNC_CLK_SKEW_BLE_CALL_UL,   */
#else
    {FUNC_CLK_SKEW_BLE_CALL_UL,                                           0,                          0,                                             NULL,                                            NULL},/*0x6F FUNC_CLK_SKEW_BLE_CALL_UL,   */
#endif
#else
    {FUNC_CLK_SKEW_BLE_CALL_UL,                                           0,                          0,                                             NULL,                                            NULL},/*0x6F FUNC_CLK_SKEW_BLE_CALL_UL,   */
#endif
    {FUNC_QUEUE_BUFFER,                              DSP_QUE_BUFFER_MEMSIZE,                          0,          stream_function_queue_buffer_initialize,             stream_function_queue_buffer_process},/*0x70 FUNC_QUEUE_BUFFER,   */
#if AIR_VP_PEQ_ENABLE
    {FUNC_VP_PEQ,                                           DSP_PEQ_MEMSIZE,                          0,               stream_function_vp_peq_initialize,                  stream_function_vp_peq_process},
    {FUNC_VP_PEQ2,                                          DSP_PEQ_MEMSIZE,                          0,               stream_function_vp_peq2_initialize,                 stream_function_vp_peq2_process},
    {FUNC_VP_DRC,                                           DRC_AU_MEMSIZE,                           0,               stream_function_drc_vp_initialize,                  stream_function_drc_vp_process},
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    {FUNC_VP_AEQ,                                           DSP_PEQ_MEMSIZE,                          0,               stream_function_vp_aeq_initialize,                  stream_function_vp_aeq_process},
    {FUNC_VP_AEQ_DRC,                                       DRC_AU_MEMSIZE,                           0,               stream_function_drc_vp_aeq_initialize,              stream_function_drc_vp_aeq_process},
#else
    {FUNC_VP_AEQ,                                                         0,                          0,               NULL,                  NULL},
    {FUNC_VP_AEQ_DRC,                                                     0,                          0,               NULL,                  NULL},
#endif
#else
    {FUNC_VP_PEQ,                                                         0,                          0,               NULL,                  NULL},
    {FUNC_VP_PEQ2,                                                        0,                          0,               NULL,                  NULL},
    {FUNC_VP_DRC,                                                         0,                          0,               NULL,                  NULL},
    {FUNC_VP_AEQ,                                                         0,                          0,               NULL,                  NULL},
    {FUNC_VP_AEQ_DRC,                                                     0,                          0,               NULL,                  NULL},
#endif
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    {FUNC_CH_SEL_WIRELESS_MIC,                                                      0,                          0, stream_function_channel_selector_initialize_wireless_mic,    stream_function_channel_selector_process_wireless_mic},/*0x6C FUNC_CH_SEL_WIRELESS_MIC,       */
#else
    {FUNC_CH_SEL_WIRELESS_MIC,                                              0,                          0,                                            NULL,                                            NULL},/*0x6C FUNC_CH_SEL_WIRELESS_MIC,       */
#endif
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    {FUNC_HA,                              DSP_HA_MEMSIZE,                          0,         stream_function_hearing_aid_initialize,      stream_function_hearing_aid_process},/*0x77 FUNC_HA,       */
#else
    {FUNC_HA,                                               0,                          0,                                            NULL,                                            NULL},/*0x77  FUNC_HA,       */
#endif
#if defined(AIR_SURROUND_AUDIO_ENABLE)
    {FUNC_SURROUND_AUDIO,                                                    0,                          0,       stream_function_surround_audio_initialize,          stream_function_surround_audio_process},/*0x78 FUNC_SURROUND_AUDIO,    */
#else
    {FUNC_SURROUND_AUDIO,                                                    0,                          0,                                            NULL,                                             NULL},/*0x78 FUNC_SURROUND_AUDIO,    */
#endif /* AIR_SURROUND_AUDIO_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
    {FUNC_SW_DRC,                                                          0,                         0,          stream_function_sw_compander_initialize,            stream_function_sw_compander_process},/*0x79 FUNC_SW_DRC,       */
#else
    {FUNC_SW_DRC,                                                          0,                         0,                                             NULL,                                            NULL},/*0x79 FUNC_SW_DRCC,      */
#endif /* AIR_SOFTWARE_DRC_ENABLE */
#if defined (AIR_WIRED_AUDIO_ENABLE)&&(AIR_WIRELESS_MIC_TX_ENABLE)&&(MTK_PEQ_ENABLE)
    {FUNC_MIC_PEQ,                                 DSP_PEQ_MEMSIZE,                          0,         stream_function_mic_peq_initialize,      stream_function_mic_peq_process},/*0x7A FUNC_MIC_PEQ,       */
#else
    {FUNC_MIC_PEQ,                                               0,                          0,                                            NULL,                                            NULL},/*0x7A  FUNC_MIC_PEQ,       */
#endif
#if defined (AIR_RECORD_ADVANCED_ENABLE)&&(MTK_PEQ_ENABLE)
    {FUNC_ADVANCED_RECORD_PEQ,                                 DSP_PEQ_MEMSIZE,                          0,         stream_function_advanced_record_peq_initialize,      stream_function_advanced_record_peq_process},/*0x7B FUNC_ADVANCED_RECORD_PEQ,       */
#else
    {FUNC_ADVANCED_RECORD_PEQ,                                               0,                          0,                                            NULL,                                            NULL},/*0x7B  FUNC_ADVANCED_RECORD_PEQ,       */
#endif
#if defined (AIR_DCHS_MODE_ENABLE)
    {FUNC_DCHS_UPLINK_TX,                                                     0,                          0,            stream_function_dchs_uplink_tx_initialize,                    stream_function_dchs_uplink_tx_process},/*0x7C FUNC_DCHS_UPLINK_TX,     */
    {FUNC_DCHS_SW_BUFFER_SLAVE,                                                 0,                          0,           stream_function_dchs_uplink_sw_buffer_slave_initialize,             stream_function_dchs_uplink_sw_buffer_slave_process},/*0x7D FUNC_DCHS_SW_BUFFER_L,     */
    {FUNC_DCHS_SW_BUFFER_MASTER,                                                 0,                          0,           stream_function_dchs_uplink_sw_buffer_master_initialize,             stream_function_dchs_uplink_sw_buffer_master_process},/*0x7E FUNC_DCHS_SW_BUFFER_R,     */
#else
    {FUNC_DCHS_UPLINK_TX,                                                   0,                          0,                                                NULL,                                            NULL},/*0x7C FUNC_DCHS_UPLINK_TX,     */
    {FUNC_DCHS_SW_BUFFER_SLAVE,                                             0,                          0,                                                NULL,                                            NULL},/*0x7D FUNC_DCHS_SW_BUFFER_L,     */
    {FUNC_DCHS_SW_BUFFER_MASTER,                                            0,                          0,                                                NULL,                                            NULL},/*0x7E FUNC_DCHS_SW_BUFFER_R,     */
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    {FUNC_WIRED_USB_PEQ,                                              DSP_PEQ_MEMSIZE,                          0,                 stream_function_wired_usb_peq_initialize,                    stream_function_wired_usb_peq_process},/*0x7F FUNC_WIRED_USB_PEQ,     */
    {FUNC_WIRED_USB_DRC,                                                DRC_AU_MEMSIZE,                          0,           stream_function_wired_usb_drc_initialize,               stream_function_wired_usb_drc_process},/*0x80 FUNC_WIRED_USB_DRC,       */
#else
    {FUNC_WIRED_USB_PEQ,                                                            0,                          0,                                            NULL,                                            NULL},/*0x7F FUNC_WIRED_USB_PEQ,     */
    {FUNC_WIRED_USB_DRC,                                                          0,                          0,                                            NULL,                                             NULL},/*0x80 FUNC_WIRED_USB_DRC,       */
#endif
#if defined (MTK_LINEIN_INS_ENABLE)
    {FUNC_USB_INS,                                              DSP_INS_MEMSIZE,                          0,                 stream_function_usb_ins_audio_initialize,                    stream_function_usb_ins_audio_process},/*0x81 FUNC_USB_INS,     */
#else
    {FUNC_USB_INS,                                                            0,                          0,                                            NULL,                                            NULL},/*0x81 FUNC_USB_INS,     */
#endif
    {FUNC_COPY_TO_VIRTUAL_SOURCE,                                             0,                          0,                 stream_copy_to_virtual_source_initialize,                    stream_copy_to_virtual_sourc_process},/*0x82 FUNC_COPY_TO_VIRTUAL_SOURCE */
#if defined (AIR_CUSTOMIZED_LLF_ENABLE)
    {FUNC_LLF_SAMPLE,                                    LLF_EXAMPLE_MEM_SIZE,                       0,                      stream_function_llf_example_initialize,                         stream_function_llf_example_process},/*0x83 FUNC_LLF_SAMPLE               */
#else
    {FUNC_LLF_SAMPLE,                                                       0,                       0,                      NULL,                         NULL},/*0x83 FUNC_LLF_SAMPLE               */
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
    {FUNC_CH_SEL_USB_MIC,                                                    0,                          0,      stream_function_channel_selector_initialize_usb_mic,    stream_function_channel_selector_process_usb_mic},/*0x84 FUNC_CH_SEL_USB_MIC,       */
#else
    {FUNC_CH_SEL_USB_MIC,                                                    0,                          0,                                                     NULL,                                                NULL},/*0x84 FUNC_CH_SEL_USB_MIC,       */
#endif
#if defined (AIR_HEARTHROUGH_VIVID_PT_ENABLE)
    {FUNC_HEARTHROUGH_VIVID_PT,                                    DSP_VIVID_PASSTHRU_MEMSIZE,                       0,                      stream_function_vivid_passthru_initialize,                         stream_function_vivid_passthru_process},/*0x85 FUNC_HEARTHROUGH_VIVID_PT               */
    {FUNC_HEARTHROUGH_VIVID_PEQ,                                    DSP_PEQ_MEMSIZE,                       0,                      stream_function_vivid_peq_initialize,                         stream_function_vivid_peq_process},/*0x86 FUNC_HEARTHROUGH_VIVID_PEQ               */
#else
    {FUNC_HEARTHROUGH_VIVID_PT,                                                       0,                       0,                      NULL,                         NULL},/*0x85 FUNC_HEARTHROUGH_VIVID_PT               */
    {FUNC_HEARTHROUGH_VIVID_PEQ,                                                       0,                       0,                      NULL,                         NULL},/*0x86 FUNC_HEARTHROUGH_VIVID_PEQ               */
#endif
#if defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    {FUNC_HEARTHROUGH_POST_PROC,                              0,                          0,         stream_function_hearing_aid_postproc_initialize,      stream_function_hearing_aid_postproc_process},/*0x87 FUNC_HEARTHROUGH_POST_PROC,       */
#else
    {FUNC_HEARTHROUGH_POST_PROC,                                               0,                          0,                                            NULL,                                            NULL},/*0x87  FUNC_HEARTHROUGH_POST_PROC,       */
#endif
#if defined (AIR_AUDIO_VOLUME_MONITOR_ENABLE)
    {FUNC_AUDIO_VOLUME_MONITOR,                                               0,                       0,                      stream_function_audio_spectrum_meter_initialize,           stream_function_audio_spectrum_meter_process}, /*0x88 FUNC_AUDIO_VOLUME_MONITOR */
#else
    {FUNC_AUDIO_VOLUME_MONITOR,                                               0,                       0,                      NULL,                         NULL}, /*0x88 FUNC_AUDIO_VOLUME_MONITOR */
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    {FUNC_HW_VIVID_PT,                                                 0,                          0,      stream_function_hw_vivid_passthru_initialize,          stream_function_hw_vivid_passthru_process},/*0x89 FUNC_HW_VIVID_PT,     */
#else
    {FUNC_HW_VIVID_PT,                                                 0,                          0,                                              NULL,                                               NULL},/*0x89 FUNC_HW_VIVID_PT,     */
#endif
};
/*==========================================================================================================================================================================================================*/


/*             feature_type,                         memory_size,     codec_output_size,                           initialize_entry,                      process_entry*/
stream_feature_codec_t     stream_feature_sample_codec    = {   CODEC_DECODER_SAMPLE,            CODEC_SAMPLE_MEMORY_SIZE,     CODEC_OUTPUT_SIZE,     stream_codec_decoder_sample_initialize, stream_codec_decoder_sample_process};

/*            feature_type,                         memory_size, 0(Must equal to zero),                           initialize_entry,                  process_entry*/
stream_feature_function_t  stream_feature_sample_function = {            FUNC_SAMPLE,         FUNCTION_SAMPLE_MEMORY_SIZE,                     0,          stream_function_sample_initialize, stream_function_sample_process};


stream_feature_ctrl_entry_t stream_feature_sample_entry = {stream_feature_sample_open, stream_feature_sample_close};

#ifdef AIR_BT_HFP_ENABLE
stream_feature_list_t stream_feature_list_hfp_uplink[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL_HFP,
    FUNC_MIC_SW_GAIN,
    FUNC_CLK_SKEW_HFP_UL,
#if (defined(AIR_UL_FIX_SAMPLING_RATE_48K) || defined(AIR_UL_FIX_SAMPLING_RATE_32K) && defined(AIR_FIXED_RATIO_SRC))
    FUNC_SRC_FIXED_RATIO,
#endif
#ifndef AIR_HFP_DNN_PATH_ENABLE
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_SLAVE,
    FUNC_ECNR_POST_PROCESS,
#else
#ifndef AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#endif
#ifdef AIR_3RD_PARTY_NR_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
#endif
    FUNC_TX_WB_DRC,
#ifdef MTK_VOICE_AGC_ENABLE
    //FUNC_TX_AGC,
#endif
#endif /*AIR_HFP_DNN_PATH_ENABLE*/
    CODEC_ENCODER_MSBC,
    FUNC_END,
};

#ifdef AIR_HFP_DNN_PATH_ENABLE
stream_feature_list_t stream_feature_list_hfp_uplink_dnn[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_DNN_NR,
    FUNC_END,
};
#endif

#if defined (AIR_DCHS_MODE_ENABLE)
stream_feature_list_t stream_feature_list_hfp_uplink_dchs[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL_HFP,
    FUNC_MIC_SW_GAIN,
    #ifndef AIR_BTA_IC_STEREO_HIGH_G3
    FUNC_CLK_SKEW_HFP_UL,
    #endif
#ifndef AIR_HFP_DNN_PATH_ENABLE
#ifndef AIR_BTA_IC_STEREO_HIGH_G3
    FUNC_TX_WB_DRC,
#endif
#endif /*AIR_HFP_DNN_PATH_ENABLE*/
    CODEC_ENCODER_MSBC,
    FUNC_END,
};
#endif

stream_feature_list_t stream_feature_list_hfp_downlink_msbc[] = {
    CODEC_DECODER_MSBC,
    FUNC_PLC,
#ifndef AIR_HFP_DNN_PATH_ENABLE
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_RX_NR,
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    //FUNC_RX_WB_AGC,
#endif
#ifdef AIR_VOICE_DRC_ENABLE
    FUNC_RX_WB_DRC,
#endif
#endif
    FUNC_CLK_SKEW_HFP_DL,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_hfp_downlink_cvsd[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
    FUNC_PLC,
#ifndef AIR_HFP_DNN_PATH_ENABLE
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_RX_NR,
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    //FUNC_RX_WB_AGC,
#endif
#ifdef AIR_VOICE_DRC_ENABLE
    FUNC_RX_WB_DRC,
#endif
#endif
    FUNC_CLK_SKEW_HFP_DL,
    FUNC_END,
};
#endif

stream_feature_list_t stream_feature_list_a2dp[] = {
    CODEC_DECODER_SBC,
    /*
        FUNC_MUTE_SMOOTHER,
        //FUNC_PROC_SIZE_CONV,
    */
#if MTK_HWSRC_IN_STREAM
    DSP_SRC,
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
    FUNC_AUDIO_PLC,
#endif
#ifdef AIR_A2DP_PERIODIC_PROCEDURE_V2_EN
    FUNC_QUEUE_BUFFER,
#endif
    FUNC_CH_SEL,
#ifdef MTK_PEQ_ENABLE
    FUNC_PEQ,
    FUNC_PEQ2,
    FUNC_DRC,
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    FUNC_ADAPTIVE_EQ,
    FUNC_ADAPTIVE_EQ_DRC,
#endif
#endif
    FUNC_CLK_SKEW_A2DP_DL,
    FUNC_END,

};

stream_feature_list_t stream_feature_list_playback[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL,
#ifdef MTK_PEQ_ENABLE
    FUNC_PEQ_INSTANT,
    FUNC_PEQ2, //for hybrid ANC L/R ch
    FUNC_DRC,
#endif
    FUNC_END,
};
#if defined(MTK_BT_A2DP_VENDOR_BC_ENABLE) || defined(MTK_BT_A2DP_VENDOR_1_BC_ENABLE)
stream_feature_list_t stream_feature_list_vend_a2dp[] = {
    CODEC_DECODER_VENDOR,
    FUNC_CH_SEL,
    FUNC_END,
};
#endif
stream_feature_list_t stream_feature_list_mic_record[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_mic_record_airdump[] = {
    CODEC_PCM_COPY,
    FUNC_END,
};

#ifdef MTK_WWE_ENABLE
stream_feature_list_t stream_feature_list_wwe_mic_record[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_WWE_PREPROC,
    FUNC_WWE_PROC,
    FUNC_END,
};
#endif

stream_feature_list_t stream_feature_list_prompt[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL_VP,
#ifdef AIR_VP_PEQ_ENABLE
    FUNC_VP_PEQ,
    FUNC_VP_PEQ2,
    FUNC_VP_DRC,
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    FUNC_VP_AEQ,
    FUNC_VP_AEQ_DRC,
#endif
#endif
    FUNC_END,
};

#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
stream_feature_list_t stream_feature_list_prompt_dummy_source[] = {
    CODEC_PCM_COPY,
    FUNC_END,
};
#endif

stream_feature_list_t stream_feature_list_linein[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL,
#ifdef MTK_LINEIN_INS_ENABLE
    FUNC_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_PEQ3,
    FUNC_DRC3,
#endif
    FUNC_END,
};

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
stream_feature_list_t stream_feature_list_audio_loopback_test[] = {
    CODEC_PCM_COPY,
    FUNC_AUDIO_LOOPBACK_TEST,
    FUNC_END,
};
#endif

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
stream_feature_list_t stream_feature_list_tdm[] = {
    CODEC_PCM_COPY,
    FUNC_FIXED_SW_GAIN_TDM,
    FUNC_END,
};
#endif
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
stream_feature_list_t stream_feature_list_leakage_compensation[] = {
    CODEC_PCM_COPY,
    FUNC_LEAKAGE_COMPENSATION,
    FUNC_END,
};
#endif

#ifdef MTK_USER_TRIGGER_FF_ENABLE
stream_feature_list_t stream_feature_list_user_trigger_adaptive_ff_sz[] = {
    CODEC_PCM_COPY,
    FUNC_USER_TRIGGER_FF_SZ,
    FUNC_END,
};
stream_feature_list_t stream_feature_list_user_trigger_adaptive_ff_pz[] = {
    CODEC_PCM_COPY,
    FUNC_USER_TRIGGER_FF_PZ,
    FUNC_END,
};
stream_feature_list_t stream_feature_list_user_trigger_adaptive_ff_pz_fir[] = {
    CODEC_PCM_COPY,
    FUNC_USER_TRIGGER_FF_PZ_FIR,
    FUNC_END,
};
#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
stream_feature_list_t AudioFeatureList_GSensorMotionDetect[] = {
    CODEC_PCM_COPY,
    FUNC_END,
};

stream_feature_list_t AudioFeatureList_GSensorMotionDetect_virtual[] = {
    CODEC_PCM_COPY,
    FUNC_GSENSOR_MOTION_DETECT, //a virtual sample feature
    FUNC_END,
};
#endif


#ifdef AIR_MULTI_MIC_STREAM_ENABLE
const stream_feature_list_t stream_featuremulti_mic_function_a[] = {
    CODEC_PCM_COPY,
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    FUNC_FIXED_SW_GAIN,
    FUNC_FUNCTION_A,
#endif
    FUNC_END,
};
const stream_feature_list_t stream_featuremulti_mic_function_b[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    FUNC_FUNCTION_B,
#endif
    FUNC_END,
};
const stream_feature_list_t stream_featuremulti_mic_function_c[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    FUNC_FUNCTION_C,
#endif
    FUNC_END,
};
const stream_feature_list_t stream_featuremulti_mic_function_f[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    FUNC_FUNCTION_F,
#endif
    FUNC_END,
};
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
stream_feature_list_t AudioFeatureList_BLE_Call_UL[] = {
    CODEC_PCM_COPY,
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
    FUNC_MIC_SW_GAIN,
    FUNC_CLK_SKEW_BLE_CALL_UL,
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_SLAVE,
    FUNC_ECNR_POST_PROCESS,
#else
#ifndef AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#endif
#ifdef AIR_3RD_PARTY_NR_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
#endif
#ifdef AIR_VOICE_DRC_ENABLE
#ifdef AIR_BT_BLE_SWB_ENABLE
    FUNC_TX_SWB_DRC,
#else
    FUNC_TX_WB_DRC,
#endif
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    //FUNC_TX_AGC,
#endif
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
    CODEC_ENCODER_LC3,
    FUNC_END,
};

stream_feature_list_t AudioFeatureList_BLE_Music_DL[] = {
    CODEC_DECODER_LC3,
    FUNC_CH_SEL,
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_PEQ,
    FUNC_PEQ2,
    FUNC_DRC,
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    FUNC_ADAPTIVE_EQ,
    FUNC_ADAPTIVE_EQ_DRC,
#endif
#endif
    FUNC_CLK_SKEW_BLE_MUSIC_DL,
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    FUNC_CH_SEL_2CH_TO_4CH,
#endif
    FUNC_END,
};


stream_feature_list_t AudioFeatureList_BLE_Call_DL[] = {
    CODEC_DECODER_LC3,
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_RX_NR,
#endif
#ifdef MTK_VOICE_AGC_ENABLE
    //FUNC_RX_SWB_AGC,
#endif
#ifdef AIR_VOICE_DRC_ENABLE
#ifdef AIR_BT_BLE_SWB_ENABLE
    FUNC_RX_SWB_DRC,
#else
    FUNC_RX_WB_DRC,
#endif
#endif
// Workaround
#ifndef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
    FUNC_CLK_SKEW_BLE_CALL_DL,
#endif
    FUNC_END,
};
#endif

stream_feature_list_t AudioFeatureList_BLE_LL_DL[] = {
        CODEC_DECODER_LC3,
        FUNC_CH_SEL,
        FUNC_CLK_SKEW_BLE_MUSIC_DL,
        FUNC_END,
};

stream_feature_list_t AudioFeatureList_ULL_BLE_DL[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_CH_SEL,
#ifdef MTK_PEQ_ENABLE
    FUNC_PEQ,
    FUNC_PEQ2,
    FUNC_DRC,
#endif
    FUNC_END,
};

stream_feature_list_t AudioFeatureList_ULL_BLE_UL[] = {
    CODEC_PCM_COPY,
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
#if (defined(AIR_UL_FIX_SAMPLING_RATE_32K) && !defined(AIR_BT_ULL_SWB_ENABLE)) || defined(AIR_UL_FIX_SAMPLING_RATE_48K)
    FUNC_SRC_FIXED_RATIO,
#endif
#endif
// #ifdef AIR_SOFTWARE_GAIN_ENABLE
//     FUNC_SW_GAIN,
// #else
    FUNC_MIC_SW_GAIN,
// #endif
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_MASTER,
#else
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#ifdef AIR_3RD_PARTY_NR_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
#endif
#ifdef AIR_VOICE_DRC_ENABLE
#if defined(AIR_BT_ULL_SWB_ENABLE) || defined(AIR_BT_ULL_FB_ENABLE)
    FUNC_TX_SWB_DRC,
#else
    FUNC_TX_WB_DRC,
#endif
#endif
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};

stream_feature_list_t AudioFeatureList_ULL_BLE_UL_NR_OFFLOAD[] = {
    CODEC_PCM_COPY,
// #ifdef AIR_SOFTWARE_GAIN_ENABLE
//     FUNC_SW_GAIN,
// #else
    FUNC_MIC_SW_GAIN,
// #endif
#ifdef AIR_ECNR_PREV_PART_ENABLE
    FUNC_ECNR_PREV_PROCESS,
#endif
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
stream_feature_list_t AudioFeatureList_WirelessMic_chsel_in_front[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_WIRELESS_MIC,
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_MIC_PEQ,
#endif
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
stream_feature_list_t AudioFeatureList_WirelessMic_chsel_in_back[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_MIC_PEQ,
#endif
    FUNC_CH_SEL_WIRELESS_MIC,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
#else
stream_feature_list_t AudioFeatureList_WirelessMic_chsel_in_front[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_WIRELESS_MIC,
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_MIC_PEQ,
#ifdef AIR_SOFTWARE_DRC_ENABLE
    FUNC_SW_DRC,//FUNC_DRC3,
#endif
#endif
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
stream_feature_list_t AudioFeatureList_WirelessMic_chsel_in_back[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_MIC_PEQ,
#ifdef AIR_SOFTWARE_DRC_ENABLE
    FUNC_SW_DRC,//FUNC_DRC3,
#endif
#endif
    FUNC_CH_SEL_WIRELESS_MIC,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
#endif
#endif

#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
stream_feature_list_t stream_featuremulti_anc_monitor[] = {
    CODEC_PCM_COPY,
#if (defined(AIR_UL_FIX_SAMPLING_RATE_48K) || defined(AIR_UL_FIX_SAMPLING_RATE_32K))  && defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    FUNC_WIND_DETECT,
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    FUNC_USER_UNAWARE,
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    FUNC_ENVIRONMENT_DETECTION,
#endif
    FUNC_END,
};

#endif

#if defined(AIR_ADAPTIVE_EQ_ENABLE)
stream_feature_list_t stream_featuremulti_adaptive_eq_monitor[] = {
    CODEC_PCM_COPY,
    FUNC_ADAPTIVE_EQ_DETECT,
    FUNC_END,
};
#endif

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
stream_feature_list_t stream_feature_list_usb_in_broadcast_0[] = {
    CODEC_PCM_COPY,
#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_OPUS,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_usb_in_broadcast_1[] = {
    CODEC_PCM_COPY,
#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_OPUS,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_usb_out_broadcast[] = {
    CODEC_DECODER_CELT_HD,
#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_ECNR_POST_PART_ENABLE
    FUNC_ECNR_POST_PROCESS,
    FUNC_TX_WB_DRC,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
    /* Support line out mix with usb out */
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#else
    FUNC_MIC_SW_GAIN,
#endif
    FUNC_END,
};
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
stream_feature_list_t stream_feature_list_game_line_in_broadcast[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_OPUS,
    FUNC_END,
};
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
stream_feature_list_t stream_feature_list_game_line_out_broadcast[] = {
    CODEC_DECODER_CELT_HD,
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
// #ifdef AIR_SOFTWARE_SRC_ENABLE
//     FUNC_SW_SRC,
// #endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#else
    FUNC_MIC_SW_GAIN,
#endif
    FUNC_END,
};
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
stream_feature_list_t stream_feature_list_game_i2s_in_broadcast[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_OPUS,
    FUNC_END,
};
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
stream_feature_list_t stream_feature_list_game_headset_ul[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
#ifndef AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE
#if 0
    FUNC_CLK_SKEW_UL,
#endif
    FUNC_TX_NR,
#ifdef AIR_3RD_PARTY_NR_ENABLE
    FUNC_TX_EQ,
#endif
    FUNC_TX_WB_DRC,
#endif /* not AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE */
    CODEC_ENCODER_OPUS,
    FUNC_END,
};

#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(AIR_WIRED_AUDIO_ENABLE)
stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_surround_audio[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#if defined(AIR_SURROUND_AUDIO_ENABLE)
    FUNC_SURROUND_AUDIO,
#endif
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    //FUNC_SW_MIXER,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_seperate_peq[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef MTK_LINEIN_INS_ENABLE
    FUNC_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_PEQ3,
    FUNC_DRC3,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_passthrough[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_in_1_seperate_peq[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef MTK_LINEIN_INS_ENABLE
    FUNC_USB_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_WIRED_USB_PEQ,
    FUNC_WIRED_USB_DRC,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_in_1_no_peq[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    //FUNC_SW_MIXER,
#endif
    FUNC_END,
};

const stream_feature_list_t stream_feature_list_wired_audio_line_in[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL,
#ifdef MTK_LINEIN_INS_ENABLE
    FUNC_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_PEQ3,
    FUNC_DRC3,
#endif

#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_END,
};


stream_feature_list_t const stream_feature_list_wired_audio_usb_out_iem[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef MTK_PEQ_ENABLE
    FUNC_MIC_PEQ,
    FUNC_SW_DRC,
#endif
    FUNC_COPY_TO_VIRTUAL_SOURCE,
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_out[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL_USB_MIC,
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_HFP,
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_MASTER,
#else
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
    FUNC_TX_WB_DRC,
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wired_audio_usb_out_swb[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL_USB_MIC,
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_HFP,
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_MASTER,
#else
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
    FUNC_TX_SWB_DRC,
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
    FUNC_END,
};

const stream_feature_list_t stream_feature_list_wired_audio_line_out[] = {
    CODEC_PCM_COPY,
#if defined(AIR_FIXED_RATIO_SRC)
#if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE))
    FUNC_SRC_FIXED_RATIO,
#endif
#endif
#if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
    FUNC_MIC_SW_GAIN,
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_MASTER,
#else
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
    FUNC_TX_WB_DRC,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#endif /*AIR_WIRED_AUDIO_ENABLE*/
#if defined(AIR_FIXED_RATIO_SRC) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE) /* && defined(AIR_BTA_IC_PREMIUM_G2) */
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_END,
};

const stream_feature_list_t stream_feature_list_wired_audio_line_out_swb[] = {
    CODEC_PCM_COPY,
#if defined(AIR_FIXED_RATIO_SRC)
#if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE))
    FUNC_SRC_FIXED_RATIO,
#endif
#endif
    FUNC_MIC_SW_GAIN,
#if defined (AIR_DCHS_MODE_ENABLE)
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_MASTER,
#else
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_TX_NR,
#endif
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    FUNC_TX_EQ,
#endif
#endif
    FUNC_TX_SWB_DRC,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#if defined(AIR_FIXED_RATIO_SRC) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE) /* && defined(AIR_BTA_IC_PREMIUM_G2) */
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_END,
};

stream_feature_list_t const stream_feature_list_wired_audio_usb_in_out_iem[] = {
    CODEC_PCM_COPY,
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};

stream_feature_list_t const stream_feature_list_wired_audio_main_stream[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};

#endif /*AIR_WIRED_AUDIO_ENABLE*/

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
stream_feature_list_t stream_feature_list_advanced_passthrough[] = {
    CODEC_PCM_COPY,
    FUNC_SW_GAIN,
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
#ifdef AIR_AFC_ENABLE
    FUNC_AFC,
#endif /* AIR_AFC_ENABLE */
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
    FUNC_PEQ4,
#ifdef AIR_AT_AGC_ENABLE
    FUNC_AT_AGC,
#endif /* AIR_AT_AGC_ENABLE */
#if defined(AIR_FIXED_RATIO_SRC)
    FUNC_SRC_FIXED_RATIO,
#endif
    FUNC_END,
};
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_0[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_LC3_BRANCH,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_1[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_LC3_BRANCH,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_out_broadcast[] = {
    CODEC_DECODER_LC3,
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    FUNC_SW_CLK_SKEW,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
stream_feature_list_t stream_feature_list_ble_audio_dongle_line_in_broadcast[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_LC3_BRANCH,
    FUNC_END,
};
#endif

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
stream_feature_list_t stream_feature_list_ble_audio_dongle_i2s_in_broadcast[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
#ifdef AIR_SOFTWARE_SRC_ENABLE
    FUNC_SW_SRC,
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    CODEC_ENCODER_LC3_BRANCH,
    FUNC_END,
};
#endif

#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_GAIN,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_PEQ3,
    FUNC_DRC3,
#endif
#if defined(AIR_SURROUND_AUDIO_ENABLE)
    FUNC_SURROUND_AUDIO,
#endif /* AIR_SURROUND_AUDIO_ENABLE */
    FUNC_SW_MIXER,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1[] = {
    CODEC_PCM_COPY,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_GAIN,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
#ifdef MTK_LINEIN_PEQ_ENABLE
    FUNC_WIRED_USB_PEQ,
    FUNC_WIRED_USB_DRC,
#endif
    FUNC_SW_MIXER,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_BUFFER,
    FUNC_SW_GAIN,
    FUNC_SW_SRC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast_nr_offload[] = {
    CODEC_DECODER_LC3PLUS,
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
#ifdef AIR_ECNR_POST_PART_ENABLE
    FUNC_ECNR_POST_PROCESS,
#endif
#ifdef AIR_BT_ULL_SWB_ENABLE
    FUNC_TX_SWB_DRC,
#else
    FUNC_TX_WB_DRC,
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    FUNC_SW_BUFFER,
#endif
    FUNC_SW_MIXER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_BUFFER,
    FUNC_SW_GAIN,
    FUNC_SW_SRC,
    FUNC_END,
};

#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_in_broadcast[] = {
    // INS???
    CODEC_PCM_COPY,
    // FUNC_SW_BUFFER,
    // FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
#endif

#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast[] = {
    CODEC_PCM_COPY,
    // FUNC_SW_BUFFER,
    // FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_LC3PLUS,
    FUNC_END,
};
#endif

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_GAIN,
    FUNC_END,
};
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_slv_out[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_END,
};
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

#if defined (AIR_DCHS_MODE_ENABLE)
stream_feature_list_t stream_feature_list_dchs_dl_right[] = {
    CODEC_PCM_COPY,
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    FUNC_SW_GAIN,
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    FUNC_SW_MIXER,
#endif
    FUNC_END,
};
stream_feature_list_t stream_feature_list_dchs_dl_left[] = {
    CODEC_PCM_COPY,
    FUNC_END,
};
stream_feature_list_t stream_feature_list_dchs_uart_ul_right[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_HFP,
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_UPLINK_TX,
    FUNC_END,
};
stream_feature_list_t stream_feature_list_dchs_uart_ul_left[] = {
    CODEC_PCM_COPY,
    FUNC_MIC_SW_GAIN,
    FUNC_CH_SEL_HFP,
    FUNC_ECNR_PREV_PROCESS,
    FUNC_DCHS_SW_BUFFER_SLAVE,
    FUNC_ECNR_POST_PROCESS,
    FUNC_DCHS_UPLINK_TX,
    FUNC_END,
};
#endif

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
stream_feature_list_t stream_feature_list_wireless_mic_rx_usb_out[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_DRC,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_BUFFER,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wireless_mic_rx_line_out[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_DRC,
    FUNC_SW_GAIN,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_wireless_mic_rx_i2s_slv_out[] = {
    CODEC_DECODER_LC3PLUS,
    FUNC_SW_MIXER,
    FUNC_SW_DRC,
    FUNC_SW_GAIN,
    FUNC_END,
};
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_a2dp_lhdc[] = {
    CODEC_PCM_COPY,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_VENDOR,
    FUNC_END,
};
#endif

stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_a2dp[] = {
    CODEC_PCM_COPY,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_SRC,
    FUNC_SW_MIXER,
    CODEC_ENCODER_SBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_hfp_msbc[] = {
    CODEC_PCM_COPY,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_MSBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_hfp_cvsd[] = {
    CODEC_PCM_COPY,
    FUNC_SW_BUFFER,
    FUNC_SW_CLK_SKEW,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_out_hfp_msbc[] = {
    //FUNC_SW_MIXER,
    CODEC_DECODER_MSBC,
    FUNC_PLC,
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_RX_NR,
    FUNC_RX_WB_DRC,
#endif
    FUNC_SW_CLK_SKEW,
    FUNC_SW_BUFFER,
    FUNC_SW_GAIN,
    FUNC_SRC_FIXED_RATIO,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_out_hfp_cvsd[] = {
    // FUNC_SW_MIXER,
    CODEC_PCM_COPY,
    FUNC_PLC,
#if defined(AIR_VOICE_NR_ENABLE)
    FUNC_RX_NR,
    FUNC_RX_WB_DRC,
#endif
    FUNC_SW_CLK_SKEW,
    FUNC_SW_BUFFER,
    FUNC_SW_GAIN,
    FUNC_SW_SRC,
    FUNC_END,
};

#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_0[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_SBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_1[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_SBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_2[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_SBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_0[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_MSBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_1[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_MSBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_2[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_PEQ,
    FUNC_DRC,
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    CODEC_ENCODER_MSBC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_0[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    FUNC_SW_SRC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_1[] = {
    CODEC_PCM_COPY,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    FUNC_SW_SRC,
    FUNC_END,
};

stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_2[] = {
    CODEC_PCM_COPY,
    FUNC_CH_SEL,
// #ifdef MTK_LINEIN_INS_ENABLE
//     FUNC_INS,
// #endif
    FUNC_SRC_FIXED_RATIO,
    FUNC_SW_MIXER,
    FUNC_SW_SRC,
    FUNC_END,
};
#endif /* afe in */

#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */

#if defined(AIR_RECORD_ADVANCED_ENABLE)
stream_feature_list_t stream_feature_list_advanced_record_n_mic[] = {
    CODEC_PCM_COPY,
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    FUNC_CH_SEL_WIRELESS_MIC,
#endif
    FUNC_MIC_SW_GAIN,
#ifdef AIR_LD_NR_ENABLE
    FUNC_LD_NR,
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
    FUNC_AUDIO_VOLUME_MONITOR,
#endif
#ifdef MTK_PEQ_ENABLE
    FUNC_ADVANCED_RECORD_PEQ,
#ifdef AIR_SOFTWARE_DRC_ENABLE
    FUNC_SW_DRC,//FUNC_DRC3,
#endif
#endif
    FUNC_END,
};
#endif

static const uint16_t coefficient_table_16k[13] = { //13-tap
    0x0127, 0x027A, 0x0278, 0x0227,
    0xFFD5, 0xFD22, 0xFABF, 0xFAEB,
    0xFE90, 0x05EB, 0x0F47, 0x180A,
    0x1D4E
};

////////////////////////////////////////////////////////////////////////////////
// DSPMEM FUNCTION DECLARATIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * dsp_sdk_add_feature
 *
 * Add customer's feature to feature table
 *
 *
 *
 */
void dsp_sdk_initialize(void)
{
    dsp_sdk_add_feature_table(&stream_feature_sample_codec);
    dsp_sdk_add_feature_table(&stream_feature_sample_function);
    dsp_sdk_add_feature_ctrl(&stream_feature_sample_function, &stream_feature_sample_entry);
}

/**
 * stream_codec_decoder_sample_process
 *
 *
 */
bool stream_codec_decoder_sample_initialize(void *para)
{
    stream_codec_sample_instance_ptr_t codec_sample_pointer;

    //Get working buffer pointer
    codec_sample_pointer = (stream_codec_sample_instance_ptr_t)stream_codec_get_workingbuffer(para);

    //Do initialize
    codec_sample_pointer->memory_check = true;
    codec_sample_pointer->reset_check = false;
    memset(codec_sample_pointer->scratch_memory, 0xFF, 128);

    //return 0 when successfully initialize
    return 0;
}


/**
 * stream_codec_decoder_sample_process
 *
 *
 */
bool stream_codec_decoder_sample_process(void *para)
{
    stream_codec_sample_instance_ptr_t codec_sample_pointer;
    uint8_t *pattern_input_pointer;
    int16_t *output_l_pointer, *output_r_pointer;
    uint32_t input_length, output_length;

    //Get working buffer pointer, stream buffer pointer, and length
    codec_sample_pointer = (stream_codec_sample_instance_ptr_t)stream_codec_get_workingbuffer(para);
    pattern_input_pointer = stream_codec_get_input_buffer(para, 1);
    output_l_pointer = stream_codec_get_output_buffer(para, 1);
    output_r_pointer = stream_codec_get_output_buffer(para, 2);
    input_length = stream_codec_get_input_size(para);
    output_length = stream_codec_get_output_size(para);


    //Call decoder
    //output sample rate : 16kHz
    //output resolution  : 16-bit
    output_length = codec_decoder_sample_api(pattern_input_pointer, output_l_pointer, output_r_pointer, input_length);

    //Check decoder output
    if (output_length == 0) {
        //Do reinitialize when an error occurs.
        stream_feature_reinitialize(para);
    }

    //Check expected resolution
    if (stream_codec_get_output_resolution(para) == RESOLUTION_32BIT) {
        dsp_converter_16bit_to_32bit((int32_t *)output_l_pointer, (int16_t *)output_l_pointer, output_length / sizeof(int16_t));
        dsp_converter_16bit_to_32bit((int32_t *)output_r_pointer, (int16_t *)output_r_pointer, output_length / sizeof(int16_t));
        output_length = output_length * 2;
    }

    //Modify stream buffering format
    stream_codec_modify_output_samplingrate(para, FS_RATE_16K);
    stream_codec_modify_output_size(para, output_length);
    stream_codec_modify_resolution(para, stream_codec_get_output_resolution(para));

    //return 0 when successfully process
    return 0;
}

/**
 * stream_function_sample_initialize
 *
 *
 */
bool stream_function_sample_initialize(void *para)
{
    stream_function_sample_instance_ptr_t function_sample_pointer;

    //Get working buffer pointer
    function_sample_pointer = (stream_function_sample_instance_ptr_t)stream_function_get_working_buffer(para);

    //Do initialize
    memcpy(function_sample_pointer->filter, coefficient_table_16k, 13 * sizeof(uint16_t));
    function_sample_pointer->coefficient_size = 13;

    //return 0 when successfully initialize
    return 0;
}

/**
 * stream_function_sample_process
 *
 *
 */
bool stream_function_sample_process(void *para)
{
    int16_t *l_pointer, *r_pointer;
    uint32_t frame_length;
    stream_function_sample_instance_ptr_t function_sample_pointer;

    //Get working buffer pointer, stream buffer pointer, and length
    function_sample_pointer = (stream_function_sample_instance_ptr_t)stream_function_get_working_buffer(para);
    l_pointer = stream_function_get_inout_buffer(para, 1);
    r_pointer = stream_function_get_inout_buffer(para, 2);
    frame_length = stream_function_get_output_size(para);

    //Call function API
    function_sample_api(function_sample_pointer,
                        l_pointer,
                        r_pointer,
                        function_sample_pointer->buffer_l,
                        function_sample_pointer->buffer_r,
                        frame_length);



    //return 0 when successfully process
    return 0;
}

static uint32_t g_feature_sample_counter = 0;

uint32_t stream_feature_sample_open(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    //dummy open entry
    UNUSED(code_address);
    UNUSED(data_address);
    UNUSED(dram_pic_usage);

    g_feature_sample_counter++;

    return g_feature_sample_counter;
}


uint32_t stream_feature_sample_close(void)
{
    //dummy close enty

    if (g_feature_sample_counter > 0) {
        g_feature_sample_counter--;
    }

    return g_feature_sample_counter;
}


uint32_t codec_decoder_sample_api(uint8_t *pattern_pointer, int16_t *l_pointer, int16_t *r_pointer, uint32_t input_length)
{
    uint32_t codec_output_length = CODEC_OUTPUT_SIZE;
    UNUSED(pattern_pointer);
    UNUSED(l_pointer);
    UNUSED(r_pointer);
    UNUSED(input_length);
    return codec_output_length;
}

uint32_t function_sample_api(stream_function_sample_instance_ptr_t instance_ptr, int16_t *input_l_pointer, int16_t *input_r_pointer, int16_t *output_l_pointer, int16_t *output_r_pointer, uint32_t length)
{
    UNUSED(instance_ptr);
    UNUSED(input_l_pointer);
    UNUSED(input_r_pointer);
    UNUSED(output_l_pointer);
    UNUSED(output_r_pointer);
    UNUSED(length);
    return 0;
}

