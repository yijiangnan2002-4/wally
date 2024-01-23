/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "task_def.h"
#include "bt_sink_srv_am_task.h"
#include "bt_sink_srv_ami.h"
#include "sidetone_playback.h"
#include "peq_setting.h"
#include "hal_resource_assignment.h"
//#include "bt_sink_srv_media_mgr.h"
#include "audio_nvkey_struct.h"
#include "bt_sink_srv_audio_tunning.h"
#include "bt_sink_srv_audio_setting.h"
#include "bt_sink_srv_audio_setting_nvkey_struct.h"
#include "bt_connection_manager_internal.h"
#include "FreeRTOS.h"
#ifdef __BT_AWS_SUPPORT__
#include "bt_aws.h"
#include "bt_codec.h"
#include "bt_sink_srv_audio_sync.h"
#include "bt_sink_srv_music.h"
#include "bt_sink_srv_aws.h"
#endif /* __BT_AWS_SUPPORT__ */
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
#include "bt_sink_srv_music.h"
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "bt_sink_srv_le_cap_audio_manager.h"
#ifdef  BT_LE_AUDIO_ENABLE
#include "bt_le_audio_def.h"
#endif
#endif

#ifdef __BT_SINK_AUDIO_TUNING__
#include "audio_dsp_fd216_db_to_gain_value_mapping_table.h"
#endif /* __BT_SINK_AUDIO_TUNING__ */
#if PRODUCT_VERSION == 2533
#include "external_dsp_application.h"
#endif

#ifdef __AM_DEBUG_INFO__
#include "bt_sink_srv_utils.h"
#endif

#if defined(MTK_LINEIN_PLAYBACK_ENABLE)
#include "linein_playback.h"
#endif

#ifdef MTK_LINE_IN_ENABLE
#include "audio_sink_srv_line_in.h"
#endif

#include "bt_sink_srv_audio_setting.h"
#include "bt_sink_srv_audio_setting_nvkey_struct.h"

#ifdef AIR_PROMPT_SOUND_ENABLE
#include "prompt_control.h"
#endif

#if defined(MTK_AVM_DIRECT)
#include "audio_src_srv_internal.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_audio_internal.h"
#include "hal_audio_internal_nvkey_struct.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_resource_assignment.h"
#endif

#if defined (AIR_KEEP_I2S_ENABLE) || defined(MTK_AMP_DC_COMPENSATION_ENABLE)
#include "audio_set_driver.h"
#endif

#include "audio_nvdm_common.h"
#include "audio_log.h"
#include "nvkey.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_callback_manager.h"
#endif

#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
#include "usb_audio_playback.h"
#endif

#if defined (AIR_AUDIO_TRANSMITTER_ENABLE)
#include "audio_transmitter_playback.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_service.h"
#endif /* defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) */
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#include "bt_ull_le_service.h"
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_CODEC_BLE_ENABLED) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#include "bt_ble_codec_internal.h"
#endif

#ifdef MTK_USER_TRIGGER_FF_ENABLE
#include "user_trigger_adaptive_ff.h"
#endif

#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
#include "audio_calibration.h"
#endif

#if defined (AIR_AUDIO_TRANSMITTER_ENABLE) && defined (MTK_ANC_SURROUND_MONITOR_ENABLE)
#include "anc_monitor.h"
#endif
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_internal.h"
#if defined(AIR_WIRED_AUDIO_ENABLE)
#include "scenario_wired_audio.h"
#endif
#endif

#if defined (AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
#include "audio_anc_llf_control_internal.h"
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
#include "audio_anc_vivid_pt_api.h"
#endif
#if defined(AIR_CUSTOMIZED_LLF_ENABLE)
#include "audio_anc_llf_example.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#include "race_cmd_co_sys.h"
#include "race_cmd_dsprealtime.h"
#include "race_cmd_hostaudio.h"
#if defined(MTK_ANC_ENABLE)
extern race_dsp_realtime_callback_t  g_race_cosys_dsp_realtime_callback;
#endif
extern void RACE_DSPREALTIME_callback(uint32_t event_id);
extern void RACE_DSPREALTIME_COSYS_GET_PARAM(am_feature_type_t audio_feature, void *data);
#endif

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
extern uint8_t default_dac_mode;
#endif

#ifdef AIR_RECORD_ENABLE
#include "record_playback.h"
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#include "scenario_dchs.h"
#endif
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#include "peq_setting.h"
#endif

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
#include "hal_audio_message_struct_common.h"
#include "ecnr_setting.h"
#endif /* AIR_BT_AUDIO_SYNC_ENABLE */


#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
#include "downlink_sw_gain.h"
#endif

#ifndef _UNUSED
#define _UNUSED(x)  ((void)(x))
#endif
#ifdef MTK_DEVELOPMENT_BOARD_HDK
#define AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE HAL_AUDIO_DEVICE_MAIN_MIC
#else
#define AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC
#endif
#if defined(__AFE_HS_DC_CALIBRATION__)
#define AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HANDSET
#else
#define AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HEADSET
#endif

const uint32_t g_volume_out_config[DEVICE_OUT_MAX][AUD_VOL_TYPE][AUD_VOL_OUT_MAX][2] = {
    {   {{0xFFFFFC18, 0x044C}, {0xFFFFFCE0, 0x044C}, {0xFFFFFDA8, 0x044C}, {0xFFFFFE70, 0x044C}, {0xFFFFFF38, 0x044C}, {0x00000000, 0x044C}, {0x00000000, 0x0384}},
        {{0xFFFFFC18, 0x044C}, {0xFFFFFCE0, 0x044C}, {0xFFFFFDA8, 0x044C}, {0xFFFFFE70, 0x044C}, {0xFFFFFF38, 0x044C}, {0x00000000, 0x044C}, {0x00000000, 0x0384}}
    },
    {   {{0xFFFFF768, 0x0000}, {0xFFFFF8F8, 0x0000}, {0xFFFFFA88, 0x0000}, {0xFFFFFC18, 0x0000}, {0xFFFFFDA8, 0x0000}, {0xFFFFFF38, 0x0000}, {0x00000000, 0x00C8}},
        {{0xFFFFF768, 0x0000}, {0xFFFFF8F8, 0x0000}, {0xFFFFFA88, 0x0000}, {0xFFFFFC18, 0x0000}, {0xFFFFFDA8, 0x0000}, {0xFFFFFF38, 0x0000}, {0x00000000, 0x00C8}}
    }
};
const uint32_t g_volume_in_config[DEVICE_IN_MAX][AUD_VOL_IN_MAX][2] = {{{0x00000000, 0x0258}}};

bt_sink_srv_am_amm_struct *ptr_callback_amm = NULL;
bt_sink_srv_am_amm_struct *ptr_isr_callback_amm = NULL;
bt_sink_srv_am_aud_id_type_t g_rAm_aud_id[AM_REGISTER_ID_TOTAL];
static bt_sink_srv_am_background_t *g_prCurrent_resumer = NULL;
static uint8_t *g_bBT_Ringbuf = NULL;
#ifdef RTOS_TIMER
static TimerHandle_t g_xTimer_am;
static uint16_t g_lExpire_count = 0;
#endif
static bt_sink_srv_am_background_t g_rBackground_head = {0};
static bt_sink_srv_am_sink_state_t g_rSink_state = A2DP_SINK_CODEC_CLOSE;
#ifdef AIR_BT_HFP_ENABLE
static bt_sink_srv_am_media_handle_t g_prHfp_sink_event_handle;
#endif
bt_media_handle_t *g_prHfp_media_handle = NULL;
static bt_sink_srv_am_media_handle_t g_prA2dp_sink_event_handle;
bt_media_handle_t *g_prA2dp_sink_handle = NULL;
bt_sink_srv_am_background_t *g_prCurrent_playback[AM_PLAYBACK_INDEX_TOTAL] = {NULL, NULL, NULL, NULL};
#ifdef MTK_LINE_IN_ENABLE
audio_sink_srv_am_line_in_codec_t g_prLineIN_sink_handle;
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
static bt_sink_srv_am_media_handle_t g_prBle_sink_event_handle;
bt_media_handle_t *g_prBle_media_handle = NULL;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
static bt_sink_srv_am_media_handle_t g_prUllBleUl_sink_event_handle;
bt_media_handle_t *g_prUllBleUl_media_handle = NULL;
static bt_sink_srv_am_media_handle_t g_prUllBleDl_sink_event_handle;
bt_media_handle_t *g_prUllBleDl_media_handle = NULL;
#endif
extern audio_src_srv_priority_t g_bt_ull_le_am_priority;
#endif

#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
extern anc_control_t g_anc_control;
static bool g_am_anc_flag = false;
#endif
#endif
#endif
static uint16_t *aud_memory = NULL;
#ifndef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
static uint8_t g_bt_sink_srv_am_ring_buffer[AM_RING_BUFFER_SIZE];
#endif

int32_t g_test_fix_warning = 0;

#if defined(MTK_AVM_DIRECT)
volatile uint32_t g_am_task_mask = 0;
extern HAL_AUDIO_CHANNEL_SELECT_t audio_Channel_Select;
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
extern HAL_AUDIO_DVFS_CLK_SELECT_t audio_nvdm_dvfs_config;
xSemaphoreHandle g_xSemaphore_Audio = NULL;
extern const audio_version_t SW_version;
extern volatile audio_version_t nvdm_version;
#endif
#ifdef AIR_RECORD_ENABLE
extern uint16_t g_stream_in_code_type;//modify for opus
extern encoder_bitrate_t g_bit_rate;
extern uint16_t g_wwe_mode;
#endif

extern int g_side_tone_gain_common;
extern int g_side_tone_gain_hfp;

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
extern uint8_t g_peq_init_flag;
extern peq_nvdm_misc_param_t g_peq_handle;
#endif
/* external dsp*/
//external_dsp_sending_path_register_value_t external_dsp_sending_path_register_value;

/* External DSP*/
//#if PRODUCT_VERSION == 2533
#if 0
external_dsp_sending_path_register_value_t external_dsp_sending_path_register_value;
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
audio_channel_selection_t g_wireless_mic_channel_select = AUDIO_CHANNEL_SELECTION_MIC_DEFAULE;
#endif

/* files media handle */
static bt_sink_srv_am_files_media_handle_t g_am_files_media_handle;
static bt_sink_srv_am_file_state_t g_am_file_state = FILE_CODEC_CLOSE;

/* audio codec */
#ifdef __AUDIO_COMMON_CODEC_ENABLE__
static audio_codec_media_handle_t *g_am_audio_media_handle = NULL;
#endif

#ifdef __BT_AWS_SUPPORT__
static bt_aws_codec_type_t g_aws_codec_type = 0;
static uint16_t g_aws_sample_rate = 0;
static uint32_t g_aws_skew_loop_count = 0;
#endif /* __BT_AWS_SUPPORT__ */
static uint32_t g_a2dp_underflow_loop_count = 0;
#ifdef __BT_AWS_SUPPORT__
static uint32_t g_aws_underflow_loop_count = 0;
#endif /* __BT_AWS_SUPPORT__ */
#define BT_SINK_SRV_AM_MAX_UNDERFLOW_COUNT          (1)

#define AUDIO_DUMP_CONFIG_MAX_NUM                   (20)
ATTR_SHARE_ZIDATA static uint16_t audio_dump_ids[AUDIO_DUMP_CONFIG_MAX_NUM];

static bool g_am_volume_enable = false;
static void am_receive_msg(bt_sink_srv_am_amm_struct *amm_ptr);
static void bt_codec_am_a2dp_sink_open(bt_sink_srv_am_a2dp_codec_t *a2dp_codec_t);
static bt_status_t bt_codec_am_a2dp_sink_play(bt_sink_srv_am_id_t aud_id);
static bt_status_t bt_codec_am_a2dp_sink_stop(bt_sink_srv_am_id_t aud_id);
static bt_status_t bt_codec_am_a2dp_sink_close(void);
#ifdef AIR_BT_HFP_ENABLE
static void bt_codec_am_hfp_open(bt_sink_srv_am_hfp_codec_t *hfp_codec_t);
static bt_status_t bt_codec_am_hfp_stop(void);
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
static void bt_codec_am_ble_open(bt_sink_srv_am_ble_codec_t *hfp_codec_t);
static bt_status_t bt_codec_am_ble_stop(void);
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static void bt_codec_am_ull_ble_ul_open(bt_codec_le_audio_ul_t ul_param,bt_sink_srv_am_background_t *background_ptr);
static bt_status_t bt_codec_am_ull_ble_ul_stop(void);
static void bt_codec_am_ull_ble_dl_open(bt_codec_le_audio_dl_t dl_param);
static bt_status_t bt_codec_am_ull_ble_dl_stop(void);
#endif

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
extern void hal_audio_set_a2dp_play_en(uint32_t seq_no, uint32_t bt_clk);
#endif

static void am_files_codec_open(bt_sink_srv_am_files_format_t *files_format);

static void am_files_codec_close(void);

#ifdef __BT_AWS_SUPPORT__
static int32_t bt_a2dp_aws_set_flag(bt_sink_srv_am_id_t aud_id, bool flag);
static int32_t bt_a2dp_aws_set_initial_sync(bt_sink_srv_am_id_t aud_id);
static void bt_a2dp_aws_plh_init(bt_sink_srv_am_id_t aud_id);
static void bt_a2dp_aws_plh_deinit(void);
static void aud_process_aws_a2dp_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
static int32_t bt_mp3_aws_set_flag(bt_sink_srv_am_id_t aud_id, bool flag);
static int32_t bt_mp3_aws_set_initial_sync(bt_sink_srv_am_id_t aud_id);
static int32_t bt_mp3_aws_init();
static int32_t bt_mp3_aws_deinit();
static int32_t bt_mp3_aws_set_clock_skew_compensation_value(int32_t sample_count);
static int32_t bt_mp3_aws_get_clock_skew_status(int32_t *status);
#endif /* __BT_AWS_SUPPORT__ */

static void audio_set_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
static void audio_set_pause_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);

void audio_side_tone_enable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
void audio_side_tone_disable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
void audio_side_tone_set_volume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);

static void audio_dl_suspend_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
static void audio_dl_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
static void audio_ul_suspend_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
static void audio_ul_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);

#ifdef AIR_BT_SINK_MUSIC_ENABLE
extern void BT_A2DP_CONVERT_SBC_CODEC(bt_codec_capability_t *dst_codec,
                                      bt_a2dp_codec_capability_t *src_codec);

extern void BT_A2DP_CONVERT_AAC_CODEC(bt_codec_capability_t *dst_codec,
                                      bt_a2dp_codec_capability_t *src_codec);

#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
extern void RACE_DSPREALTIME_ANC_PASSTHRU_RELAY_COSYS_CALLBACK(bool is_critical, uint8_t *buff, uint32_t len);
#endif

#ifndef MTK_DEBUG_LEVEL_INFO
extern int32_t g_test_fix_warning;
#endif


#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
#endif

#ifdef __BT_AWS_SUPPORT__
extern void bt_sink_srv_fetch_bt_offset();
#endif

#if defined(MTK_AVM_DIRECT)
static void audio_set_feature_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
bt_sink_srv_am_result_t audio_nvdm_configure_init(void);
#endif

void audio_message_audio_handler(hal_audio_event_t event, void *data);

#if defined(AIR_WIRED_AUDIO_ENABLE)
static void audio_transmitter_wired_audio_stop_close_usb_in(uint16_t scenario_and_id);
#endif /* AIR_WIRED_AUDIO_ENABLE */

#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
extern prompt_control_status_t prompt_control_dummy_source_start_internal(prompt_control_dummy_source_param_t param);
extern void prompt_control_dummy_source_stop_internal(void);
#endif /* VP Dummy Sourece */

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
extern cm4_dsp_audio_sync_request_param_t *hal_audio_query_audio_sync_request_param(void);
#endif /* AIR_BT_AUDIO_SYNC_ENABLE */

#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
void audio_volume_monitor_stop_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
#endif /* AIR_AUDIO_VOLUME_MONITOR_ENABLE */

extern void vPortFreeNC(void *pv);
extern void *pvPortMallocNC(size_t xWantedSize);
#ifndef WIN32_UT
/*****************************************************************************
 * FUNCTION
 *  am_task_create
 * DESCRIPTION
 *  This function is used to create a task.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void am_task_create(void)
{
    xTaskCreate(am_task_main,
                AM_TASK_NAME,
                AM_TASK_STACKSIZE,
                NULL,
                AM_TASK_PRIO,
                NULL);
}
#endif

bt_sink_srv_am_background_t *am_get_aud_by_type(bt_sink_srv_am_type_t type)
{
    bt_sink_srv_am_background_t *prTarget_player = NULL;
    uint32_t i;
    for (i = 0; i < AM_REGISTER_ID_TOTAL; i++) {
        if (g_rAm_aud_id[i].contain_ptr && g_rAm_aud_id[i].contain_ptr->type == type) {
            prTarget_player = g_rAm_aud_id[i].contain_ptr;
            break;
        }
    }
    return prTarget_player;
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_set_play
 * DESCRIPTION
 *  This function is used to play the audio handler.
 * PARAMETERS
 *  background_ptr   [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void am_set_A2DP_volume(bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__

    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = VOL_A2DP;
    vol_info.vol_info.a2dp_vol_info.dev = out_audio_device;
    vol_info.vol_info.a2dp_vol_info.lev = out_audio_volume;
    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
#else
    aud_set_volume_level(STREAM_OUT, AUD_VOL_AUDIO, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */
}

void am_set_HFP_volume(bt_hfp_audio_codec_type_t type, bt_sink_srv_am_device_set_t in_audio_device, bt_sink_srv_am_volume_level_in_t in_audio_volume,
                             bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume, bool set_in_vol, bool set_out_vol)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = VOL_HFP;
    vol_info.vol_info.hfp_vol_info.codec = type;
    vol_info.vol_info.hfp_vol_info.dev_in = in_audio_device;
    vol_info.vol_info.hfp_vol_info.dev_out = out_audio_device;
    vol_info.vol_info.hfp_vol_info.lev_in = in_audio_volume;
    vol_info.vol_info.hfp_vol_info.lev_out = out_audio_volume;

    if(set_in_vol){
        bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
            }
    if(set_out_vol){
        bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
        }
#else
    if(set_in_vol){
        aud_set_volume_level(STREAM_IN, AUD_VOL_AUDIO, in_audio_device, (bt_sink_srv_am_volume_level_t)in_audio_volume);
    }
    if(set_out_vol){
        aud_set_volume_level(STREAM_OUT, AUD_VOL_SPEECH, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
    }
#endif
}

static void am_set_PCM_volume(bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = VOL_PCM;
    vol_info.vol_info.pcm_vol_info.dev = out_audio_device;
    vol_info.vol_info.pcm_vol_info.lev = out_audio_volume;
    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
#else
    aud_set_volume_level(STREAM_OUT, AUD_VOL_AUDIO, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
#endif
}

static void am_set_MP3_volume(bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = VOL_MP3;
    vol_info.vol_info.mp3_vol_info.dev = out_audio_device;
    vol_info.vol_info.mp3_vol_info.lev = out_audio_volume;
    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
#else
    aud_set_volume_level(STREAM_OUT, AUD_VOL_AUDIO, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
#endif
}

#ifdef AIR_RECORD_ENABLE
static void am_set_VC_volume(void)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};
    vol_info.type = VOL_VC;
    bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
#else
    hal_audio_set_stream_in_volume(3000, 0);
#endif
}
#endif

#if defined(AIR_WIRED_AUDIO_ENABLE) || defined(MTK_LINE_IN_ENABLE)
static void am_set_LINE_IN_volume(vol_type_t vol_type, bt_sink_srv_am_device_set_t in_audio_device, bt_sink_srv_am_volume_level_in_t in_audio_volume,
                             bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume, bool set_in_vol, bool set_out_vol)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = vol_type;
    vol_info.vol_info.lineIN_vol_info.dev_in  = in_audio_device;
    vol_info.vol_info.lineIN_vol_info.dev_out = out_audio_device;
    vol_info.vol_info.lineIN_vol_info.lev_in  = in_audio_volume;
    vol_info.vol_info.lineIN_vol_info.lev_out = out_audio_volume;

    if(set_in_vol){
        bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
    }
    if(set_out_vol){
        bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
        }
#else
    if(set_in_vol){
        aud_set_volume_level(STREAM_IN, AUD_VOL_AUDIO, in_audio_device, (bt_sink_srv_am_volume_level_t)in_audio_volume);
        }
    if(set_out_vol){
        aud_set_volume_level(STREAM_OUT, AUD_VOL_SPEECH, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
        }
#endif
}
#endif

static void am_set_USB_AUDIO_IN_volume(vol_type_t vol_type, bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume)
{
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
    bt_sink_srv_audio_setting_vol_info_t vol_info= {0};

    vol_info.type = vol_type;
    vol_info.vol_info.usb_audio_vol_info.dev = out_audio_device;
    vol_info.vol_info.usb_audio_vol_info.lev = out_audio_volume;
    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
#else
    aud_set_volume_level(STREAM_OUT, AUD_VOL_AUDIO, out_audio_device, (bt_sink_srv_am_volume_level_t)out_audio_volume);
#endif
}

#ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
bt_sink_srv_am_hal_result_t aud_set_volume_level(bt_sink_srv_am_stream_type_t in_out,
                                                 bt_sink_srv_am_volume_type_t vol_type,
                                                 bt_sink_srv_am_device_set_t device,
                                                 bt_sink_srv_am_volume_level_t level)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_hal_result_t eResult = HAL_AUDIO_STATUS_ERROR;
    bt_sink_srv_am_device_out_t dev_out;
    bt_sink_srv_am_device_in_t dev_in;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
    ami_execute_vendor_se(EVENT_VOLUME_CHANGE);
#endif

    audio_src_srv_report("[Sink][AM]in_out:%d, level:%d, device:0x%x, vol_type:%d, g_am_volume_enable:%d", 5, in_out, level, device, vol_type, g_am_volume_enable);

    if (in_out == STREAM_OUT) {
        if (level < AUD_VOL_OUT_MAX) {
            if (device & DEVICE_LOUDSPEAKER) {
                //device = LOUDSPEAKER_STREAM_OUT;
                dev_out = LOUDSPEAKER_STREAM_OUT;
                //audio_src_srv_report("[Sink][AM]dev_out:%d, digital:0x%x, analog:0x%x", 3, dev_out, g_volume_out_config[dev_out][vol_type][level][0], g_volume_out_config[dev_out][vol_type][level][1]);
                if (g_am_volume_enable) {
                    ;
                } else {
                    eResult = hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, g_volume_out_config[dev_out][vol_type][level][0], g_volume_out_config[dev_out][vol_type][level][1]);
                }
            } else if (device & DEVICE_EARPHONE) {
                //device = EARPHONE_STREAM_OUT;
                dev_out = EARPHONE_STREAM_OUT;
                //audio_src_srv_report("[Sink][AM]dev_out:%d, digital:0x%x, analog:0x%x", 3, dev_out, g_volume_out_config[dev_out][vol_type][level][0], g_volume_out_config[dev_out][vol_type][level][1]);
                if (g_am_volume_enable) {
                    ;
            } else {
                    eResult = hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, g_volume_out_config[dev_out][vol_type][level][0], g_volume_out_config[dev_out][vol_type][level][1]);
                        }
                    }
            audio_src_srv_report("[Sink][AM]dev_out:%d, digital:0x%x, analog:0x%x", 3, dev_out, g_volume_out_config[dev_out][vol_type][level][0], g_volume_out_config[dev_out][vol_type][level][1]);
        }
    } else if (in_out == STREAM_IN) {
        if (level < AUD_VOL_IN_MAX) {
            if (device & DEVICE_IN_LIST) {
                //device = MICPHONE_STREAM_IN;
                dev_in = MICPHONE_STREAM_IN;
                if (g_am_volume_enable) {
                    ;
                } else {
                    eResult = hal_audio_set_stream_in_volume(g_volume_in_config[dev_in][level][0], g_volume_in_config[dev_in][level][1]);
                }
            }
        }
        }

    if (eResult == HAL_AUDIO_STATUS_OK) {
        return HAL_AUDIO_STATUS_OK;
    } else {
#ifdef __AM_DEBUG_INFO__
        audio_src_srv_report("[Sink][AM][ERROR] Device and Stream type/Vol-level Not Matched", 0);
#endif
        return HAL_AUDIO_STATUS_ERROR;
    }
}
#endif /*#ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__*/


static void aud_prepare_a2dp_nvkey(bt_sink_srv_am_feature_t *local_feature)
{
#if defined(MTK_AVM_DIRECT)
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_A2DP[] = {
#ifdef MTK_PEQ_ENABLE
        FUNC_PEQ_A2DP,
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        FUNC_ADAPTIVE_EQ,
#endif
        FUNC_END,
    };
#ifdef MTK_PEQ_ENABLE
    bt_sink_srv_am_peq_param_t am_peq_param;
#endif

#ifdef MTK_PEQ_ENABLE
    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set pre PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.a2dp_pre_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.a2dp_pre_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_A2DP, &am_peq_param);
#ifndef MTK_ANC_ENABLE
    /* set post PEQ*/
    am_peq_param.phase_id = 1;
    am_peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.a2dp_post_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_A2DP, &am_peq_param);
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    aud_set_adaptive_aeq_param();
    /* set adaptive PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.adaptive_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.adaptive_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_ADAPTIVE_EQ, &am_peq_param);
#endif
#endif
    audio_set_anc_compensate(A2DP, 0, NULL);
#ifdef MTK_ANC_ENABLE
    audio_set_anc_compensate_phase2(A2DP, 0);
#endif
    audio_nvdm_reset_sysram();
    status = audio_nvdm_set_feature(sizeof(AudioFeatureList_A2DP) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_A2DP);
    if (status != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0 && "[Sink][AM] A2DP open is failed to set nvkey to share memory");
    }
#if (defined(MTK_ANC_ENABLE) && defined(MTK_LEAKAGE_DETECTION_ENABLE) && (MTK_AP_DEFAULT_TYPE == 2))
#ifndef MTK_ANC_V2
    //workaround for A3 leakage detection
    if (audio_anc_leakage_compensation_get_status()) {
        uint32_t positive_gain[4], positive_gain_size = 16;
        mem_nvdm_info_t mem_nvdm;
        status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain[0], &positive_gain_size);
        if (status != NVDM_STATUS_NAT_OK) {
            AUDIO_ASSERT(0 && "[Sink][AM] A2DP open is failed to read NVKEY_DSP_PARA_POSITIVE_GAIN");
        }
        positive_gain[2] += 12; //+6dB
        mem_nvdm.nvdm_id = NVKEY_DSP_PARA_POSITIVE_GAIN;
        mem_nvdm.length = positive_gain_size;
        mem_nvdm.mem_pt = (const void *)&positive_gain[0];
        status = nat_table_write_audio_nvdm_data(mem_nvdm, c_sram_mode);
        if (status != NVDM_STATUS_NAT_OK) {
            AUDIO_ASSERT(0 && "[Sink][AM] A2DP open is failed to set NVKEY_DSP_PARA_POSITIVE_GAIN");
        }
    }
#endif
#endif
#endif
}

#if defined(MTK_LINE_IN_ENABLE) || (defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
#if defined(MTK_LINEIN_PEQ_ENABLE) || defined(MTK_LINEIN_INS_ENABLE)
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
#include "nvkey.h"
#define NVKEYID_E450            "E450"
static const uint8_t NVKEY_E450[] = {
    0x01, 0x00, 0x00, 0x00, 0x67, 0xF3, 0xFF, 0xFF, 0xC8, 0x00, 0x00, 0x00, 0x67, 0xF3, 0xFF, 0xFF, 0xD0, 0x07, 0x00, 0x00, 0x18, 0xFC, 0xFF, 0xFF, 0x19, 0x00, 0x00, 0x00, 0xE7, 0xFF, 0xFF, 0xFF, 0x91, 0x87, 0x08, 0x00, 0xCB, 0xBF, 0x04, 0x00, 0x90, 0x87, 0x08, 0x00, 0xE8, 0x9F, 0xB8, 0xFF, 0x25, 0x29, 0x32, 0x00, 0x2E, 0xC5, 0x42, 0x00, 0x5F, 0xC7, 0xF3, 0xFF, 0x2D, 0xC5, 0x42, 0x00, 0x29, 0x00, 0xB8, 0xFF, 0x29, 0x36, 0x64, 0x00, 0x00, 0x00, 0x40, 0x00, 0xC9, 0x7E, 0x74, 0x00, 0xCF, 0x84, 0x8B, 0xFF, 0xC8, 0x7E, 0x74, 0x00, 0xA6, 0x23, 0x88, 0xFF, 0x71, 0xA4, 0x70, 0x00, 0xD0, 0xEA, 0x7E, 0x00, 0x16, 0x29, 0x81, 0xFF, 0xCF, 0xEA, 0x7E, 0x00, 0xDF, 0x73, 0x81, 0xFF, 0x85, 0x91, 0x7D, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x1E, 0x00, 0x06, 0x00, 0xB0, 0x0A, 0x0C, 0x00, 0x00, 0x00, 0x00,
};
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
static void aud_prepare_linein_nvkey(bt_sink_srv_am_feature_t *local_feature, uint8_t type)
{
#if defined(MTK_AVM_DIRECT)
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_LINEIN[] = {
#ifdef MTK_LINEIN_INS_ENABLE
        FUNC_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
        FUNC_PEQ_LINEIN,
#endif
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
        FUNC_GAME_CHAT_VOLUME_SMART_BALANCE,
#endif
        FUNC_END,
    };
#ifdef MTK_LINEIN_PEQ_ENABLE
    bt_sink_srv_am_peq_param_t am_peq_param;
#endif

#ifdef MTK_LINEIN_PEQ_ENABLE
    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set pre PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.linein_pre_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.linein_pre_peq_sound_mode;
    if((type == 0) || (type == 1)){
        aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &am_peq_param);
    }else if(type == 2){
        aud_set_peq_param(PEQ_AUDIO_PATH_USB, &am_peq_param);
    }

#ifndef MTK_ANC_ENABLE
    /* set post PEQ*/
    am_peq_param.phase_id = 1;
    am_peq_param.enable = g_peq_handle.linein_post_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.linein_post_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &am_peq_param);
#else
    audio_set_anc_compensate(LINE_IN, 0, NULL);
#endif
#endif
    audio_nvdm_reset_sysram();
    // status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LINEIN)/sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LINEIN);
    // if (status != NVDM_STATUS_NAT_OK){
    //     audio_src_srv_err("[Sink][AM] LINEIN open is failed to set nvkey to share memory - err(%d)\r\n", 1, status);
    //     audio_src_srv_report("[Sink][AM] LINEIN open is failed to set nvkey to share memory", 0);
    //     AUDIO_ASSERT(0);
    // }
    status = NVDM_STATUS_ERROR;
    while (status != NVDM_STATUS_NAT_OK) {
        /* set NVKEYs that the usb chat stream uses into the share buffer */
        status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LINEIN) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LINEIN);
        if (status != NVDM_STATUS_NAT_OK) {
//             audio_src_srv_err("[Sink][AM] LINEIN open is failed to set parameters to share memory - err(%d)\r\n", 1, status);

//             /* workaround for that maybe this NVKEY_F50C is not exsited after the FOTA */
//             if ((nvdm_status_t)status == NVDM_STATUS_ITEM_NOT_FOUND) {
// #ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
//                 //audio_src_srv_report("[Sink][AM] LINEIN open is set default parameters into the NVKEY\r\n", 0);
//                 nvdm_status_t nvdm_status = nvdm_write_data_item("AB15", NVKEYID_F50C, NVDM_DATA_ITEM_TYPE_RAW_DATA, (const uint8_t *)NVKEY_F50C, sizeof(NVKEY_F50C));
//                 if (nvdm_status != NVDM_STATUS_OK) {
//                     audio_src_srv_report("[Sink][AM] LINEIN open is failed to set default parameters into the NVKEY - err(%d)\r\n", 1, nvdm_status);
//                     AUDIO_ASSERT(0);
//                 }
// #else
//                 AUDIO_ASSERT(0);
// #endif
//             } else {
//                 AUDIO_ASSERT(0);
//             }
            AUDIO_ASSERT(0 && "[Sink][AM] LINEIN open is failed to set parameters to share memory");
        }
    }
#endif
}
#endif
#endif

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
static void aud_prepare_hearing_aid_nvkey(bt_sink_srv_am_feature_t *local_feature)
{
    bt_sink_srv_am_peq_param_t am_peq_param;
    nvdm_status_t nvdm_status = NVDM_STATUS_OK;
    uint32_t item_size = 0;
    bool afc_enable = false;
    uint8_t *nvkey_buf;

    /* set special PEQ */
    nvdm_status = nvkey_data_item_length(NVID_DSP_ALG_AFC, &item_size);
    if ((nvdm_status != NVDM_STATUS_OK) || (item_size == 0)) {
        audio_src_srv_report("[Hearing-aid[Warning]NVDM AFC Fail. Status:%d Len:%u\n", 2, nvdm_status, item_size);
        afc_enable = false;
    } else {
        nvkey_buf = (uint8_t *)pvPortMallocNC(item_size);
        nvdm_status = nvkey_read_data(NVID_DSP_ALG_AFC, nvkey_buf, &item_size);
        if (nvdm_status != NVDM_STATUS_OK) {
            audio_src_srv_report("[Hearing-aid[Warning]NVDM AFC Read Fail. Status:%d Len:%u\n", 2, nvdm_status, item_size);
            afc_enable = false;
        } else {
            if (*nvkey_buf == 0) {
                afc_enable = false;
                //audio_src_srv_report("[Hearing-aid]NVDM AFC disable.", 0);
            } else {
                afc_enable = true;
                //audio_src_srv_report("[Hearing-aid]NVDM AFC enable.", 0);
            }
            audio_src_srv_report("[Hearing-aid]NVDM AFC enable/disable  %d .", 1, afc_enable);
        }
        vPortFreeNC(nvkey_buf);
    }
    if (afc_enable != false) {
        /* enable special PEQ */
        aud_set_special_peq_param(0xf, NVID_DSP_ALG_PSAP_SPEC_PEQ);
    } else {
        /* disable special PEQ */
        aud_set_special_peq_param(0xf, 0);
    }

    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set pre PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable     = g_peq_handle.advanced_passthrough_pre_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.advanced_passthrough_pre_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH, &am_peq_param);
}
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */

#if defined (MTK_PEQ_ENABLE)
#if defined (AIR_WIRED_AUDIO_ENABLE)&& defined(AIR_WIRELESS_MIC_TX_ENABLE)
static void aud_prepare_mic_peq_nvkey(void)
{
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList[] = {
        FUNC_WIRELESS_MIC,
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        FUNC_SILENCE_DETECTION,
#endif
        FUNC_END,
    };

    bt_sink_srv_am_peq_param_t am_peq_param;
    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.mic_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.mic_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_MIC, &am_peq_param);
    audio_nvdm_reset_sysram();
    status = NVDM_STATUS_ERROR;
    while (status != NVDM_STATUS_NAT_OK) {
        /* set NVKEYs that the usb chat stream uses into the share buffer */
        status = audio_nvdm_set_feature(sizeof(AudioFeatureList) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM] MIC PEQ open is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
    }
}
#endif

#if defined (AIR_RECORD_ADVANCED_ENABLE)
static void aud_prepare_advanced_record_nvkey(void)
{
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList[] = {
        FUNC_WIRELESS_MIC,
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        FUNC_SILENCE_DETECTION,
#endif
        FUNC_END,
    };

    bt_sink_srv_am_peq_param_t am_peq_param;
    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.advanced_record_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.advanced_record_peq_sound_mode;
    aud_set_peq_param(PEQ_AUDIO_PATH_MIC, &am_peq_param);
    audio_nvdm_reset_sysram();
    status = NVDM_STATUS_ERROR;
    while (status != NVDM_STATUS_NAT_OK) {
        /* set NVKEYs that the usb chat stream uses into the share buffer */
        status = audio_nvdm_set_feature(sizeof(AudioFeatureList) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM] MIC PEQ open is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
    }
}
#endif
#endif //MTK_PEQ_ENABLE

void aud_stream_out_callback(bt_sink_srv_am_event_result_t event, void *user_data);

void aud_stream_in_callback(bt_sink_srv_am_event_result_t event, void *user_data);
#ifdef AIR_RECORD_ENABLE
static void am_audio_record_sink_play(bt_sink_srv_am_background_t *background_ptr);
static void am_audio_record_sink_stop(void);
#endif

static void aud_dl_control(bool isCodecOpen);
#if defined(AIR_BT_CODEC_BLE_ENABLED) || defined(AIR_BT_HFP_ENABLE) || defined(AIR_RECORD_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static void aud_ul_control(bool isCodecOpen);
#endif

#if defined(AIR_WIRED_AUDIO_ENABLE) || defined(LINE_IN_USB_IN_EXCLUSIVE_WITH_RECORD_ENABLE)
typedef struct {
    audio_scenario_type_t type;
    bt_sink_srv_am_cb_sub_msg_t sub_msg;
}audio_transmitter_suspend_msg_t;

const static audio_transmitter_suspend_msg_t audio_transmitter_suspend_msg[] = {
    {AUDIO_SCENARIO_TYPE_A2DP, AUD_SUSPEND_BY_A2DP},
    {AUDIO_SCENARIO_TYPE_BLE_UL, AUD_SUSPEND_BY_LE_CALL},
    {AUDIO_SCENARIO_TYPE_HFP_UL, AUD_SUSPEND_BY_HFP},
    {AUDIO_SCENARIO_TYPE_RECORD, AUD_SUSPEND_BY_RECORDER},
    {AUDIO_SCENARIO_TYPE_AUDIO_HW_LOOPBACK_ADC_TO_I2S0, AUD_SUSPEND_BY_HFP},
    {AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT, AUD_SUSPEND_BY_LINE_OUT},
    {AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT, AUD_SUSPEND_BY_USB_OUT},
    {AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN, AUD_SUSPEND_BY_LINE_IN},
    {AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0, AUD_SUSPEND_BY_USB_IN},
};

static void audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list_t *list, uint8_t list_length, audio_scenario_type_t type, bool check_sidetone)
{
    uint16_t scenario_and_id;
    if (true == audio_transmitter_get_is_running_by_scenario_list(list, list_length)) {
        for (uint32_t i = 0; i < list_length; i++) {
            if (true == audio_transmitter_get_is_running_by_scenario_list(&list[i], 1)) {
                scenario_and_id = (list[i].scenario_type << 8) | list[i].scenario_sub_id;
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is running, suspended by type=%d\n", 2, scenario_and_id, type);

                if(check_sidetone) {
                    if ((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)
                        && (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_WAITING_STOP))) {
                        audio_src_srv_report("[Sink][AM][transmitter]current_scenario_and_id = %x is running, stop sidetone by am\n", 1, scenario_and_id);
                        audio_side_tone_disable_hdlr(NULL);
                    }
                }
#ifdef LINE_IN_USB_IN_EXCLUSIVE_WITH_RECORD_ENABLE
                if ((list[i].scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0)
                    || (list[i].scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
                    audio_transmitter_wired_audio_stop_close_usb_in(scenario_and_id);
                } else
#endif
                {
                    if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) &&
                        (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable)) {
#if defined(MTK_LINEIN_PLAYBACK_ENABLE)
                        pure_linein_playback_close();
#endif
                    } else {
                        audio_transmitter_playback_stop(scenario_and_id);
                        audio_transmitter_playback_close(scenario_and_id);
                    }
                }
#if defined (AIR_WIRED_AUDIO_ENABLE)
                if((audio_transmitter_wired_audio_check_mix_stream(list[i].scenario_type, list[i].scenario_sub_id))
                    &&(wired_audio_get_mix_stream_start_number() == 0)) {
                    audio_transmitter_wired_audio_main_stream_stop_close_playback();
                }
#endif
                bt_sink_srv_am_id_t am_id = audio_transmitter_get_am_id_by_scenario(list[i].scenario_type, list[i].scenario_sub_id);
                if (am_id != -1) {
                    if (g_rAm_aud_id[am_id].use != ID_CLOSE_STATE) {
                        g_rAm_aud_id[am_id].use = ID_IDLE_STATE;
                    }
                    if (g_rAm_aud_id[am_id].contain_ptr->notify_cb != NULL) {
                        for (uint8_t i = 0; i < sizeof(audio_transmitter_suspend_msg)/sizeof(audio_transmitter_suspend_msg_t); i++) {
                            if(audio_transmitter_suspend_msg[i].type == type){
                                g_rAm_aud_id[am_id].contain_ptr->notify_cb(am_id, AUD_SUSPEND_BY_IND, audio_transmitter_suspend_msg[i].sub_msg, &scenario_and_id);
                            }
                        }
                        if(i == sizeof(audio_transmitter_suspend_msg)/sizeof(audio_transmitter_suspend_msg_t)){
                            AUDIO_ASSERT(0 && "not find suspend msg id");
                        }
                    }
                } else {
                    AUDIO_ASSERT(0 && "[Sink][AM][transmitter]No am id for the running scenario, abnormal!");
                }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_TRANSMITTER_STOP);
#endif
                break;
            }
        }
    }
}
#endif

/*****************************************************************************
 * FUNCTION
 *  am_audio_set_play
 * DESCRIPTION
 *  This function is used to play the audio handler.
 * PARAMETERS
 *  background_ptr   [IN]
 * RETURNS
 *  void
 *****************************************************************************/

static void am_audio_set_play(bt_sink_srv_am_background_t *background_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_hal_result_t        eResult = HAL_AUDIO_STATUS_ERROR;
    bt_sink_srv_am_id_t                bAud_id = background_ptr->aud_id;
    bt_sink_srv_am_audio_stream_out_t  *stream_out = &(background_ptr->audio_stream_out);
    bt_sink_srv_am_audio_stream_in_t   *stream_in = &(background_ptr->audio_stream_in);
    bt_sink_srv_am_stream_node_t       *pcm_stream_node = NULL;
    bt_sink_srv_am_stream_type_t       eIn_out;
#ifdef __BT_SINK_SRV_AUDIO_TUNING__
    bt_sink_srv_audio_tunning_context_t *aud_tunning_p = NULL;
#endif

#ifdef __BT_AWS_SUPPORT__
    bt_aws_role_t aws_role = BT_AWS_ROLE_NONE;
    bt_aws_codec_type_t aws_codec = 0;
    bt_sink_srv_am_a2dp_codec_t am_a2dp_codec = {{0}};
#endif /* __BT_AWS_SUPPORT__ */

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    audio_src_srv_report("[Sink][AM]Set Play: type:%d\r\n", 1, background_ptr->type);
    if (background_ptr->type != RECORD) {
#if defined (AIR_AUDIO_TRANSMITTER_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        // Temp solution for audio transmitter, the UL cases should bypass the flag setting
        if ((background_ptr->type != AUDIO_TRANSMITTER)&&(background_ptr->type != ULL_BLE_UL)) {
            ami_set_audio_mask(AM_TASK_MASK_DL1_HAPPENING, true);
        }
#else
        ami_set_audio_mask(AM_TASK_MASK_DL1_HAPPENING, true);
#endif
    }

    if (ptr_callback_amm == NULL) {
        ptr_callback_amm = (bt_sink_srv_am_amm_struct *)pvPortMalloc(sizeof(bt_sink_srv_am_amm_struct));
    }
    if (background_ptr->type == NONE) {
        g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
    } else if (background_ptr->type == PCM) {
        pcm_stream_node = &(background_ptr->local_context.pcm_format.stream);
        eIn_out = background_ptr->local_context.pcm_format.in_out;
        if (eIn_out == STREAM_OUT) {
            audio_src_srv_report("[Sink][AM]register_stream_out_callback: 0x%x\r\n", 1, aud_stream_out_callback);
#if defined(MTK_AVM_DIRECT)
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, NULL, true);
#endif
            hal_audio_register_stream_out_callback(aud_stream_out_callback, NULL);
            hal_audio_set_stream_out_sampling_rate(pcm_stream_node->stream_sample_rate);
            hal_audio_set_stream_out_channel_number(pcm_stream_node->stream_channel);
            hal_audio_set_stream_out_device(stream_out->audio_device);

            am_set_PCM_volume(stream_out->audio_device, stream_out->audio_volume);

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
            hal_audio_mute_stream_out(stream_out->audio_mute, HAL_AUDIO_STREAM_OUT1);
#else
            hal_audio_mute_stream_out(stream_out->audio_mute);
#endif
            eResult = hal_audio_write_stream_out(pcm_stream_node->buffer, pcm_stream_node->size);
        } else if (eIn_out == STREAM_IN) {
#ifndef MTK_AM_NOT_SUPPORT_STREAM_IN
#ifdef AIR_RECORD_ENABLE
            audio_src_srv_report("[Sink][AM]register_stream_in_callback: 0x%x\r\n", 1, aud_stream_in_callback);
            hal_audio_register_stream_in_callback(aud_stream_in_callback, NULL);
            hal_audio_set_stream_in_sampling_rate(pcm_stream_node->stream_sample_rate);
            hal_audio_set_stream_in_channel_number(pcm_stream_node->stream_channel);
            hal_audio_set_stream_in_device(stream_in->audio_device);
            /* should add this case ?? */
            #ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            aud_set_volume_level(STREAM_IN, AUD_VOL_AUDIO, stream_in->audio_device, (bt_sink_srv_am_volume_level_t)stream_in->audio_volume);
            #endif
            hal_audio_mute_stream_in(stream_in->audio_mute);
            eResult = hal_audio_read_stream_in(pcm_stream_node->buffer, pcm_stream_node->size);
#endif
#endif
        }
        if (eResult == HAL_AUDIO_STATUS_ERROR) {
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
        } else {
            if (eIn_out == STREAM_OUT) {
                hal_audio_start_stream_out(background_ptr->audio_path_type);
            } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
                hal_audio_start_stream_in(background_ptr->audio_path_type);
#endif
            }
            g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
        }
    } else if ((background_ptr->type == A2DP)
#if defined(MTK_AWS_MCE_ENABLE)
               || (background_ptr->type == AWS)
#endif
              ) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_A2DP_START);
#endif

        am_set_A2DP_volume(stream_out->audio_device, stream_out->audio_volume);

        hal_audio_set_stream_in_device(stream_in->audio_device);
        aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));
        bt_codec_am_a2dp_sink_open(&(background_ptr->local_context.a2dp_format.a2dp_codec));

    } else if (background_ptr->type == BLE) {
#ifdef AIR_BT_CODEC_BLE_ENABLED
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_BLE_START);
#endif
#if defined(MTK_AVM_DIRECT)
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
        if (g_anc_control.enable) {
            g_am_anc_flag = true;
            anc_off(g_anc_control.anc_off_callback, 0, 0);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_ANC_STOP);
#endif
        }
#endif
#endif
#endif

        if (background_ptr->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY) {
            if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_RECORD_STOP);
#endif
#ifdef AIR_RECORD_ENABLE
                am_audio_record_sink_stop();
#else
                //hal_audio_stop_stream_in();
                audio_src_srv_err("[Sink][AM] use record, must enable AIR_RECORD_ENABLE !", 0);
                AUDIO_ASSERT(0);
#endif
                if (g_prCurrent_playback[2] != NULL) {
                    g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SUSPEND_BY_IND, AUD_SUSPEND_BY_LE_CALL, NULL);
                    g_rAm_aud_id[g_prCurrent_playback[2]->aud_id].use = ID_IDLE_STATE;
                    g_prCurrent_playback[2] = NULL;
                    hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_RECORD);
                }
            }
        }
        //Check MIC user with transmitter
#if defined(AIR_WIRED_AUDIO_ENABLE)
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT}
        };
        audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_BLE_UL, false);
#endif
        if ((background_ptr->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY)||((background_ptr->local_context.ble_format.ble_codec.context_type & AUDIO_CONTENT_TYPE_CONVERSATIONAL) != 0)) {
            am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC,  stream_in->audio_device, stream_in->audio_volume,stream_out->audio_device,stream_out->audio_volume, false, true);
        } else {
            am_set_A2DP_volume(stream_out->audio_device, stream_out->audio_volume);
        }

        audio_src_srv_report("LE audio stream_out->audio_mute = %d\r\n", 1, stream_out->audio_mute);

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
        hal_audio_mute_stream_out(stream_out->audio_mute, HAL_AUDIO_STREAM_OUT1);
#else
        hal_audio_mute_stream_out(stream_out->audio_mute);
#endif

        am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, stream_in->audio_device, stream_in->audio_volume,stream_out->audio_device,stream_out->audio_volume, true, false);
#endif
        if ((background_ptr->local_context.ble_format.ble_codec.context_type & AUDIO_CONTENT_TYPE_GAME) != 0){
            audio_src_srv_report("LE audio context type force gaminge 0x%x", 1, background_ptr->local_context.ble_format.ble_codec.context_type);
            background_ptr->local_context.ble_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_GAME;
        }
        else if ((background_ptr->local_context.ble_format.ble_codec.context_type & AUDIO_CONTENT_TYPE_CONVERSATIONAL) != 0)
        {
            audio_src_srv_report("LE audio context type force conversational 0x%x", 1, background_ptr->local_context.ble_format.ble_codec.context_type);
            background_ptr->local_context.ble_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_CONVERSATIONAL;
        }
        if ((background_ptr->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_ONLY)&&(background_ptr->local_context.ble_format.ble_codec.context_type != AUDIO_CONTENT_TYPE_CONVERSATIONAL)) {
            if(background_ptr->local_context.ble_format.ble_codec.context_type != AUDIO_CONTENT_TYPE_GAME){
                background_ptr->local_context.ble_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_MEDIA;
            }
            aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));
        } else if ((background_ptr->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_UL_BOTH)||(background_ptr->local_context.ble_format.ble_codec.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL)) {
            if(background_ptr->local_context.ble_format.ble_codec.context_type != AUDIO_CONTENT_TYPE_GAME){
                background_ptr->local_context.ble_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_CONVERSATIONAL;
            }

            if (background_ptr->local_context.ble_format.ble_codec.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL) {
                DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[3] = {
#if defined(AIR_BT_BLE_FB_ENABLE)
                    FUNC_RX_NR_FB,
#elif defined(AIR_BT_BLE_SWB_ENABLE)
                    FUNC_RX_NR_SWB,
#else
                    FUNC_RX_NR,
#endif
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                    FUNC_SILENCE_DETECTION,
#endif
                    FUNC_END,
                };
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
                if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                    #if defined(AIR_BT_BLE_FB_ENABLE)
                    AudioFeatureList_LEAudioCall[0] = FUNC_FB_BOOM_MIC;
                    #elif defined(AIR_BT_BLE_SWB_ENABLE)
                    AudioFeatureList_LEAudioCall[0] = FUNC_SWB_BOOM_MIC;
                    #else
                    AudioFeatureList_LEAudioCall[0] = FUNC_WB_BOOM_MIC;
                    #endif
                    audio_src_srv_report("[BLE] ble feature mode: boom mic %d", 1, mic_cur_type);
                }
#endif
                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                sysram_status_t status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
                if (status != NVDM_STATUS_NAT_OK) {
                    AUDIO_ASSERT(0 && "LE audio call failed to set parameters to share memory");
                }
            } else {
                aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));
                DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[4] = {
                    FUNC_TX_NR_v2,
#ifndef AIR_BTA_IC_PREMIUM_G2
#ifdef MTK_PEQ_ENABLE
                    FUNC_PEQ_A2DP,
#endif
#endif
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                    FUNC_SILENCE_DETECTION,
#endif
                    FUNC_END,
                };
                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                sysram_status_t status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
                if (status != NVDM_STATUS_NAT_OK) {
                    AUDIO_ASSERT(0 && "LE audio call failed to set parameters to share memory");
                }
            }
            audio_src_srv_report("LE audio call load nvdm ok\r\n", 0);
        } else if (background_ptr->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_UL_ONLY) {
            DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[3] =
            {
#if defined(AIR_BT_BLE_FB_ENABLE)
                FUNC_RX_NR_FB,
#elif defined(AIR_BT_BLE_SWB_ENABLE)
                FUNC_RX_NR_SWB,
#else
                FUNC_RX_NR,
#endif
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                FUNC_SILENCE_DETECTION,
#endif
                FUNC_END,
            };
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
            if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
            #if defined(AIR_BT_BLE_FB_ENABLE)
                AudioFeatureList_LEAudioCall[0] = FUNC_FB_BOOM_MIC;
            #elif defined(AIR_BT_BLE_SWB_ENABLE)
                AudioFeatureList_LEAudioCall[0] = FUNC_SWB_BOOM_MIC;
            #else
                AudioFeatureList_LEAudioCall[0] = FUNC_WB_BOOM_MIC;
             #endif
                audio_src_srv_report("[BLE] ble feature mode: boom mic %d", 1, mic_cur_type);
            }
#endif
            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            sysram_status_t status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
            if (status != NVDM_STATUS_NAT_OK){
                AUDIO_ASSERT(0 && "LE audio call failed to set parameters to share memory");
            }
            audio_src_srv_report("LE audio call only load nvdm ok\r\n", 0);
        }
        bt_codec_am_ble_open(&(background_ptr->local_context.ble_format.ble_codec));
#endif

    }
    else if (background_ptr->type == ULL_BLE_UL) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_ULL_BLE_UL_START);
#endif
#if defined(MTK_AVM_DIRECT)
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
            if (g_anc_control.enable) {
                g_am_anc_flag = true;
                anc_off(g_anc_control.anc_off_callback, 0, 0);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_ANC_STOP);
#endif
            }
#endif
#endif
#endif


            if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD)) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_RECORD_STOP);
#endif
#ifdef AIR_RECORD_ENABLE
                am_audio_record_sink_stop();
#else
                AUDIO_ASSERT(0 && "[Sink][AM] use record, must enable AIR_RECORD_ENABLE !");
#endif
                if (g_prCurrent_playback[2] != NULL) {
                    g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SUSPEND_BY_IND, AUD_SUSPEND_BY_LE_CALL, NULL);
                    g_rAm_aud_id[g_prCurrent_playback[2]->aud_id].use = ID_IDLE_STATE;
                    g_prCurrent_playback[2] = NULL;
                    hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_RECORD);
                }
            }

        am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, stream_in->audio_device, stream_in->audio_volume,stream_out->audio_device,stream_out->audio_volume, true, false);

        #if defined (AIR_DCHS_MODE_ENABLE)
                mcu2dsp_open_param_p open_param;
                open_param = (mcu2dsp_open_param_p)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
                memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

                open_param->stream_in_param.vol_info.codec = BT_HFP_CODEC_TYPE_MSBC;
                open_param->stream_in_param.vol_info.dev_in = stream_in->audio_device;
                open_param->stream_in_param.vol_info.dev_out = stream_out->audio_device;
                open_param->stream_in_param.vol_info.lev_in = stream_in->audio_volume;
                open_param->stream_in_param.vol_info.lev_out = stream_out->audio_volume;
                dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_UL_VOLUME, AUDIO_SCENARIO_TYPE_HFP_UL, open_param, NULL);
                free(open_param);
        #endif
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    #if defined(MTK_PEQ_ENABLE)
            aud_prepare_mic_peq_nvkey();
    #endif
#else
 {
                    DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[2] = {
#if defined(AIR_BT_ULL_SWB_ENABLE)
                        FUNC_RX_NR_SWB,
#else
                        FUNC_RX_NR,
#endif
                        FUNC_END,
                    };
#if defined(AIR_BT_ULL_FB_ENABLE)
                    if (background_ptr->local_context.ull_ble_ul_format.ble_codec.sample_rate == 48000) {
                        AudioFeatureList_LEAudioCall[0] = FUNC_RX_NR_FB;
                        audio_src_srv_report("[BLE][ULL] config FB nvkey", 0);
                    }
#endif

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                    voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
                    if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                    #if defined(AIR_BT_ULL_SWB_ENABLE)
                        AudioFeatureList_LEAudioCall[0] = FUNC_SWB_BOOM_MIC;
                    #else
                        AudioFeatureList_LEAudioCall[0] = FUNC_WB_BOOM_MIC;
                    #endif
                    #if defined(AIR_BT_ULL_FB_ENABLE)
                    if (background_ptr->local_context.ull_ble_ul_format.ble_codec.sample_rate == 48000) {
                        AudioFeatureList_LEAudioCall[0] = FUNC_FB_BOOM_MIC;
                        audio_src_srv_report("[BLE][ULL] config FB Boomic nvkey", 0);
                    }
                    #endif

                        audio_src_srv_report("[BLE][ULL] ble feature mode: boom mic %d", 1, mic_cur_type);
                    }
#endif

                    /* reset share buffer before put parameters*/
                    audio_nvdm_reset_sysram();
                    sysram_status_t status = audio_nvdm_set_feature(2, AudioFeatureList_LEAudioCall);
                    if (status != NVDM_STATUS_NAT_OK) {
                        AUDIO_ASSERT(0 && "ULL LE UL failed to set parameters to share memory");
                    }
                audio_src_srv_report("[BLE][ULL] ULL LE UL load nvdm ok 0x%02x\r\n", 1, AudioFeatureList_LEAudioCall[0]);
            }
#endif
            bt_codec_am_ull_ble_ul_open(background_ptr->local_context.ull_ble_ul_format.ble_codec,background_ptr);

#endif

    }
    else if (background_ptr->type == ULL_BLE_DL) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_ULL_BLE_DL_START);
#endif
#if defined(MTK_AVM_DIRECT)
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
                if (g_anc_control.enable) {
                    g_am_anc_flag = true;
                    anc_off(g_anc_control.anc_off_callback, 0, 0);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_ANC_STOP);
#endif
                }
#endif
#endif
#endif
                am_set_A2DP_volume(stream_out->audio_device, stream_out->audio_volume);

                audio_src_srv_report("LE audio stream_out->audio_mute = %d\r\n", 1, stream_out->audio_mute);

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
                hal_audio_mute_stream_out(stream_out->audio_mute, HAL_AUDIO_STREAM_OUT1);
#else
                hal_audio_mute_stream_out(stream_out->audio_mute);
#endif

#endif

                aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));

                bt_codec_am_ull_ble_dl_open(background_ptr->local_context.ull_ble_dl_format.ble_codec);
#endif

    }
#ifdef AIR_BT_HFP_ENABLE
    else if (background_ptr->type == HFP) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_HFP_START);
#endif
#if defined(MTK_AVM_DIRECT)
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
        if (g_anc_control.enable) {
            g_am_anc_flag = true;
            anc_off(g_anc_control.anc_off_callback, 0, 0);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_ANC_STOP);
#endif
        }
#endif
#endif
#endif
        ami_set_audio_mask(AM_TASK_MASK_UL1_HAPPENING, true);
#ifdef AIR_RECORD_ENABLE
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
#ifdef AIR_AMA_HOTWORD_DURING_CALL_ENABLE
            if (g_prCurrent_playback[2] != NULL) {
                if (g_prCurrent_playback[2]->local_context.record_format.record_codec.codec_cap.wwe_mode != WWE_MODE_AMA) {
#endif
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_RECORD_STOP);
#endif
#ifdef AIR_RECORD_ENABLE
                    am_audio_record_sink_stop();
#else
                    AUDIO_ASSERT(0 && "[Sink][AM] use record, must enable AIR_RECORD_ENABLE !");
#endif
                    if (g_prCurrent_playback[2] != NULL) {
                        g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SUSPEND_BY_IND, AUD_SUSPEND_BY_HFP, NULL);
                        g_rAm_aud_id[g_prCurrent_playback[2]->aud_id].use = ID_IDLE_STATE;
                        g_prCurrent_playback[2] = NULL;
                        hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_RECORD);
                    }
#ifdef AIR_AMA_HOTWORD_DURING_CALL_ENABLE
                } else {
                    audio_src_srv_report("AMA WWE during the call", 0);
                }
            }
#endif
        }
#endif

//Check MIC user with transmitter
#if defined(AIR_WIRED_AUDIO_ENABLE)
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT}
        };
        audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_HFP_UL, false);
#endif

        if ((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) && (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_WAITING_STOP))) {
            uint32_t sidetone_gain;
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            if (audio_nvdm_HW_config.Voice_Sidetone_Gain >> 7) {
                sidetone_gain   = (0xFFFFFF00 | audio_nvdm_HW_config.Voice_Sidetone_Gain) * 100;      /*Default: -100 /100 = -1dB*/
            } else {
                sidetone_gain   = (audio_nvdm_HW_config.Voice_Sidetone_Gain) * 100;
            }
#else
            if (audio_nvdm_HW_config.sidetone_config.SideTone_Gain >> 7) {
                sidetone_gain   = (0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Gain) * 100;      /*Default: -100 /100 = -1dB*/
            } else {
                sidetone_gain   = (audio_nvdm_HW_config.sidetone_config.SideTone_Gain) * 100;
            }
#endif
            sidetone_gain = (g_side_tone_gain_hfp == SIDETONE_GAIN_MAGIC_NUM) ? g_side_tone_gain_hfp : sidetone_gain;
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_SET_VOLUME, 0, sidetone_gain, false);
        }
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
        if (prompt_control_dummy_source_query_state()) {
            audio_src_srv_report("[VP/DmySrc][Sink][AM HFP] HFP enter, stop DJ mode firstly, won't resume.", 0);
            hal_audio_mute_stream_out(true, HAL_AUDIO_STREAM_OUT3); // mute HW GAIN 3 to reduce pop noise
            prompt_control_dummy_source_stop_internal();
        }
#endif /* VP Dummy Source */

        audio_src_srv_report("[Sink][AM HFP] hfp_codec: %d, stream_out_device %d, stream_out_vol %d", 3,
                             background_ptr->local_context.hfp_format.hfp_codec.type,
                             stream_out->audio_device,
                             stream_out->audio_volume);
#endif

        am_set_HFP_volume(background_ptr->local_context.hfp_format.hfp_codec.type, stream_in->audio_device, stream_in->audio_volume,stream_out->audio_device,stream_out->audio_volume, true, true);
#if defined (AIR_DCHS_MODE_ENABLE)
        mcu2dsp_open_param_p open_param;
        open_param = (mcu2dsp_open_param_p)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

        open_param->stream_in_param.vol_info.codec = background_ptr->local_context.hfp_format.hfp_codec.type;
        open_param->stream_in_param.vol_info.dev_in = stream_in->audio_device;
        open_param->stream_in_param.vol_info.dev_out = stream_out->audio_device;
        open_param->stream_in_param.vol_info.lev_in = stream_in->audio_volume;
        open_param->stream_in_param.vol_info.lev_out = stream_out->audio_volume;
        dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_UL_VOLUME, AUDIO_SCENARIO_TYPE_HFP_UL, open_param, NULL);
        free(open_param);
#endif
        bt_codec_am_hfp_open(&(background_ptr->local_context.hfp_format.hfp_codec));

#ifdef AIR_PROMPT_SOUND_ENABLE
        //bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HFP_ON, NULL);
#endif
    }
#endif
#if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == AWS) {

#ifdef __BT_AWS_SUPPORT__
        aws_role = background_ptr->local_context.aws_format.aws_codec.role;
        aws_codec = background_ptr->local_context.aws_format.aws_codec.codec_cap.type;
        /* AWS A2DP */
        if ((aws_role == BT_AWS_ROLE_SINK) &&
            ((aws_codec == BT_AWS_CODEC_TYPE_SBC) || (aws_codec == BT_AWS_CODEC_TYPE_AAC) || (aws_codec == BT_AWS_CODEC_TYPE_VENDOR))) {
            am_set_A2DP_volume(stream_out->audio_device, stream_out->audio_volume);
            hal_audio_set_stream_in_device(stream_in->audio_device);
            am_a2dp_codec.role = BT_A2DP_SINK;
            if (aws_codec == BT_AWS_CODEC_TYPE_SBC) {
                am_a2dp_codec.codec_cap.type = BT_A2DP_CODEC_SBC;
            } else if (aws_codec == BT_AWS_CODEC_TYPE_AAC) {
                am_a2dp_codec.codec_cap.type = BT_A2DP_CODEC_AAC;
            } else if (aws_codec == BT_AWS_CODEC_TYPE_VENDOR) {
                am_a2dp_codec.codec_cap.type = BT_A2DP_CODEC_VENDOR;
            } else {
                AUDIO_ASSERT(0 && ""[Sink][AM][ERROR] Not Support A2DP AWS Codec."");
            }
            am_a2dp_codec.codec_cap.sep_type = BT_A2DP_SINK;
            am_a2dp_codec.codec_cap.length = background_ptr->local_context.aws_format.aws_codec.codec_cap.length;
            if (am_a2dp_codec.codec_cap.length > sizeof(bt_sink_srv_am_codec_t)) {
                am_a2dp_codec.codec_cap.length = sizeof(bt_sink_srv_am_codec_t);
                audio_src_srv_report("A2DP codec length error, please check.", 0);
            }
            memcpy(&(am_a2dp_codec.codec_cap.codec),
                   &(background_ptr->local_context.aws_format.aws_codec.codec_cap.codec),
                   am_a2dp_codec.codec_cap.length);

            aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));
            bt_codec_am_a2dp_sink_open(&am_a2dp_codec);
        } else if ((aws_role == BT_AWS_ROLE_SINK) &&
                   (aws_codec == BT_AWS_CODEC_TYPE_MP3)) {
            bt_sink_srv_am_files_format_t file_fmt;
            file_fmt.file_type = FILE_MP3;
            am_files_codec_open(&file_fmt); // need to adjust yiquan
            aud_set_volume_level(STREAM_OUT, AUD_VOL_AUDIO, stream_out->audio_device, (bt_sink_srv_am_volume_level_t)stream_out->audio_volume);
            hal_audio_set_stream_in_device(stream_in->audio_device);
            hal_audio_set_stream_out_device(stream_out->audio_device);
        }
#endif
    }
#endif //#if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == FILES) {
#if defined(MTK_AVM_DIRECT)
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, NULL, true);
#endif
        am_files_codec_open(&(background_ptr->local_context.files_format));
        hal_audio_set_stream_out_device(stream_out->audio_device);
        am_set_MP3_volume(stream_out->audio_device, stream_out->audio_volume);

    } else if (background_ptr->type == RECORD) {
#ifdef AIR_RECORD_ENABLE
#ifndef AIR_AMA_HOTWORD_DURING_CALL_ENABLE
        if (g_prHfp_media_handle == NULL) {
#else
        if (((g_prHfp_media_handle == NULL) && (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL) != true)) || (background_ptr->local_context.record_format.record_codec.codec_cap.wwe_mode == WWE_MODE_AMA)) {
#endif
#ifdef MTK_LINE_IN_ENABLE
            if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN)) {
                audio_src_srv_report("[Sink][AM][PLAY] Line-in stopped by record.", 0);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
#ifdef MTK_VENDOR_SOUND_EFFECT_EXTENSION_ENABLE
                ami_execute_vendor_se(EVENT_BEFORE_LINEINPLAYBACK_STOP);
#endif
#endif
                audio_linein_playback_stop();
                aud_dl_control(false);
                audio_linein_playback_close();
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                ami_execute_vendor_se(EVENT_LINEINPLAYBACK_SUSPEND);
                ami_execute_vendor_se(EVENT_LINEINPLAYBACK_STOP);
#endif
            }
#endif

#ifdef AIR_WIRED_AUDIO_ENABLE
            audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER},
#ifdef LINE_IN_USB_IN_EXCLUSIVE_WITH_RECORD_ENABLE
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1},
#endif
            };

            /* For line-in/USB-in/line-out/usb-out, will be suspended by record */
#ifdef LINE_IN_USB_IN_EXCLUSIVE_WITH_RECORD_ENABLE
            audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 8, AUDIO_SCENARIO_TYPE_RECORD, false);
#else
            audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_RECORD, false);
#endif
#endif
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_RECORD_START);
#endif
            ami_set_audio_mask(AM_TASK_MASK_UL1_HAPPENING, true);
            am_set_VC_volume();

            g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
            if (background_ptr->local_context.record_format.Reserve_callback != NULL) {
                hal_audio_register_stream_in_callback(background_ptr->local_context.record_format.Reserve_callback, background_ptr->local_context.record_format.Reserve_callback_user_data);
            } else {
                audio_src_srv_report("[Sink][AM][ERROR] Record CCNI NULL", 0);
            }
            am_audio_record_sink_play(background_ptr);
        } else {
            /*Notify Record user HFP is happening, the request of open record resource will be rejected.*/
        }
#endif
    } else if (background_ptr->type == LINE_IN) {
#ifdef MTK_LINE_IN_ENABLE
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_LINEINPLAYBACK_REJECT);
#endif
            audio_src_srv_report("[Sink][AM][PLAY] Line-in rejected by record.", 0);
            g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;//ToDO
        } else {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_LINEINPLAYBACK_START);
#endif
#if defined(MTK_AVM_DIRECT)
            if (background_ptr->local_context.line_in_format.line_in_codec.codec_cap.out_audio_device & HAL_AUDIO_DEVICE_DAC_DUAL) {
                audio_message_audio_handler(HAL_AUDIO_EVENT_DATA_REQUEST, NULL);
            }
#endif
            am_set_LINE_IN_volume(VOL_LINE_IN, stream_in->audio_device, stream_in->audio_volume,stream_out->audio_device,stream_out->audio_volume, true, true);
            audio_src_srv_report("[Sink][AM][PLAY] Line-in play.", 0);

#if defined(MTK_LINEIN_PEQ_ENABLE) || defined(MTK_LINEIN_INS_ENABLE)
            aud_prepare_linein_nvkey(&(background_ptr->local_feature),0);
#endif
            audio_linein_playback_open(background_ptr->local_context.line_in_format.line_in_codec.codec_cap.linein_sample_rate,
                                       background_ptr->local_context.line_in_format.line_in_codec.codec_cap.in_audio_device,
                                       background_ptr->local_context.line_in_format.line_in_codec.codec_cap.out_audio_device);
            g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, NULL);
#if 0
            g_prLineIN_sink_handle = (audio_sink_srv_am_line_in_codec_t *)pvPortMalloc(sizeof(audio_sink_srv_am_line_in_codec_t));
            if (g_prLineIN_sink_handle == NULL) {
                audio_src_srv_report("[Sink][AM][PLAY] Line-in play: allocate fail\r\n", 0);
            }
            memcpy(&(g_prLineIN_sink_handle), &(background_ptr->local_context.line_in_format.line_in_codec), sizeof(audio_sink_srv_am_line_in_codec_t));
#else
            g_prLineIN_sink_handle.codec_cap.linein_sample_rate = background_ptr->local_context.line_in_format.line_in_codec.codec_cap.linein_sample_rate;
            g_prLineIN_sink_handle.codec_cap.in_audio_device    = background_ptr->local_context.line_in_format.line_in_codec.codec_cap.in_audio_device;
            g_prLineIN_sink_handle.codec_cap.out_audio_device   = background_ptr->local_context.line_in_format.line_in_codec.codec_cap.out_audio_device;
#endif
            aud_dl_control(true);
            audio_linein_playback_start();
            g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;//ToDO
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
#ifdef MTK_VENDOR_SOUND_EFFECT_EXTENSION_ENABLE
            ami_execute_vendor_se(EVENT_AFTER_LINEINPLAYBACK_START);
#endif
#endif
        }
#endif
    } else if (background_ptr->type == USB_AUDIO_IN) {
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_USB_AUDIO_START);
#endif
        audio_src_srv_report("[Sink][AM][PLAY]USB_AUDIO play.", 0);

        am_set_USB_AUDIO_IN_volume(VOL_USB_AUDIO_IN, stream_out->audio_device, stream_out->audio_volume);

        aud_prepare_a2dp_nvkey(&(background_ptr->local_feature));

        audio_usb_audio_playback_open();
        aud_dl_control(true);
        audio_usb_audio_playback_start();
        g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;//ToDO
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, NULL);
#endif
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    } else if (background_ptr->type == AUDIO_TRANSMITTER) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_TRANSMITTER_START);
#endif
        uint16_t scenario_and_id = ((background_ptr->local_context.audio_transmitter_format.scenario_type) << 8) + background_ptr->local_context.audio_transmitter_format.scenario_sub_id;
        audio_src_srv_report("[Sink][AM][transmitter]playback_open/start scenario_and_id = %x", 1, scenario_and_id);

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
        if (background_ptr->local_context.audio_transmitter_format.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
                AUDIO_ASSERT(0 && "record is running when play AUDIO_TRANSMITTER_GAMING_MODE.");
            }
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_UL) || ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_DL)) {
                AUDIO_ASSERT(0 && "HFP is running when play AUDIO_TRANSMITTER_GAMING_MODE.");
            }
        }
#endif
#if defined(AIR_WIRED_AUDIO_ENABLE)
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER},//workaround
        };
        /*
        *  1. Line-out/USB-out exclusive with recorder, the recorder has a higher priority
        *  2. Line-out/USB-out exclusive with HFP, the HFP has a higher priority
        *  3. Line-out/USB-out exclusive with each other, the later one has a higher priority
        *  4. Line-out/USB-out exclusive with A2DP
        *  5. Dual chip workaround: HW loopback MIC has a higher priority than Line out
        */
        //Workaround for dual chip HFP/Line-out cohenrency case!!
        if (scenario_and_id == ((AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK << 8) + AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_ADC_TO_I2S0)) {

            /* For line-out/USB-out, the later one has a higher priority than the former one */
            audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_AUDIO_HW_LOOPBACK_ADC_TO_I2S0, true);
            /*
            *  1. Line-in/USB-in exclusive with recorder, the recorder has a higher priority
            *  2. Line-in/USB-in exvlusive with each other, the later one has a higher priority
            */
        } else if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT))) {
            /* recorder has a higher priority than Line-out/USB-out */
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is rejected by record\n", 1, scenario_and_id);
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_SUSPEND_BY_RECORDER, &scenario_and_id);
                return;
            }
            /* HFP has a higher priority than Line-out/USB-out */
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_UL) || ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_DL)) {
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is rejected by HFP\n", 1, scenario_and_id);
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_SUSPEND_BY_HFP, &scenario_and_id);
                return;
            }
            /* LE CALL has a higher priority than Line-out/USB-out */
            audio_src_srv_report("[Sink][AM][transmitter]get le audio running flag %d %d\n", 2, ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL), ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL));
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL) || ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL)) {
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is rejected by LE_CALL\n", 1, scenario_and_id);
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_SUSPEND_BY_LE_CALL, &scenario_and_id);
                return;
            }
            /* For line-out/USB-out, the later one has a higher priority than the former one */
            if(((scenario_and_id & 0xFF) == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) || ((scenario_and_id & 0xFF) == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
                audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT, false);
                    } else {
                audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list, 3, AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT, false);
            }

            /* Line-out/USB-out exclusive with A2DP */
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_A2DP) == true) {
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is rejected by A2DP\n", 1, scenario_and_id);
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_SUSPEND_BY_A2DP, &scenario_and_id);
                return;
            }

            //set USB_OUT LINE_OUT default volume
            am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);

            /*
            *  1. Line-in/USB-in exclusive with recorder, the recorder has a higher priority
            *  2. Line-in/USB-in exvlusive with each other, the later one has a higher priority
            */
        } else if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM)) || \
                   (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1))) {
            #ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            if(scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM)){
                wired_audio_add_mix_stream_start_number();
            }
            #endif

            audio_transmitter_scenario_list_t audio_transmitter_scenario_list_usb_in[]  =  {
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1},
            };

            audio_transmitter_scenario_list_t audio_transmitter_scenario_list_line_in[]  =  {
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE}
            };
#ifdef LINE_IN_USB_IN_EXCLUSIVE_WITH_RECORD_ENABLE
            /* recorder has a higher priority than Line-in/USB-in */
            if (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
                audio_src_srv_report("[Sink][AM][transmitter]scenario_and_id = %x is rejected by record\n", 1, scenario_and_id);
                //AUDIO_ASSERT(0);
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_SUSPEND_BY_RECORDER, &scenario_and_id);
                return;
            }
#endif
            /* For line-in/USB-in, the later one has a higher priority than the former one */
            /* Line-in preempt USB-in */
            if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) || \
                (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) || \
                (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE))) {
                audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list_usb_in, 2, AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN, false);

                /* If line in bypass mode is selected, just call pure line in flow */
                if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) &&
                        (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable)) {
#if defined(MTK_LINEIN_PLAYBACK_ENABLE)
                    hal_audio_device_t in_audio_device, out_audio_device;
                    mcu2dsp_open_stream_in_param_t *p_open_stream_in_param;
                    mcu2dsp_open_stream_out_param_t *p_open_stream_out_param;
                    p_open_stream_in_param = (mcu2dsp_open_stream_in_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
                    AUDIO_ASSERT(p_open_stream_in_param != NULL);
                    hal_audio_get_stream_in_setting_config(AU_DSP_LINEIN, p_open_stream_in_param);
                    in_audio_device = p_open_stream_in_param->afe.audio_device;
                    vPortFree(p_open_stream_in_param);
                    p_open_stream_out_param = (mcu2dsp_open_stream_out_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_out_param_t));
                    AUDIO_ASSERT(p_open_stream_out_param != NULL);
                    hal_audio_get_stream_out_setting_config(AU_DSP_LINEIN, p_open_stream_out_param);
                    out_audio_device = p_open_stream_out_param->afe.audio_device;
                    vPortFree(p_open_stream_out_param);
                    pure_linein_playback_open(HAL_AUDIO_SAMPLING_RATE_48KHZ, in_audio_device, out_audio_device);
#endif
                    g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
                    background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_COMPLETE, &scenario_and_id);
                    return;
                }

                /* Line-in pre-setting before play */
#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
                if (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) {
                    am_set_LINE_IN_volume(VOL_LINE_IN, AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE,AUD_VOL_IN_LEVEL0,AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE,AUD_VOL_OUT_LEVEL0, true, true);
                } else {
                    am_set_LINE_IN_volume(VOL_LINE_IN_DL3, AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE,AUD_VOL_IN_LEVEL0,AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE,AUD_VOL_OUT_LEVEL0, true, true);
                }
#else
                am_set_LINE_IN_volume(VOL_LINE_IN_DL3, AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE,AUD_VOL_IN_LEVEL0,AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE,AUD_VOL_OUT_LEVEL0, true, true);
#endif
#if defined(MTK_LINE_IN_ENABLE) || (defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
#if defined(MTK_LINEIN_PEQ_ENABLE) || defined(MTK_LINEIN_INS_ENABLE)
                aud_prepare_linein_nvkey(&(background_ptr->local_feature),0);
#endif
#endif
            } else if (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM)) {
                //reuse usb in level
                bt_sink_srv_audio_setting_vol_info_t vol_info;
                vol_info.type = VOL_USB_AUDIO_IN_DL3;
                vol_info.vol_info.usb_audio_vol_info.dev = HAL_AUDIO_DEVICE_DAC_DUAL;
                vol_info.vol_info.usb_audio_vol_info.lev = audio_get_max_sound_level_out(VOL_USB_AUDIO_IN);
                bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
            } else {
                /* USB-in preempt Line-in */
                audio_transmitter_suspend_by_other_scenario(audio_transmitter_scenario_list_line_in, 3, AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0, false);

                /* USB-in pre-setting before play */
                wired_audio_add_mix_stream_start_number();
                // wired_audio_add_usb_audio_start_number();
                // if (wired_audio_get_usb_audio_start_number() == 1) {
                //     wired_audio_set_usb_use_afe_subid(background_ptr->local_context.audio_transmitter_format.scenario_sub_id);
                // } else if (wired_audio_get_usb_audio_start_number() == 2) {
                //     if (wired_audio_get_usb_use_afe_subid() == background_ptr->local_context.audio_transmitter_format.scenario_sub_id) {
                //         g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
                //         //bypass
                //         //audio_transmitter_playback_open(scenario_and_id);
                //         //audio_transmitter_playback_start(scenario_and_id);
                //         background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_COMPLETE, &scenario_and_id);
                //         return;
                //     }
                // } else if (wired_audio_get_usb_audio_start_number() > 2) {
                //     AUDIO_ASSERT(0 && "too much wire usb audio set play");
                // }

                //reuse line in level
                am_set_USB_AUDIO_IN_volume(VOL_USB_AUDIO_IN_DL3, HAL_AUDIO_DEVICE_DAC_DUAL, audio_get_max_sound_level_out(VOL_USB_AUDIO_IN));
#ifdef MTK_LINE_IN_ENABLE
#if defined(MTK_LINEIN_PEQ_ENABLE) || defined(MTK_LINEIN_INS_ENABLE)
                // if (wired_audio_get_usb_audio_start_number() <= 2) {
                    aud_prepare_linein_nvkey(&(background_ptr->local_feature),background_ptr->local_context.audio_transmitter_format.scenario_sub_id);
                // }
#endif
#endif
            }
        }

#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
        if ((background_ptr->local_context.audio_transmitter_format.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE)
            && (background_ptr->local_context.audio_transmitter_format.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET)) {
            am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);
        }
#endif

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
        if ((background_ptr->local_context.audio_transmitter_format.scenario_type == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH)
            && (background_ptr->local_context.audio_transmitter_format.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH_HEARING_AID)) {
            am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);
            aud_prepare_hearing_aid_nvkey(&(background_ptr->local_feature));
        }
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#ifdef AIR_WIRED_AUDIO_ENABLE
        if ((background_ptr->local_context.audio_transmitter_format.scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO)
            && (background_ptr->local_context.audio_transmitter_format.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT)) {
#if defined(MTK_PEQ_ENABLE)
            aud_prepare_mic_peq_nvkey();
#endif
        }
#endif
#ifdef AIR_RECORD_ADVANCED_ENABLE
        if ((background_ptr->local_context.audio_transmitter_format.scenario_type == AUDIO_TRANSMITTER_ADVANCED_RECORD)
            && (background_ptr->local_context.audio_transmitter_format.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC)) {
#if defined(MTK_PEQ_ENABLE)
            aud_prepare_advanced_record_nvkey();
#endif
        }
#endif
#endif /* AIR_WIRELESS_MIC_TX_ENABLE */


        /* FOR STREAM MIX */
#if defined (AIR_WIRED_AUDIO_ENABLE)
        if (audio_transmitter_wired_audio_check_mix_stream(background_ptr->local_context.audio_transmitter_format.scenario_type,background_ptr->local_context.audio_transmitter_format.scenario_sub_id)
             && (wired_audio_get_mix_stream_start_number() == 1)) {
            bt_sink_srv_audio_setting_vol_info_t vol_info;
            vol_info.type = VOL_USB_AUDIO_IN_DL3;
            vol_info.vol_info.usb_audio_vol_info.dev = HAL_AUDIO_DEVICE_DAC_DUAL;
            vol_info.vol_info.usb_audio_vol_info.lev = audio_get_max_sound_level_out(VOL_USB_AUDIO_IN);
            bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
            audio_transmitter_wired_audio_main_stream_open_start_playback();
        }
#endif
        g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;

        audio_transmitter_playback_open(scenario_and_id);
        audio_transmitter_playback_start(scenario_and_id);
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_COMPLETE, &scenario_and_id);

#endif
    } else {
        /* trace error */
    }
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_set_suspend
 * DESCRIPTION
 *  This function set to SUSPEND state when be interrupt by others.
 * PARAMETERS
 *  lead             [IN]
 *  background_ptr   [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void am_audio_set_suspend(bt_sink_srv_am_type_t lead, bt_sink_srv_am_background_t *background_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_cb_sub_msg_t   msg_id = AUD_SUSPEND_BY_NONE;
    bt_sink_srv_am_id_t             bAud_id = background_ptr->aud_id;
    bt_sink_srv_am_stream_type_t    eIn_out;
    void *parm = NULL;
#ifdef __BT_AWS_SUPPORT__
    bt_aws_role_t aws_role = BT_AWS_ROLE_NONE;
    bt_aws_codec_type_t aws_codec = 0;
#endif /* __BT_AWS_SUPPORT__ */

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    switch (lead) {
        case NONE:
            msg_id = AUD_SUSPEND_BY_NONE;
            break;
        case PCM:
            msg_id = AUD_SUSPEND_BY_PCM;
            break;
        case A2DP:
            msg_id = AUD_SUSPEND_BY_A2DP;
            break;
        case HFP:
            msg_id = AUD_SUSPEND_BY_HFP;
            break;
        case FILES:
            msg_id = AUD_SUSPEND_BY_FILES;
            break;
        case AWS:
            msg_id = AUD_SUSPEND_BY_AWS;
            break;
        default:
            break;
    }

    if (background_ptr->type == NONE) {

    } else if (background_ptr->type == PCM) {
        eIn_out = background_ptr->local_context.pcm_format.in_out;
        if (eIn_out == STREAM_OUT) {
            hal_audio_stop_stream_out();
        } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
            hal_audio_stop_stream_in();
#endif
        }
    } else if ((background_ptr->type == A2DP)
#if defined(MTK_AWS_MCE_ENABLE)
               || (background_ptr->type == AWS)
#endif
              ) {
        if (g_rSink_state == A2DP_SINK_CODEC_OPEN) {
            if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                audio_src_srv_report("[Sink][AM][ERROR] Suspend A2DP", 0);
#endif
                return;
            }
        } else if (g_rSink_state == A2DP_SINK_CODEC_PLAY) {
            if ((bt_codec_am_a2dp_sink_stop(background_ptr->aud_id) == BT_CODEC_MEDIA_STATUS_ERROR) ||
                (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR)) {
#ifdef __AM_DEBUG_INFO__
                audio_src_srv_report("[Sink][AM][ERROR] Suspend A2DP", 0);
#endif
                return;
            }
        }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_A2DP_STOP);
#endif
    }
#ifdef AIR_BT_HFP_ENABLE
    else if (background_ptr->type == HFP) {
        if (bt_codec_am_hfp_stop() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Suspend HFP", 0);
#endif
            return;
        }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_HFP_STOP);
#endif
    }
#endif
#if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == AWS) {
#ifdef __BT_AWS_SUPPORT__
        aws_role = background_ptr->local_context.aws_format.aws_codec.role;
        aws_codec = background_ptr->local_context.aws_format.aws_codec.codec_cap.type;
        /* AWS A2DP */
        if ((aws_role == BT_AWS_ROLE_SINK) &&
            ((aws_codec == BT_AWS_CODEC_TYPE_SBC) || (aws_codec == BT_AWS_CODEC_TYPE_AAC) || (aws_codec == BT_AWS_CODEC_TYPE_VENDOR))) {
            if (g_rSink_state == A2DP_SINK_CODEC_OPEN) {
                if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                    audio_src_srv_report("[Sink][AM][ERROR][AWS]Suspend A2DP 1", 0);
#endif
                    return;
                }
            } else if (g_rSink_state == A2DP_SINK_CODEC_PLAY) {
                if ((bt_codec_am_a2dp_sink_stop(background_ptr->aud_id) == BT_CODEC_MEDIA_STATUS_ERROR) ||
                    (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR)) {
#ifdef __AM_DEBUG_INFO__
                    audio_src_srv_report("[Sink][AM][ERROR][AWS]Suspend A2DP 2", 0);
#endif
                    return;
                }
            }
        } else if ((aws_role == BT_AWS_ROLE_SINK) && (aws_codec == BT_AWS_CODEC_TYPE_MP3)) {
            am_files_codec_close();
        }
#endif /* __BT_AWS_SUPPORT__ */
    }
#endif // #if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == FILES) {
        am_files_codec_close();
    }

    g_rAm_aud_id[bAud_id].use = ID_SUSPEND_STATE;
    if (AUD_SUSPEND_BY_NONE == msg_id || AUD_SUSPEND_BY_HFP == msg_id) {
        parm = (void *) &g_int_dev_addr;
    }

    background_ptr->notify_cb(background_ptr->aud_id, AUD_SUSPEND_BY_IND, msg_id, parm);
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_search_suspended
 * DESCRIPTION
 *  This function is used to find the highest suspend event to resume it when no player.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void am_audio_search_suspended(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *recoder_current_ptr = NULL;
    bt_sink_srv_am_id_t bAud_id;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    recoder_current_ptr = g_rBackground_head.next;
    while (recoder_current_ptr != NULL) {
        bAud_id = recoder_current_ptr->aud_id;
        if (g_rAm_aud_id[bAud_id].use == ID_SUSPEND_STATE) {
            g_rAm_aud_id[bAud_id].use = ID_RESUME_STATE;
            g_prCurrent_resumer = recoder_current_ptr;
#ifdef RTOS_TIMER
            xTimerStart(g_xTimer_am, 0);
#endif
            recoder_current_ptr->notify_cb(recoder_current_ptr->aud_id, AUD_RESUME_IND, AUD_RESUME_IDLE_STATE, NULL);
            break;
        }
        recoder_current_ptr = recoder_current_ptr->next;
    }
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_set_stop
 * DESCRIPTION
 *  This function is used to stop audio handler.
 * PARAMETERS
 *  background_ptr   [IN]
 * RETURNS
 *  void
 *****************************************************************************/
#ifdef AIR_USB_AUDIO_OUT_ENABLE
void default_app_usb_out_resume_by_music(void)
{
    return;
}
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:app_usb_out_resume_by_music=default_app_usb_out_resume_by_music")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
void app_usb_out_resume_by_music(void);
#pragma weak app_usb_out_resume_by_music = default_app_usb_out_resume_by_music
#else
#error "Unsupported Platform"
#endif
#endif

#ifdef AIR_LINE_OUT_ENABLE
void default_app_line_out_resume_by_music(void)
{
    return;
}
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:app_line_out_resume_by_music=default_app_line_out_resume_by_music")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
void app_line_out_resume_by_music(void);
#pragma weak app_line_out_resume_by_music = default_app_line_out_resume_by_music
#else
#error "Unsupported Platform"
#endif
#endif


static void am_audio_set_stop(bt_sink_srv_am_background_t *background_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_stream_type_t    eIn_out;
#ifdef __BT_AWS_SUPPORT__
    bt_aws_role_t aws_role = BT_AWS_ROLE_NONE;
    bt_aws_codec_type_t aws_codec = 0;
#endif /* __BT_AWS_SUPPORT__ */

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (background_ptr->type == NONE) {
    } else if (background_ptr->type == PCM) {
        eIn_out = background_ptr->local_context.pcm_format.in_out;
        if (eIn_out == STREAM_OUT) {
            hal_audio_stop_stream_out();
#if defined(MTK_AVM_DIRECT)
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, NULL, false);
#endif
        } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
            hal_audio_stop_stream_in();
#endif
        }
    } else if ((background_ptr->type == A2DP)
#if defined(MTK_AWS_MCE_ENABLE)
               || (background_ptr->type == AWS)
#endif
              ) {
        audio_src_srv_report("[Sink][AM]A2DP sink state: %d", 1, g_rSink_state);
        if (g_rSink_state == A2DP_SINK_CODEC_STOP) {
            if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                audio_src_srv_report("[Sink][AM][ERROR] A2DP codec close fail", 0);
#endif
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                return;
            }
        } else if (g_rSink_state == A2DP_SINK_CODEC_OPEN) {
            if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                audio_src_srv_report("[Sink][AM][ERROR] A2DP codec close fail", 0);
#endif
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                return;
            }
        } else if (g_rSink_state != A2DP_SINK_CODEC_CLOSE) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] A2DP codec NOT CLOSE", 0);
#endif
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
            return;
        }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_A2DP_STOP);
#endif
#ifdef AIR_USB_AUDIO_OUT_ENABLE
        //extern void app_usb_out_resume_by_music(void);
        app_usb_out_resume_by_music();
#endif
#ifdef AIR_LINE_OUT_ENABLE
        //extern void app_line_out_resume_by_music(void);
        app_line_out_resume_by_music();
#endif
    } else if (background_ptr->type == BLE) {
#ifdef AIR_BT_CODEC_BLE_ENABLED
        if (bt_codec_am_ble_stop() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Close BLE", 0);
#endif
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
            return;
        }

#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_BLE_STOP);
#endif
#endif //#ifdef AIR_BT_CODEC_BLE_ENABLED
    }
#ifdef AIR_BT_HFP_ENABLE
    else if (background_ptr->type == HFP) {
        if (bt_codec_am_hfp_stop() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Close HFP", 0);
#endif
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
            return;
        }
#ifdef AIR_PROMPT_SOUND_ENABLE
        //bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HFP_OFF, NULL);
#endif
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_HFP_STOP);
#endif
    }
#endif
#if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == AWS) {
#ifdef __BT_AWS_SUPPORT__
        aws_role = background_ptr->local_context.aws_format.aws_codec.role;
        aws_codec = background_ptr->local_context.aws_format.aws_codec.codec_cap.type;
        /* AWS A2DP */
        if ((aws_role == BT_AWS_ROLE_SINK) &&
            ((aws_codec == BT_AWS_CODEC_TYPE_SBC) || (aws_codec == BT_AWS_CODEC_TYPE_AAC) || (aws_codec == BT_AWS_CODEC_TYPE_VENDOR))) {
            audio_src_srv_report("[Sink][AM][AWS]A2DP sink state: %d", 1, g_rSink_state);
            if (g_rSink_state == A2DP_SINK_CODEC_STOP) {
                if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                    audio_src_srv_report("[Sink][AM][ERROR][AWS]A2DP codec close fail", 0);
#endif
                    background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                    return;
                }
            } else if (g_rSink_state == A2DP_SINK_CODEC_OPEN) {
                if (bt_codec_am_a2dp_sink_close() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
                    audio_src_srv_report("[Sink][AM][ERROR][AWS]A2DP codec close fail", 0);
#endif
                    background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                    return;
                }
            } else if (g_rSink_state != A2DP_SINK_CODEC_CLOSE) {
#ifdef __AM_DEBUG_INFO__
                audio_src_srv_report("[Sink][AM][ERROR][AWS]A2DP codec NOT CLOSE", 0);
#endif
                background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                return;
            }
        }
#if defined(MTK_AVM_DIRECT)
        ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_BT_AUDIO_DL, false);
#endif
#endif /* __BT_AWS_SUPPORT__ */
    }
#endif // #if !defined(MTK_AWS_MCE_ENABLE)
    else if (background_ptr->type == FILES) {
        am_files_codec_close();
#if defined(MTK_AVM_DIRECT)
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, NULL, false);
#endif
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_FILE_EVENT_DATA_END, NULL);
    } else if (background_ptr->type == RECORD) {
#ifdef AIR_RECORD_ENABLE
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
            am_audio_record_sink_stop();
            if (background_ptr->local_context.record_format.Reserve_callback != NULL) {
                hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_RECORD);
            }
            ami_set_audio_mask(AM_TASK_MASK_UL1_HAPPENING, false);
            if (g_prCurrent_playback[2] != NULL) {
                g_prCurrent_playback[2] = NULL;
            }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_RECORD_STOP);
#endif
        } else {
            audio_src_srv_report("[Sink][AM][RECORD] record is stopped by other scenarios already.", 0);
        }
#endif /*AIR_RECORD_ENABLE*/
    } else if (background_ptr->type == LINE_IN) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
#ifdef MTK_VENDOR_SOUND_EFFECT_EXTENSION_ENABLE
        ami_execute_vendor_se(EVENT_BEFORE_LINEINPLAYBACK_STOP);
#endif
#endif
#ifdef MTK_LINE_IN_ENABLE
#if defined(MTK_AVM_DIRECT)
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN)) {
            //audio_src_srv_report("[Sink][AM][PLAY] Line-in stop.", 0);
            audio_linein_playback_stop();
            aud_dl_control(false);
            audio_linein_playback_close();
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_LINEINPLAYBACK_STOP);
#endif
        } else {
            //audio_src_srv_report("[Sink][AM][PLAY] Line-in stopped already.", 0);
        }
        audio_src_srv_report("[Sink][AM][PLAY] Line-in stopped already (%d).", 1, !hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN));
#endif
#endif
    } else if (background_ptr->type == USB_AUDIO_IN) {
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
        audio_src_srv_report("[Sink][AM][PLAY]USB_AUDIO stop.", 0);
        audio_usb_audio_playback_stop();
        aud_dl_control(false);
        audio_usb_audio_playback_close();
#endif
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_USB_AUDIO_STOP);
#endif
    }
#if defined (AIR_AUDIO_TRANSMITTER_ENABLE)
    else if (background_ptr->type == AUDIO_TRANSMITTER) {
        uint16_t scenario_and_id = ((background_ptr->local_context.audio_transmitter_format.scenario_type) << 8) + background_ptr->local_context.audio_transmitter_format.scenario_sub_id;
        audio_src_srv_report("[Sink][AM][transmitter]playback_stop/close scenario_and_id = %x", 1, scenario_and_id);
#if defined(AIR_WIRED_AUDIO_ENABLE)
        if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0))
            || (scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1))) {
            audio_transmitter_wired_audio_stop_close_usb_in(scenario_and_id);
        } else if ((scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)) &&
                        (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable)) {
#if defined(MTK_LINEIN_PLAYBACK_ENABLE)
            pure_linein_playback_close();
#endif
        } else
#endif /* AIR_WIRED_AUDIO_ENABLE */
        {
            #ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            if(scenario_and_id == ((AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM)){
                wired_audio_minus_mix_stream_start_number();
            }
            #endif
            audio_transmitter_playback_stop(scenario_and_id);
            audio_transmitter_playback_close(scenario_and_id);
        }
#if defined (AIR_WIRED_AUDIO_ENABLE)
        if((audio_transmitter_wired_audio_check_mix_stream(background_ptr->local_context.audio_transmitter_format.scenario_type,background_ptr->local_context.audio_transmitter_format.scenario_sub_id))
            &&(wired_audio_get_mix_stream_start_number() == 0)) {
            audio_transmitter_wired_audio_main_stream_stop_close_playback();
        }
#endif
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, &scenario_and_id);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_TRANSMITTER_STOP);
#endif
    }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    else if (background_ptr->type == ULL_BLE_UL) {
        if (bt_codec_am_ull_ble_ul_stop() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Close ULL BLE UL", 0);
#endif
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
            return;
        }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_ULL_BLE_UL_STOP);
#endif
    }
    else if (background_ptr->type == ULL_BLE_DL) {
            if (bt_codec_am_ull_ble_dl_stop() == BT_CODEC_MEDIA_STATUS_ERROR) {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Close ULL BLE DL", 0);
#endif
            background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
            return;
        }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_ULL_BLE_DL_STOP);
#endif
    }
#endif
    if (g_rAm_aud_id[background_ptr->aud_id].use != ID_CLOSE_STATE) {
        g_rAm_aud_id[background_ptr->aud_id].use = ID_IDLE_STATE;
    }
    background_ptr->notify_cb(background_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);

#if defined(MTK_AVM_DIRECT)
    if (background_ptr->type != RECORD) {
#if defined (AIR_AUDIO_TRANSMITTER_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        // Temp solution for audio transmitter, the UL cases should bypass the flag setting
        if ((background_ptr->type == AUDIO_TRANSMITTER)||(background_ptr->type == ULL_BLE_UL)) {
            return;
        }
#endif
        ami_set_audio_mask(AM_TASK_MASK_DL1_HAPPENING, false);
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
        ami_set_afe_param(STREAM_OUT, 0, false);
#endif
    }
#endif
}

/*****************************************************************************
 * FUNCTION
 *  aud_initial
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_sink_srv_am_hal_result_t
 *****************************************************************************/
static void aud_initial(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    memset(&g_rAm_aud_id[0], 0, sizeof(bt_sink_srv_am_aud_id_type_t)*AM_REGISTER_ID_TOTAL);
    memset(&g_prA2dp_sink_event_handle, 0, sizeof(g_prA2dp_sink_event_handle));

    hal_audio_set_sysram();
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    if(g_peq_init_flag == 0){
        aud_peq_init();
    }
#endif
    am_audio_side_tone_init();
#ifdef AIR_BTA_IC_PREMIUM_G3
    am_audio_cpd_init();
#endif
#ifdef MTK_AWS_MCE_ENABLE
    if (bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_PEQ, bt_aws_mce_report_peq_callback) != BT_STATUS_SUCCESS) {
        audio_src_srv_err("peq failed to register aws mce report callback\r\n", 0);
    }
#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
    extern void bt_aws_mce_report_DL_SW_gain_callback(bt_aws_mce_report_info_t *info);
    bt_status_t st = bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_AUDIO_GAIN, bt_aws_mce_report_DL_SW_gain_callback);
    if (st != BT_STATUS_SUCCESS)
    {
        audio_src_srv_err("[DL_SW_GAIN] failed to register aws mce report callback err:0x%x\r\n", 1, st);
    }
    extern bt_status_t bc_cm_event_DL_SW_gain_callback(bt_cm_event_t event_id, void *params, uint32_t params_len);
    bt_cm_register_event_callback(bc_cm_event_DL_SW_gain_callback);
#endif
#endif

}

/*****************************************************************************
 * FUNCTION
 *  aud_set_open_stream_req_hdlr
 * DESCRIPTION
 *  Get register ID
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_set_open_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *Background_ptr = NULL;
    bt_sink_srv_am_background_t *recoder_current_ptr = NULL, *recoder_previous_ptr = NULL;
    bt_sink_srv_am_hal_result_t eResult = HAL_AUDIO_STATUS_ERROR;
    uint32_t g_AudMem_size = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_aud_id_num == 1) {
        eResult = hal_audio_get_memory_size(&g_AudMem_size);
        if (eResult == HAL_AUDIO_STATUS_OK) {
            if (!aud_memory) {
#ifndef WIN32_UT
                aud_memory = (uint16_t *)pvPortMalloc(g_AudMem_size);
#else
                aud_memory = (uint16_t *)malloc(g_AudMem_size);
#endif
            }
            hal_audio_set_memory(aud_memory);
        } else {
#ifdef __AM_DEBUG_INFO__
            audio_src_srv_report("[Sink][AM][ERROR] Get memoey size", 0);
#endif
        }
    }

#ifndef WIN32_UT
    Background_ptr = (bt_sink_srv_am_background_t *)pvPortMalloc(sizeof(bt_sink_srv_am_background_t));
#else
    Background_ptr = (bt_sink_srv_am_background_t *)malloc(sizeof(bt_sink_srv_am_background_t));
#endif
    if (Background_ptr) {
        Background_ptr->aud_id = bAud_id;
        Background_ptr->type = am_background_temp->type;
        Background_ptr->priority = am_background_temp->priority;
        Background_ptr->priority_table = am_background_temp->priority_table;
        Background_ptr->notify_cb = am_background_temp->notify_cb;
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
        memset(&Background_ptr->local_feature.feature_param.peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
        Background_ptr->local_feature.feature_param.peq_param.enable = PEQ_DEFAULT_ENABLE;
        Background_ptr->local_feature.feature_param.peq_param.sound_mode = PEQ_DEFAULT_SOUND_MODE;
        Background_ptr->local_feature.feature_param.peq_param.setting_mode = PEQ_DIRECT;
#endif

        recoder_previous_ptr = &g_rBackground_head;
        recoder_current_ptr = g_rBackground_head.next;
        while ((recoder_current_ptr != NULL) && (recoder_current_ptr->priority > Background_ptr->priority)) {
            recoder_previous_ptr = recoder_current_ptr;
            recoder_current_ptr = recoder_current_ptr->next;
        }
        Background_ptr->prior = recoder_previous_ptr;
        Background_ptr->next = recoder_current_ptr;
        recoder_previous_ptr->next = Background_ptr;
        if (recoder_current_ptr != NULL) {
            recoder_current_ptr->prior = Background_ptr;
        }
        g_rAm_aud_id[bAud_id].contain_ptr = Background_ptr;
        //Background_ptr->notify_cb(am_background_temp->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
    }

    //printf("[AudM]open_hdr-id: %d, num: %d, use: %d, b_ptr: 0x%x\n", bAud_id,
    //g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)Background_ptr);
    audio_src_srv_report("[AudM]open_hdr-id: %d, num: %d, use: %d, b_ptr: 0x%x", 4, bAud_id,
                         g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)Background_ptr);
}

/*****************************************************************************
 * FUNCTION
 *  aud_set_play_stream_req_hdlr
 * DESCRIPTION
 *  Start to play the specified audio handler.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_set_play_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t         bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;
    bt_sink_srv_am_background_t *recoder_high_t = NULL;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prCurrent_resumer != NULL) {
#ifdef RTOS_TIMER
        xTimerStop(g_xTimer_am, 20);
        g_lExpire_count = 0;
#endif
        g_rAm_aud_id[g_prCurrent_resumer->aud_id].use = ID_IDLE_STATE;
        g_prCurrent_resumer = NULL;
    }

    //printf("[AudM]play_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x\n",
    //bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);
    audio_src_srv_report("[AudM]play_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x", 4,
                         bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);

    if (recoder_current_ptr) {
        recoder_current_ptr->type = am_background_temp->type;
        recoder_current_ptr->audio_path_type = am_background_temp->audio_path_type;
        memcpy(&(recoder_current_ptr->local_context), &(am_background_temp->local_context), sizeof(bt_sink_srv_am_codec_t));
        memcpy(&(recoder_current_ptr->audio_stream_in), &(am_background_temp->audio_stream_in), sizeof(bt_sink_srv_am_audio_stream_in_t));
        memcpy(&(recoder_current_ptr->audio_stream_out), &(am_background_temp->audio_stream_out), sizeof(bt_sink_srv_am_audio_stream_out_t));
        /* Find which is high priority */
        recoder_high_t = g_rBackground_head.next;
        while ((recoder_high_t != NULL) && (recoder_high_t->priority > recoder_current_ptr->priority)) {
            recoder_high_t = recoder_high_t->next;
        }
        if (recoder_high_t &&(recoder_high_t->aud_id != bAud_id)) {
            if (recoder_current_ptr->next != NULL) {
                recoder_current_ptr->next->prior = recoder_current_ptr->prior;
            }
            recoder_current_ptr->prior->next = recoder_current_ptr->next;
            recoder_high_t->prior->next = recoder_current_ptr;
            recoder_current_ptr->prior = recoder_high_t->prior;
            recoder_high_t->prior = recoder_current_ptr;
            recoder_current_ptr->next = recoder_high_t;
        }

#ifdef AIR_PROMPT_SOUND_ENABLE
        //prompt_control_stop_tone();
        // bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_MIXER_TONE_STOP, NULL);
#endif
        audio_src_srv_report("[AudM]play stream req: g_prCurrent_player:0x%x", 1, g_prCurrent_player);
        if (g_prCurrent_player != NULL) {
            if (recoder_current_ptr->priority < g_prCurrent_player->priority) {
                g_rAm_aud_id[bAud_id].use = ID_SUSPEND_STATE;
                audio_src_srv_report("[AudM]play stream req: suspend, In_p:%d, Current_p:%d", 2, recoder_current_ptr->priority, g_prCurrent_player->priority);
                recoder_current_ptr->notify_cb(bAud_id, AUD_SUSPEND_IND, AUD_EMPTY, NULL);
            } else {
                am_audio_set_suspend(recoder_current_ptr->type, g_prCurrent_player);
                g_prCurrent_player = recoder_current_ptr;
                am_audio_set_play(g_prCurrent_player);
            }
        } else {
            g_prCurrent_player = recoder_current_ptr;
            am_audio_set_play(g_prCurrent_player);
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *
 *
 *
 *****************************************************************************/
#ifdef AIR_RECORD_ENABLE

static void aud_prepare_wwe_para(uint32_t language_mode_address, uint32_t language_mode_length)
{
    uint32_t nvkey_vad_length_common, nvkey_vad_length_para;

    n9_dsp_share_info_t *n9_dsp_share_info = hal_audio_query_record_share_info();
    mcu2dsp_vad_param_p mcu2dsp_vad_param = (mcu2dsp_vad_param_p)(n9_dsp_share_info->start_addr);
    mcu2dsp_vad_param = (mcu2dsp_vad_param_p)hal_memview_dsp0_to_cm4((uint32_t)mcu2dsp_vad_param);

    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VAD_CMN_PARA, &nvkey_vad_length_common)) {
        flash_memory_read_nvdm_data(NVID_DSP_ALG_VAD_CMN_PARA, (uint8_t *)&mcu2dsp_vad_param->vad_nvkey_common, &nvkey_vad_length_common);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VAD Para, Para ID = [0x%04x]", 1, NVID_DSP_ALG_VAD_CMN_PARA);
    }

    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VAD_1MIC_V, &nvkey_vad_length_para)) {
        flash_memory_read_nvdm_data(NVID_DSP_ALG_VAD_1MIC_V, (uint8_t *)&mcu2dsp_vad_param->vad_nvkey_1mic_v_mode, &nvkey_vad_length_para);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VAD Para, Para ID = [0x%04x]", 1, NVID_DSP_ALG_VAD_1MIC_V);
    }

    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VAD_1MIC_C, &nvkey_vad_length_para)) {
        flash_memory_read_nvdm_data(NVID_DSP_ALG_VAD_1MIC_C, (uint8_t *)&mcu2dsp_vad_param->vad_nvkey_1mic_c_mode, &nvkey_vad_length_para);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VAD Para, Para ID = [0x%04x]", 1, NVID_DSP_ALG_VAD_1MIC_C);
    }

    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VAD_2MIC_V, &nvkey_vad_length_para)) {
        flash_memory_read_nvdm_data(NVID_DSP_ALG_VAD_2MIC_V, (uint8_t *)&mcu2dsp_vad_param->vad_nvkey_2mic_v_mode, &nvkey_vad_length_para);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VAD Para, Para ID = [0x%04x]", 1, NVID_DSP_ALG_VAD_2MIC_V);
    }
    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VAD_2MIC_C, &nvkey_vad_length_para)) {
        flash_memory_read_nvdm_data(NVID_DSP_ALG_VAD_2MIC_C, (uint8_t *)&mcu2dsp_vad_param->vad_nvkey_2mic_c_mode, &nvkey_vad_length_para);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VAD Para, Para ID = [0x%04x]", 1, NVID_DSP_ALG_VAD_2MIC_C);
    }

    mcu2dsp_vad_param->language_mode_address = language_mode_address;
    mcu2dsp_vad_param->language_mode_length = language_mode_length;

    audio_src_srv_err("[VAD_NVKEY]language_mode_address = [0x%08x], language_mode_length = [0x%08x]", 2, language_mode_address, language_mode_length);

    /*read vow config param*/
    if (NVDM_STATUS_NAT_OK == flash_memory_query_nvdm_data_length(NVID_DSP_FW_VOW_PARA, &nvkey_vad_length_common)) {
        flash_memory_read_nvdm_data(NVID_DSP_FW_VOW_PARA, (uint8_t *)&mcu2dsp_vad_param->vow_setting, &nvkey_vad_length_common);
    } else {
        audio_src_srv_err("[VAD_NVKEY]Failded to Read VOW Para, Para ID = [0x%04x]", 1, NVID_DSP_FW_VOW_PARA);
    }
    /*vow mic config*/
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    mcu2dsp_open_param_t *open_param = NULL;
    open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    if(open_param != NULL) {
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_VAD_PHASE0, &open_param->stream_in_param);
        mcu2dsp_vad_param->vow_setting.main_mic = open_param->stream_in_param.afe.audio_device;
        mcu2dsp_vad_param->vow_setting.ref_mic  = open_param->stream_in_param.afe.audio_device1;

        mcu2dsp_vad_param->vow_setting.main_interface = open_param->stream_in_param.afe.audio_interface;
        mcu2dsp_vad_param->vow_setting.ref_interface = open_param->stream_in_param.afe.audio_interface1;
            /*all vow mic use same adda setting*/
        mcu2dsp_vad_param->adda_analog_mic_mode = open_param->stream_in_param.afe.ul_adc_mode[0];
#ifdef AIR_BTA_IC_STEREO_HIGH_G3
        mcu2dsp_vad_param->vow_setting.enable = 0;
#endif
        audio_src_srv_report("[VAD_NVKEY]main_mic = %d,ref_mic = %d, [VAD_VOW]adda_analog_mic_mode = %d", 3, mcu2dsp_vad_param->vow_setting.main_mic, mcu2dsp_vad_param->vow_setting.ref_mic,mcu2dsp_vad_param->adda_analog_mic_mode);

        vPortFree(open_param);
    } else {
        AUDIO_ASSERT(0 && "[VAD_PARA] malloc open_para fail!");
    }
#endif
}

static void am_audio_record_sink_play(bt_sink_srv_am_background_t *background_ptr)
{
    hal_audio_status_t result;
    g_stream_in_code_type = background_ptr->local_context.record_format.record_codec.codec_cap.codec_type;
    g_bit_rate = background_ptr->local_context.record_format.record_codec.codec_cap.bit_rate;
    g_wwe_mode = background_ptr->local_context.record_format.record_codec.codec_cap.wwe_mode;
    uint32_t language_mode_address =  background_ptr->local_context.record_format.record_codec.codec_cap.wwe_language_mode_address;
    uint32_t language_mode_length =  background_ptr->local_context.record_format.record_codec.codec_cap.wwe_language_mode_length;
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
    ami_set_afe_param(STREAM_IN, HAL_AUDIO_SAMPLING_RATE_16KHZ, true);
#endif

    if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_LC) {
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        audio_src_srv_report("[RECORD_LC]hal_audio_start_stream_in_leakage_compensation, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in_leakage_compensation();
#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
    } else if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_FADP_ANC_COMP) {
        audio_src_srv_report("[RECORD_SzD]hal_audio_start_stream_in_fadp_anc_compensation, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in_fadp_anc_compensation();
#endif
#else
        result = HAL_AUDIO_STATUS_INVALID_PARAMETER;
#endif
#if defined(MTK_USER_TRIGGER_FF_ENABLE) & defined(MTK_USER_TRIGGER_ADAPTIVE_FF_V2)
    } else if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_SZ) {
        audio_src_srv_report("[user_trigger_ff]hal_audio_start_stream_in_user_trigger_adaptive_ff, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in_user_trigger_adaptive_ff(2);

    } else if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ) {
        audio_src_srv_report("[user_trigger_ff]hal_audio_start_stream_in_user_trigger_adaptive_ff, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in_user_trigger_adaptive_ff(1);

    } else if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ_FIR) {
        audio_src_srv_report("[user_trigger_ff]hal_audio_start_stream_in_user_trigger_adaptive_ff, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in_user_trigger_adaptive_ff(0);
#endif
    } else {
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
        if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF) {
            audio_anc_user_trigger_ff_write_Sz_coef_share_buff();
        }
#endif
#endif
        //audio_src_srv_report("hal_audio_start_stream_in, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
#ifdef MTK_USB_AUDIO_RECORD_ENABLE
        result = hal_audio_start_stream_in_usb(HAL_AUDIO_RECORD_VOICE);
#else
        if (g_wwe_mode != WWE_MODE_NONE) {
            aud_prepare_wwe_para(language_mode_address, language_mode_length);
        }
        //audio_src_srv_report("hal_audio_start_stream_in, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
        result = hal_audio_start_stream_in(HAL_AUDIO_RECORD_VOICE);
#endif
        audio_src_srv_report("hal_audio_start_stream_in, codec_type:0x%x\r\n", 1, g_stream_in_code_type);
    }

    if (result == HAL_AUDIO_STATUS_OK) {
        g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_COMPLETE, NULL);
    } else {
        g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_FAILURE, NULL);
    }
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
    if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
        ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
        aud_ul_control(true);
    }
}
static void am_audio_record_sink_stop(void)
{
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
    if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
        ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
        aud_ul_control(false);
    }
    if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_ANC_LC) {
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        hal_audio_stop_stream_in_leakage_compensation();
#endif
#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
    } else if (g_stream_in_code_type == AUDIO_DSP_CODEC_TYPE_FADP_ANC_COMP) {
        hal_audio_stop_stream_in_fadp_anc_compensation();
#endif
    } else {
        hal_audio_stop_stream_in();
    }
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
    ami_set_afe_param(STREAM_IN, HAL_AUDIO_SAMPLING_RATE_16KHZ, false);
#endif
}
#endif /*AIR_RECORD_ENABLE*/
uint32_t am_audio_check_pri_table(uint16_t index,
                                  bt_sink_srv_am_priority_table_t *ori_pri_table,
                                  bt_sink_srv_am_priority_table_t *cur_pri_table)
{
    uint32_t higher_table = 0x0;
#if 1
    if (index == 0) {
        if (ori_pri_table->DL_1 < cur_pri_table->DL_1) {
            higher_table |= AM_PLAYBACK_DL1;
        }
    } else if (index == 1) {
        if (ori_pri_table->DL_2 < cur_pri_table->DL_2) {
            higher_table |= AM_PLAYBACK_DL2;
        }
    } else if (index == 2) {
        if (ori_pri_table->UL_1 < cur_pri_table->UL_1) {
            higher_table |= AM_PLAYBACK_UL1;
        }
    } else if (index == 3) {
        if (ori_pri_table->UL_2 < cur_pri_table->UL_2) {
            higher_table |= AM_PLAYBACK_UL2;
        }
    }
#else
    if (ori_pri_table + index < cur_pri_table + index) {
        higher_table |= 1 << index;
    }
#endif
    audio_src_srv_report("[AudM] check_pri_table index(%d) preempt (0x%x)", 2, index, higher_table);
    return higher_table;
}

am_playback_arbitration_result_t am_audio_get_permission_request(bt_sink_srv_am_background_t *request)
{
    am_playback_arbitration_result_t result;
    result.am_arbitration_status = AM_PLAYBACK_REJECT;
    result.preempt_index = 0x0;
    bt_sink_srv_am_priority_table_t pri_table_NULL = {AUD_LOW, AUD_LOW, AUD_LOW, AUD_LOW};
    for (uint16_t i = 0; i < AM_PLAYBACK_INDEX_TOTAL; i++) {
        if (g_prCurrent_playback[i] != NULL) {
            if (am_audio_check_pri_table(i, &g_prCurrent_playback[i]->priority_table, &request->priority_table) != 0x0) {
                result.preempt_index |= 1 << i;
            }
        } else if (am_audio_check_pri_table(i, &pri_table_NULL, &request->priority_table) != 0x0) {
            result.preempt_index |= 1 << i;
        }
    }
    if (result.preempt_index != 0x0) {
        result.am_arbitration_status = AM_PLAYBACK_PERMISSION;
    }
#ifdef AIR_RECORD_ENABLE
#ifndef AIR_AMA_HOTWORD_DURING_CALL_ENABLE
    if (g_prHfp_media_handle != NULL)
#else
    if ((g_prHfp_media_handle != NULL) && (request->local_context.record_format.record_codec.codec_cap.wwe_mode != WWE_MODE_AMA))
#endif
#else
    if (g_prHfp_media_handle != NULL)
#endif
    {
        result.am_arbitration_status = AM_PLAYBACK_REJECT;
    }
    return result;
}

static void test_aud_set_play_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t         bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;
    //bt_sink_srv_am_background_t *recoder_high_t = NULL;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    audio_src_srv_report("[AudM]TEST_play_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x", 4,
                         bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);

    if (recoder_current_ptr) {
        recoder_current_ptr->type = am_background_temp->type;
        recoder_current_ptr->audio_path_type = am_background_temp->audio_path_type;
        memcpy(&(recoder_current_ptr->local_context), &(am_background_temp->local_context), sizeof(bt_sink_srv_am_codec_t));
        memcpy(&(recoder_current_ptr->audio_stream_in), &(am_background_temp->audio_stream_in), sizeof(bt_sink_srv_am_audio_stream_in_t));
        memcpy(&(recoder_current_ptr->audio_stream_out), &(am_background_temp->audio_stream_out), sizeof(bt_sink_srv_am_audio_stream_out_t));

#if defined (AIR_AUDIO_TRANSMITTER_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        //bypass permission check
        if ((recoder_current_ptr->type == AUDIO_TRANSMITTER)||(recoder_current_ptr->type == ULL_BLE_UL)) {
            am_audio_set_play(recoder_current_ptr);
        } else
#endif
        {
            /*----------------------------------------------------------------*/
            /* Check Permission                                                      */
            /*----------------------------------------------------------------*/
            am_playback_arbitration_result_t am_arbitration;
            am_arbitration = am_audio_get_permission_request(recoder_current_ptr);
            audio_src_srv_report("[AudM] get_permission_request status(%d) preempt_index (0x%x)", 2, am_arbitration.am_arbitration_status, am_arbitration.preempt_index);
            if (am_arbitration.am_arbitration_status == AM_PLAYBACK_REJECT) {
#ifdef AIR_RECORD_ENABLE
                if ((recoder_current_ptr->type == RECORD) && ((recoder_current_ptr->local_context.record_format.record_codec.codec_cap.codec_type == AUDIO_DSP_CODEC_TYPE_ANC_LC)
#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
                                                              || (recoder_current_ptr->local_context.record_format.record_codec.codec_cap.codec_type == AUDIO_DSP_CODEC_TYPE_FADP_ANC_COMP)
#endif
#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
                                                              || (recoder_current_ptr->local_context.record_format.record_codec.codec_cap.codec_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ)
                                                              || (recoder_current_ptr->local_context.record_format.record_codec.codec_cap.codec_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ_FIR)
                                                              || (recoder_current_ptr->local_context.record_format.record_codec.codec_cap.codec_type == AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_SZ)
#endif
                                                             )
                    && (g_prCurrent_playback[2] != NULL)) {
                    audio_src_srv_report("[AudM] Leakage detection will replace normal record", 0);
                    hal_audio_stop_stream_in();
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_RECORD_STOP);
#endif
                    g_prCurrent_playback[2]->notify_cb(g_prCurrent_playback[2]->aud_id, AUD_SUSPEND_BY_IND, AUD_SUSPEND_BY_LC, NULL);
                    g_rAm_aud_id[g_prCurrent_playback[2]->aud_id].use = ID_IDLE_STATE;
                    g_prCurrent_playback[2] = recoder_current_ptr;
                    audio_src_srv_report("[AudM] g_prCurrent_playback[2](0x%x) g_rAm_aud_id(0x%x)", 2, g_prCurrent_playback[2], &g_rAm_aud_id[bAud_id].contain_ptr);
                    am_audio_set_play(recoder_current_ptr);
                } else
#endif
                {
                    /*NOTIFY Playback play request was fail. */
                    recoder_current_ptr->notify_cb(recoder_current_ptr->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
                }
            } else if (am_arbitration.am_arbitration_status == AM_PLAYBACK_PERMISSION) {
                /*Use preempt_index to close playback which have exit*/
                if ((am_arbitration.preempt_index & AM_PLAYBACK_DL1)) {
                    if (g_prCurrent_playback[0] != NULL) {
                        am_audio_set_stop(g_prCurrent_playback[0]);
                    }
                    g_prCurrent_playback[0] = recoder_current_ptr;
                }
                if ((am_arbitration.preempt_index & AM_PLAYBACK_DL2)) {
                    if (g_prCurrent_playback[1] != NULL) {
                        am_audio_set_stop(g_prCurrent_playback[1]);
                    }
                    g_prCurrent_playback[1] = recoder_current_ptr;
                }
                if ((am_arbitration.preempt_index & AM_PLAYBACK_UL1)) {
                    if (g_prCurrent_playback[2] != NULL) {
                        am_audio_set_stop(g_prCurrent_playback[2]);
                    }
                    g_prCurrent_playback[2] = recoder_current_ptr;
                    audio_src_srv_report("[AudM] g_prCurrent_playback[2](0x%x) g_rAm_aud_id(0x%x)", 2, g_prCurrent_playback[2], &g_rAm_aud_id[bAud_id].contain_ptr);
                }
                if ((am_arbitration.preempt_index & AM_PLAYBACK_UL2)) {
                    if (g_prCurrent_playback[3] != NULL) {
                        am_audio_set_stop(g_prCurrent_playback[3]);
                    }
                    g_prCurrent_playback[3] = recoder_current_ptr;
                }
                am_audio_set_play(recoder_current_ptr);
            } else {
                /*ERROR*/
            }
        }

#if 0
        /* Find which is high priority */
        recoder_high_t = g_rBackground_head.next;
        while ((recoder_high_t != NULL) && (recoder_high_t->priority > recoder_current_ptr->priority)) {
            recoder_high_t = recoder_high_t->next;
        }
        if (recoder_high_t &&(recoder_high_t->aud_id != bAud_id)) {
            if (recoder_current_ptr->next != NULL) {
                recoder_current_ptr->next->prior = recoder_current_ptr->prior;
            }
            recoder_current_ptr->prior->next = recoder_current_ptr->next;
            recoder_high_t->prior->next = recoder_current_ptr;
            recoder_current_ptr->prior = recoder_high_t->prior;
            recoder_high_t->prior = recoder_current_ptr;
            recoder_current_ptr->next = recoder_high_t;
        }
        audio_src_srv_report("[AudM]paly stream req: g_prCurrent_player:0x%x", 1, g_prCurrent_player);
        if (g_prCurrent_player != NULL) {
            if (recoder_current_ptr->priority < g_prCurrent_player->priority) {
                g_rAm_aud_id[bAud_id].use = ID_SUSPEND_STATE;
                audio_src_srv_report("[AudM]paly stream req: suspend, In_p:%d, Current_p:%d", 2, recoder_current_ptr->priority, g_prCurrent_player->priority);
                recoder_current_ptr->notify_cb(bAud_id, AUD_SUSPEND_IND, AUD_EMPTY, NULL);
            } else {
                am_audio_set_suspend(recoder_current_ptr->type, g_prCurrent_player);
                g_prCurrent_player = recoder_current_ptr;
                am_audio_set_play(g_prCurrent_player);
            }
        } else {
            g_prCurrent_player = recoder_current_ptr;
            am_audio_set_play(g_prCurrent_player);
        }
#endif
    }
}

static void test_aud_set_stop_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    //printf("[AudM]stop_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x\n",
    //bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);

    audio_src_srv_report("[AudM]Test_stop_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x", 4,
                         bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);
    if (g_rAm_aud_id[bAud_id].use == ID_IDLE_STATE) {
        //audio_src_srv_report("[AudM]Test_stop_hdr-id, state idle, no need to do anything", 0);
        return;
    }

    //g_prCurrent_player = NULL;
    am_audio_set_stop(recoder_current_ptr);
    am_audio_search_suspended();
}

/*****************************************************************************
 * FUNCTION
 *  aud_set_stop_stream_req_hdlr
 * DESCRIPTION
 *  Stop playing the specified audio handler.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_set_stop_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    //printf("[AudM]stop_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x\n",
    //bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);

    audio_src_srv_report("[AudM]stop_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x", 4,
                         bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);

    g_prCurrent_player = NULL;
    am_audio_set_stop(recoder_current_ptr);
    am_audio_search_suspended();
}

/*****************************************************************************
 * FUNCTION
 *  aud_set_close_stream_req_hdlr
 * DESCRIPTION
 *  Close the opening audio handler.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_set_close_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_rAm_aud_id[bAud_id].use == ID_RESUME_STATE) {
#ifdef RTOS_TIMER
        xTimerStop(g_xTimer_am, 20);
        g_lExpire_count = 0;
#endif
        g_prCurrent_resumer = NULL;
    }

    //printf("[AudM]close_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x\n",
    //bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);
    audio_src_srv_report("[AudM]close_hdr-id: %d, num: %d, use: %d, c_ptr: 0x%x", 4,
                         bAud_id, g_aud_id_num, g_rAm_aud_id[bAud_id].use, (unsigned int)recoder_current_ptr);
    if (g_rAm_aud_id[bAud_id].use > ID_IDLE_STATE) {
        audio_src_srv_report("[AudM]close_hdr-id: %d, !!!!!!State abnormal, need to check!!!!!", 1, bAud_id);
    }
    if (recoder_current_ptr) {
        g_rAm_aud_id[bAud_id].contain_ptr = NULL;
        //recoder_current_ptr->notify_cb(bAud_id, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
        if (recoder_current_ptr->next != NULL) {
            recoder_current_ptr->next->prior = recoder_current_ptr->prior;
        }
        if (recoder_current_ptr->prior != NULL) {
            recoder_current_ptr->prior->next = recoder_current_ptr->next;
        }
#ifndef WIN32_UT
        vPortFree(recoder_current_ptr);
#else
        free(recoder_current_ptr);
#endif

        if (g_aud_id_num == 1) {
            if (aud_memory != NULL) {
#ifndef WIN32_UT
                vPortFree(aud_memory);
#else
                free(aud_memory);
#endif
                aud_memory = NULL;
            }
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_set_volume_stream_out_req_hdlr
 * DESCRIPTION
 *  Set audio in/out volume.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_set_volume_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_volume_level_t level;
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!g_prCurrent_player) {
        audio_src_srv_report("[AudM]volume setting when no current player", 0);
        return ;
    }
    if (g_prCurrent_player->aud_id != am_background_temp->aud_id) {
        audio_src_srv_err("[AudM]set volume g_prCurrent_player->aud_id %d != am_background_temp->aud_id %d\n", 2, g_prCurrent_player->aud_id, am_background_temp->aud_id);
        return;
    }
    audio_src_srv_report("[AudM]volume setting start, current player type:%d out device:%d volume:%d ", 3,g_prCurrent_player->type,g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume);

    if (am_background_temp->in_out == STREAM_OUT) {
        if(am_background_temp->audio_stream_out.audio_volume_set_by_gain_value == true){
            g_prCurrent_player->audio_stream_out.audio_gain_value = am_background_temp->audio_stream_out.audio_gain_value;
            if((g_prCurrent_player->type == HFP)||(g_prCurrent_player->type == BLE)||(g_prCurrent_player->type == ULL_BLE_DL)||(g_prCurrent_player->type == A2DP)||(g_prCurrent_player->type == AWS)){
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, g_prCurrent_player->audio_stream_out.audio_gain_value, 0x7FFF);
            } else {
                audio_src_srv_err("[AudM]set volume by gain not support, type %d",1,g_prCurrent_player->type);
                assert(0);
            }
            return;
        }
        level = (bt_sink_srv_am_volume_level_t)am_background_temp->audio_stream_out.audio_volume;
        g_prCurrent_player->audio_stream_out.audio_volume = (bt_sink_srv_am_volume_level_out_t)level;
        if (g_prCurrent_player->type == HFP) {
            am_set_HFP_volume(g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                                g_prCurrent_player->audio_stream_in.audio_device,
                                g_prCurrent_player->audio_stream_in.audio_volume,
                                g_prCurrent_player->audio_stream_out.audio_device,
                                g_prCurrent_player->audio_stream_out.audio_volume,
                                false, true);
#ifdef AIR_BT_CODEC_BLE_ENABLED
        } else if ((g_prCurrent_player->type == BLE)||(g_prCurrent_player->type == ULL_BLE_DL)) {

                #if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                if (g_prCurrent_player->type == ULL_BLE_DL)
                {
                    audio_src_srv_report("[AudM]volume setting for ULL BLE DL type: %d\n", 1, g_prCurrent_player->type);
                am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                } else if ((g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY)||(g_prCurrent_player->local_context.ble_format.ble_codec.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL))
                #else
                if ((g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY)||(g_prCurrent_player->local_context.ble_format.ble_codec.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL))
                #endif
                {
                am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC,
                            g_prCurrent_player->audio_stream_in.audio_device,
                            g_prCurrent_player->audio_stream_in.audio_volume,
                            g_prCurrent_player->audio_stream_out.audio_device,
                            g_prCurrent_player->audio_stream_out.audio_volume,
                            false, true);
                } else
                {
                am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
            }

#endif
        } else {
                if (g_prCurrent_player->type == PCM)
                {
                    am_set_PCM_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                } else if (g_prCurrent_player->type == A2DP)
                {
                    am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                } else if (g_prCurrent_player->type == FILES)
                {
                    am_set_MP3_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                } else if (g_prCurrent_player->type == AWS)
                {
                    am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                } else if (g_prCurrent_player->type == LINE_IN)
                {
#ifdef MTK_LINE_IN_ENABLE
                    am_set_LINE_IN_volume(VOL_LINE_IN, g_prCurrent_player->audio_stream_in.audio_device, g_prCurrent_player->audio_stream_in.audio_volume,
                    g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume, false, true);

#endif
                } else if (g_prCurrent_player->type == USB_AUDIO_IN)
                {
                    am_set_USB_AUDIO_IN_volume(VOL_USB_AUDIO_IN, g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume);
                } else
                {
                    // Fix Coverity issue
                    bt_sink_srv_audio_setting_vol_info_t vol_info;
                    vol_info.type = VOL_DEF;
                    vol_info.vol_info.def_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                    vol_info.vol_info.def_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;

                bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);

                    audio_src_srv_report("[AudM]volume. Unknown type: %d\n", 1, g_prCurrent_player->type);
            }
        }
    } else if (am_background_temp->in_out == STREAM_IN) {
        if (g_prCurrent_player->type == HFP) {
            level = (bt_sink_srv_am_volume_level_t)am_background_temp->audio_stream_in.audio_volume;
            g_prCurrent_player->audio_stream_in.audio_volume = (bt_sink_srv_am_volume_level_in_t)level;
            am_set_HFP_volume(g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                            g_prCurrent_player->audio_stream_in.audio_device,
                            g_prCurrent_player->audio_stream_in.audio_volume,
                            g_prCurrent_player->audio_stream_out.audio_device,
                            g_prCurrent_player->audio_stream_out.audio_volume,
                            true, false);
#ifdef AIR_BT_CODEC_BLE_ENABLED
        }   else if ((g_prCurrent_player->type == BLE)||(g_prCurrent_player->type == ULL_BLE_UL)) {
            if(am_background_temp->audio_stream_in.audio_volume_set_by_gain_value) {
                int32_t ble_gain = audio_get_gain_in_in_dB(am_background_temp->audio_stream_in.audio_gain_value, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
                audio_src_srv_report("[AudM][BLE] UL set SW gain to level %d->%d dB\r\n", 2, am_background_temp->audio_stream_in.audio_gain_value,ble_gain);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_SET_VOLUME, 0, ble_gain, false);
            } else {
                level = (bt_sink_srv_am_volume_level_t)am_background_temp->audio_stream_in.audio_volume;
                g_prCurrent_player->audio_stream_in.audio_volume = (bt_sink_srv_am_volume_level_in_t)level;
                am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC,
                    g_prCurrent_player->audio_stream_in.audio_device,
                    g_prCurrent_player->audio_stream_in.audio_volume,
                    g_prCurrent_player->audio_stream_out.audio_device,
                    g_prCurrent_player->audio_stream_out.audio_volume,
                    true, false);
            }
#endif
        } else if (g_prCurrent_player->type == LINE_IN) {
#ifdef MTK_LINE_IN_ENABLE
            level = (bt_sink_srv_am_volume_level_t)am_background_temp->audio_stream_in.audio_volume;
            g_prCurrent_player->audio_stream_in.audio_volume = (bt_sink_srv_am_volume_level_in_t)level;
            am_set_LINE_IN_volume(VOL_LINE_IN, g_prCurrent_player->audio_stream_in.audio_device, g_prCurrent_player->audio_stream_in.audio_volume,
                                g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume, true, false);
#endif /* MTK_LINE_IN_ENABLE */
        } else {
            level = (bt_sink_srv_am_volume_level_t)am_background_temp->audio_stream_in.audio_volume;
            g_prCurrent_player->audio_stream_in.audio_volume = (bt_sink_srv_am_volume_level_in_t)level;
            #ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            aud_set_volume_level(STREAM_IN, AUD_VOL_AUDIO, g_prCurrent_player->audio_stream_in.audio_device, level);
            #endif
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_mute_device_stream_req_hdlr
 * DESCRIPTION
 *  Mute audio input/output device.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_mute_device_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    bool err_flag = false;
    bool mute = false;
    hal_audio_hw_stream_out_index_t hw_gain_index = HAL_AUDIO_STREAM_OUT_ALL; // default mute all
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    switch (am_background_temp->in_out) {
        case STREAM_IN:
            /* code */
            if ((am_background_temp->aud_id != FEATURE_NO_NEED_ID) && (g_prCurrent_player)) {
                g_prCurrent_player->audio_stream_in.audio_mute = am_background_temp->audio_stream_in.audio_mute;
            }
            mute = am_background_temp->audio_stream_in.audio_mute;
            // hw_gain_index = HAL_AUDIO_STREAM_OUT1;
            if (((g_prCurrent_player != NULL) && ((g_prCurrent_player->type == HFP) || (g_prCurrent_player->type == BLE))) || (am_background_temp->type == ULL_BLE_UL)) {
                hal_audio_mute_stream_in_by_scenario(HAL_AUDIO_STREAM_IN_SCENARIO_HFP, mute);
            } else {
                hal_audio_mute_stream_in(mute); // Actually, mute stream_in won't work.
            }
            return;
        case STREAM_OUT:   // DL1
        case STREAM_OUT_2: // DL2 (VP)
        case STREAM_OUT_3: // DL3
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
        case STREAM_OUT_4: // DL3
#endif
            if ((am_background_temp->aud_id != FEATURE_NO_NEED_ID) && (g_prCurrent_player)) {
                g_prCurrent_player->audio_stream_out.audio_mute = am_background_temp->audio_stream_out.audio_mute;;
            }
            mute = am_background_temp->audio_stream_out.audio_mute;
            hw_gain_index = (am_background_temp->in_out == STREAM_OUT) ? HAL_AUDIO_STREAM_OUT1 :
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                            ((am_background_temp->in_out == STREAM_OUT_2) ? HAL_AUDIO_STREAM_OUT2 :
                            ((am_background_temp->in_out == STREAM_OUT_3) ? HAL_AUDIO_STREAM_OUT3 : HAL_AUDIO_STREAM_OUT4));
#else
                            ((am_background_temp->in_out == STREAM_OUT_2) ? HAL_AUDIO_STREAM_OUT2 : HAL_AUDIO_STREAM_OUT3);
#endif
            break;
        default:
            err_flag = true;
            break;
    }
    if (err_flag) {
        audio_src_srv_report("[Sink][AM] aud_mute_device_stream_req_hdlr error, in_out = %d", 1, am_background_temp->in_out);
        return;
    }
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
    hal_audio_mute_stream_out(mute, hw_gain_index);
#else
    hal_audio_mute_stream_out(mute);
#endif
}

/*****************************************************************************
 * FUNCTION
 *  aud_config_device_stream_req_hdlr
 * DESCRIPTION
 *  Set audio input/output device.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_config_device_stream_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (am_background_temp->in_out == STREAM_OUT) {
        g_prCurrent_player->audio_stream_out.audio_device = am_background_temp->audio_stream_out.audio_device;
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
        hal_audio_mute_stream_out(TRUE, HAL_AUDIO_STREAM_OUT1);
#else
        hal_audio_mute_stream_out(TRUE);
#endif
        hal_audio_set_stream_out_device(g_prCurrent_player->audio_stream_out.audio_device);
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
        hal_audio_mute_stream_out(FALSE, HAL_AUDIO_STREAM_OUT1);
#else
        hal_audio_mute_stream_out(FALSE);
#endif
    } else if (am_background_temp->in_out == STREAM_IN) {
        g_prCurrent_player->audio_stream_in.audio_device = am_background_temp->audio_stream_in.audio_device;
        hal_audio_mute_stream_in(TRUE);
        hal_audio_set_stream_in_device(g_prCurrent_player->audio_stream_out.audio_device);
        hal_audio_mute_stream_in(FALSE);
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_rw_stream_data_req_hdlr
 * DESCRIPTION
 *  Write data to audio output for palyback / Read data to audio input for record.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_rw_stream_data_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t     *am_background_temp = &(amm_ptr->background_info);
    uint8_t                     bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_hal_result_t     eResult = HAL_AUDIO_STATUS_ERROR;
    bt_sink_srv_am_stream_type_t    eIn_out;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_rAm_aud_id[bAud_id].use == ID_RESUME_STATE) {
#ifdef __AM_DEBUG_INFO__
        audio_src_srv_report("[Sink][AM] Resume to play", 0);
#endif
#ifdef RTOS_TIMER
        xTimerStop(g_xTimer_am, 20);
        g_lExpire_count = 0;
#endif
        g_prCurrent_player = g_prCurrent_resumer;
        am_audio_set_play(g_prCurrent_player);
        g_rAm_aud_id[bAud_id].use = ID_PLAY_STATE;
        g_prCurrent_resumer = NULL;
    } else {
#ifdef __AM_DEBUG_INFO__
        audio_src_srv_report("[Sink][AM] Continue playing", 0);
#endif
        eIn_out = g_prCurrent_player->local_context.pcm_format.in_out;
        if (eIn_out == STREAM_OUT) {
            eResult = hal_audio_write_stream_out(am_background_temp->local_context.pcm_format.stream.buffer,
                                                 am_background_temp->local_context.pcm_format.stream.size);
            if (eResult == HAL_AUDIO_STATUS_OK) {
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_STREAM_DATA_REQ, AUD_CMD_COMPLETE, NULL);
                return;
            }
        } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
            eResult = hal_audio_read_stream_in(am_background_temp->local_context.pcm_format.stream.buffer,
                                               am_background_temp->local_context.pcm_format.stream.size);
            if (eResult == HAL_AUDIO_STATUS_OK) {
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_STREAM_DATA_REQ, AUD_CMD_COMPLETE, NULL);
                return;
            }
#endif
        }
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_STREAM_DATA_REQ, AUD_CMD_FAILURE, NULL);
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_query_stream_len_req_hdlr
 * DESCRIPTION
 *  Query available input/output data length.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_query_stream_len_req_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_background_t     *am_background_temp = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t             bAud_id = am_background_temp->aud_id;
    bt_sink_srv_am_background_t     *recoder_current_ptr = g_rAm_aud_id[bAud_id].contain_ptr;
    bt_sink_srv_am_hal_result_t     eResult = HAL_AUDIO_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (am_background_temp->in_out == STREAM_OUT) {
        eResult = hal_audio_get_stream_out_sample_count(am_background_temp->data_length_ptr);
    } else if (am_background_temp->in_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
        eResult = hal_audio_get_stream_in_sample_count(am_background_temp->data_length_ptr);
#endif
    }
    /* CallBack to AP */
    if (eResult == HAL_AUDIO_STATUS_OK) {
        recoder_current_ptr->notify_cb(bAud_id, AUD_SELF_CMD_REQ,  AUD_CMD_COMPLETE, NULL);
    } else {
        recoder_current_ptr->notify_cb(bAud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_bt_codec_a2dp_callback
 * DESCRIPTION
 *  This function is used to send L1Audio events to A.M.
 * PARAMETERS
 *  handle           [IN]
 *  event_id         [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_bt_codec_a2dp_callback(bt_media_handle_t *handle, bt_sink_srv_am_bt_event_t event_id)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prCurrent_player->type == A2DP) {
#ifdef __BT_AWS_SUPPORT__
        if (event_id == BT_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW) {
            g_aws_skew_loop_count++;
            if (g_aws_skew_loop_count >= BT_SINK_SRV_AWS_SKEW_LOOP_COUNT) {
                g_aws_skew_loop_count = 0;
                g_prCurrent_player->local_context.a2dp_format.a2dp_event = event_id;
                bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_A2DP_PROC_IND,
                                         MSG_ID_MEDIA_A2DP_PROC_CALL_EXT_REQ,
                                         g_prCurrent_player,
                                         TRUE, ptr_callback_amm);
            }
        } else if (event_id == BT_CODEC_MEDIA_AWS_CHECK_UNDERFLOW) {
            if (g_aws_skew_loop_count == BT_SINK_SRV_AWS_SKEW_LOOP_1ST_COUNT) {
                g_prCurrent_player->local_context.a2dp_format.a2dp_event = event_id;
                bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_A2DP_PROC_IND,
                                         MSG_ID_MEDIA_A2DP_PROC_CALL_EXT_REQ,
                                         g_prCurrent_player,
                                         TRUE, ptr_callback_amm);
            }
        } else {
            g_prCurrent_player->local_context.a2dp_format.a2dp_event = event_id;
            bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_A2DP_PROC_IND,
                                     MSG_ID_MEDIA_A2DP_PROC_CALL_EXT_REQ,
                                     g_prCurrent_player,
                                     TRUE, ptr_callback_amm);
        }
#else
        g_prCurrent_player->local_context.a2dp_format.a2dp_event = event_id;
        bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_A2DP_PROC_IND,
                                 MSG_ID_MEDIA_A2DP_PROC_CALL_EXT_REQ,
                                 g_prCurrent_player,
                                 TRUE, ptr_callback_amm);
#endif /* __BT_AWS_SUPPORT__ */
    } else if (g_prCurrent_player->type == AWS) {
#ifdef __BT_AWS_SUPPORT__
        g_prCurrent_player->local_context.aws_format.aws_event = event_id;
        if (event_id == BT_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW) {
            g_aws_skew_loop_count++;
            if (g_aws_skew_loop_count >= BT_SINK_SRV_AWS_SKEW_LOOP_COUNT) {
                g_aws_skew_loop_count = 0;
                bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_AWS_A2DP_PROC_IND,
                                         MSG_ID_MEDIA_AWS_A2DP_PROC_CALL_EXT_REQ,
                                         g_prCurrent_player,
                                         TRUE, ptr_callback_amm);
            }
        } else if (event_id == BT_CODEC_MEDIA_AWS_CHECK_UNDERFLOW) {
            if (g_aws_skew_loop_count == BT_SINK_SRV_AWS_SKEW_LOOP_1ST_COUNT) {
                bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_AWS_A2DP_PROC_IND,
                                         MSG_ID_MEDIA_AWS_A2DP_PROC_CALL_EXT_REQ,
                                         g_prCurrent_player,
                                         TRUE, ptr_callback_amm);
            }
        } else {
            bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_AWS_A2DP_PROC_IND,
                                     MSG_ID_MEDIA_AWS_A2DP_PROC_CALL_EXT_REQ,
                                     g_prCurrent_player,
                                     TRUE, ptr_callback_amm);
        }
#elif defined(MTK_AWS_MCE_ENABLE)
        g_prCurrent_player->local_context.aws_format.aws_event = event_id;
        bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_AWS_A2DP_PROC_IND,
                                 MSG_ID_MEDIA_AWS_A2DP_PROC_CALL_EXT_REQ,
                                 g_prCurrent_player,
                                 TRUE, ptr_callback_amm);
#endif /* __BT_AWS_SUPPORT__ */
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_bt_codec_hfp_callback
 * DESCRIPTION
 *  This function is used to send L1Audio events to A.M.
 * PARAMETERS
 *  handle           [IN]
 *  event_id         [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_bt_codec_hfp_callback(bt_media_handle_t *handle, bt_sink_srv_am_bt_event_t event_id)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    switch(event_id){
        case BT_CODEC_MEDIA_HFP_AVC_PARA_SEND:
        case BT_CODEC_MEDIA_LEQ_PARA_SEND:
                    if ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) //for earbuds
        ||((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_NONE))) //for headset
        {
            printf("[NDVC] It's Agent, send avc_vol");
            if (g_prCurrent_player->type == HFP) {
                g_prCurrent_player->local_context.hfp_format.hfp_event = event_id;
                if(event_id == BT_CODEC_MEDIA_HFP_AVC_PARA_SEND){
                    am_hfp_ndvc_sent_avc_vol(AVC_UPDATE_SYNC);
                }else if(event_id == BT_CODEC_MEDIA_LEQ_PARA_SEND){
                    am_voice_sent_leq_gain();
                }
            }
        } else {
            //printf("[NDVC] It's Partner, don't send avc_vol");
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_stream_out_callback
 * DESCRIPTION
 *  This callback function is notified A.M. for stream-out by AUD HISR.
 * PARAMETERS
 *  event            [OUT]
 *  user_data        [OUT]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_stream_out_callback(bt_sink_srv_am_event_result_t event, void *user_data)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prCurrent_player == NULL) {
        return;
    }
    if (g_prCurrent_player->type == PCM) {
        g_prCurrent_player->local_context.pcm_format.event = event;
        bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_STREAM_EVENT_IND,
                                 MSG_ID_MEDIA_EVENT_STREAM_OUT_CALL_EXT_REQ,
                                 g_prCurrent_player,
                                 TRUE, ptr_callback_amm);
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_stream_in_callback
 * DESCRIPTION
 *  This callback function is notified A.M. for stream-in by AUD HISR.
 * PARAMETERS
 *  event            [OUT]
 *  user_data        [OUT]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_stream_in_callback(bt_sink_srv_am_event_result_t event, void *user_data)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prCurrent_player == NULL) {
        return;
    }
    if (g_prCurrent_player->type == PCM) {
        g_prCurrent_player->local_context.pcm_format.event = event;
        bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_STREAM_EVENT_IND,
                                 MSG_ID_MEDIA_EVENT_STREAM_IN_CALL_EXT_REQ,
                                 g_prCurrent_player,
                                 TRUE, ptr_callback_amm);
    }
}

/*****************************************************************************
 * FUNCTION
 *  aud_process_a2dp_callback_hdlr
 * DESCRIPTION
 *  This function is used to handle A2DP process callback.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_process_a2dp_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_bt_event_t event_id = amm_ptr->background_info.local_context.a2dp_format.a2dp_event;
    //bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
#ifndef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
#endif
    bt_sink_srv_am_device_set_t dev;
    bt_sink_srv_am_volume_level_out_t vol;
    uint32_t digital_vol = 0x7FFF, analog_vol = 0x0002;
#ifdef __BT_AWS_SUPPORT__
    uint32_t sampling_rate = 44100;
#endif

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prA2dp_sink_handle != NULL) {
        g_prA2dp_sink_handle->process(g_prA2dp_sink_handle, event_id);

        switch (event_id) {
            case BT_CODEC_MEDIA_ERROR: {
#ifdef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
                //g_prA2dp_sink_handle->stop(g_prA2dp_sink_handle);
                //g_rSink_state = A2DP_SINK_CODEC_STOP;
                dev = g_prCurrent_player->audio_stream_out.audio_device;
                vol = g_prCurrent_player->audio_stream_out.audio_volume;
                hal_audio_set_stream_out_device(dev);
                if (dev & DEVICE_LOUDSPEAKER) {
                    digital_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else if (dev & DEVICE_EARPHONE) {
                    digital_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else {
                    ;
                }
                if (g_am_volume_enable) {
                    audio_src_srv_report("[sink][AM]-d_gain: 0x%x, a_gain: 0x%x", 2, digital_vol, analog_vol);
                } else {
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
                    {
                        bt_sink_srv_audio_setting_vol_info_t vol_info;

                        vol_info.type = VOL_A2DP;
                        vol_info.vol_info.a2dp_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                        vol_info.vol_info.a2dp_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;
                        bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                    }
#else
                    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_vol, analog_vol);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */
                }
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_CODEC_RESTART, NULL);
#else
                g_prA2dp_sink_handle->stop(g_prA2dp_sink_handle);
                g_rSink_state = A2DP_SINK_CODEC_STOP;
                g_prA2dp_sink_handle->reset_share_buffer(g_prA2dp_sink_handle);
                dev = g_prCurrent_player->audio_stream_out.audio_device;
                vol = g_prCurrent_player->audio_stream_out.audio_volume;
                hal_audio_set_stream_out_device(dev);
                if (dev & DEVICE_LOUDSPEAKER) {
                    digital_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else if (dev & DEVICE_EARPHONE) {
                    digital_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else {
                    ;
                }
                if (g_am_volume_enable) {
                    ;
                } else {
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
                    {
                        bt_sink_srv_audio_setting_vol_info_t vol_info;

                        vol_info.type = VOL_A2DP;
                        vol_info.vol_info.a2dp_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                        vol_info.vol_info.a2dp_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;
                        bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                    }
#else
                    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_vol, analog_vol);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */
                }

                //eResult = g_prA2dp_sink_handle->play(g_prA2dp_sink_handle);
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_CODEC_RESTART, NULL);
                if (eResult == BT_CODEC_MEDIA_STATUS_ERROR) {
                    audio_src_srv_report("[sink][AM] Error", 0);
                }
                audio_src_srv_report("[sink][AM]-restart play", 0);
#endif /* __BT_SINK_SRV_AM_MED_LIST_SUPPORT__ */
                break;
            }

#ifdef __BT_AWS_SUPPORT__
            case BT_CODEC_MEDIA_AWS_CHECK_UNDERFLOW: {
                bt_sink_srv_fetch_bt_offset();
                break;
            }

            case BT_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW: {
                if (BT_AWS_CODEC_TYPE_SBC == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 8:
                            sampling_rate = 16000;
                            break;
                        case 4:
                            sampling_rate = 32000;
                            break;
                        case 2:
                            sampling_rate = 44100;
                            break;
                        case 1:
                            sampling_rate = 48000;
                            break;

                        default:
                            sampling_rate = 44100;
                            break;
                    }
                } else if (BT_AWS_CODEC_TYPE_AAC == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 0x800:
                            sampling_rate = 8000;
                            break;
                        case 0x400:
                            sampling_rate = 11025;
                            break;
                        case 0x200:
                            sampling_rate = 12000;
                            break;
                        case 0x100:
                            sampling_rate = 16000;
                            break;
                        case 0x80:
                            sampling_rate = 22050;
                            break;
                        case 0x40:
                            sampling_rate = 24000;
                            break;
                        case 0x20:
                            sampling_rate = 32000;
                            break;
                        case 0x10:
                            sampling_rate = 44100;
                            break;
                        case 0x8:
                            sampling_rate = 48000;
                            break;
                        default:
                            sampling_rate = 44100;
                            break;
                    }
                } else if (BT_AWS_CODEC_TYPE_VENDOR == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 0x20:
                            sampling_rate = 44100;
                            break;
                        case 0x10:
                            sampling_rate = 48000;
                            break;
                        case 0x08:
                            sampling_rate = 88200;
                            break;
                        case 0x04:
                            sampling_rate = 96000;
                            break;
                        case 0x02:
                            sampling_rate = 176400;
                            break;
                        case 0x01:
                            sampling_rate = 192000;
                            break;
                        default:
                            sampling_rate = 44100;
                            break;
                    }
                }
                bt_sink_srv_audio_sync_calc_t audio_calc;
                audio_calc.media_handle = g_prA2dp_sink_handle;
                audio_calc.sampling_rate = sampling_rate;
                audio_calc.type = g_aws_codec_type;
                bt_sink_srv_audio_clock_calibrate(&audio_calc);
                break;
            }
#endif /* __BT_AWS_SUPPORT__ */

            case BT_CODEC_MEDIA_UNDERFLOW: {
                g_a2dp_underflow_loop_count++;
                if (g_a2dp_underflow_loop_count >= BT_SINK_SRV_AM_MAX_UNDERFLOW_COUNT) {
                    g_a2dp_underflow_loop_count = 0;
                    g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_AWS_UNDERFLOW, NULL);
                }
                break;
            }

#if defined(MTK_AVM_DIRECT)
            case BT_CODEC_MEDIA_TIME_REPORT: {
                audio_dsp_a2dp_dl_time_param_t *p_param = hal_audio_a2dp_dl_get_time_report();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_TIME_REPORT, p_param);
                return;  // avoid to callback twice
            }

            case BT_CODEC_MEDIA_LTCS_DATA_REPORT: {
                audio_dsp_a2dp_ltcs_report_param_t param;
                param.p_ltcs_asi_buf = hal_audio_query_ltcs_asi_buf();
                param.p_ltcs_min_gap_buf = hal_audio_query_ltcs_min_gap_buf();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_LTCS_REPORT, &param);
                return;
            }

            case BT_CODEC_MEDIA_AUDIO_DL_REINIT_REQUEST: {
                uint32_t param_reinit = hal_audio_dsp2mcu_data_get();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_DL_REINIT_REQUEST, &param_reinit);
                break;
            }

            case BT_CODEC_MEDIA_AUDIO_DL_ALC_REQUEST: {
                uint32_t ALC_latency = hal_audio_dsp2mcu_AUDIO_DL_ACL_data_get();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_ACTIVE_LATENCY_REQUEST, &ALC_latency);
                return;
            }
#endif

            default:
                break;
        }
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, (bt_sink_srv_am_cb_sub_msg_t)event_id, NULL);
        _UNUSED(digital_vol);
        _UNUSED(analog_vol);
    }
}

#ifdef AIR_BT_HFP_ENABLE
/*****************************************************************************
 * FUNCTION
 *  aud_process_hfp_callback_hdlr
 * DESCRIPTION
 *  This function is used to inform HF AP about hf error event.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_process_hfp_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_bt_event_t event_id = amm_ptr->background_info.local_context.hfp_format.hfp_event;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prA2dp_sink_handle != NULL) {
        if (event_id == BT_CODEC_MEDIA_ERROR) {
            bt_codec_am_hfp_stop();
        }
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_HFP_EVENT_IND, (bt_sink_srv_am_cb_sub_msg_t)event_id, NULL);
    }
}
#endif

/*****************************************************************************
 * FUNCTION
 *  aud_event_stream_callback_hdlr
 * DESCRIPTION
 *  This function is used to handle stream event callback.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_event_stream_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_event_result_t eEvent = amm_ptr->background_info.local_context.pcm_format.event;
    bt_sink_srv_am_id_t bAud_id = amm_ptr->background_info.aud_id;
    bt_sink_srv_am_stream_type_t    eIn_out;
    uint32_t                    data_length = 0;
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((g_prCurrent_player == NULL) || (bAud_id != g_prCurrent_player->aud_id)) {
        return;
    }
    if (g_prCurrent_player->type == PCM) {
        eIn_out = g_prCurrent_player->local_context.pcm_format.in_out;
        if (eEvent == HAL_AUDIO_EVENT_UNDERFLOW) {
            if (eIn_out == STREAM_OUT) {
                hal_audio_stop_stream_out();
            } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
                hal_audio_stop_stream_in();
#endif
            }
            g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_IDLE_STATE;
            g_prCurrent_player->notify_cb(bAud_id, AUD_STREAM_EVENT_IND, AUD_STREAM_EVENT_UNDERFLOW, NULL);
        } else if (eEvent == HAL_AUDIO_EVENT_DATA_REQUEST) {
            if (eIn_out == STREAM_OUT) {
                hal_audio_get_stream_out_sample_count(&data_length);
            } else if (eIn_out == STREAM_IN) {
#ifdef AIR_RECORD_ENABLE
                hal_audio_get_stream_in_sample_count(&data_length);
#endif
            }
            g_prCurrent_player->notify_cb(bAud_id, AUD_STREAM_EVENT_IND, AUD_STREAM_EVENT_DATA_REQ, &data_length);
        } else if (eEvent == HAL_AUDIO_EVENT_ERROR) {
            g_prCurrent_player->notify_cb(bAud_id, AUD_STREAM_EVENT_IND, AUD_STREAM_EVENT_ERROR, NULL);
        } else if (eEvent == HAL_AUDIO_EVENT_NONE) {
            g_prCurrent_player->notify_cb(bAud_id, AUD_STREAM_EVENT_IND, AUD_STREAM_EVENT_UNDERFLOW, NULL);
        } else if (eEvent == HAL_AUDIO_EVENT_DATA_NOTIFICATION) {
            g_prCurrent_player->notify_cb(bAud_id, AUD_STREAM_EVENT_IND, AUD_STREAM_EVENT_DATA_NOTIFICATION, NULL);
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *  bt_get_write_buffer_a2dp_sink_codec
 * DESCRIPTION
 *  This function is used to process "get_write_buffer" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 *  buffer           [OUT]
 *  length           [OUT]
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
static bt_status_t bt_get_write_buffer_a2dp_sink_codec(bt_sink_srv_am_id_t aud_id, uint8_t **buffer, uint32_t *length)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (aud_id == g_prCurrent_player->aud_id) {
        g_prA2dp_sink_handle->get_write_buffer(g_prA2dp_sink_handle, buffer, length);
        return BT_CODEC_MEDIA_STATUS_OK;
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  bt_write_data_done_a2dp_sink_codec
 * DESCRIPTION
 *  This function is used to process "write data done" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 *  length           [IN]
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
static bt_status_t bt_write_data_done_a2dp_sink_codec(bt_sink_srv_am_id_t aud_id, uint32_t length)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (aud_id == g_prCurrent_player->aud_id) {
        g_prA2dp_sink_handle->write_data_done(g_prA2dp_sink_handle, length);
        return BT_CODEC_MEDIA_STATUS_OK;
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  bt_finish_write_data_a2dp_sink_codec
 * DESCRIPTION
 *  This function is used to process "finish_write_data" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 *  length           [IN]
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
static bt_status_t bt_finish_write_data_a2dp_sink_codec(bt_sink_srv_am_id_t aud_id)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (aud_id == g_prCurrent_player->aud_id) {
        g_prA2dp_sink_handle->finish_write_data(g_prA2dp_sink_handle);
        return BT_CODEC_MEDIA_STATUS_OK;
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  bt_get_free_space_a2dp_sink_codec
 * DESCRIPTION
 *  This function is used to process "get_free_space" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 *  length           [IN]
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t bt_get_free_space_a2dp_sink_codec(bt_sink_srv_am_id_t aud_id)
{
    if (aud_id == g_prCurrent_player->aud_id) {
        return g_prA2dp_sink_handle->get_free_space(g_prA2dp_sink_handle);
    }
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *  bt_reset_share_buffer_a2dp_sink_codec
 * DESCRIPTION
 *  This function is used to process "reset_share_buffer" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void bt_reset_share_buffer_a2dp_sink_codec(bt_sink_srv_am_id_t aud_id)
{
    if (aud_id == g_prCurrent_player->aud_id) {
        g_prA2dp_sink_handle->reset_share_buffer(g_prA2dp_sink_handle);
    }
}


#if defined(MTK_AVM_DIRECT)
/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_a2dp_sink_open_callback
 * DESCRIPTION
 *  This function is used to callback after A2DP sink open for asynchronous notification.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void aud_process_a2dp_open_callback_hdlr(void)
{
    audio_src_srv_report("[sink][AM]a2dp sink open codec success, send event to host", 0);
    g_rSink_state = A2DP_SINK_CODEC_OPEN;
    g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
    if (g_prCurrent_player->type == A2DP) {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, &g_prA2dp_sink_event_handle);
    } else if (g_prCurrent_player->type == AWS) {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, &g_prA2dp_sink_event_handle);
    } else {
        /* trace error */
    }
}

#ifdef AIR_BT_SINK_MUSIC_ENABLE
static void bt_codec_am_a2dp_sink_open_callback(void)
{
    aud_process_a2dp_open_callback_hdlr();
}
#endif

#endif

/*****************************************************************************
 * FUNCTION
 *  aud_dl_suspend
 * DESCRIPTION
 *  This function is used to suspend DL 1
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void aud_dl_suspend(void)
{
#if defined(MTK_AVM_DIRECT)
    if (!(g_am_task_mask & AM_TASK_MASK_DL_SUSPEND)) {
        audio_src_srv_report("[Sink][AM]aud_dl_suspend\n", 0);
        ami_set_audio_mask(AM_TASK_MASK_DL_SUSPEND, true);
        if ((g_prCurrent_player->type == A2DP) || (g_prCurrent_player->type == AWS)) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_AUDIO_DL_SUSPEND, 0, 0, true);
        } else if ((g_prCurrent_player->type == HFP) || (g_prCurrent_player->type == PCM)) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_SUSPEND, 0, 0, true);
        } else if (g_prCurrent_player->type == LINE_IN) {
#ifdef MTK_LINE_IN_ENABLE
#if defined(MTK_PURE_LINEIN_PLAYBACK_ENABLE)
            pure_linein_playback_close();
#else
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            if (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable) {
                pure_linein_playback_close();
            } else {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_SUSPEND, 0, 0, true);
            }
#else
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_SUSPEND, 0, 0, true);
#endif
#endif
#endif
        } else if (g_prCurrent_player->type == USB_AUDIO_IN) {
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_SUSPEND, 0, 0, true);
#endif
        } else if ((g_prCurrent_player->type == BLE)||(g_prCurrent_player->type == ULL_BLE_DL)) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_SUSPEND, 0, 0, true);
        }
    } else {
        audio_src_srv_report("[Sink][AM]aud_dl_suspend Fail: Already suspended\n", 0);
    }
#endif
}

static void aud_ul_suspend(void)
{
#if defined(MTK_AVM_DIRECT)
    if (!(g_am_task_mask & AM_TASK_MASK_UL_SUSPEND)) {
        audio_src_srv_report("[Sink][AM]aud_ul_suspend\n", 0);
        ami_set_audio_mask(AM_TASK_MASK_UL_SUSPEND, true);
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_SUSPEND, 0, 0, true);
        } else if (g_prCurrent_player->type == HFP) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_UL_SUSPEND, 0, 0, true);
#ifdef AIR_BT_CODEC_BLE_ENABLED
        } else if (g_prCurrent_player->type == BLE && g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_SUSPEND, 0, 0, true);
#endif
        }
    } else {
        audio_src_srv_report("[Sink][AM]aud_ul_suspend Fail: Already suspended\n", 0);
    }
#endif
}

extern audio_common_t audio_common;
/*****************************************************************************
 * FUNCTION
 *  aud_dl_resume
 * DESCRIPTION
 *  This function is used to resume DL 1
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void aud_dl_resume(void)
{
#if defined(MTK_AVM_DIRECT)
    if (g_am_task_mask & AM_TASK_MASK_DL_SUSPEND) {
        audio_src_srv_report("[Sink][AM]aud_dl_resume\n", 0);
        ami_set_audio_mask(AM_TASK_MASK_DL_SUSPEND, false);
        /*Change dl1 resume control for g_prCurrent_player = NULL*/
        if (g_prCurrent_player != NULL) {
            if ((g_prCurrent_player->type == A2DP) || (g_prCurrent_player->type == AWS)) {
                am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_AUDIO_DL_RESUME, 0, 0, true);
            } else if ((g_prCurrent_player->type == HFP) || (g_prCurrent_player->type == PCM)) {
                if (g_prCurrent_player->type == HFP) {
                    am_set_HFP_volume(g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                                        g_prCurrent_player->audio_stream_in.audio_device,
                                        g_prCurrent_player->audio_stream_in.audio_volume,
                                        g_prCurrent_player->audio_stream_out.audio_device,
                                        g_prCurrent_player->audio_stream_out.audio_volume,
                                        false, true);
                } else if (g_prCurrent_player->type == PCM) {
                    am_set_PCM_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                }

                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_RESUME, 0, 0, true);
            } else if (g_prCurrent_player->type == LINE_IN) {
#ifdef MTK_LINE_IN_ENABLE
#if defined(MTK_PURE_LINEIN_PLAYBACK_ENABLE)
                pure_linein_playback_open(g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.linein_sample_rate,
                                          g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.in_audio_device,
                                          g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.out_audio_device);
#else
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
                if (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable) {
                    pure_linein_playback_open(g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.linein_sample_rate,
                                              g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.in_audio_device,
                                              g_prCurrent_player->local_context.line_in_format.line_in_codec.codec_cap.out_audio_device);
                } else {
                    am_set_LINE_IN_volume(VOL_LINE_IN, g_prCurrent_player->audio_stream_in.audio_device, g_prCurrent_player->audio_stream_in.audio_volume,
                                        g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume, true, true);
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_RESUME, 0, 0, true);
                }
#else
                am_set_LINE_IN_volume(VOL_LINE_IN, g_prCurrent_player->audio_stream_in.audio_device, g_prCurrent_player->audio_stream_in.audio_volume,
                                    g_prCurrent_player->audio_stream_out.audio_device,g_prCurrent_player->audio_stream_out.audio_volume, true, true);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_RESUME, 0, 0, true);
#endif
#endif
#endif
            } else if (g_prCurrent_player->type == USB_AUDIO_IN) {
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
                am_set_USB_AUDIO_IN_volume(VOL_USB_AUDIO_IN, g_prCurrent_player->audio_stream_out.audio_device,  g_prCurrent_player->audio_stream_out.audio_volume);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_RESUME, 0, 0, true);
#endif
            } else if ((g_prCurrent_player->type == BLE)|| (g_prCurrent_player->type == ULL_BLE_DL)) {
#ifdef AIR_BT_CODEC_BLE_ENABLED
                if (g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY) {
                    am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC,
                                        g_prCurrent_player->audio_stream_in.audio_device, g_prCurrent_player->audio_stream_in.audio_volume,
                                        g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume,
                                        true,true);
                } else {
                    am_set_A2DP_volume(g_prCurrent_player->audio_stream_out.audio_device, g_prCurrent_player->audio_stream_out.audio_volume);
                }
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_RESUME, 0, 0, true);
#endif
            }
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
            ami_set_afe_param(STREAM_OUT, audio_common.stream_out.stream_sampling_rate, true);
#endif
        } else {
            if (g_prHfp_media_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_RESUME, 0, 0, true);
            } else if (g_prA2dp_sink_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_AUDIO_DL_RESUME, 0, 0, true);
            }
#ifdef MTK_LINE_IN_ENABLE
            else if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN)) {
#if defined(MTK_PURE_LINEIN_PLAYBACK_ENABLE)
                pure_linein_playback_open(g_prLineIN_sink_handle.codec_cap.linein_sample_rate, g_prLineIN_sink_handle.codec_cap.in_audio_device, g_prLineIN_sink_handle.codec_cap.out_audio_device);
#else
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
                if (0x01 == audio_nvdm_HW_config.audio_scenario.Audio_Pure_Linein_enable) {
                    pure_linein_playback_open(g_prLineIN_sink_handle.codec_cap.linein_sample_rate, g_prLineIN_sink_handle.codec_cap.in_audio_device, g_prLineIN_sink_handle.codec_cap.out_audio_device);
                } else {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_RESUME, 0, 0, true);
                }
#else
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LINEIN_PLAYBACK_RESUME, 0, 0, true);
#endif
#endif
            }
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
            else if (g_prBle_media_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_RESUME, 0, 0, true);
            }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
            else if (g_prUllBleDl_media_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_RESUME, 0, 0, true);
            }
#endif
#endif
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
            else if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_USB_AUDIO_PLAYBACK)) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_RESUME, 0, 0, true);
            }
#endif
            else {
                audio_src_srv_report("[Sink][AM]aud_dl_resume Fail: HFP & A2DP & BLE handler was NULL.\n", 0);
            }
        }
    } else {
        audio_src_srv_report("[Sink][AM]aud_dl_resume Fail: Have not suspended\n", 0);
    }
#endif
}

static void aud_ul_resume(void)
{
#if defined(MTK_AVM_DIRECT)
    if (g_am_task_mask & AM_TASK_MASK_UL_SUSPEND) {
        audio_src_srv_report("[Sink][AM]aud_ul_resume\n", 0);
        ami_set_audio_mask(AM_TASK_MASK_UL_SUSPEND, false);
        /*Change dl1 resume control for g_prCurrent_player = NULL*/
        if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_RESUME, 0, 0, true);
        } else if (g_prCurrent_player != NULL) {
            if (g_prCurrent_player->type == HFP
#ifdef AIR_BT_CODEC_BLE_ENABLED
            || (g_prCurrent_player->type == BLE && g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY)
#endif
            ) {
                am_set_HFP_volume(g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                                        g_prCurrent_player->audio_stream_in.audio_device,
                                        g_prCurrent_player->audio_stream_in.audio_volume,
                                        g_prCurrent_player->audio_stream_out.audio_device,
                                        g_prCurrent_player->audio_stream_out.audio_volume,
                                        true, false);
                if (g_prCurrent_player->type == HFP) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_UL_RESUME, 0, 0, true);
                } else if(g_prCurrent_player->type == BLE) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_RESUME, 0, 0, true);
                }
            }
        } else {
            if (g_prHfp_media_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_UL_RESUME, 0, 0, true);
#ifdef AIR_BT_CODEC_BLE_ENABLED
            } else if (g_prBle_media_handle != NULL) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_RESUME, 0, 0, true);
#endif
            } else {
                audio_src_srv_report("[Sink][AM]aud_ul_resume Fail: HFP handler was NULL.\n", 0);
            }
        }
    } else {
        audio_src_srv_report("[Sink][AM]aud_ul_resume Fail: Have not suspended\n", 0);
    }
#endif
}

/*****************************************************************************
 * FUNCTION
 *  aud_dl_control
 * DESCRIPTION
 *  This function is used to contorl DL 1 suspend / resume when side tone enable / disable
 * PARAMETERS
 *  isCodecOpen     [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_dl_control(bool isCodecOpen)
{
    if ((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)
        || (g_am_task_mask & AM_TASK_MASK_VP_HAPPENING)
        || (g_am_task_mask & AM_TASK_MASK_DL1_HAPPENING)) {
        audio_src_srv_report("[Sink][AM]aud_dl_control: g_am_task_mask = 0x%08x ", 1, g_am_task_mask);
        if (isCodecOpen) {
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            if (((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) && !((g_prCurrent_player->type == HFP) || (g_prCurrent_player->type == ULL_BLE_DL) || (g_prCurrent_player->type == BLE) || (g_prCurrent_player->type == PCM)))) {

                aud_dl_suspend();
            } else
#endif
                if (g_am_task_mask & AM_TASK_MASK_VP_HAPPENING) {
                    aud_dl_suspend();
                }
        } else {
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) {
                aud_dl_resume();
            } else
#endif
                if (g_am_task_mask & AM_TASK_MASK_DL1_HAPPENING) {
                    aud_dl_resume();
                }
        }
    }
}

#if defined(AIR_BT_CODEC_BLE_ENABLED) || defined(AIR_BT_HFP_ENABLE) || defined(AIR_RECORD_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static void aud_ul_control(bool isCodecOpen)
{
    if ((g_am_task_mask & AM_TASK_MASK_VP_HAPPENING)
        || (g_am_task_mask & AM_TASK_MASK_UL1_HAPPENING)) {
        audio_src_srv_report("[Sink][AM]aud_ul_control: g_am_task_mask = 0x%08x", 1, g_am_task_mask);
        if (isCodecOpen) {
            if (g_prCurrent_player && (g_prCurrent_player->type == HFP || g_prCurrent_player->type == BLE)
                && (g_am_task_mask & AM_TASK_MASK_VP_HAPPENING)) {
                aud_ul_suspend();
            }
        } else {
            if (g_am_task_mask & AM_TASK_MASK_UL1_HAPPENING) {
                aud_ul_resume();
            }
        }
    }
}
#endif

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_a2dp_sink_open
 * DESCRIPTION
 *  This function is used to open codec for A2DP sink by BT APP
 * PARAMETERS
 *  a2dp_codec_t     [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void bt_codec_am_a2dp_sink_open(bt_sink_srv_am_a2dp_codec_t *a2dp_codec_t)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
#ifdef AIR_BT_SINK_MUSIC_ENABLE
    bt_codec_a2dp_audio_t a2dp_codec = {{0}};
#endif
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    AUD_LOG_W("[AM]Open A2DP codec = %d.", 1, a2dp_codec_t->role);
    switch (a2dp_codec_t->role) {
#ifdef AIR_BT_SINK_MUSIC_ENABLE
        case BT_A2DP_SINK:
            a2dp_codec.role = a2dp_codec_t->role;
            if (a2dp_codec_t->codec_cap.type == BT_A2DP_CODEC_SBC) {
                BT_A2DP_CONVERT_SBC_CODEC(&(a2dp_codec.codec_cap), &(a2dp_codec_t->codec_cap));
                //AUD_LOG_W("[AM]Open A2DP SBC codec.", 0);
#ifdef __BT_AWS_SUPPORT__
                g_aws_codec_type = BT_AWS_CODEC_TYPE_SBC;
                g_aws_sample_rate = a2dp_codec.codec_cap.codec.sbc.sample_rate;
#endif /* __BT_AWS_SUPPORT__ */
            } else if (a2dp_codec_t->codec_cap.type == BT_A2DP_CODEC_AAC) {
                BT_A2DP_CONVERT_AAC_CODEC(&(a2dp_codec.codec_cap), &(a2dp_codec_t->codec_cap));
                //AUD_LOG_W("[AM]Open A2DP AAC codec.", 0);
#ifdef __BT_AWS_SUPPORT__
                g_aws_codec_type = BT_AWS_CODEC_TYPE_AAC;
                g_aws_sample_rate = a2dp_codec.codec_cap.codec.aac.sample_rate;
#endif /* __BT_AWS_SUPPORT__ */
            } else if (a2dp_codec_t->codec_cap.type == BT_A2DP_CODEC_VENDOR) {
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
                BT_A2DP_CONVERT_VENDOR_CODEC(&(a2dp_codec.codec_cap), &(a2dp_codec_t->codec_cap));
                //AUD_LOG_W("[AM]Open A2DP VENDOR codec.", 0);
#ifdef __BT_AWS_SUPPORT__
                g_aws_codec_type = BT_AWS_CODEC_TYPE_VENDOR;
                g_aws_sample_rate = a2dp_codec.codec_cap.codec.vendor.sample_rate;
#endif /* __BT_AWS_SUPPORT__ */
#else
                //audio_src_srv_report("[sink][AM]a2dp sink open Vendor not open", 0);
                AUD_LOG_W("[AM][error]Open A2DP VENDOR codec error(define not open).", 0);
#endif
            } else if (a2dp_codec_t->codec_cap.type == 0x65) { //BT_A2DP_CODEC_AIRO_CELT=0x65
                //BT_A2DP_CONVERT_AIRO_CELT_CODEC(&(a2dp_codec.codec_cap), &(a2dp_codec_t->codec_cap));
                a2dp_codec.codec_cap.type = 0x65;//BT_A2DP_CODEC_AIRO_CELT=0x65;
                //AUD_LOG_W("[AM]Open A2DP AIRO_CELT codec.", 0);
#ifdef __BT_AWS_SUPPORT__  //TODO
                g_aws_codec_type = BT_AWS_CODEC_TYPE_SBC;
                g_aws_sample_rate = a2dp_codec.codec_cap.codec.sbc.sample_rate;
#endif /* __BT_AWS_SUPPORT__ */
            } else {
                //audio_src_srv_report("[sink][AM][error]a2dp sink open codec error codec not support.", 0);
                AUD_LOG_W("[AM][error]Open A2DP codec error(not support).", 0);
            }
#if defined(MTK_AVM_DIRECT)
            hal_audio_am_register_a2dp_open_callback(bt_codec_am_a2dp_sink_open_callback);
#endif

            g_prA2dp_sink_handle = bt_codec_a2dp_open(aud_bt_codec_a2dp_callback, &a2dp_codec);
            audio_src_srv_report("[sink][AM]a2dp sink open: g_prA2dp_sink_handle:0x%x", 1, g_prA2dp_sink_handle);
            AUDIO_ASSERT(g_prA2dp_sink_handle);
#if defined(MTK_AVM_DIRECT)
            aud_dl_control(true);
#endif
            break;

        default:
            audio_src_srv_report("[sink][AM]a2dp sink open: undefined role:%d", 1, a2dp_codec_t->role);
            break;
#endif
    }
    if (g_bBT_Ringbuf == NULL) {
#ifndef WIN32_UT
        //g_bBT_Ringbuf = (uint8_t *)pvPortMalloc(AM_RING_BUFFER_SIZE * sizeof(uint8_t));
        g_bBT_Ringbuf = NULL;
#ifndef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
        g_bBT_Ringbuf = g_bt_sink_srv_am_ring_buffer;
#endif
#else
        g_bBT_Ringbuf = (uint8_t *)malloc(AM_RING_BUFFER_SIZE * sizeof(uint8_t));
#endif
    }
#ifdef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
    g_prA2dp_sink_handle->set_get_data_function(g_prA2dp_sink_handle, bt_sink_srv_media_mgr_read_data);
    g_prA2dp_sink_handle->set_get_data_count_function(g_prA2dp_sink_handle, bt_sink_srv_media_mgr_get_data_count);
#else
    g_prA2dp_sink_handle->set_buffer(g_prA2dp_sink_handle, g_bBT_Ringbuf, AM_RING_BUFFER_SIZE);
#endif /* __BT_SINK_SRV_AM_MED_LIST_SUPPORT__ */
    g_prA2dp_sink_event_handle.get_write_buffer = bt_get_write_buffer_a2dp_sink_codec;
    g_prA2dp_sink_event_handle.write_data_done = bt_write_data_done_a2dp_sink_codec;
    g_prA2dp_sink_event_handle.finish_write_data = bt_finish_write_data_a2dp_sink_codec;
    g_prA2dp_sink_event_handle.get_free_space = bt_get_free_space_a2dp_sink_codec;
    g_prA2dp_sink_event_handle.reset_share_buffer = bt_reset_share_buffer_a2dp_sink_codec;
    g_prA2dp_sink_event_handle.play = bt_codec_am_a2dp_sink_play;
    g_prA2dp_sink_event_handle.stop = bt_codec_am_a2dp_sink_stop;
    g_prA2dp_sink_event_handle.med_hd = g_prA2dp_sink_handle;
#ifdef __BT_AWS_SUPPORT__
    g_prA2dp_sink_event_handle.set_aws_flag = bt_a2dp_aws_set_flag;
    g_prA2dp_sink_event_handle.set_aws_initial_sync = bt_a2dp_aws_set_initial_sync;
    g_prA2dp_sink_event_handle.aws_plh_init = bt_a2dp_aws_plh_init;
    g_prA2dp_sink_event_handle.aws_plh_deinit = bt_a2dp_aws_plh_deinit;
#endif /* __BT_AWS_SUPPORT__ */
#if defined(MTK_AVM_DIRECT)
    // callback will be done when DSP ack open command.
#else
    g_rSink_state = A2DP_SINK_CODEC_OPEN;
    g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
    audio_src_srv_report("[sink][AM]a2dp sink open: type:%d, handle:0x%x", 2, g_prCurrent_player->type, &g_prA2dp_sink_event_handle);
    if (g_prCurrent_player->type == A2DP) {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, &g_prA2dp_sink_event_handle);
    } else if (g_prCurrent_player->type == AWS) {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_SINK_OPEN_CODEC, AUD_SINK_PROC_PTR, &g_prA2dp_sink_event_handle);
    } else {
        /* trace error */
        audio_src_srv_report("[sink][AM]a2dp sink open error", 0);
    }
#endif

}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_a2dp_sink_play
 * DESCRIPTION
 *  This function is used to process "play" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_a2dp_sink_play(bt_sink_srv_am_id_t aud_id)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
    bt_sink_srv_am_device_set_t dev;
#if 0
    bt_sink_srv_am_volume_level_out_t vol;
    uint32_t digital_vol = 0x7FFF, analog_vol = 0x0002;
#endif
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prCurrent_player == NULL) {
        audio_src_srv_report("[sink][AM]sink_play--aud_id: %d, g_prCurrent_player is NULL, return error", 1, aud_id);
        return BT_CODEC_MEDIA_STATUS_ERROR;
    }
    // audio_src_srv_report("[sink][AM]sink_play--aud_id: %d, aid: %d, g_prCurrent_player: 0x%x, g_prA2dp_sink_handle: 0x%x", 4,
    //                      aud_id, g_prCurrent_player->aud_id, g_prCurrent_player, g_prA2dp_sink_handle);

    if ((g_prCurrent_player != NULL) && (aud_id == g_prCurrent_player->aud_id)) {
        if (g_prA2dp_sink_handle != NULL) {
            dev = g_prCurrent_player->audio_stream_out.audio_device;
            //vol = g_prCurrent_player->audio_stream_out.audio_volume;
            hal_audio_set_stream_out_device(dev);
            //To save set volume time. BT sink must set volume before playing.
            /*if (dev & DEVICE_LOUDSPEAKER) {
                digital_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                analog_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
            } else if (dev & DEVICE_EARPHONE) {
                digital_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                analog_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
            } else {
                ;
            }
            if (g_am_volume_enable) {
                audio_src_srv_report("[sink][AM]play-d_gain: 0x%x, a_gain: 0x%x",2, digital_vol, analog_vol);
            } else {
                #ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
                {
                    bt_sink_srv_audio_setting_vol_info_t vol_info;

                    vol_info.type = VOL_A2DP;
                    vol_info.vol_info.a2dp_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                    vol_info.vol_info.a2dp_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;
                    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                }
                #else
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_vol, analog_vol);
                #endif //__BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            }*/
#ifdef __BT_SINK_SRV_AUDIO_TUNING__
            aud_tunning_p = bt_sink_srv_audio_tunning_get_context();
            if ((aud_tunning_p->flag & TUNNING_FLAG_INIT) &&
                (aud_tunning_p->flag & TUNNING_FLAG_ON)) {
                bt_sink_srv_audio_tunning_update(TUNNING_SCENARIO_A2DP, TUNNING_TYPE_VOL);
                bt_sink_srv_audio_tunning_update(TUNNING_SCENARIO_A2DP, TUNNING_TYPE_DEV);
            }
#endif /* __BT_SINK_SRV_AUDIO_TUNING__ */
            eResult = g_prA2dp_sink_handle->play(g_prA2dp_sink_handle);
            if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
                g_rSink_state = A2DP_SINK_CODEC_PLAY;
                return BT_CODEC_MEDIA_STATUS_OK;
            }
        }
    }

    audio_src_srv_report("[sink][AM]sink_play--aud_id: %d, aid: %d, g_prCurrent_player: 0x%x, g_prA2dp_sink_handle: 0x%x", 4,
                         aud_id, g_prCurrent_player->aud_id, g_prCurrent_player, g_prA2dp_sink_handle);

    return BT_CODEC_MEDIA_STATUS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_a2dp_sink_stop
 * DESCRIPTION
 *  This function is used to process "play" for A2DP sink by BT APP.
 * PARAMETERS
 *  aud_id           [IN]
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_a2dp_sink_stop(bt_sink_srv_am_id_t aud_id)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (aud_id == g_prCurrent_player->aud_id) {
        if (g_prA2dp_sink_handle != NULL) {
            eResult = g_prA2dp_sink_handle->stop(g_prA2dp_sink_handle);
            if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
                g_prA2dp_sink_handle->reset_share_buffer(g_prA2dp_sink_handle);
                g_rSink_state = A2DP_SINK_CODEC_STOP;
                return BT_CODEC_MEDIA_STATUS_OK;
            }
        }
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_a2dp_sink_close
 * DESCRIPTION
 *  This function is used to process "close" for A2DP sink by BT APP.
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
#ifdef AIR_BT_SINK_MUSIC_ENABLE
extern uint32_t g_Rdebug_a2dp_ltcs_last_time;
#endif
bt_status_t bt_codec_am_a2dp_sink_close(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
#ifdef AIR_BT_SINK_MUSIC_ENABLE
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
#endif
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prA2dp_sink_handle != NULL) {
#if defined(MTK_AVM_DIRECT)
        aud_dl_control(false);
#endif
#ifdef AIR_BT_SINK_MUSIC_ENABLE
        eResult = bt_codec_a2dp_close(g_prA2dp_sink_handle);
        /* for A2DP LTCS debug*/
        g_Rdebug_a2dp_ltcs_last_time = 0;

        if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
            g_prA2dp_sink_handle = NULL;
            g_rSink_state = A2DP_SINK_CODEC_CLOSE;
            if (g_bBT_Ringbuf != NULL) {
#ifndef WIN32_UT
                //vPortFree(g_bBT_Ringbuf);
                bt_sink_srv_memset(g_bt_sink_srv_am_ring_buffer, 0, sizeof(g_bt_sink_srv_am_ring_buffer));
#else
                free(g_bBT_Ringbuf);
                g_bBT_Ringbuf = NULL;
#endif
            }
            return BT_CODEC_MEDIA_STATUS_OK;
        }
#endif
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}


#ifdef __BT_AWS_SUPPORT__
/**
 * @brief     This function sets the advanced wireless stereo flag.
 * @param[in] aud_id is audio ID.
 * @param[in] flag is used to determine to turn on or turn off AWS mechanism. 1: turn on AWS, 0: turn off AWS.
 * @return    If the operation completed successfully, the return value is #BT_CODEC_MEDIA_STATUS_OK, otherwise the return value is #BT_CODEC_MEDIA_STATUS_ERROR.
 */
static int32_t bt_a2dp_aws_set_flag(bt_sink_srv_am_id_t aud_id, bool flag)
{
    int32_t ret = 0;

    audio_src_srv_report("[sink][AM]aws_set_flag--aud_id: %d, aid: %d, flag: %d", 3,
                         aud_id, g_prCurrent_player->aud_id, flag);

    if (aud_id == g_prCurrent_player->aud_id) {
        ret = bt_codec_a2dp_aws_set_flag(g_prA2dp_sink_handle, flag);
    }

    return ret;
}


/**
 * @brief     This function sets the advanced wireless stereo initial synchronization.
 * @param[in] aud_id is audio ID.
 * @return    If the operation completed successfully, the return value is #BT_CODEC_MEDIA_STATUS_OK, otherwise the return value is #BT_CODEC_MEDIA_STATUS_ERROR.
 */
static int32_t bt_a2dp_aws_set_initial_sync(bt_sink_srv_am_id_t aud_id)
{
    int32_t ret = AM_ERR_FAIL_1ST;

    if (aud_id == g_prCurrent_player->aud_id) {
        //hal_gpio_set_output(HAL_GPIO_12, HAL_GPIO_DATA_HIGH);
        ret = bt_codec_a2dp_aws_set_initial_sync(g_prA2dp_sink_handle);
        //hal_gpio_set_output(HAL_GPIO_12, HAL_GPIO_DATA_LOW);
    }

    return ret;
}


static void bt_a2dp_aws_plh_init(bt_sink_srv_am_id_t aud_id)
{
    if (aud_id == g_prCurrent_player->aud_id) {
        bt_a2dp_plh_init(g_prA2dp_sink_handle);
    }
}


static void bt_a2dp_aws_plh_deinit(void)
{
    bt_a2dp_plh_deinit();
}

void bt_sink_srv_skew_test(const char *value)
{
    int32_t sample_count = 0;

    sample_count = atoi(value);

    LOG_I("[Sink][AM]skew_test--value: %s, count: %d", value, sample_count);

    if (g_prA2dp_sink_handle) {
        bt_codec_a2dp_aws_set_clock_skew_compensation_value(g_prA2dp_sink_handle, sample_count);
    }
}


#endif /* __BT_AWS_SUPPORT__ */

#if defined(__BT_AWS_SUPPORT__) || defined(MTK_AWS_MCE_ENABLE)


/*****************************************************************************
 * FUNCTION
 *  aud_process_aws_a2dp_callback_hdlr
 * DESCRIPTION
 *  This function is used to handle A2DP process callback.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void aud_process_aws_a2dp_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_bt_event_t event_id = amm_ptr->background_info.local_context.aws_format.aws_event;
#ifndef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
#endif
    bt_sink_srv_am_device_set_t dev;
    bt_sink_srv_am_volume_level_out_t vol;
    uint32_t digital_vol = 0x7FFF, analog_vol = 0x0002;
#ifdef __BT_AWS_SUPPORT__
    uint32_t sampling_rate = 44100;
#endif
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (g_prA2dp_sink_handle != NULL) {
        g_prA2dp_sink_handle->process(g_prA2dp_sink_handle, event_id);

        switch (event_id) {
            case BT_CODEC_MEDIA_ERROR: {
#ifdef __BT_SINK_SRV_AM_MED_LIST_SUPPORT__
                //g_prA2dp_sink_handle->stop(g_prA2dp_sink_handle);
                //g_rSink_state = A2DP_SINK_CODEC_STOP;
                dev = g_prCurrent_player->audio_stream_out.audio_device;
                vol = g_prCurrent_player->audio_stream_out.audio_volume;
                hal_audio_set_stream_out_device(dev);
                if (dev & DEVICE_LOUDSPEAKER) {
                    digital_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else if (dev & DEVICE_EARPHONE) {
                    digital_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else {
                    audio_src_srv_report("[sink][AM]aws-d_gain: 0x%x, a_gain: 0x%x", 2, digital_vol, analog_vol);
                }
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
                {
                    bt_sink_srv_audio_setting_vol_info_t vol_info;

                    vol_info.type = VOL_A2DP;
                    vol_info.vol_info.a2dp_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                    vol_info.vol_info.a2dp_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;
                    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                }
#else
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_vol, analog_vol);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_A2DP_PROC_IND, AUD_A2DP_CODEC_RESTART, NULL);
#else
                g_prA2dp_sink_handle->stop(g_prA2dp_sink_handle);
                g_rSink_state = A2DP_SINK_CODEC_STOP;
                g_prA2dp_sink_handle->reset_share_buffer(g_prA2dp_sink_handle);
                dev = g_prCurrent_player->audio_stream_out.audio_device;
                vol = g_prCurrent_player->audio_stream_out.audio_volume;
                hal_audio_set_stream_out_device(dev);
                if (dev & DEVICE_LOUDSPEAKER) {
                    digital_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[LOUDSPEAKER_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else if (dev & DEVICE_EARPHONE) {
                    digital_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][0];
                    analog_vol = g_volume_out_config[EARPHONE_STREAM_OUT][AUD_VOL_AUDIO][vol][1];
                } else {
                    ;
                }
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
                {
                    bt_sink_srv_audio_setting_vol_info_t vol_info;

                    vol_info.type = VOL_A2DP;
                    vol_info.vol_info.a2dp_vol_info.dev = g_prCurrent_player->audio_stream_out.audio_device;
                    vol_info.vol_info.a2dp_vol_info.lev = g_prCurrent_player->audio_stream_out.audio_volume;
                    bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                }
#else
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_vol, analog_vol);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */

                //eResult = g_prA2dp_sink_handle->play(g_prA2dp_sink_handle);
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_A2DP_PROC_IND, AUD_A2DP_CODEC_RESTART, NULL);
                if (eResult == BT_CODEC_MEDIA_STATUS_ERROR) {
                    audio_src_srv_report("[sink][AM] Error", 0);
                }
                audio_src_srv_report("[sink][AM]-restart play", 0);
#endif /* __BT_SINK_SRV_AM_MED_LIST_SUPPORT__ */
                break;
            }
#ifdef __BT_AWS_SUPPORT__
            case BT_CODEC_MEDIA_AWS_CHECK_UNDERFLOW: {
#ifndef MTK_AVM_DIRECT
                bt_sink_srv_fetch_bt_offset();
#endif
                break;
            }

            case BT_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW: {
                if (BT_AWS_CODEC_TYPE_SBC == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 8:
                            sampling_rate = 16000;
                            break;
                        case 4:
                            sampling_rate = 32000;
                            break;
                        case 2:
                            sampling_rate = 44100;
                            break;
                        case 1:
                            sampling_rate = 48000;
                            break;

                        default:
                            sampling_rate = 44100;
                            break;
                    }
                } else if (BT_AWS_CODEC_TYPE_AAC == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 0x800:
                            sampling_rate = 8000;
                            break;
                        case 0x400:
                            sampling_rate = 11025;
                            break;
                        case 0x200:
                            sampling_rate = 12000;
                            break;
                        case 0x100:
                            sampling_rate = 16000;
                            break;
                        case 0x80:
                            sampling_rate = 22050;
                            break;
                        case 0x40:
                            sampling_rate = 24000;
                            break;
                        case 0x20:
                            sampling_rate = 32000;
                            break;
                        case 0x10:
                            sampling_rate = 44100;
                            break;
                        case 0x8:
                            sampling_rate = 48000;
                            break;
                        default:
                            sampling_rate = 44100;
                            break;
                    }
                } else if (BT_AWS_CODEC_TYPE_VENDOR == g_aws_codec_type) {
                    switch (g_aws_sample_rate) {
                        case 0x20:
                            sampling_rate = 44100;
                            break;
                        case 0x10:
                            sampling_rate = 48000;
                            break;
                        case 0x08:
                            sampling_rate = 88200;
                            break;
                        case 0x04:
                            sampling_rate = 96000;
                            break;
                        case 0x02:
                            sampling_rate = 176400;
                            break;
                        case 0x01:
                            sampling_rate = 192000;
                            break;
                        default:
                            sampling_rate = 44100;
                            break;
                    }
                }
                /* myler */
#ifdef __BT_AWS_SUPPORT__
                bt_sink_srv_audio_sync_calc_t audio_calc;
                audio_calc.media_handle = g_prA2dp_sink_handle;
                audio_calc.sampling_rate = sampling_rate;
                audio_calc.type = g_aws_codec_type;
                bt_sink_srv_audio_clock_calibrate(&audio_calc);
#endif
                break;
            }

            case BT_CODEC_MEDIA_UNDERFLOW: {
#ifdef __BT_AWS_SUPPORT__
                g_aws_underflow_loop_count++;
                if (g_aws_underflow_loop_count >= BT_SINK_SRV_AM_MAX_UNDERFLOW_COUNT) {
                    g_aws_underflow_loop_count = 0;
                    g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_A2DP_PROC_IND, AUD_A2DP_AWS_UNDERFLOW, NULL);
                }
#endif
                break;
            }
#endif
#if defined(MTK_AVM_DIRECT)
            case BT_CODEC_MEDIA_LTCS_DATA_REPORT: {
                audio_dsp_a2dp_ltcs_report_param_t param;
                param.p_ltcs_asi_buf = hal_audio_query_ltcs_asi_buf();
                param.p_ltcs_min_gap_buf = hal_audio_query_ltcs_min_gap_buf();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_LTCS_REPORT, &param);
                break;
            }

            case BT_CODEC_MEDIA_LTCS_DATA_TIMEOUT: {
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_RECONNECT_REQUEST, NULL);
                return;
            }

            case BT_CODEC_MEDIA_AUDIO_DL_REINIT_REQUEST: {
                uint32_t param_reinit = hal_audio_dsp2mcu_data_get();
                g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_A2DP_PROC_IND, AUD_A2DP_DL_REINIT_REQUEST, &param_reinit);
                break;
            }
#endif
            default:
                break;
        }
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_AWS_A2DP_PROC_IND, (bt_sink_srv_am_cb_sub_msg_t)event_id, NULL);
        _UNUSED(digital_vol);
        _UNUSED(analog_vol);
    }
}
#endif /* defined(__BT_AWS_SUPPORT__) || defined(MTK_AWS_MCE_ENABLE) */


#ifdef __BT_SINK_AUDIO_TUNING__
#define BT_SINK_AM_SIDE_TONE_DB_MAX 31
void bt_codec_am_speech_nvdm_changed_callback(void *data)
{
    audio_eaps_t *am_speech_eaps;

#if PRODUCT_VERSION == 2533
    aud_process_dsp_nvdm_setting();

    am_speech_eaps = (audio_eaps_t *)pvPortMalloc(sizeof(audio_eaps_t));

    audio_src_srv_report("[Sink][AM] nvdm changed, g_prCurrent_player:0x%x", 1, g_prCurrent_player);

    if (am_speech_eaps != NULL) {
        audio_nvdm_get_eaps_data_by_memcpy(am_speech_eaps);

#ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__

        speech_update_common(am_speech_eaps->speech_common_parameter.speech_common_parameter);
        speech_update_nb_fir(NULL,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_nb_band.voice_nb_parameter_with_external_dsp[0].voice_nb_enhancement_parameter.nb_stream_out_fir_coefficient);
        speech_update_nb_param(am_speech_eaps->voice_parameter.voice_nb_band.voice_nb_parameter_with_external_dsp[0].voice_nb_enhancement_parameter.nb_mode_parameter);
        speech_update_wb_fir(NULL,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_wb_band.voice_wb_parameter_with_external_dsp[0].voice_wb_enhancement_parameter.wb_stream_out_fir_coefficient);
        speech_update_wb_param(am_speech_eaps->voice_parameter.voice_wb_band.voice_wb_parameter_with_external_dsp[0].voice_wb_enhancement_parameter.wb_mode_parameter);
#endif
        if (NULL != g_prCurrent_player && HFP == g_prCurrent_player->type) {
            int32_t stream_out_db = 0, stream_in_db = 0;
            int32_t analog_gain_in_db = 0, digital_gain_in_db = 0;
            int16_t temp_int16 = 0;
            uint32_t side_tone_db = 0;
            bool result = false;
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            {
                bt_sink_srv_audio_setting_vol_info_t vol_info;

                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = g_prCurrent_player->local_context.hfp_format.hfp_codec.type;
                vol_info.vol_info.hfp_vol_info.dev_in = g_prCurrent_player->audio_stream_in.audio_device;
                vol_info.vol_info.hfp_vol_info.dev_out = g_prCurrent_player->audio_stream_out.audio_device;
                vol_info.vol_info.hfp_vol_info.lev_in = g_prCurrent_player->audio_stream_in.audio_volume;
                vol_info.vol_info.hfp_vol_info.lev_out = g_prCurrent_player->audio_stream_out.audio_volume;
                speech_set_enhancement(false);
                bt_sink_srv_audio_setting_update_voice_fillter_setting(&vol_info, am_speech_eaps);
                speech_set_enhancement(true);
            }
#endif

            audio_src_srv_report("[Sink][AM] type:0x%x, out:%d, in:%d", 3,
                                 g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                                 g_prCurrent_player->audio_stream_out.audio_volume,
                                 g_prCurrent_player->audio_stream_in.audio_volume);

            if (BT_HFP_CODEC_TYPE_CVSD == g_prCurrent_player->local_context.hfp_format.hfp_codec.type) {
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_nb_band.voice_nb_parameter_with_external_dsp[0].voice_nb_gain.stream_out_gain[g_prCurrent_player->audio_stream_out.audio_volume];
                stream_out_db = (int32_t)temp_int16;
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_nb_band.voice_nb_parameter_with_external_dsp[0].voice_nb_gain.stream_in_gain[g_prCurrent_player->audio_stream_in.audio_volume];
                stream_in_db = (int32_t)temp_int16;
                side_tone_db = (uint32_t)am_speech_eaps->voice_parameter.voice_nb_band.voice_nb_parameter_with_external_dsp[0].voice_nb_gain.sidetone_gain;
                result = true;
            } else if (BT_HFP_CODEC_TYPE_MSBC == g_prCurrent_player->local_context.hfp_format.hfp_codec.type) {
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_wb_band.voice_wb_parameter_with_external_dsp[0].voice_wb_gain.stream_out_gain[g_prCurrent_player->audio_stream_out.audio_volume];
                stream_out_db = (int32_t)temp_int16;
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_wb_band.voice_wb_parameter_with_external_dsp[0].voice_wb_gain.stream_in_gain[g_prCurrent_player->audio_stream_in.audio_volume];
                stream_in_db = (int32_t)temp_int16;
                side_tone_db = (uint32_t)am_speech_eaps->voice_parameter.voice_wb_band.voice_wb_parameter_with_external_dsp[0].voice_wb_gain.sidetone_gain;
                result = true;
            }

            audio_src_srv_report("[Sink][AM] out_db:%d, in_db:%d, side_tone:%d", 3, stream_out_db, stream_in_db, side_tone_db);

#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            {
                bt_sink_srv_audio_setting_vol_info_t vol_info;

                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = g_prCurrent_player->local_context.hfp_format.hfp_codec.type;
                vol_info.vol_info.hfp_vol_info.dev_in = g_prCurrent_player->audio_stream_in.audio_device;
                vol_info.vol_info.hfp_vol_info.dev_out = g_prCurrent_player->audio_stream_out.audio_device;
                vol_info.vol_info.hfp_vol_info.lev_in = g_prCurrent_player->audio_stream_in.audio_volume;
                vol_info.vol_info.hfp_vol_info.lev_out = g_prCurrent_player->audio_stream_out.audio_volume;
                audio_src_srv_report("[Sink][AM]2533 update gain", 0);
                bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
            }
#endif

            if (result) {
                audio_downlink_amp_db_transfer_to_analog_gain_and_digital_gain(stream_out_db, &analog_gain_in_db, &digital_gain_in_db);
                digital_gain_in_db *= 100;
                analog_gain_in_db *= 100;
                audio_src_srv_report("[Sink][AM] stream out, digital:0x%x, analog:0x%x", 2, digital_gain_in_db, analog_gain_in_db);
#ifndef __BT_SINK_SRV_AUDIO_TUNING__
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, (uint32_t)digital_gain_in_db, (uint32_t)analog_gain_in_db);
#endif
                voice_uplink_dmic_db_transfer_to_analog_gain_and_digital_gain(stream_in_db, &analog_gain_in_db, &digital_gain_in_db);
                digital_gain_in_db *= 100;
                analog_gain_in_db *= 100;
                audio_src_srv_report("[Sink][AM] stream in, digital:0x%x, analog:0x%x", 2, digital_gain_in_db, analog_gain_in_db);
#ifndef __BT_SINK_SRV_AUDIO_TUNING__
                hal_audio_set_stream_in_volume((uint32_t)digital_gain_in_db, (uint32_t)analog_gain_in_db);
#endif

                if (BT_SINK_AM_SIDE_TONE_DB_MAX >= side_tone_db) {
                    audio_src_srv_report("[Sink][AM] side_tone_db:0x%x", 1, side_tone_db);
                    speech_set_sidetone_volume(side_tone_db);
                }
            }
        }

        vPortFree(am_speech_eaps);
    }

#else

    am_speech_eaps = (audio_eaps_t *)pvPortMalloc(sizeof(audio_eaps_t));

    audio_src_srv_report("[Sink][AM] nvdm changed, g_prCurrent_player:0x%x", 1, g_prCurrent_player);

    if (am_speech_eaps != NULL) {
        audio_nvdm_get_eaps_data_by_memcpy(am_speech_eaps);

#ifndef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
        speech_set_enhancement(false);
        speech_update_common(am_speech_eaps->speech_common_parameter.speech_common_parameter);
        speech_update_nb_fir((int16_t *)am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_enhancement_parameter.nb_stream_in_fir_coefficient,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_enhancement_parameter.nb_stream_out_fir_coefficient);
        speech_update_nb_param(am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_enhancement_parameter.nb_mode_parameter);
        speech_update_wb_fir((int16_t *)am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_enhancement_parameter.wb_stream_in_fir_coefficient,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_enhancement_parameter.wb_stream_out_fir_coefficient);
        speech_update_wb_param(am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_enhancement_parameter.wb_mode_parameter);
        speech_set_enhancement(true);
#endif

        if (NULL != g_prCurrent_player && HFP == g_prCurrent_player->type) {
            int32_t stream_out_db = 0, stream_in_db = 0;
            int32_t analog_gain_in_db = 0, digital_gain_in_db = 0;
            int16_t temp_int16 = 0;
            uint32_t side_tone_db = 0;
            bool result = false;

#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            {
                bt_sink_srv_audio_setting_vol_info_t vol_info;

                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = g_prCurrent_player->local_context.hfp_format.hfp_codec.type;
                vol_info.vol_info.hfp_vol_info.dev_in = g_prCurrent_player->audio_stream_in.audio_device;
                vol_info.vol_info.hfp_vol_info.dev_out = g_prCurrent_player->audio_stream_out.audio_device;
                vol_info.vol_info.hfp_vol_info.lev_in = g_prCurrent_player->audio_stream_in.audio_volume;
                vol_info.vol_info.hfp_vol_info.lev_out = g_prCurrent_player->audio_stream_out.audio_volume;
                speech_set_enhancement(false);
                bt_sink_srv_audio_setting_update_voice_fillter_setting(&vol_info, am_speech_eaps);
                speech_set_enhancement(true);
            }
#endif

            audio_src_srv_report("[Sink][AM] type:0x%x, out:%d, in:%d", 3,
                                 g_prCurrent_player->local_context.hfp_format.hfp_codec.type,
                                 g_prCurrent_player->audio_stream_out.audio_volume,
                                 g_prCurrent_player->audio_stream_in.audio_volume);

            if (BT_HFP_CODEC_TYPE_CVSD == g_prCurrent_player->local_context.hfp_format.hfp_codec.type) {
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_gain.stream_out_gain[g_prCurrent_player->audio_stream_out.audio_volume];
                stream_out_db = (int32_t)temp_int16;
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_gain.stream_in_gain[g_prCurrent_player->audio_stream_in.audio_volume];
                stream_in_db = (int32_t)temp_int16;
                side_tone_db = (uint32_t)am_speech_eaps->voice_parameter.voice_nb_parameter[SPEECH_MODE_HEADSET].voice_nb_gain.sidetone_gain;
                result = true;
            } else if (BT_HFP_CODEC_TYPE_MSBC == g_prCurrent_player->local_context.hfp_format.hfp_codec.type) {
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_gain.stream_out_gain[g_prCurrent_player->audio_stream_out.audio_volume];
                stream_out_db = (int32_t)temp_int16;
                temp_int16 = (int16_t)am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_gain.stream_in_gain[g_prCurrent_player->audio_stream_in.audio_volume];
                stream_in_db = (int32_t)temp_int16;
                side_tone_db = (uint32_t)am_speech_eaps->voice_parameter.voice_wb_parameter[SPEECH_MODE_HEADSET].voice_wb_gain.sidetone_gain;
                result = true;
            }

            audio_src_srv_report("[Sink][AM] out_db:%d, in_db:%d, side_tone:%d", 3, stream_out_db, stream_in_db, side_tone_db);

#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            {
                bt_sink_srv_audio_setting_vol_info_t vol_info;

                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = g_prCurrent_player->local_context.hfp_format.hfp_codec.type;
                vol_info.vol_info.hfp_vol_info.dev_in = g_prCurrent_player->audio_stream_in.audio_device;
                vol_info.vol_info.hfp_vol_info.dev_out = g_prCurrent_player->audio_stream_out.audio_device;
                vol_info.vol_info.hfp_vol_info.lev_in = g_prCurrent_player->audio_stream_in.audio_volume;
                vol_info.vol_info.hfp_vol_info.lev_out = g_prCurrent_player->audio_stream_out.audio_volume;
                audio_src_srv_report("[Sink][AM]2523 update gain", 0);
                bt_sink_srv_am_set_volume(STREAM_OUT, &vol_info);
                bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
            }
#endif

            if (result) {
                audio_downlink_amp_db_transfer_to_analog_gain_and_digital_gain(stream_out_db, &analog_gain_in_db, &digital_gain_in_db);
                digital_gain_in_db *= 100;
                analog_gain_in_db *= 100;
                audio_src_srv_report("[Sink][AM] stream out, digital:0x%x, analog:0x%x", 2, digital_gain_in_db, analog_gain_in_db);
#ifndef __BT_SINK_SRV_AUDIO_TUNING__
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, (uint32_t)digital_gain_in_db, (uint32_t)analog_gain_in_db);
#endif
                voice_uplink_amp_db_transfer_to_analog_gain_and_digital_gain(stream_in_db, &analog_gain_in_db, &digital_gain_in_db);
                digital_gain_in_db *= 100;
                analog_gain_in_db *= 100;
                audio_src_srv_report("[Sink][AM] stream in, digital:0x%x, analog:0x%x", 2, digital_gain_in_db, analog_gain_in_db);
#ifndef __BT_SINK_SRV_AUDIO_TUNING__
                hal_audio_set_stream_in_volume((uint32_t)digital_gain_in_db, (uint32_t)analog_gain_in_db);
#endif

                if (BT_SINK_AM_SIDE_TONE_DB_MAX >= side_tone_db) {
                    audio_src_srv_report("[Sink][AM] side_tone_db:0x%x", 1, side_tone_db);
                    speech_set_sidetone_volume(side_tone_db);
                }
            }
        }

        vPortFree(am_speech_eaps);
    }
#endif
}
#endif /* __BT_SINK_AUDIO_TUNING__ */

#ifdef AIR_BT_HFP_ENABLE
/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_hfp_open
 * DESCRIPTION
 *  This function is used to open HFP codec.
 * PARAMETERS
 *  hfp_codec        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void bt_codec_am_hfp_open(bt_sink_srv_am_hfp_codec_t *hfp_codec_t)
{
    bt_status_t eResult;
    AUD_LOG_W("[AM]Open HFP codec = %d.", 1, hfp_codec_t->type);
    if ((hfp_codec_t->type != BT_HFP_CODEC_TYPE_CVSD) && (hfp_codec_t->type != BT_HFP_CODEC_TYPE_MSBC)) {
        AUDIO_ASSERT(0 && "[AM][error]Open HFP codec error(not support).");
    }
    g_prHfp_media_handle = bt_codec_hfp_open(aud_bt_codec_hfp_callback, hfp_codec_t);

    // For BT to analyze duration
    g_prHfp_sink_event_handle.med_hd = g_prHfp_media_handle;
    g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_HFP_OPEN_CODEC_DONE, &g_prHfp_sink_event_handle);

    eResult = g_prHfp_media_handle->play(g_prHfp_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
        g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SELF_CMD_REQ, AUD_HFP_PLAY_OK, NULL);
    } else {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SELF_CMD_REQ, AUD_CMD_FAILURE, NULL);
    }

    aud_dl_control(true);
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
    if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
        ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
        aud_ul_control(true);
    }
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_hfp_stop
 * DESCRIPTION
 *  This function is used to stop HFP codec.
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_hfp_stop(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    eResult = g_prHfp_media_handle->stop(g_prHfp_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
#if defined(MTK_AVM_DIRECT)
        aud_dl_control(false);
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
        if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
            ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
            aud_ul_control(false);
        }
#endif
        if (bt_codec_hfp_close(g_prHfp_media_handle) == BT_CODEC_MEDIA_STATUS_OK) {
            g_prHfp_media_handle = NULL;
            /*Close sidetone if there is sidetone exit.*/
            if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) {
                audio_side_tone_disable_hdlr(NULL);
            }
            ami_set_audio_mask(AM_TASK_MASK_UL1_HAPPENING, false);
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_HFP_UL, NULL, false);
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_HFP_DL, NULL, false);
            return BT_CODEC_MEDIA_STATUS_OK;
        }
    }
    ami_set_audio_mask(AM_TASK_MASK_UL1_HAPPENING, false);

    return BT_CODEC_MEDIA_STATUS_ERROR;
}
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
static uint32_t lc3_sampling_rate_enum_to_value(uint8_t sampling_rate_enum)
{
    switch (sampling_rate_enum) {
        case SAMPLING_RATE_16K:
            return 16000;
        case SAMPLING_RATE_24K:
            return 24000;
        case SAMPLING_RATE_32K:
            return 32000;
        case SAMPLING_RATE_44_1K:
            return 44100;
        case SAMPLING_RATE_48K:
            return 48000;
        case SAMPLING_RATE_96K:
            return 96000;
        default:
            audio_src_srv_report("[sink][am][error]CONVERT_LC3 sampling_frequency not support(%d)", 1, sampling_rate_enum);
            return 0;
    }
}
void lc3_set_channel_num(uint8_t *src_channel_num, uint8_t *des_channel_num)
{
    switch (*src_channel_num) {
        case CHANNEL_MONO:
        case CHANNEL_STEREO:
        case CHANNEL_DUAL_MONO:
            *des_channel_num = *src_channel_num;
            break;
        default:
            audio_src_srv_report("[sink][am][error]CONVERT_LC3 channel_num not support(%d)", 1, *src_channel_num);
            break;
    }
}
void BT_BLE_CONVERT_LC3_CODEC(bt_codec_le_audio_param_t *dst_codec,
                              bt_sink_srv_am_ble_codec_t *src_codec)
{
    dst_codec->dl_param.codec = BT_CODEC_TYPE_LE_AUDIO_LC3;
    dst_codec->ul_param.codec = BT_CODEC_TYPE_LE_AUDIO_LC3;
    switch (src_codec->channel_mode) {
        case CHANNEL_MODE_DL_ONLY:
        case CHANNEL_MODE_DL_UL_BOTH:
        case CHANNEL_MODE_UL_ONLY:
            dst_codec->channel_mode = src_codec->channel_mode;
            audio_src_srv_report("[sink][am][med]CONVERT_LC3 channel_mode(%d)", 1, src_codec->channel_mode);
    }

    dst_codec->dl_param.sample_rate = lc3_sampling_rate_enum_to_value(src_codec->sample_rate);
    dst_codec->ul_param.sample_rate = lc3_sampling_rate_enum_to_value(src_codec->ul_sample_rate);
    lc3_set_channel_num(&(src_codec->channel_num),&(dst_codec->dl_param.channel_num));
    lc3_set_channel_num(&(src_codec->ul_channel_num),&(dst_codec->ul_param.channel_num));

    dst_codec->dl_param.frame_duration = (src_codec->frame_duration == FRAME_MS_10) ? LE_AUDIO_interval_10 : LE_AUDIO_interval_7_5;
    dst_codec->dl_param.context_type = src_codec->context_type;
    dst_codec->ul_param.context_type = src_codec->ul_context_type;
    dst_codec->dl_param.maxframe_payload_length = src_codec->frame_payload_length;
    dst_codec->ul_param.frame_payload_length = src_codec->ul_frame_payload_length;
    dst_codec->ul_param.frame_duration = dst_codec->dl_param.frame_duration;
    audio_src_srv_report("[sink][am][med]CONVERT_LC3, frame_ms: %d,[DL] context_type: %d, channel_num: %d, sampling_frequency: %d, frame_payload_length: %d", 5,
                         dst_codec->dl_param.frame_duration, dst_codec->dl_param.context_type,
                         dst_codec->dl_param.channel_num, dst_codec->dl_param.sample_rate, dst_codec->dl_param.maxframe_payload_length);
    audio_src_srv_report("[sink][am][med]CONVERT_LC3--[UL] context_type: %d, channel_num: %d, sampling_frequency: %d, frame_payload_length: %d", 4,
                         dst_codec->ul_param.context_type, dst_codec->ul_param.channel_num,
                         dst_codec->ul_param.sample_rate, dst_codec->ul_param.frame_payload_length);

}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_ble_open
 * DESCRIPTION
 *  This function is used to open BLE codec.
 * PARAMETERS
 *  hfp_codec        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void bt_codec_am_ble_open(bt_sink_srv_am_ble_codec_t *ble_codec_t)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_codec_le_audio_param_t ble_codec = {0};
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
    //const audio_eaps_t *am_speech_eaps = (const audio_eaps_t *) audio_nvdm_get_global_eaps_address();

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (ble_codec_t->codec == CODEC_LC3) {
        BT_BLE_CONVERT_LC3_CODEC(&ble_codec, ble_codec_t);
    }
    #ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    else if ((ble_codec_t->codec == CODEC_LC3PLUS_CBR)||(ble_codec_t->codec == CODEC_LC3PLUS_VBR)){
        BT_BLE_CONVERT_LC3_CODEC(&ble_codec, ble_codec_t);
        ble_codec.dl_param.codec = BT_CODEC_TYPE_LE_AUDIO_LC3PULS;
        ble_codec.ul_param.codec = BT_CODEC_TYPE_LE_AUDIO_LC3PULS;
    }
    #endif
    else{
        audio_src_srv_report("[sink][AM]ble open codec fail--not support codec: %d", 1, ble_codec_t->codec);
    }

    g_prBle_media_handle = bt_codec_le_audio_open(aud_bt_codec_ble_callback,&ble_codec);

#if defined(MTK_AVM_DIRECT)
    // For BT to analyze duration
    aud_dl_control(true);
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
    if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
        ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
        aud_ul_control(true);
    }
    g_prBle_sink_event_handle.med_hd = g_prBle_media_handle;
#endif
    eResult = g_prBle_media_handle->play(g_prBle_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
        g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_HFP_OPEN_CODEC_DONE, &g_prBle_sink_event_handle);
    } else {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_FAILURE, &g_prBle_sink_event_handle);
    }
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_ble_stop
 * DESCRIPTION
 *  This function is used to stop BLE codec.
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_ble_stop(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    eResult = g_prBle_media_handle->stop(g_prBle_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
#if defined(MTK_AVM_DIRECT)
        aud_dl_control(false);
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
        if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
            ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
            aud_ul_control(false);
        }
#endif
        if (bt_codec_le_audio_close(g_prBle_media_handle,0) == BT_CODEC_MEDIA_STATUS_OK) {
            g_prBle_media_handle = NULL;
            /*Close sidetone if there is sidetone exit.*/
#if 0
            if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) {
                audio_side_tone_disable_hdlr(NULL);
            }
#endif
            return BT_CODEC_MEDIA_STATUS_OK;
        }
    }
    return BT_CODEC_MEDIA_STATUS_ERROR;
}

#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
void bt_codec_am_ull_ble_ul_open(bt_codec_le_audio_ul_t ul_param,bt_sink_srv_am_background_t *background_ptr)
{
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE

    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_codec_le_audio_param_t ble_codec = {0};
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
    //const audio_eaps_t *am_speech_eaps = (const audio_eaps_t *) audio_nvdm_get_global_eaps_address();

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    audio_src_srv_report("[sink][AM]ull ble ul open codec : %d", 1, ul_param.codec);
    ble_codec.channel_mode = CHANNEL_MODE_UL_ONLY;
    ble_codec.ul_param = ul_param;
    g_prUllBleUl_media_handle = bt_codec_le_audio_open(aud_bt_codec_ble_callback,&ble_codec);

    aud_ul_control(true);
#if defined(MTK_AVM_DIRECT)
    // For BT to analyze duration
    g_prUllBleUl_sink_event_handle.med_hd = g_prUllBleUl_media_handle;
#endif
    eResult = g_prUllBleUl_media_handle->play(g_prUllBleUl_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
        g_rAm_aud_id[background_ptr->aud_id].use = ID_PLAY_STATE;
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_HFP_OPEN_CODEC_DONE, &g_prUllBleUl_sink_event_handle);
    } else {
        background_ptr->notify_cb(background_ptr->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_FAILURE, &g_prUllBleUl_sink_event_handle);
    }
#endif
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_ble_stop
 * DESCRIPTION
 *  This function is used to stop BLE codec.
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_ull_ble_ul_stop(void)
{
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    eResult = g_prUllBleUl_media_handle->stop(g_prUllBleUl_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
        if (bt_codec_le_audio_close(g_prUllBleUl_media_handle,2) == BT_CODEC_MEDIA_STATUS_OK) {
            g_prUllBleUl_media_handle = NULL;
            aud_ul_control(false);
            /*Close sidetone if there is sidetone exit.*/
            return BT_CODEC_MEDIA_STATUS_OK;
        }
    }
#endif
    return BT_CODEC_MEDIA_STATUS_ERROR;
}
void bt_codec_am_ull_ble_dl_open(bt_codec_le_audio_dl_t dl_param)
{
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_codec_le_audio_param_t ble_codec = {0};
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;
    //const audio_eaps_t *am_speech_eaps = (const audio_eaps_t *) audio_nvdm_get_global_eaps_address();

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    audio_src_srv_report("[sink][AM]ull ble dl open codec : %d", 1, dl_param.codec);
    ble_codec.channel_mode = CHANNEL_MODE_DL_ONLY;
    ble_codec.dl_param = dl_param;
    ble_codec.dl_param.context_type = AUDIO_CONTENT_TYPE_ULL_BLE;
    if (dl_param.codec == BT_CODEC_TYPE_LE_AUDIO_ULD) {
        ble_codec.dl_param.frame_duration = LE_AUDIO_interval_2;
    } else {
        /* LC3PLUS/ VENDOR Codec */
        ble_codec.dl_param.frame_duration = LE_AUDIO_interval_5;
    }

    /* remap channel mode para from bt_ull_le_am_dl_play_ind() */
    switch(ble_codec.dl_param.channel_num)
    {
        case BT_ULL_LE_CHANNEL_MODE_MONO:
            ble_codec.dl_param.channel_num = CHANNEL_MONO;
        break;
        //case BT_ULL_LE_CHANNEL_MODE_STEREO:
        //    ble_codec.dl_param.channel_num = CHANNEL_STEREO;
        //break;
        /* remap stereo to dual mono for headset */
        case BT_ULL_LE_CHANNEL_MODE_STEREO:
        case BT_ULL_LE_CHANNEL_MODE_DUAL_MONO:
            ble_codec.dl_param.channel_num = CHANNEL_DUAL_MONO;
        break;
        default:
            audio_src_srv_report("[sink][AM]undefined channel_num : %d", 1, ble_codec.dl_param.channel_num);
            AUDIO_ASSERT(0);
        break;
    }

    g_prUllBleDl_media_handle = bt_codec_le_audio_open(aud_bt_codec_ble_callback,&ble_codec);

    aud_dl_control(true);
#if defined(MTK_AVM_DIRECT)
    // For BT to analyze duration
    g_prUllBleDl_sink_event_handle.med_hd = g_prUllBleDl_media_handle;
#endif
    eResult = g_prUllBleDl_media_handle->play(g_prUllBleDl_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
        g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_HFP_OPEN_CODEC_DONE, &g_prUllBleDl_sink_event_handle);
    } else {
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id, AUD_SINK_OPEN_CODEC, AUD_CMD_FAILURE, &g_prUllBleDl_sink_event_handle);
    }
#endif
}

/*****************************************************************************
 * FUNCTION
 *  bt_codec_am_ble_stop
 * DESCRIPTION
 *  This function is used to stop BLE codec.
 * PARAMETERS
 *  void
 * RETURNS
 *  bt_status_t
 *****************************************************************************/
bt_status_t bt_codec_am_ull_ble_dl_stop(void)
{
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_status_t eResult = BT_CODEC_MEDIA_STATUS_ERROR;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    eResult = g_prUllBleDl_media_handle->stop(g_prUllBleDl_media_handle);
    if (eResult == BT_CODEC_MEDIA_STATUS_OK) {
#if defined(MTK_AVM_DIRECT)
        aud_dl_control(false);
        //aud_ul_control(false);
#endif
        if (bt_codec_le_audio_close(g_prUllBleDl_media_handle,3) == BT_CODEC_MEDIA_STATUS_OK) {
            g_prUllBleDl_media_handle = NULL;
            /*Close sidetone if there is sidetone exit.*/
            return BT_CODEC_MEDIA_STATUS_OK;
        }
    }
#endif
    return BT_CODEC_MEDIA_STATUS_ERROR;
}


#endif



#ifdef RTOS_TIMER
/*****************************************************************************
 * FUNCTION
 *  aud_timer_callback
 * DESCRIPTION
 *  This callback function is used to notify A.M. when the timer expires on timer service task.
 * PARAMETERS
 *  pxTimer          [OUT]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_timer_callback(TimerHandle_t pxTimer)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    uint32_t lTimer_ID;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
#ifdef __AM_DEBUG_INFO__
    //audio_src_srv_report("[Sink][AM] Timer tick", 0);
#endif
    AUDIO_ASSERT(pxTimer);
    lTimer_ID = (uint32_t)pvTimerGetTimerID(pxTimer);
    if (lTimer_ID == AM_TIMER_ID) {
        g_lExpire_count++;
        if (g_lExpire_count == AM_EXPIRE_TIMER_MAX) {
            xTimerStop(pxTimer, 0);
            g_lExpire_count = 0;
            bt_sink_srv_ami_send_amm(MOD_AM, MOD_TMR, AUD_TIMER_IND,
                                     MSG_ID_TIMER_OUT_CALL_EXT_REQ,
                                     g_prCurrent_resumer,
                                     FALSE, NULL);
        }
    }
}
#endif

/*****************************************************************************
 * FUNCTION
 *  aud_timer_out_callback_hdlr
 * DESCRIPTION
 *  This function is used to handle timer callback on A.M. task.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void aud_timer_out_callback_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    g_rAm_aud_id[g_prCurrent_resumer->aud_id].use = ID_IDLE_STATE;
    g_prCurrent_resumer->notify_cb(g_prCurrent_resumer->aud_id, AUD_RESUME_IND, AUD_RESUME_IDLE_STATE, NULL);
    g_prCurrent_resumer = NULL;
    am_audio_search_suspended();
}

#if defined(MTK_AVM_DIRECT)
/*****************************************************************************
 * FUNCTION
 *  audio_event_task_notification
 * DESCRIPTION
 *  HAL audio use the function to notify AM task.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void audio_event_task_notification(void)
{
    bt_sink_srv_ami_send_amm(MOD_AM, MOD_L1SP, AUD_HAL_EVENT_IND,
                             MSG_ID_HAL_EVENT_EXT_REQ,
                             g_prCurrent_player,
                             TRUE, ptr_isr_callback_amm);
}


/*****************************************************************************
 * FUNCTION
 *  audio_task_delay_ms_func
 * DESCRIPTION
 *  Provide task delay to HAL.
 *  HAL need to wait for CCNI or DSP ack.
 *  Task delay is better than GPT delay.
 * PARAMETERS
 *  ms_duration      [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void audio_task_delay_ms_func(uint32_t ms_duration)
{
    vTaskDelay(ms_duration / portTICK_RATE_MS);
}

#endif

#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
void audio_set_vendor_se_hdlr(bt_sink_srv_am_amm_struct *amm_pt)
{
    ami_execute_vendor_se(EVENT_SET_VENDOREFFECT);
    return;
}
#endif

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
static void audio_set_audio_transmitter_config(bt_sink_srv_am_amm_struct *amm_pt)
{
    bt_sink_srv_am_background_t *background_ptr = &(amm_pt->background_info);
    if (background_ptr->type == AUDIO_TRANSMITTER) {
        uint16_t scenario_and_id = ((background_ptr->local_context.audio_transmitter_format.scenario_type) << 8) + background_ptr->local_context.audio_transmitter_format.scenario_sub_id;
        audio_src_srv_report("[Sink][AM][transmitter]playback_set_runtime_config, scenario_and_id = %x,", 1, scenario_and_id);
        audio_transmitter_playback_set_runtime_config(scenario_and_id, (background_ptr->local_context.audio_transmitter_format.scenario_runtime_config_type), &(background_ptr->local_context.audio_transmitter_format.scenario_runtime_config));
        background_ptr->notify_cb(background_ptr->aud_id, AUD_HAL_EVENT_IND, AUD_CMD_COMPLETE, &scenario_and_id);
    }
}
#endif

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#ifdef AIR_WIRED_AUDIO_ENABLE
extern void line_out_stream_out_afe_config(audio_transmitter_config_t *config, uint8_t scenario_sub_id, mcu2dsp_open_param_t *open_param);
#endif
extern uint16_t g_sidetone_user;
static void audio_set_voice_mic_type_hdlr(bt_sink_srv_am_amm_struct *amm_pt)
{
    bt_sink_srv_am_background_t *background_ptr = &(amm_pt->background_info);
    voice_mic_type_t voice_mic_type = background_ptr->local_context.audio_detachable_mic_format.voice_mic_type;
    uint32_t mic_used_flag;
    mcu2dsp_open_param_t *open_param;
    open_param = (mcu2dsp_open_param_p)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    memset(open_param, 0, sizeof(mcu2dsp_open_param_t));


    //audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr\n", 0);

    mic_used_flag = 0;

    background_ptr->local_feature.feature_param.sidetone_param.scenario = g_sidetone_user;
    #ifdef AIR_DCHS_MODE_ENABLE
    dchs_uart_detachable_mic_sync(voice_mic_type, SYNC_START);
    #endif
    //Restart sidetone if sidetone is enabled
    if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
        if ((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)
            && (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_WAITING_STOP))) {
            audio_side_tone_disable_hdlr(amm_pt);
            audio_side_tone_enable_hdlr(amm_pt);
        }
    } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {

    }

    //AEC_NR start/stop
    if (voice_mic_type == VOICE_MIC_TYPE_FIXED) {
        //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_EN, 0, 1, false);
    } else {
        //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_EN, 0, 0, false);
    }

    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &(open_param->stream_in_param));
    void *p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_BT_VOICE_DL);

    //Check MIC user with transmitter
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_internal.h"

    _UNUSED(g_audio_transmitter_control);

    uint16_t scenario_and_id;

    audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
        {AUDIO_TRANSMITTER_GAMING_MODE, AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET},
#ifdef AIR_WIRED_AUDIO_ENABLE
        //{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT},
        {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
        {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER},
#endif
    };

    if (true == audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list, sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t))) {
        for (uint32_t i = 0; i < sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t); i++) {
            if (true == audio_transmitter_get_is_running_by_scenario_list(&audio_transmitter_scenario_list[i], 1)) {
                mic_used_flag = 1;
                scenario_and_id = (audio_transmitter_scenario_list[i].scenario_type << 8) | audio_transmitter_scenario_list[i].scenario_sub_id;
                audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: \
                scenario_and_id = %x is running, change mic to %d\n", 2, scenario_and_id, voice_mic_type);

                //DSP NVKEY PARA
                sysram_status_t status;
                DSP_FEATURE_TYPE_LIST AudioFeatureList_GameVoiceHead[2] = {
                    FUNC_GAMING_HEADSET,
                    FUNC_END,
                };

                //SUSPEND TRANSMITTER
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_SUSPEND, scenario_and_id, 0, true);
                if ((audio_transmitter_scenario_list[i].scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) && (audio_transmitter_scenario_list[i].scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET)) {
                    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_HEADSET, NULL, false);
                    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_HEADSET, open_param, true);

                    if (voice_mic_type == VOICE_MIC_TYPE_FIXED) {
                        AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_HEADSET;
                    } else if (voice_mic_type == VOICE_MIC_TYPE_DETACHABLE) {
                        AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_BOOM_MIC;
                    }
                }
#ifdef AIR_WIRED_AUDIO_ENABLE
                else {
                    line_out_stream_out_afe_config(NULL, audio_transmitter_scenario_list[i].scenario_sub_id, open_param);

                    if (audio_transmitter_scenario_list[i].scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT, NULL, false);
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT, open_param, true);
                    } else if (audio_transmitter_scenario_list[i].scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) {
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT, NULL, false);
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT, open_param, true);
                    } else if (audio_transmitter_scenario_list[i].scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) {
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER, NULL, false);
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER, open_param, true);
                    }

                    if (voice_mic_type == VOICE_MIC_TYPE_FIXED) {
                        AudioFeatureList_GameVoiceHead[0] = FUNC_RX_NR;
                    } else if (voice_mic_type == VOICE_MIC_TYPE_DETACHABLE) {
                        AudioFeatureList_GameVoiceHead[0] = FUNC_WB_BOOM_MIC;
                    }
                }
#endif

                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                status = audio_nvdm_set_feature(2, AudioFeatureList_GameVoiceHead);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[DETACHABLE_MIC][Transmitter] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                }

                // update MIC gain settings
                am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);

                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, 1, 0, false);

                //RESUME TRANSMITTER
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_RESUME, scenario_and_id, (uint32_t)p_param_share, true);
                break;
            }
        }
    } else {
        audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: [transmitter]no player use MIC currently, do nothing\n", 0);
    }
#endif//AIR_AUDIO_TRANSMITTER_ENABLE

    //Check MIC user record
    if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE)) {
        audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: current player is record\n", 0);
        if (mic_used_flag) {
            audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: unexpected!! MIC should not has multiple user!!\n", 0);
            //AUDIO_ASSERT(0);
        } else {
            mic_used_flag = 2;
            am_set_VC_volume();

            //SUSPEND RECORD
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_SUSPEND, 0, 0, true);
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_RECORD, NULL, false);
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_RECORD, open_param, true);
            //RESUME RECORD
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_RESUME, 0, (uint32_t)p_param_share, true);
        }
    }

    #ifdef AIR_DCHS_MODE_ENABLE
    audio_scenario_type_t scenario_type = AUDIO_SCENARIO_TYPE_COMMON;
    U32 code_type   = BT_HFP_CODEC_TYPE_NONE;
    U32 sample_rate = 0;
    dchs_check_ul_running_scenario(&scenario_type, &code_type, &sample_rate);
    if(scenario_type && code_type != AUDIO_CONTENT_TYPE_ULL_BLE){ //ULL2.0 will restart stream
        scenario_and_id = (AUDIO_TRANSMITTER_DCHS << 8) | AUDIO_TRANSMITTER_DCHS_UART_UL;
        //SUSPEND DCHS UL
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_SUSPEND, scenario_and_id, 0, true);
        //update nvkey
        dchs_ul_replace_feature_nvkey(scenario_type, code_type, sample_rate);
        audio_src_srv_report("[DETACHABLE_MIC][DCHS UL] mic type:%d change done, scenario_type:%d, code_type:%d, sample_rate:%d\n", 4, voice_mic_type, scenario_type, code_type, sample_rate);
        // update MIC gain settings
        am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);
        //RESUME DCHS UL
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_RESUME, scenario_and_id, (uint32_t)p_param_share, true);
    }
    #endif

    //Check MIC user HFP
    if (g_prCurrent_player == NULL) {
        audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: no current player\n", 0);
        vPortFree(open_param);
        #ifdef AIR_DCHS_MODE_ENABLE
        dchs_uart_detachable_mic_sync(voice_mic_type, SYNC_END);
        #endif
        //unmute stream in
        hal_audio_mute_stream_in_by_scenario(HAL_AUDIO_STREAM_IN_SCENARIO_HFP, FALSE);
        //Restart sidetone if sidetone is enabled
        if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_RESTART){
            ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_RESTART, false);
            audio_side_tone_enable_hdlr(amm_pt);
        }
        return;
    } else {
        audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: g_prCurrent_player = 0x%08x, type = %d\n", 2, g_prCurrent_player, g_prCurrent_player->type);
        if (HFP == g_prCurrent_player->type) {
            //audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: current player is HFP\n", 0);
            if (mic_used_flag) {
                audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: unexpected!! MIC should not has multiple user!!\n", 0);
                //AUDIO_ASSERT(0);
            } else {
                mic_used_flag = 3;
                //SUSPEND HFP
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_UL_SUSPEND, 0, 0, true);
#ifndef AIR_DCHS_MODE_ENABLE
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_HFP_UL, NULL, false);
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_HFP_UL, open_param, true);
#else
                ami_hal_audio_status_set_running_flag_detach_mic(AUDIO_SCENARIO_TYPE_HFP_UL, NULL, false);
                ami_hal_audio_status_set_running_flag_detach_mic(AUDIO_SCENARIO_TYPE_HFP_UL, open_param, true);
#endif
                //DSP NVKEY PARA
                sysram_status_t status;
                DSP_FEATURE_TYPE_LIST AudioFeatureList_WBeSCO[2] = {
                    FUNC_RX_NR,
                    FUNC_END,
                };
                if ((voice_mic_type == VOICE_MIC_TYPE_FIXED) && (g_prCurrent_player->local_context.hfp_format.hfp_codec.type == BT_HFP_CODEC_TYPE_CVSD)) {
                    AudioFeatureList_WBeSCO[0] = FUNC_TX_NR;
                } else if ((voice_mic_type == VOICE_MIC_TYPE_DETACHABLE) && (g_prCurrent_player->local_context.hfp_format.hfp_codec.type == BT_HFP_CODEC_TYPE_CVSD)) {
                    AudioFeatureList_WBeSCO[0] = FUNC_NB_BOOM_MIC;
                } else if ((voice_mic_type == VOICE_MIC_TYPE_DETACHABLE) && (g_prCurrent_player->local_context.hfp_format.hfp_codec.type == BT_HFP_CODEC_TYPE_MSBC)) {
                    AudioFeatureList_WBeSCO[0] = FUNC_WB_BOOM_MIC;
                }
                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                status = audio_nvdm_set_feature(2, AudioFeatureList_WBeSCO);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[DETACHABLE_MIC][HFP] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                }

                // update MIC gain settings
                am_set_HFP_volume(BT_HFP_CODEC_TYPE_MSBC, HAL_AUDIO_DEVICE_MAIN_MIC, 0, HAL_AUDIO_DEVICE_HEADSET, 0, true, false);

                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, 1, 0, false);
                //RESUME HFP with new stream in setting of the detachable mic
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_UL_RESUME, 0, (uint32_t)p_param_share, true);
            }
        }
        else if (BLE == g_prCurrent_player->type) {
            audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: current player is BLE\n", 0);
            if (mic_used_flag) {
                audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: unexpected!! MIC should not has multiple user!!\n", 0);
                //AUDIO_ASSERT(0);
        } else {
                mic_used_flag = 4;
                //SUSPEND BLE
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_SUSPEND, 0, 0, true);
#ifndef AIR_DCHS_MODE_ENABLE
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL, NULL, false);
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL, open_param, true);
#else
                ami_hal_audio_status_set_running_flag_detach_mic(AUDIO_SCENARIO_TYPE_BLE_UL, NULL, false);
                ami_hal_audio_status_set_running_flag_detach_mic(AUDIO_SCENARIO_TYPE_BLE_UL, open_param, true);
#endif
                //DSP NVKEY PARA
                sysram_status_t status;
                DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioBoom[2] = {
                    #if defined(AIR_BT_BLE_FB_ENABLE)
                    FUNC_RX_NR_FB,
                    #elif defined(AIR_BT_BLE_SWB_ENABLE)
                    FUNC_RX_NR_SWB,
                    #else
                    FUNC_RX_NR,
                    #endif
                    FUNC_END,
                };

                if (voice_mic_type == VOICE_MIC_TYPE_FIXED) {
                    #if defined(AIR_BT_BLE_FB_ENABLE)
                    AudioFeatureList_LEAudioBoom[0] = FUNC_RX_NR_FB;
                    #elif defined(AIR_BT_BLE_SWB_ENABLE)
                    AudioFeatureList_LEAudioBoom[0] = FUNC_RX_NR_SWB;
                    #else
                    AudioFeatureList_LEAudioBoom[0] = FUNC_RX_NR;
                    #endif
                } else if (voice_mic_type == VOICE_MIC_TYPE_DETACHABLE) {
                    #if defined(AIR_BT_BLE_FB_ENABLE)
                    AudioFeatureList_LEAudioBoom[0] = FUNC_FB_BOOM_MIC;
                    #elif defined(AIR_BT_BLE_SWB_ENABLE)
                    AudioFeatureList_LEAudioBoom[0] = FUNC_SWB_BOOM_MIC;
                    #else
                    AudioFeatureList_LEAudioBoom[0] = FUNC_WB_BOOM_MIC;
                    #endif
                }

                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                status = audio_nvdm_set_feature(2, AudioFeatureList_LEAudioBoom);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[DETACHABLE_MIC][BLE_UL] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                }

                // update MIC gain settings
                bt_sink_srv_audio_setting_vol_info_t vol_info;
                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = BT_HFP_CODEC_TYPE_MSBC;
                vol_info.vol_info.hfp_vol_info.dev_in = HAL_AUDIO_DEVICE_MAIN_MIC;
                vol_info.vol_info.hfp_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
                vol_info.vol_info.hfp_vol_info.lev_in = 0;
                vol_info.vol_info.hfp_vol_info.lev_out = 0;
                bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);

                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, 1, 0, false);
                //RESUME BLE with new stream in setting of the detachable mic
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_RESUME, 0, (uint32_t)p_param_share, true);
            }
        }
        else {
            audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: current player not use MIC\n", 2, g_prCurrent_player, g_prCurrent_player->type);
        }
    }
    vPortFree(open_param);
    //unmute stream in
    hal_audio_mute_stream_in_by_scenario(HAL_AUDIO_STREAM_IN_SCENARIO_HFP, FALSE);

    #ifdef AIR_DCHS_MODE_ENABLE
    dchs_uart_detachable_mic_sync(voice_mic_type, SYNC_END);
    #endif

    //Restart sidetone if sidetone is enabled
    if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_RESTART){
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_RESTART, false);
        audio_side_tone_enable_hdlr(amm_pt);
    }

    audio_src_srv_report("[DETACHABLE_MIC][Sink][AM]audio_set_voice_mic_type_hdlr: current MIC user is %d\n", 1, mic_used_flag);
}
#endif

/*****************************************************************************
 * FUNCTION
 *  am_task_main
 * DESCRIPTION
 *  This function is a main message handler on A.M. task.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
#ifdef WIN32_UT
void am_task_main(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    uint32_t bAud_timer_id = AM_TIMER_ID;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    hal_audio_register_stream_out_callback(aud_stream_out_callback, NULL);
#ifndef MTK_AM_NOT_SUPPORT_STREAM_IN
    hal_audio_register_stream_in_callback(aud_stream_in_callback, NULL);
#endif
    aud_initial();
    while (1) {
        am_receive_msg(g_prAmm_current);
        free(g_prAmm_current);
        if (g_prCurrent_player != NULL) {
            audio_src_srv_report("\tHave player: ID=%d\n", 1, g_prCurrent_player->aud_id);
        } else if (g_prCurrent_resumer != NULL) {
            audio_src_srv_report("\tHave resumer: ID=%d\n", 1, g_prCurrent_resumer->aud_id);
        }
        break;
    }
}
#else
QueueHandle_t g_xQueue_am;
/*g_audio_nvdm_init_flg is use for check if audio nvkey have init done*/
bool g_audio_nvdm_init_flg = false;
extern void bt_sink_srv_audio_setting_init_vol_level(void);

#if defined(AIR_HFP_FEATURE_MODE_ENABLE) && defined(AIR_BT_HFP_ENABLE)
extern SemaphoreHandle_t g_xSemaphore_hfp_nvkey_change_notify;
extern void hfp_feature_mode_notify_callback(audio_nvdm_user_t user, audio_nvdm_status_t status, void *user_data);
#endif
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
extern SemaphoreHandle_t g_xSemaphore_ble_nvkey_change_notify;
extern void ble_feature_mode_notify_callback(audio_nvdm_user_t user, audio_nvdm_status_t status, void *user_data);
#endif

void am_task_main(void *arg)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    bt_sink_srv_am_amm_struct *amm_temp_t = NULL;
    uint32_t bAud_timer_id = AM_TIMER_ID;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
#ifdef __AM_DEBUG_INFO__
    audio_src_srv_report("[Sink][AM] Start AM_Task", 0);
#endif
    g_xQueue_am = xQueueCreate(AM_QUEUE_NUMBER, sizeof(bt_sink_srv_am_amm_struct *));

    if (g_xQueue_am != 0) {
        g_xSemaphore_ami = xSemaphoreCreateMutex();
        g_xSemaphore_Audio = xSemaphoreCreateMutex();
        g_xTimer_am = xTimerCreate("Timer",
                                   (AM_TIMER_PERIOD / portTICK_PERIOD_MS),
                                   pdTRUE,
                                   (void *)bAud_timer_id,
                                   aud_timer_callback);
        hal_audio_register_stream_out_callback(aud_stream_out_callback, NULL);
#ifndef MTK_AM_NOT_SUPPORT_STREAM_IN
#ifdef AIR_RECORD_ENABLE
        hal_audio_register_stream_in_callback(aud_stream_in_callback, NULL);
#endif
#endif

        aud_initial();

#ifdef AIR_RECORD_ENABLE
        record_init();
#endif

#if defined(MTK_AVM_DIRECT)
        hal_audio_set_task_notification_callback(audio_event_task_notification);
        hal_audio_set_task_ms_delay_function(audio_task_delay_ms_func);
        if (ptr_callback_amm == NULL) {
            ptr_callback_amm = (bt_sink_srv_am_amm_struct *)pvPortMalloc(sizeof(bt_sink_srv_am_amm_struct));
            if (ptr_callback_amm == NULL) {
                AUDIO_ASSERT(0 && "[AM][ERROR]Init audio ptr_cb fail.");
            }
        }
        if (ptr_isr_callback_amm == NULL) {
            ptr_isr_callback_amm = (bt_sink_srv_am_amm_struct *)pvPortMalloc(sizeof(bt_sink_srv_am_amm_struct));
            if (ptr_isr_callback_amm == NULL) {
                AUDIO_ASSERT(0 && "[AM][ERROR]Init audio ptr_isr_cb fail.");
            }
        }

        if (AUD_EXECUTION_FAIL == audio_nvdm_configure_init()) {
            audio_src_srv_report("[AM][ERROR]Init audio configure from NVDM.", 0);
        }
        am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_STEREO); //Default
#endif

#if defined(MTK_AMP_DC_COMPENSATION_ENABLE)
        audio_driver_dc_compensation();
#endif
#ifdef AIR_KEEP_I2S_ENABLE
        audio_driver_set_device(HAL_AUDIO_DEVICE_I2S_MASTER, HAL_AUDIO_INTERFACE_1, true);
#endif

        //Register audio callback
        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_AFE, audio_message_audio_handler, NULL);

#if defined(AIR_HFP_FEATURE_MODE_ENABLE) && defined(AIR_BT_HFP_ENABLE)
        //Register hfp nvkey real time change process callback
        g_xSemaphore_hfp_nvkey_change_notify = xSemaphoreCreateBinary();
        audio_nvdm_register_user_callback(AUDIO_NVDM_USER_HFP, hfp_feature_mode_notify_callback, NULL);
#endif
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
        //Register ble nvkey real time change process callback
        g_xSemaphore_ble_nvkey_change_notify = xSemaphoreCreateBinary();
        audio_nvdm_register_user_callback(AUDIO_NVDM_USER_BLE, ble_feature_mode_notify_callback, NULL);
#endif

        am_load_audio_output_volume_parameters_from_nvdm();

        DSP_PARA_DATADUMP_STRU *DumpInfo = (DSP_PARA_DATADUMP_STRU *)pvPortMalloc(sizeof(DSP_PARA_DATADUMP_STRU));
        get_audio_dump_info_from_nvdm(DumpInfo);
#ifdef MTK_AUDIO_DUMP_BY_SPDIF_ENABLE
        mcu2dsp_open_param_t open_param;
        open_param.param.stream_in = STREAM_IN_DUMMY;
        open_param.param.stream_out = STREAM_OUT_AFE;
        open_param.stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_SPDIF;
        open_param.stream_out_param.afe.sampling_rate = 96000;
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_SPDIF, &open_param, true);
#endif
        memcpy(audio_dump_ids,DumpInfo->Dump_IDs,sizeof(audio_dump_ids));

        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_DUMP_MASK, 0, hal_memview_mcu_to_dsp0((uint32_t)audio_dump_ids), false);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_DUMP_MASK, 1, DumpInfo->Dump_Id_Version, false);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_DUMP_MASK, 2, DumpInfo->Dump_Scenario_Sel, false);

        vPortFree(DumpInfo);

#ifdef AIR_SILENCE_DETECTION_ENABLE
        audio_silence_detection_nvdm_ccni();
#endif

#ifdef __BT_SINK_AUDIO_TUNING__
        bt_sink_hf_audio_tuning_atci_init();
        bt_sink_srv_audio_tunning_init();
#endif /* __BT_SINK_AUDIO_TUNING__ */
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
        bt_sink_srv_audio_setting_init();
        bt_sink_srv_audio_setting_init_vol_level();
#endif /* bt_sink_srv_audio_setting_init */

#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
        audio_anc_control_init();
#ifdef MTK_USER_TRIGGER_FF_ENABLE
        audio_user_trigger_adaptive_ff_initialize();
#endif
#if defined (AIR_AUDIO_TRANSMITTER_ENABLE) && defined (MTK_ANC_SURROUND_MONITOR_ENABLE)
        audio_anc_monitor_anc_init();
        audio_anc_control_register_callback((audio_anc_control_callback_t) audio_anc_monitor_anc_callback, AUDIO_ANC_CONTROL_EVENT_ON | AUDIO_ANC_CONTROL_EVENT_OFF | AUDIO_ANC_CONTROL_EVENT_SUSPEND_NOTIFY | AUDIO_ANC_CONTROL_EVENT_RESUME_NOTIFY, AUDIO_ANC_CONTROL_CB_LEVEL_ALL);
#endif
#if defined (AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        llf_control_init();
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
        audio_anc_vivid_pt_init();
#endif
#if defined(AIR_CUSTOMIZED_LLF_ENABLE)
        audio_llf_sample_init();
#endif
#else
        anc_init();
#endif

#ifdef MTK_AWS_MCE_ENABLE
        audio_anc_control_register_interface(AUDIO_ANC_SYNC_CONTROL);
#else
#ifdef AIR_DCHS_MODE_ENABLE
        audio_anc_control_register_interface(AUDIO_ANC_COSYS_CONTROL);
#else
        audio_anc_control_register_interface(AUDIO_ANC_CONTROL);
#endif
#endif


#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        race_cosys_register_data_callback(RACE_COSYS_MODULE_ID_ANC_PASSTHROUGH, RACE_DSPREALTIME_ANC_PASSTHRU_RELAY_COSYS_CALLBACK);
#ifdef RACE_DSP_REALTIME_CMD_ENABLE
        race_cosys_register_data_callback(RACE_COSYS_MODULE_ID_DSP_COMMON, race_dsprealtime_cosys_relay_callback);
#endif
#ifdef RACE_HOSTAUDIO_CMD_ENABLE
        race_cosys_register_data_callback(RACE_COSYS_MODULE_ID_HOST_AUDIO_SET, race_cmd_hostaudio_cosys_relay_set_cmd_callback);
#endif
#if defined(MTK_ANC_ENABLE)&&defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
        g_race_cosys_dsp_realtime_callback = RACE_DSPREALTIME_callback;
#endif
#endif
#ifdef MTK_AWS_MCE_ENABLE
        bt_callback_manager_register_callback(bt_callback_type_app_event, (uint32_t)MODULE_MASK_AWS_MCE, (void *)bt_event_ami_callback);
#endif

#ifdef AIR_DCHS_MODE_ENABLE
        //mcu_uart_init();
        // hal_pinmux_set_function(HAL_GPIO_17, 0);
        // hal_gpio_set_direction(HAL_GPIO_17, HAL_GPIO_DIRECTION_OUTPUT);
        // hal_gpio_set_output(HAL_GPIO_17, HAL_GPIO_DATA_LOW);
        if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
            mcu_uart_open(UART_AUDIO_CMD);
        }
#endif
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
        audio_calibration_init(NULL, NULL, NULL);
#endif
#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
    bt_sink_srv_audio_setting_init_audio_platform_info();
#endif
#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
        DL_SW_gain_init();
#endif
#ifdef FPGA_ENV
        audio_src_srv_report("[Sink][AM] AM task start ok", 0);
#endif
        while (1) {
            if (xQueueReceive(g_xQueue_am, &amm_temp_t, portMAX_DELAY)) {
                am_receive_msg((bt_sink_srv_am_amm_struct *)amm_temp_t);
                if ((amm_temp_t != ptr_callback_amm) && (amm_temp_t != ptr_isr_callback_amm)) {
                    vPortFree(amm_temp_t);
                }
            }
        }
    }
#ifdef __AM_DEBUG_INFO__
    audio_src_srv_report("[Sink][AM][ERROR] Start AM_Task Error", 0);
#endif
}
#endif /* WIN32_UT */

/*****************************************************************************
 * FUNCTION
 *  am_receive_msg
 * DESCRIPTION
 *  This function is a main message dispatching function of A.M.
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void am_receive_msg(bt_sink_srv_am_amm_struct *amm_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /*
    #ifdef __AM_DEBUG_INFO__
     audio_src_srv_report("[Sink][AM] Received Message", 0);
     audio_src_srv_report("[Sink][AM] Aud ID: %d, Type: %d, Priority: %d, Vol_D: %d, Vol_A: %d, SR: %d",6,
                     amm_ptr->background_info.aud_id,
                     amm_ptr->background_info.type,
                     amm_ptr->background_info.priority,
                     amm_ptr->background_info.audio_stream_out.audio_volume.digital_gain_index,
                     amm_ptr->background_info.audio_stream_out.audio_volume.analog_gain_index,
                     amm_ptr->background_info.local_context.pcm_format.stream.stream_sample_rate
                   );
    #endif
    */
    switch (amm_ptr->msg_id) {
        case MSG_ID_STREAM_OPEN_REQ:
            aud_set_open_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_PLAY_REQ:
            aud_set_play_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_PLAY_REQ_SUB:
            test_aud_set_play_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_STOP_REQ:
            aud_set_stop_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_STOP_REQ_SUB:
            test_aud_set_stop_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_CLOSE_REQ:
            aud_set_close_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_SET_VOLUME_REQ:
            aud_set_volume_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_MUTE_DEVICE_REQ:
            aud_mute_device_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_CONFIG_DEVICE_REQ:
            aud_config_device_stream_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_READ_WRITE_DATA_REQ:
            aud_rw_stream_data_req_hdlr(amm_ptr);
            break;
        case MSG_ID_STREAM_GET_LENGTH_REQ:
            aud_query_stream_len_req_hdlr(amm_ptr);
            break;

        case MSG_ID_MEDIA_A2DP_PROC_CALL_EXT_REQ:
            aud_process_a2dp_callback_hdlr(amm_ptr);
            break;
        case MSG_ID_MEDIA_HFP_EVENT_CALL_EXT_REQ:
#ifdef AIR_BT_HFP_ENABLE
            aud_process_hfp_callback_hdlr(amm_ptr);
#endif
            break;
        case MSG_ID_MEDIA_EVENT_STREAM_OUT_CALL_EXT_REQ:
            aud_event_stream_callback_hdlr(amm_ptr);
            break;
        case MSG_ID_MEDIA_EVENT_STREAM_IN_CALL_EXT_REQ:
            aud_event_stream_callback_hdlr(amm_ptr);
            break;
        case MSG_ID_TIMER_OUT_CALL_EXT_REQ:
            aud_timer_out_callback_hdlr(amm_ptr);
            break;
        case MSG_ID_MEDIA_AWS_A2DP_PROC_CALL_EXT_REQ:
#if defined(__BT_AWS_SUPPORT__) || defined(MTK_AWS_MCE_ENABLE)
            aud_process_aws_a2dp_callback_hdlr(amm_ptr);
#endif
            break;
        case MSG_ID_MEDIA_FILE_PROCE_CALL_EXT_REQ:

            break;

#if defined(MTK_AVM_DIRECT)
        case MSG_ID_HAL_EVENT_EXT_REQ:
            hal_audio_dsp_message_process();
            break;
#endif

        case MSG_ID_AUDIO_SET_PAUSE:
            audio_set_pause_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_SET_RESUME:
            audio_set_resume_hdlr(amm_ptr);
            break;

        case MSG_ID_AUDIO_SIDE_TONE_ENABLE:
            audio_side_tone_enable_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_SIDE_TONE_DISABLE:
            audio_side_tone_disable_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_SET_SIDE_TONE_VOLUME:
            audio_side_tone_set_volume_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_DL_SUSPEND:
            audio_dl_suspend_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_DL_RESUME:
            audio_dl_resume_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_UL_SUSPEND:
            audio_ul_suspend_hdlr(amm_ptr);
            break;
        case MSG_ID_AUDIO_UL_RESUME:
            audio_ul_resume_hdlr(amm_ptr);
            break;

        case MSG_ID_AUDIO_SET_FEATURE:
            audio_set_feature_hdlr(amm_ptr);
            break;

#if defined(MTK_VENDOR_SOUND_EFFECT_ENABLE)
        case MSG_ID_AUDIO_SET_VENDOR_SE:
            audio_set_vendor_se_hdlr(amm_ptr);
            break;
#endif
#if defined(AIR_AUDIO_TRANSMITTER_ENABLE)
        case MSG_ID_TRANS_SET_CONFIG_REQ:
            audio_set_audio_transmitter_config(amm_ptr);
            break;
#endif
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        case MSG_ID_VOICE_SET_MIC_TYPE_REQ:
            audio_set_voice_mic_type_hdlr(amm_ptr);
            break;
#endif
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
        case MSG_ID_AUDIO_REQUEST_SYNC:
            audio_request_sync_hdlr(amm_ptr);
            break;
#endif /* AIR_BT_AUDIO_SYNC_ENABLE */
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        case MSG_ID_AUDIO_VOLUME_MONITOR_STOP:
            audio_volume_monitor_stop_hdlr(amm_ptr);
            break;
#endif
        default:
            break;
    }
}

#ifdef AIR_AM_DIRECT_EXEC_ENABLE

/*
 * Execute AM func directry/immediately
 */
void am_direct_exec(bt_sink_srv_am_amm_struct *amm_temp_t)
{
    am_receive_msg((bt_sink_srv_am_amm_struct *)amm_temp_t);
    if ((amm_temp_t != ptr_callback_amm) && (amm_temp_t != ptr_isr_callback_amm)) {
        vPortFree(amm_temp_t);
    }
}
#endif

#if STANDALONE_TEST
/* Hal API func */
#ifndef WIN32_UT
hal_audio_status_t hal_audio_init(void)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_deinit(void)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_set_stream_out_sampling_rate(hal_audio_sampling_rate_t sample_rate)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_set_stream_out_channel_number(hal_audio_channel_number_t channel_number)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_start_stream_out(hal_audio_active_type_t active_type)
{
    return HAL_AUDIO_STATUS_OK;
}

void hal_audio_stop_stream_out(void)
{}

hal_audio_status_t hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, uint32_t digital_volume_index, uint32_t analog_volume_index)
{
#ifdef __AM_DEBUG_INFO__
    audio_src_srv_report("[Sink][AM] Digital: %04x, Analog: %04x", 2, digital_volume_index, analog_volume_index);
#endif
    return HAL_AUDIO_STATUS_OK;
}

void hal_audio_mute_stream_out(bool mute)
{}

hal_audio_status_t hal_audio_set_stream_out_device(hal_audio_device_t device)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_write_stream_out(const void *buffer, uint32_t size)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_get_stream_out_sample_count(uint32_t *sample_count)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_register_stream_out_callback(hal_audio_stream_out_callback_t callback, void *user_data)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_set_stream_in_sampling_rate(hal_audio_sampling_rate_t sample_rate)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_set_stream_in_channel_number(hal_audio_channel_number_t channel_number)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_start_stream_in(hal_audio_active_type_t active_type)
{
    return HAL_AUDIO_STATUS_OK;
}

void hal_audio_stop_stream_in(void)
{}

hal_audio_status_t hal_audio_set_stream_in_volume(uint32_t digital_volume_index, uint32_t analog_volume_index)
{
#ifdef __AM_DEBUG_INFO__
    audio_src_srv_report("[Sink][AM] Digital: %04x, Analog: %04x", 2, digital_volume_index, analog_volume_index);
#endif
    return HAL_AUDIO_STATUS_OK;
}

void hal_audio_mute_stream_in(bool mute)
{}

hal_audio_status_t hal_audio_set_stream_in_device(hal_audio_device_t device)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_read_stream_in(void *buffer, uint32_t sample_count)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_get_stream_in_sample_count(uint32_t *sample_count)
{
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_register_stream_in_callback(hal_audio_stream_in_callback_t callback, void *user_data)
{
    return HAL_AUDIO_STATUS_OK;
}


/* BT API func */
bt_media_handle_t *temp_t = NULL;
bt_media_handle_t *hf_temp_t = NULL;

void bt_codec_a2dp_set_buffer(bt_media_handle_t *handle, uint8_t  *buffer, uint32_t  length)
{

}

void bt_codec_a2dp_get_write_buffer(bt_media_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    *length = 500;
}

void bt_codec_a2dp_reset_buffer(bt_media_handle_t *handle)
{

}

bt_status_t bt_codec_a2dp_play(bt_media_handle_t *handle)
{
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_status_t bt_codec_a2dp_stop(bt_media_handle_t *handle)
{
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_media_handle_t *bt_codec_a2dp_open(bt_codec_a2dp_callback_t callback, const bt_sink_srv_am_a2dp_codec_t *param)
{
    temp_t = (bt_media_handle_t *)pvPortMalloc(sizeof(bt_media_handle_t));
    temp_t->set_buffer         = bt_codec_a2dp_set_buffer;
    temp_t->get_write_buffer   = bt_codec_a2dp_get_write_buffer;
    temp_t->reset_share_buffer = bt_codec_a2dp_reset_buffer;
    temp_t->play               = bt_codec_a2dp_play;
    temp_t->stop               = bt_codec_a2dp_stop;
    return (bt_media_handle_t *)temp_t;
}

bt_status_t bt_codec_a2dp_close(bt_media_handle_t *handle)
{
    vPortFree(temp_t);
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_status_t bt_codec_hfp_play(bt_media_handle_t *handle)
{
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_status_t bt_codec_hfp_stop(bt_media_handle_t *handle)
{
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_media_handle_t *bt_codec_hfp_open(bt_codec_hfp_callback_t callback, const bt_sink_srv_am_hfp_codec_t *param)
{
    hf_temp_t = (bt_media_handle_t *)pvPortMalloc(sizeof(bt_media_handle_t));
    hf_temp_t->play = bt_codec_hfp_play;
    hf_temp_t->stop = bt_codec_hfp_stop;
    return (bt_media_handle_t *)hf_temp_t;
}

bt_status_t bt_codec_hfp_close(bt_media_handle_t *handle)
{
    vPortFree(hf_temp_t);
    return BT_STATUS_SUCCESS;
}
#endif /*#ifndef WIN32_UT*/
#endif

static void am_files_codec_open(bt_sink_srv_am_files_format_t *files_format)
{
    am_file_type_t type = FILE_NONE;
    //int32_t len = 0;
    //bt_sink_srv_file_path_t *path = NULL;
    int32_t ret = 0, err = 0;

#if 0
#if FILE_PATH_TYPE_LEN == 1
    path = files_format->path;
    len = strlen(path);
    if (strncmp(&path[len - 4], ".mp3", 4) == 0) {
        type = FILE_MP3;
    } else if (strncmp(&path[len - 4], ".wav", 4) == 0) {
        type = FILE_WAV;
    } else {
        type = FILE_NONE;
    }
#else
    path = (wchar_t *)files_format->path;
    len = wcslen(path);
    if (wcsncmp(&path[len - 4], L".mp3", 4) == 0) {
        type = FILE_MP3;
    } else if (wcsncmp(&path[len - 4], L".wav", 4) == 0) {
        type = FILE_WAV;
    } else {
        type = FILE_NONE;
    }
#endif
#endif
    type = files_format->file_type;
    switch (type) {
        case FILE_MP3: {
            break;
        }

        case FILE_WAV: {
            break;
        }

        default:
            break;
    }

    if (ret > AM_ERR_SUCCESS_OK) {
        g_am_file_state = FILE_CODEC_OPEN;
        g_rAm_aud_id[g_prCurrent_player->aud_id].use = ID_PLAY_STATE;
        g_prCurrent_player->notify_cb(g_prCurrent_player->aud_id,
                                      AUD_FILE_OPEN_CODEC, AUD_FILE_PROC_PTR, &g_am_files_media_handle);

    }

#ifdef __AM_DEBUG_INFO__
    audio_src_srv_report("[Sink][AM]files_codec_open--type: %d, ret: %d, err: %d", 3, type, ret, err);
#endif
#ifndef MTK_DEBUG_LEVEL_INFO
    g_test_fix_warning = err;
#endif
}


static void am_files_codec_close(void)
{
    int32_t ret = AM_ERR_FAIL_7TH, err = AM_ERR_FAIL_7TH;
    am_file_type_t type = FILE_NONE;

    type = g_am_files_media_handle.type;

    switch (type) {
        case FILE_MP3: {
            break;
        }

        case FILE_WAV: {
#ifdef __AUDIO_WAV_ENABLE__
            if (g_am_audio_media_handle) {
                if (g_am_file_state == FILE_CODEC_PLAY) {
                    err = g_am_audio_media_handle->stop(g_am_audio_media_handle);
                }
                ret = audio_codec_wav_codec_close(g_am_audio_media_handle);
                g_am_audio_media_handle = NULL;
                g_am_file_state = FILE_CODEC_CLOSE;
            } else {
                audio_src_srv_report("[Sink][AM]files_codec_close--empty hd", 0);
            }
#endif /*__AUDIO_WAV_ENABLE__*/
            break;
        }

        default:
            break;
    }

    audio_src_srv_report("[Sink][AM]files_codec_close--type: %d, ret: %d, err: %d, f_state: %d", 4, type, ret, err, g_am_file_state);
#ifndef MTK_DEBUG_LEVEL_INFO
    g_test_fix_warning = ret;
    //g_test_fix_warning = err;
    _UNUSED(ret);
#endif
}

static void audio_set_pause_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    //send DSP mute message
    audio_src_srv_report("[Sink][AM]audio_set_pause_hdlr\n", 0);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_OUTPUT_DEVICE, 0, (HAL_AUDIO_STREAM_OUT1 << 16) | 1, false);
}

static void audio_set_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    bt_sink_srv_am_background_t *am_background_temp = &(amm_ptr->background_info);
    //send DSP deinit message
    audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr with current am type: %d\n", 1, am_background_temp->type);

    switch (am_background_temp->type) {
        case A2DP:
        case AWS:
        case USB_AUDIO_IN: {
#ifdef MTK_PEQ_ENABLE
            bt_sink_srv_am_peq_param_t ami_peq_param;
            sysram_status_t status;
            DSP_FEATURE_TYPE_LIST AudioFeatureList_A2DP[2] = {
                FUNC_PEQ_A2DP,
                FUNC_END,
            };
            memset(&ami_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(2, AudioFeatureList_A2DP);
            if (status != NVDM_STATUS_NAT_OK) {
                audio_src_srv_report("A2DP set resume is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            }
            /* set pre PEQ*/
            ami_peq_param.phase_id = 0;
            ami_peq_param.enable = g_peq_handle.a2dp_pre_peq_enable;
            ami_peq_param.sound_mode = g_peq_handle.a2dp_pre_peq_sound_mode;
            ami_peq_param.setting_mode = PEQ_DIRECT;
            aud_set_peq_param(aud_get_peq_audio_path(g_prCurrent_player->type), &ami_peq_param);
            /* set post PEQ*/
            ami_peq_param.phase_id = 1;
            ami_peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
            ami_peq_param.sound_mode = g_peq_handle.a2dp_post_peq_sound_mode;
            ami_peq_param.setting_mode = PEQ_DIRECT;
            aud_set_peq_param(aud_get_peq_audio_path(g_prCurrent_player->type), &ami_peq_param);
#endif
            break;
        }
        case LINE_IN: {
#if defined(MTK_LINEIN_PEQ_ENABLE) || defined(MTK_LINEIN_INS_ENABLE)
            bt_sink_srv_am_peq_param_t ami_peq_param;
            sysram_status_t status;
            DSP_FEATURE_TYPE_LIST AudioFeatureList_LINEIN[] = {
#ifdef MTK_LINEIN_INS_ENABLE
                FUNC_INS,
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
                FUNC_PEQ_LINEIN,
#endif
                FUNC_END,
            };
            memset(&ami_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LINEIN) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LINEIN);
            if (status != NVDM_STATUS_NAT_OK) {
                audio_src_srv_report("LINEIN set resume is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            }
            /* set pre PEQ*/
            ami_peq_param.phase_id = 0;
            ami_peq_param.enable = g_peq_handle.linein_pre_peq_enable;
            ami_peq_param.sound_mode = g_peq_handle.linein_pre_peq_sound_mode;
            ami_peq_param.setting_mode = PEQ_DIRECT;
            aud_set_peq_param(aud_get_peq_audio_path(g_prCurrent_player->type), &ami_peq_param);

            ami_peq_param.phase_id = 1;
            ami_peq_param.enable = g_peq_handle.linein_post_peq_enable;
            ami_peq_param.sound_mode = g_peq_handle.linein_post_peq_sound_mode;
            ami_peq_param.setting_mode = PEQ_DIRECT;
            aud_set_peq_param(aud_get_peq_audio_path(g_prCurrent_player->type), &ami_peq_param);
#endif
            break;
        }
#ifdef AIR_BT_HFP_ENABLE
        case HFP: {
            sysram_status_t status;
            bt_hfp_audio_codec_type_t codec_type;
            DSP_FEATURE_TYPE_LIST AudioFeatureList_eSCO[3];

            codec_type = g_prCurrent_player->local_context.hfp_format.hfp_codec.type;
            if (codec_type == BT_HFP_CODEC_TYPE_CVSD) {
                AudioFeatureList_eSCO[0] = FUNC_TX_NR;
            } else {
                AudioFeatureList_eSCO[0] = FUNC_RX_NR;
            }
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                AudioFeatureList_eSCO[1] = FUNC_SILENCE_DETECTION;
                AudioFeatureList_eSCO[2] = FUNC_END;
#else
                AudioFeatureList_eSCO[1] = FUNC_END;
#endif

#if defined(AIR_HFP_FEATURE_MODE_ENABLE) || defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE)
            hfp_replace_feature_mode_nvkey_id(codec_type, AudioFeatureList_eSCO);
#endif

            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(AudioFeatureList_eSCO) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_eSCO);
            if (status != NVDM_STATUS_NAT_OK) {
                audio_src_srv_err("eSCO set resume is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            }
            break;
        }
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
        case BLE:
        {
            sysram_status_t status;
            //audio_src_srv_report("[DEBUG]LE audio call set resume ch %d context %d", 2, g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode,g_prCurrent_player->local_context.ble_format.ble_codec.context_type);
            if(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_UL_BOTH){
                if(g_prCurrent_player->local_context.ble_format.ble_codec.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL){
                    DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[3] =
                    {
#if defined(AIR_BT_BLE_FB_ENABLE)
                        FUNC_RX_NR_FB,
#elif defined(AIR_BT_BLE_SWB_ENABLE)
                        FUNC_RX_NR_SWB,
#else
                        FUNC_RX_NR,
#endif
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                        FUNC_SILENCE_DETECTION,
#endif
                        FUNC_END,
                    };

#if defined(AIR_BLE_FEATURE_MODE_ENABLE) || defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE)
                    ble_replace_feature_mode_nvkey_id(AudioFeatureList_LEAudioCall);
#endif

                    /* reset share buffer before put parameters*/
                    audio_nvdm_reset_sysram();
                    status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
                    if (status != NVDM_STATUS_NAT_OK){
                        audio_src_srv_report("LE audio call set resume is failed to set parameters to share memory - err(%d)\r\n", 1, status);
                        AUDIO_ASSERT(0);
                    }
                }else {
                    DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[4] = {
                        FUNC_TX_NR_v2,
#ifdef MTK_PEQ_ENABLE
                        FUNC_PEQ_A2DP,
#endif
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                        FUNC_SILENCE_DETECTION,
#endif
                        FUNC_END,
                };
                    /* reset share buffer before put parameters*/
                    audio_nvdm_reset_sysram();
                    status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
                    if (status != NVDM_STATUS_NAT_OK) {
                        audio_src_srv_report("LE audio call failed to set parameters to share memory - err(%d)\r\n", 1, status);
                        AUDIO_ASSERT(0);
                    }
                }
            }
            break;
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case ULL_BLE_DL:
        {
            aud_prepare_a2dp_nvkey(&(am_background_temp->local_feature));
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, AUDIO_STRAM_DEINIT_ULL_DL, 0, false);
            break;
        }

        // TODO: ULL UL support runting turning
        case ULL_BLE_UL:
        {
            sysram_status_t status;
            DSP_FEATURE_TYPE_LIST AudioFeatureList_LEAudioCall[] = {
                #if defined(AIR_BT_ULL_SWB_ENABLE)
                FUNC_RX_NR_SWB,
                #else
                FUNC_RX_NR,
                #endif
                //FUNC_PEQ_A2DP,
                FUNC_END,
            };
            #if defined(AIR_BT_ULL_FB_ENABLE)
            if (am_background_temp->local_context.ull_ble_ul_format.ble_codec.sample_rate == 48000) {
                AudioFeatureList_LEAudioCall[0] = FUNC_RX_NR_FB;
                audio_src_srv_report("[BLE][ULL] config FB nvkey", 0);
            }
            #endif

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
            if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                #if defined(AIR_BT_ULL_SWB_ENABLE)
                AudioFeatureList_LEAudioCall[0] = FUNC_SWB_BOOM_MIC;
                #else
                AudioFeatureList_LEAudioCall[0] = FUNC_WB_BOOM_MIC;
                #endif
                #if defined(AIR_BT_ULL_FB_ENABLE)
                if (am_background_temp->local_context.ull_ble_ul_format.ble_codec.sample_rate == 48000) {
                    AudioFeatureList_LEAudioCall[0] = FUNC_FB_BOOM_MIC;
                    audio_src_srv_report("[BLE][ULL] config FB Boomic nvkey", 0);
                }
                #endif
                audio_src_srv_report("[BLE][ULL] ble feature mode: boom mic %d", 1, mic_cur_type);
            }
#endif

            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(AudioFeatureList_LEAudioCall) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_LEAudioCall);
            if (status != NVDM_STATUS_NAT_OK) {
                AUDIO_ASSERT(0 && "ULL LE UL failed to set parameters to share memory");
            }
            audio_src_srv_report("[BLE][ULL] ULL LE UL load nvdm ok 0x%02x\r\n", 1, AudioFeatureList_LEAudioCall[0]);

            break;
        }
#endif
#if defined (AIR_AUDIO_TRANSMITTER_ENABLE)
        case AUDIO_TRANSMITTER:
        {
            #ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            audio_transmitter_scenario_list_t voice_headset_scenario_list[]  =
                {{AUDIO_TRANSMITTER_GAMING_MODE, AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET},
                };
            if(true == audio_transmitter_get_is_running_by_scenario_list(voice_headset_scenario_list, sizeof(voice_headset_scenario_list) / sizeof(audio_transmitter_scenario_list_t))){
                voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();

                //DSP NVKEY PARA
                sysram_status_t status;
                DSP_FEATURE_TYPE_LIST AudioFeatureList_GameVoiceHead[2] = {
                    FUNC_GAMING_HEADSET,
                    FUNC_END,
                };
                audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr audio transmitter mic type = %d", 1, mic_cur_type);
                if(mic_cur_type == VOICE_MIC_TYPE_FIXED) {
                    AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_HEADSET;
                } else if(mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                    AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_BOOM_MIC;
                }
                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                status = audio_nvdm_set_feature(2, AudioFeatureList_GameVoiceHead);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr audio transmitter set resume failed to set parameters to share memory - err(%d)\r\n", 1, status);
                }
            }
            #endif
            #if defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE) && defined(AIR_WIRED_AUDIO_ENABLE)
            audio_transmitter_scenario_list_t voice_wired_audio_out_list[]  =
                {{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT},
                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER},
                };
            if(true == audio_transmitter_get_is_running_by_scenario_list(voice_wired_audio_out_list, sizeof(voice_wired_audio_out_list) / sizeof(audio_transmitter_scenario_list_t))){
                voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();

                //DSP NVKEY PARA
                sysram_status_t status;
                DSP_FEATURE_TYPE_LIST AudioFeatureList_WiredAudioOut[2] = {
                    FUNC_RX_NR,
                    FUNC_END,
                };
                audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr audio transmitter mic type = %d", 1, mic_cur_type);
                if(mic_cur_type == VOICE_MIC_TYPE_FIXED) {
                    AudioFeatureList_WiredAudioOut[0] = FUNC_RX_NR;
                } else if(mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                    AudioFeatureList_WiredAudioOut[0] = FUNC_WB_BOOM_MIC;
                }
                /* reset share buffer before put parameters*/
                audio_nvdm_reset_sysram();
                status = audio_nvdm_set_feature(2, AudioFeatureList_WiredAudioOut);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr audio transmitter set resume failed to set parameters to share memory - err(%d)\r\n", 1, status);
                }
            }
            #endif
            #ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
            audio_transmitter_scenario_list_t voice_dongle_usb_out_scenario_list[]  =
                {{AUDIO_TRANSMITTER_GAMING_MODE, AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT}};
            if(audio_transmitter_get_is_running_by_scenario_list(&voice_dongle_usb_out_scenario_list[0], 1) == true){
                bt_ull_streaming_t stream;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                /* reopen audio stream for loading new settings */
                /* stop stream */
                audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr stop ULL uplink stream", 0);
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(bt_ull_streaming_t));
                /* delay 100ms */
                vTaskDelay(100 / portTICK_RATE_MS);
                /* start stream */
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(bt_ull_streaming_t));
                audio_src_srv_report("[Sink][AM]audio_set_resume_hdlr start ULL uplink stream", 0);
    }
            #endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */
    }
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

        default:
            break;
    }

    if(am_background_temp->type != ULL_BLE_DL)
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, AUDIO_STRAM_DEINIT_ALL, 0, false);

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_OUTPUT_DEVICE, 0, ((HAL_AUDIO_STREAM_OUT_ALL << 16) | 0), false);
    if(g_prCurrent_player != NULL) {
        am_background_temp->in_out = STREAM_IN;
        am_background_temp->audio_stream_in.audio_mute = false;
        am_background_temp->audio_stream_in.audio_volume = g_prCurrent_player->audio_stream_in.audio_volume;;
        aud_set_volume_stream_req_hdlr(amm_ptr);
        am_background_temp->in_out = STREAM_OUT;
        am_background_temp->audio_stream_out.audio_mute = false;
        am_background_temp->audio_stream_out.audio_volume = g_prCurrent_player->audio_stream_out.audio_volume;;
        aud_set_volume_stream_req_hdlr(amm_ptr);
    }
}

static void audio_dl_suspend_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
    audio_src_srv_report("[Sink][AM]audio_dl_suspend_hdlr\n", 0);
    aud_dl_suspend();
#endif
}

static void audio_dl_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
    audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr\n", 0);
    if (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)) {
        aud_dl_resume();
    } else {
        if (g_prCurrent_player != NULL) {
            if (g_prCurrent_player->type == HFP) {
                aud_dl_resume();
                audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr eSCO/BLE happening. type=%d\n", 1, g_prCurrent_player->type);
            }
#ifdef AIR_BT_CODEC_BLE_ENABLED
            else if((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY)){
                aud_dl_resume();
                audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr eSCO/BLE happening. type=%d\n", 1, g_prCurrent_player->type);
            }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            else if ((g_prCurrent_player->type == ULL_BLE_DL) && (g_bt_ull_le_am_priority == AUDIO_SRC_SRV_PRIORITY_HIGH)) {
                aud_dl_resume();
                audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr ULL_BLE happening. type=%d priority: %d\n", 2, g_prCurrent_player->type, g_bt_ull_le_am_priority);
            }
#endif
#endif
        }
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
        audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr fail, sidetone still exist\n", 0);
#else
        aud_dl_resume();
        audio_src_srv_report("[Sink][AM]audio_dl_resume_hdlr pass, not suspended by sidetone\n", 0);
#endif
    }
    if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
        if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_REQUEST) {
            extern sidetone_scenario_t g_sidetone_request_scenario;
            if(amm_ptr==NULL){
                bt_sink_srv_am_amm_struct *am_amm;
                am_amm = (bt_sink_srv_am_amm_struct *)pvPortMalloc(sizeof(bt_sink_srv_am_amm_struct));
                if(am_amm == NULL){
                    AUDIO_ASSERT(0 && "[Sink][AM] AM_TASK_MASK_SIDE_TONE_REQUEST malloc fail.");
                }
                am_amm->background_info.local_feature.feature_param.sidetone_param.scenario = g_sidetone_request_scenario;
                audio_side_tone_enable_hdlr(am_amm);
                ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_REQUEST, false);
                vPortFree(am_amm);
            }else{
                amm_ptr->background_info.local_feature.feature_param.sidetone_param.scenario = g_sidetone_request_scenario;
                audio_side_tone_enable_hdlr(amm_ptr);
                ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_REQUEST, false);
            }
        }
    }
#endif
}

static void audio_ul_suspend_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
    audio_src_srv_report("[Sink][AM]audio_ul_suspend_hdlr\n", 0);
    aud_ul_suspend();
#endif
}

static void audio_ul_resume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x02) && (((audio_nvdm_HW_config.Voice_OutputDev & 0xF0) >> 4) == 0x03)) {
#else
    if (((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03)) ||
       ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select <= 0x60) && (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select >= 0x10) && (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03))) {
#endif
        audio_src_srv_report("[Sink][AM]audio_ul_resume_hdlr\n", 0);
        aud_ul_resume();
    } else {
        audio_src_srv_report("[Sink][AM]audio_ul_resume_hdlr. Not use I2S_Master, No need resume UL.\n", 0);
    }
#endif
}

/*****************************************************************************
 * FUNCTION
 *  audio_set_feature_hdlr
 * DESCRIPTION
 *  This function is an interface that can set parameters to DSP or set feature control through AM task
 * PARAMETERS
 *  amm_ptr          [IN]
 * RETURNS
 *  void
 *****************************************************************************/
static void audio_set_feature_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    bt_sink_srv_am_feature_t *ami_feature = &amm_ptr->background_info.local_feature;
    am_feature_type_t feature_type, feature_type_mask;
    uint8_t shift = 0;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    bt_sink_srv_am_type_t cur_type = NONE;
    cur_type = bt_sink_srv_ami_get_current_scenario();
#endif

    if (amm_ptr->background_info.aud_id == FEATURE_NO_NEED_ID) {
    } else {
        if (g_prCurrent_player == NULL) {
            audio_src_srv_report("[Sink][AM]audio_set_param_hdlr error, no current player\n", 0);
            return;
        } else if (amm_ptr->background_info.type != g_prCurrent_player->type) {
            audio_src_srv_report("[Sink][AM]audio_set_param_hdlr error, player changed (%d -> %d)\n", 2, amm_ptr->background_info.type, g_prCurrent_player->type);
            return;
        }
    }
    audio_src_srv_report("[Sink][AM]audio_set_param_hdlr with feature type 0x%x\n", 1, ami_feature->type_mask);

    feature_type_mask = ami_feature->type_mask;

    do {
        feature_type = feature_type_mask & (1 << shift++);
        feature_type_mask &= (~feature_type);
        if (feature_type == 0) {
            continue;
        }
        switch (feature_type) {
#ifdef MTK_PEQ_ENABLE
            case AM_A2DP_PEQ: {
#ifdef AIR_PEQ_WRITE_NVKEY_ENABLE
                if (ami_feature->feature_param.peq_param.sound_mode == (aud_peq_get_total_mode(PEQ_AUDIO_PATH_A2DP, 0) + 1) || ami_feature->feature_param.peq_param.sound_mode == PEQ_ON) {
                    ami_feature->feature_param.peq_param.sound_mode = g_peq_handle.a2dp_pre_peq_sound_mode;
                    audio_src_srv_report("[Sink][AM]PEQ ON,SOUND MODE: %d\n", 1, ami_feature->feature_param.peq_param.sound_mode);
                }
#endif
                if (aud_set_peq_param(PEQ_AUDIO_PATH_A2DP, &ami_feature->feature_param.peq_param) == 0) {
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.a2dp_pre_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.a2dp_pre_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        } else if (ami_feature->feature_param.peq_param.phase_id == 1) {
                            g_peq_handle.a2dp_post_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.a2dp_post_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        }
#ifdef AIR_PEQ_WRITE_NVKEY_ENABLE
                        aud_peq_save_misc_param();
#endif
                    }

                    audio_src_srv_report("[Sink][AM]Set A2DP PEQ param finish, phase_id:%d not_clear_sysram:%d\n", 2, ami_feature->feature_param.peq_param.phase_id, ami_feature->feature_param.peq_param.not_clear_sysram);
                }
                break;
            }
#endif
#ifdef MTK_LINEIN_PEQ_ENABLE
            case AM_LINEIN_PEQ: {
                if (aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &ami_feature->feature_param.peq_param) == 0) {
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.linein_pre_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.linein_pre_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        } else if (ami_feature->feature_param.peq_param.phase_id == 1) {
                            g_peq_handle.linein_post_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.linein_post_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        }
                    }
                    audio_src_srv_report("[Sink][AM]Set LINEIN PEQ param finish, phase_id:%d not_clear_sysram:%d\n", 2, ami_feature->feature_param.peq_param.phase_id, ami_feature->feature_param.peq_param.not_clear_sysram);
                }
                break;
            }
            case AM_USB_IN_PEQ: {
                if (aud_set_peq_param(PEQ_AUDIO_PATH_USB, &ami_feature->feature_param.peq_param) == 0) {
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.linein_pre_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.linein_pre_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        }
                    }
                    audio_src_srv_report("[Sink][AM]Set LINEIN PEQ param finish, phase_id:%d not_clear_sysram:%d\n", 2, ami_feature->feature_param.peq_param.phase_id, ami_feature->feature_param.peq_param.not_clear_sysram);
                }
                break;
            }
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
            case AM_AUDIO_AEQ: {
                if (ami_feature->feature_param.peq_param.not_clear_sysram == 0) {
                }
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.adaptive_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.adaptive_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                            aud_aeq_save_misc();
                        }
                    }
                    if(cur_type == A2DP || cur_type == BLE) {
                        if (aud_set_peq_param(PEQ_AUDIO_PATH_ADAPTIVE_EQ, &ami_feature->feature_param.peq_param) == 0){
                            audio_src_srv_report("[Sink][AM]Set ADAPITVE EQ param finish, phase_id:%d not_clear_sysram:%d\n",2,ami_feature->feature_param.peq_param.phase_id,ami_feature->feature_param.peq_param.not_clear_sysram);
                        }
                    }
                break;
            }
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE) && defined(MTK_PEQ_ENABLE)
            case AM_MIC_PEQ: {
                if (ami_feature->feature_param.peq_param.not_clear_sysram == 0) {
                    //audio_nvdm_reset_sysram();
                }
                if (aud_set_peq_param(PEQ_AUDIO_PATH_MIC, &ami_feature->feature_param.peq_param) == 0) {
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.mic_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.mic_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        }
                    }
                    audio_src_srv_report("[Sink][AM]Set MIC PEQ param finish, phase_id:%d not_clear_sysram:%d\n", 2, ami_feature->feature_param.peq_param.phase_id, ami_feature->feature_param.peq_param.not_clear_sysram);
                }
                break;
            }
#endif
#if defined (AIR_RECORD_ADVANCED_ENABLE) && defined(MTK_PEQ_ENABLE)
            case AM_ADVANCED_RECORD_PEQ: {
                if (ami_feature->feature_param.peq_param.not_clear_sysram == 0) {
                    //audio_nvdm_reset_sysram();
                }
                if (aud_set_peq_param(PEQ_AUDIO_PATH_ADVANCED_RECORD, &ami_feature->feature_param.peq_param) == 0) {
                    if (ami_feature->feature_param.peq_param.sound_mode != PEQ_SOUND_MODE_REAlTIME) {
                        if (ami_feature->feature_param.peq_param.phase_id == 0) {
                            g_peq_handle.advanced_record_peq_enable = ami_feature->feature_param.peq_param.enable;
                            if (ami_feature->feature_param.peq_param.enable) {
                                g_peq_handle.advanced_record_peq_sound_mode = ami_feature->feature_param.peq_param.sound_mode;
                            }
                        }
                    }
                    audio_src_srv_report("[Sink][AM]Set advanced_record PEQ param finish, phase_id:%d not_clear_sysram:%d\n", 2, ami_feature->feature_param.peq_param.phase_id, ami_feature->feature_param.peq_param.not_clear_sysram);
                }
                break;
            }
#endif
#if defined(MTK_AMP_DC_COMPENSATION_ENABLE)
            case DC_COMPENSATION: {
                hal_audio_set_dc_compensation();
                break;
            }
#endif
#ifdef AIR_KEEP_I2S_ENABLE
            case AM_AUDIO_DEVICE: {
                hal_audio_set_audio_device(ami_feature->feature_param.audio_driver_param.audio_device,ami_feature->feature_param.audio_driver_param.audio_interface,ami_feature->feature_param.audio_driver_param.enable);
                break;
            }
#endif
//#ifdef MTK_AWS_MCE_ENABLE
            case AM_HFP_AVC: {
                //audio_src_srv_report("[NDVC] ami_feature->feature_param.avc_vol: %d", 1, ami_feature->feature_param.avc_vol);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_AVC_PARA_SEND, 0, (uint32_t)ami_feature->feature_param.avc_vol, false);
                break;
            }
            case AM_VOICE_LEQ: {
                audio_src_srv_report("[AM][CPD] ami_feature->feature_param.cpd_param.leq_gain: %d", 1, ami_feature->feature_param.cpd_param.leq_gain);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_LEQ_PARA_SEND, 0, (uint16_t)ami_feature->feature_param.cpd_param.leq_gain, false);
                break;
            }
//#endif
#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
            case AM_ANC: {
                audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                audio_src_srv_report("[Sink][AM]audio_set_param_hdlr with event(%d)\n", 1, ami_feature->feature_param.anc_param.event);
                if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_ON) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_ANC_START);
#endif
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
                } else if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_OFF) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_ANC_STOP);
#endif
                } else if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_COPY_FILTER) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
                } else if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_SET_REG) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
#ifdef AIR_ANC_V3
                } else if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_FADP_REQUEST) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
                } else if (ami_feature->feature_param.anc_adapt_param.event == AUDIO_ANC_CONTROL_EVENT_FADP_SYNC) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_adapt_param.event, &ami_feature->feature_param.anc_adapt_param.cap);
#ifdef AIR_ANC_ADAPTIVE_STATUS_SYNC_ENABLE
                } else if (ami_feature->feature_param.anc_adapt_param.event == AUDIO_ANC_CONTROL_EVENT_FADP_STATUS_SYNC) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_adapt_param.event, &ami_feature->feature_param.anc_adapt_param.cap);
#endif
#endif
#endif
                } else if (ami_feature->feature_param.anc_param.event == AUDIO_ANC_CONTROL_EVENT_SIDETONE_REQUEST) {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
                }
                audio_src_srv_report("[Sink][AM]audio_set_param_hdlr with ret(%d) filter_id(%d) type(%d) filter_mask(0x%x) sram_bank(%d)\n", 5,
                                     anc_ret, ami_feature->feature_param.anc_param.cap.filter_cap.filter_id, ami_feature->feature_param.anc_param.cap.filter_cap.type,
                                     ami_feature->feature_param.anc_param.cap.filter_cap.filter_mask, ami_feature->feature_param.anc_param.cap.filter_cap.sram_bank);
                break;
            }
#else
            case AM_ANC: {
                if (ami_feature->feature_param.anc_param.event == ANC_CONTROL_EVENT_ON) {
                    anc_on(ami_feature->feature_param.anc_param.filter, ami_feature->feature_param.anc_param.user_runtime_gain, ami_feature->feature_param.anc_param.anc_control_callback, (uint8_t)ami_feature->feature_param.anc_param.param);
                } else if (ami_feature->feature_param.anc_param.event == ANC_CONTROL_EVENT_OFF) {
                    anc_off(ami_feature->feature_param.anc_param.anc_control_callback, 0, (uint8_t)ami_feature->feature_param.anc_param.param);
                } else if (ami_feature->feature_param.anc_param.event == ANC_CONTROL_EVENT_SET_VOLUME) {
                    anc_set_volume(&ami_feature->feature_param.anc_param.sw_gain);
                } else if (ami_feature->feature_param.anc_param.event == ANC_CONTROL_EVENT_SET_RUNTIME_VOLUME) {
                    anc_set_runtime_volume(ami_feature->feature_param.anc_param.user_runtime_gain);
                } else if (ami_feature->feature_param.anc_param.event == ANC_CONTROL_EVENT_SET_ATTACH_FUNC) {
                    anc_set_attach_enable((uint8_t)ami_feature->feature_param.anc_param.param);
                }
                break;
            }
#endif
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            case AM_AUDIO_COSYS: {
                if (ami_feature->feature_param.cosys_param.sub_type == AM_ANC) {
                    audio_set_anc_compensate(NONE, ami_feature->feature_param.cosys_param.cosys_event, NULL);
                }
                break;
            }
#endif
#ifdef AIR_PROMPT_SOUND_ENABLE
            case AM_VP: {
                if (ami_feature->feature_param.vp_param.event == PROMPT_CONTROL_MEDIA_PLAY) {
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_VP_START);
#endif
#if 0
                    prompt_control_play_tone(ami_feature->feature_param.vp_param.tone_type,
                                             ami_feature->feature_param.vp_param.tone_buf,
                                             ami_feature->feature_param.vp_param.tone_size,
                                             ami_feature->feature_param.vp_param.vp_end_callback);
#else
                    bool ret = false;
                    ret = prompt_control_play_tone_internal(ami_feature->feature_param.vp_param.tone_type,
                                                            ami_feature->feature_param.vp_param.tone_buf,
                                                            ami_feature->feature_param.vp_param.tone_size,
                                                            ami_feature->feature_param.vp_param.sync_time,
                                                            ami_feature->feature_param.vp_param.vp_end_callback);
                    _UNUSED(ret);
#endif
                } else if (ami_feature->feature_param.vp_param.event == PROMPT_CONTROL_MEDIA_STOP) {
                    prompt_control_stop_tone_internal();
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
                    ami_execute_vendor_se(EVENT_VP_STOP);
#endif
#ifdef AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
                } else if (ami_feature->feature_param.vp_param.event == PROMPT_CONTROL_DUMMY_SOURCE_START) {
                    audio_src_srv_report("[AM_VP][DummySource]index(%d) mode(%d) vol_lev(%d)", 3, ami_feature->feature_param.vp_param.dummy_source_param.index,
                                         ami_feature->feature_param.vp_param.dummy_source_param.mode, ami_feature->feature_param.vp_param.dummy_source_param.vol_lev);
                    prompt_control_dummy_source_start_internal(ami_feature->feature_param.vp_param.dummy_source_param);
                } else if (ami_feature->feature_param.vp_param.event == PROMPT_CONTROL_DUMMY_SOURCE_STOP) {
                    prompt_control_dummy_source_stop_internal();
#endif
                } else {
                    audio_src_srv_report("AM_VP Event error", 0);
                }
                break;
            }
#endif
            case AM_DYNAMIC_CHANGE_DSP_SETTING: {
                if (ami_feature->feature_param.channel == AUDIO_CHANNEL_SELECTION_STEREO) {
                    hal_gpio_status_t status;
                    hal_gpio_data_t channel_gpio_data = HAL_GPIO_DATA_LOW;
                    uint8_t channel_temp;
                    //Audio Channel selection setting
                    if (audio_Channel_Select.modeForAudioChannel) {
                        //HW_mode
                        status = hal_gpio_get_input((hal_gpio_pin_t)audio_Channel_Select.hwAudioChannel.gpioIndex, &channel_gpio_data);
                        if (status == HAL_GPIO_STATUS_OK) {
                            if (channel_gpio_data == HAL_GPIO_DATA_HIGH) {
                                channel_temp = (audio_Channel_Select.hwAudioChannel.audioChannelGPIOH & 0x0F);
                            } else {
                                channel_temp = (audio_Channel_Select.hwAudioChannel.audioChannelGPIOL & 0x0F);
                            }
                        } else {
                            channel_temp = AU_DSP_CH_LR; //default.
                            audio_src_srv_report("Get Stream in channel setting false with HW_mode.", 0);
                        }
                    } else {
                        //SW_mode
                        channel_temp = (audio_Channel_Select.audioChannel & 0x0F);
                    }
                    switch (channel_temp) {
                        case AU_DSP_CH_LR: {
                            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_STEREO, false);
                            break;
                        }
                        case AU_DSP_CH_L: {
                            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
                            break;
                        }
                        case AU_DSP_CH_R: {
                            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_R, false);
                            break;
                        }
                        case AU_DSP_CH_SWAP: {
                            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_SWAP, false);
                            break;
                        }
                        default: {
                            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_STEREO, false);
                            break;
                        }
                    }
                } else if (ami_feature->feature_param.channel == AUDIO_CHANNEL_SELECTION_MONO) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_MONO, false);
                } else if (ami_feature->feature_param.channel == AUDIO_CHANNEL_SELECTION_BOTH_L) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
                } else if (ami_feature->feature_param.channel == AUDIO_CHANNEL_SELECTION_BOTH_R) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_R, false);
                } else if (ami_feature->feature_param.channel == AUDIO_CHANNEL_SELECTION_SWAP) {
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 0, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_SWAP, false);
                }
                break;
            }
            #ifdef AIR_WIRELESS_MIC_TX_ENABLE
            case AM_DYNAMIC_CHANGE_DSP_SETTING_WIRELESS_MIC: {
                #ifdef AIR_AUDIO_ULD_CODEC_ENABLE
                audio_src_srv_report("wireless_mic ch_sel %d", 1, ami_feature->feature_param.channel);
                g_wireless_mic_channel_select = ami_feature->feature_param.channel;
                if(ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_BLE_UL) > 2){
                    configASSERT(0 && "[wireless_mic] HWIO config input channel > 2 ! error");
                }
                if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC_DEFAULE){
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_STEREO, false);
                } else if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC1){
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
                } else if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC2){
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_R, false);
                }
                break;
                #else
                    audio_src_srv_report("wireless_mic ch_sel %d, not support", 1, ami_feature->feature_param.channel);
                #endif
            }
            #endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
            case AM_ADAPTIVE_FF: {

                n9_dsp_share_info_t *p_share_info = hal_audio_query_record_share_info();
                hal_audio_reset_share_info(p_share_info);
                U8 *write_ptr = (U8 *)p_share_info->start_addr;
                U32 copy_length = sizeof(anc_fwd_iir_t);

                if ((ami_feature->feature_param.adaptive_ff_param.cmp_new_filter) && (ami_feature->feature_param.adaptive_ff_param.cmp_ori_filter)) {
                    memcpy(write_ptr, (U8 *)ami_feature->feature_param.adaptive_ff_param.cmp_ori_filter, copy_length);
                    memcpy(write_ptr + copy_length, (U8 *)ami_feature->feature_param.adaptive_ff_param.cmp_new_filter, copy_length);
                    p_share_info->write_offset += (2 * copy_length);
                    audio_src_srv_report("[Sink][AM][user_trigger_ff]cpy ori/new filter to share buff, write_ptr=0x%x, write_offset:%d, read_offset:%d, length:%d", 4, (U32)write_ptr, p_share_info->write_offset, p_share_info->read_offset, p_share_info->length);
                }
                audio_src_srv_report("[Sink][AM][user_trigger_ff]notify DSP to cmp filter\n", 0);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_USER_TRIGGER_FF_CMP_FILTER, 0, 0, true);
                break;
            }
#else
            case AM_ADAPTIVE_FF: {
                audio_src_srv_report("[Sink][AM][user_trigger_ff]Set param\n", 0);
                audio_user_trigger_adaptive_ff_set_parameters();
                break;
            }
#endif
#endif
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
            case AM_ANC_MONITOR: {
                switch (ami_feature->feature_param.anc_monitor_param.event) {
                    case AUDIO_ANC_MONITOR_SET_INFO: {
                        audio_src_srv_report("[Sink][AM][ANC_Monitor]Set info, type:%d, para:%d \n", 2, ami_feature->feature_param.anc_monitor_param.event_type, ami_feature->feature_param.anc_monitor_param.param);
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_ANC_ADAPTIVE_SET,
                                                              ami_feature->feature_param.anc_monitor_param.event_type,
                                                              ami_feature->feature_param.anc_monitor_param.param,
                                                              true);
                        break;
                    }
                    case AUDIO_ANC_MONITOR_GET_INFO: {
                        extern bool g_adaptive_ANC_getinfo_DSP_write_done;
                        audio_src_srv_report("[Sink][AM][ANC_Monitor]Get info, share_buffer_addr 0x%x\n", 1, ami_feature->feature_param.anc_monitor_param.param);

                        //ami_feature->feature_param.anc_monitor_param.param = ANC_MONITOR_INFO_BASE; //share buffer addr
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_ANC_ADAPTIVE_GET,
                                                              ami_feature->feature_param.anc_monitor_param.event_type,
                                                              ami_feature->feature_param.anc_monitor_param.param,
                                                              true);
                        g_adaptive_ANC_getinfo_DSP_write_done = true;
                        break;
                    }
                    case AUDIO_ANC_MONITOR_SET_PARAMS: {
                        audio_src_srv_report("[Sink][AM][ANC_Monitor]Set params, type:0x%x, para:0x%x \n", 2, ami_feature->feature_param.anc_monitor_param.event_type, ami_feature->feature_param.anc_monitor_param.param);
                        audio_anc_monitor_transmitter_send_ccni(ami_feature->feature_param.anc_monitor_param.event_type);
                        break;
                    }
                    default: {
                        audio_src_srv_report("[Sink][AM][ANC_Monitor]Wrong event\n", 0);
                        break;
                    }
                }
                break;
            }
#endif
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
            case AM_AUDIO_UPDATE_DAC_MODE: {
                audio_src_srv_report("[AM][DAC Change Mode] receive am event type:0x%x , param:0x%x", 2, ami_feature->feature_param.dac_mode_update_param.event_type,ami_feature->feature_param.dac_mode_update_param.param);
                hal_audio_status_set_ha_flag_and_update_dac_mode(ami_feature->feature_param.dac_mode_update_param.event_type, ami_feature->feature_param.dac_mode_update_param.param);
                break;
            }
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
            case AM_BLE: {
                void *p_param_share;

                audio_src_srv_report("[Sink][AM][ble] ble feature event %d", 1, ami_feature->feature_param.ble_param.event);
                if (ami_feature->feature_param.ble_param.event == BLE_FEATURE_EVENT_PLAY_INFO) {
                    p_param_share = hal_audio_dsp_controller_put_paramter(&(ami_feature->feature_param.ble_param.param.play_info), sizeof(ble_init_play_info_t), AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL);
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_INIT_PLAY_INFO, 0, (uint32_t)p_param_share, TRUE);
                }
                break;
            }
#endif
#if defined (MTK_AUDIO_LOOPBACK_TEST_ENABLE)
            case AM_AUDIO_LOOPBACK: {
                n9_dsp_share_info_t *share_info = hal_audio_query_bt_audio_dl_share_info();
                memset(share_info, 0, sizeof(uint32_t) * 4);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_LOOPBACK_TEST, 0, (uint32_t)share_info, false);
                break;
            }
#endif
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            case AM_AUDIO_BT_SET_PLAY_EN: {
                hal_audio_set_a2dp_play_en(ami_feature->feature_param.play_en_param.sequence_number, ami_feature->feature_param.play_en_param.bt_clock);
                break;
            }
#endif
            case AM_NR_PARAM:
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_SET_PARAM, ami_feature->feature_param.nr_param.type, ami_feature->feature_param.nr_param.value, false);
                break;
#ifdef AIR_DCHS_MODE_ENABLE
            case AM_UART_COSYS_CONTROL_DL: {
                dchs_cosys_ctrl_cmd_execute(&(ami_feature->feature_param.cosys_ctrl_dl));
                break;
            }
            case AM_UART_COSYS_CONTROL_UL: {
                dchs_cosys_ctrl_cmd_execute(&(ami_feature->feature_param.cosys_ctrl_ul));
                break;
            }
            case AM_DCHS: {
                dchs_cosys_ctrl_cmd_execute(&(ami_feature->feature_param.cosys_ctrl_ul));
                break;
            }
            case AM_UART_COSYS_CONTROL_ANC: {
                audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                anc_ret = audio_anc_dchs_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.anc_param.event, &ami_feature->feature_param.anc_param.cap);
                audio_src_srv_report("[DCHS ANC] audio_set_param_hdlr with event(%d), result %d\n", 2, ami_feature->feature_param.anc_param.event, anc_ret);
                break;
            }
#endif
#if defined (AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
            case AM_LLF: {
                llf_status_t llf_status;
                llf_status = llf_control_command_handler(AUDIO_LLF_CONTROL_SOURCE_FROM_AM, ami_feature->feature_param.llf_param.event, &(ami_feature->feature_param.llf_param.cap));
                audio_src_srv_report("[Sink][AM][LLF]audio_set_param_hdlr with ret(%d) event(0x%x) type(%d) sub_mode(%d)\n", 4,
                                     llf_status, ami_feature->feature_param.llf_param.event, ami_feature->feature_param.llf_param.cap.type, ami_feature->feature_param.llf_param.cap.sub_mode);
                break;
            }
#endif
            default:
                break;
        }
    } while (feature_type_mask != 0);
}

void bt_sink_srv_am_set_volume_change(bool enable)
{
    g_am_volume_enable = enable;
    audio_src_srv_report("[AudM]set_volume_change-enalbe: %d\n", 1, enable);
}

#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
extern uint8_t g_DL_SW_gain_remember_main_level;
extern uint8_t g_DL_SW_gain_remember_main_codec_type;
extern uint8_t g_DL_SW_gain_remember_main_level_2;
extern uint8_t g_DL_SW_gain_remember_main_level_3;

void am_set_volume_to_DL_SW_gain(void)
{
    set_volume_to_DL_SW_gain(&g_DL_SW_gain_lr_balance_para);
}
#endif

void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info)
{
#ifdef MTK_VENDOR_STREAM_OUT_VOLUME_TABLE_ENABLE

    uint32_t digital_gain, analog_gain;

    if (in_out == STREAM_OUT) {

        vol_type_t type = vol_info->type;
        uint32_t level = AUD_VOL_OUT_LEVEL10;
        if (type == VOL_ANC) {
            type = VOL_HFP;
        }
        if (g_prCurrent_player != NULL) {
            level = g_prCurrent_player->audio_stream_out.audio_volume;
        }

        bt_sink_srv_am_result_t ami_result = ami_get_stream_out_volume(type, level, &digital_gain, &analog_gain);

        if (ami_result != AUD_EXECUTION_SUCCESS) {
            // audio_src_srv_report("[Sink][AM]bt_sink_srv_am_set_volume: ami_get_stream_out_volume fail!", 0);
            // return;
        } else {
            // audio_src_srv_report("[Sink][AM]bt_sink_srv_am_set_volume stream out");
            hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_gain, analog_gain);
#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
        bt_sink_srv_audio_setting_set_audio_platform_output_volume(type, (int32_t)digital_gain);
#endif
            //return;
        }
		audio_src_srv_report("[Sink][AM]bt_sink_srv_am_set_volume   ami_result=%d", 1, ami_result);
		return;
    }

#endif

    uint32_t analog = 0;
    bt_sink_srv_audio_setting_vol_t *vol;
    vol = (bt_sink_srv_audio_setting_vol_t *)pvPortMalloc(sizeof(bt_sink_srv_audio_setting_vol_t));
    vol_type_t type;
    uint32_t digital = 0;

#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
    if(in_out == STREAM_OUT){
        if((vol_info->type == VOL_A2DP)&&
#ifdef AIR_BT_CODEC_BLE_ENABLED
            ((((g_prCurrent_player->type == A2DP)||(g_prCurrent_player->type == AWS)) && (g_DL_SW_gain_default_para->enable_a2dp))
            || ((g_prCurrent_player->type == ULL_BLE_DL) && (g_DL_SW_gain_default_para->enable_ull_music))
            || ((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_ONLY) && (g_DL_SW_gain_default_para->enable_ble_music)))
#else
            (((g_prCurrent_player->type == A2DP)||(g_prCurrent_player->type == AWS)) && (g_DL_SW_gain_default_para->enable_a2dp))
#endif
        ){
            g_DL_SW_gain_remember_main_level = vol_info->vol_info.a2dp_vol_info.lev;
            am_set_volume_to_DL_SW_gain();
            return;
        } else if((vol_info->type == VOL_HFP)&&
#ifdef AIR_BT_CODEC_BLE_ENABLED
            (((g_prCurrent_player->type == HFP) && (g_DL_SW_gain_default_para->enable_hfp))
            ||((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY) && (g_DL_SW_gain_default_para->enable_ble_call)))
#else
            ((g_prCurrent_player->type == HFP) && (g_DL_SW_gain_default_para->enable_hfp))
#endif
        ){
            g_DL_SW_gain_remember_main_codec_type =  vol_info->vol_info.hfp_vol_info.codec;
            g_DL_SW_gain_remember_main_level = vol_info->vol_info.hfp_vol_info.lev_out;
            am_set_volume_to_DL_SW_gain();
            return;
        } else if(vol_info->type == VOL_LINE_IN){
            audio_transmitter_scenario_list_t list_line_in[] = {{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT}};
            audio_transmitter_scenario_list_t list_usb_in[] = {{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
                                                                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1}};
             if((audio_transmitter_get_is_running_by_scenario_list(list_line_in, 1) && g_DL_SW_gain_default_para->enable_line_in)
                ||(audio_transmitter_get_is_running_by_scenario_list(list_usb_in, 1) && g_DL_SW_gain_default_para->enable_usb_in)
            ){
                g_DL_SW_gain_remember_main_level_3 = vol_info->vol_info.lineIN_vol_info.lev_out;
                am_set_volume_to_DL_SW_gain();
                return;
            }
        }
    }
#endif


#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    bt_sink_audio_setting_multi_vol_config_t *multi_vol_config = NULL;
    multi_vol_config = (bt_sink_audio_setting_multi_vol_config_t *)pvPortMalloc(sizeof(bt_sink_audio_setting_multi_vol_config_t));
    if (multi_vol_config == NULL) {
        vPortFree(vol);
        configASSERT(0 && "[Sink][Setting] multi_vol_config malloc error");
        return;
    }
#endif
#else
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
    uint32_t digital_Ref1 = 0;
    uint32_t digital_Ref2 = 0;
    uint32_t digital_Ref3 = 0;
    uint32_t digital_EchoREF = 0;
    uint32_t analog_R = 0;
#endif
#endif

    vol_type_t remember_type = vol_info->type;
    if (vol_info->type == VOL_LINE_IN_DL3) {
        remember_type = VOL_LINE_IN_DL3;
        vol_info->type = VOL_LINE_IN;
    } else if (vol_info->type == VOL_USB_AUDIO_IN_DL3) {
        remember_type = VOL_USB_AUDIO_IN_DL3;
        vol_info->type = VOL_USB_AUDIO_IN;
    }

    memset(vol, 0, sizeof(bt_sink_srv_audio_setting_vol_t));
    bt_sink_srv_audio_setting_get_vol(vol_info, vol);
    type = vol->type;
    switch (type) {
        case VOL_A2DP: {
            digital = vol->vol.a2dp_vol.vol.digital;
            analog = vol->vol.a2dp_vol.vol.analog_L;
            break;
        }

        case VOL_USB_AUDIO_IN: {
            digital = vol->vol.usb_audio_vol.vol.digital;
            analog = vol->vol.usb_audio_vol.vol.analog_L;
            break;
        }

        case VOL_ANC:
        case VOL_HFP: {
            if (STREAM_IN == in_out) {
                analog   = vol->vol.hfp_vol.vol_in.analog_L;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                /*when enable MTK_AUDIO_GAIN_SETTING_ENHANCE feature option*/
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                // multi_vol_config->digital_MIC0_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC0_L_Digital_Vol;
                // multi_vol_config->digital_MIC0_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC0_R_Digital_Vol;
                // multi_vol_config->digital_MIC1_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC1_L_Digital_Vol;
                // multi_vol_config->digital_MIC1_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC1_R_Digital_Vol;
                // multi_vol_config->digital_MIC2_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC2_L_Digital_Vol;
                // multi_vol_config->digital_MIC2_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC2_R_Digital_Vol;
                // multi_vol_config->digital_I2S0_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S0_L_Digital_Vol;
                // multi_vol_config->digital_I2S0_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S0_R_Digital_Vol;
                // multi_vol_config->digital_I2S1_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S1_L_Digital_Vol;
                // multi_vol_config->digital_I2S1_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S1_R_Digital_Vol;
                // multi_vol_config->digital_I2S2_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S2_L_Digital_Vol;
                // multi_vol_config->digital_I2S2_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_I2S2_R_Digital_Vol;
                // multi_vol_config->digital_LINEIN_L = vol.vol.hfp_vol.vol_multiMIC_in.digital_LINEIN_L_Digital_Vol;
                // multi_vol_config->digital_LINEIN_R = vol.vol.hfp_vol.vol_multiMIC_in.digital_LINEIN_R_Digital_Vol;
                // multi_vol_config->digital_Echo     = vol.vol.hfp_vol.vol_multiMIC_in.digital_Echo_Reference_Vol;
                // multi_vol_config->digital_MIC0_L_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC0_L_Func_Digital_Vol;
                // multi_vol_config->digital_MIC0_R_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC0_R_Func_Digital_Vol;
                // multi_vol_config->digital_MIC1_L_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC1_L_Func_Digital_Vol;
                // multi_vol_config->digital_MIC1_R_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC1_R_Func_Digital_Vol;
                // multi_vol_config->digital_MIC2_L_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC2_L_Func_Digital_Vol;
                // multi_vol_config->digital_MIC2_R_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_MIC2_R_Func_Digital_Vol;
                // multi_vol_config->digital_Echo_func = vol.vol.hfp_vol.vol_multiMIC_in.digital_Echo_Func_Reference_Vol;
                memcpy((uint8_t*)(multi_vol_config), (uint8_t*)&(vol->vol.hfp_vol.vol_multiMIC_in.digital_MIC0_L_Digital_Vol), 22*sizeof(uint32_t));
                // multi_vol_config->analog_R = vol.vol.hfp_vol.vol_in.analog_R;
                // multi_vol_config->analog_MIC2 = vol.vol.hfp_vol.vol_in.analog_MIC2;
                // multi_vol_config->analog_MIC3 = vol.vol.hfp_vol.vol_in.analog_MIC3;
                // multi_vol_config->analog_MIC4 = vol.vol.hfp_vol.vol_in.analog_MIC4;
                // multi_vol_config->analog_MIC5 = vol.vol.hfp_vol.vol_in.analog_MIC5;
                memcpy((uint8_t*)(multi_vol_config)+22*sizeof(uint32_t), (uint8_t*)&(vol->vol.hfp_vol.vol_in.analog_R), 5*sizeof(uint32_t));

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                bt_sink_srv_audio_setting_detach_mic_gain_config(&analog, &digital, multi_vol_config);
#endif
#else
                digital = vol->vol.hfp_vol.vol_in.digital;
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                bt_sink_srv_audio_setting_detach_mic_gain_config(&analog, &digital, NULL);
#endif
#endif
#else
                digital = vol->vol.hfp_vol.vol_in.digital;
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
                digital_Ref1    = vol->vol.hfp_vol.vol_multiMIC_in.digital_Ref1;
                digital_Ref2    = vol->vol.hfp_vol.vol_multiMIC_in.digital_Ref2;
                digital_Ref3    = vol->vol.hfp_vol.vol_multiMIC_in.digital_RESERVE;
                digital_EchoREF = vol->vol.hfp_vol.vol_multiMIC_in.digital_Echo;
                analog_R = vol->vol.hfp_vol.vol_in.analog_R;
#endif
#endif
            } else if (STREAM_OUT == in_out) {
                analog = vol->vol.hfp_vol.vol_out.analog_L;
                digital = vol->vol.hfp_vol.vol_out.digital;
            }
            break;
        }

        case VOL_PCM: {
            digital = vol->vol.pcm_vol.vol.digital;
            analog = vol->vol.pcm_vol.vol.analog_L;
            break;
        }

        case VOL_MP3: {
            digital = vol->vol.mp3_vol.vol.digital;
            analog = vol->vol.mp3_vol.vol.analog_L;
            break;
        }

        case VOL_DEF: {
            digital = vol->vol.def_vol.vol.digital;
            analog = vol->vol.def_vol.vol.analog_L;
            break;
        }

        case VOL_VC: {
            analog  = vol->vol.vc_vol.vol.analog_L;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
            /*when enable MTK_AUDIO_GAIN_SETTING_ENHANCE feature option*/
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
            // multi_vol_config->digital_MIC0_L = vol.vol.vc_vol.vol_multiMIC.digital_MIC0_L_Digital_Vol;
            // multi_vol_config->digital_MIC0_R = vol.vol.vc_vol.vol_multiMIC.digital_MIC0_R_Digital_Vol;
            // multi_vol_config->digital_MIC1_L = vol.vol.vc_vol.vol_multiMIC.digital_MIC1_L_Digital_Vol;
            // multi_vol_config->digital_MIC1_R = vol.vol.vc_vol.vol_multiMIC.digital_MIC1_R_Digital_Vol;
            // multi_vol_config->digital_MIC2_L = vol.vol.vc_vol.vol_multiMIC.digital_MIC2_L_Digital_Vol;
            // multi_vol_config->digital_MIC2_R = vol.vol.vc_vol.vol_multiMIC.digital_MIC2_R_Digital_Vol;
            // multi_vol_config->digital_I2S0_L = vol.vol.vc_vol.vol_multiMIC.digital_I2S0_L_Digital_Vol;
            // multi_vol_config->digital_I2S0_R = vol.vol.vc_vol.vol_multiMIC.digital_I2S0_R_Digital_Vol;
            // multi_vol_config->digital_I2S1_L = vol.vol.vc_vol.vol_multiMIC.digital_I2S1_L_Digital_Vol;
            // multi_vol_config->digital_I2S1_R = vol.vol.vc_vol.vol_multiMIC.digital_I2S1_R_Digital_Vol;
            // multi_vol_config->digital_I2S2_L = vol.vol.vc_vol.vol_multiMIC.digital_I2S2_L_Digital_Vol;
            // multi_vol_config->digital_I2S2_R = vol.vol.vc_vol.vol_multiMIC.digital_I2S2_R_Digital_Vol;
            // multi_vol_config->digital_LINEIN_L = vol.vol.vc_vol.vol_multiMIC.digital_LINEIN_L_Digital_Vol;
            // multi_vol_config->digital_LINEIN_R = vol.vol.vc_vol.vol_multiMIC.digital_LINEIN_R_Digital_Vol;
            // multi_vol_config->digital_Echo     = vol.vol.vc_vol.vol_multiMIC.digital_Echo_Reference_Vol;
            // multi_vol_config->digital_MIC0_L_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC0_L_Func_Digital_Vol;
            // multi_vol_config->digital_MIC0_R_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC0_R_Func_Digital_Vol;
            // multi_vol_config->digital_MIC1_L_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC1_L_Func_Digital_Vol;
            // multi_vol_config->digital_MIC1_R_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC1_R_Func_Digital_Vol;
            // multi_vol_config->digital_MIC2_L_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC2_L_Func_Digital_Vol;
            // multi_vol_config->digital_MIC2_R_func = vol.vol.vc_vol.vol_multiMIC.digital_MIC2_R_Func_Digital_Vol;
            // multi_vol_config->digital_Echo_func = vol.vol.vc_vol.vol_multiMIC.digital_Echo_Func_Reference_Vol;
            memcpy((uint8_t*)(multi_vol_config), (uint8_t*)&(vol->vol.vc_vol.vol_multiMIC.digital_MIC0_L_Digital_Vol), 22*sizeof(uint32_t));
            // multi_vol_config->analog_R = vol.vol.vc_vol.vol.analog_R;
            // multi_vol_config->analog_MIC2 = vol.vol.vc_vol.vol.analog_MIC2;
            // multi_vol_config->analog_MIC3 = vol.vol.vc_vol.vol.analog_MIC3;
            // multi_vol_config->analog_MIC4 = vol.vol.vc_vol.vol.analog_MIC4;
            // multi_vol_config->analog_MIC5 = vol.vol.vc_vol.vol.analog_MIC5;
            memcpy((uint8_t*)(multi_vol_config)+22*sizeof(uint32_t), (uint8_t*)&(vol->vol.vc_vol.vol.analog_R), 5*sizeof(uint32_t));
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            bt_sink_srv_audio_setting_detach_mic_gain_config(&analog, &digital, multi_vol_config);
#endif
#else
            digital = vol->vol.vc_vol.vol.digital;

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            bt_sink_srv_audio_setting_detach_mic_gain_config(&analog, &digital, NULL);
#endif
#endif
#else
            digital = vol->vol.vc_vol.vol.digital;
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital_Ref1 = vol->vol.vc_vol.vol_multiMIC.digital_Ref1;
            analog_R     = vol->vol.vc_vol.vol.analog_R;
#endif
#endif
            break;
        }

        case VOL_LINE_IN: {
#if defined  (MTK_LINE_IN_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE) ||  defined (AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE)
            if (STREAM_IN == in_out) {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                analog   = vol->vol.lineIN_vol.vol_in.analog_L;
                multi_vol_config->analog_R = vol->vol.lineIN_vol.vol_in.analog_R;
#else
                analog = vol->vol.lineIN_vol.vol_in.analog_L;
#endif
                digital = vol->vol.lineIN_vol.vol_in.digital;
#else
                digital = vol->vol.lineIN_vol.vol_in.digital;
                analog = vol->vol.lineIN_vol.vol_in.analog_L;
#endif
            } else if (STREAM_OUT == in_out) {
                digital = vol->vol.lineIN_vol.vol_out.digital;
                analog = vol->vol.lineIN_vol.vol_out.analog_L;
            }
#endif
            break;
        }

        default:
            break;
    }

    if (g_am_volume_enable) {
        ;
    } else {
        if (STREAM_IN == in_out) {
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            hal_audio_set_stream_in_volume(digital, analog);
            audio_src_srv_report("[Sink][Setting]AM set vol-- analog gain = %d,digital_MIC0_L gain = %d", 2, analog, digital);
#else
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            if (((type == VOL_HFP) && (((audio_nvdm_HW_config.Voice_InputDev & 0xC0) >> 6) == 0x03)) || (type == VOL_VC) || (type == VOL_ANC)) {
#else
            if (((type == VOL_HFP) && ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) || (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x03) || (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x02))) || (type == VOL_VC) || (type == VOL_ANC)) {
#endif
                /*digital gain setting to DSP side*/
                if (type != VOL_ANC) {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                    /*multi mic gain*/
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC0_L, multi_vol_config->digital_MIC0_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D0_D1);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC1_L, multi_vol_config->digital_MIC1_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D2_D3);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC2_L, multi_vol_config->digital_MIC2_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D4_D5);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_I2S0_L, multi_vol_config->digital_I2S0_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D6_D7);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_I2S1_L, multi_vol_config->digital_I2S1_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D8_D9);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_I2S2_L, multi_vol_config->digital_I2S2_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D10_D11);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_LINEIN_L, multi_vol_config->digital_LINEIN_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D12_D13);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_Echo, 0x7FFF, HAL_AUDIO_INPUT_GAIN_SELECTION_D14);
                    /*mic functions gain*/
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC0_L_func, multi_vol_config->digital_MIC0_R_func, HAL_AUDIO_INPUT_GAIN_SELECTION_D18_19);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC1_L_func, multi_vol_config->digital_MIC1_R_func, HAL_AUDIO_INPUT_GAIN_SELECTION_D20_21);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC2_L_func, multi_vol_config->digital_MIC2_R_func, HAL_AUDIO_INPUT_GAIN_SELECTION_D22_23);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_Echo_func, 0x7FFF, HAL_AUDIO_INPUT_GAIN_SELECTION_D24_25);
                    /*debug log*/
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC0_L gain = %d,digital_MIC0_R gain = %d", 2, multi_vol_config->digital_MIC0_L, multi_vol_config->digital_MIC0_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC1_L gain = %d,digital_MIC1_R gain = %d", 2, multi_vol_config->digital_MIC1_L, multi_vol_config->digital_MIC1_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC2_L gain = %d,digital_MIC2_R gain = %d", 2, multi_vol_config->digital_MIC2_L, multi_vol_config->digital_MIC2_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_I2S0_L gain = %d,digital_I2S0_R gain = %d", 2, multi_vol_config->digital_I2S0_L, multi_vol_config->digital_I2S0_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_I2S1_L gain = %d,digital_I2S1_R gain = %d", 2, multi_vol_config->digital_I2S1_L, multi_vol_config->digital_I2S1_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_I2S2_L gain = %d,digital_I2S2_R gain = %d", 2, multi_vol_config->digital_I2S2_L, multi_vol_config->digital_I2S2_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_LINEIN_L gain = %d,digital_LINEIN_R gain = %d", 2, multi_vol_config->digital_LINEIN_L, multi_vol_config->digital_LINEIN_R);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_Echo gain = %d", 1, multi_vol_config->digital_Echo);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC0_L_func gain = %d,digital_MIC0_R_func gain = %d", 2, multi_vol_config->digital_MIC0_L_func, multi_vol_config->digital_MIC0_R_func);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC1_L_func gain = %d,digital_MIC1_R_func gain = %d", 2, multi_vol_config->digital_MIC1_L_func, multi_vol_config->digital_MIC1_R_func);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_MIC2_L_func gain = %d,digital_MIC2_R_func gain = %d", 2, multi_vol_config->digital_MIC2_L_func, multi_vol_config->digital_MIC2_R_func);
                    // audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic digital_Echo_func gain = %d", 1, multi_vol_config->digital_Echo_func);
                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP multi mic gain: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",22,
                    multi_vol_config->digital_MIC0_L, multi_vol_config->digital_MIC0_R, multi_vol_config->digital_MIC1_L, multi_vol_config->digital_MIC1_R,
                    multi_vol_config->digital_MIC2_L, multi_vol_config->digital_MIC2_R, multi_vol_config->digital_I2S0_L, multi_vol_config->digital_I2S0_R,
                    multi_vol_config->digital_I2S1_L, multi_vol_config->digital_I2S1_R, multi_vol_config->digital_I2S2_L, multi_vol_config->digital_I2S2_R,
                    multi_vol_config->digital_LINEIN_L, multi_vol_config->digital_LINEIN_R,  multi_vol_config->digital_Echo,
                    multi_vol_config->digital_MIC0_L_func, multi_vol_config->digital_MIC0_R_func, multi_vol_config->digital_MIC1_L_func, multi_vol_config->digital_MIC1_R_func,
                    multi_vol_config->digital_MIC2_L_func, multi_vol_config->digital_MIC2_R_func,multi_vol_config->digital_Echo_func);
#else
                    hal_audio_set_stream_in_volume_for_multiple_microphone(digital, digital_Ref1, HAL_AUDIO_INPUT_GAIN_SELECTION_D0_D1);
                    if (type == VOL_HFP) {
                        hal_audio_set_stream_in_volume_for_multiple_microphone(digital_Ref2, digital_Ref3, HAL_AUDIO_INPUT_GAIN_SELECTION_D2_D3);
                        hal_audio_set_stream_in_volume_for_multiple_microphone(digital_EchoREF, digital_EchoREF, HAL_AUDIO_INPUT_GAIN_SELECTION_D14);
                    }
#endif
                }

                /*analog gain setting to DSP side*/
                if (type != VOL_ANC) {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                    hal_audio_set_stream_in_volume_for_multiple_microphone(analog, multi_vol_config->analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1);
                    //audio_src_srv_report("[Sink][Setting]AM set vol-- analog gain = %d,analog_R gain = %d", 2, analog, multi_vol_config->analog_R);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->analog_MIC2, multi_vol_config->analog_MIC3, HAL_AUDIO_INPUT_GAIN_SELECTION_A2_A3);
                    hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->analog_MIC4, multi_vol_config->analog_MIC5, HAL_AUDIO_INPUT_GAIN_SELECTION_A4_A5);
                    audio_src_srv_report("[Sink][Setting]AM set vol-- analog gain = %d,analog_R gain = %d,analog_MIC2 gain = %d,analog_MIC3 gain = %d,analog_MIC4 gain = %d,analog_MIC5 gain = %d", 6, analog, multi_vol_config->analog_R, multi_vol_config->analog_MIC2, multi_vol_config->analog_MIC3, multi_vol_config->analog_MIC4, multi_vol_config->analog_MIC5);
#else
                    hal_audio_set_stream_in_volume_for_multiple_microphone(analog, analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1);
#endif
                } else {
                    mcu2dsp_open_stream_in_param_t  *in_device = (mcu2dsp_open_stream_in_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
                    memset(in_device, 0, sizeof(mcu2dsp_open_stream_in_param_t));
                    hal_audio_get_stream_in_setting_config(AU_DSP_ANC, in_device);
                    mcu2dsp_open_stream_in_param_t  *voice_in_device = (mcu2dsp_open_stream_in_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
                    memset(voice_in_device, 0, sizeof(mcu2dsp_open_stream_in_param_t));
                    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, voice_in_device);
                    uint32_t interface_mask = 0;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                    interface_mask = in_device->afe.audio_interface | in_device->afe.audio_interface1 | in_device->afe.audio_interface2 | in_device->afe.audio_interface3 | voice_in_device->afe.audio_interface;
#else
                    interface_mask = in_device->afe.audio_interface | voice_in_device->afe.audio_interface;
#endif
                    if (interface_mask & HAL_AUDIO_INTERFACE_1) {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                        hal_audio_set_stream_in_volume_for_multiple_microphone(analog, multi_vol_config->analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1);
                        audio_src_srv_report("[Sink][Setting]AM set vol-- analog gain = %d,analog_R gain = %d", 2, analog, multi_vol_config->analog_R);
#else
                        hal_audio_set_stream_in_volume_for_multiple_microphone(analog, analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1);
                        audio_src_srv_report("[Sink][Setting]AM set vol-- analog gain = %d,analog_R gain = %d", 2, analog, analog_R);
#endif
                    }
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                    if (interface_mask & HAL_AUDIO_INTERFACE_2) {
                        hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->analog_MIC2, multi_vol_config->analog_MIC3, HAL_AUDIO_INPUT_GAIN_SELECTION_A2_A3);
                    }
                    if (interface_mask & HAL_AUDIO_INTERFACE_3) {
                        hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->analog_MIC4, multi_vol_config->analog_MIC5, HAL_AUDIO_INPUT_GAIN_SELECTION_A4_A5);
                    }
                    audio_src_srv_report("[Sink][Setting]AM set vol-- analog_MIC2 gain = %d,analog_MIC3 gain = %d,analog_MIC4 gain = %d,analog_MIC5 gain = %d", 4, multi_vol_config->analog_MIC2, multi_vol_config->analog_MIC3, multi_vol_config->analog_MIC4, multi_vol_config->analog_MIC5);
#endif
                    vPortFree(in_device);
                    vPortFree(voice_in_device);

                }
            } else {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                if (type == VOL_LINE_IN) {
                    /*Line in L R analog gain*/
                    #ifndef AIR_BTA_IC_PREMIUM_G3
                    hal_audio_set_stream_in_volume_for_multiple_microphone(analog, multi_vol_config->analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1);
                    #else//AB158X use ADC23 for line in
                    hal_audio_set_stream_in_volume_for_multiple_microphone(analog, multi_vol_config->analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A2_A3);
                    #endif
                    hal_audio_set_stream_in_volume_for_multiple_microphone(digital, 0x7FFF, HAL_AUDIO_INPUT_GAIN_SELECTION_D0_D1);
                    audio_src_srv_report("[Sink][Setting]AM set vol-- Line in analog gain = %d,analog_R gain = %d", 3, analog, multi_vol_config->analog_R, digital);
                } else if (type == VOL_HFP) {
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                    audio_src_srv_report("[Sink][Setting]hal_audio_query_voice_mic_type() = %d", 1, hal_audio_query_voice_mic_type());
                    if (hal_audio_query_voice_mic_type() == VOICE_MIC_TYPE_DETACHABLE) {
                        if (ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_BLE_UL)==1)
                        {
                        hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_L, analog);
                        audio_src_srv_report("[Sink][Setting]AM set vol-- detachable MIC analog gain = %d,digital_MIC0_L gain = %d", 2, analog, multi_vol_config->digital_MIC0_L);
                        }
                        else if (ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_BLE_UL)==2)
                        {
                            hal_audio_set_stream_in_volume_for_multiple_microphone(analog, multi_vol_config->analog_R, HAL_AUDIO_INPUT_GAIN_SELECTION_A2_A3);
                            hal_audio_set_stream_in_volume_for_multiple_microphone(multi_vol_config->digital_MIC1_L, multi_vol_config->digital_MIC1_R, HAL_AUDIO_INPUT_GAIN_SELECTION_D2_D3);
                            audio_src_srv_report("[Sink][Setting]AM set vol-- detachable MIC stereo analog gain = %d %d,digital_MIC1 gain = %d %d %d %d %d %d", 8,
                            analog, multi_vol_config->analog_R,
                            multi_vol_config->digital_MIC0_L, multi_vol_config->digital_MIC0_R,
                            multi_vol_config->digital_MIC1_L, multi_vol_config->digital_MIC1_R,
                            multi_vol_config->digital_MIC2_L, multi_vol_config->digital_MIC2_R);
                        }
                        else {
                            audio_src_srv_err("[Sink][Setting]AM set vol-- detachable MIC not support more than 2 channel", 0);
                            AUDIO_ASSERT(0);
                        }
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                        vPortFree(multi_vol_config);
#endif
#endif
                        vPortFree(vol);
                        return;
                    }
#endif
                    /*HFP with 1 mic HWIO config*/
                    switch (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path) {
                        case 0x00: //0x0: Analog Mic
                            switch (audio_nvdm_HW_config.voice_scenario.Voice_Analog_MIC_Sel) {
                                case 0x00: //mic 0
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_L, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic0 analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC0_L);
                                    break;
                                case 0x01: //mic 1
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_R, multi_vol_config->analog_R);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic1 analog gain = %d,digital gain = %d", 2, multi_vol_config->analog_R, multi_vol_config->digital_MIC0_R);
                                    break;
                                case 0x02: //mic 2
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC1_L, multi_vol_config->analog_MIC2);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic2 analog gain = %d,digital gain = %d", 2, multi_vol_config->analog_MIC2, multi_vol_config->digital_MIC1_L);
                                    break;
                                case 0x03: //mic 3
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC1_R, multi_vol_config->analog_MIC3);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic3 analog gain = %d,digital gain = %d", 2, multi_vol_config->analog_MIC3, multi_vol_config->digital_MIC1_R);
                                    break;
                                case 0x04: //mic 4
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC2_L, multi_vol_config->analog_MIC4);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic4 analog gain = %d,digital gain = %d", 2, multi_vol_config->analog_MIC4, multi_vol_config->digital_MIC2_L);
                                    break;
                                case 0x05: //mic 5
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC2_R, multi_vol_config->analog_MIC5);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic5 analog gain = %d,digital gain = %d", 2, multi_vol_config->analog_MIC5, multi_vol_config->digital_MIC2_R);
                                    break;
                            }
                            break;
                        case 0x01: //0x1: Digital Mic
                            switch (audio_nvdm_HW_config.voice_scenario.Voice_Digital_MIC_Sel) {
                                case 0x08: //mic 0
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_L, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic8 analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC0_L);
                                    break;
                                case 0x09: //mic 1
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_R, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one mic9 analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC0_R);
                                    break;
                                case 0x0A: //mic 2
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC1_L, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one micA analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC1_L);
                                    break;
                                case 0x0B: //mic 3
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC1_R, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one micB analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC1_R);
                                    break;
                                case 0x0C: //mic 4
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC2_L, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one micC analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC2_L);
                                    break;
                                case 0x0D: //mic 5
                                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC2_R, analog);
                                    audio_src_srv_report("[Sink][Setting]AM set vol-- HFP one micD analog gain = %d,digital gain = %d", 2, analog, multi_vol_config->digital_MIC2_R);
                                    break;
                            }
                            break;
                    }
                } else {
                    hal_audio_set_stream_in_volume(multi_vol_config->digital_MIC0_L, analog);
                    audio_src_srv_report("[Sink][Setting]AM set vol-- other analog gain = %d,digital_MIC0_L gain = %d", 2, analog, multi_vol_config->digital_MIC0_L);
                }
#else
                hal_audio_set_stream_in_volume(digital, analog);
#endif
            }
#endif
        } else if (STREAM_OUT == in_out) {
            if ((remember_type == VOL_LINE_IN_DL3) || (remember_type == VOL_USB_AUDIO_IN_DL3)) {
#ifdef AIR_WIRED_AUDIO_ENABLE
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT3, digital, analog);
#endif
            } else {
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital, analog);
            }
#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
            bt_sink_srv_audio_setting_set_audio_platform_output_volume(type, (int32_t)digital);
#endif

        }
    }

    vol_info->type = remember_type;

#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    vPortFree(multi_vol_config);
#endif
#endif
    vPortFree(vol);
    audio_src_srv_report("[Sink][Setting]AM set vol--in_out: %d, d: 0x%x, a: 0x%x, type1: %d, type2: %d", 5,
                         in_out, digital, analog, vol_info->type, type);
}

#endif

/**
 * @brief      Init HW I/O config by reading NV key into static table.
 * @return     #HAL_AUDIO_STATUS_OK, if OK. #HAL_AUDIO_STATUS_ERROR, if wrong.
 * @note      This function has SW_Mode and HW_Mode(//TODO).
 */
#if defined(MTK_AVM_DIRECT)
audio_version_t audio_nvdm_check_version(uint32_t HWIO_Configure_Size)
{
    audio_version_t local_version;
    switch (HWIO_Configure_Size) {
        case 0x1D:
            local_version = SDK_V1p4;
            break;
        case 0x09:
        default:
            local_version = SDK_V1p3;
            break;
    }
    return local_version;
}

#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
bt_sink_srv_am_result_t audio_nvdm_configure_init_mapping(audio_version_t FW_version, audio_version_t key_version)
{
    if (FW_version == key_version) {
        //No need to mapping
        audio_src_srv_report("[AM]audio_nvdm_configure_init_mapping no need to mapping.version(%d)\n", 1, FW_version);
        return AUD_EXECUTION_SUCCESS;
    }
    if (FW_version == SDK_V1p4) {
        switch (key_version) {
            case SDK_V1p3: {
                audio_src_srv_report("[AM]audio_nvdm_configure_init_mapping version(%d)(%d)\n", 2, FW_version, key_version);
                mcu2dsp_open_param_t eSCO_open_param;
                memset(&eSCO_open_param, 0, sizeof(eSCO_open_param));
                hal_audio_get_stream_out_setting_config(AU_DSP_VOICE, &eSCO_open_param.stream_out_param);
                hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &eSCO_open_param.stream_in_param);
                if (eSCO_open_param.stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_DAC_L) {
                    audio_nvdm_HW_config.ANC_OutputDev = 0x00;
                } else if (eSCO_open_param.stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_DAC_R) {
                    audio_nvdm_HW_config.ANC_OutputDev = 0x10;
                } else if (eSCO_open_param.stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_DAC_DUAL) {
                    audio_nvdm_HW_config.ANC_OutputDev = 0x20;
                } else {
                    audio_nvdm_HW_config.ANC_OutputDev = 0x00;
                }
                if ((eSCO_open_param.stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_MAIN_MIC_DUAL) != 0) {
                    audio_nvdm_HW_config.MIC_Select_ANC_FF = 0x00;
                    audio_nvdm_HW_config.MIC_Select_ANC_FB = 0x01;
                    if (eSCO_open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_MAIN_MIC_L) {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x00;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref  = 0xff;
                    } else if (eSCO_open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_MAIN_MIC_R) {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x01;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref  = 0xff;
                    } else {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x00;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref = 0x01;
                    }
                } else if ((eSCO_open_param.stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL) != 0) {
                    audio_nvdm_HW_config.MIC_Select_ANC_FF = 0x02;
                    audio_nvdm_HW_config.MIC_Select_ANC_FB = 0x03;
                    if (eSCO_open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_DIGITAL_MIC_L) {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x02;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref  = 0xff;
                    } else if (eSCO_open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_DIGITAL_MIC_R) {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x03;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref  = 0xff;
                    } else {
                        audio_nvdm_HW_config.MIC_Select_Record_Main = 0x02;
                        audio_nvdm_HW_config.MIC_Select_Record_Ref  = 0x03;
                    }
                }
            }
            default:
                break;
        }
    }
    return AUD_EXECUTION_SUCCESS;
}
#endif

bt_sink_srv_am_result_t audio_nvdm_configure_init(void)
{
    bt_sink_srv_am_result_t status = AUD_EXECUTION_SUCCESS;
    nvdm_status_t nvdm_status = NVDM_STATUS_OK;
    uint32_t tableSize = 0;

    if (g_audio_nvdm_init_flg == true) {
        return AUD_EXECUTION_SUCCESS;
    }

    //Audio HW Configure..  /*HFP sidetone, gain and AMIC DCC, ACC*/
    nvdm_status = nvkey_data_item_length(NVID_DSP_FW_AUDIO_HW_IO_CFG, &tableSize);
    if (nvdm_status || !tableSize) {
        audio_src_srv_report("[AM][Warning]NVDM HWIO_config_init Fail. Status:%d Len:%u\n", 2, nvdm_status, tableSize);
        status = AUD_EXECUTION_FAIL;
    }
    nvdm_version = audio_nvdm_check_version(tableSize);
    if (nvdm_version == SDK_NONE) {
        AUDIO_ASSERT(0 && "NVDM HWIO_config_init SDK Version not match.");
    }
    if (tableSize != sizeof(audio_nvdm_HW_config)) {
        audio_src_srv_report("[AM][Warning]NVDM HWIO_config_init size not match. NVDM tableSize (%u) != (%d)\n", 2, tableSize, sizeof(audio_nvdm_HW_config));
        status = AUD_EXECUTION_FAIL;
    }
    nvdm_status = nvkey_read_data(NVID_DSP_FW_AUDIO_HW_IO_CFG, (uint8_t *)&audio_nvdm_HW_config, &tableSize);
    if (nvdm_status) {
        audio_src_srv_report("[AM][Warning]NVDM HWIO_config_init Fail 2. Status:%d pNvdmMp3Vol:0x%x\n", 2, nvdm_status, (unsigned int)&audio_nvdm_HW_config);
        status = AUD_EXECUTION_FAIL;
    }
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
        default_dac_mode = audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel;
#endif
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    audio_nvdm_configure_init_mapping(SW_version, nvdm_version);
#endif

    //Audio Channel Selection from NV key.
    nvdm_status = nvkey_data_item_length(NVID_DSP_FW_AUDIO_CH_CFG, &tableSize);
    if (nvdm_status || !tableSize) {
        audio_src_srv_report("[AM][Warning]NVDM Channel_selection_init Fail. Status:%u Len:%lu\n", 2, nvdm_status, tableSize);
        status = AUD_EXECUTION_FAIL;
    }
    if (tableSize != sizeof(audio_Channel_Select)) {
        audio_src_srv_report("[AM][Warning]NVDM Channel_selection_init size not match. NVDM tableSize (%lu) != (%d)\n", 2, tableSize, sizeof(audio_Channel_Select));
        status = AUD_EXECUTION_FAIL;
    }
    nvdm_status = nvkey_read_data(NVID_DSP_FW_AUDIO_CH_CFG, (uint8_t *)&audio_Channel_Select, &tableSize);
    if (nvdm_status) {
        audio_src_srv_report("[AM][Warning]NVDM Channel_selection_init Fail 2. Status:%d pNvdmMp3Vol:0x%x\n", 2, nvdm_status, (unsigned int)&audio_Channel_Select);
        status = AUD_EXECUTION_FAIL;
    }
#ifndef FPGA_ENV
    //AMIC ACC DCC Setting
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((audio_nvdm_HW_config.UL_AFE_PARA & 0x02) != 0) {
#else
#if defined(AIR_BTA_IC_PREMIUM_G2)
    if ((audio_nvdm_HW_config.adc_dac_config.amic_config[0].ADDA_Analog_MIC_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.amic_config[1].ADDA_Analog_MIC_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.amic_config[2].ADDA_Analog_MIC_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.amic_config[3].ADDA_Analog_MIC_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.amic_config[4].ADDA_Analog_MIC_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.amic_config[5].ADDA_Analog_MIC_Mode != 0x02)) {

#else
    if ((audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC1_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC2_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC3_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC4_Mode != 0x02) ||
        (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC5_Mode != 0x02)) {
#endif
#endif
        //AMIC ACC mode
        *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
        *((volatile uint32_t *)(0xA2120B04)) |= 0x10;
        audio_src_srv_report("AMIC change to ACC mode", 0);
    }
    //ADC, DAC Configure (NM HP mode)
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((audio_nvdm_HW_config.UL_AFE_PARA & 0x01) != 0) {
#else
    if ((audio_nvdm_HW_config.audio_scenario.Audio_Analog_LineIn_Performance_Sel == 0x01) ||
        (audio_nvdm_HW_config.voice_scenario.Voice_Analog_ADC_Performance_Sel == 0x01) ||
        (audio_nvdm_HW_config.record_scenario.Record_Analog_ADC_Performance_Sel == 0x01)) {
#endif
        //ADC HP
        *((volatile uint32_t *)(0xA2120B04)) |= 0x200;
        audio_src_srv_report("UL ADC change to HP mode", 0);
    } else {
        //ADC NM
        *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFDFF;
    }
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((audio_nvdm_HW_config.DL_AFE_PARA & 0x01) != 0) {
#else
    if (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Class_AB_G_Performance_Sel != 0x00) {
#endif
        //DAC HP
        *((volatile uint32_t *)(0xA2120B04)) |= 0x100;
        audio_src_srv_report("DL DAC change to HP mode", 0);
    } else {
        //DAC NM
        *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFEFF;
    }
#endif
    //DVFS Clk Selection from NVKEY
    nvdm_status = nvkey_data_item_length(NVID_DSP_FW_AUDIO_DVFS_CLK_SET, &tableSize);

    if (nvdm_status || !tableSize) {
        audio_src_srv_report("[AM][Warning]NVDM DVFS_CLK_SEL Fail. Status:%d Len:%u\n", 2, nvdm_status, tableSize);
        status = AUD_EXECUTION_FAIL;
    }
    if (tableSize != sizeof(audio_nvdm_dvfs_config)) {
        audio_src_srv_report("[AM][Warning]NVDM HWIO_config_init size not match. NVDM tableSize (%u) != (%d)\n", 2, tableSize, sizeof(audio_nvdm_dvfs_config));
        status = AUD_EXECUTION_FAIL;
    }

    nvdm_status = nvkey_read_data(NVID_DSP_FW_AUDIO_DVFS_CLK_SET, (uint8_t *)&audio_nvdm_dvfs_config, &tableSize);

    if (nvdm_status) {
        audio_src_srv_report("[AM][Warning]NVDM DVFS_CLK_SEL Fail 2. Status:%d pNvdmMp3Vol:0x%x\n", 2, nvdm_status, (unsigned int)&audio_nvdm_dvfs_config);
        status = AUD_EXECUTION_FAIL;
    }
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    audio_src_srv_report("HFP Sidetone_EN(%d) HFP_Sidetone_gain(0x%x) G_Sidetone_gain(0x%x) UL_AFE_PARA(0x%x) DL_AFE_PARA(0x%x)", 5, audio_nvdm_HW_config.Voice_Sidetone_EN, audio_nvdm_HW_config.Voice_Sidetone_Gain, audio_nvdm_HW_config.Reserve_Sidetone_Gain, audio_nvdm_HW_config.UL_AFE_PARA, audio_nvdm_HW_config.DL_AFE_PARA);
    audio_src_srv_report("CH_sel mode(%d) sw_channel(%d) hw_H(%d) hw_L(%d) hw_index(%d)", 5, audio_Channel_Select.modeForAudioChannel, audio_Channel_Select.audioChannel, audio_Channel_Select.hwAudioChannel.audioChannelGPIOH, audio_Channel_Select.hwAudioChannel.audioChannelGPIOL, audio_Channel_Select.hwAudioChannel.gpioIndex);
    audio_src_srv_report("HW Config Audio_in device(0x%x) Audio_out device(0x%x) Voice_in device(0x%x) Voice_out device(0x%x)", 4, audio_nvdm_HW_config.Audio_InputDev, audio_nvdm_HW_config.Audio_OutputDev, audio_nvdm_HW_config.Voice_InputDev, audio_nvdm_HW_config.Voice_OutputDev);
    audio_src_srv_report("HW Config Micbias Audio(0x%x) Voice(0x%x) Reserve(0x%x)", 3, audio_nvdm_HW_config.MICBIAS_PARA_Audio, audio_nvdm_HW_config.MICBIAS_PARA_Voice, audio_nvdm_HW_config.MICBIAS_PARA_RESERVE);
    audio_src_srv_report("HW Config I2S Audio(0x%x) Voice(0x%x) Reserve(0x%x)", 3, audio_nvdm_HW_config.I2S_PARA_Audio, audio_nvdm_HW_config.I2S_PARA_Voice, audio_nvdm_HW_config.I2S_PARA_RESERVE);
    audio_src_srv_report("HW Config PARA AMIC_PARA(0x%x) DMIC_PARA(0x%x) LOOPBACK_RATE(0x%x) LOOPBACK_PARA(0x%x)", 4, audio_nvdm_HW_config.AMIC_PARA, audio_nvdm_HW_config.DMIC_PARA, audio_nvdm_HW_config.LOOPBACK_RATE, audio_nvdm_HW_config.LOOPBACK_PARA);
    audio_src_srv_report("HW Config mic MIC_Select_Main(0x%x) MIC_Select_Ref(0x%x) MIC_Select_Ref_2(0x%x) MIC_Select_Ref_3(0x%x)", 4, audio_nvdm_HW_config.MIC_Select_Main, audio_nvdm_HW_config.MIC_Select_Ref, audio_nvdm_HW_config.MIC_Select_Ref_2, audio_nvdm_HW_config.MIC_Select_Ref_3);
    audio_src_srv_report("HW Config mic MIC_Select_ANC_FF(0x%x) MIC_Select_ANC_FB(0x%x) ANC_OutputDev(0x%x) ", 3, audio_nvdm_HW_config.MIC_Select_ANC_FF, audio_nvdm_HW_config.MIC_Select_ANC_FB, audio_nvdm_HW_config.ANC_OutputDev);
    audio_src_srv_report("HFP_DVFS_CLK(0x%x) ", 1, audio_nvdm_dvfs_config.HFP_DVFS_CLK);
#endif

    /*set audio nvdm read done flg*/
    g_audio_nvdm_init_flg = true;

    return status;
}
#endif //MTK_AVM_DIRECT


void audio_message_audio_handler(hal_audio_event_t event, void *data)
{
    audio_src_srv_report("[Audio Clock] dsp amp mcu callback, event [%d]", 1, event);
    _UNUSED(data);
    if (event == HAL_AUDIO_EVENT_DATA_REQUEST) {
        // #ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
        //     bool set_dl_clkmux = ami_hal_audio_status_query_running_flag(AUDIO_MESSAGE_TYPE_AFE)?false:true;
        // #endif
        //     ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_AFE, true);
        // #ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
        //     if(set_dl_clkmux){
        //         hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_AFE, HAL_AUDIO_DEVICE_DAC_DUAL, HAL_AUDIO_FIXED_AFE_96K_SAMPLE_RATE, true);
        //         audio_src_srv_report("[hi-res]Set AFE count", 0);
        //     }
        // #endif
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
    } else if (event == HAL_AUDIO_EVENT_DATA_DIRECT) {
        if((*(uint32_t*)data) == AFE_COMMON_PARA_DAC_DEACTIVE){
            hal_audio_status_change_dac_mode_handler();
        }
#endif
    } else {
        // #ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
        //     hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_AFE, 0, 0, false);
        // #endif
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_AMP, NULL, false);
    }
}

#if defined(AIR_WIRED_AUDIO_ENABLE)
extern uint8_t g_usb_volume_sub_id;
static void audio_transmitter_wired_audio_stop_close_usb_in(uint16_t scenario_and_id)
{
    audio_transmitter_playback_stop(scenario_and_id);
    audio_transmitter_playback_close(scenario_and_id);

    wired_audio_minus_mix_stream_start_number();
}
#endif /* AIR_WIRED_AUDIO_ENABLE */

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
extern cm4_dsp_audio_sync_request_param_t *hal_audio_query_audio_sync_request_param(void);
extern uint8_t g_hfp_RX_EQ_mode;
extern uint8_t g_ble_RX_EQ_mode;
extern uint32_t *g_nv_normal;
extern uint32_t *g_nv_mode1;
extern uint32_t *g_nv_mode2;
void audio_request_sync_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    bt_sink_srv_am_background_t         *background_info = &(amm_ptr->background_info);
    bt_sink_srv_am_id_t                 aud_id           = background_info->aud_id;
    cm4_dsp_audio_sync_request_param_t *info = hal_audio_query_audio_sync_request_param();
    uint32_t enter_cnt = 0; // function enter gpt count
    uint32_t exit_cnt = 0; // function exit gpt count
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &enter_cnt);
    uint16_t sub_id = background_info->sync_parm.sync_scenario_type << 8 | background_info->sync_parm.sync_action_type;
    // convert volume level into digital/analog gain
    bt_sink_srv_audio_setting_vol_info_t vol_info = {0};
    bt_sink_srv_audio_setting_vol_t vol = {0};
    memset(info, 0, sizeof(cm4_dsp_audio_sync_request_param_t)); // init share info
    // check current pseudo device
    if (background_info->sync_parm.sync_action_type == MCU2DSP_SYNC_REQUEST_SET_VOLUME) {
        switch (background_info->sync_parm.sync_scenario_type) { // scenario type [HFP/A2DP/ANC/VP]
            case MCU2DSP_SYNC_REQUEST_HFP:
                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = g_rAm_aud_id[aud_id].contain_ptr->local_context.hfp_format.hfp_codec.type;
                vol_info.vol_info.hfp_vol_info.dev_out = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_device;
                g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_volume = background_info->sync_parm.vol_out.vol_level; // update
                vol_info.vol_info.hfp_vol_info.lev_out = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_volume;
                vol_info.vol_info.hfp_vol_info.dev_in = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_in.audio_device;
                vol_info.vol_info.hfp_vol_info.lev_in = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_in.audio_volume;
                bt_sink_srv_audio_setting_get_vol(&vol_info, &vol);
                info->vol_gain_info.gain = vol.vol.hfp_vol.vol_out.digital;
                info->vol_gain_info.gain_select = background_info->sync_parm.vol_out.channel;
                break;
            case MCU2DSP_SYNC_REQUEST_A2DP:
                vol_info.type = VOL_A2DP;
                vol_info.vol_info.a2dp_vol_info.dev = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_device;
                g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_volume = background_info->sync_parm.vol_out.vol_level;
                vol_info.vol_info.a2dp_vol_info.lev = g_rAm_aud_id[aud_id].contain_ptr->audio_stream_out.audio_volume;
                bt_sink_srv_audio_setting_get_vol(&vol_info, &vol);
                info->vol_gain_info.gain = vol.vol.a2dp_vol.vol.digital;
                info->vol_gain_info.gain_select = background_info->sync_parm.vol_out.channel;
                break;
            case MCU2DSP_SYNC_REQUEST_ANC: // differ from others
                // [TODO]

                break;
            case MCU2DSP_SYNC_REQUEST_VP:
                vol_info.type = VOL_VP;
                // VP no need audio_device parameter
                vol_info.vol_info.vp_vol_info.lev = background_info->sync_parm.vol_out.vol_level;
                bt_sink_srv_audio_setting_get_vol(&vol_info, &vol);
                info->vol_gain_info.gain = vol.vol.vp_vol.vol.digital;
                info->vol_gain_info.gain_select = background_info->sync_parm.vol_out.channel;
                break;
            default:
                // error happen
                audio_src_srv_report("[DSP SYNC] am type error [%d]", (uint32_t)background_info->sync_parm.sync_scenario_type);
                return;
                break;
        }
    } else if (background_info->sync_parm.sync_action_type == MCU2DSP_SYNC_REQUEST_SET_MUTE) {
        if (background_info->sync_parm.sync_scenario_type == MCU2DSP_SYNC_REQUEST_LLF) {
            info->vol_gain_info.gain = background_info->sync_parm.vol_out.vol_level;
        }
    } else if (background_info->sync_parm.sync_action_type == MCU2DSP_SYNC_REQUEST_SET_RX_EQ) {
        bool hfp_cvsd = false;
        if ((background_info->sync_parm.sync_scenario_type == MCU2DSP_SYNC_REQUEST_HFP)
            && (g_rAm_aud_id[aud_id].contain_ptr->local_context.hfp_format.hfp_codec.type == BT_HFP_CODEC_TYPE_CVSD)) {
            hfp_cvsd = true;
        }
        if (!((g_nv_normal) && (g_nv_mode1) && (g_nv_mode2))) {
            voice_set_RX_EQ_nv(background_info->sync_parm.sync_scenario_type, hfp_cvsd);
        }

        switch (background_info->sync_parm.sync_scenario_type) {
            case MCU2DSP_SYNC_REQUEST_HFP:
                switch (g_hfp_RX_EQ_mode) {
                    case call_RX_EQ_MODE_NORMAL:
                        info->nvkey_addr = g_nv_normal;
                        break;
                    case call_RX_EQ_MODE_1:
                        info->nvkey_addr = g_nv_mode1;
                        break;
                    case call_RX_EQ_MODE_2:
                        info->nvkey_addr = g_nv_mode2;
                        break;
                    default:
                        break;
                }
                break;
            case MCU2DSP_SYNC_REQUEST_BLE:
                switch (g_ble_RX_EQ_mode) {
                    case call_RX_EQ_MODE_NORMAL:
                        info->nvkey_addr = g_nv_normal;
                        break;
                    case call_RX_EQ_MODE_1:
                        info->nvkey_addr = g_nv_mode1;
                        break;
                    case call_RX_EQ_MODE_2:
                        info->nvkey_addr = g_nv_mode2;
                        break;
                    default:
                        break;
                }
                break;
            default:
                // error happen
                audio_src_srv_report("[DSP SYNC] am type error [%d]", (uint32_t)background_info->sync_parm.sync_scenario_type);
                return;
                break;
        }

        audio_src_srv_report("[DSP SYNC] cm4 send info: type [%d], t_cnt = %u, nvkey_addr = 0x%x", 3,
                             background_info->sync_parm.sync_scenario_type, background_info->sync_parm.target_gpt_cnt, info->nvkey_addr);
    }

    info->gpt_count = background_info->sync_parm.target_gpt_cnt;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &exit_cnt);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_SYNC_REQUEST, sub_id, (uint32_t)info, true);
    audio_src_srv_report("[DSP SYNC] cm4 send info: type [%d], t_cnt = %u, out_level = %d, channel = %d, out_gain = 0x%x", 5,
                         background_info->sync_parm.sync_scenario_type, info->gpt_count, background_info->sync_parm.vol_out.vol_level,
                         info->vol_gain_info.gain_select, info->vol_gain_info.gain);
    audio_src_srv_report("[DSP SYNC] cm4 send info: enter_cnt = %u, exit_cnt = %u", 2, enter_cnt, exit_cnt);
}

#endif /* AIR_BT_AUDIO_SYNC_ENABLE */

#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
extern void audio_volume_monitor_node_deinit(audio_scenario_type_t type);
void audio_volume_monitor_stop_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    bt_sink_srv_am_background_t *background_info = &(amm_ptr->background_info);
    audio_scenario_type_t type = background_info->local_feature.feature_param.type;
    /* we need am to aviod async between dsp and mcu */
    hal_audio_dsp_controller_send_message(MSG_DSP2MCU_AUDIO_VOLUME_MONITOR_STOP, 0, (uint32_t)type, true); // no need wait ack
    audio_volume_monitor_node_deinit(type);
}
#endif /* AIR_AUDIO_VOLUME_MONITOR_ENABLE */
