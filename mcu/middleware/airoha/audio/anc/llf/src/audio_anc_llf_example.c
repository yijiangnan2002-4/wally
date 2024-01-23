/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#include <stdio.h>
#include "syslog.h"

#include "audio_anc_llf_example.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
#include "bt_sink_srv_ami.h"
#include "hal_resource_assignment.h"


/* Private define ------------------------------------------------------------*/
#define LLF_SAMPLE_FRAME_SIZE    (50)
#define LLF_SAMPLE_FRAME_NUM     (6)
#define LLF_SAMPLE_BUFFER_LEN    (LLF_SAMPLE_FRAME_SIZE * LLF_SAMPLE_FRAME_NUM)
#define LLF_SAMPLE_FORMAT        (HAL_AUDIO_PCM_FORMAT_S32_LE)
#define AUDIO_ANC_CONTROL_LLF_FILTER_DEFAULT   (1)

#if 0
#define LOGE(fmt,arg...)   LOG_E(aud, "[LLF][SAMPLE] "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(aud, "[LLF][SAMPLE] "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(aud, "[LLF][SAMPLE] "fmt,##arg)
#else
#define LOGMSGIDE(fmt,arg...)   LOG_MSGID_E(aud, "[LLF][SAMPLE] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)   LOG_MSGID_W(aud, "[LLF][SAMPLE] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)   LOG_MSGID_I(aud, "[LLF][SAMPLE] "fmt,##arg)
#endif

/* Private typedef -----------------------------------------------------------*/
typedef uint8_t (*llf_handler) (bool enable);
typedef uint8_t (*llf_fade_out)   (void);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
U32 input_data_order[LLF_DATA_TYPE_NUM] = {
    LLF_DATA_TYPE_REAR_L,
    LLF_DATA_TYPE_INEAR_L,
    LLF_DATA_TYPE_TALK,
    LLF_DATA_TYPE_MUSIC_VOICE, // DL without LLF data
    LLF_DATA_TYPE_REF,         //DL with LLF data
    0xFFFFFFFF,//invalid value
    0xFFFFFFFF,//invalid value

};
/* Public variables ----------------------------------------------------------*/
extern uint8_t LLF_enable;
extern uint8_t g_anc_LLF_fadeout_check;

/* Private functions ---------------------------------------------------------*/
uint32_t *hal_audio_query_llf_share_info(U32 index);
extern void anc_set_LLF_handler_callback(llf_handler handler, llf_fade_out fade_out_function);

llf_status_t llf_sample_stream_handler(bool enable)
{
    U16 data16 = (LLF_TYPE_SAMPLE << 8) | LLF_SUB_MODE_SAMPLE;
    U32 channl_number = 0;
    llf_status_t llf_res = LLF_STATUS_SUCCESS;
    LOGMSGIDI("stream enable:%d", 1, enable);

    if (enable) {
        if (!ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM)) {
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, true);
        }
        llf_control_set_status(LLF_RUNNING_STATUS_RUNNING, LLF_TYPE_SAMPLE, 0, NULL);

        // Open framework
        mcu2dsp_open_param_t* open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
        llf_open_param_t* p_stream_in_param = &(open_param->stream_in_param.LLF);
        llf_open_param_t* p_stream_out_param = &(open_param->stream_out_param.LLF);
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

        // Collect parameters
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID;
        open_param->param.stream_in  = STREAM_IN_LLF;
        open_param->param.stream_out = STREAM_OUT_LLF;
        p_stream_in_param->frame_size = LLF_SAMPLE_FRAME_SIZE;
        p_stream_in_param->frame_number = LLF_SAMPLE_FRAME_NUM;
        p_stream_in_param->format = LLF_SAMPLE_FORMAT;
        p_stream_out_param->frame_size = LLF_SAMPLE_FRAME_SIZE;
        p_stream_out_param->frame_number = LLF_SAMPLE_FRAME_NUM;

        //input config
        mcu2dsp_open_stream_in_param_t* voice_in_device = (mcu2dsp_open_stream_in_param_t*)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, voice_in_device);
        mcu2dsp_open_stream_in_param_t* anc_in_device = (mcu2dsp_open_stream_in_param_t*)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_ANC, anc_in_device);

        for (uint32_t i = 0; i < LLF_DATA_TYPE_NUM; i++) {
            switch (input_data_order[i]) {
                case LLF_DATA_TYPE_REAR_L: {
                    p_stream_in_param->audio_device[LLF_DATA_TYPE_REAR_L] = anc_in_device->afe.audio_device;
                    p_stream_in_param->audio_interface[LLF_DATA_TYPE_REAR_L] = anc_in_device->afe.audio_interface;
                    p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_REAR_L] = true;
                    channl_number++;
                    break;
                }
                case LLF_DATA_TYPE_INEAR_L: {
                    p_stream_in_param->audio_device[LLF_DATA_TYPE_INEAR_L] = anc_in_device->afe.audio_device1;
                    p_stream_in_param->audio_interface[LLF_DATA_TYPE_INEAR_L] = anc_in_device->afe.audio_interface1;
                    p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_INEAR_L] = true;
                    channl_number++;
                    break;
                }
                case LLF_DATA_TYPE_TALK: {
                    p_stream_in_param->audio_device[LLF_DATA_TYPE_TALK] = voice_in_device->afe.audio_device;
                    p_stream_in_param->audio_interface[LLF_DATA_TYPE_TALK] = voice_in_device->afe.audio_interface;
                    p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_TALK] = true;
                    channl_number++;
                    break;
                }
                case LLF_DATA_TYPE_MUSIC_VOICE: {
                    p_stream_in_param->echo_reference[0] = true;
                    channl_number++;
                    break;
                }
                case LLF_DATA_TYPE_REF: {
                    p_stream_in_param->echo_reference[1] = true;
                    channl_number++;
                    break;
                }
                default:
                    break;
            }
        }
        memcpy(p_stream_in_param->in_data_order, input_data_order, LLF_DATA_TYPE_NUM * sizeof(U32));
        p_stream_in_param->channel_num = (U8)channl_number;
        p_stream_in_param->music_need_compensation = false; // true:  LLF need to compensate music data and mix it with LLF output

        p_stream_in_param->earbuds_ch = (ami_get_audio_channel() == AUDIO_CHANNEL_R) ? 2 : 1;
        p_stream_out_param->earbuds_ch = open_param->stream_in_param.LLF.earbuds_ch;


        p_stream_in_param->share_info.start_addr = (U32)hal_audio_query_llf_share_info(SHARE_BUFFER_LLF_INFO_ID_DSP);
        p_stream_in_param->share_info.length = SHARE_BUFFER_LLF_INFO_COUNT_DSP * 4;
        LOGMSGIDI("frame size(%d), frame number(%d), format(%d), channel(%d), earbuds_ch(%d), two_mic_exist(%d)", 6,
                                p_stream_in_param->frame_size,
                                p_stream_in_param->frame_number,
                                p_stream_in_param->format,
                                p_stream_in_param->channel_num,
                                p_stream_in_param->earbuds_ch,
                                p_stream_in_param->music_need_compensation);

        LOGMSGIDI("device(%d %d %d %d %d), interface(%d %d %d %d %d), enable(%d %d %d %d %d), echo_ref(%d %d)", 17,
                        p_stream_in_param->audio_device[0],
                        p_stream_in_param->audio_device[1],
                        p_stream_in_param->audio_device[2],
                        p_stream_in_param->audio_device[3],
                        p_stream_in_param->audio_device[4],
                        p_stream_in_param->audio_interface[0],
                        p_stream_in_param->audio_interface[1],
                        p_stream_in_param->audio_interface[2],
                        p_stream_in_param->audio_interface[3],
                        p_stream_in_param->audio_interface[4],
                        p_stream_in_param->audio_device_enable[0],
                        p_stream_in_param->audio_device_enable[1],
                        p_stream_in_param->audio_device_enable[2],
                        p_stream_in_param->audio_device_enable[3],
                        p_stream_in_param->audio_device_enable[4],
                        p_stream_in_param->echo_reference[0],
                        p_stream_in_param->echo_reference[1]);
        void *p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_ANC);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_OPEN, data16, (uint32_t)p_param_share, true);

        // Start framework
        mcu2dsp_start_param_t *start_param = (mcu2dsp_start_param_t *)pvPortMalloc(sizeof(mcu2dsp_start_param_t));
        memset(start_param, 0, sizeof(mcu2dsp_start_param_t));
        // Collect parameters
        start_param->param.stream_in     = STREAM_IN_LLF;
        start_param->param.stream_out    = STREAM_OUT_LLF;
        p_param_share = hal_audio_dsp_controller_put_paramter(start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_ANC);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_START, 0, (uint32_t)p_param_share, true);

        LLF_enable = 1;
        vPortFree(voice_in_device);
        vPortFree(anc_in_device);
        vPortFree(open_param);
        vPortFree(start_param);
    } else {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_CLOSE, 0, 0, true);

        LLF_enable = 0;
        if (!g_anc_LLF_fadeout_check) {
            g_anc_LLF_fadeout_check = 1;
        }
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, false);
        llf_control_set_status(LLF_RUNNING_STATUS_CLOSE, LLF_TYPE_SAMPLE, 0, NULL);
        llf_callback_service(LLF_TYPE_SAMPLE, LLF_CONTROL_EVENT_OFF, LLF_STATUS_SUCCESS);

    }
    return llf_res;
}

llf_status_t audio_anc_llf_sample_fade_out()
{
    //fade-out before close
    return LLF_STATUS_SUCCESS;
}

void audio_anc_llf_get_mic_input_path(U32 *sel)
{
    if (sel) {
        *sel = 0;
        for (uint32_t i = 0; i < LLF_DATA_TYPE_NUM; i++) {
            if (input_data_order[i] == LLF_DATA_TYPE_REAR_L) {
                *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FF_1;

            } else if (input_data_order[i] == LLF_DATA_TYPE_INEAR_L) {
                *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FB_1;

            } else if (input_data_order[i] == LLF_DATA_TYPE_TALK) {
                *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_TALK;
            }
        }
        LOGMSGIDI("get mic input: 0x%x", 1, *sel);
    }
}

/* Public functions ----------------------------------------------------------*/
llf_status_t audio_llf_sample_init(void)
{
    llf_control_entry_t entry = {
        .open_entry = audio_llf_sample_open,
        .close_entry = audio_llf_sample_close,
    };
    llf_status_t res = llf_control_register_entry(LLF_TYPE_SAMPLE, &entry);

    anc_set_LLF_handler_callback((llf_handler)llf_sample_stream_handler, (llf_fade_out)audio_anc_llf_sample_fade_out);

    for (uint32_t i = 0; i < LLF_DATA_TYPE_NUM; i++) {
    switch (input_data_order[i]) {
        case LLF_DATA_TYPE_REAR_L: {
            break;
        }
        case LLF_DATA_TYPE_INEAR_L: {
            break;
        }
        case LLF_DATA_TYPE_TALK: {
            break;
        }
        default:
            break;
    }
}
    return res;
}

llf_status_t audio_llf_sample_open(void)
{
    LOGMSGIDI("open", 0);
    audio_anc_control_result_t         control_ret;
    audio_anc_control_filter_id_t      target_filter_id;
    audio_anc_control_type_t           target_anc_type;
    audio_anc_control_misc_t           local_misc = {0};
    U32 llf_mic_input = 0;

    audio_anc_llf_get_mic_input_path(&llf_mic_input);
    local_misc.type_mask_param.ANC_path_mask = 0;
                                               //default none
                                               //hybrid ANC:
                                               //AUDIO_ANC_CONTROL_RAMP_FF_L |
                                               //AUDIO_ANC_CONTROL_RAMP_FB_L;

    target_anc_type      = AUDIO_ANC_CONTROL_TYPE_LLF | llf_mic_input;

    target_filter_id     = AUDIO_ANC_CONTROL_LLF_FILTER_DEFAULT; //1~4

    control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);

    return (control_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? LLF_STATUS_SUCCESS : LLF_STATUS_FAIL;
}

llf_status_t audio_llf_sample_close(void)
{
    LOGMSGIDI("close", 0);

    return (audio_anc_control_disable(NULL) == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? LLF_STATUS_SUCCESS : LLF_STATUS_FAIL;
}


