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

/* Includes ------------------------------------------------------------------*/

#include "syslog.h"

#include "bt_sink_srv_ami.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_internal.h"

#include "audio_nvdm_common.h"
#include "audio_nvdm_coef.h"
#include "nvkey_dspalg.h"
#include "exception_handler.h"

#include "hal_audio.h"
#include "record_playback.h"
#include "hal_resource_assignment.h"
#include "hal_audio_message_struct_common.h"
extern bool g_record_airdump;
extern uint16_t g_stream_in_sample_rate;
extern uint16_t g_stream_in_code_type;//modify for opus
extern uint16_t g_wwe_mode;
extern encoder_bitrate_t g_bit_rate;

/**
  * @ Start the recording of audio stream
  */
hal_audio_status_t hal_audio_start_stream_in(hal_audio_active_type_t active_type)
{
    //ToDo: limit the scope -- treat it as recording
    //audio_dsp_playback_info_t temp_param;
    void *p_param_share;
    bool is_running;
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_RECORD;
    audio_scenario_type_t           audio_scenario_type;

    // Open playback
    mcu2dsp_open_param_t *open_param = NULL;
    open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    if (open_param == NULL) {
        AUDIO_ASSERT(0 && "[hal_audio_start_stream_in] malloc open_para fail!");
    } else {
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

        // Collect parameters
        open_param->param.stream_in  = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_RECORD;
        if(g_wwe_mode != WWE_MODE_NONE) {
            audio_scenario_type = AUDIO_SCENARIO_TYPE_WWE;
        } else {
            audio_scenario_type = AUDIO_SCENARIO_TYPE_RECORD;
        }
        open_param->audio_scenario_type = audio_scenario_type;

#ifdef AIR_AMA_HOTWORD_DURING_CALL_ENABLE
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM_SUB;
#else
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1;
#endif

        if (g_wwe_mode != WWE_MODE_NONE) {
            /*WWE*/
            hal_audio_get_stream_in_setting_config(AU_DSP_VAD_PHASE1, &open_param->stream_in_param);
        } else {
            /*record*/
            hal_audio_get_stream_in_setting_config(AU_DSP_RECORD, &open_param->stream_in_param);
        }

#if defined (AIR_HFP_DNN_PATH_ENABLE) || defined (AIR_UL_FIX_RESOLUTION_32BIT)
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
#else
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
#endif

#ifdef AIR_UL_FIX_SAMPLING_RATE_48K
        open_param->stream_in_param.afe.sampling_rate   = 48000;
#elif defined(AIR_UL_FIX_SAMPLING_RATE_32K)
        open_param->stream_in_param.afe.sampling_rate   = 32000;
#else
        open_param->stream_in_param.afe.sampling_rate   = g_stream_in_sample_rate;
#endif
        open_param->stream_in_param.afe.irq_period      = 10;
        //open_param.stream_in_param.afe.frame_size      = 256; // Warning: currently fixed @ 480 in DSP
#if(MTK_RECORD_INTERLEAVE_ENABLE)
        if ((open_param->stream_in_param.afe.audio_device != 0) && (open_param->stream_in_param.afe.audio_device1 != 0)) {
            open_param->stream_in_param.afe.frame_size      = 128;
        } else {
            open_param->stream_in_param.afe.frame_size      = 256;
        }
#else
        open_param->stream_in_param.afe.frame_size      = 160;
#endif

#ifdef AIR_UL_FIX_RESOLUTION_32BIT
        open_param->stream_in_param.afe.frame_number    = 2;
#else
        open_param->stream_in_param.afe.frame_number    = 4;
#endif
        open_param->stream_in_param.afe.hw_gain         = false;

        open_param->stream_out_param.record.p_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);
        open_param->stream_out_param.record.frames_per_message = 4; // DSP triggers CCNI message after collecting this value of frames
        open_param->stream_out_param.record.bitrate = g_bit_rate;
#if(MTK_RECORD_INTERLEAVE_ENABLE)
        if ((open_param->stream_in_param.afe.audio_device != 0) && (open_param->stream_in_param.afe.audio_device1 != 0)) {
            open_param->stream_out_param.record.interleave = true;
        } else {
            open_param->stream_out_param.record.interleave = false;
        }
#else
        open_param->stream_out_param.record.interleave = false;
#endif

        if ((open_param->stream_in_param.afe.sampling_rate == 16000) || (open_param->stream_in_param.afe.sampling_rate == 32000)
            || (open_param->stream_in_param.afe.sampling_rate == 48000) || (open_param->stream_in_param.afe.sampling_rate == 96000)) {
            if (open_param->stream_in_param.afe.sampling_rate == 16000) {
                open_param->stream_in_param.afe.frame_size = 160;
            } else if (open_param->stream_in_param.afe.sampling_rate == 32000) {
                open_param->stream_in_param.afe.frame_size = 320;
            } else if (open_param->stream_in_param.afe.sampling_rate == 48000) {
                open_param->stream_in_param.afe.frame_size = 480;
            } else if (open_param->stream_in_param.afe.sampling_rate == 96000) {
                open_param->stream_in_param.afe.frame_size = 960;
            }

            log_hal_msgid_info("[RECORD]sampling_rate: %u, frame_size: %u", 2, open_param->stream_in_param.afe.sampling_rate, open_param->stream_in_param.afe.frame_size);
        } else {
            log_hal_msgid_error("[RECORD]Not support sampling_rate: %u", 1, open_param->stream_in_param.afe.sampling_rate);
        }

        hal_audio_reset_share_info(open_param->stream_out_param.record.p_share_info);

        p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), msg_type);

        is_running = hal_audio_status_query_running_flag(audio_scenario_type);
        if (is_running) {
            // Re-entry: don't allow multiple recording
            //log_hal_msgid_info("Re-entry\r\n", 0);
        } else {
            // Temp protect in APP level    //TODO record middleware.
            ami_hal_audio_status_set_running_flag(audio_scenario_type, open_param, true);
            if (g_wwe_mode == WWE_MODE_AMA) {
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_VOW, open_param, true);
            }
        }

        //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_OPEN, AUDIO_DSP_CODEC_TYPE_PCM, (uint32_t)p_param_share, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_OPEN, (g_wwe_mode << 12) | g_stream_in_code_type, (uint32_t)p_param_share, true); //modify for opus

        // Start playback
        mcu2dsp_start_param_t start_param;

        // Collect parameters
        start_param.param.stream_in     = STREAM_IN_AFE;
        start_param.param.stream_out    = STREAM_OUT_RECORD;

        start_param.stream_in_param.afe.aws_flag   =  false;

        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_open_param_t), msg_type);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_START, 0, (uint32_t)p_param_share, true);

        vPortFree(open_param);
    }
    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Stop the recording of audio stream
  */
void hal_audio_stop_stream_in(void)
{
    audio_scenario_type_t           audio_scenario_type;
    if(g_wwe_mode != WWE_MODE_NONE) {
        audio_scenario_type = AUDIO_SCENARIO_TYPE_WWE;
    } else {
        audio_scenario_type = AUDIO_SCENARIO_TYPE_RECORD;
    }

    //ToDo: limit the scope -- treat it as recording
    if (hal_audio_status_query_running_flag(audio_scenario_type)) {
    // Stop recording
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_CLOSE, 0, 0, true);

        // Temp protect in APP level    //TODO record middleware.
        ami_hal_audio_status_set_running_flag(audio_scenario_type, NULL, false);
        if (g_wwe_mode == WWE_MODE_AMA) {
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_VOW, NULL, false);
        }
    } else {
        log_hal_msgid_info("Recording was not existed.", 0);
    }
}

/**
  * @ Start audio recording
  * @ buffer: buffer pointer for the recorded data storing
  * @ size : number of audio data
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed.
  */
hal_audio_status_t hal_audio_read_stream_in(void *buffer, uint32_t size)
{
    //ToDo: limit the scope -- treat it as recording
    n9_dsp_share_info_t *p_info = hal_audio_query_record_share_info();
    uint32_t data_byte_count;
    hal_audio_status_t result = HAL_AUDIO_STATUS_OK;
    uint32_t i;
    uint8_t *p_dest_buf = (uint8_t *)buffer;
    bool is_notify;
    uint8_t *start_addr;

    // Check buffer
    if (buffer == NULL) {
        return HAL_AUDIO_STATUS_ERROR;
    }

    // Check data amount
    data_byte_count = hal_audio_buf_mgm_get_data_byte_count(p_info);
    if (size > data_byte_count) {
        return HAL_AUDIO_STATUS_ERROR;
    }

    start_addr = (uint8_t *)hal_memview_dsp0_to_cm4(p_info->start_addr);
    // When buffer is enough
    for (i = 0; (i < 2) && size; i++) {
        uint8_t *p_source_buf;
        uint32_t buf_size, segment;

        hal_audio_buf_mgm_get_data_buffer(p_info, &p_source_buf, &buf_size);
        if (size >= buf_size) {
            segment = buf_size;
        } else {
            segment = size;
        }
        p_source_buf = (uint8_t *)hal_memview_dsp0_to_cm4((uint32_t)p_source_buf);
        if((p_source_buf + segment - 1) > (start_addr + p_info->length - 1)){
            unsigned int len1, len2;
            len1 = start_addr + p_info->length - p_source_buf;
            len2 = segment - len1;
            memcpy(p_dest_buf, p_source_buf, len1);
            p_source_buf = (uint8_t *)start_addr;
            memcpy(p_dest_buf + len1, p_source_buf, len2);
        }else{
            memcpy(p_dest_buf, p_source_buf, segment);
        }
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
        if (g_record_airdump) {
            memcpy(p_dest_buf + segment, p_source_buf + p_info->length, segment);
        }
#endif
        hal_audio_buf_mgm_get_read_data_done(p_info, segment);
        p_dest_buf += segment;
        size -= segment;
    }

    // if (p_info->bBufferIsFull) {
    //     p_info->bBufferIsFull = 0;
    // }

    // Check status and notify DSP
    is_notify = hal_audio_status_query_notify_flag(AUDIO_MESSAGE_TYPE_RECORD);
    if (is_notify) {
        hal_audio_status_set_notify_flag(AUDIO_MESSAGE_TYPE_RECORD, false);
        hal_audio_dsp_controller_send_message(AUDIO_CCNI_MESSAGE_ACK(MSG_DSP2MCU_RECORD_DATA_NOTIFY), 0, 0, false);
    }

    return result;
}

/**
  * @ Query the data amount of input stream.
  * @ sample_count : number of data [in bytes]
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed
  */
hal_audio_status_t hal_audio_get_stream_in_sample_count(uint32_t *sample_count)
{
    //ToDo: limit the scope -- treat it as recording
    n9_dsp_share_info_t *p_info = hal_audio_query_record_share_info();

    *sample_count = hal_audio_buf_mgm_get_data_byte_count(p_info);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Register the callback of stream in.
  * @ callback : callback function
  * @ user_data : pointer of user data
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed
  */
hal_audio_status_t hal_audio_register_stream_in_callback(hal_audio_stream_in_callback_t callback, void *user_data)
{
    //ToDo: limit the scope -- treat it as recording

    hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_RECORD, callback, user_data);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Start the recording of audio stream for wearing condition detection
  */
hal_audio_status_t hal_audio_start_stream_in_leakage_compensation(void)
{
    mcu2dsp_open_param_t *open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    if (open_param == NULL) {
        return HAL_AUDIO_STATUS_ERROR;
    }
    void *p_param_share;
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_RECORD;
    bool is_running;

    memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

    open_param->param.stream_in  = STREAM_IN_AFE;
    open_param->param.stream_out = STREAM_OUT_RECORD;
    open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_RECORD;


#if defined(HAL_DVFS_MODULE_ENABLED)
#if AIR_BTA_IC_PREMIUM_G3
    hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
    log_hal_msgid_info("[RECORD_LC]speed is risen to 156M", 0);
#elif AIR_BTA_IC_PREMIUM_G2
    hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
    log_hal_msgid_info("[RECORD_LC]speed is risen to 208M", 0);
#endif
#endif

#if 1
    //AMIC mono R
//    open_param.stream_in_param.afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
    hal_audio_get_stream_in_setting_config(AU_DSP_ANC, &(open_param->stream_in_param));
    open_param->stream_in_param.afe.audio_device = open_param->stream_in_param.afe.audio_device1;
    open_param->stream_in_param.afe.audio_device1 = 0;
    open_param->stream_in_param.afe.audio_device2 = 0;
    open_param->stream_in_param.afe.audio_device3 = 0;
    open_param->stream_in_param.afe.audio_device4 = 0;
    open_param->stream_in_param.afe.audio_device5 = 0;
    open_param->stream_in_param.afe.audio_device6 = 0;
    open_param->stream_in_param.afe.audio_device7 = 0;

    open_param->stream_in_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
    open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;//HAL_AUDIO_MEM3 to enable echo referencr
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    open_param->stream_in_param.afe.audio_interface = open_param->stream_in_param.afe.audio_interface1;
    open_param->stream_in_param.afe.audio_interface1 = 0;
    open_param->stream_in_param.afe.audio_interface2 = 0;
    open_param->stream_in_param.afe.audio_interface3 = 0;
#endif
    open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
#else
    //setting from nvkey
    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param.stream_in_param.afe.audio_device, &open_param.stream_in_param.afe.stream_channel, &open_param.stream_in_param.afe.audio_interface);
    if (open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param.stream_in_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
    } else {
        open_param.stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
    }
#endif
    open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
    open_param->stream_in_param.afe.sampling_rate   = 16000;
    open_param->stream_in_param.afe.frame_size      = 240; // Warning: currently fixed @ 480 in DSP
    open_param->stream_in_param.afe.irq_period      = 0;
    log_hal_msgid_info("[RECORD_LC]hal_audio_start_stream_in_leakage_compensation, device:%d, audio_interface:%d, sampling_rate:%d, set frame size:%d\r\n", 4, open_param->stream_in_param.afe.audio_device, open_param->stream_in_param.afe.audio_interface, open_param->stream_in_param.afe.sampling_rate, open_param->stream_in_param.afe.frame_size);
    open_param->stream_in_param.afe.frame_number    = 4;
    open_param->stream_in_param.afe.hw_gain         = false;

    open_param->stream_out_param.record.p_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);
    open_param->stream_out_param.record.frames_per_message = 4; // DSP triggers CCNI message after collecting this value of frames
    open_param->stream_out_param.record.bitrate = g_bit_rate;
    hal_audio_reset_share_info(open_param->stream_out_param.record.p_share_info);

    p_param_share = hal_audio_dsp_controller_put_paramter( open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_RECORD);

    is_running = hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD);
    if (is_running) {
        // Re-entry: don't allow multiple recording
        //log_hal_msgid_info("Re-entry\r\n", 0);
    } else {
        // Temp protect in APP level    //TODO record middleware.
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_RECORD, open_param, true);
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_OPEN, g_stream_in_code_type, (uint32_t)p_param_share, true);
    vPortFree(open_param);

    // Start playback
    mcu2dsp_start_param_t *start_param = (mcu2dsp_start_param_t *)pvPortMalloc(sizeof(mcu2dsp_start_param_t));
    if (start_param == NULL) {
        return HAL_AUDIO_STATUS_ERROR;
    }

    // Collect parameters
    start_param->param.stream_in     = STREAM_IN_AFE;
    start_param->param.stream_out    = STREAM_OUT_RECORD;

    start_param->stream_in_param.afe.aws_flag   =  true;

    p_param_share = hal_audio_dsp_controller_put_paramter(start_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_RECORD);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_START, 0, (uint32_t)p_param_share, true);
    vPortFree(start_param);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Stop the recording of audio stream for wearing condition detection
  */
void hal_audio_stop_stream_in_leakage_compensation(void)
{
    //ToDo: limit the scope -- treat it as recording
    if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD)) {
        // Stop recording
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_CLOSE, 0, 0, true);

        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_RECORD, NULL, false);

#if defined(HAL_DVFS_MODULE_ENABLED)
#if AIR_BTA_IC_PREMIUM_G3
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
        log_hal_msgid_info("[RECORD_LC]speed is set back", 0);
#elif AIR_BTA_IC_PREMIUM_G2
        hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
        log_hal_msgid_info("[RECORD_LC]speed is risen to 208M", 0);
#endif
#endif
    } else {
        log_hal_msgid_info("Recording was not existed.", 0);
    }
}

#if defined(MTK_USER_TRIGGER_FF_ENABLE) & defined(MTK_USER_TRIGGER_ADAPTIVE_FF_V2)
/**
  * @ Start the recording of audio stream for User Triggered adaptive FF
  */
hal_audio_status_t hal_audio_start_stream_in_user_trigger_adaptive_ff(uint8_t mode)
{
    mcu2dsp_open_param_t open_param;
    void *p_param_share;
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_RECORD;
    bool is_running;

    memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
    open_param.param.stream_in  = STREAM_IN_AFE;
    open_param.param.stream_out = STREAM_OUT_RECORD;
    open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_RECORD;


#if defined(HAL_DVFS_MODULE_ENABLED)
    #ifdef AIR_BTA_IC_PREMIUM_G3
    hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
    log_hal_msgid_info("[user_trigger_ff]speed is risen to 156M", 0);
    #endif
    #ifdef AIR_BTA_IC_PREMIUM_G2
    hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
    log_hal_msgid_info("[user_trigger_ff]speed is risen to 208M", 0);
    #endif
#endif


    //AMIC mono R
//    open_param.stream_in_param.afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
    hal_audio_get_stream_in_setting_config(AU_DSP_ANC, &open_param.stream_in_param);
//    log_hal_msgid_info("[user_trigger_ff]hal_audio_get_stream_in_setting_config, ESCO device:%d, device1:%d, device2:%d, device3:%d, device4:%d, device5:%d, device6:%d, device7:%d", 8, open_param.stream_in_param.afe.audio_device, open_param.stream_in_param.afe.audio_device1,open_param.stream_in_param.afe.audio_device2, open_param.stream_in_param.afe.audio_device3, open_param.stream_in_param.afe.audio_device4, open_param.stream_in_param.afe.audio_device5, open_param.stream_in_param.afe.audio_device6, open_param.stream_in_param.afe.audio_device7);

    switch (mode) {
        case 2: {//SZ: FB mic + echo_ref
//            open_param.stream_in_param.afe.audio_device = open_param.stream_in_param.afe.audio_device1;
//            open_param.stream_in_param.afe.audio_device1 = open_param.stream_in_param.afe.audio_device2;
            open_param.stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;//HAL_AUDIO_MEM3 to enable echo referencr
            break;
        }
        case 1: //PZ: FF+FB mic
        case 0: {//PZ_FIR: FF+FB mic
//            open_param.stream_in_param.afe.audio_device = open_param.stream_in_param.afe.audio_device1;
//            open_param.stream_in_param.afe.audio_device1 = open_param.stream_in_param.afe.audio_device2;
            open_param.stream_in_param.afe.memory          = HAL_AUDIO_MEM1;
            break;
        }
    }
    open_param.stream_in_param.afe.audio_device2 = 0;
    open_param.stream_in_param.afe.audio_device3 = 0;
    open_param.stream_in_param.afe.audio_device4 = 0;
    open_param.stream_in_param.afe.audio_device5 = 0;
    open_param.stream_in_param.afe.audio_device6 = 0;
    open_param.stream_in_param.afe.audio_device7 = 0;

    open_param.stream_in_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
    open_param.stream_in_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
    open_param.stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;

    open_param.stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;

    switch (mode) {
        case 0: {
            open_param.stream_in_param.afe.sampling_rate   = 48000;
            open_param.stream_in_param.afe.irq_period      = 10;
            open_param.stream_in_param.afe.frame_size      = 480; // Warning: currently fixed @ 480 in DSP
            break;
        }
        case 1: {
            open_param.stream_in_param.afe.sampling_rate   = 48000;
            open_param.stream_in_param.afe.irq_period      = 10;
            open_param.stream_in_param.afe.frame_size      = 480; // Warning: currently fixed @ 480 in DSP
            break;
        }
        case 2: {
            open_param.stream_in_param.afe.sampling_rate   = 48000;
            open_param.stream_in_param.afe.irq_period      = 10;
            open_param.stream_in_param.afe.frame_size      = 480; // Warning: currently fixed @ 480 in DSP
            break;
        }
    }

    open_param.stream_in_param.afe.frame_number    = 2;
    open_param.stream_in_param.afe.hw_gain         = false;

    //disable iir filter
    open_param.stream_in_param.afe.iir_filter[0] = 0xF;
    open_param.stream_in_param.afe.iir_filter[1] = 0xF;
    open_param.stream_in_param.afe.iir_filter[2] = 0xF;

    open_param.stream_out_param.record.p_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);
    open_param.stream_out_param.record.frames_per_message = 2; // DSP triggers CCNI message after collecting this value of frames
    open_param.stream_out_param.record.bitrate = g_bit_rate;
    open_param.stream_out_param.record.interleave = false;//work round for report data
    log_hal_msgid_info("[user_trigger_ff]hal_audio_start_stream_in_user_trigger_adaptive_ff, mode:%d, device:0x%x, device1:0x%x, memory_path:0x%x, sampling_rate:%d, frame size:%d, frame_number:%d\r\n", 6, mode, open_param.stream_in_param.afe.audio_device, open_param.stream_in_param.afe.audio_device1, open_param.stream_in_param.afe.memory, open_param.stream_in_param.afe.sampling_rate, open_param.stream_in_param.afe.frame_size, open_param.stream_in_param.afe.frame_number);
    hal_audio_reset_share_info(open_param.stream_out_param.record.p_share_info);


    is_running = hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD);
    if (is_running) {
        // Re-entry: don't allow multiple recording
        //log_hal_msgid_info("Re-entry\r\n", 0);
    } else {
        // Temp protect in APP level    //TODO record middleware.
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_RECORD, &open_param, true);
    }
    p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_RECORD);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_OPEN, g_stream_in_code_type, (uint32_t)p_param_share, true);

    // Start playback
    mcu2dsp_start_param_t start_param;

    // Collect parameters
    start_param.param.stream_in     = STREAM_IN_AFE;
    start_param.param.stream_out    = STREAM_OUT_RECORD;

    start_param.stream_in_param.afe.aws_flag   =  true;

    p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_RECORD);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_START, 0, (uint32_t)p_param_share, true);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Stop the recording of audio stream for wearing condition detection
  */
void hal_audio_stop_stream_in_user_trigger_adaptive_ff(void)
{
    //ToDo: limit the scope -- treat it as recording
    if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD)) {
        // Stop recording
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_RECORD_CLOSE, 0, 0, true);

        // Temp protect in APP level    //TODO record middleware.
        ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_RECORD, NULL, false);

#if defined(HAL_DVFS_MODULE_ENABLED)
        #ifdef AIR_BTA_IC_PREMIUM_G3
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
        log_hal_msgid_info("[user_trigger_ff]speed is set back", 0);
        #endif
        #ifdef AIR_BTA_IC_PREMIUM_G2
        hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
        log_hal_msgid_info("[user_trigger_ff]speed is set back", 0);
        #endif
#endif

    } else {
        log_hal_msgid_info("Recording was not existed.", 0);
    }
}
#endif

