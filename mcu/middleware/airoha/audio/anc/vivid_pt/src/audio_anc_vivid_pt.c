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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "syslog.h"

#include "audio_anc_vivid_pt_internal.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
//#include "anc_control.h"
#include "bt_sink_srv_ami.h"
#include "hal_resource_assignment.h"
#include "peq_setting.h"

/* Private define ------------------------------------------------------------*/
#define VIVID_PT_FRAME_SIZE    (10)
#define VIVID_PT_FRAME_NUM     (6)
#define VIVID_PT_BUFFER_LEN    (VIVID_PT_FRAME_SIZE * VIVID_PT_FRAME_NUM)
#define VIVID_PT_FORMAT        (HAL_AUDIO_PCM_FORMAT_S32_LE)
#define VIVID_PT_LIMITER_LEN   (136)

#ifdef AUDIO_USE_MSGID_LOG
#define LOGMSGIDE(fmt,arg...)               LOG_MSGID_E(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_MSGID_W(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_MSGID_I(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_MSGID_D(aud,"[VIVID_PT] "fmt,##arg)
#else
#define LOGMSGIDE(fmt,arg...)               LOG_E(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_W(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_I(aud,"[VIVID_PT] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_D(aud,"[VIVID_PT] "fmt,##arg)
#endif

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    U8 framework_enable;
    U8 afc_enable;
    U8 nr_enable;
    U8 bypass_enable;
    uint32_t *share_addr;
} vivid_pt_ctrl_t;

typedef enum {
    VIVID_PT_RUNTIME_CONFIG_EVENT_ENABLE_DEVICE       = 0,
    VIVID_PT_RUNTIME_CONFIG_EVENT_AFC_SWTICH          = 1,
    VIVID_PT_RUNTIME_CONFIG_EVENT_NR_SWTICH           = 2,
    VIVID_PT_RUNTIME_CONFIG_EVENT_BYPASS_SWTICH       = 3,
    VIVID_PT_RUNTIME_CONFIG_EVENT_NUM,
} vivid_pt_runtime_config_event_t;

typedef uint8_t (*sw_vivid_pt_handler) (bool enable);
typedef uint8_t (*sw_vivid_fade_out)   (void);


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
vivid_pt_ctrl_t vivid_pt_ctrl;

U32 input_data_order_vivid[LLF_DATA_TYPE_NUM] = {
    LLF_DATA_TYPE_REAR_L,
    //LLF_DATA_TYPE_INEAR_L,
    //LLF_DATA_TYPE_TALK,
    //LLF_DATA_TYPE_MUSIC_VOICE, // DL without LLF data
    //LLF_DATA_TYPE_REF,         //DL with LLF data
    0xFFFFFFFF,//invalid value
    0xFFFFFFFF,//invalid value

    0xFFFFFFFF,//invalid value
    0xFFFFFFFF,//invalid value
    0xFFFFFFFF,//invalid value
    0xFFFFFFFF,//invalid value
};
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void anc_set_SW_VIVID_PT_handler_callback(sw_vivid_pt_handler handler, sw_vivid_fade_out fade_out_function);
uint32_t *hal_audio_query_llf_share_info(U32 index);

void audio_vivid_pt_set_parameter(U32 is_runtime)
{
    U8 *nvkey_data_afc, *nvkey_data_nr, *nvkey_data_limiter;
    sysram_status_t nvdm_status;
    U32 nvkey_length_afc = 0, nvkey_length_nr = 0, nvkey_length_limiter = VIVID_PT_LIMITER_LEN, nvkey_length_total = 0;

    aud_set_vivid_pt_peq_param();

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_AFC, &nvkey_length_afc);
    if (nvdm_status || !nvkey_length_afc) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_AFC, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FA");
    }
    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_LDNR, &nvkey_length_nr);
    if (nvdm_status || !nvkey_length_nr) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_LDNR, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FB");
    }
    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_LIMITER, &nvkey_length_limiter);


    nvkey_length_total = nvkey_length_nr + nvkey_length_afc + VIVID_PT_LIMITER_LEN;

    nvkey_data_nr = (U8 *)pvPortMalloc(nvkey_length_total);
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_LDNR, nvkey_data_nr, &nvkey_length_nr);

    nvkey_data_afc = nvkey_data_nr + nvkey_length_nr;
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_AFC, nvkey_data_afc, &nvkey_length_afc);

    nvkey_data_limiter = nvkey_data_afc + nvkey_length_afc;
    if (nvdm_status || !nvkey_length_limiter) {
        LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_LIMITER, nvdm_status);
        //assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FD");
        U8 default_val[VIVID_PT_LIMITER_LEN] = {   0x01, 0x01, 0xC4, 0xFF, 0x2C, 0xCA, 0x3C, 0x2F, 0x2C, 0xCA,
                                                   0x3C, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        memcpy(nvkey_data_limiter, default_val, VIVID_PT_LIMITER_LEN);
    } else {
        if (nvkey_length_limiter != VIVID_PT_LIMITER_LEN) {
            LOGMSGIDE(" Nvkey NVID_DSP_ALG_VIVID_PT_LIMITER length is uncorrect:%d ", 1, nvkey_length_limiter);
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_LIMITER, nvkey_data_limiter, &nvkey_length_limiter);
    }

    if (is_runtime) {
        *nvkey_data_nr = vivid_pt_ctrl.nr_enable;
        *nvkey_data_afc = vivid_pt_ctrl.afc_enable;
    } else {
        vivid_pt_ctrl.nr_enable = *nvkey_data_nr;
        vivid_pt_ctrl.afc_enable = *nvkey_data_afc;
    }

    llf_control_realtime_set_parameter(NVID_DSP_ALG_VIVID_PT_LDNR, nvkey_length_total, nvkey_data_nr);

    LOGMSGIDI("OPEN, AFC_EN(%d), NR_EN(%d)", 2, *nvkey_data_afc, *nvkey_data_nr);

    vPortFree(nvkey_data_nr);
}

extern uint8_t SW_VIVID_PT_enable;
extern uint8_t g_anc_Vivid_PT_fadeout_check;
llf_status_t vivid_passthru_stream_handler(bool enable)
{
    U16 data16 = (LLF_TYPE_VIVID_PT << 8) | VIVID_PT_SUB_MODE;
    U32 channl_number = 0;
    llf_status_t llf_res = AUDIO_VIVID_PT_STATUS_SUCCESS;
    LOGMSGIDI("stream enable:%d", 1, enable);
    vivid_pt_ctrl.framework_enable = enable;

    if (enable) {
        LOGMSGIDI("open", 0);
        if (!ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM)) {
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, true);
        }
        llf_control_set_status(LLF_RUNNING_STATUS_RUNNING, LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, NULL);
        audio_vivid_pt_set_parameter(1);

        // Open framework
        mcu2dsp_open_param_t* open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
        llf_open_param_t* p_stream_in_param = &(open_param->stream_in_param.LLF);
        llf_open_param_t* p_stream_out_param = &(open_param->stream_out_param.LLF);
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

        // Collect parameters
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID;
        open_param->param.stream_in  = STREAM_IN_LLF;
        open_param->param.stream_out = STREAM_OUT_LLF;
        p_stream_in_param->frame_size = VIVID_PT_FRAME_SIZE;
        p_stream_in_param->frame_number = VIVID_PT_FRAME_NUM;
        p_stream_in_param->format = VIVID_PT_FORMAT;
        p_stream_out_param->frame_size = VIVID_PT_FRAME_SIZE;
        p_stream_out_param->frame_number = VIVID_PT_FRAME_NUM;

        //input config
        mcu2dsp_open_stream_in_param_t* voice_in_device = (mcu2dsp_open_stream_in_param_t*)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, voice_in_device);
        mcu2dsp_open_stream_in_param_t* anc_in_device = (mcu2dsp_open_stream_in_param_t*)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_ANC, anc_in_device);

        p_stream_in_param->audio_device[LLF_DATA_TYPE_REAR_L] = anc_in_device->afe.audio_device;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_REAR_L] = anc_in_device->afe.audio_interface;
        p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_REAR_L] = true;
        p_stream_in_param->audio_device[LLF_DATA_TYPE_INEAR_L] = anc_in_device->afe.audio_device1;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_INEAR_L] = anc_in_device->afe.audio_interface1;
        p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_INEAR_L] = true;
        p_stream_in_param->audio_device[LLF_DATA_TYPE_TALK] = voice_in_device->afe.audio_device;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_TALK] = voice_in_device->afe.audio_interface;


        for (uint32_t i = 0; i < LLF_DATA_TYPE_NUM; i++) {
            switch (input_data_order_vivid[i]) {
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
        memcpy(p_stream_in_param->in_data_order, input_data_order_vivid, LLF_DATA_TYPE_NUM * sizeof(U32));
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

        SW_VIVID_PT_enable = 1;
        vPortFree(voice_in_device);
        vPortFree(anc_in_device);
        vPortFree(open_param);
        vPortFree(start_param);
        llf_callback_service(LLF_TYPE_VIVID_PT, LLF_CONTROL_EVENT_ON, LLF_STATUS_SUCCESS);
    } else {
        LOGMSGIDI("close", 0);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_CLOSE, 0, 0, true);
        SW_VIVID_PT_enable = 0;
        if (!g_anc_Vivid_PT_fadeout_check) {
            g_anc_Vivid_PT_fadeout_check = 1;
        }
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, false);
        llf_control_set_status(LLF_RUNNING_STATUS_CLOSE, LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, NULL);
        llf_callback_service(LLF_TYPE_VIVID_PT, LLF_CONTROL_EVENT_OFF, LLF_STATUS_SUCCESS);

    }
    return llf_res;
}

audio_vivid_pt_status_t audio_vivid_pt_runtime_config_handler(U8 event, S32 param, void* misc)
{
    llf_status_t llf_status = AUDIO_VIVID_PT_STATUS_FAIL;
    switch(event) {
        case VIVID_PT_RUNTIME_CONFIG_EVENT_AFC_SWTICH: {
            llf_status = audio_anc_vivid_pt_control_afc_set_switch(param);
            break;
        }
        case VIVID_PT_RUNTIME_CONFIG_EVENT_NR_SWTICH: {
            llf_status = audio_anc_vivid_pt_control_nr_set_switch(param);
            break;
        }
        case VIVID_PT_RUNTIME_CONFIG_EVENT_BYPASS_SWTICH: {
            llf_status = audio_anc_vivid_pt_control_bypass_set_switch(param);
            break;
        }
        default:
            llf_status = AUDIO_VIVID_PT_STATUS_FAIL;
            break;
    }
    return llf_status;
}

/* Public functions ----------------------------------------------------------*/
void audio_anc_vivid_pt_init(void)
{
    U32 nvkey_length = 0;
    sysram_status_t nvdm_status;
    U8 *nvkey_data_afc, *nvkey_data_nr;

    anc_set_SW_VIVID_PT_handler_callback((sw_vivid_pt_handler)vivid_passthru_stream_handler, (sw_vivid_fade_out)audio_anc_vivid_pt_control_mute_extend_internal);

    llf_control_entry_t reg_entry = {
        .set_para_entry = audio_vivid_pt_set_parameter,
        .runtime_config_handler_entry = audio_vivid_pt_runtime_config_handler,
    };
    LOGMSGIDI("register entry 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 7,
        reg_entry.open_entry,
        reg_entry.close_entry,
        reg_entry.set_para_entry,
        reg_entry.dsp_callback_entry,
        reg_entry.bt_aws_mce_report_callback_entry,
        reg_entry.runtime_config_handler_entry,
        reg_entry.switch_mode_callback_entry);
    llf_control_register_entry(LLF_TYPE_VIVID_PT, &reg_entry);

    memset(&vivid_pt_ctrl, 0, sizeof(vivid_pt_ctrl));
    vivid_pt_ctrl.framework_enable = 0;
    vivid_pt_ctrl.share_addr = hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);

    //get Vivid PT parameters
    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_AFC, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_AFC, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FA");
    }
    nvkey_data_afc = (U8*)pvPortMalloc(nvkey_length);
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_AFC, nvkey_data_afc, &nvkey_length);
    vivid_pt_ctrl.afc_enable = *nvkey_data_afc;

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_LDNR, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_LDNR, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FA");
    }
    nvkey_data_nr = (U8 *)pvPortMalloc(nvkey_length);
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_LDNR, nvkey_data_nr, &nvkey_length);
    vivid_pt_ctrl.nr_enable = *nvkey_data_nr;

    LOGMSGIDI("INIT, AFC_EN(%d), NR_EN(%d)", 2, vivid_pt_ctrl.afc_enable, vivid_pt_ctrl.nr_enable);
    vPortFree(nvkey_data_afc);
    vPortFree(nvkey_data_nr);
}


void audio_anc_vivid_pt_get_mic_input_path(U32 *sel)
{
    if (sel) {
        *sel = AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FF_1;
    }
}

void audio_vivid_pt_switch_mode_callback(void)
{
    if (vivid_pt_ctrl.framework_enable) {
        vivid_pt_ctrl.framework_enable = false;
    }
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_mute_internal(void)
{
    //enable/disable unused device
    llf_status_t res = AUDIO_VIVID_PT_STATUS_FAIL;
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t llf_cap = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, VIVID_PT_RUNTIME_CONFIG_EVENT_ENABLE_DEVICE, 0};

    memcpy(malloc_ptr, &config, para_len);
    res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &llf_cap);


    return res;
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_mute_extend_internal(uint8_t fwk_action)
{
    UNUSED(fwk_action);
    llf_status_t res = AUDIO_VIVID_PT_STATUS_SUCCESS;
    //fade out
    audio_anc_vivid_pt_control_mute_internal();

    llf_control_cap_t llf_cap = {  LLF_TYPE_VIVID_PT,
                                    VIVID_PT_SUB_MODE,
                                    VIVID_PT_FRAME_SIZE, VIVID_PT_BUFFER_LEN,
                                    vivid_passthru_stream_handler,
                                    500,//delay 500 ms
                                    0};

    //OFF
    res = llf_control(LLF_CONTROL_EVENT_OFF, &llf_cap);

    return res;
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_afc_set_switch(bool enable)
{
    uint32_t *share_addr = vivid_pt_ctrl.share_addr;
    LOGMSGIDI("AFC switch:%d", 1, enable);
    if (!vivid_pt_ctrl.framework_enable | !share_addr) {
        return AUDIO_VIVID_PT_STATUS_FAIL;
    }

    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);

    llf_control_cap_t llf_cap = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, VIVID_PT_RUNTIME_CONFIG_EVENT_AFC_SWTICH, (U32)enable};

    memcpy(malloc_ptr, &config, para_len);

    vivid_pt_ctrl.afc_enable = enable;
    return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &llf_cap);
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_afc_get_switch(bool *enable)
{
    if (enable) {
        *enable = vivid_pt_ctrl.afc_enable;
    }
    return AUDIO_VIVID_PT_STATUS_SUCCESS;
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_nr_set_switch(bool enable)
{
    uint32_t *share_addr = vivid_pt_ctrl.share_addr;
    LOGMSGIDI("NR switch:%d", 1, enable);
    if (!vivid_pt_ctrl.framework_enable | !share_addr) {
        return AUDIO_VIVID_PT_STATUS_FAIL;
    }
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);

    llf_control_cap_t llf_cap = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, VIVID_PT_RUNTIME_CONFIG_EVENT_NR_SWTICH, (U32)enable};

    memcpy(malloc_ptr, &config, para_len);

    vivid_pt_ctrl.nr_enable = enable;
    return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &llf_cap);
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_nr_get_switch(bool *enable)
{
    if (enable) {
        *enable = vivid_pt_ctrl.nr_enable;
    }
    return AUDIO_VIVID_PT_STATUS_SUCCESS;
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_bypass_set_switch(bool enable)
{
    LOGMSGIDI("BYPASS switch:%d", 1, enable);
    if (!vivid_pt_ctrl.framework_enable) {
        return AUDIO_VIVID_PT_STATUS_FAIL;
    }
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);

    llf_control_cap_t llf_cap = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_VIVID_PT, VIVID_PT_SUB_MODE, VIVID_PT_RUNTIME_CONFIG_EVENT_BYPASS_SWTICH, (U32)enable};

    memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));

    vivid_pt_ctrl.bypass_enable = enable;
    return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &llf_cap);

}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_bypass_get_switch(bool *enable)
{
    if (enable) {
        *enable = vivid_pt_ctrl.bypass_enable;
    }
    return AUDIO_VIVID_PT_STATUS_SUCCESS;
}

audio_vivid_pt_status_t audio_anc_vivid_pt_control_save_setting(void)
{
    U8 *nvkey_data_afc, *nvkey_data_nr;
    sysram_status_t nvdm_status, save_status = 0;
    U32 nvkey_length = 0;

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_AFC, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_AFC, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FA");
    }
    nvkey_data_afc = (U8*)pvPortMalloc(nvkey_length);
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_AFC, nvkey_data_afc, &nvkey_length);
    *nvkey_data_afc = vivid_pt_ctrl.afc_enable;
    save_status |= flash_memory_write_nvdm_data(NVID_DSP_ALG_VIVID_PT_AFC, nvkey_data_afc, nvkey_length);


    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VIVID_PT_LDNR, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_VIVID_PT_LDNR, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE8FA");
    }
    nvkey_data_nr = (U8 *)pvPortMalloc(nvkey_length);
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VIVID_PT_LDNR, nvkey_data_nr, &nvkey_length);
    *nvkey_data_nr = vivid_pt_ctrl.nr_enable;
    save_status |= flash_memory_write_nvdm_data(NVID_DSP_ALG_VIVID_PT_LDNR, nvkey_data_nr, nvkey_length);

    vPortFree(nvkey_data_afc);
    vPortFree(nvkey_data_nr);

    if (save_status != NVDM_STATUS_NAT_OK) {
        LOGMSGIDE("save customer setting FAIL:%d", 1, nvdm_status);
        return AUDIO_VIVID_PT_STATUS_FAIL;
    }

    return AUDIO_VIVID_PT_STATUS_SUCCESS;

}
