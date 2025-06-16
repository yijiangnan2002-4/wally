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
#include "audio_anc_psap_control_internal.h"
#include "audio_anc_psap_user_setting_nvkey_struct.h"
#include "audio_anc_psap_algo_nvkey_struct.h"
#include "audio_anc_psap_mp_nvkey_struct.h"

#include <stdio.h>

#include "syslog.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
#include "hal_dvfs.h"
#include "nvkey_id_list.h"
#include "audio_nvdm_common.h"
#include "anc_control_api.h"
#include "bt_sink_srv_ami.h"
#include "hal_resource_assignment.h"
#include "leakage_detection_control.h"

#include "nvkey.h"
#include "nvdm_config_factory_reset.h"


/* Private define ------------------------------------------------------------*/

#if defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#define HA_SUB_MODE      (HA_SUB_MODE_PSAP)
#else
#define HA_SUB_MODE      (HA_SUB_MODE_HEARING_AID)
#endif
#define HA_FRAME_SIZE    (50)
#define HA_FRAME_NUM     (6)
#define HA_BUFFER_LEN    (HA_FRAME_SIZE * HA_FRAME_NUM)
#define HA_FORMAT        (HAL_AUDIO_PCM_FORMAT_S32_LE)

#define HA_NV_SIZE_SYS   (sizeof(psap_system_nvdm_t))
#define HA_NV_SIZE_USR   (sizeof(psap_usr_setting_nvdm_t))
#define HA_RUNTIME_SYNC_PARA_ID    (0xE87F)

#define DL_SCENARIO_RUNNING_FLAG_FOR_HA ((1 << AUDIO_SCENARIO_TYPE_HFP_DL) \
                                          | (1 << AUDIO_SCENARIO_TYPE_A2DP) \
                                          | (1 << AUDIO_SCENARIO_TYPE_BLE_DL) \
                                          | (1 << AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL) \
                                          | (1 << AUDIO_SCENARIO_TYPE_USB_AUDIO_PLAYBACK))
#define DL_MUTE_TIME_HA_OFF                 (500)//unit: ms
#define DL_MUTE_TIME_HA_ON                  (1500)//unit: ms
#define DL_MUTE_TIME_HA_RESET               (3000)//unit: ms

#ifdef AUDIO_USE_MSGID_LOG
#define LOGMSGIDE(fmt,arg...)               LOG_MSGID_E(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_MSGID_W(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_MSGID_I(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_MSGID_D(aud,"[LLF][HEARING_AID] "fmt,##arg)
#else
#define LOGMSGIDE(fmt,arg...)               LOG_E(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_W(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_I(aud,"[LLF][HEARING_AID] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_D(aud,"[LLF][HEARING_AID] "fmt,##arg)
#endif

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    U8  enable;
    S8  dBFS;
    U16 freq;
} ha_puretone_gen_t;

typedef struct {
    U8 switch_l: 1;
    U8 switch_r: 1;
    U8 reserved: 6;
} PACKED ha_tuning_mode_switch_t;

typedef struct {
    U8  mic_cal_mode_l;
    U8  mic_cal_mode_r;
} PACKED ha_mic_cal_t;

typedef struct {
    hal_audio_device_t    ff_device;
    hal_audio_interface_t ff_interface;
    hal_audio_device_t    talk_device;
    hal_audio_interface_t talk_interface;
    hal_audio_device_t    fb_device;
    hal_audio_interface_t fb_interface;
    U8                    ff_enable;
    U8                    talk_enable;
    U8                    fb_enable;
    U8                    reserved;
} ha_mic_ctrl;

typedef struct {
    U8 ha_switch;
    ha_tuning_mode_switch_t tuning_mode_ctrl;
    U8 mp_test_switch;
    U8 reserved1;

    U8 mute_l;
    U8 mute_r;
    U8 passthru_switch_l;
    U8 passthru_switch_r;

    ha_puretone_gen_t puretone_ctrl_l;
    ha_puretone_gen_t puretone_ctrl_r;

} ha_runtime_sync_ctrl_para_t;

typedef struct {
    ha_runtime_sync_ctrl_para_t ctrl_para;
    psap_usr_setting_nvdm_t usr_setting;
} ha_runtime_sync_para_t;

typedef struct {
    U8 framework_enable;
    U8 ha_enable;
    U8 is_sync_partner;
    U8 usr_switch;

    uint32_t *share_addr;
    audio_psap_device_role_t  role;
    psap_noti_callback_t               noti_callback;

    S32 dl_mute_dur_ha_on;
    S32 dl_mute_dur_ha_off;
    U16 reserved;
    U16 senario_state;
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
    U32 audio_running_flag;
    U16 dvfs_mid_lock;
    U16 dvfs_high_lock;
#endif

    U8 fb_det_enable;
    U8 anc_filter_id;
    U8 mic_cal_mode_l;
    U8 mic_cal_mode_r;

    ha_mic_ctrl mic_ctrl;

    psap_drc_ctrl_nvdm_t    drc_setting;
    psap_mode_ctrl_nvdm_t   mode_setting;
    psap_vol_nvdm_t         vol_setting;
    psap_system_nvdm_t      sys_setting;

    ha_runtime_sync_ctrl_para_t ctrl_para; //ha_runtime_sync_para_t
    psap_usr_setting_nvdm_t usr_setting; //ha_runtime_sync_para_t
} ha_control_t;
#define HA_RUNTIME_SYNC_PARA_LEN    (sizeof(ha_runtime_sync_para_t))
#define HA_RUNTIME_SYNC_CTRL_PARA_LEN    (sizeof(ha_runtime_sync_ctrl_para_t))

typedef enum {
    HA_RUNTIME_CONFIG_EVENT_HA_ENABLE       = 0,
    HA_RUNTIME_CONFIG_EVENT_LEVEL_IND       = 1,
    HA_RUNTIME_CONFIG_EVENT_VOL_IND         = 2,
    HA_RUNTIME_CONFIG_EVENT_MODE_IND        = 3,
    HA_RUNTIME_CONFIG_EVENT_MODE_TABLE      = 4,
    HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG      = 5,
    HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH      = 6,
    HA_RUNTIME_CONFIG_EVENT_BF_CONFIG       = 7,
    HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG      = 8,
    HA_RUNTIME_CONFIG_EVENT_INR_CONFIG      = 9,
    HA_RUNTIME_CONFIG_EVENT_USR_EQ_SWITCH   = 10,
    HA_RUNTIME_CONFIG_EVENT_USR_EQ_GAIN     = 11,
    HA_RUNTIME_CONFIG_EVENT_PURETONE_GEN    = 12,
    HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG  = 13,
    HA_RUNTIME_CONFIG_EVENT_MUTE            = 14,
    HA_RUNTIME_CONFIG_EVENT_HOWLING_DET     = 15,
    HA_RUNTIME_CONFIG_EVENT_DET_FB          = 16,
    HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL     = 17,
    HA_RUNTIME_CONFIG_EVENT_PASSTHRU_SWITCH = 18,
    HA_RUNTIME_CONFIG_EVENT_FADE_OUT        = 19,
    HA_RUNTIME_CONFIG_EVENT_AEA_JUDGEMENT   = 20,
    HA_RUNTIME_CONFIG_EVENT_AWS_INFO        = 21,
    HA_RUNTIME_CONFIG_EVENT_TRIAL_RUN       = 22,
    HA_RUNTIME_CONFIG_EVENT_SET_MIC_CAL_MODE = 23,
    HA_RUNTIME_CONFIG_EVENT_GET_MIC_CAL_DATA = 24,
    HA_RUNTIME_CONFIG_EVENT_NUM,
} ha_runtime_config_event_t;

typedef enum {
    HA_AWS_MCE_EVENT_AEA_SYNC_NUMBER = 1 << 0,
    //LLF_AWS_MCE_EVENT_SYNC_CTRL = (0x1 << 31),
} ha_aws_mce_event_t;

typedef uint8_t (*ha_psap_handler)     (bool enable);
typedef uint8_t (*ha_psap_fade_out)    (uint8_t fwk_action);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
ha_control_t g_ha_ctrl;
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
static TimerHandle_t g_psap_one_shot_timer = NULL;
#endif
/* Private functions ---------------------------------------------------------*/
extern bool app_hear_through_activity_is_out_case();
extern bool app_hearing_aid_activity_is_power_off();
extern void anc_set_HA_PSAP_handler_callback(ha_psap_handler handler, ha_psap_fade_out fade_out_function);
#if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
extern void anc_psap_control_mic(U8 mic_enable, audio_anc_control_type_t anc_type, audio_anc_control_filter_id_t anc_id);
#endif
uint32_t *hal_audio_query_llf_share_info(U32 index);
audio_psap_status_t audio_anc_psap_control_trial_run(ha_trial_run_event_t event, U32 data_len, void* data);
audio_psap_status_t audio_anc_psap_control_disable_extend_internal(uint8_t fwk_action);
audio_psap_status_t audio_anc_psap_control_set_mic_cal_mode(U8 *mic_cal_mode);
audio_psap_status_t audio_anc_psap_control_get_mic_cal_mode(U8 *mic_cal_mode, U8 *mic_cal_mode_len);
audio_psap_status_t audio_anc_psap_control_get_mic_cal_data(U32 *len, U8* data);

void audio_anc_psap_control_set_parameter(U32 is_runtime)
{
    ha_runtime_sync_ctrl_para_t *sync_ctrl_para = &g_ha_ctrl.ctrl_para;
    sysram_status_t nvdm_status;
    U32 nvkey_length = 0;

    if (is_runtime | (U32)g_ha_ctrl.is_sync_partner) {
        LOGMSGIDI("set para in runtime, bf_switch:%d, WNR_switch:%d, mode_index:%d", 3, g_ha_ctrl.usr_setting.psap_bf_config_t.bf_switch, g_ha_ctrl.usr_setting.WNR_switch, g_ha_ctrl.usr_setting.mode_index);
        if (g_ha_ctrl.usr_setting.psap_master_mic_ch == 0) {
            LOGMSGIDE("Wrong para, psap_master_mic_ch:%d", 1, g_ha_ctrl.usr_setting.psap_master_mic_ch);
        }
        llf_control_realtime_set_parameter(NVID_DSP_ALG_HA_SYS, HA_NV_SIZE_SYS, (U8*)&g_ha_ctrl.sys_setting);//called first to avoid race condition
        llf_control_realtime_set_parameter(NVID_DSP_ALG_HA_CUS_SETTING, HA_NV_SIZE_USR, (U8*)&g_ha_ctrl.usr_setting);//called first to avoid race condition
        g_ha_ctrl.is_sync_partner = 0;
    } else {
        //re-load from NVKey
        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_SYS, &nvkey_length);
        if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_system_nvdm_t))) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
            assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE824");
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_SYS, (uint8_t *)&(g_ha_ctrl.sys_setting), &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE824");
        }

        llf_control_realtime_set_parameter(NVID_DSP_ALG_HA_SYS, HA_NV_SIZE_SYS, (U8*)&g_ha_ctrl.sys_setting);


        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_CUS_SETTING, &nvkey_length);
        if (nvdm_status || !nvkey_length || (nvkey_length != HA_NV_SIZE_USR)) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
            assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE825");
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING, (uint8_t *)&(g_ha_ctrl.usr_setting), &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE825");
        }
        llf_control_realtime_set_parameter(NVID_DSP_ALG_HA_CUS_SETTING, HA_NV_SIZE_USR, (U8*)&g_ha_ctrl.usr_setting);

    }

    llf_control_realtime_set_parameter(HA_RUNTIME_SYNC_PARA_ID, HA_RUNTIME_SYNC_CTRL_PARA_LEN, (U8*)sync_ctrl_para);

    U16 nvkey[37] = {
        NVID_DSP_ALG_HA_MP_DATA,
        NVID_DSP_ALG_HA_AFC_COEF,
        NVID_DSP_ALG_HA_DRC,
        NVID_DSP_ALG_HA_LEVEL_1,
        NVID_DSP_ALG_HA_LEVEL_2,
        NVID_DSP_ALG_HA_LEVEL_3,
        NVID_DSP_ALG_HA_LEVEL_4,
        NVID_DSP_ALG_HA_LEVEL_5,
        NVID_DSP_ALG_HA_LEVEL_6,
        NVID_DSP_ALG_HA_LEVEL_7,
        NVID_DSP_ALG_HA_LEVEL_8,
        NVID_DSP_ALG_HA_LEVEL_9,
        NVID_DSP_ALG_HA_LEVEL_10,
        NVID_DSP_ALG_HA_LEVEL_11,
        NVID_DSP_ALG_HA_LEVEL_12,
        NVID_DSP_ALG_HA_LEVEL_13,
        NVID_DSP_ALG_HA_LEVEL_14,
        NVID_DSP_ALG_HA_LEVEL_15,
        NVID_DSP_ALG_HA_LEVEL_16,
        NVID_DSP_ALG_HA_PARA,
        NVID_DSP_ALG_HA_LOWCUT,
        NVID_DSP_ALG_HA_MODE_CTRL,
        NVID_DSP_ALG_HA_MODE_EQ_1,
        NVID_DSP_ALG_HA_MODE_EQ_2,
        NVID_DSP_ALG_HA_MODE_EQ_3,
        NVID_DSP_ALG_HA_MODE_EQ_4,
        NVID_DSP_ALG_HA_MODE_EQ_5,
        NVID_DSP_ALG_HA_MODE_EQ_6,
        NVID_DSP_ALG_HA_MODE_EQ_7,
        NVID_DSP_ALG_HA_MODE_EQ_8,
        NVID_DSP_ALG_HA_VENDOR_EQ,
        NVID_DSP_ALG_HA_VOL_TABLE,
        NVID_DSP_ALG_HA_AEA_COEF,
        NVID_DSP_ALG_HA_BF,
        NVID_DSP_ALG_HA_HD,
        NVID_DSP_ALG_HA_WNR,
        NVID_DSP_ALG_HA_BF_SC,
    };
    llf_control_set_multi_parameters(nvkey, 37);

    if (is_runtime == 0) {
        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_DRC, &nvkey_length);
        if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_drc_ctrl_nvdm_t))) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_DRC, nvdm_status);
            assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE802");
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_DRC, (uint8_t *)&(g_ha_ctrl.drc_setting), &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_DRC, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE802");
        }

        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_MODE_CTRL, &nvkey_length);
        if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_mode_ctrl_nvdm_t))) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MODE_CTRL, nvdm_status);
            assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE815");
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_MODE_CTRL, (uint8_t *)&(g_ha_ctrl.mode_setting), &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MODE_CTRL, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE815");
        }

        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_VOL_TABLE, &nvkey_length);
        if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_vol_nvdm_t))) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
            assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE820");
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_VOL_TABLE, (uint8_t *)&(g_ha_ctrl.vol_setting), &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE820");
        }
    }
}

void audio_anc_psap_control_get_input_data_order(U32 *channel_num, U32 data_order[LLF_DATA_TYPE_NUM])
{
    U32 index = 0;
    U32 ha_data_order[LLF_DATA_TYPE_NUM] = {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    };
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    U8 bf_switch_tmp = 0;

    if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
        bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;

    } else if (usr_setting->psap_bf_config_t.bf_switch) {
        bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
    }
#if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
    if (1 | usr_setting->WNR_switch | bf_switch_tmp) {
#else
    if (usr_setting->WNR_switch | bf_switch_tmp) {
#endif
        ha_data_order[index++] = LLF_DATA_TYPE_REAR_L;
        ha_data_order[index++] = LLF_DATA_TYPE_TALK;
    } else if (g_ha_ctrl.usr_setting.psap_master_mic_ch == 2) {
        ha_data_order[index++] = LLF_DATA_TYPE_TALK;
    } else {
        ha_data_order[index++] = LLF_DATA_TYPE_REAR_L;
    }
    if (g_ha_ctrl.fb_det_enable) {
        ha_data_order[index++] = LLF_DATA_TYPE_INEAR_L;
    }
    ha_data_order[index++] = LLF_DATA_TYPE_MUSIC_VOICE;
    *channel_num = index;
    memcpy(data_order, ha_data_order, LLF_DATA_TYPE_NUM * sizeof(U32));
    LOGMSGIDI("audio_anc_psap_control_get_input_data_order, %d %d %d %d %d %d %d", 7, ha_data_order[0], ha_data_order[1], ha_data_order[2], ha_data_order[3], ha_data_order[4], ha_data_order[5], ha_data_order[6]);
}

void audio_anc_psap_control_get_mic_input_path(U32 *sel)
{
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    if (sel) {
        *sel = 0;
        if (mic_ctrl->ff_enable) {
            *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FF_1;
        }
        if (mic_ctrl->fb_enable) {
            *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FB_1;
        }
        if (mic_ctrl->talk_enable) {
            *sel |= AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_TALK;
        }
        LOGMSGIDI("get mic input path:0x%x", 1, *sel);
    }
}

void audio_anc_psap_control_update_mic_input_path(void)
{
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    psap_bf_config_t *BF_config = (psap_bf_config_t*)&(usr_setting->psap_bf_config_t);
    U8 bf_switch_tmp = 0;

    if (BF_config->bf_switch_mode_ctrl) {
        bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;
    } else if (BF_config->bf_switch) {
        bf_switch_tmp = (U8)BF_config->bf_switch;
    }

    if (usr_setting->WNR_switch | bf_switch_tmp) {
        mic_ctrl->ff_enable = 1;
        mic_ctrl->talk_enable = 1;
    } else if (usr_setting->psap_master_mic_ch == 2) {
        mic_ctrl->ff_enable = 0;
        mic_ctrl->talk_enable = 1;
    } else {
        mic_ctrl->ff_enable = 1;
        mic_ctrl->talk_enable = 0;
    }
    LOGMSGIDI("update mic input path: ff(%d) fb(%d) talk(%d)", 3, mic_ctrl->ff_enable, mic_ctrl->fb_enable, mic_ctrl->talk_enable);
}

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
void audio_anc_psap_control_mute_dl(bool is_mute, bool need_sync, U32 sync_time)
{
    // sync_time(us)
    llf_sync_capability_t llf_sync_cap = {0};
    llf_sync_cap.action = is_mute ? LLF_SYNC_ACTION_DL_MUTE : LLF_SYNC_ACTION_DL_UNMUTE;
    llf_sync_cap.is_sync = need_sync;
    llf_sync_cap.sync_time = sync_time;
    llf_control_sync_control(&llf_sync_cap);
}
#endif

extern uint8_t g_anc_HA_fadeout_check;
extern uint8_t HA_PSAP_enable;
llf_status_t audio_anc_psap_control_stream_handler(bool enable)
{
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    U32 channl_number = 0;
    U16 data16 = (LLF_TYPE_HEARING_AID << 8) | HA_SUB_MODE;
    audio_psap_status_t psap_res = AUDIO_PSAP_STATUS_SUCCESS;

    LOGMSGIDI("stream enable:%d, sub_mode:%d", 2, enable, HA_SUB_MODE);

    // g_ha_ctrl.framework_enable = enable;

    if (!g_ha_ctrl.framework_enable && enable) {
        g_ha_ctrl.framework_enable = enable;
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
        if (g_ha_ctrl.dl_mute_dur_ha_off != DL_MUTE_TIME_HA_RESET) {
            audio_anc_psap_control_mute_dl(true, true, 100000);
            xTimerChangePeriod(g_psap_one_shot_timer, g_ha_ctrl.dl_mute_dur_ha_on / portTICK_PERIOD_MS, 0);
        }
        g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_OFF;
#endif
        LOGMSGIDI("open", 0);
        if (!ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM)) {
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, true);
        }

        llf_control_set_status(LLF_RUNNING_STATUS_RUNNING, LLF_TYPE_HEARING_AID, HA_SUB_MODE, NULL);
        //prepare parameters
        audio_anc_psap_control_set_parameter(1);

        // Open framework
        void *p_param_share;
        mcu2dsp_open_param_t open_param;
        llf_open_param_t* p_stream_in_param = &open_param.stream_in_param.LLF;
        llf_open_param_t* p_stream_out_param = &open_param.stream_out_param.LLF;
        memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));

        // Collect parameters
        open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID;
        open_param.param.stream_in  = STREAM_IN_LLF;
        open_param.param.stream_out = STREAM_OUT_LLF;
        p_stream_in_param->frame_size = HA_FRAME_SIZE;
        p_stream_in_param->frame_number = HA_FRAME_NUM;
        p_stream_in_param->format = HA_FORMAT;

        p_stream_in_param->audio_device[LLF_DATA_TYPE_REAR_L] = mic_ctrl->ff_device;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_REAR_L] = mic_ctrl->ff_interface;
        p_stream_in_param->audio_device[LLF_DATA_TYPE_TALK] = mic_ctrl->talk_device;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_TALK] = mic_ctrl->talk_interface;
        p_stream_in_param->audio_device[LLF_DATA_TYPE_INEAR_L] = mic_ctrl->fb_device;
        p_stream_in_param->audio_interface[LLF_DATA_TYPE_INEAR_L] = mic_ctrl->fb_interface;


        if (mic_ctrl->ff_enable) {
            p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_REAR_L] = true;
        }
        if (mic_ctrl->fb_enable) {
            p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_INEAR_L] = true;
        }
        if (mic_ctrl->talk_enable) {
            p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_TALK] = true;
        }

        p_stream_in_param->earbuds_ch = (ami_get_audio_channel() == AUDIO_CHANNEL_R) ? 2 : 1;
        p_stream_out_param->earbuds_ch = open_param.stream_in_param.LLF.earbuds_ch;

        //p_stream_in_param->two_mic_exist = true;
        p_stream_in_param->echo_reference[1] = true;//music_voice
        p_stream_in_param->anc_ff_cal_gain = 0;
        audio_anc_control_calibrate_gain_t audio_anc_calibrate_gain;
        if (!audio_anc_control_get_calibrate_gain_from_flash(&audio_anc_calibrate_gain)) {
            p_stream_in_param->anc_ff_cal_gain = audio_anc_calibrate_gain.gain_index_ff_l;
        }
        p_stream_in_param->music_need_compensation = true;

        //audio_anc_psap_control_get_input_data_order(mic_ctrl->ff_enable, mic_ctrl->fb_enable, mic_ctrl->talk_enable, p_stream_in_param->in_data_order);
        audio_anc_psap_control_get_input_data_order(&channl_number, p_stream_in_param->in_data_order);

        p_stream_in_param->channel_num = (U8)channl_number;
        p_stream_in_param->share_info.start_addr = (U32)hal_audio_query_llf_share_info(SHARE_BUFFER_LLF_INFO_ID_DSP);
        p_stream_in_param->share_info.length = SHARE_BUFFER_LLF_INFO_COUNT_DSP * 4;


        p_stream_out_param->frame_size = HA_FRAME_SIZE;
        p_stream_out_param->frame_number = HA_FRAME_NUM;

        LOGMSGIDI("frame size(%d), frame number(%d), format(%d), channel(%d), device(%d %d %d), interface(%d %d %d), enable(%d %d %d), earbuds_ch(%d), echo_ref(%d %d), music_need_cp(%d)", 17,
                p_stream_in_param->frame_size,
                p_stream_in_param->frame_number,
                p_stream_in_param->format,
                p_stream_in_param->channel_num,
                p_stream_in_param->audio_device[LLF_DATA_TYPE_REAR_L],
                p_stream_in_param->audio_device[LLF_DATA_TYPE_INEAR_L],
                p_stream_in_param->audio_device[LLF_DATA_TYPE_TALK],
                p_stream_in_param->audio_interface[LLF_DATA_TYPE_REAR_L],
                p_stream_in_param->audio_interface[LLF_DATA_TYPE_INEAR_L],
                p_stream_in_param->audio_interface[LLF_DATA_TYPE_TALK],
                p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_REAR_L],
                p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_INEAR_L],
                p_stream_in_param->audio_device_enable[LLF_DATA_TYPE_TALK],
                p_stream_in_param->earbuds_ch,
                p_stream_in_param->echo_reference[0],
                p_stream_in_param->echo_reference[1],
                p_stream_in_param->music_need_compensation);
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_ANC);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_OPEN, data16, (uint32_t)p_param_share, true);



        // Start framework
        mcu2dsp_start_param_t start_param;
        memset(&start_param, 0, sizeof(mcu2dsp_start_param_t));

        // Collect parameters
        start_param.param.stream_in     = STREAM_IN_LLF;
        start_param.param.stream_out    = STREAM_OUT_LLF;
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_ANC);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_START, 0, (uint32_t)p_param_share, true);
        HA_PSAP_enable = 1;
        llf_callback_service(LLF_TYPE_HEARING_AID, LLF_CONTROL_EVENT_ON, LLF_STATUS_SUCCESS);

    } else if(g_ha_ctrl.framework_enable && !enable) {
        g_ha_ctrl.framework_enable = enable;
        LOGMSGIDI("close", 0);
        g_ha_ctrl.ha_enable = false;
        g_ha_ctrl.ctrl_para.ha_switch = false;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_STOP, 0, 0, true);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_CLOSE, 0, 0, true);

        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM, NULL, false);

        HA_PSAP_enable = 0;
        llf_running_status_t running;
        llf_type_t type;
        llf_control_get_status(&running, &type, NULL, NULL);
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
#if defined(HAL_DVFS_MODULE_ENABLED)
#if AIR_BTA_IC_PREMIUM_G3
        hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
        LOGMSGIDI("speed is risen to 260M", 0);
#endif
#endif
#endif

        if ((type == LLF_TYPE_HEARING_AID) && (running != LLF_RUNNING_STATUS_RESET)) {
            llf_control_set_status(LLF_RUNNING_STATUS_CLOSE, LLF_TYPE_HEARING_AID, HA_SUB_MODE, NULL);
            llf_callback_service(LLF_TYPE_HEARING_AID, LLF_CONTROL_EVENT_OFF, LLF_STATUS_SUCCESS);
        }

        if (!g_anc_HA_fadeout_check) {
            g_anc_HA_fadeout_check = 1;
        }
    }
    return psap_res;
}

audio_psap_status_t audio_anc_psap_control_runtime_config_handler(U8 event, S32 param, void* misc)
{
    audio_psap_status_t psap_status = AUDIO_PSAP_STATUS_FAIL;
    switch(event) {
        case HA_RUNTIME_CONFIG_EVENT_HA_ENABLE: {
            if (param) {
                audio_anc_psap_control_enable();

            } else {
                audio_anc_psap_control_disable();

            }
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_LEVEL_IND: {
            audio_anc_psap_control_set_level_index(param, 0xFF, AUDIO_PSAP_DEVICE_ROLE_LEFT);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_VOL_IND: {
            audio_anc_psap_control_set_volume_index(param, 0xFF, AUDIO_PSAP_DEVICE_ROLE_LEFT);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MODE_IND: {
            audio_anc_psap_control_set_mode_index(param);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH: {
            audio_anc_psap_control_set_wnr_switch(param);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_BF_CONFIG: {
            audio_anc_psap_control_set_beamforming_setting((psap_bf_config_t*)&param);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG: {
            audio_anc_psap_control_set_afc_configuration((psap_afc_config_t*)&param, AUDIO_PSAP_DEVICE_ROLE_LEFT);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_USR_EQ_SWITCH: {
            audio_anc_psap_control_set_user_eq_switch((psap_user_eq_switch_t*)&param, AUDIO_PSAP_DEVICE_ROLE_LEFT);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL: {
            audio_anc_psap_control_set_mic_channel(param);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MUTE: {
            audio_anc_psap_control_set_mute(param, 0);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_TRIAL_RUN: {
            psap_status = audio_anc_psap_control_trial_run(HA_TRIAL_RUN_EVENT_CHECK_CALIBRATION, 0, NULL);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_SET_MIC_CAL_MODE: {
            U8 mic_cal_mode[2] = {1,2};
            psap_status = audio_anc_psap_control_set_mic_cal_mode(mic_cal_mode);
            U8 get_cal_mode[4];
            U8 len;
            audio_anc_psap_control_get_mic_cal_mode(get_cal_mode, &len);
            LOGMSGIDI("[atci] get mic cal mode, mic_cal_mode(%d %d), len(%d)", 3, mic_cal_mode[0], mic_cal_mode[1], len);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_GET_MIC_CAL_DATA: {
            U32 len;
            U8 data[104];
            psap_status = audio_anc_psap_control_get_mic_cal_data(&len, data);
            LOGMSGIDI("get data (len %d): %x%x%x%x%x%x%x%x%x%x", 11, len,
                      *data, *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7), *(data+8), *(data+9));
            break;
        }
        default:
            psap_status = AUDIO_PSAP_STATUS_FAIL;
            break;
    }
    return psap_status;
}

void audio_anc_psap_control_dsp_callback(S32 event, void *user_data)
{
    psap_noti_event_t ha_event = (psap_noti_event_t)event;
    U32* data = (U32*)user_data;
    //LOGMSGIDI("audio_anc_psap_control_dsp_callback, event0x%x, user_data[0]:0x%x, user_data[0]:0x%x", 3, event, data[0], data[1]);
    U8 data_len = (U8)((data[0] >> 8) & 0xFF);
    switch (ha_event) {
        case PSAP_NOTI_EVENT_FB_DETECT: {
            if (g_ha_ctrl.noti_callback) {
                U8 *det_fb = (U8*)hal_memview_dsp0_to_cm4(data[1]);
                LOGMSGIDI("dsp noti, detect FB, data_len:%d, det_fb(0x%x)=%d, det_fb(0x%x)=%d, det_fb(0x%x)=%d", 7, data_len, &det_fb[0], det_fb[0], &det_fb[1], det_fb[1], &det_fb[2], det_fb[2]);
                g_ha_ctrl.noti_callback(PSAP_NOTI_EVENT_FB_DETECT, det_fb, data_len);
            }
            g_ha_ctrl.fb_det_enable = 0;
            break;
        }
        case PSAP_NOTI_EVENT_AEA_CHANGE_MODE: {
            U8 *mode = (U8*)hal_memview_dsp0_to_cm4(data[1]);
            LOGMSGIDI("dsp noti, AEA_CHANGE_MODE:%d, data_len:%d", 2, *mode, data_len);
            if (g_ha_ctrl.noti_callback) {
                g_ha_ctrl.noti_callback(PSAP_NOTI_EVENT_AEA_CHANGE_MODE, mode, data_len);
            }
            break;
        }
        case PSAP_NOTI_EVENT_AEA_SYNC_DATA: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            if (BT_AWS_MCE_ROLE_PARTNER == role) {
                if (data_len != 20) {
                    LOGMSGIDI("dsp noti, AEA_SYNC_DATA, Wrong data length:%d", 1, data_len);
                    return;
                }
                U8 *aea_data = (U8*)hal_memview_dsp0_to_cm4(data[1]);
                //LOGMSGIDI("dsp noti, AEA_SYNC_DATA, data_len:%d, aea_data(0~9)=%d %d %d %d %d %d %d %d %d %d ", 11, data_len, aea_data[0], aea_data[1], aea_data[2], aea_data[3], aea_data[4], aea_data[5], aea_data[6], aea_data[7], aea_data[8], aea_data[9]);
                //LOGMSGIDI("dsp noti, AEA_SYNC_DATA, data_len:%d, aea_data(10~19)=%d %d %d %d %d %d %d %d %d %d ", 11, data_len, aea_data[10], aea_data[11], aea_data[12], aea_data[13], aea_data[14], aea_data[15], aea_data[16], aea_data[17], aea_data[18], aea_data[19]);
                llf_control_send_aws_mce_param(HA_AWS_MCE_EVENT_AEA_SYNC_NUMBER, data_len, aea_data);
            }
            break;
        }
        case PSAP_NOTI_EVENT_QUERY_AWS_INFO: {
            bt_aws_mce_agent_state_type_t aws_state = bt_sink_srv_cm_get_aws_link_state();
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            U32 info = (aws_state << 8) | (role & 0xFF);
            LOGMSGIDI("dsp query aws info, aws_state:0x%x, role:0x%x", 2, aws_state, role);
            if (g_ha_ctrl.framework_enable) {
                U32 para_len = sizeof(llf_control_runtime_config_t);
                void *malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_AWS_INFO, info};

                memcpy(malloc_ptr, &config, para_len);

                llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
            break;
        }
        default:
            break;
    }
}

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
static void audio_anc_psap_control_one_shot_timer_callback(TimerHandle_t xTimer)
{
    bool fit_det_stat = audio_anc_leakage_detection_get_mute_status();
    if (fit_det_stat) {
        LOGMSGIDI("skip unmute for fit detection", 0);
        return;
    }

    bool need_sync = app_hear_through_activity_is_out_case() ?
                     (app_hearing_aid_activity_is_power_off() ? false : true) : false;

    audio_anc_psap_control_mute_dl(false, need_sync, 100000);
    UNUSED(xTimer);
}
#endif

audio_psap_status_t audio_anc_psap_control_open_internal()
{
    audio_anc_control_result_t         control_ret;
    audio_anc_control_filter_id_t      target_filter_id;
    audio_anc_control_type_t           target_anc_type;
    audio_anc_control_misc_t           local_misc = {0};
    local_misc.type_mask_param.ANC_path_mask = AUDIO_ANC_CONTROL_RAMP_FF_L |
                                               AUDIO_ANC_CONTROL_RAMP_FB_L;
    target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP |
                           AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FF_1 |
                           AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_FB_1 |
                           AUDIO_ANC_CONTROL_EXTEND_TYPE_MASK_PATH_TALK;
    target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
    control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);
    return (audio_psap_status_t)control_ret;
}

audio_psap_status_t audio_anc_psap_control_close_internal()
{
    return (audio_psap_status_t)audio_anc_control_disable(NULL);
}

/* Public functions ----------------------------------------------------------*/
void bt_aws_mce_report_ha_callback(bt_aws_mce_report_info_t *para)
{
    aws_mce_report_llf_param_t *param = (aws_mce_report_llf_param_t*)para->param;

    if (param->event_id & HA_AWS_MCE_EVENT_AEA_SYNC_NUMBER) {
        U8 *aea_data = (U8*)&(param->param);
        LOGMSGIDI("receive partner AEA numbers, data_len:%d, aea_data(0~9)=%d %d %d %d %d %d %d %d %d %d ", 11, param->param_len, aea_data[0], aea_data[1], aea_data[2], aea_data[3], aea_data[4], aea_data[5], aea_data[6], aea_data[7], aea_data[8], aea_data[9]);
        LOGMSGIDI("receive partner AEA numbers, data_len:%d, aea_data(10~19)=%d %d %d %d %d %d %d %d %d %d ", 11, param->param_len, aea_data[10], aea_data[11], aea_data[12], aea_data[13], aea_data[14], aea_data[15], aea_data[16], aea_data[17], aea_data[18], aea_data[19]);
        U32 config_len = sizeof(llf_control_runtime_config_t);
        U32 para_len = config_len + param->param_len;
        void *malloc_ptr = pvPortMalloc(para_len);
        llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
        llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_AEA_JUDGEMENT, param->param_len};


        memcpy(malloc_ptr, &config, config_len);

        malloc_ptr += config_len;
        memcpy(malloc_ptr, aea_data, param->param_len);

        llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }
}

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
void audio_anc_psap_control_check_audio_running_flag(audio_scenario_type_t type, bool is_running)
{
    U32 *audio_running_flag = &(g_ha_ctrl.audio_running_flag);
    U32 val = is_running << type;
    U32 msk = 1 << type;
    *audio_running_flag = (*audio_running_flag & (~msk)) | (val & msk);
    LOGMSGIDI("type %d is running:%d, g_ha_ctrl.audio_running_flag = 0x%x", 3, type, is_running, *audio_running_flag);
    if (type == AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM) {
        if (is_running) {
            if (!g_ha_ctrl.dvfs_mid_lock) {
                hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
                g_ha_ctrl.dvfs_mid_lock = 1;
                LOGMSGIDI("dvfs lock:%d", 1, HAL_DVFS_OPP_MID);
            }

        } else {
            if (g_ha_ctrl.dvfs_mid_lock) {
                hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
                g_ha_ctrl.dvfs_mid_lock = 0;
                LOGMSGIDI("dvfs unlock:%d", 1, HAL_DVFS_OPP_MID);
            }
        }

    }

    if ((g_ha_ctrl.dvfs_mid_lock != 0) && (*audio_running_flag & DL_SCENARIO_RUNNING_FLAG_FOR_HA)) {
        if (!g_ha_ctrl.dvfs_high_lock) {
            hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
            g_ha_ctrl.dvfs_high_lock = 1;
            LOGMSGIDI("dvfs lock:%d", 1, HAL_DVFS_OPP_HIGH);
        }
    } else {
        if (g_ha_ctrl.dvfs_high_lock) {
            hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
            g_ha_ctrl.dvfs_high_lock = 0;
            LOGMSGIDI("dvfs unlock:%d", 1, HAL_DVFS_OPP_HIGH);
        }
    }
}
#endif

audio_psap_status_t audio_anc_psap_control_init(void)
{
    U32 nvkey_length = 0;
    sysram_status_t nvdm_status;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    U8 bf_switch_tmp = 0;

    anc_set_HA_PSAP_handler_callback((ha_psap_handler)audio_anc_psap_control_stream_handler, (ha_psap_fade_out)audio_anc_psap_control_disable_extend_internal);

    llf_control_entry_t reg_entry = {
        .open_entry = audio_anc_psap_control_open_internal,
        .close_entry = audio_anc_psap_control_close_internal,
        .set_para_entry = audio_anc_psap_control_set_parameter,
        .dsp_callback_entry = audio_anc_psap_control_dsp_callback,
        .bt_aws_mce_report_callback_entry = bt_aws_mce_report_ha_callback,
        .runtime_config_handler_entry = audio_anc_psap_control_runtime_config_handler,
    };
    LOGMSGIDI("register entry 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x,HA_NV_SIZE_USR=%d", 8,
        reg_entry.open_entry,
        reg_entry.close_entry,
        reg_entry.set_para_entry,
        reg_entry.dsp_callback_entry,
        reg_entry.bt_aws_mce_report_callback_entry,
        reg_entry.runtime_config_handler_entry,
        reg_entry.switch_mode_callback_entry,HA_NV_SIZE_USR);

    llf_control_register_entry(LLF_TYPE_HEARING_AID, &reg_entry);

    memset(&g_ha_ctrl, 0, sizeof(ha_control_t));

    g_ha_ctrl.role = (ami_get_audio_channel() == AUDIO_CHANNEL_R) ? AUDIO_PSAP_DEVICE_ROLE_RIGHT: AUDIO_PSAP_DEVICE_ROLE_LEFT;

    g_ha_ctrl.share_addr = hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);

    g_ha_ctrl.framework_enable = false;

    //get HA parameters
    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_CUS_SETTING, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != HA_NV_SIZE_USR)) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE825");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING, (uint8_t *)&(g_ha_ctrl.usr_setting), &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE825");
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_DRC, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_drc_ctrl_nvdm_t))) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_DRC, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE802");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_DRC, (uint8_t *)&(g_ha_ctrl.drc_setting), &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_DRC, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE802");
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_MODE_CTRL, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_mode_ctrl_nvdm_t))) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MODE_CTRL, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE815");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_MODE_CTRL, (uint8_t *)&(g_ha_ctrl.mode_setting), &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MODE_CTRL, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE815");
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_VOL_TABLE, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_vol_nvdm_t))) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE820");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_VOL_TABLE, (uint8_t *)&(g_ha_ctrl.vol_setting), &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE820");
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_SYS, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != sizeof(psap_system_nvdm_t))) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE824");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_SYS, (uint8_t *)&(g_ha_ctrl.sys_setting), &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_VOL_TABLE, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE824");
    }


    //get mic setting
    mcu2dsp_open_stream_in_param_t voice_in_device;
    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &voice_in_device);
    mcu2dsp_open_stream_in_param_t anc_in_device;
    hal_audio_get_stream_in_setting_config(AU_DSP_ANC, &anc_in_device);
    mic_ctrl->ff_device = anc_in_device.afe.audio_device;
    mic_ctrl->ff_interface = anc_in_device.afe.audio_interface;
    mic_ctrl->talk_device = voice_in_device.afe.audio_device;
    mic_ctrl->talk_interface = voice_in_device.afe.audio_interface;
    mic_ctrl->fb_device = anc_in_device.afe.audio_device1;
    mic_ctrl->fb_interface = anc_in_device.afe.audio_interface1;

    if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
        bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;

    } else if (usr_setting->psap_bf_config_t.bf_switch) {
        bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
    }
    if (usr_setting->WNR_switch | bf_switch_tmp) {
        mic_ctrl->ff_enable = 1;
        mic_ctrl->talk_enable = 1;
    } else if (usr_setting->psap_master_mic_ch == 2) {
        mic_ctrl->ff_enable = 0;
        mic_ctrl->talk_enable = 1;
    } else {
        mic_ctrl->ff_enable = 1;
    }
    if ((g_ha_ctrl.sys_setting.ANC_mode_with_HA_PSAP == AUDIO_ANC_CONTROL_TYPE_HYBRID) || (g_ha_ctrl.sys_setting.ANC_mode_with_HA_PSAP == AUDIO_ANC_CONTROL_TYPE_FB)) {
        mic_ctrl->fb_enable = 1;
    }

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    g_ha_ctrl.dl_mute_dur_ha_on = DL_MUTE_TIME_HA_ON;
    g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_OFF;
    if (g_psap_one_shot_timer== NULL) {
        g_psap_one_shot_timer = xTimerCreate("PSAPOneShotTimer", (g_ha_ctrl.dl_mute_dur_ha_on / portTICK_PERIOD_MS), pdFALSE, 0, audio_anc_psap_control_one_shot_timer_callback);
        if (g_psap_one_shot_timer == NULL) {
            assert(0 && "create timer FAIL!!");
        }
    }
#endif

    LOGMSGIDI("init done, is_role_R:%d, ha_enable:%d", 2, g_ha_ctrl.role, g_ha_ctrl.ha_enable);
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_enable(void)
{
    uint32_t *share_addr = g_ha_ctrl.share_addr;
    LOGMSGIDI("HA enable", 0);
    if (!g_ha_ctrl.framework_enable | !share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }

    if (!g_ha_ctrl.ha_enable) {
            void *malloc_ptr = NULL;
            U32 para_len = sizeof(llf_control_runtime_config_t);
            malloc_ptr = pvPortMalloc(para_len);

            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_HA_ENABLE, true};

            memcpy(malloc_ptr, &config, para_len);

            g_ha_ctrl.ha_enable = true;
            g_ha_ctrl.ctrl_para.ha_switch = true;
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
                        /* phase 2.0 */
//            hal_audio_status_send_update_dac_mode_event_to_am(0,0);
#endif
            return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_disable(void)
{
    uint32_t *share_addr = g_ha_ctrl.share_addr;
    LOGMSGIDI("HA disable", 0);
    if (!g_ha_ctrl.ha_enable) {
        return AUDIO_PSAP_STATUS_SUCCESS;
    }
    if (!g_ha_ctrl.framework_enable | !share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }

    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};

    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_HA_ENABLE, false};


    memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));

    g_ha_ctrl.ha_enable = false;
    g_ha_ctrl.ctrl_para.ha_switch = false;

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
        /* phase 2.0 */
    //    hal_audio_status_send_update_dac_mode_event_to_am(0,0);
#endif

    return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

}

audio_psap_status_t audio_anc_psap_control_get_status(bool *enable)
{
    if (enable) {
        *enable = g_ha_ctrl.ha_enable;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
audio_psap_status_t audio_anc_psap_control_get_switch_status(bool *enable)
{
    if (enable) {
        *enable = g_ha_ctrl.ctrl_para.ha_switch;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}
#endif

audio_psap_status_t audio_anc_psap_control_set_usr_switch(bool enable)
{
    LOGMSGIDI("USER HA switch(%d)", 1, enable);

    g_ha_ctrl.usr_switch = enable;

    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_get_usr_switch(bool *enable)
{
    if (enable) {
        *enable = g_ha_ctrl.usr_switch;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_suspend(void)
{
    if (g_ha_ctrl.framework_enable) {
        return llf_control_suspend();
    }
    return AUDIO_PSAP_STATUS_FAIL;
}

audio_psap_status_t audio_anc_psap_control_resume(void)
{
    if (g_ha_ctrl.framework_enable) {
        return llf_control_resume();
    }
    return AUDIO_PSAP_STATUS_FAIL;
}

audio_psap_status_t audio_anc_psap_control_set_level_index(U8 level_index_L, U8 level_index_R, audio_psap_device_role_t role)
{
    U32 drc_level_max_count = g_ha_ctrl.drc_setting.drc_level_max_count;
    psap_usr_setting_nvdm_t *usr_setting = &(g_ha_ctrl.usr_setting);
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_LEVEL_IND, 0};

    LOGMSGIDI("set level index L(%d) R(%d) role(%d), max(%d)", 4, level_index_L, level_index_R, role, drc_level_max_count);

    if (level_index_L < drc_level_max_count) {
        usr_setting->drc_level_index_l = level_index_L;
        if ((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (g_ha_ctrl.framework_enable != 0) && (malloc_ptr != 0)) {

            config.setting = (U32)level_index_L;
            memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    if (level_index_R < drc_level_max_count) {
        usr_setting->drc_level_index_r = level_index_R;
        if ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (g_ha_ctrl.framework_enable != 0) && (malloc_ptr != 0)) {

            config.setting = (U32)level_index_R;
            memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_level_index(U8 *level_index_L, U8 *level_index_R)
{
    if (level_index_L) {
        *level_index_L = g_ha_ctrl.usr_setting.drc_level_index_l;
    }
    if (level_index_R) {
        *level_index_R = g_ha_ctrl.usr_setting.drc_level_index_r;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_get_level_mode_max_count(U8 *level_max_cnt, U8 *mode_max_cnt, U8 *vol_max_cnt)
{
    if (level_max_cnt) {
        *level_max_cnt = g_ha_ctrl.drc_setting.drc_level_max_count;
    }
    if (mode_max_cnt) {
        *mode_max_cnt = g_ha_ctrl.mode_setting.mode_count;
    }
    if (vol_max_cnt) {
        *vol_max_cnt = g_ha_ctrl.vol_setting.psap_vol_max_count;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_volume_index(U8 vol_index_L, U8 vol_index_R, audio_psap_device_role_t role)
{
    U32 psap_vol_max_count = g_ha_ctrl.vol_setting.psap_vol_max_count;
    psap_usr_setting_nvdm_t *usr_setting = &(g_ha_ctrl.usr_setting);
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_VOL_IND, 0};

    LOGMSGIDI("set volume index L(%d) R(%d) role(%d), max(%d)", 4, vol_index_L, vol_index_R, role, psap_vol_max_count);

    if (vol_index_L <= psap_vol_max_count) {
        usr_setting->psap_vol_index_l = vol_index_L;
        if ((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (g_ha_ctrl.framework_enable != 0) && (malloc_ptr != 0)) {

            config.setting = (U32)vol_index_L;
            memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    if (vol_index_R <= psap_vol_max_count) {
        usr_setting->psap_vol_index_r = vol_index_R;
        if ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (g_ha_ctrl.framework_enable != 0) && (malloc_ptr != 0)) {

            config.setting = (U32)vol_index_R;
            memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;

}

audio_psap_status_t audio_anc_psap_control_get_volume_index(U8 *vol_index_L, U8 *vol_index_R)
{
    if (vol_index_L) {
        *vol_index_L = g_ha_ctrl.usr_setting.psap_vol_index_l;
    }
    if (vol_index_R) {
        *vol_index_R = g_ha_ctrl.usr_setting.psap_vol_index_r;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_mode_index(U8 mode_index)
{
    U32 mode_count = g_ha_ctrl.mode_setting.mode_count;
    uint32_t *share_addr = g_ha_ctrl.share_addr;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U8 bf_switch_tmp = 0;

    LOGMSGIDI("set mode index(%d), max(%d)", 2, mode_index, mode_count);

    if (mode_index < mode_count) {
        usr_setting->mode_index = mode_index;
        res = AUDIO_PSAP_STATUS_SUCCESS;
        if (g_ha_ctrl.framework_enable && (share_addr != NULL)) {

            if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
                bf_switch_tmp = (U8)usr_setting->psap_mode[mode_index].beamforming_switch;

            } else if (usr_setting->psap_bf_config_t.bf_switch) {
                bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
            }
            if ((bf_switch_tmp | usr_setting->WNR_switch) ^ (mic_ctrl->ff_enable & mic_ctrl->talk_enable)) {

                if (usr_setting->WNR_switch | bf_switch_tmp) {
                    mic_ctrl->ff_enable = 1;
                    mic_ctrl->talk_enable = 1;
                } else if (usr_setting->psap_master_mic_ch == 2) {
                    mic_ctrl->ff_enable = 0;
                    mic_ctrl->talk_enable = 1;
                } else {
                    mic_ctrl->ff_enable = 1;
                    mic_ctrl->talk_enable = 0;
                }
                U32 mic_path;
                audio_anc_psap_control_get_mic_input_path(&mic_path);
                LOGMSGIDI("get mic setting:0x%x", 1, mic_path);
            #if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
                U32 para_len = sizeof(llf_control_runtime_config_t);
                void *malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MODE_IND, (U32)mode_index};
                memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
                
                audio_anc_control_filter_id_t      target_filter_id;
                audio_anc_control_type_t           target_anc_type;
                target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_path;
                target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
                anc_psap_control_mic(mic_ctrl->ff_enable << FF_ENABLE | mic_ctrl->fb_enable << FB_ENABLE | mic_ctrl->talk_enable << TALK_ENABLE, target_anc_type, target_filter_id);
            #endif
                g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_RESET;

            } else {

                U32 para_len = sizeof(llf_control_runtime_config_t);
                void *malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MODE_IND, (U32)mode_index};
                memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
        }
    }

    return res;

}

audio_psap_status_t audio_anc_psap_control_get_mode_index(U8 *mode_index)
{
    if (mode_index) {
        *mode_index = g_ha_ctrl.usr_setting.mode_index;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_specific_mode_table(U8 mode_index, psap_mode_table_t *mode_para, audio_psap_device_role_t role)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    psap_usr_setting_nvdm_t *usr_setting = &(g_ha_ctrl.usr_setting);
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U8 bf_switch_tmp = 0;
    LOGMSGIDI("set specific mode table, index(%d)", 1, mode_index);

    if ((mode_index < g_ha_ctrl.mode_setting.mode_count) && (mode_para != NULL)) {
        if (usr_setting->mode_index == mode_index) {
            if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && ((usr_setting->psap_mode[mode_index].mfa_switch_l | usr_setting->psap_mode[mode_index].nr_switch) & (ctrl_para->tuning_mode_ctrl.switch_l)))
                || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && ((usr_setting->psap_mode[mode_index].mfa_switch_r | usr_setting->psap_mode[mode_index].nr_switch) & (ctrl_para->tuning_mode_ctrl.switch_r)))
                || (ctrl_para->mp_test_switch != 0)) {
                    LOGMSGIDE("set specific mode table FAIL, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
                    return AUDIO_PSAP_STATUS_FAIL;
            }
        }
        memcpy(&usr_setting->psap_mode[mode_index], mode_para, sizeof(psap_mode_table_t));

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable && (g_ha_ctrl.share_addr != NULL)) {

            if (mode_index == usr_setting->mode_index) {
                if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
                    bf_switch_tmp = (U8)usr_setting->psap_mode[mode_index].beamforming_switch;

                } else if (usr_setting->psap_bf_config_t.bf_switch) {
                    bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
                }
                if ((bf_switch_tmp | usr_setting->WNR_switch) ^ (mic_ctrl->ff_enable & mic_ctrl->talk_enable)) {

                    if (usr_setting->WNR_switch | bf_switch_tmp) {
                        mic_ctrl->ff_enable = 1;
                        mic_ctrl->talk_enable = 1;
                    } else if (usr_setting->psap_master_mic_ch == 2) {
                        mic_ctrl->ff_enable = 0;
                        mic_ctrl->talk_enable = 1;
                    } else {
                        mic_ctrl->ff_enable = 1;
                        mic_ctrl->talk_enable = 0;
                    }
                #if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
                    psap_mode_nvdm_t* now_mode_config = &usr_setting->psap_mode[mode_index];
                    psap_mode_nvdm_t mode_config = {  .mfa_switch_l = now_mode_config->mfa_switch_l,
                                                .mfa_switch_r = now_mode_config->mfa_switch_r,
                                                .low_cut_switch_l = now_mode_config->low_cut_switch_l,
                                                .low_cut_switch_r = now_mode_config->low_cut_switch_r,
                                                .nr_switch = now_mode_config->nr_switch ,
                                                .nr_level = now_mode_config->nr_level,
                                                .beamforming_switch = bf_switch_tmp};

                    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MODE_TABLE, (U32)mode_index};
                    U32 config_len = sizeof(llf_control_runtime_config_t);
                    U32 mode_table_len = sizeof(psap_mode_table_t);
                    U32 para_len = config_len + mode_table_len;
                    void* malloc_ptr = pvPortMalloc(para_len);
                    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};

                    memcpy(malloc_ptr, &config, config_len);

                    malloc_ptr += config_len;
                    memcpy(malloc_ptr, &mode_config, mode_table_len);

                    res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

                    U32 mic_input_path;
                    audio_anc_control_filter_id_t      target_filter_id;
                    audio_anc_control_type_t           target_anc_type;
                    audio_anc_psap_control_get_mic_input_path(&mic_input_path);
                    target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_input_path;
                    target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
                    anc_psap_control_mic(mic_ctrl->ff_enable << FF_ENABLE | mic_ctrl->fb_enable << FB_ENABLE | mic_ctrl->talk_enable << TALK_ENABLE, target_anc_type, target_filter_id);
                #endif
                    g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_RESET;

                    return res;
                }
            }
            U32 config_len = sizeof(llf_control_runtime_config_t);
            U32 mode_table_len = sizeof(psap_mode_table_t);
            U32 para_len = config_len + mode_table_len;
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MODE_TABLE, (U32)mode_index};

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, mode_para, mode_table_len);

            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_specific_mode_table(U8 mode_index, psap_mode_table_t *mode_para)
{
    //LOGMSGIDI("get specific mode table(index %d)", 1, mode_index);
    if ((mode_index < g_ha_ctrl.mode_setting.mode_count) && (mode_para != NULL)) {
        psap_mode_nvdm_t *psap_mode_ptr = &(g_ha_ctrl.usr_setting.psap_mode[mode_index]);
        memcpy(mode_para, psap_mode_ptr, sizeof(psap_mode_table_t));
        LOGMSGIDI("get specific mode table(index %d) mfa_switch(%d %d) low_cut_switch(%d %d) nr_switch(%d) nr_level(%d) beamforming_switch(%d)", 8, mode_index,
                psap_mode_ptr->mfa_switch_l,
                psap_mode_ptr->mfa_switch_r,
                psap_mode_ptr->low_cut_switch_l,
                psap_mode_ptr->low_cut_switch_r,
                psap_mode_ptr->nr_switch,
                psap_mode_ptr->nr_level,
                psap_mode_ptr->beamforming_switch);
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_aea_configuration(psap_aea_config_t *aea_config)
{
    audio_psap_device_role_t role = g_ha_ctrl.role;
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;

    if (aea_config) {
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (aea_config->aea_switch & (ctrl_para->tuning_mode_ctrl.switch_l)))
            || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (aea_config->aea_switch & (ctrl_para->tuning_mode_ctrl.switch_r)))
            || (ctrl_para->mp_test_switch != 0)) {
                LOGMSGIDE("set AFC config fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
                return AUDIO_PSAP_STATUS_FAIL;
        }
        LOGMSGIDI("set AEA config", 0);
        memcpy(&g_ha_ctrl.usr_setting.psap_aea_config_t, aea_config, sizeof(psap_aea_config_t));

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable && (g_ha_ctrl.share_addr != NULL)) {
            U32 config_len = sizeof(llf_control_runtime_config_t);
            U32 aea_config_len = sizeof(psap_aea_config_t);
            U32 para_len = config_len + aea_config_len;
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG, 0};


            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, aea_config, aea_config_len);

            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

        }


    }

    return res;
}

audio_psap_status_t audio_anc_psap_control_get_aea_configuration(psap_aea_config_t *aea_config)
{
    audio_psap_device_role_t role = g_ha_ctrl.role;
    ha_runtime_sync_ctrl_para_t *ctrl_para = &(g_ha_ctrl.ctrl_para);
    ha_tuning_mode_switch_t *tuning_mode_ctrl = &(ctrl_para->tuning_mode_ctrl);

    if (aea_config) {
        memcpy(aea_config, &g_ha_ctrl.usr_setting.psap_aea_config_t, sizeof(psap_aea_config_t));
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (tuning_mode_ctrl->switch_l))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (tuning_mode_ctrl->switch_r))
        || (ctrl_para->mp_test_switch != 0)) {
            aea_config->aea_switch = 0;
        }
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_wnr_switch(bool enable)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U8 bf_switch_tmp = 0;

    if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (enable & (ctrl_para->tuning_mode_ctrl.switch_l)))
        || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (enable & (ctrl_para->tuning_mode_ctrl.switch_r)))
        || (ctrl_para->mp_test_switch != 0)) {
            LOGMSGIDE("set WNR switch fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
            return AUDIO_PSAP_STATUS_FAIL;
    }

    usr_setting->WNR_switch = enable ? 1 : 0;

    LOGMSGIDI("WNR switch:%d, g_ha_ctrl.usr_setting.WNR_switch:%d", 2, enable, usr_setting->WNR_switch);
    audio_anc_psap_control_save_setting();

    res = AUDIO_PSAP_STATUS_SUCCESS;

    if (g_ha_ctrl.framework_enable && (g_ha_ctrl.share_addr != NULL)) {
        if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
            bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;

        } else if (usr_setting->psap_bf_config_t.bf_switch) {
            bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
        }

        if ((bf_switch_tmp | usr_setting->WNR_switch) ^ (mic_ctrl->ff_enable & mic_ctrl->talk_enable)) {

            if (usr_setting->WNR_switch | usr_setting->psap_bf_config_t.bf_switch) {
                mic_ctrl->ff_enable = 1;
                mic_ctrl->talk_enable = 1;
            } else if (usr_setting->psap_master_mic_ch == 2) {
                mic_ctrl->ff_enable = 0;
                mic_ctrl->talk_enable = 1;
            } else {
                mic_ctrl->ff_enable = 1;
                mic_ctrl->talk_enable = 0;
            }
            g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_RESET;

        #if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH, (U32)(enable ? 1 : 0)};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            U32 mic_input_path;
            audio_anc_control_filter_id_t      target_filter_id;
            audio_anc_control_type_t           target_anc_type;
            audio_anc_psap_control_get_mic_input_path(&mic_input_path);
            target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_input_path;
            target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
            anc_psap_control_mic(mic_ctrl->ff_enable << FF_ENABLE | mic_ctrl->fb_enable << FB_ENABLE | mic_ctrl->talk_enable << TALK_ENABLE, target_anc_type, target_filter_id);
        #endif

        } else {

            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH, (U32)(enable ? 1 : 0)};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_wnr_switch(bool *enable)
{
    audio_psap_device_role_t role = g_ha_ctrl.role;
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    ha_tuning_mode_switch_t *tuning_mode_ctrl = &(ctrl_para->tuning_mode_ctrl);
    if (enable) {
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (tuning_mode_ctrl->switch_l))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (tuning_mode_ctrl->switch_r))
        || (ctrl_para->mp_test_switch != 0)) {
            *enable = 0;
        } else {
            *enable = g_ha_ctrl.usr_setting.WNR_switch;
        }
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_beamforming_setting(psap_bf_config_t *BF_config)
{
    audio_psap_device_role_t role = g_ha_ctrl.role;
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    uint32_t *share_addr = g_ha_ctrl.share_addr;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U8 bf_switch_tmp = 0;

    if (BF_config) {
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && ((BF_config->bf_switch | BF_config->bf_switch_mode_ctrl) & (ctrl_para->tuning_mode_ctrl.switch_l)))
            || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && ((BF_config->bf_switch | BF_config->bf_switch_mode_ctrl) & (ctrl_para->tuning_mode_ctrl.switch_r)))
            || (ctrl_para->mp_test_switch != 0)) {
                LOGMSGIDE("set BF setting fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
                return AUDIO_PSAP_STATUS_FAIL;
        }
        usr_setting->psap_bf_config_t.bf_switch = BF_config->bf_switch;
        usr_setting->psap_bf_config_t.bf_switch_mode_ctrl = BF_config->bf_switch_mode_ctrl;
        LOGMSGIDI("set BF setting, bf_switch:%d, bf_switch_mode_ctrl:%d", 2, usr_setting->psap_bf_config_t.bf_switch, usr_setting->psap_bf_config_t.bf_switch_mode_ctrl);

        audio_anc_psap_control_save_setting();

        if (BF_config->bf_switch_mode_ctrl) {
            bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;
        } else if (BF_config->bf_switch) {
            bf_switch_tmp = (U8)BF_config->bf_switch;
        }

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable && (share_addr != NULL)) {

            if ((bf_switch_tmp | usr_setting->WNR_switch) ^ (mic_ctrl->ff_enable & mic_ctrl->talk_enable)) {

                if (usr_setting->WNR_switch | bf_switch_tmp) {
                    mic_ctrl->ff_enable = 1;
                    mic_ctrl->talk_enable = 1;
                } else if (usr_setting->psap_master_mic_ch == 2) {
                    mic_ctrl->ff_enable = 0;
                    mic_ctrl->talk_enable = 1;
                } else {
                    mic_ctrl->ff_enable = 1;
                    mic_ctrl->talk_enable = 0;
                }
                g_ha_ctrl.dl_mute_dur_ha_off = DL_MUTE_TIME_HA_RESET;
            #if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
                U8* p = (U8*)BF_config;
                U32 para_len = sizeof(llf_control_runtime_config_t);
                void* malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_BF_CONFIG, (U32)(*p)};

                memcpy(malloc_ptr, &config, para_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

                U32 mic_input_path;
                audio_anc_control_filter_id_t      target_filter_id;
                audio_anc_control_type_t           target_anc_type;
                audio_anc_psap_control_get_mic_input_path(&mic_input_path);
                target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_input_path;
                target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
                anc_psap_control_mic(mic_ctrl->ff_enable << FF_ENABLE | mic_ctrl->fb_enable << FB_ENABLE | mic_ctrl->talk_enable << TALK_ENABLE, target_anc_type, target_filter_id);
            #endif

            } else {

                U8* p = (U8*)BF_config;
                U32 para_len = sizeof(llf_control_runtime_config_t);
                void *malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_BF_CONFIG, (U32)(*p)};

                memcpy(malloc_ptr, &config, para_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_beamforming_setting(psap_bf_config_t *BF_config)
{
    audio_psap_device_role_t role = g_ha_ctrl.role;
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    ha_tuning_mode_switch_t *tuning_mode_ctrl = &(ctrl_para->tuning_mode_ctrl);
    psap_bf_config_t *cur_bf_config = (psap_bf_config_t*)&(g_ha_ctrl.usr_setting.psap_bf_config_t);
    if (BF_config) {
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (tuning_mode_ctrl->switch_l))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (tuning_mode_ctrl->switch_r))
        || (ctrl_para->mp_test_switch != 0)) {
            BF_config->bf_switch = 0;
        } else {
            BF_config->bf_switch = cur_bf_config->bf_switch;
        }
        BF_config->bf_switch_mode_ctrl = cur_bf_config->bf_switch_mode_ctrl;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_afc_configuration(psap_afc_config_t *afc_config, audio_psap_device_role_t role)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;

    if (afc_config) {
        if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (afc_config->afc_ctrl_switch_l & (ctrl_para->tuning_mode_ctrl.switch_l)))
            || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (afc_config->afc_ctrl_switch_r & (ctrl_para->tuning_mode_ctrl.switch_r)))
            || (ctrl_para->mp_test_switch != 0)) {
                LOGMSGIDE("set AFC config fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
                return AUDIO_PSAP_STATUS_FAIL;
        }

        LOGMSGIDI("set AFC config", 0);
        g_ha_ctrl.usr_setting.psap_afc_config_t.afc_ctrl_switch_l = afc_config->afc_ctrl_switch_l;
        g_ha_ctrl.usr_setting.psap_afc_config_t.afc_ctrl_switch_r = afc_config->afc_ctrl_switch_r;

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable) {
            U8* p = (U8*)afc_config;
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG, (U32)(*p)};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }

    return res;
}

audio_psap_status_t audio_anc_psap_control_get_afc_configuration(psap_afc_config_t *afc_config)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    psap_afc_config_t *usr_setting_afc = (psap_afc_config_t*)&g_ha_ctrl.usr_setting.psap_afc_config_t;
    audio_psap_device_role_t role = g_ha_ctrl.role;
    if (afc_config) {
        *afc_config = *usr_setting_afc;
        if ((ctrl_para->mp_test_switch != 0)) {
            afc_config->afc_ctrl_switch_l = 0;
            afc_config->afc_ctrl_switch_r = 0;
        } else {
            if ((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (ctrl_para->tuning_mode_ctrl.switch_l)) {
                afc_config->afc_ctrl_switch_l = 0;
            } else if ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (ctrl_para->tuning_mode_ctrl.switch_r)) {
                afc_config->afc_ctrl_switch_r = 0;
            }
        }
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_inr_configuration(ha_inr_config_t *INR_config, audio_psap_device_role_t role)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;

    if (INR_config) {
        if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (INR_config->inr_switch_l & (ctrl_para->tuning_mode_ctrl.switch_l)))
            || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (INR_config->inr_switch_r & (ctrl_para->tuning_mode_ctrl.switch_r)))
            || (ctrl_para->mp_test_switch != 0)) {
                LOGMSGIDE("set INR config fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
                return AUDIO_PSAP_STATUS_FAIL;
        }
        LOGMSGIDI("set INR config", 0);
        U8* src = (U8*)INR_config;
        U8* dst = (U8*)&g_ha_ctrl.usr_setting.ha_inr_config_t;
        memcpy(dst, src, sizeof(ha_inr_config_t));
        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable) {
            U32 config_len = sizeof(llf_control_runtime_config_t);
            U32 inr_config_len = sizeof(ha_inr_config_t);
            U32 para_len = config_len + inr_config_len;
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_INR_CONFIG, (U32)role};

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, src, inr_config_len);

            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }

    return res;
}


audio_psap_status_t audio_anc_psap_control_get_inr_configuration(ha_inr_config_t *INR_config)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    U32 mp_test_switch = (U32)ctrl_para->mp_test_switch;
    ha_tuning_mode_switch_t tuning_mode_ctrl = ctrl_para->tuning_mode_ctrl;
    U32 role = g_ha_ctrl.role;
    if (INR_config) {
        memcpy(INR_config, &g_ha_ctrl.usr_setting.ha_inr_config_t, sizeof(ha_inr_config_t));
        if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (tuning_mode_ctrl.switch_l))
        || (mp_test_switch != 0)) {
            INR_config->inr_switch_l = 0;
        }
        if (((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (tuning_mode_ctrl.switch_r))
        || (mp_test_switch != 0)) {
            INR_config->inr_switch_r = 0;
        }
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_user_eq_switch(psap_user_eq_switch_t *usr_eq_switch, audio_psap_device_role_t role)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    LOGMSGIDI("set USER EQ switch, role:%d", 1, role);

    if (usr_eq_switch) {
        U8* src = (U8*)usr_eq_switch;
        U8* dst = (U8*)&g_ha_ctrl.usr_setting.psap_user_eq_switch_t;
        *dst = *src;

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable) {
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_USR_EQ_SWITCH, (U32)(*src)};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_user_eq_switch(psap_user_eq_switch_t *usr_eq_switch)
{
    if (usr_eq_switch) {
        U8* dst = (U8*)usr_eq_switch;
        U8* src = (U8*)&g_ha_ctrl.usr_setting.psap_user_eq_switch_t;
        *dst = *src;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_user_eq_gain(psap_usr_eq_para_t *usr_eq_gain, audio_psap_device_role_t role)
{
    UNUSED(role);
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    LOGMSGIDI("set USER EQ gain, role:%d", 1, role);

    if (usr_eq_gain) {
        void* src = (void*)usr_eq_gain;
        void* dst = (void*)&g_ha_ctrl.usr_setting.psap_usr_eq_para_t;
        memcpy(dst, src, sizeof(psap_usr_eq_para_t));

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable && (g_ha_ctrl.share_addr != NULL)) {
            U32 config_len = sizeof(llf_control_runtime_config_t);
            U32 usr_eq_len = sizeof(psap_usr_eq_para_t);
            U32 para_len = config_len + usr_eq_len;
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_USR_EQ_GAIN, role};

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, src, usr_eq_len);

            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_user_eq_gain(psap_usr_eq_para_t *usr_eq_gain)
{
    if (usr_eq_gain) {
        void* dst = (void*)usr_eq_gain;
        void* src = (void*)&g_ha_ctrl.usr_setting.psap_usr_eq_para_t;
        memcpy(dst, src, sizeof(psap_usr_eq_para_t));
    }

    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_get_speaker_reference(psap_test_spk_ref_t *spk_ref)
{
    if (spk_ref) {
        U32 nvkey_length = 0;
        sysram_status_t nvdm_status;

        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_MP_DATA, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MP_DATA, nvdm_status);
            assert(0 && "Read Nvkey length Fail, id:0xE800");
        }
        psap_mp_nvdm_t* src = (psap_mp_nvdm_t*)pvPortMalloc(nvkey_length);
        if (!src) {
            return AUDIO_PSAP_STATUS_FAIL;
        }
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_MP_DATA, (U8*)src, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_MP_DATA, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE800");
        }
        memcpy(spk_ref, &(src->test_spk_ref_l_64), sizeof(psap_test_spk_ref_t));
        vPortFree(src);
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_puretone_generator(bool enable_l, U16 freq_l, S8 dBFS_l, bool enable_r, U16 freq_r, S8 dBFS_r)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    ha_puretone_gen_t puretone_l = {.enable = enable_l,
                                    .dBFS = dBFS_l,
                                    .freq = freq_l};
    ha_puretone_gen_t puretone_r = {.enable = enable_r,
                                    .dBFS = dBFS_r,
                                    .freq = freq_r};
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;

    LOGMSGIDI("set pure tone generator, L[enable(%d) freq(%d) dBFS(%d)] R[enable(%d) freq(%d) dBFS(%d)]", 6, enable_l, freq_l, dBFS_l, enable_r, freq_r, dBFS_r);
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }


    U32 config_len = sizeof(llf_control_runtime_config_t);
    U32 puretone_len = sizeof(ha_puretone_gen_t);
    U32 para_len = config_len + puretone_len;
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_PURETONE_GEN, 0};

    memcpy(malloc_ptr, &config, config_len);

    malloc_ptr += config_len;
    memcpy(malloc_ptr, (g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) ? &puretone_l : &puretone_r, puretone_len);

    res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    ctrl_para->puretone_ctrl_l = puretone_l;
    ctrl_para->puretone_ctrl_r = puretone_r;
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_puretone_generator(bool *enable_l, U16 *freq_l, S8 *dBFS_l, bool *enable_r, U16 *freq_r, S8 *dBFS_r)
{
    ha_puretone_gen_t *puretone_ctrl_ptr = &(g_ha_ctrl.ctrl_para.puretone_ctrl_l);
    if (enable_l) {
        *enable_l = puretone_ctrl_ptr->enable;
    }
    if (freq_l) {
        *freq_l = puretone_ctrl_ptr->freq;
    }
    if (dBFS_l) {
        *dBFS_l = puretone_ctrl_ptr->dBFS;
    }
    puretone_ctrl_ptr = &(g_ha_ctrl.ctrl_para.puretone_ctrl_r);
    if (enable_r) {
        *enable_r = puretone_ctrl_ptr->enable;
    }
    if (freq_r) {
        *freq_r = puretone_ctrl_ptr->freq;
    }
    if (dBFS_r) {
        *dBFS_r = puretone_ctrl_ptr->dBFS;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_mix_mode(psap_scenario_mix_mode_t *mix_mode_para, audio_psap_device_role_t role)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U8* tmp = (U8*)mix_mode_para;

    if (mix_mode_para) {
        LOGMSGIDI("set mix mode, role:%d, 0x%x %x %x %x %x %x %x %x %x ", 10, role, *tmp, *(tmp+1), *(tmp+2), *(tmp+3), *(tmp+4), *(tmp+5), *(tmp+6), *(tmp+7), *(tmp+8));

        memcpy(&g_ha_ctrl.usr_setting.psap_scenario_mix_mode_t, mix_mode_para, sizeof(psap_scenario_mix_mode_t));

        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (g_ha_ctrl.framework_enable) {
            U32 config_len = sizeof(llf_control_runtime_config_t);
            U32 mix_mode_len = sizeof(psap_scenario_mix_mode_t);
            U32 para_len = config_len + mix_mode_len;
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG, role};

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, mix_mode_para, mix_mode_len);

            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }

    return res;
}

audio_psap_status_t audio_anc_psap_control_get_mix_mode(psap_scenario_mix_mode_t *mix_mode_para)
{
    U8* tmp = (U8*)mix_mode_para;
    if (mix_mode_para) {
        memcpy(mix_mode_para, &g_ha_ctrl.usr_setting.psap_scenario_mix_mode_t, sizeof(psap_scenario_mix_mode_t));
        LOGMSGIDI("get mix mode, 0x%x %x %x %x %x %x %x %x %x ", 9,*tmp, *(tmp+1), *(tmp+2), *(tmp+3), *(tmp+4), *(tmp+5), *(tmp+6), *(tmp+7), *(tmp+8));
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_tuning_mode(U8 mode_switch)//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    ha_tuning_mode_switch_t *tuning_mode_ctrl_ptr = &(ctrl_para->tuning_mode_ctrl);
    psap_usr_setting_nvdm_t *usr_setting_ptr = &(g_ha_ctrl.usr_setting);

    LOGMSGIDI("set tuning mode, role:%d, mode_switch:%d", 2, g_ha_ctrl.role, mode_switch);
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }

    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;
    ha_tuning_mode_switch_t* tuning_switch = (ha_tuning_mode_switch_t*)&mode_switch;

    U32 config_len = sizeof(llf_control_runtime_config_t);
    U32 para_len = config_len;
    void *malloc_ptr = NULL;
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, 0, 0};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0};
    U8 to_switch, now_switch, now_afc_switch, now_inr_switch, now_mfa_switch;
    psap_mode_nvdm_t* now_mode_config = &usr_setting_ptr->psap_mode[usr_setting_ptr->mode_index];

    if (g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        to_switch = tuning_switch->switch_l;
        now_switch = tuning_mode_ctrl_ptr->switch_l;
        now_afc_switch = usr_setting_ptr->psap_afc_config_t.afc_ctrl_switch_l;
        now_inr_switch = usr_setting_ptr->ha_inr_config_t.inr_switch_l;
        now_mfa_switch = now_mode_config->mfa_switch_l;
    } else {
        to_switch = tuning_switch->switch_r;
        now_switch = tuning_mode_ctrl_ptr->switch_r;
        now_afc_switch = usr_setting_ptr->psap_afc_config_t.afc_ctrl_switch_r;
        now_inr_switch = usr_setting_ptr->ha_inr_config_t.inr_switch_r;
        now_mfa_switch = now_mode_config->mfa_switch_r;
    }

    if (to_switch != now_switch) {
        if (to_switch) { //enable tuning mode
            if (now_afc_switch) {
                //turn off AFC
                config.config_event = HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG;
                config.setting = 0;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = config_len;
                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (now_inr_switch) {
                //turn off INR
                ha_inr_config_t INR_config = {  .inr_switch_l = 0,
                                                .inr_sensitivity_l = usr_setting_ptr->ha_inr_config_t.inr_sensitivity_l,
                                                .inr_strength_l = usr_setting_ptr->ha_inr_config_t.inr_strength_l,
                                                .inr_switch_r = 0,
                                                .inr_sensitivity_r = usr_setting_ptr->ha_inr_config_t.inr_sensitivity_r,
                                                .inr_strength_r = usr_setting_ptr->ha_inr_config_t.inr_strength_r};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_INR_CONFIG;

                U32 inr_config_len = sizeof(ha_inr_config_t);
                para_len = config_len + inr_config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);
                malloc_ptr += config_len;
                memcpy(malloc_ptr, &INR_config, inr_config_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (now_mfa_switch) {
                //turn off MFA
                psap_mode_nvdm_t mode_config = {  .mfa_switch_l = 0,
                                                .mfa_switch_r = 0,
                                                .low_cut_switch_l = now_mode_config->low_cut_switch_l,
                                                .low_cut_switch_r = now_mode_config->low_cut_switch_r,
                                                .nr_switch = 0,
                                                .nr_level = now_mode_config->nr_level,
                                                .beamforming_switch = now_mode_config->beamforming_switch};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_MODE_TABLE;
                config.setting = usr_setting_ptr->mode_index;

                U32 mode_table_len = sizeof(psap_mode_table_t);
                para_len = config_len + mode_table_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &mode_config, mode_table_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (usr_setting_ptr->psap_bf_config_t.bf_switch) {
                //turn off BF
                config.config_event = HA_RUNTIME_CONFIG_EVENT_BF_CONFIG;
                config.setting = 0;

                para_len = config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;
                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
            if (usr_setting_ptr->WNR_switch) {
                //turn off WNR
                config.config_event = HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH;
                config.setting = 0;

                para_len = config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;
                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
            if (usr_setting_ptr->psap_aea_config_t.aea_switch) {
                //turn off AEA
                psap_aea_config_t aea_config = {  .aea_switch = 0,
                                                .aea_nr_switch = usr_setting_ptr->psap_aea_config_t.aea_nr_switch,
                                                .aea_nr_level = usr_setting_ptr->psap_aea_config_t.aea_nr_level,
                                                .aea_det_period = usr_setting_ptr->psap_aea_config_t.aea_det_period};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG;

                U32 aea_config_len = sizeof(psap_aea_config_t);
                para_len = config_len + aea_config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &aea_config, aea_config_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (usr_setting_ptr->how_det_enable) {
                //turn off HD
                psap_how_det_t how_det = {.how_det_enable = 0,
                                        .sup_han = usr_setting_ptr->sup_han};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_HOWLING_DET;
                config.setting = 0;

                U32 how_config_len = sizeof(psap_how_det_t);
                para_len = config_len + how_config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &how_det, how_config_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }

        } else {//disable tuning mode
            if (now_afc_switch) {
                //turn on AFC
                psap_afc_config_t afc_config = {.afc_ctrl_switch_l = usr_setting_ptr->psap_afc_config_t.afc_ctrl_switch_l,
                                              .afc_ctrl_switch_r = usr_setting_ptr->psap_afc_config_t.afc_ctrl_switch_r};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG;
                config.setting = (U32)*((U8*)&afc_config);

                para_len = config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (now_inr_switch) {
                //turn on INR
                ha_inr_config_t INR_config = {  .inr_switch_l = usr_setting_ptr->ha_inr_config_t.inr_switch_l,
                                                .inr_sensitivity_l = usr_setting_ptr->ha_inr_config_t.inr_sensitivity_l,
                                                .inr_strength_l = usr_setting_ptr->ha_inr_config_t.inr_strength_l,
                                                .inr_switch_r = usr_setting_ptr->ha_inr_config_t.inr_switch_r,
                                                .inr_sensitivity_r = usr_setting_ptr->ha_inr_config_t.inr_sensitivity_r,
                                                .inr_strength_r = usr_setting_ptr->ha_inr_config_t.inr_strength_r};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_INR_CONFIG;

                U32 inr_config_len = sizeof(ha_inr_config_t);
                para_len = config_len + inr_config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);
                malloc_ptr += config_len;
                memcpy(malloc_ptr, &INR_config, inr_config_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (now_mfa_switch) {
                //turn on MFA
                psap_mode_nvdm_t mode_config = {  .mfa_switch_l = now_mode_config->mfa_switch_l,
                                                .mfa_switch_r = now_mode_config->mfa_switch_r,
                                                .low_cut_switch_l = now_mode_config->low_cut_switch_l,
                                                .low_cut_switch_r = now_mode_config->low_cut_switch_r,
                                                .nr_switch = now_mode_config->nr_switch ,
                                                .nr_level = now_mode_config->nr_level,
                                                .beamforming_switch = now_mode_config->beamforming_switch};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_MODE_TABLE;
                config.setting = usr_setting_ptr->mode_index;

                U32 mode_table_len = sizeof(psap_mode_table_t);
                para_len = config_len + mode_table_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &mode_config, mode_table_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (usr_setting_ptr->psap_bf_config_t.bf_switch) {
                //turn on BF
                psap_bf_config_t BF_config = {.bf_switch = usr_setting_ptr->psap_bf_config_t.bf_switch,
                                            .bf_switch_mode_ctrl = usr_setting_ptr->psap_bf_config_t.bf_switch_mode_ctrl};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_BF_CONFIG;
                config.setting = (U32)*((U8*)&BF_config);

                para_len = config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
            if (usr_setting_ptr->WNR_switch) {
                //turn on WNR
                config.config_event = HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH;
                config.setting = (U32)usr_setting_ptr->WNR_switch;

                para_len = config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            }
            if (usr_setting_ptr->psap_aea_config_t.aea_switch) {
                //turn on AEA
                psap_aea_config_t aea_config = {  .aea_switch = usr_setting_ptr->psap_aea_config_t.aea_switch,
                                                .aea_nr_switch = usr_setting_ptr->psap_aea_config_t.aea_nr_switch,
                                                .aea_nr_level = usr_setting_ptr->psap_aea_config_t.aea_nr_level,
                                                .aea_det_period = usr_setting_ptr->psap_aea_config_t.aea_det_period};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG;

                U32 aea_config_len = sizeof(psap_aea_config_t);
                para_len = config_len + aea_config_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &aea_config, aea_config_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
            if (usr_setting_ptr->how_det_enable) {
                //turn off HD
                psap_how_det_t how_det = {.how_det_enable = usr_setting_ptr->how_det_enable,
                                        .sup_han = usr_setting_ptr->sup_han};
                config.config_event = HA_RUNTIME_CONFIG_EVENT_HOWLING_DET;
                config.setting = 0;

                U32 how_det_len = sizeof(psap_how_det_t);
                para_len = config_len + how_det_len;
                malloc_ptr = pvPortMalloc(para_len);
                psap_cap.param = (U32)malloc_ptr;
                psap_cap.param_len = para_len;

                memcpy(malloc_ptr, &config, config_len);

                malloc_ptr += config_len;
                memcpy(malloc_ptr, &how_det, how_det_len);

                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }


        }

    }

    ctrl_para->tuning_mode_ctrl = *tuning_switch;
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_tuning_mode(U8 *mode_switch)//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved
{
    if (mode_switch) {
        *mode_switch = *((U8*)&g_ha_ctrl.ctrl_para.tuning_mode_ctrl);
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_mp_test_mode(U8 mode_switch)//0x00(Off), 0x01(On)
{
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }
    LOGMSGIDI("set MP test mode, role:%d, mode_switch:%d", 2, g_ha_ctrl.role, mode_switch);
    if (!g_ha_ctrl.ha_enable) {
        audio_anc_psap_control_enable();
    }

    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;

    U32 config_len = sizeof(llf_control_runtime_config_t);
    U32 para_len = config_len;
    void *malloc_ptr = NULL;
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE_HEARING_AID, 0, 0, 0, 0, 0, 0};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE_HEARING_AID, 0, 0};
    U8 to_switch, now_afc_switch, now_inr_switch, now_mfa_switch, now_drc_level_index;
    psap_usr_setting_nvdm_t *usr_setting = &(g_ha_ctrl.usr_setting);
    psap_mode_nvdm_t* now_mode_config = &(usr_setting->psap_mode[usr_setting->mode_index]);

    if (!(mode_switch ^ g_ha_ctrl.ctrl_para.mp_test_switch)) {
        return res;
    }
    if (mode_switch) {
        audio_anc_psap_control_save_setting();
    } else {
        U32 nvkey_length = 0;
        sysram_status_t nvdm_status;
        nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_CUS_SETTING, &nvkey_length);
        nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING, (uint8_t *)usr_setting, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
            assert(0 && "Read Nvkey data Fail id:0xE825");
        }
    }
    if (g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        to_switch = mode_switch;
        now_afc_switch = usr_setting->psap_afc_config_t.afc_ctrl_switch_l;
        now_inr_switch = usr_setting->ha_inr_config_t.inr_switch_l;
        now_mfa_switch = now_mode_config->mfa_switch_l;
        now_drc_level_index = usr_setting->drc_level_index_l;
    } else {
        to_switch = mode_switch;
        now_afc_switch = usr_setting->psap_afc_config_t.afc_ctrl_switch_r;
        now_inr_switch = usr_setting->ha_inr_config_t.inr_switch_r;
        now_mfa_switch = now_mode_config->mfa_switch_r;
        now_drc_level_index = usr_setting->drc_level_index_r;
    }



    if (to_switch) { //enable MP mode

        if (now_drc_level_index != g_ha_ctrl.drc_setting.drc_Level_index_MP_test_mode) {
            audio_anc_psap_control_set_level_index(g_ha_ctrl.drc_setting.drc_Level_index_MP_test_mode, g_ha_ctrl.drc_setting.drc_Level_index_MP_test_mode, g_ha_ctrl.role);
        }
        if (now_afc_switch) {
            //turn off AFC
            config.config_event = HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG;
            config.setting = 0;

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            usr_setting->psap_afc_config_t.afc_ctrl_switch_l = 0;
            usr_setting->psap_afc_config_t.afc_ctrl_switch_r = 0;
        }
        if (now_inr_switch) {
            //turn off INR
            ha_inr_config_t *INR_config_ptr = (ha_inr_config_t*)&(usr_setting->ha_inr_config_t);
            ha_inr_config_t INR_config = {  .inr_switch_l = 0,
                                            .inr_sensitivity_l = INR_config_ptr->inr_sensitivity_l,
                                            .inr_strength_l = INR_config_ptr->inr_strength_l,
                                            .inr_switch_r = 0,
                                            .inr_sensitivity_r = INR_config_ptr->inr_sensitivity_r,
                                            .inr_strength_r = INR_config_ptr->inr_strength_r};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_INR_CONFIG;

            U32 inr_config_len = sizeof(llf_control_runtime_config_t);
            para_len = config_len + inr_config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            malloc_ptr += config_len;
            memcpy(malloc_ptr, &INR_config, inr_config_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            INR_config_ptr->inr_switch_l = 0;
            INR_config_ptr->inr_switch_r = 0;
        }
        if (now_mfa_switch | now_mode_config->nr_switch | now_mode_config->beamforming_switch) {
            //turn off MFA/NR/BF in mode table
            psap_mode_nvdm_t mode_config = {  .mfa_switch_l = 0,
                                            .mfa_switch_r = 0,
                                            .low_cut_switch_l = now_mode_config->low_cut_switch_l,
                                            .low_cut_switch_r = now_mode_config->low_cut_switch_r,
                                            .nr_switch = 0,
                                            .nr_level = now_mode_config->nr_level,
                                            .beamforming_switch = 0};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_MODE_TABLE;
            config.setting = usr_setting->mode_index;

            U32 mode_table_len = sizeof(psap_mode_table_t);
            para_len = config_len + mode_table_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &mode_config, mode_table_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            now_mode_config->mfa_switch_l = 0;
            now_mode_config->mfa_switch_r = 0;
            now_mode_config->nr_switch = 0;
            now_mode_config->beamforming_switch = 0;
        }
        if (usr_setting->psap_bf_config_t.bf_switch) {
            //turn off BF
            config.config_event = HA_RUNTIME_CONFIG_EVENT_BF_CONFIG;
            config.setting = 0;

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            usr_setting->psap_bf_config_t.bf_switch = 0;
            usr_setting->psap_bf_config_t.bf_switch_mode_ctrl = 0;
        }
        if (usr_setting->WNR_switch) {
            //turn off WNR
            config.config_event = HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH;
            config.setting = 0;

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            usr_setting->WNR_switch = 0;
        }
        if (usr_setting->psap_aea_config_t.aea_switch) {
            //turn off AEA
            psap_aea_config_t *aea_config_ptr = (psap_aea_config_t*)&(usr_setting->psap_aea_config_t);
            psap_aea_config_t aea_config = {  .aea_switch = 0,
                                            .aea_nr_switch = aea_config_ptr->aea_nr_switch,
                                            .aea_nr_level = aea_config_ptr->aea_nr_level,
                                            .aea_det_period = aea_config_ptr->aea_det_period};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG;

            U32 aea_config_len = sizeof(psap_aea_config_t);
            para_len = config_len + aea_config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &aea_config, aea_config_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            aea_config_ptr->aea_switch = 0;
        }
        if (usr_setting->how_det_enable) {
            //turn off HD
            psap_how_det_t how_det = {.how_det_enable = 0,
                                    .sup_han = usr_setting->sup_han};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_HOWLING_DET;

            U32 how_det_len = sizeof(psap_how_det_t);
            para_len = config_len + how_det_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;


            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &how_det, how_det_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            usr_setting->how_det_enable = 0;
        }

        //turn off mix mode
        psap_scenario_mix_mode_t *mix_mode_para_ptr = (psap_scenario_mix_mode_t*)&(usr_setting->psap_scenario_mix_mode_t);
        psap_scenario_mix_mode_t mix_mode_para = {0};
        config.config_event = HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG;

        U32 mix_mode_len = sizeof(psap_scenario_mix_mode_t);
        para_len = config_len + mix_mode_len;
        malloc_ptr = pvPortMalloc(para_len);
        psap_cap.param = (U32)malloc_ptr;
        psap_cap.param_len = para_len;

        memcpy(malloc_ptr, &config, config_len);

        malloc_ptr += config_len;
        memcpy(malloc_ptr, &mix_mode_para, mix_mode_len);

        res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

        *mix_mode_para_ptr = mix_mode_para;

    } else {//disable MP mode
        if (now_drc_level_index != g_ha_ctrl.drc_setting.drc_Level_index_MP_test_mode) {
            config.config_event = HA_RUNTIME_CONFIG_EVENT_LEVEL_IND;
            config.setting = (U32)now_drc_level_index;

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

        }
        if (now_afc_switch) {
            //turn on AFC
            psap_afc_config_t afc_config = {.afc_ctrl_switch_l = usr_setting->psap_afc_config_t.afc_ctrl_switch_l,
                                          .afc_ctrl_switch_r = usr_setting->psap_afc_config_t.afc_ctrl_switch_r};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG;
            config.setting = (U32)*((U8*)&afc_config);

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (now_inr_switch) {
            //turn on INR
            ha_inr_config_t *INR_config_ptr = (ha_inr_config_t*)&(usr_setting->ha_inr_config_t);
            ha_inr_config_t INR_config = {  .inr_switch_l = INR_config_ptr->inr_switch_l,
                                            .inr_sensitivity_l = INR_config_ptr->inr_sensitivity_l,
                                            .inr_strength_l = INR_config_ptr->inr_strength_l,
                                            .inr_switch_r = INR_config_ptr->inr_switch_r,
                                            .inr_sensitivity_r = INR_config_ptr->inr_sensitivity_r,
                                            .inr_strength_r = INR_config_ptr->inr_strength_r};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_INR_CONFIG;

            U32 inr_config_len = sizeof(ha_inr_config_t);
            para_len = config_len + inr_config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            malloc_ptr += config_len;
            memcpy(malloc_ptr, &INR_config, inr_config_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (now_mfa_switch) {
            //turn on MFA
            psap_mode_nvdm_t mode_config = {  .mfa_switch_l = now_mode_config->mfa_switch_l,
                                            .mfa_switch_r = now_mode_config->mfa_switch_r,
                                            .low_cut_switch_l = now_mode_config->low_cut_switch_l,
                                            .low_cut_switch_r = now_mode_config->low_cut_switch_r,
                                            .nr_switch = now_mode_config->nr_switch ,
                                            .nr_level = now_mode_config->nr_level,
                                            .beamforming_switch = now_mode_config->beamforming_switch};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_MODE_TABLE;
            config.setting = usr_setting->mode_index;

            U32 mode_table_len = sizeof(psap_mode_table_t);
            para_len = config_len + mode_table_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &mode_config, mode_table_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (usr_setting->psap_bf_config_t.bf_switch) {
            //turn on BF
            psap_bf_config_t BF_config = {.bf_switch = usr_setting->psap_bf_config_t.bf_switch,
                                        .bf_switch_mode_ctrl = usr_setting->psap_bf_config_t.bf_switch_mode_ctrl};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_BF_CONFIG;
            config.setting = (U32)*((U8*)&BF_config);

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (usr_setting->WNR_switch) {
            //turn on WNR
            config.config_event = HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH;
            config.setting = (U32)usr_setting->WNR_switch;

            para_len = config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);
            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (usr_setting->psap_aea_config_t.aea_switch) {
            //turn on AEA
            psap_aea_config_t *aea_config_ptr = (psap_aea_config_t*)&(usr_setting->psap_aea_config_t);
            psap_aea_config_t aea_config = {  .aea_switch = aea_config_ptr->aea_switch,
                                            .aea_nr_switch = aea_config_ptr->aea_nr_switch,
                                            .aea_nr_level = aea_config_ptr->aea_nr_level,
                                            .aea_det_period = aea_config_ptr->aea_det_period};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG;

            U32 aea_config_len = sizeof(psap_aea_config_t);
            para_len = config_len + aea_config_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &aea_config, aea_config_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
        if (usr_setting->how_det_enable) {
            //turn on HD
            psap_how_det_t how_det = {.how_det_enable = usr_setting->how_det_enable,
                                    .sup_han = usr_setting->sup_han};
            config.config_event = HA_RUNTIME_CONFIG_EVENT_HOWLING_DET;
            config.setting = 0;

            U32 how_det_len = sizeof(psap_how_det_t);
            para_len = config_len + how_det_len;
            malloc_ptr = pvPortMalloc(para_len);
            psap_cap.param = (U32)malloc_ptr;
            psap_cap.param_len = para_len;

            memcpy(malloc_ptr, &config, config_len);

            malloc_ptr += config_len;
            memcpy(malloc_ptr, &how_det, how_det_len);

            res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }

        //turn on mix mode
        psap_scenario_mix_mode_t *mix_mode_para_ptr = (psap_scenario_mix_mode_t*)&(usr_setting->psap_scenario_mix_mode_t);
        config.config_event = HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG;

        U32 mix_mode_len = sizeof(psap_scenario_mix_mode_t);
        para_len = config_len + mix_mode_len;
        malloc_ptr = pvPortMalloc(para_len);
        psap_cap.param = (U32)malloc_ptr;
        psap_cap.param_len = para_len;

        memcpy(malloc_ptr, &config, config_len);

        malloc_ptr += config_len;
        memcpy(malloc_ptr, mix_mode_para_ptr, mix_mode_len);

        res |= llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }

    g_ha_ctrl.ctrl_para.mp_test_switch = to_switch;

    return res;

}

audio_psap_status_t audio_anc_psap_control_get_mp_test_mode(U8 *mode_switch)//0x00(Off), 0x01(On)
{
    if (mode_switch) {
        *mode_switch = g_ha_ctrl.ctrl_para.mp_test_switch;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_restore_setting(U8 mode)
{
    U32 nvkey_length = 0;
    sysram_status_t nvdm_status;
    psap_usr_setting_nvdm_t restore_setting;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;

    LOGMSGIDI("set restore setting, mode:%d", 1, mode);

    nvdm_status =  flash_memory_query_nvdm_data_length(NVID_DSP_ALG_HA_CUS_SETTING_BK, &nvkey_length);
    if (nvdm_status || !nvkey_length || (nvkey_length != HA_NV_SIZE_USR)) {
        //LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
        assert(0 && "Read Nvkey length Fail or length mismatch, id:0xE826");
    }
    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING_BK, (uint8_t *)&restore_setting, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        //LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_HA_CUS_SETTING, nvdm_status);
        assert(0 && "Read Nvkey data Fail id:0xE826");
    }

    if (mode == 1) {//restore other setting
        restore_setting.drc_level_index_l = usr_setting->drc_level_index_l;
        restore_setting.drc_level_index_r = usr_setting->drc_level_index_r;
        memcpy(&restore_setting.psap_user_eq_switch_t, &(usr_setting->psap_user_eq_switch_t), sizeof(psap_user_eq_switch_t) + sizeof(psap_usr_eq_para_t));
    }

    memcpy(usr_setting, &restore_setting, HA_NV_SIZE_USR);

    nvdm_status = flash_memory_write_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING, (uint8_t*)usr_setting, HA_NV_SIZE_USR);

    audio_anc_psap_control_update_mic_input_path();

    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_detect_feedback(void)
{
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }
    LOGMSGIDI("detect feedback, callback:0x%x, MP_test_mode:%d", 2, g_ha_ctrl.noti_callback, g_ha_ctrl.ctrl_para.mp_test_switch);

    g_ha_ctrl.fb_det_enable = 1;
    U32 para_len = sizeof(llf_control_runtime_config_t);
    void *malloc_ptr = pvPortMalloc(para_len);
    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_DET_FB, 0};


    memcpy(malloc_ptr, &config, para_len);

    //g_ha_ctrl.detect_fb_callback = callback;

    return llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    return AUDIO_PSAP_STATUS_FAIL;
}

audio_psap_status_t audio_anc_psap_control_set_mute(bool mute_l, bool mute_r)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para_ptr = &g_ha_ctrl.ctrl_para;
    audio_psap_device_role_t role = g_ha_ctrl.role;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;
    LOGMSGIDI("set mute:L(%d) R(%d)", 2, mute_l, mute_r);
    LOGMSGIDI("sizeof bool(%d)", 2, sizeof(bool));
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }
    //mute: 0:off, 1:on
    if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (ctrl_para_ptr->mute_l != mute_l))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (ctrl_para_ptr->mute_r != mute_r))) {

        U32 para_len = sizeof(llf_control_runtime_config_t);
        void *malloc_ptr = pvPortMalloc(para_len);
        llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
        llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MUTE, (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) ? mute_l : mute_r};


        memcpy(malloc_ptr, &config, para_len);

        res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }
    ctrl_para_ptr->mute_l = mute_l;
    ctrl_para_ptr->mute_r = mute_r;
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_mute(bool *mute_l, bool *mute_r)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para_ptr = &g_ha_ctrl.ctrl_para;
    if (mute_l) {
        *mute_l = ctrl_para_ptr->mute_l;
    }
    if (mute_r) {
        *mute_r = ctrl_para_ptr->mute_r;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_howling_detection(psap_how_det_t *how_det)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    if (!g_ha_ctrl.framework_enable | !g_ha_ctrl.share_addr) {
        return AUDIO_PSAP_STATUS_FAIL;
    }
    if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (how_det->how_det_enable & (ctrl_para->tuning_mode_ctrl.switch_l)))
        || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (how_det->how_det_enable & (ctrl_para->tuning_mode_ctrl.switch_r)))
        || (ctrl_para->mp_test_switch != 0)) {
            LOGMSGIDE("set howling detection fail, tuning_mode(%d), MP_test_mode(%d)", 2, *((U8*)&(ctrl_para->tuning_mode_ctrl)), ctrl_para->mp_test_switch);
            return AUDIO_PSAP_STATUS_FAIL;
    }

    if (how_det) {
        LOGMSGIDI("set howling detection, en:%d, 0x%x", 2, how_det->how_det_enable, how_det->sup_han);
        g_ha_ctrl.usr_setting.how_det_enable = how_det->how_det_enable;
        g_ha_ctrl.usr_setting.sup_han = how_det->sup_han;
        res = AUDIO_PSAP_STATUS_SUCCESS;

        U32 config_len = sizeof(llf_control_runtime_config_t);
        U32 how_det_len = sizeof(psap_how_det_t);
        U32 para_len = config_len + how_det_len;
        void *malloc_ptr = pvPortMalloc(para_len);
        llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
        llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_HOWLING_DET, 0};

        memcpy(malloc_ptr, &config, config_len);

        malloc_ptr += config_len;
        memcpy(malloc_ptr, how_det, how_det_len);

        res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }

    return res;
}

audio_psap_status_t audio_anc_psap_control_get_howling_detection(psap_how_det_t *how_det)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para = &g_ha_ctrl.ctrl_para;
    if (how_det) {
        if (((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (ctrl_para->tuning_mode_ctrl.switch_l))
        || ((g_ha_ctrl.role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (ctrl_para->tuning_mode_ctrl.switch_r))
        || (ctrl_para->mp_test_switch != 0)) {
            how_det->how_det_enable = 0;
        } else {
            how_det->how_det_enable = g_ha_ctrl.usr_setting.how_det_enable;
        }
        how_det->sup_han = g_ha_ctrl.usr_setting.sup_han;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

llf_status_t audio_anc_psap_control_set_mpo_adjustment(ha_mpo_adjust_t *mpo_adj)
{
    return AUDIO_PSAP_STATUS_SUCCESS;
}

llf_status_t audio_anc_psap_control_get_mpo_adjustment(ha_mpo_adjust_t *mpo_adj)
{
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_mic_channel(U8 channel)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    psap_usr_setting_nvdm_t *usr_setting = &g_ha_ctrl.usr_setting;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    U8 bf_switch_tmp = 0;

    if (usr_setting->psap_bf_config_t.bf_switch_mode_ctrl) {
        bf_switch_tmp = (U8)usr_setting->psap_mode[usr_setting->mode_index].beamforming_switch;

    } else if (usr_setting->psap_bf_config_t.bf_switch) {
        bf_switch_tmp = (U8)usr_setting->psap_bf_config_t.bf_switch;
    }
    LOGMSGIDI("set mic ch(%d), bf_switch(%d)", 2, channel, bf_switch_tmp);

    if (channel <= 2) {
        usr_setting->psap_master_mic_ch = channel;
        audio_anc_psap_control_save_setting();
        res = AUDIO_PSAP_STATUS_SUCCESS;

        if (usr_setting->WNR_switch | bf_switch_tmp) { // 2 mic
            mic_ctrl->ff_enable = 1;
            mic_ctrl->talk_enable = 1;
        #if !defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
            if (g_ha_ctrl.framework_enable) {
                U32 para_len = sizeof(llf_control_runtime_config_t);
                void *malloc_ptr = pvPortMalloc(para_len);
                llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
                llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL, channel};

                memcpy(malloc_ptr, &config, para_len);
                res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
            }
        #endif
        } else {

            if (usr_setting->psap_master_mic_ch == 2) {
                mic_ctrl->ff_enable = 0;
                mic_ctrl->talk_enable = 1;
            } else {
                mic_ctrl->ff_enable = 1;
                mic_ctrl->talk_enable = 0;
            }
        }
    #if defined(HAL_AUDIO_PSAP_SEAMLESS_SWITCH_ENABLE)
        U32 mic_input_path;
        audio_anc_control_filter_id_t      target_filter_id;
        audio_anc_control_type_t           target_anc_type;
        audio_anc_psap_control_get_mic_input_path(&mic_input_path);
        target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_input_path;
        target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
        anc_psap_control_mic(mic_ctrl->ff_enable << FF_ENABLE | mic_ctrl->fb_enable << FB_ENABLE | mic_ctrl->talk_enable << TALK_ENABLE, target_anc_type, target_filter_id);
    
        if (g_ha_ctrl.framework_enable) {
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL, channel};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    #endif
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_mic_channel(U8 *channel)
{
    if (channel) {
        *channel = g_ha_ctrl.usr_setting.psap_master_mic_ch;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_set_inear_detect_switch(U8 enable)
{
    if (enable <= 1) {
        //g_ha_ctrl.usr_setting.inear_detect_switch = enable;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_get_inear_detect_switch(U8 *enable)
{
    if (enable) {
        //*enable = g_ha_ctrl.usr_setting.inear_detect_switch;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_save_setting(void)
{
    sysram_status_t nvdm_status;
    U8 mp_test_mode;


#if 1
    uint32_t i, size;
    uint8_t factrst_flag;;
    nvkey_status_t nvkey_status;

    size = sizeof(factrst_flag);
    nvkey_status = nvkey_read_data(NVID_SYS_FACTORY_RESET_FLAG, &factrst_flag, &size);
    if (nvkey_status == NVKEY_STATUS_OK) {

    } else {
         LOGMSGIDE("Wrong para, audio_anc_psap_control_save_setting read nvk fail", 0);
   
    }
   
#endif
    audio_anc_psap_control_get_mp_test_mode(&mp_test_mode);
    LOGMSGIDI("audio_anc_psap_control_save_setting, mp_test_mode:0x%x,factrst_flag=0x%x,mode_index=%d", 3, mp_test_mode,factrst_flag,g_ha_ctrl.usr_setting.mode_index);
    if (mp_test_mode) {
        return AUDIO_PSAP_STATUS_SUCCESS;
    }
    if (g_ha_ctrl.usr_setting.psap_master_mic_ch == 0) {
        LOGMSGIDE("Wrong para, psap_master_mic_ch:%d, psap_vol_index_l:%d, psap_vol_index_r:%d", 3, g_ha_ctrl.usr_setting.psap_master_mic_ch, g_ha_ctrl.usr_setting.psap_vol_index_l, g_ha_ctrl.usr_setting.psap_vol_index_r);
        return AUDIO_PSAP_STATUS_FAIL;
    }
    if (factrst_flag == FACTORY_RESET_FLAG)   
    {  
    audio_anc_psap_control_set_restore_setting(0);    
    
	// g_ha_ctrl.usr_setting.mode_index=0;
	// g_ha_ctrl.usr_setting.psap_vol_index_l= 4;
	// g_ha_ctrl.usr_setting.psap_vol_index_r= 4;

    // g_ha_ctrl.usr_setting.psap_afc_config_t.afc_ctrl_switch_l = 1;  //harry 0611
    // g_ha_ctrl.usr_setting.psap_afc_config_t.afc_ctrl_switch_r = 1;

    }

    nvdm_status = flash_memory_write_nvdm_data(NVID_DSP_ALG_HA_CUS_SETTING, (uint8_t*)&g_ha_ctrl.usr_setting, HA_NV_SIZE_USR);

    if (nvdm_status == NVDM_STATUS_NAT_OK) {
        return AUDIO_PSAP_STATUS_SUCCESS;
    } else {
        LOGMSGIDE("save customer setting FAIL:%d", 1, nvdm_status);
        return AUDIO_PSAP_STATUS_FAIL;
    }
}

void audio_anc_psap_control_set_runtime_sync_parameter(uint16_t len, uint8_t* para)
{
    ha_runtime_sync_para_t *runtime_sync_para = (ha_runtime_sync_para_t*)para;
    U32 ha_switch = runtime_sync_para->ctrl_para.ha_switch;
    ha_mic_ctrl *mic_ctrl = &g_ha_ctrl.mic_ctrl;
    ha_mic_ctrl pre_mic_ctrl;

    if ((len == HA_RUNTIME_SYNC_PARA_LEN) & (para != NULL)) {
        memcpy(&pre_mic_ctrl, mic_ctrl, sizeof(ha_mic_ctrl));
        memcpy((uint8_t*)&g_ha_ctrl.ctrl_para, para, len);
        LOGMSGIDI("sync parameter SUCCESS, 0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 16, *para, *(para + 1), *(para + 2), *(para + 3), *(para + 4), *(para + 5),
                   *(para + 6), *(para + 7), *(para + 8), *(para + 9), *(para + 10), *(para + 11), *(para + 12), *(para + 13), *(para + 14), *(para + 15));
        // g_ha_ctrl.ha_enable = ha_switch;
        g_ha_ctrl.ctrl_para.ha_switch = ha_switch;
        g_ha_ctrl.is_sync_partner = 1;
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
        /* phase 2.0 */
//        hal_audio_status_send_update_dac_mode_event_to_am(0,0);
#endif
        audio_anc_psap_control_update_mic_input_path();
        if (g_ha_ctrl.framework_enable &
            (!(pre_mic_ctrl.ff_enable ^ mic_ctrl->ff_enable) &
             !(pre_mic_ctrl.fb_enable ^ mic_ctrl->fb_enable) &
             !(pre_mic_ctrl.talk_enable ^ mic_ctrl->talk_enable))) {
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, 0};
            llf_control(LLF_CONTROL_EVENT_UPDATE_PARA, &psap_cap);
        }

    } else {
        LOGMSGIDE("sync parameter FAIL, len:%d", 1, len);
    }
}

uint8_t* audio_anc_psap_control_get_runtime_sync_parameter(uint16_t *len)
{
    if (len) {
        ha_runtime_sync_para_t *runtime_sync_para = (ha_runtime_sync_para_t*)pvPortMalloc(HA_RUNTIME_SYNC_PARA_LEN);
        memcpy(runtime_sync_para, &g_ha_ctrl.ctrl_para, HA_RUNTIME_SYNC_PARA_LEN);
        *len = HA_RUNTIME_SYNC_PARA_LEN;
        LOGMSGIDI("get sync parameter, mode ind:%d, level(%d %d)", 3, g_ha_ctrl.usr_setting.mode_index, g_ha_ctrl.usr_setting.drc_level_index_l, g_ha_ctrl.usr_setting.drc_level_index_r);
        return (uint8_t*)runtime_sync_para;
    }
    return NULL;
}

audio_psap_status_t audio_anc_psap_control_set_passthrough_switch(bool enable_l, bool enable_r)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para_ptr = &g_ha_ctrl.ctrl_para;
    audio_psap_device_role_t role = g_ha_ctrl.role;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;

    LOGMSGIDI("set passthrough: L(%d) R(%d)", 2, enable_l, enable_r);

    if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (g_ha_ctrl.ctrl_para.passthru_switch_l != enable_l))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (g_ha_ctrl.ctrl_para.passthru_switch_r != enable_r))) {

        if (g_ha_ctrl.framework_enable && (g_ha_ctrl.share_addr != NULL)) {
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_PASSTHRU_SWITCH, (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) ? enable_l : enable_r};

            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }
    ctrl_para_ptr->passthru_switch_l = enable_l;
    ctrl_para_ptr->passthru_switch_r = enable_r;
    return res;
}

audio_psap_status_t audio_anc_psap_control_get_passthrough_switch(bool *enable_l, bool *enable_r)
{
    ha_runtime_sync_ctrl_para_t *ctrl_para_ptr = &g_ha_ctrl.ctrl_para;

    if (enable_l) {
        *enable_l = ctrl_para_ptr->passthru_switch_l;
    }
    if (enable_r) {
        *enable_r = ctrl_para_ptr->passthru_switch_r;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_register_notification_callback(psap_noti_callback_t callback)
{
    if (callback) {
        g_ha_ctrl.noti_callback = callback;
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}

void audio_anc_psap_control_senario_notification(llf_scenario_change_event_t scenario, bool is_on)
{
    U32 mask = 1 << scenario;
    U32 pre_state = g_ha_ctrl.senario_state & mask;
    U32 cur_state = is_on? mask : 0;
    llf_running_status_t running;
    llf_type_t type;
    U8 sub_mode;
    if (scenario <= LLF_SCENARIO_CHANGE_UL_MAX) {
        if (pre_state ^ cur_state) {
            llf_control_get_status(&running, &type, &sub_mode, NULL);
            if ((running < LLF_RUNNING_STATUS_RESET) && (type == LLF_TYPE_HEARING_AID) && (sub_mode == HA_SUB_MODE)) {
                //audio_anc_psap_control_reset_framework();
            }
            g_ha_ctrl.senario_state = (g_ha_ctrl.senario_state & ~mask) | (cur_state & mask);
            LOGMSGIDI("scenario state:0x%x", 1, g_ha_ctrl.senario_state);
        }
    }
}

audio_psap_status_t audio_anc_psap_control_disable_internal(void)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_SUCCESS;

    if (g_ha_ctrl.framework_enable) {
        void *malloc_ptr = NULL;
        U32 para_len = sizeof(llf_control_runtime_config_t);
        malloc_ptr = pvPortMalloc(para_len);

        llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
        llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_FADE_OUT, 0};

        memcpy(malloc_ptr, &config, sizeof(llf_control_runtime_config_t));
        res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_disable_extend_internal(uint8_t fwk_action)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    if (g_ha_ctrl.framework_enable) {
        //fade out
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
#if defined(HAL_DVFS_MODULE_ENABLED)
#if AIR_BTA_IC_PREMIUM_G3
        hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
        LOGMSGIDI("[RECORD_LC]speed is risen to 260M", 0);
#endif
#endif

    bool need_sync = app_hear_through_activity_is_out_case() ?
                     (app_hearing_aid_activity_is_power_off() ? false : true) : false;

    audio_anc_psap_control_mute_dl(true, need_sync, 30000);
    xTimerChangePeriod(g_psap_one_shot_timer, g_ha_ctrl.dl_mute_dur_ha_off / portTICK_PERIOD_MS, 0);

#endif
        audio_anc_psap_control_disable_internal();

        llf_control_cap_t psap_cap = {  LLF_TYPE_HEARING_AID,
                                        HA_SUB_MODE,
                                        HA_FRAME_SIZE, HA_BUFFER_LEN,
                                        audio_anc_psap_control_stream_handler,
                                        500,//delay 200 ms
                                        0};
        if (fwk_action == 1) {
            //MIC switch, reopen framework
            res = llf_control(LLF_CONTROL_EVENT_RESET, &psap_cap);
        } else {
            //OFF
            res = llf_control(LLF_CONTROL_EVENT_OFF, &psap_cap);
        }
    }
    return res;
}

audio_psap_status_t audio_anc_psap_control_trial_run(ha_trial_run_event_t event, U32 data_len, void* data)
{
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    LOGMSGIDI("Trial Run event(%d), len(%d)", 2, event, data_len);
    if (data_len > (SHARE_BUFFER_LLF_INFO_COUNT * 4)) {
        return res;
    }

    llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)g_ha_ctrl.share_addr};
    llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_TRIAL_RUN, ((U32)event << 16) | (data_len & 0xFF)};

    U8 *share_addr = (U8*)g_ha_ctrl.share_addr;

    if (g_ha_ctrl.framework_enable && (share_addr != NULL)) {
        if (llf_control_query_share_buffer() == pdPASS) {
            memcpy(share_addr, &config, sizeof(llf_control_runtime_config_t));
            if (data_len) {
                share_addr += sizeof(llf_control_runtime_config_t);
                memcpy(share_addr, data, data_len);
            }
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);
        }
    }


    return res;
}

audio_psap_status_t audio_anc_psap_control_set_mic_cal_mode(U8 *mic_cal_mode)
{
    ha_mic_cal_t *mic_cal_para = (ha_mic_cal_t*)mic_cal_mode;
    audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;
    U32 apply_val = (ami_get_audio_channel() == AUDIO_CHANNEL_L) ? (U32)mic_cal_para->mic_cal_mode_l : (U32)mic_cal_para->mic_cal_mode_r;

    LOGMSGIDI("set mic cal mode(%d %d)", 2, mic_cal_para->mic_cal_mode_l, mic_cal_para->mic_cal_mode_r);

    if (g_ha_ctrl.framework_enable) {
        U32 para_len = sizeof(llf_control_runtime_config_t);
        void *malloc_ptr = pvPortMalloc(para_len);
        llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
        llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_SET_MIC_CAL_MODE, apply_val};

        memcpy(malloc_ptr, &config, para_len);
        res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

    }
    g_ha_ctrl.mic_cal_mode_l = mic_cal_para->mic_cal_mode_l;
    g_ha_ctrl.mic_cal_mode_r = mic_cal_para->mic_cal_mode_r;

    return res;
}

audio_psap_status_t audio_anc_psap_control_get_mic_cal_mode(U8 *mic_cal_mode, U8 *mic_cal_mode_len)
{
    U8 para_len = sizeof(ha_mic_cal_t);
    ha_mic_cal_t mic_cal = { .mic_cal_mode_l = g_ha_ctrl.mic_cal_mode_l,
                             .mic_cal_mode_r = g_ha_ctrl.mic_cal_mode_r};
    if (mic_cal_mode_len) {
        *mic_cal_mode_len = para_len;
    }
    if (mic_cal_mode) {
        memcpy(mic_cal_mode, &mic_cal, para_len);
    }
    LOGMSGIDI("get mic cal mode, mic_cal_mode(%d %d), len(%d)", 3, mic_cal.mic_cal_mode_l, mic_cal.mic_cal_mode_r, para_len);
    return AUDIO_PSAP_STATUS_SUCCESS;
}

audio_psap_status_t audio_anc_psap_control_get_mic_cal_data(U32 *len, U8* data)
{
    if ((len != NULL) && (data != NULL)) {
        audio_psap_status_t res = AUDIO_PSAP_STATUS_FAIL;

        LOGMSGIDI("get mic cal data", 0);
        if (g_ha_ctrl.framework_enable) {
            U32 para_len = sizeof(llf_control_runtime_config_t);
            void *malloc_ptr = pvPortMalloc(para_len);
            llf_control_cap_t psap_cap = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, 0, 0, 0, 0, (U32)malloc_ptr, para_len};
            llf_control_runtime_config_t config = {LLF_TYPE_HEARING_AID, HA_SUB_MODE, HA_RUNTIME_CONFIG_EVENT_GET_MIC_CAL_DATA, 0};

            llf_control_query_data_wait();
            memcpy(malloc_ptr, &config, para_len);
            res = llf_control(LLF_CONTROL_EVENT_RUNTIME_CONFIG, &psap_cap);

            if (res != AUDIO_PSAP_STATUS_SUCCESS) {
                return res;
            }
            res = llf_control_query_data_ready();
            if (res == AUDIO_PSAP_STATUS_SUCCESS) {
                U32* dsp_share_buf = hal_audio_query_llf_share_info(SHARE_BUFFER_LLF_INFO_ID_DSP);
                *len = *dsp_share_buf;
                dsp_share_buf += 1;
//                LOGMSGIDI("data ready(len %d): %x%x%x%x%x%x%x%x%x%x", 11, *len,
//                                    *get_data, *(get_data+1), *(get_data+2), *(get_data+3), *(get_data+4), *(get_data+5), *(get_data+6), *(get_data+7), *(get_data+8), *(get_data+9));
                memcpy(data, (U8*)dsp_share_buf, *len);
            }
            return res;
        }
    }
    return AUDIO_PSAP_STATUS_SUCCESS;
}
