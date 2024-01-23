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

#ifndef __HAL_AUDIO_MESSAGE_STRUCT_H__
#define __HAL_AUDIO_MESSAGE_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hal_audio.h"

#include "hal_audio_message_struct_common.h"
#include "hal_audio_nvkey_struct.h"

//--------------------------------------------
// Message queue
//--------------------------------------------
#define AUDIO_MESSAGE_QUEUE_SIZE 32

typedef struct {
    uint16_t message16;
    uint16_t data16;
    uint32_t data32;
} audio_message_element_t;

typedef struct {
    uint32_t                read_index;
    uint32_t                write_index;
    audio_message_element_t message[AUDIO_MESSAGE_QUEUE_SIZE];
} audio_message_queue_t;

//--------------------------------------------
// Share buffer information
//--------------------------------------------
typedef enum {
    SHARE_BUFFER_INFO_INDEX_BT_AUDIO_DL,
    SHARE_BUFFER_INFO_INDEX_BT_VOICE_UL,
    SHARE_BUFFER_INFO_INDEX_BT_VOICE_DL,
#ifdef AIR_BT_CODEC_BLE_ENABLED
    SHARE_BUFFER_INFO_INDEX_BLE_AUDIO_UL,
    SHARE_BUFFER_INFO_INDEX_BLE_AUDIO_DL,
    SHARE_BUFFER_INFO_INDEX_BLE_AUDIO_SUB_UL,
    SHARE_BUFFER_INFO_INDEX_BLE_AUDIO_SUB_DL,
#endif
    SHARE_BUFFER_INFO_INDEX_PROMPT,
    SHARE_BUFFER_INFO_INDEX_RECORD,
    SHARE_BUFFER_INFO_INDEX_RINGTONE,
    SHARE_BUFFER_INFO_INDEX_NVKEY_PARAMETER,
    SHARE_BUFFER_INFO_INDEX_MAX
} n9_dsp_share_info_index_t;

typedef enum {
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_A2DP_SOURCE_UL,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_RECEIVE_FROM_AIR,  //AVM buffer
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_SEND_TO_AIR,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_SEND_TO_MCU,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_SEND_TO_MCU,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_0,  //AVM buffer
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_SEND_TO_MCU,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_0,  //Share buffer
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_RECEIVE_FROM_MCU_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_RECEIVE_FROM_MCU_1,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_SEND_TO_MCU_0,
    AUDIO_TRANSMITTER_SHARE_INFO_INDEX_MAX
} audio_transmitter_share_info_index_t;

#define AVM_SHAEE_BUF_INFO

#if defined (AIR_BLE_AUDIO_DONGLE_ENABLE)
#define SHARE_BUFFER_BT_AUDIO_DL_SIZE      (50*1024)
#else
#define SHARE_BUFFER_BT_AUDIO_DL_SIZE      (40*1024)
#endif
#define SHARE_BUFFER_USB_AUDIO_DL_SIZE      SHARE_BUFFER_BT_AUDIO_DL_SIZE

#ifdef MTK_BT_A2DP_VENDOR_CODEC_BC_ENABLE
#define SHARE_BUFFER_VENDOR_CODEC_SIZE (8*1024)
#endif

#define SHARE_BUFFER_BT_VOICE_UL_SIZE      (480) /* 60*4 */
#define SHARE_BUFFER_BT_VOICE_DL_SIZE      (640) /* 60*8 + 20*8 */
#ifdef AIR_BT_CODEC_BLE_ENABLED
#define SHARE_BUFFER_BLE_AUDIO_UL_PORTION          (1)
#define SHARE_BUFFER_BLE_AUDIO_DL_PORTION          (1)
#define SHARE_BUFFER_BLE_AUDIO_SUB_UL_PORTION      (0) // so far no sub UL
#define SHARE_BUFFER_BLE_AUDIO_SUB_DL_PORTION      (1)
#define SHARE_BUFFER_BLE_AUDIO_TOTAL     (SHARE_BUFFER_BLE_AUDIO_UL_PORTION + SHARE_BUFFER_BLE_AUDIO_DL_PORTION + SHARE_BUFFER_BLE_AUDIO_SUB_UL_PORTION + SHARE_BUFFER_BLE_AUDIO_SUB_DL_PORTION) //  (28+156) *6
#endif
#define SHARE_BUFFER_PROMPT_SIZE           (960*2*4)
#define SHARE_BUFFER_RECORD_SIZE           (2304)
#define SHARE_BUFFER_RINGTONE_SIZE         (2*1024)
#ifdef MTK_BT_SPEAKER_ENABLE
#define SHARE_BUFFER_FEC_SIZE              (10*700)
#endif
#define SHARE_BUFFER_MCU2DSP_PARAMETER_SIZE (2*1024)
#define SHARE_BUFFER_CLK_INFO_SIZE          (8)
#define SHARE_BUFFER_ULL_CLK_INFO_SIZE      (8)
#ifdef AIR_ADAPTIVE_EQ_ENABLE
#define SHARE_BUFFER_AEQ_INDEX_INFO_SIZE    (8)
#endif
#define SHARE_BUFFER_AIRDUMP_SIZE           (400)
#define SHARE_BUFFER_DSP2MCU_PARAMETER_SIZE (1*1024-SHARE_BUFFER_CLK_INFO_SIZE-SHARE_BUFFER_AIRDUMP_SIZE)
#define SHARE_BUFFER_NVKEY_PARAMETER_SIZE   (7*1024)
#define SHARE_BUFFER_AUDIO_SYNC_INFO_SIZE   (sizeof(AUDIO_SYNC_INFO))
#define SHARE_BUFFER_RX_AUDIO_FORWARDER_BUF_SIZE (760)
#define SHARE_BUFFER_TX_AUDIO_FORWARDER_BUF_SIZE (720)
#define SHARE_BUFFER_SYSRAM3_RX_AUDIO_FORWARDER_BUF_SIZE (280)
#define SHARE_BUFFER_SYSRAM3_TX_AUDIO_FORWARDER_BUF_SIZE (240)
#define SHARE_BUFFER_SYSRAM3_AUDIO_FORWARDER_BUF_SIZE (SHARE_BUFFER_SYSRAM3_RX_AUDIO_FORWARDER_BUF_SIZE + SHARE_BUFFER_SYSRAM3_TX_AUDIO_FORWARDER_BUF_SIZE)
#define SHARE_BUFFER_GAMING_MODE_SIZE       (10*1024)
#define SHARE_BUFFER_AUDIO_SYNC_REQUEST_PARAM_SIZE  (sizeof(cm4_dsp_audio_sync_request_param_t))
#ifdef AIR_WIRED_AUDIO_ENABLE
#define SHARE_BUFFER_BT_AUDIO_USB_IN_SIZE   (15*1024)
#define SHARE_BUFFER_BT_AUDIO_USB_OUT_SIZE   (18*1024)
#endif
#define SHARE_BUFFER_RACE_CMD_AUDIO_BUF_SIZE (128)
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
#define SHARE_BUFFER_ANC_MONITOR_INFO_COUNT 5 //max usage: UU gain info = uint32_t *5 = 20B
#endif
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)
#define SHARE_BUFFER_USB_SIZE               (45*1024)
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
#define SHARE_BUFFER_LLF_INFO_COUNT (2710) //max usate: 10808B
#define SHARE_BUFFER_LLF_INFO_COUNT_DSP (5) //max usate: 20B
#define SHARE_BUFFER_LLF_INFO_ID_CM4 (0)
#define SHARE_BUFFER_LLF_INFO_ID_DSP (1)
#endif
#if defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
#define SHARE_BUFFER_AUDIO_PLATFORM_BUFFER_SIZE (256)
#endif

typedef struct {
    uint32_t bt_audio_dl[(SHARE_BUFFER_BT_AUDIO_DL_SIZE+3)/4];
    uint32_t bt_voice_ul[(SHARE_BUFFER_BT_VOICE_UL_SIZE+3)/4];
    uint32_t bt_voice_dl[(SHARE_BUFFER_BT_VOICE_DL_SIZE+3)/4];
    uint32_t prompt[(SHARE_BUFFER_PROMPT_SIZE+3)/4];
    uint32_t record[(SHARE_BUFFER_RECORD_SIZE+3)/4];
    uint32_t ringtone[(SHARE_BUFFER_RINGTONE_SIZE+3)/4];
    uint32_t mcu2dsp_param[(SHARE_BUFFER_MCU2DSP_PARAMETER_SIZE+3)/4];
    uint32_t dsp2mcu_param[(SHARE_BUFFER_DSP2MCU_PARAMETER_SIZE+3)/4];
    uint32_t clk_info[(SHARE_BUFFER_CLK_INFO_SIZE+3)/4];
    uint32_t ull_clk_info[(SHARE_BUFFER_ULL_CLK_INFO_SIZE+3)/4];
    uint32_t airdump[(SHARE_BUFFER_AIRDUMP_SIZE+3)/4];
    uint32_t nvkey_param[(SHARE_BUFFER_NVKEY_PARAMETER_SIZE+3)/4];
    uint32_t audio_sync_info[(SHARE_BUFFER_AUDIO_SYNC_INFO_SIZE+3)/4];
    uint32_t rx_audio_forwarder_buf[(SHARE_BUFFER_RX_AUDIO_FORWARDER_BUF_SIZE+3)/4];
    uint32_t tx_audio_forwarder_buf[(SHARE_BUFFER_TX_AUDIO_FORWARDER_BUF_SIZE+3)/4];
#ifdef AIR_WIRED_AUDIO_ENABLE
    uint32_t usb_in_0[(SHARE_BUFFER_BT_AUDIO_USB_IN_SIZE+3)/4];
    uint32_t usb_in_1[(SHARE_BUFFER_BT_AUDIO_USB_IN_SIZE+3)/4];
    uint32_t usb_out[(SHARE_BUFFER_BT_AUDIO_USB_OUT_SIZE + 3) / 4];
#endif
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    uint32_t dongle_usb_in_0[(SHARE_BUFFER_USB_SIZE+3)/4];
    uint32_t dongle_usb_in_1[(SHARE_BUFFER_USB_SIZE+3)/4];
    uint32_t dongle_usb_out_0[(SHARE_BUFFER_USB_SIZE+3)/4];
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
    uint32_t race_cmd_audio_buf[(SHARE_BUFFER_RACE_CMD_AUDIO_BUF_SIZE + 3) / 4];
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    uint32_t adaptive_eq_index[(SHARE_BUFFER_AEQ_INDEX_INFO_SIZE + 3) / 4];
#endif
} audio_share_buffer_t;
n9_dsp_share_info_t *hal_audio_query_audio_transmitter_share_info(audio_transmitter_share_info_index_t index);
#ifdef AVM_SHAEE_BUF_INFO
void hal_audio_set_gaming_mode_avm_info(avm_share_buf_info_t *p_info, uint32_t buf_len,  uint16_t blksize);
#endif
void hal_audio_set_audio_transmitter_share_info(n9_dsp_share_info_t *p_share_info, uint32_t buf_len, uint32_t max_payload_size);

#ifdef AIR_BT_CODEC_BLE_ENABLED
typedef struct {
    uint32_t *ble_audio_ul;
    uint32_t *ble_audio_dl;
    uint32_t *ble_audio_sub_ul;
    uint32_t *ble_audio_sub_dl;
} leaudio_share_buffer_t;
#endif

//--------------------------------------------
// Error report
//--------------------------------------------
typedef enum {
    DSP_ERROR_REPORT_ERROR,
    DSP_ERROR_REPORT_END,
} dsp_error_report_t;

typedef enum {
    WWE_MODE_NONE =    0,
    WWE_MODE_AMA =     1,
    WWE_MODE_GSOUND =   2,
    WWE_MODE_VENDOR1 = 3,
    WWE_MODE_MAX =     4
} wwe_mode_t;

//--------------------------------------------
// Start Parameters
//--------------------------------------------
typedef struct {
    hal_audio_bits_per_sample_t bit_type;
    uint32_t                    sampling_rate;
    uint32_t                    channel_number;
} audio_dsp_file_pcm_param_t;


typedef struct {
    uint32_t                    header;
} audio_dsp_file_mp3_param_t;

typedef struct {
    uint32_t                    header;
} audio_dsp_file_aac_param_t;

typedef struct {
    uint32_t                    header;
} audio_dsp_file_vendor_param_t;

//===CM4 to DSP message structure==

/* re-init message parameter */
typedef enum {
    MSG2_CM4toDSP_NOT_REINIT =  0,
    MSG2_CM4toDSP_REINIT_BUF_ABNORMAL =  1,
    MSG2_CM4toDSP_REINIT_AFE_ABNORMAL =  2,
} PARTNER_REINIT_CAUSE;



/* SideTone message parameter structure */
typedef struct {
    hal_audio_device_t                      in_device;
    hal_audio_interface_t                   in_interface;
    uint32_t                                in_misc_parms;
    hal_audio_device_t                      out_device;
    hal_audio_interface_t                   out_interface;
    uint32_t                                out_misc_parms;
    hal_audio_channel_selection_t           in_channel; /*HW out channel default R+L*/
    uint32_t                                gain;
    uint32_t                                sample_rate;
#ifdef AIR_SIDETONE_VERIFY_ENABLE
    uint32_t                                sample_rate_in;
#endif
    uint16_t                                on_delay_time;
    uint8_t                                 performance;       // enum : hal_audio_performance_mode_t /*uplink performance: 0:NM, 1:HP, 2:LP, 3:ULP, 4:SULP*/
    uint8_t                                 dmic_pin_sel;
#ifdef AIR_SIDETONE_CUSTOMIZE_ENABLE
    uint8_t                                 ul_adc_mode;       // enum : hal_audio_analog_mdoe_t /*uplink adc mode: 0x0:ACC_10k, 0x1:ACC_20k, 0x2:DCC*/
    bool                                    bias1_2_with_LDO0;
    uint8_t                                 iir_filter;        // enum : hal_audio_ul_iir_t
    uint32_t                                in_device_sample_rate;
#endif
    uint16_t                                *FIR_nvdm_param;
    hal_audio_analog_mdoe_t                 adc_mode;
} mcu2dsp_sidetone_param_t, *mcu2dsp_sidetone_param_p;

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#define PEQ_DIRECT      (0)
#define PEQ_SYNC        (1)
typedef struct {
    uint8_t         *nvkey_addr;
    uint16_t        peq_nvkey_id;
    uint8_t         drc_enable;
    uint8_t         setting_mode;
    uint32_t        target_bt_clk;
    uint8_t         phase_id;
    uint8_t         drc_force_disable;
    uint8_t         gpt_time_sync;
} mcu2dsp_peq_param_t, *mcu2dsp_peq_param_p;
#endif

typedef struct {
    uint32_t config_operation;
    uint8_t config_param[250];
} mcu2dsp_audio_transmitter_runtime_config_param_t;

typedef struct {
    DSP_NVKEY_VAD_COMM vad_nvkey_common;
    DSP_NVKEY_VAD_PARA vad_nvkey_1mic_v_mode;
    DSP_NVKEY_VAD_PARA vad_nvkey_1mic_c_mode;
    DSP_NVKEY_VAD_PARA vad_nvkey_2mic_v_mode;
    DSP_NVKEY_VAD_PARA vad_nvkey_2mic_c_mode;
    uint32_t language_mode_address;
    uint32_t language_mode_length;
    DSP_NVKEY_VOW_PARA vow_setting;
    uint8_t adda_analog_mic_mode; /*0x0: ACC_10k, 0x1: ACC_20k,0x2: DCC*/
} mcu2dsp_vad_param_t, *mcu2dsp_vad_param_p;

//--------------------------------------------
// Config structure
//--------------------------------------------
typedef enum {
    AUDIO_PLAYBACK_CONFIG_EOF
} audio_playback_config_index_t;

//--------------------------------------------
// Audio Sync Parameters
//--------------------------------------------
typedef struct {
    uint32_t                    time_stamp;
    uint32_t                    sample;
} audio_dsp_a2dp_dl_time_param_t;

//--------------------------------------------
// ULL Audio PlayEN Parameters
//--------------------------------------------
typedef struct {
    uint32_t sequence_number;
    uint32_t bt_clock;
} audio_dsp_a2dp_dl_play_en_param_t;


//--------------------------------------------
// A2DP LTCS data
//--------------------------------------------
typedef struct {
    uint32_t *p_ltcs_asi_buf;
    uint32_t *p_ltcs_min_gap_buf;
} audio_dsp_a2dp_ltcs_report_param_t;

typedef struct {
    int32_t drift_comp_val; // long term drift compensation value
    uint32_t anchor_clk;    // long term drift anchor clk
    uint32_t asi_base;      // 1st time is play_en asi, later is anchor asi
    uint32_t asi_cur;   // asi base for current play
} ltcs_anchor_info_t;

//--------------------------------------------
// Leakage compensation data
//--------------------------------------------
typedef void (*anc_leakage_compensation_callback_t)(uint16_t leakage_status);

typedef struct {
    uint16_t calibration_status;
    uint16_t wz_set;
    anc_leakage_compensation_callback_t api_callback;
    anc_leakage_compensation_callback_t vp_start_callback;
    uint8_t enable;
    uint8_t anc_enable;
    uint8_t anc_filter_id;
    uint8_t reserved;
    uint32_t anc_type;
} audio_dsp_leakage_compensation_report_param_t;

#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
typedef struct {
    uint16_t calibration_status;
    anc_leakage_compensation_callback_t api_callback;
    anc_leakage_compensation_callback_t vp_start_callback;
    uint8_t enable;
    uint8_t anc_enable;
    uint8_t anc_filter_id;
    uint8_t anc_type;
} audio_dsp_fadp_anc_compensation_report_param_t;
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
typedef struct STRU_SD_PARA_s
{
    int32_t AutoPowerOff_TH_dB;
    uint32_t AutoPowerOff_Time;
    int32_t NLE_TH_dB;
    uint32_t NLE_Time;
    int32_t RSDM_TH_dB;
    uint32_t RSDM_Time;
    int32_t ExtAmpOff_TH_dB;
    uint32_t ExtAmpOff_Time;
    bool APO_isEnable;
    bool NLE_isEnable;
    bool RSDM_isEnable;
    bool EAO_isEnable;
}PACKED SD_NVKEY_STATE;

typedef void (*silence_detection_callback_t)(bool silence_flag);
typedef void (*silence_detection_scenario_callback_t)(audio_scenario_type_t scenario_type, bool silence_flag);
typedef struct {
    silence_detection_callback_t callback;
    SD_NVKEY_STATE NvKey;
    audio_scenario_type_t scenario_type[4];
    silence_detection_scenario_callback_t scenario_callback[4];
} audio_dsp_silence_detection_param_t;
#endif

#ifdef AIR_ADAPTIVE_EQ_ENABLE
typedef void (*adaptive_eq_notify_callback_t)(int16_t eq_index);
typedef struct {
    adaptive_eq_notify_callback_t callback;
} audio_dsp_adaptive_eq_notify_t;
#endif

#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
typedef void (*mute_speaking_detection_callback_t)(bool speaking_flag);
typedef struct {
    mute_speaking_detection_callback_t callback;
} audio_dsp_mute_speaking_detection_param_t;
#endif
//--------------------------------------------
// Temp audio data
//--------------------------------------------


#ifdef __cplusplus
}
#endif

#endif /*__HAL_AUDIO_MESSAGE_STRUCT_H__ */
