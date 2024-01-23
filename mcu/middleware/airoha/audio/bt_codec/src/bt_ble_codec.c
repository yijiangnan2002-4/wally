/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_BT_CODEC_BLE_ENABLED

#include "bt_ble_codec_internal.h"
#ifdef  BT_LE_AUDIO_ENABLE
#include "bt_le_audio_def.h"
#endif

#include "hal_gpt.h"
#include "hal_audio.h"
#include "hal_dvfs.h"
#include "task_def.h"
#include "fixrate_control.h"
#include "bt_ble_codec_internal.h"
#include "bt_sink_srv_ami.h"
#include "audio_nvdm_common.h"
#include "audio_nvdm_coef.h"
#include "audio_log.h"

#include "bt_sink_srv_le_cap_audio_manager.h"

#define CODEC_AUDIO_MODE 0x0
#define CODEC_VOICE_MODE 0x1
#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
#define CODEC_LL_MODE       0x2
#endif

#if defined(AIR_BTA_IC_PREMIUM_G3)
#if defined(HAL_CLOCK_MODULE_ENABLED)
#define BLE_MUSIC_DVFS_SPEED HAL_DVFS_OPP_LOW
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE)
#define BLE_CALL_DVFS_SPEED HAL_DVFS_OPP_MID
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_OPP_HIGH
#else
#define BLE_CALL_DVFS_SPEED HAL_DVFS_OPP_LOW
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_OPP_LOW
#endif

#include "hal_audio_message_struct_common.h"

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define ULL_BLE_UL_DVFS_SPEED HAL_DVFS_OPP_HIGH
#define ULL_BLE_DL_DVFS_SPEED_MONO      HAL_DVFS_OPP_MID
#define ULL_BLE_DL_DVFS_SPEED_STEREO    HAL_DVFS_OPP_MID
#endif
#endif // #if defined(HAL_CLOCK_MODULE_ENABLED)
#include "hal_audio_message_struct_common.h"

#elif defined(AIR_BTA_IC_PREMIUM_G2)
#if defined(HAL_CLOCK_MODULE_ENABLED)
#define BLE_MUSIC_DVFS_SPEED HAL_DVFS_HALF_SPEED_52M
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE)
#define BLE_CALL_DVFS_SPEED HAL_DVFS_FULL_SPEED_104M
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_HIGH_SPEED_208M
#else
#define BLE_CALL_DVFS_SPEED HAL_DVFS_HALF_SPEED_52M
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_HALF_SPEED_52M
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define ULL_BLE_UL_DVFS_SPEED HAL_DVFS_HIGH_SPEED_208M
#define ULL_BLE_DL_DVFS_SPEED_MONO      HAL_DVFS_FULL_SPEED_104M
#define ULL_BLE_DL_DVFS_SPEED_STEREO    HAL_DVFS_HIGH_SPEED_208M
#endif
#endif // #if defined(HAL_CLOCK_MODULE_ENABLED)

#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#if defined(HAL_CLOCK_MODULE_ENABLED)
#define BLE_MUSIC_DVFS_SPEED HAL_DVFS_OPP_LOW
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE)
#define BLE_CALL_DVFS_SPEED HAL_DVFS_OPP_MID
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_OPP_HIGH
#if defined(AIR_FULL_ADAPTIVE_ANC_ENABLE)
#include "anc_control_api.h"
#define BLE_CALL_DVFS_SPEED_FADP_ANC_ON     HAL_DVFS_OPP_HIGH
#define BLE_CALL_DVFS_SPEED_FADP_ANC_OFF    HAL_DVFS_OPP_LOW
#endif
#else
#define BLE_CALL_DVFS_SPEED HAL_DVFS_OPP_LOW
#define BLE_CALL_DVFS_SPEED_HIGH_RES HAL_DVFS_OPP_LOW
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define ULL_BLE_UL_DVFS_SPEED           HAL_DVFS_OPP_HIGH
#define ULL_BLE_DL_DVFS_SPEED_MONO      HAL_DVFS_OPP_MID
#define ULL_BLE_DL_DVFS_SPEED_STEREO    HAL_DVFS_OPP_HIGH
#endif
#endif // #if defined(HAL_CLOCK_MODULE_ENABLED)

#endif

log_create_module(BT_BLE_CODEC, PRINT_LEVEL_INFO);

#define BLE_LOG_MSGID_E(fmt, arg...) LOG_MSGID_E(BT_BLE_CODEC, "[BLE] "fmt,##arg)
#define BLE_LOG_MSGID_W(fmt, arg...) LOG_MSGID_W(BT_BLE_CODEC, "[BLE] "fmt,##arg)
#define BLE_LOG_MSGID_I(fmt, arg...) LOG_MSGID_I(BT_BLE_CODEC, "[BLE] "fmt,##arg)
#define BLE_LOG_MSGID_D(fmt, arg...) LOG_MSGID_D(BT_BLE_CODEC, "[BLE] "fmt,##arg)

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif
#define BLE_CALL_APPLY_EC_NR_EQ

static bool bt_ble_codec_dl_is_running = false;
static bool bt_ble_codec_ul_is_running = false;
#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
static bool bt_ble_is_speical_device = false;
#endif
static bool bt_ble_is_lightmode = false;

static bt_ble_codec_internal_handle_t *bt_ble_codec_dl_internal_handle = NULL;
static bt_ble_codec_internal_handle_t *bt_ble_codec_ul_internal_handle = NULL;

/*********************************************
 *********For BLE sink side code flow**************
 *********************************************/
#if 0
static bt_codec_media_status_t bt_ble_process(bt_media_handle_t *handle, bt_codec_media_event_t event)
{
    BLE_LOG_MSGID_E("process\r\n", 0);
    return BT_CODEC_MEDIA_STATUS_ERROR;
}


static void bt_set_aws_flag(bt_media_handle_t *handle, bool is_aws)
{
#if defined(MTK_AVM_DIRECT)
    bt_ble_codec_internal_handle_t *internal_handle = (bt_ble_codec_internal_handle_t *)handle;

    BLE_LOG_MSGID_I("BLE codec set aws flag:%d\r\n", 1, is_aws);
    internal_handle->aws_flag = is_aws;
#endif
}
#endif

static void bt_trigger_mic(bt_media_handle_t *handle)
{
#if defined(MTK_AVM_DIRECT)
    if (handle) {
        BLE_LOG_MSGID_I("BLE codec Trigger Mic On\r\n", 0);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_PLAY_EN, 0, 0, 0);
    }
#endif
}
#if 0
static void bt_side_tone_enable(bt_media_handle_t *handle)
{
#if 0
    mcu2dsp_sidetone_param_t sidetone;
    void *p_param_share;

    if (!handle) {
        BLE_LOG_MSGID_I("[HFP Codec]SideTone Enable No Handle\n", 0);
        return;
    }

    BLE_LOG_MSGID_I("[HFP Codec]SideTone Enable\n", 0);

    sidetone.in_device                       = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
    sidetone.in_interface                    = HAL_AUDIO_INTERFACE_1;
    sidetone.in_misc_parms                   = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
    sidetone.out_device                      = HAL_AUDIO_DEVICE_DAC_DUAL;
    sidetone.out_interface                   = HAL_AUDIO_INTERFACE_NONE;
    sidetone.out_misc_parms                  = DOWNLINK_PERFORMANCE_NORMAL;
    sidetone.in_channel                      = HAL_AUDIO_DIRECT;
    sidetone.gain                            = 600;
    sidetone.sample_rate                     = 16000;

    p_param_share = hal_audio_dsp_controller_put_paramter(&sidetone, sizeof(mcu2dsp_sidetone_param_t), AUDIO_MESSAGE_TYPE_SIDETONE);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_START, 0, (uint32_t)p_param_share, true);
#else
    /****/
#endif
}

static void bt_side_tone_disable(bt_media_handle_t *handle)
{
#if 0
    mcu2dsp_sidetone_param_t sidetone;
    void *p_param_share;

    if (!handle) {
        BLE_LOG_MSGID_I("[HFP Codec]SideTone Disable No Handle\n", 0);
        return;
    }

    BLE_LOG_MSGID_I("[HFP Codec]SideTone Disable\n", 0);

    sidetone.in_device                       = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
    sidetone.in_interface                    = HAL_AUDIO_INTERFACE_1;
    sidetone.in_misc_parms                   = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
    sidetone.out_device                      = HAL_AUDIO_DEVICE_DAC_DUAL;
    sidetone.out_interface                   = HAL_AUDIO_INTERFACE_NONE;
    sidetone.out_misc_parms                  = DOWNLINK_PERFORMANCE_NORMAL;
    sidetone.in_channel                      = HAL_AUDIO_DIRECT;
    sidetone.gain           = 0;
    sidetone.sample_rate    = 16000;

    p_param_share = hal_audio_dsp_controller_put_paramter(&sidetone, sizeof(mcu2dsp_sidetone_param_t), AUDIO_MESSAGE_TYPE_SIDETONE);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_STOP, 0, (uint32_t)p_param_share, true);
#else
    /****/
#endif
}
#endif
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
static bool g_ble_feature_mode_busy_flg = false;
static bool g_ble_feature_mode_1st_notify = false;
static uint32_t g_ble_feature_mode = 0;
SemaphoreHandle_t g_xSemaphore_ble_nvkey_change_notify;

ble_feature_mode_t ble_get_feature_mode(void)
{
    BLE_LOG_MSGID_I("ble_get_feature_mode: %d", 1, g_ble_feature_mode);
    return g_ble_feature_mode;
}

void ble_feature_mode_notify_callback(audio_nvdm_user_t user, audio_nvdm_status_t status, void *user_data)
{
    switch (status) {
        case AUDIO_NVDM_STATUS_PREV_CHANGE:
            g_ble_feature_mode_1st_notify = true;
            break;
        case AUDIO_NVDM_STATUS_CHANGE:
            break;
        /*The audio nvkey can be update now*/
        case AUDIO_NVDM_STATUS_POST_CHANGE:
            if (g_ble_feature_mode_1st_notify == false) {
                if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
                    xSemaphoreGive(g_xSemaphore_ble_nvkey_change_notify);
                }else {
                    xSemaphoreGiveFromISR(g_xSemaphore_ble_nvkey_change_notify,NULL);
                }
            } else {
                g_ble_feature_mode_1st_notify = false;
            }
            break;
        default:
            break;
    }
}

/* Four type of call case
 * Case 1. call when the BLE is not start.
 * Case 2. call when the BLE is just start up, but not completed.
 * Case 3. call when the BLE has been start up.
 * Case 4. call when the BLE has been start up, and other user is changing the mode.
 * Only case 1/3 allows the mode change.
 */
bool ble_set_feature_mode(uint32_t mode)
{
    uint32_t irq_status;
    bt_sink_srv_am_result_t am_status;

    if ((mode >= BLE_FEATURE_MODE_MAX) || (mode == g_ble_feature_mode) ||
        (g_ble_feature_mode_1st_notify == true) ||
        (g_ble_feature_mode_busy_flg == true)) {
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
            BLE_LOG_MSGID_I("ble feature mode: para check fail, prev mode %d, curr mode: %d, mode_status %d %d", 4,
                g_ble_feature_mode ,mode, g_ble_feature_mode_1st_notify, g_ble_feature_mode_busy_flg);
        return false;
    }

    g_ble_feature_mode_busy_flg = true;
    hal_nvic_restore_interrupt_mask(irq_status);

    /* update call mode */
    g_ble_feature_mode = mode;

    /* If it's not in BLE scenario, just save the current mode for next BLE open. */
    if ((g_prCurrent_player == NULL) || (g_prCurrent_player->type != BLE)) {
        BLE_LOG_MSGID_I("ble feature mode: not in BLE scenario", 0);
        g_ble_feature_mode_busy_flg = false;
        return true;
    }

    am_status = am_audio_set_resume();
    if (am_status != AUD_EXECUTION_SUCCESS) {
        g_ble_feature_mode_busy_flg = false;
        return false;
    }

    /* wait for audio nvkey sync to DSP */
    if (pdTRUE != xSemaphoreTake(g_xSemaphore_ble_nvkey_change_notify, 1000 / portTICK_PERIOD_MS)) {
        BLE_LOG_MSGID_I("ble feature mode: wait DSP timeout", 0);
        g_ble_feature_mode_busy_flg = false;
        return false;
    }
    g_ble_feature_mode_busy_flg = false;
    BLE_LOG_MSGID_I("ble feature mode: set mode %d done", 1, g_ble_feature_mode);

    return true;
}

#endif

#if defined(AIR_BLE_FEATURE_MODE_ENABLE) || defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE)

void ble_replace_feature_mode_nvkey_id(DSP_FEATURE_TYPE_LIST *p_feature_list)
{
#ifdef AIR_PSAP_ENABLE
    if (ble_get_feature_mode() == BLE_FEATURE_MODE_PSAP_0) {
#ifdef AIR_BT_BLE_SWB_ENABLE
        p_feature_list[0] = FUNC_PSAP_BLE_SWB_1;
#else
        p_feature_list[0] = FUNC_PSAP_HFP_WB_1;
#endif
    }
#endif
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
    if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
#ifdef AIR_BT_BLE_SWB_ENABLE
        p_feature_list[0] = FUNC_SWB_BOOM_MIC;
#else
        p_feature_list[0] = FUNC_WB_BOOM_MIC;
#endif
    }
#endif
}

uint16_t ble_restore_feature_mode_nvkey_id(uint16_t nvkey_id)
{
    uint16_t repalce_nvkey_id;

    repalce_nvkey_id = nvkey_id;
    if ((g_prCurrent_player) && ((g_prCurrent_player->type == BLE) || (g_prCurrent_player->type == ULL_BLE_UL) || (g_prCurrent_player->type == ULL_BLE_DL))) {
#ifdef AIR_PSAP_ENABLE
        if (nvkey_id == NVKEY_DSP_PARA_SWB_RX_EQ_2ND) {
            repalce_nvkey_id = NVKEY_DSP_PARA_SWB_RX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_RX_EQ_2ND) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_RX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_POSITIVE_GAIN_2ND) {
            repalce_nvkey_id = NVKEY_DSP_PARA_POSITIVE_GAIN;
        }
#endif
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        if (nvkey_id == NVKEY_DSP_PARA_AEC_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC;
        } else if (nvkey_id == NVKEY_DSP_PARA_NR_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NR;
        } else if (nvkey_id == NVKEY_DSP_PARA_INEAR_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_INEAR;
        } else if (nvkey_id == NVKEY_DSP_PARA_NB_TX_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NB_TX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_VO_CPD_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_VO_CPD;
        } else if (nvkey_id == NVKEY_DSP_PARA_SWB_TX_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_SWB_TX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_NB_TX_FIR_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NB_TX_FIR_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_FIR_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_FIR_EQ;
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        } else if (nvkey_id == NVKEY_DSP_PARA_AEC_FB_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC;
        } else if (nvkey_id == NVKEY_DSP_PARA_AEC_NR_FB_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC_NR_SWB;
#endif
#if 0
        } else if (nvkey_id == NVKEY_DSP_PARA_SWB_TX_FIR_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_SWB_TX_FIR_EQ;
#endif
        } else if (nvkey_id == NVKEY_DSP_PARA_AEC_NR_SWB_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC_NR_SWB;
        } else if (nvkey_id == NVKEY_DSP_PARA_SWB_TX_VO_CPD_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_SWB_TX_VO_CPD;
        }
#endif
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        if (nvkey_id == NVKEY_DSP_PARA_AEC_FB) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC;
        } else if (nvkey_id == NVKEY_DSP_PARA_AEC_NR_FB) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC_NR_SWB;
        }
#endif
    }

    return repalce_nvkey_id;
}

#endif

void aud_bt_codec_ble_callback(bt_media_handle_t *handle, bt_codec_media_event_t event_id)
{
    switch(event_id){
        case BT_CODEC_MEDIA_HFP_AVC_PARA_SEND:
        case BT_CODEC_MEDIA_LEQ_PARA_SEND:
        if ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) //for earbuds
        ||((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_NONE))) //for headset
        {
            printf("[NDVC] It's Agent, send avc_vol");
            if (g_prCurrent_player->type == BLE) {
                g_prCurrent_player->local_context.ble_format.ble_event = event_id;
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

static void bt_ble_isr_handler(hal_audio_event_t event, void *data)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    bt_ble_codec_internal_handle_t *internal_handle = (bt_ble_codec_internal_handle_t *)data;
    bt_media_handle_t *handle = (bt_media_handle_t *)internal_handle;
    //BLE_LOG_MSGID_I("[NDVC] bt_ble_isr_handler, event %d",1,event);
    if ((event == HAL_AUDIO_EVENT_HFP_PARA_SEND) && (data != NULL) && (handle->handler != NULL)) {
        handle->handler(handle, BT_CODEC_MEDIA_HFP_AVC_PARA_SEND);
    }else if ((event == HAL_AUDIO_EVENT_LEQ_PARA_SEND) && (data != NULL) && (handle->handler != NULL)) {
        handle->handler(handle, BT_CODEC_MEDIA_LEQ_PARA_SEND);
    }
#endif
}

static bt_codec_media_status_t bt_ble_play(bt_media_handle_t *handle)
{
    bt_ble_codec_internal_handle_t *internal_handle = (bt_ble_codec_internal_handle_t *)handle;
    bool aws_flag = internal_handle->aws_flag;
    mcu2dsp_start_param_t start_param;
    memset(&start_param, 0, sizeof(mcu2dsp_start_param_t));
    void *p_param_share;

    if (internal_handle->codec_info.channel_mode != CHANNEL_MODE_UL_ONLY) {
        BLE_LOG_MSGID_I("start DL\r\n", 0);
    start_param.param.stream_in = STREAM_IN_BLE;  //Rdebug Need DSP in
    start_param.param.stream_out = STREAM_OUT_AFE;
    start_param.stream_out_param.afe.aws_flag = aws_flag;

        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL, bt_ble_isr_handler, handle);  //Rdebug Need message
#ifdef AIR_DCHS_MODE_ENABLE
    start_param.stream_out_param.afe.mce_flag = true; //enable play en
    dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_DL_START, AUDIO_SCENARIO_TYPE_BLE_DL , NULL, &start_param);
#endif
    p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_START, 0, (uint32_t)p_param_share, true);
    }
    if (internal_handle->codec_info.channel_mode != CHANNEL_MODE_DL_ONLY) {
        BLE_LOG_MSGID_I("start UL\r\n", 0);
        start_param.param.stream_in = STREAM_IN_AFE;
        start_param.param.stream_out = STREAM_OUT_BLE;
        start_param.stream_in_param.afe.aws_flag = aws_flag;
        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL, bt_ble_isr_handler, handle);  //Rdebug Need message
#ifdef AIR_DCHS_MODE_ENABLE
	    dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_UL_START, AUDIO_SCENARIO_TYPE_BLE_UL , NULL, &start_param);
#endif
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_START, 0, (uint32_t)p_param_share, true);

#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
        if(internal_handle->codec_info.ul_param.codec == BT_CODEC_TYPE_LE_AUDIO_LC3){
        audio_volume_monitor_param_t param = {0};
        param.cb = mute_mic_detection;
        param.volume_len = 1;
        param.ch         = 1;
        param.user_data  = NULL;
        audio_volume_monitor_start(AUDIO_SCENARIO_TYPE_BLE_UL, &param);
        }
#endif
    }

    handle->state = BT_CODEC_STATE_PLAY;

    return BT_CODEC_MEDIA_STATUS_OK;
}
#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
void bt_ble_set_special_device(bool is_special)
{
    BLE_LOG_MSGID_I("ble_set_special_device:%d", 1, is_special);
    bt_ble_is_speical_device = is_special;
}
#endif

void bt_ble_set_lightmode(bool is_lightmode)
{
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
    BLE_LOG_MSGID_I("bt_ble_set_lightmode:%d but set to %d", 2, is_lightmode, 0);
    bt_ble_is_lightmode = 0;
#else
    BLE_LOG_MSGID_I("bt_ble_set_lightmode:%d", 1, is_lightmode);
    bt_ble_is_lightmode = is_lightmode;
#endif
}

#if defined(HAL_DVFS_MODULE_ENABLED)
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static void bt_ble_lightmode_dvfs_setting(dvfs_frequency_t *ble_dvfs_clk) {
    dvfs_frequency_t ble_default_clk = *ble_dvfs_clk;
    if (bt_ble_is_lightmode) {
        switch (ble_default_clk) {

#if defined(AIR_BTA_IC_PREMIUM_G3)||defined(AIR_BTA_IC_STEREO_HIGH_G3)
            case HAL_DVFS_OPP_LOW:
                break;
            case HAL_DVFS_OPP_MID:
                *ble_dvfs_clk = HAL_DVFS_OPP_LOW;
                break;
            case HAL_DVFS_OPP_HIGH:
                *ble_dvfs_clk = HAL_DVFS_OPP_MID;
                break;
#else
            case HAL_DVFS_HALF_SPEED_52M:
                break;
            case HAL_DVFS_FULL_SPEED_104M:
                *ble_dvfs_clk = HAL_DVFS_HALF_SPEED_52M;
                break;
            case HAL_DVFS_HIGH_SPEED_208M:
                *ble_dvfs_clk = HAL_DVFS_FULL_SPEED_104M;
                break;
#endif
            default:
                break;
        }
        BLE_LOG_MSGID_W("dvfs default %d, lightmode %d\r\n", 2, ble_default_clk, *ble_dvfs_clk);
    }
}
#endif
#endif

#if defined(HAL_DVFS_MODULE_ENABLED)
static dvfs_frequency_t ble_dl_dvfs_clk;
static dvfs_frequency_t ble_ul_dvfs_clk;

#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE) && (defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE))
static dvfs_frequency_t ble_ul_fadp_dvfs_clk;

static void bt_ble_call_fadp_anc_callback_handler(audio_anc_control_event_t event_id, audio_anc_control_result_t result)
{
    if ((AUDIO_ANC_CONTROL_EXECUTION_SUCCESS == result) && (event_id == AUDIO_ANC_CONTROL_EVENT_ON)) {
        /*ANC ON Case*/
        if(ble_ul_fadp_dvfs_clk != BLE_CALL_DVFS_SPEED_FADP_ANC_ON) {
            ble_ul_fadp_dvfs_clk = BLE_CALL_DVFS_SPEED_FADP_ANC_ON;
            BLE_LOG_MSGID_I("FADP ANC ON: UL dvfs lock %d\r\n", 1, ble_ul_fadp_dvfs_clk);
            hal_dvfs_lock_control(ble_ul_fadp_dvfs_clk, HAL_DVFS_LOCK);
        }
    } else if ((AUDIO_ANC_CONTROL_EXECUTION_SUCCESS == result) && (event_id == AUDIO_ANC_CONTROL_EVENT_OFF)) {
        /*ANC OFF Case*/
        if(ble_ul_fadp_dvfs_clk != BLE_CALL_DVFS_SPEED_FADP_ANC_OFF) {
            BLE_LOG_MSGID_I("FADP ANC OFF: UL dvfs unlock %d\r\n", 1, ble_ul_fadp_dvfs_clk);
            hal_dvfs_lock_control(ble_ul_fadp_dvfs_clk, HAL_DVFS_UNLOCK);
            ble_ul_fadp_dvfs_clk = BLE_CALL_DVFS_SPEED_FADP_ANC_OFF;
        }
    }

}
#endif // #ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
#endif // #if defined(HAL_DVFS_MODULE_ENABLED)

static bt_codec_media_status_t bt_ble_stop(bt_media_handle_t *handle)
{
    bt_ble_codec_internal_handle_t *internal_handle = (bt_ble_codec_internal_handle_t *)handle;


    if (internal_handle->codec_info.channel_mode != CHANNEL_MODE_UL_ONLY) {
        BLE_LOG_MSGID_I("stop DL\r\n", 0);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_STOP, 0, 0, true);
        hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL);

        #if defined(HAL_DVFS_MODULE_ENABLED)
        BLE_LOG_MSGID_I("DL dvfs unlock %d\r\n", 1, ble_dl_dvfs_clk);
        hal_dvfs_lock_control(ble_dl_dvfs_clk, HAL_DVFS_UNLOCK);
        #endif
    }
    if (internal_handle->codec_info.channel_mode != CHANNEL_MODE_DL_ONLY) {
        BLE_LOG_MSGID_I("stop UL\r\n", 0);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_STOP, 0, 0, true);
        hal_audio_service_unhook_callback(AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL);

        #if defined(HAL_DVFS_MODULE_ENABLED)
        BLE_LOG_MSGID_I("UL dvfs unlock %d\r\n", 1, ble_ul_dvfs_clk);
        hal_dvfs_lock_control(ble_ul_dvfs_clk, HAL_DVFS_UNLOCK);

        #if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE) && (defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE))
        audio_anc_control_deregister_callback(bt_ble_call_fadp_anc_callback_handler);
        if(ble_ul_fadp_dvfs_clk != BLE_CALL_DVFS_SPEED_FADP_ANC_OFF) {
            BLE_LOG_MSGID_I("FADP ANC OFF: UL dvfs unlock %d\r\n", 1, ble_ul_fadp_dvfs_clk);
            hal_dvfs_lock_control(ble_ul_fadp_dvfs_clk, HAL_DVFS_UNLOCK);
            ble_ul_fadp_dvfs_clk = BLE_CALL_DVFS_SPEED_FADP_ANC_OFF;
        }
        #endif
        #endif // #if defined(HAL_DVFS_MODULE_ENABLED)
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
        if(internal_handle->codec_info.ul_param.codec == BT_CODEC_TYPE_LE_AUDIO_LC3){
            audio_volume_monitor_stop(AUDIO_SCENARIO_TYPE_BLE_UL);
        }
#endif
    }
    handle->state = BT_CODEC_STATE_STOP;
    return BT_CODEC_MEDIA_STATUS_OK;
}


static bt_codec_media_status_t bt_ble_dl_internal_open(const bt_codec_le_audio_dl_t *param)
{
    mcu2dsp_open_param_t *open_param;
    open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    void *p_param_share;
    bt_codec_type_le_audio_t codec_type = param->codec;

    #if defined(HAL_DVFS_MODULE_ENABLED)

    #if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3PULS || codec_type == BT_CODEC_TYPE_LE_AUDIO_VENDOR || codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD) {
        if (param->channel_num == CHANNEL_MONO) {
            ble_dl_dvfs_clk = ULL_BLE_DL_DVFS_SPEED_MONO;
        } else {
            ble_dl_dvfs_clk = ULL_BLE_DL_DVFS_SPEED_STEREO;
        }
        bt_ble_lightmode_dvfs_setting(&ble_dl_dvfs_clk);
    } else
    #endif
    if (param->context_type != AUDIO_CONTENT_TYPE_GAME) {
        ble_dl_dvfs_clk = BLE_MUSIC_DVFS_SPEED;
    } else {
#ifdef CORE_CM33
        ble_dl_dvfs_clk = HAL_DVFS_OPP_HIGH;
#else
        ble_dl_dvfs_clk = HAL_DVFS_HIGH_SPEED_208M;
#endif
    }
    BLE_LOG_MSGID_I("DL dvfs lock %d\r\n", 1, ble_dl_dvfs_clk);
    hal_dvfs_lock_control(ble_dl_dvfs_clk, HAL_DVFS_LOCK);
    #endif

    if(!open_param)
    {
        BLE_LOG_MSGID_E("dl open_param allocate fail", 0);
        AUDIO_ASSERT(0);
        return BT_CODEC_MEDIA_STATUS_ERROR;
    }

    memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

    open_param->param.stream_in  = STREAM_IN_BLE;
    open_param->param.stream_out = STREAM_OUT_AFE;
    open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL;

    open_param->stream_in_param.ble.codec_type     = codec_type;
    open_param->stream_in_param.ble.context_type   = param->context_type;
    open_param->stream_in_param.ble.sampling_frequency = param->sample_rate;
    open_param->stream_in_param.ble.is_lightmode   = bt_ble_is_lightmode;
    open_param->stream_in_param.ble.channel_num    = param->channel_num; /* channel number */
    if (param->channel_num == CHANNEL_MONO) {
        open_param->stream_in_param.ble.channel_num = 1;
    } else {
        open_param->stream_in_param.ble.channel_num = 2;
        if (param->channel_num == CHANNEL_DUAL_MONO) {
            open_param->stream_in_param.ble.dual_cis_mode = TRUE;
            open_param->stream_in_param.ble.p_sub_share_info = hal_audio_query_ble_audio_sub_dl_share_info();
        }
    }
    if (param->frame_duration == LE_AUDIO_interval_5) {
        open_param->stream_in_param.ble.frame_ms   = 5000;
    } else if (param->frame_duration == LE_AUDIO_interval_7_5) {
        open_param->stream_in_param.ble.frame_ms   = 7500;
    } else if (param->frame_duration == LE_AUDIO_interval_10) {
        open_param->stream_in_param.ble.frame_ms   = 10000;
    } else if (param->frame_duration == LE_AUDIO_interval_1) {
        open_param->stream_in_param.ble.frame_ms   = 1000;
    } else if (param->frame_duration == LE_AUDIO_interval_2) {
        open_param->stream_in_param.ble.frame_ms   = 2000;
    } else {
        open_param->stream_in_param.ble.frame_ms   = 5000;
    }
    open_param->stream_in_param.ble.frame_payload_length = param->maxframe_payload_length;
    //open_param->stream_in_param.ble.bit_depth      = HAL_AUDIO_BITS_PER_SAMPLING_16; //:TODO: decided by BT
    //open_param->stream_in_param.ble.bt_inf_address = bt_sink_srv_ami_get_bt_inf_address();
    open_param->stream_in_param.ble.p_share_info     = hal_audio_query_ble_audio_dl_share_info();
    //open_param->stream_in_param.ble.clk_info_address = hal_audio_query_rcdc_share_info();
    //open_param->stream_in_param.ble.p_air_dump_buf =
#ifdef AIR_AIRDUMP_ENABLE
	open_param->stream_in_param.ble.p_air_dump_buf = hal_audio_query_hfp_air_dump();
#endif
    hal_audio_reset_share_info(open_param->stream_in_param.ble.p_share_info);
    BLE_LOG_MSGID_I("DL codec_type:%d, context_type:0x%x, sampling_rate:%d, channel_num:%d, channel_mode:%d, frame_duration:%d, frame_payload_length:%d, start_addr:0x%x\r\n", 8,
                    open_param->stream_in_param.ble.codec_type,
                    open_param->stream_in_param.ble.context_type,
                    open_param->stream_in_param.ble.sampling_frequency,
                    open_param->stream_in_param.ble.channel_num,
                    open_param->stream_in_param.ble.channel_mode,
                    open_param->stream_in_param.ble.frame_ms,
                    open_param->stream_in_param.ble.frame_payload_length,
                    open_param->stream_in_param.ble.p_share_info->start_addr);
        if (param->context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL) {
            hal_audio_get_stream_out_setting_config(AU_DSP_VOICE, &open_param->stream_out_param);
        } else {
            hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);
        }
        BLE_LOG_MSGID_I("out_device(0x%x), channel(%d), interface(%d)", 3, open_param->stream_out_param.afe.audio_device, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface);
        open_param->stream_out_param.afe.memory          = HAL_AUDIO_MEM1;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        if (open_param->stream_in_param.ble.context_type == AUDIO_CONTENT_TYPE_MEDIA) {
            open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_I2S_MASTER;
            open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
            open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_DAC_DUAL;
            open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_2;
            BLE_LOG_MSGID_I("out_device1(0x%x), channel(%d), interface1(%d)", 3, open_param->stream_out_param.afe.audio_device1, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface1);
            open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM3;
        }
 #endif

        if (param->context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL) {
            open_param->stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
        } else {
            open_param->stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
        }
        if((param->context_type == AUDIO_CONTENT_TYPE_ULL_BLE)||(param->context_type == AUDIO_CONTENT_TYPE_WIRELESS_MIC)||(codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3PULS)){
            open_param->stream_out_param.afe.stream_out_sampling_rate = param->sample_rate;
            open_param->stream_out_param.afe.sampling_rate   = param->sample_rate;
            #if defined (AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            if (param->context_type == AUDIO_CONTENT_TYPE_ULL_BLE && (codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD)) {
                open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_96KHZ);
                open_param->stream_out_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_96KHZ);
            }
            #endif
        }else{
            open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ);
            open_param->stream_out_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ);
        }

        open_param->stream_out_param.afe.irq_period      = 0;  /* do not set irq_period, let audio framework use sample count to decide it*/
        #ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
        open_param->stream_in_param.ble.decode_mode  = ((bt_ble_is_speical_device == TRUE)&&(param->context_type == AUDIO_CONTENT_TYPE_GAME)) ? CODEC_LL_MODE : CODEC_AUDIO_MODE;
        #else
        open_param->stream_in_param.ble.decode_mode  = CODEC_AUDIO_MODE;
        #endif
        open_param->stream_out_param.afe.frame_size  = (open_param->stream_out_param.afe.sampling_rate / 1000) * open_param->stream_in_param.ble.frame_ms / 1000;
        if (aud_fixrate_get_downlink_rate(open_param->audio_scenario_type) != FIXRATE_NONE) {
            open_param->stream_out_param.afe.sampling_rate = aud_fixrate_get_downlink_rate(open_param->audio_scenario_type);
        }


        /*    Voice DL Part       */
        if (param->context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL) {
        {
                open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_DL;
                #ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
                open_param->stream_in_param.ble.decode_mode  = (bt_ble_is_speical_device == TRUE) ? CODEC_LL_MODE : CODEC_VOICE_MODE;
                #else
                open_param->stream_in_param.ble.decode_mode  = CODEC_VOICE_MODE;
                #endif
                /*for hearing protection init mode selection*/
                open_param->stream_in_param.ble.cpd_param.cpd_hse_mode = (am_audio_cpd_get_hse_mode() == CPD_HSE_MODE_2) ? ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_PARTNER)? CPD_HSE_MODE_1 : CPD_HSE_MODE_2):am_audio_cpd_get_hse_mode();
                open_param->stream_in_param.ble.cpd_param.cpd_nv_length  = gDspCPDAlg_NvLength;
                BLE_LOG_MSGID_I("[CPD][BT_CODEC] HSE mode %d, gDspCPDAlg_NvLength %d", 2, open_param->stream_in_param.ble.cpd_param.cpd_hse_mode, open_param->stream_in_param.ble.cpd_param.cpd_nv_length);

#ifdef BLE_CALL_APPLY_EC_NR_EQ
                /*
                 * if EC NR feature is enabled in call scenario
                 * sample rate and must be 16KHz due to algorithm limitation
                 */
#ifdef AIR_BT_BLE_SWB_ENABLE
                open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ);
                open_param->stream_out_param.afe.sampling_rate           = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ);
#else
                open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_16KHZ);
                open_param->stream_out_param.afe.sampling_rate           = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_16KHZ);
#endif
#else
                /* no need to adjust afe sampling rate */
#endif
#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
                open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_96KHZ);
                open_param->stream_out_param.afe.sampling_rate           = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_96KHZ);
#elif defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k)
                open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ);
                open_param->stream_out_param.afe.sampling_rate           = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ);
#endif
                open_param->stream_out_param.afe.frame_size  = (open_param->stream_out_param.afe.sampling_rate / 1000) * open_param->stream_in_param.ble.frame_ms / 1000;
                if (aud_fixrate_get_downlink_rate(open_param->audio_scenario_type) != FIXRATE_NONE) {
                    open_param->stream_out_param.afe.sampling_rate = aud_fixrate_get_downlink_rate(open_param->audio_scenario_type);
                }
            } //else {
            //open_param->stream_in_param.ble.decode_mode  = CODEC_AUDIO_MODE;
            //open_param->stream_out_param.afe.frame_size  = (open_param->stream_out_param.afe.sampling_rate/1000) * open_param->stream_in_param.ble.frame_ms/1000;
        }

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        if (open_param->stream_in_param.ble.context_type == AUDIO_CONTENT_TYPE_MEDIA) {
            open_param->stream_out_param.afe.frame_number    = 3; /* AFE buffer can cache 6 frame */
        } else {
            open_param->stream_out_param.afe.frame_number    = 6; /* AFE buffer can cache 6 frame */
        }
#else
        open_param->stream_out_param.afe.frame_number    = 6; /* AFE buffer can cache 6 frame */
#endif
        open_param->stream_out_param.afe.hw_gain         = true;
        open_param->stream_out_param.afe.adc_mode        = HAL_AUDIO_ANALOG_OUTPUT_CLASSAB;
#ifdef ENABLE_HWSRC_CLKSKEW
        if ((param->context_type != AUDIO_CONTENT_TYPE_CONVERSATIONAL) && (param->context_type != AUDIO_CONTENT_TYPE_GAME) && (param->context_type != AUDIO_CONTENT_TYPE_ULL_BLE)) {
            open_param->stream_out_param.afe.clkskew_mode    = CLK_SKEW_V2;
        } else {
            open_param->stream_out_param.afe.clkskew_mode    = CLK_SKEW_V1;
        }

#if defined AIR_HWSRC_TX_TRACKING_ENABLE || defined AIR_HWSRC_RX_TRACKING_ENABLE
        open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif
#endif
#if defined(AIR_HWSRC_IN_STREAM_ENABLE)
        open_param->stream_out_param.afe.hwsrc_type      = HAL_AUDIO_HWSRC_IN_STREAM;
        if(open_param->stream_out_param.afe.hwsrc_type == HAL_AUDIO_HWSRC_IN_STREAM) {
            open_param->stream_out_param.afe.clkskew_mode    = CLK_SKEW_V1;
        }
#endif
        BLE_LOG_MSGID_I("open_param->stream_out_param.afe.clkskew_mode:%d", 1, open_param->stream_out_param.afe.clkskew_mode);

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        //hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_1DAC_1I2S_ENABLE
        open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_NONE;
        open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_DEVICE_NONE;
        open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM3;
        open_param->stream_out_param.afe.is_low_jitter[0]    = true;
#elif AIR_AUDIO_MULTIPLE_STREAM_OUT_3I2S_ENABLE
        open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_2;
        open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_INTERFACE_3;
        open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM3|HAL_AUDIO_MEM4;
#endif
        BLE_LOG_MSGID_I("out_device0(0x%x), channel(%d), interface0(%d)", 3, open_param->stream_out_param.afe.audio_device, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface);
        BLE_LOG_MSGID_I("out_device1(0x%x), channel(%d), interface1(%d)", 3, open_param->stream_out_param.afe.audio_device1, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface1);
        BLE_LOG_MSGID_I("out_device2(0x%x), channel(%d), interface2(%d)", 3, open_param->stream_out_param.afe.audio_device2, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface2);
#endif

        BLE_LOG_MSGID_I("open DL\r\n", 0);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL, open_param, true);
        p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL);
        if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_OPEN, AUDIO_DSP_CODEC_TYPE_LC3, (uint32_t)p_param_share, true);
        }
        else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3PULS) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_OPEN, AUDIO_DSP_CODEC_TYPE_LC3PLUS, (uint32_t)p_param_share, true);
        } else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_VENDOR) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_OPEN, AUDIO_DSP_CODEC_TYPE_OPUS, (uint32_t)p_param_share, true);
        } else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_OPEN, AUDIO_DSP_CODEC_TYPE_ULD, (uint32_t)p_param_share, true);
        }
    vPortFree(open_param);
    return BT_CODEC_MEDIA_STATUS_OK;

}
static bt_codec_media_status_t bt_ble_ul_internal_open(const bt_codec_le_audio_ul_t *param)
{
        mcu2dsp_open_param_t *open_param;
        open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
        void *p_param_share;
        bt_codec_type_le_audio_t codec_type = param->codec;
#if defined(HAL_DVFS_MODULE_ENABLED)
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3PULS || codec_type == BT_CODEC_TYPE_LE_AUDIO_VENDOR || codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD) {
            ble_ul_dvfs_clk = ULL_BLE_UL_DVFS_SPEED;
            bt_ble_lightmode_dvfs_setting(&ble_ul_dvfs_clk);
        } else
#endif
        {
#ifndef AIR_DCHS_MODE_ENABLE
            if(param->sample_rate > 32000)
                ble_ul_dvfs_clk = BLE_CALL_DVFS_SPEED_HIGH_RES;
#ifdef AIR_BT_BLE_SWB_ENABLE
            else if(param->sample_rate < 32000)
                ble_ul_dvfs_clk = BLE_CALL_DVFS_SPEED_HIGH_RES;
#endif
            else
                ble_ul_dvfs_clk = BLE_CALL_DVFS_SPEED;
#else
            ble_ul_dvfs_clk = HAL_DVFS_OPP_HIGH;
#endif

            #if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE) && (defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_3RD_PARTY_NR_ENABLE))
            if(ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_FADP_ANC_STREAM)){
                ble_ul_fadp_dvfs_clk = BLE_CALL_DVFS_SPEED_FADP_ANC_ON;
                BLE_LOG_MSGID_I("FADP ANC ON: UL dvfs lock %d\r\n", 1, ble_ul_fadp_dvfs_clk);
                hal_dvfs_lock_control(ble_ul_fadp_dvfs_clk, HAL_DVFS_LOCK);
            }
            else {
                ble_ul_fadp_dvfs_clk = BLE_CALL_DVFS_SPEED_FADP_ANC_OFF;
            }

            audio_anc_control_event_t event_mask = AUDIO_ANC_CONTROL_EVENT_ON | AUDIO_ANC_CONTROL_EVENT_OFF | AUDIO_ANC_CONTROL_EVENT_FORCE_OFF;
            audio_anc_control_register_callback(bt_ble_call_fadp_anc_callback_handler, event_mask, AUDIO_ANC_CONTROL_CB_LEVEL_ALL);
            #endif
        }

        BLE_LOG_MSGID_I("UL dvfs lock %d\r\n", 1, ble_ul_dvfs_clk);
        hal_dvfs_lock_control(ble_ul_dvfs_clk, HAL_DVFS_LOCK);

#endif
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#ifdef AIR_LD_NR_ENABLE
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            ami_mic_ld_nr_set_parameter(hal_audio_query_voice_mic_type());
#else
            ami_mic_ld_nr_set_parameter(VOICE_MIC_TYPE_FIXED);
#endif
#endif
#endif

        if(!open_param)
        {
            BLE_LOG_MSGID_E("ul open_param allocate fail", 0);
            AUDIO_ASSERT(0);
            return BT_CODEC_MEDIA_STATUS_ERROR;
        }

        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));


        open_param->param.stream_in  = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_BLE;
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_UL;

        open_param->stream_out_param.ble.codec_type     = codec_type;
        open_param->stream_out_param.ble.context_type   = param->context_type;
        open_param->stream_out_param.ble.sampling_frequency =  param->sample_rate;
        open_param->stream_out_param.ble.nr_offload_flag = param->nr_offload_flag;
        open_param->stream_in_param.ble.is_lightmode   = bt_ble_is_lightmode;
        if (param->frame_duration == LE_AUDIO_interval_5) {
            open_param->stream_out_param.ble.frame_ms   = 5000;
        } else if (param->frame_duration == LE_AUDIO_interval_7_5) {
            open_param->stream_out_param.ble.frame_ms   = 7500;
        }
        else if (param->frame_duration == LE_AUDIO_interval_10) {
            open_param->stream_out_param.ble.frame_ms   = 10000;
        }
        if(open_param->stream_out_param.ble.codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD) {
            open_param->stream_out_param.ble.frame_ms   = 1000;
        }

        open_param->stream_out_param.ble.frame_payload_length = param->frame_payload_length;
        //open_param->stream_out_param.ble.bit_depth      = HAL_AUDIO_BITS_PER_SAMPLING_16; //:TODO: decided by BT
        open_param->stream_out_param.ble.decode_mode    = CODEC_VOICE_MODE; //:TODO: should always keep voice mode??
        open_param->stream_out_param.ble.p_share_info = hal_audio_query_ble_audio_ul_share_info();

        hal_audio_reset_share_info(open_param->stream_out_param.ble.p_share_info);
        BLE_LOG_MSGID_I("UL codec_type:%d, context_type:0x%x, sampling_rate:%d, channel_num:%d, channel_mode:%d, frame_duration:%d, frame_payload_length:%d, nr_offload_flag:%d, start_addr:0x%x\r\n", 9,
                        open_param->stream_out_param.ble.codec_type,
                        open_param->stream_out_param.ble.context_type,
                        open_param->stream_out_param.ble.sampling_frequency,
                        open_param->stream_out_param.ble.channel_num,
                        open_param->stream_out_param.ble.channel_mode,
                        open_param->stream_out_param.ble.frame_ms,
                        open_param->stream_out_param.ble.frame_payload_length,
                        open_param->stream_out_param.ble.nr_offload_flag,
                        open_param->stream_out_param.ble.p_share_info->start_addr);

        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param); //:TODO: should add new setting for BLE audio
        BLE_LOG_MSGID_I("in_device(0x%x), channel(%d), interface(%d)", 3, open_param->stream_in_param.afe.audio_device, open_param->stream_in_param.afe.stream_channel, open_param->stream_in_param.afe.audio_interface);

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_HFP_AVC, am_hfp_ndvc_callback); //for AVC update
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_VOICE_LEQ, am_voice_leq_callback); //for LEQ update
#endif
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    if (param->context_type == AUDIO_CONTENT_TYPE_WIRELESS_MIC)
    {
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE; //for wireless mic 24bit, put 0 to low 8bit
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1;// | HAL_AUDIO_MEM3;/* HAL_AUDIO_MEM3 to enable echo reference */
        open_param->stream_in_param.afe.sw_channels     = param->channel_num;
        open_param->stream_in_param.afe.irq_period      = open_param->stream_out_param.ble.frame_ms/1000;
        open_param->stream_in_param.afe.sampling_rate   = param->sample_rate;
        open_param->stream_in_param.afe.frame_size      = (open_param->stream_in_param.afe.irq_period * open_param->stream_in_param.afe.sampling_rate) / 1000;

        open_param->stream_out_param.ble.channel_num    = param->channel_num;

        uint8_t mic_num = ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_BLE_UL);
        if(mic_num > 2){
            configASSERT(0 && "[wireless_mic] HWIO config input channel > 2 ! error");
        }
        extern audio_channel_selection_t g_wireless_mic_channel_select;
        if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC_DEFAULE){
            if ((param->channel_num == 2) && (mic_num == 1)) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
            } else {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_STEREO, false);
        }
        } else if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC1){
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
        } else if(g_wireless_mic_channel_select == AUDIO_CHANNEL_SELECTION_MIC2){
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_R, false);
        }

        if((param->channel_num == 2) && (mic_num == 2)) {

        }
        else if ((param->channel_num == 2) && (mic_num == 1))
        {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 3, (uint32_t)AUDIO_DSP_CHANNEL_SELECTION_BOTH_L, false);
        }
        else if ((param->channel_num == 1) && (mic_num == 1))
        {

        }
        else if ((param->channel_num == 1) && (mic_num == 2))
        {

        }
        else {
            BLE_LOG_MSGID_E("[wireless_mic]not support bt channel = %d, mic channel = %d", 2, param->channel_num, mic_num);
            assert(0);
        }
        BLE_LOG_MSGID_I("[wireless_mic]bt channel = %d, mic channel = %d", 2,
                        param->channel_num, mic_num);
    }
    else
#endif
    {
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE; //:TODO: always keep 16bit for audio??
#ifdef AIR_BT_ULL_FB_ENABLE
        if ((param->context_type == AUDIO_CONTENT_TYPE_ULL_BLE) && (param->sample_rate == 48000)) {
            open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
        }
#endif
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;/* HAL_AUDIO_MEM3 to enable echo reference */
        open_param->stream_in_param.afe.sw_channels     = param->channel_num;

#ifdef BLE_CALL_APPLY_EC_NR_EQ
        /*
         * if EC NR feature is enabled in call scenario
         * sample rate and frame interval must be 16KHz/ 15ms due to algorithm limitation
         * */
        if (param->nr_offload_flag == true) {
            open_param->stream_in_param.afe.irq_period      = 5; /* Only EC with 5ms is enabled in headset side */
        } else {
            #ifdef AIR_BT_ULL_FB_ENABLE
            if ((param->context_type == AUDIO_CONTENT_TYPE_ULL_BLE) && (param->sample_rate == 48000)) {
                open_param->stream_in_param.afe.irq_period      = 5; /* Only EC with 5ms is enabled in headset side */
                BLE_LOG_MSGID_W("ULL FullBand config irq period = %d", 1, open_param->stream_in_param.afe.irq_period);
            } else {
                open_param->stream_in_param.afe.irq_period      = 15; /* EC + NR with 15ms is enabled in headset side */
            }
            #else
            open_param->stream_in_param.afe.irq_period      = 15; /* EC + NR with 15ms is enabled in headset side */
            #endif
        }

        if (param->context_type == AUDIO_CONTENT_TYPE_ULL_BLE) {
            open_param->stream_in_param.afe.sampling_rate   = param->sample_rate;
        }
        else
        {
            #ifdef AIR_BT_BLE_SWB_ENABLE
            open_param->stream_in_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ);
            #else
            open_param->stream_in_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_16KHZ);
            #endif
        }
        open_param->stream_in_param.afe.frame_size = (open_param->stream_in_param.afe.irq_period * open_param->stream_in_param.afe.sampling_rate) / 1000;

#else
        open_param->stream_in_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(param->ul_sample_rate);
        if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3) {
            if (param->sample_rate == 16000) {
                open_param->stream_in_param.afe.irq_period     = 10;
                open_param->stream_in_param.afe.frame_size     = 160;
            } else if (param->sample_rate == 24000) {
                open_param->stream_in_param.afe.irq_period     = 10;
                open_param->stream_in_param.afe.frame_size     = 240;
            } else {
                open_param->stream_in_param.afe.irq_period     = 10;
                open_param->stream_in_param.afe.frame_size     = 320;
                /*
                 *  following parameter is for 15ms interval with 32K sampling rate.
                 *  up sampling is not needed but SW buffer mechanism should be applied.
                 */
                open_param->stream_in_param.afe.irq_period     = 15;
                open_param->stream_in_param.afe.frame_size     = 480;
            }
        }
#endif
    }

#if defined(AIR_UL_FIX_SAMPLING_RATE_48K)
    BLE_LOG_MSGID_I("UL AFE fix 48K", 0);
    if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_16KHZ)){
        open_param->stream_in_param.afe.frame_size = 3 * open_param->stream_in_param.afe.frame_size;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_24KHZ)){
        open_param->stream_in_param.afe.frame_size = 2 * open_param->stream_in_param.afe.frame_size;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ)){
        open_param->stream_in_param.afe.frame_size = (3 * open_param->stream_in_param.afe.frame_size) / 2;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ)){
        //open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.frame_size;
    } else {
        configASSERT(0 && "[wireless_mic] unsupported sample rate");
    }
    open_param->stream_in_param.afe.sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ);
#elif defined(AIR_UL_FIX_SAMPLING_RATE_32K)
    BLE_LOG_MSGID_I("UL AFE fix 32K", 0);
    if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_16KHZ)){
        open_param->stream_in_param.afe.frame_size = 2 * open_param->stream_in_param.afe.frame_size;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_24KHZ)){
        open_param->stream_in_param.afe.frame_size = (3 * open_param->stream_in_param.afe.frame_size) / 2;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ)){
        //open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.frame_size;
    } else if (open_param->stream_in_param.afe.sampling_rate == hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_48KHZ)){
        open_param->stream_in_param.afe.frame_size = (2 * open_param->stream_in_param.afe.frame_size) / 3;
    } else {
        configASSERT(0 && "[wireless_mic] unsupported sample rate");
    }

    open_param->stream_in_param.afe.sampling_rate = hal_audio_sampling_rate_enum_to_value(HAL_AUDIO_SAMPLING_RATE_32KHZ);
#endif
    open_param->stream_in_param.afe.frame_number    = 4; /* AFE buffer can cache 6 frame */
    open_param->stream_in_param.afe.hw_gain         = false; /* uplink use SW gain only */

#ifdef AIR_UL_FIX_RESOLUTION_32BIT
    open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
    open_param->stream_in_param.afe.frame_number    = 2;
#endif

    if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
#ifdef AIR_HWSRC_RX_TRACKING_ENABLE
        open_param->stream_in_param.afe.stream_out_sampling_rate   = open_param->stream_in_param.afe.sampling_rate;

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
        open_param->stream_in_param.afe.sampling_rate   = 96000;
#else
        open_param->stream_in_param.afe.sampling_rate   = 48000;
#endif
#endif
#ifdef AIR_DUAL_CHIP_I2S_ENABLE
        //remove HAL_AUDIO_MEM3 to enable echo reference from i2s slave;
        open_param->stream_in_param.afe.memory          &= ~HAL_AUDIO_MEM3;
#endif
    }

    open_param->stream_in_param.afe.clkskew_mode    = CLK_SKEW_V1;
    BLE_LOG_MSGID_I("open_param->stream_in_param.afe.clkskew_mode:%d", 1, open_param->stream_in_param.afe.clkskew_mode);

    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL, open_param, true);
    p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL);
    if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_OPEN, AUDIO_DSP_CODEC_TYPE_LC3, (uint32_t)p_param_share, true);
    }
    else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_LC3PULS) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_OPEN, AUDIO_DSP_CODEC_TYPE_LC3PLUS, (uint32_t)p_param_share, true);
    } else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_VENDOR) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_OPEN, AUDIO_DSP_CODEC_TYPE_OPUS, (uint32_t)p_param_share, true);
    } else if (codec_type == BT_CODEC_TYPE_LE_AUDIO_ULD) {
        #ifdef AIR_AUDIO_ULD_CODEC_ENABLE
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_OPEN, AUDIO_DSP_CODEC_TYPE_ULD, (uint32_t)p_param_share, true);
        #endif
    }
    BLE_LOG_MSGID_I("open UL", 0);

    vPortFree(open_param);
    return BT_CODEC_MEDIA_STATUS_OK;
}

bt_media_handle_t *bt_codec_le_audio_open(bt_codec_le_audio_callback_t callback, const bt_codec_le_audio_param_t *param)
{
    if (param == NULL) {
        return NULL;
    }

    bt_media_handle_t *handle = NULL;
    bt_ble_codec_internal_handle_t *internal_handle = NULL;

    BLE_LOG_MSGID_I("Open codec +++, codec type:%d\n", 1, param->dl_param.codec);
    if (param->channel_mode != CHANNEL_MODE_UL_ONLY) {
        if (bt_ble_codec_dl_internal_handle == NULL)
        {
    internal_handle = (bt_ble_codec_internal_handle_t *)pvPortMalloc(sizeof(bt_ble_codec_internal_handle_t));
    if (internal_handle == NULL) {
        return NULL;
    }
    memset(internal_handle, 0, sizeof(bt_ble_codec_internal_handle_t));
    handle = &internal_handle->handle;
            internal_handle->codec_info = *(bt_codec_le_audio_param_t *)param;
            internal_handle->codec_info.channel_mode = param->channel_mode;
    handle->play    = bt_ble_play;
    handle->stop    = bt_ble_stop;
    handle->trigger_mic = bt_trigger_mic;
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
    audio_nvdm_update_status(AUDIO_NVDM_USER_BLE, AUDIO_NVDM_STATUS_CHANGE);
#endif
        }
        if (bt_ble_dl_internal_open(&(param->dl_param)) == BT_CODEC_MEDIA_STATUS_ERROR) {
            BLE_LOG_MSGID_I("bt ble dl internal open fail\r\n", 0);
            if (bt_ble_codec_dl_internal_handle == NULL)
            {vPortFree(internal_handle);}
        return NULL;
    }

        if(handle)
        {
            handle->state = BT_CODEC_STATE_READY;
            handle->handler = callback;
        }

        bt_ble_codec_dl_internal_handle = internal_handle;
        bt_ble_codec_dl_is_running = true;
    }

    if (param->channel_mode != CHANNEL_MODE_DL_ONLY) {
        if (bt_ble_codec_ul_internal_handle == NULL)
        {
            internal_handle = (bt_ble_codec_internal_handle_t *)pvPortMalloc(sizeof(bt_ble_codec_internal_handle_t));
            if (internal_handle == NULL) {
                return NULL;
            }
            memset(internal_handle, 0, sizeof(bt_ble_codec_internal_handle_t));
            handle = &internal_handle->handle;
            internal_handle->codec_info = *(bt_codec_le_audio_param_t *)param;
            internal_handle->codec_info.channel_mode = param->channel_mode;
            handle->play    = bt_ble_play;
            handle->stop    = bt_ble_stop;
            handle->trigger_mic = bt_trigger_mic;
        }
        if (bt_ble_ul_internal_open(&(param->ul_param)) == BT_CODEC_MEDIA_STATUS_ERROR) {
            BLE_LOG_MSGID_I("bt ble dl internal open fail\r\n", 0);
            if (bt_ble_codec_ul_internal_handle == NULL)
            {vPortFree(internal_handle);}
            return NULL;
        }

        if(handle)
        {
            handle->state = BT_CODEC_STATE_READY;
            handle->handler = callback;
        }

        bt_ble_codec_ul_internal_handle = internal_handle;
        bt_ble_codec_ul_is_running = true;
    }



    BLE_LOG_MSGID_I("Open codec ---\r\n", 0);

    return handle;
}

bt_codec_media_status_t bt_codec_le_audio_close(bt_media_handle_t *handle,uint16_t close_type)
{
    bt_ble_codec_internal_handle_t *p_info;
    if ((close_type != BT_BLE_MODE_TX_ONLY)&&(bt_ble_codec_dl_internal_handle != NULL))
    {
        p_info = bt_ble_codec_dl_internal_handle;
    }
    else
    {
        p_info = bt_ble_codec_ul_internal_handle;
    }
    BLE_LOG_MSGID_I("close codec type %d , channel_mode %d +++\r\n", 2,close_type,p_info->codec_info.channel_mode);
    if ((handle == NULL) || (handle->state != BT_CODEC_STATE_STOP)) {
        return BT_CODEC_MEDIA_STATUS_ERROR;
    }

    if (p_info->codec_info.channel_mode != CHANNEL_MODE_DL_ONLY) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_CLOSE, 0, 0, true);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL, NULL, false);
        if(bt_ble_codec_ul_internal_handle)
        {
            bt_ble_codec_ul_internal_handle->handle.state = BT_CODEC_STATE_IDLE;
            vPortFree(bt_ble_codec_ul_internal_handle);
        }

        bt_ble_codec_ul_internal_handle = NULL;
        bt_ble_codec_ul_is_running = false;
    }
    if (p_info->codec_info.channel_mode != CHANNEL_MODE_UL_ONLY) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_CLOSE, 0, 0, true);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL, NULL, false);
        if(bt_ble_codec_dl_internal_handle)
        {
            bt_ble_codec_dl_internal_handle->handle.state = BT_CODEC_STATE_IDLE;
            vPortFree(bt_ble_codec_dl_internal_handle);
        }

        bt_ble_codec_dl_internal_handle = NULL;
        bt_ble_codec_dl_is_running = false;
    }

    BLE_LOG_MSGID_I("close codec ---\r\n", 0);

    return BT_CODEC_MEDIA_STATUS_OK;
}

















/*********************************************
 *********For BLE source side code flow**************
 *********************************************/

#ifdef MTK_LINEIN_PLAYBACK_ENABLE
#include "linein_playback.h"
#endif

#define TASK_LOG_MSGID_CTRL(fmt, arg...)  LOG_MSGID_I(BT_BLE_CODEC, "[BLE CODEC]: "fmt,##arg)

typedef struct {
    uint8_t packet_length;
    uint8_t packet_number_per_interrupt;
    bool two_bytes_dummy_between_packet_flag;
} bt_ble_packet_info_t;

/*******************************************************************************\
| LPK_UL_TO_HP feature                                                                    |
\*******************************************************************************/
//#define LBK_UL_TO_HP_ENABLED
#if defined LBK_UL_TO_HP_ENABLED
uint16_t audio_pcm2way_hp_buffer[320] = {0};
#endif
/*******************************************************************************\
| ROM Tables                                                                    |
\*******************************************************************************/





void bt_ble_source_playback(uint16_t num_of_sink, uint16_t frame_length, uint8_t in_sample_rate, uint8_t out_bitrate, uint16_t frame_interval_us)
{
    //in_sample_rate    : KHz
    //out_bitrate       : Kbps
    //frame_interval_us : microseconds

#ifdef MTK_LINEIN_PLAYBACK_ENABLE
    if (num_of_sink == 0xFFFF) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_PLAYBACK_DATA_INFO, 0xFFFF, 0, false);
        //TASK_LOG_MSGID_CTRL("[le audio MCU] toggle playback source between line_in/1K tone", 0);
    } else if (num_of_sink == 0x9999) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_PLAYBACK_DATA_INFO, 0x9999, 0, false);
        //TASK_LOG_MSGID_CTRL("[le audio MCU] trigger line-in irq start", 0);
    } else if (num_of_sink) {


        n9_dsp_share_info_t *p_ul1_share = hal_audio_query_ble_audio_ul_share_info();
        n9_dsp_share_info_t *p_ul2_share = hal_audio_query_ble_audio_dl_share_info();    // use dl share buffer to handle ul data for second sink device
        uint32_t dsp_param = frame_length | (in_sample_rate << 16) | (out_bitrate << 24);

        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_PLAYBACK_DATA_INFO, num_of_sink, dsp_param, false);

        if (num_of_sink >= 1) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_BUFFER_INFO, 1, (uint32_t)p_ul1_share, false);
        }
        if (num_of_sink >= 2) {
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_BUFFER_INFO, 2, (uint32_t)p_ul2_share, false);
        }

        //TASK_LOG_MSGID_CTRL("[le audio MCU] playback source ul1: %X ul2: %X", 2, p_ul1_share, p_ul2_share);


        hal_audio_sampling_rate_t fs_enum;

        fs_enum = (in_sample_rate == 48) ? HAL_AUDIO_SAMPLING_RATE_48KHZ : \
                  (in_sample_rate == 32) ? HAL_AUDIO_SAMPLING_RATE_32KHZ : \
                  (in_sample_rate == 16) ? HAL_AUDIO_SAMPLING_RATE_16KHZ : -1;

        if (fs_enum != -1) {
#ifdef AIR_LE_AUDIO_ENABLE
            uint32_t frame_sample_count = (in_sample_rate * frame_interval_us) / 1000;
            linein_playback_le_audio_open(fs_enum, fs_enum, frame_sample_count);
#endif
            linein_playback_start();
//            linein_playback_le_audio_start();
        }  else {
            //TASK_LOG_MSGID_CTRL("[le audio MCU] playback source sample rate not supported: %X ", 1, in_sample_rate);
        }

    } else {
        //TASK_LOG_MSGID_CTRL("[le audio MCU] playback source sink num == 0. Close linein transform", 0);
        linein_playback_stop();
        linein_playback_close();
    }
#endif

}

void bt_ble_source_loopback(void)
{
    n9_dsp_share_info_t *p_dl_share;
    n9_dsp_share_info_t *p_ul_share;

    p_dl_share = hal_audio_query_ble_audio_dl_share_info();
    p_ul_share = hal_audio_query_ble_audio_ul_share_info();

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_DL_BUFFER_INFO, 0x5A5A, (uint32_t)p_dl_share, false);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BLE_AUDIO_UL_BUFFER_INFO, 0x5A5A, (uint32_t)p_ul_share, false);
    //TASK_LOG_MSGID_CTRL("[le audio MCU] bt_ble_loopback source", 0);

#ifdef MTK_LINEIN_PLAYBACK_ENABLE
    linein_playback_stop();
    linein_playback_close();
#endif
}

void bt_ble_set_shared_memory_information(uint32_t base_address, volatile uint32_t *p_offset_r, volatile uint32_t *p_offset_w, volatile uint32_t *p_control)
{
    bt_ble_codec_internal_handle_t *p_info = bt_ble_codec_dl_internal_handle;
    bt_ble_shared_memory_information_t *p_mem_info = &p_info->mem_info;
    p_mem_info->base_address = base_address;
    p_mem_info->p_offset_r   = p_offset_r;
    p_mem_info->p_offset_w   = p_offset_w;
    p_mem_info->p_control    = p_control;
    return;
}

bool bt_ble_codec_query_is_running(void)
{
    return bt_ble_codec_dl_is_running;
}

uint32_t bt_ble_codec_query_sampling_rate(void)
{
    bt_ble_codec_internal_handle_t *p_info = bt_ble_codec_dl_internal_handle;
    return p_info->codec_info.dl_param.sample_rate;
}

hal_audio_channel_number_t bt_ble_codec_query_channel_number(void)
{
    return HAL_AUDIO_MONO;
}



extern audio_src_srv_priority_t g_bt_ull_le_am_priority;
extern audio_src_srv_handle_t g_audio_src_srv_handle[AUDIO_SRC_SRV_PSEUDO_DEVICE_NUM];
extern void audio_src_srv_transfer_state(audio_src_srv_state_t state);
extern audio_src_srv_handle_t *audio_src_srv_get_waiting_psedev(void);
extern void audio_src_srv_update_psedev_state(audio_src_srv_handle_t *handle, audio_src_srv_state_t state);
void bt_ull_le_set_audio_manager_priority(audio_src_srv_priority_t priority)
{
    audio_src_srv_handle_t *device = (audio_src_srv_handle_t *)audio_src_srv_get_runing_pseudo_device();
    if ((device) && (device->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE)) {
        //device->priority = priority;
        if (device->priority > priority) {
            device->priority = priority;
            audio_src_srv_handle_t* waiting_psedev = audio_src_srv_get_waiting_psedev();
            if (waiting_psedev) {
                if (waiting_psedev->priority > priority) {
                audio_src_srv_report("[ULL LE] DL setup priority trigger from waiting list start(high to low)", 0);
                audio_src_srv_update_psedev_state(device, AUDIO_SRC_SRV_STATE_PREPARE_STOP);
                audio_src_srv_transfer_state(AUDIO_SRC_SRV_STATE_PREPARE_STOP);
                device->suspend(device, waiting_psedev);
                audio_src_srv_report("[ULL LE] DL setup priority trigger from waiting list end(high to low)", 0);
                } else {
                    audio_src_srv_report("[ULL LE] DL waiting device(%d) priority is smaller than current device %d < %d", 3, waiting_psedev->type, waiting_psedev->priority, priority);
                }
            }
        } else {
            device->priority = priority;
        }

        audio_src_srv_report("[ULL LE] DL setup priority from %d to %d, current running %d", 3, g_bt_ull_le_am_priority,priority, device->type);
    } else if (device) {
        audio_src_srv_handle_t *waiting_list = g_audio_src_srv_handle;
        audio_src_srv_handle_t *ull_le_dl_psedev = NULL;
        int32_t i = 0;
        for (i = 0; i < AUDIO_SRC_SRV_PSEUDO_DEVICE_NUM; ++i) {
            if (waiting_list[i].type == AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE) {
                ull_le_dl_psedev = &waiting_list[i];
                break;
            }
        }
        if (ull_le_dl_psedev) {
            ull_le_dl_psedev->priority = priority;////change priority
            if (priority > device->priority) {
                /* change psedev state with PREPARE_STOP */
                if(ull_le_dl_psedev->flag & AUDIO_SRC_SRV_FLAG_WAITING) {
                    audio_src_srv_report("[ULL LE] DL setup priority trigger from waiting list start", 0);
                    audio_src_srv_update_psedev_state(device, AUDIO_SRC_SRV_STATE_PREPARE_STOP);
                    audio_src_srv_transfer_state(AUDIO_SRC_SRV_STATE_PREPARE_STOP);
                    /* notify interrupt */
                    device->suspend(device, ull_le_dl_psedev);
                    audio_src_srv_report("[ULL LE] DL setup priority trigger from waiting list end", 0);
                }
            } else {
                audio_src_srv_report("[ULL LE] ULL DL priority is smaller than current device(%d) %d < %d", 3, device->type, priority, device->priority);
            }
        } else {
            uint8_t i = 0;
            for (i = 0; i < AUDIO_SRC_SRV_PSEUDO_DEVICE_NUM; ++i) {
                if ((g_audio_src_srv_handle[i].flag & AUDIO_SRC_SRV_FLAG_USED) && g_audio_src_srv_handle[i].type == AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE) {
                    g_audio_src_srv_handle[i].priority = priority;
                    audio_src_srv_report("[ULL LE] DL setup priority but bot in waiting list", 0);
                    break;
                }
            }

        }
        audio_src_srv_report("[ULL LE] DL setup priority from %d to %d, current running %d", 3, g_bt_ull_le_am_priority,priority, device->type);
    } else {
        audio_src_srv_report("[ULL LE] running device is NULL, DL setup priority", 0);
        int32_t i = 0;
        for (i = 0; i < AUDIO_SRC_SRV_PSEUDO_DEVICE_NUM; ++i) {
            if ((g_audio_src_srv_handle[i].flag & AUDIO_SRC_SRV_FLAG_USED) && g_audio_src_srv_handle[i].type == AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE) {
                g_audio_src_srv_handle[i].priority = priority;
                audio_src_srv_report("[ULL LE] running device is NULL, DL set up priority: %d", 1, priority);
                break;
            }
        }
    }

    audio_src_srv_report("[ULL LE] DL setup priority from %d to %d", 2, g_bt_ull_le_am_priority,priority);
    g_bt_ull_le_am_priority = priority;
}


#endif

