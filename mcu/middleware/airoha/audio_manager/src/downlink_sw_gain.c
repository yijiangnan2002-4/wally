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

#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE

#include "downlink_sw_gain.h"

#include "nvkey.h"
#include "audio_src_srv.h"
#include "bt_sink_srv_audio_setting.h"
#include "bt_sink_srv_ami.h"
#include "hal_audio_cm4_dsp_message.h"
#include "nvkey_id_list.h"
#include <stdlib.h>

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "bt_sink_srv_le_cap_audio_manager.h"
#endif

int16_t *g_DL_SW_gain_GL = NULL;
int16_t *g_DL_SW_gain_GR = NULL;

uint8_t g_DL_SW_gain_remember_main_level = 0;
uint8_t g_DL_SW_gain_remember_main_codec_type = 1;
uint8_t g_DL_SW_gain_remember_main_level_3 = 0;

LR_balance_para_t g_DL_SW_gain_lr_balance_para;
dl_sw_gain_default_para_t *g_DL_SW_gain_default_para;

void set_volume_to_DL_SW_gain(LR_balance_para_t *para)
{
    // get current scenario
    uint32_t gain_offset = 0;
    int16_t gain_offset_l = 0;
    int16_t gain_offset_r = 0;

    int32_t gain_main = 0;
    int32_t gain_main_a_gain = 0;
    int32_t gain_main_max = 0;

    // int32_t gain_main_2 = 0;
    // int32_t gain_main_a_gain_2 = 0;
    // int32_t gain_main_max_2 = 0;

    int32_t gain_main_3 = 0;
    int32_t gain_main_a_gain_3 = 0;
    int32_t gain_main_max_3 = 0;

    bool send_para_to_dsp = FALSE;
    bool send_para_to_dsp_3 = FALSE;

    // hal_audio_hw_stream_out_index_t hw_gain_index;
    // hal_audio_hw_stream_out_index_t hw_gain_index_3;

    const audio_src_srv_handle_t *hd = audio_src_srv_get_runing_pseudo_device();
    // if(hd == NULL){
    //     return;
    // }

     if(
#ifdef AIR_BT_CODEC_BLE_ENABLED
            (((g_prCurrent_player->type == A2DP)||(g_prCurrent_player->type == AWS)) && (g_DL_SW_gain_default_para->enable_a2dp))
            || ((g_prCurrent_player->type == ULL_BLE_DL) && (g_DL_SW_gain_default_para->enable_ull_music))
            || ((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_ONLY) && (g_DL_SW_gain_default_para->enable_ble_music))
#else
            ((g_prCurrent_player->type == A2DP)||(g_prCurrent_player->type == AWS)) && (g_DL_SW_gain_default_para->enable_a2dp)
#endif
        ){

    } else if(
#ifdef AIR_BT_CODEC_BLE_ENABLED
            ((g_prCurrent_player->type == HFP) && (g_DL_SW_gain_default_para->enable_hfp))
            ||((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY) && (g_DL_SW_gain_default_para->enable_ble_call))
#else
            (g_prCurrent_player->type == HFP) && (g_DL_SW_gain_default_para->enable_hfp)
#endif
    ){

    } else {
        audio_transmitter_scenario_list_t list_line_in[] = {{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT}};
        audio_transmitter_scenario_list_t list_usb_in[] = {{AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
                                                                {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1}};
        if(((audio_transmitter_get_is_running_by_scenario_list(list_line_in, 1) && g_DL_SW_gain_default_para->enable_line_in))
            ||((audio_transmitter_get_is_running_by_scenario_list(list_usb_in, 1) && g_DL_SW_gain_default_para->enable_usb_in))
        ){
        } else {
            return;
        }
    }

    if(hd != NULL){
        if((hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP) || (hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP)
    #ifdef AIR_BT_CODEC_BLE_ENABLED
            || (g_prCurrent_player->type == ULL_BLE_DL)
            || ((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode == CHANNEL_MODE_DL_ONLY))
    #endif
            ){
            gain_main = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level, GAIN_DIGITAL, VOL_A2DP);
            gain_main_a_gain = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level, GAIN_ANALOG, VOL_A2DP);
            gain_main_max = audio_get_gain_out_in_dB(audio_get_max_sound_level_out(VOL_A2DP), GAIN_DIGITAL, VOL_A2DP);
            // hw_gain_index = HAL_AUDIO_STREAM_OUT1;

            send_para_to_dsp = TRUE;
        }else if((hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP) || (hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_HFP)
    #ifdef AIR_BT_CODEC_BLE_ENABLED
                    || ((g_prCurrent_player->type == BLE)&&(g_prCurrent_player->local_context.ble_format.ble_codec.channel_mode != CHANNEL_MODE_DL_ONLY))
    #endif
                    ){
            vol_type_t type;
            if(g_DL_SW_gain_remember_main_codec_type == BT_HFP_CODEC_TYPE_CVSD){
                type = VOL_HFP_NB;
            } else {
                type = VOL_HFP;
            }
            gain_main = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level, GAIN_DIGITAL, type);
            gain_main_a_gain = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level, GAIN_ANALOG, type);
            gain_main_max = audio_get_gain_out_in_dB(audio_get_max_sound_level_out(type), GAIN_DIGITAL, type);
            // hw_gain_index = HAL_AUDIO_STREAM_OUT1;

            send_para_to_dsp = TRUE;
        } else {
            //not support
        }
    }


#if defined(AIR_WIRED_AUDIO_ENABLE)
    audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
        {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN}
    };
    if (true == audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list, sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t))) {
        for (uint32_t i = 0; i < sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t); i++) {
            if (true == audio_transmitter_get_is_running_by_scenario_list(&audio_transmitter_scenario_list[i], 1)) {
                gain_main_3 = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level_3, GAIN_DIGITAL, VOL_LINE_IN);
                gain_main_a_gain_3 = audio_get_gain_out_in_dB(g_DL_SW_gain_remember_main_level_3, GAIN_ANALOG, VOL_LINE_IN);
                gain_main_max_3 = audio_get_gain_out_in_dB(audio_get_max_sound_level_out(VOL_LINE_IN), GAIN_DIGITAL, VOL_LINE_IN);
                send_para_to_dsp_3 = TRUE;
                // audio_src_srv_err("[DL_SW_GAIN] test gain_main_3, %d %d %d %d %d",5,g_DL_SW_gain_remember_main_level_3,audio_get_max_sound_level_out(VOL_LINE_IN),gain_main_3,gain_main_a_gain_3,gain_main_max_3);
            }
        }
    }
#endif

    //set main gain to dsp
    if(send_para_to_dsp == TRUE){
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE_VOLUME, HAL_AUDIO_STREAM_OUT1,  (gain_main_a_gain<<16) | (0 & 0xFFFF), false); //don`t set hw d-gain
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_1, gain_main, false);
    }

    //HW gain 2 set at prompt_control_set_volume(void)
    //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_2, gain_main_2, false);

    if(send_para_to_dsp_3 == TRUE){
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE_VOLUME, HAL_AUDIO_STREAM_OUT3,  (gain_main_a_gain_3<<16) | (0 & 0xFFFF), false); //don`t set hw d-gain
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_3, gain_main_3, false);
    }

    //set gain offset to dsp
    if((para->LR_balance_status == 1) && 
        (para->LR_balance_ID >= 100 - g_DL_SW_gain_default_para->level_num) && (para->LR_balance_ID <= 100 + g_DL_SW_gain_default_para->level_num)){

        gain_offset_l = g_DL_SW_gain_GL[g_DL_SW_gain_default_para->level_num + (para->LR_balance_ID - 100)];
        gain_offset_r = g_DL_SW_gain_GR[g_DL_SW_gain_default_para->level_num + (para->LR_balance_ID - 100)];
        // audio_src_srv_err("[DL_SW_GAIN]  test %d,%d  %d,%d,%d,%d,%d,%d,",8, 
        // gain_offset_l, gain_offset_r,
        // g_DL_SW_gain_GL[0],g_DL_SW_gain_GL[1],g_DL_SW_gain_GL[2],g_DL_SW_gain_GR[0],g_DL_SW_gain_GR[1],g_DL_SW_gain_GR[2]);
  
        // check max gain limiter
        if(g_DL_SW_gain_default_para->max_gain_limiter){
            if(send_para_to_dsp == TRUE){
                if(gain_main + gain_offset_l > gain_main_max){
                    gain_offset_l = gain_main_max - gain_main;
                } else if(gain_main + gain_offset_r > gain_main_max){
                    gain_offset_r = gain_main_max - gain_main;
                }
            } else if(send_para_to_dsp_3 == TRUE){
                if(gain_main_3 + gain_offset_l > gain_main_max_3){
                    gain_offset_l = gain_main_max_3 - gain_main_3;
                } else if(gain_main_3 + gain_offset_r > gain_main_max_3){
                    gain_offset_r = gain_main_max_3 - gain_main_3;
                }
            }
        }

        // set volume offset.
        if (ami_get_audio_channel() == AUDIO_CHANNEL_L){
            gain_offset = ((uint32_t)gain_offset_l)<<16 | (((uint32_t)gain_offset_l)<<16)>>16;
        } else if (ami_get_audio_channel() == AUDIO_CHANNEL_R){
            gain_offset = ((uint32_t)gain_offset_r)<<16 | (((uint32_t)gain_offset_r)<<16)>>16;
        } else {
            gain_offset = ((uint32_t)gain_offset_l)<<16 | (((uint32_t)gain_offset_r)<<16)>>16;
        }
    } else {
        if((para->LR_balance_ID < 100 - g_DL_SW_gain_default_para->level_num) || (para->LR_balance_ID > 100 + g_DL_SW_gain_default_para->level_num)){
            audio_src_srv_err("[DL_SW_GAIN] LR_balance_ID = %d error!!", 1, para->LR_balance_ID);
            return;
        }
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, 0, gain_offset, false);

        if(send_para_to_dsp == TRUE){
        audio_src_srv_report("[DL_SW_GAIN] (L?%d,R?%d,LR?%d) STREAM_OUT1 LR_balance_status = %d gain_main = %d, main_max = %d, offset_l = %d, offset_r = %d, ccni_offset = 0x%x", 9,
                    ((ami_get_audio_channel() == AUDIO_CHANNEL_L)),((ami_get_audio_channel() == AUDIO_CHANNEL_R)),((ami_get_audio_channel() == AUDIO_CHANNEL_NONE)),
                    para->LR_balance_status, gain_main, gain_main_max, gain_offset_l, gain_offset_r, gain_offset);
    }

    //HW gain 2 set at prompt_control_set_volume(void)
    //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, 1, gain_main_2, false);

    if(send_para_to_dsp_3 == TRUE){
        audio_src_srv_report("[DL_SW_GAIN] (L?%d,R?%d,LR?%d) STREAM_OUT3 LR_balance_status = %d gain_main = %d, main_max = %d, offset_l = %d, offset_r = %d, ccni_offset = 0x%x", 9,
                    ((ami_get_audio_channel() == AUDIO_CHANNEL_L)),((ami_get_audio_channel() == AUDIO_CHANNEL_R)),((ami_get_audio_channel() == AUDIO_CHANNEL_NONE)),
                    para->LR_balance_status, gain_main_3, gain_main_max_3, gain_offset_l, gain_offset_r, gain_offset);
    }
}

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_device_manager.h"
void bt_aws_mce_report_send_DL_SW_gain_info(uint8_t *ptr, uint8_t len)
{
    bt_aws_mce_report_info_t info;
    info.module_id = BT_AWS_MCE_REPORT_MODULE_AUDIO_GAIN;
    info.is_sync = false;
    info.sync_time = 0;
    info.param_len = len;
    info.param = ptr;

    audio_src_srv_report("[DL_SW_GAIN]LR_VOLUME_BALANCE agent send to partner", 0);
    bt_aws_mce_report_send_event(&info);
}

void bt_aws_mce_report_DL_SW_gain_callback(bt_aws_mce_report_info_t *info)
{
    uint32_t nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_LR_VOLUME_BALANCE;
    //write to nvkey
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    status = nvkey_write_data(nvkey_id, (uint8_t*)(info->param), info->param_len);

    if(status == NVKEY_STATUS_OK){
        set_volume_to_DL_SW_gain((LR_balance_para_t *)(info->param));
    } else {
        audio_src_srv_err("[DL_SW_GAIN] partner write key fail, nvkey_id[0x%x], st = %d", 2, nvkey_id, status);
        configASSERT(0);
    }
}

bt_status_t bc_cm_event_DL_SW_gain_callback(bt_cm_event_t event_id, void *params, uint32_t params_len)
{
    bt_cm_remote_info_update_ind_t *p = params;
    if((event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE)
    && (!(p->pre_connected_service & (1<< BT_CM_PROFILE_SERVICE_AWS))) && (p->connected_service & (1<< BT_CM_PROFILE_SERVICE_AWS))
    && (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT)){
        uint32_t size = sizeof(LR_balance_para_t);
        uint32_t nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_LR_VOLUME_BALANCE;
        LR_balance_para_t info;
        nvkey_status_t status = NVKEY_STATUS_ERROR;
        status = nvkey_read_data(nvkey_id, (uint8_t*)(&info), &size);
        if (status != NVKEY_STATUS_OK)
        {
            audio_src_srv_err("[DL_SW_GAIN] partner nvkey_read_data fail, nvkey_id[0x%x], st = %d", 2, nvkey_id, status);
            configASSERT(0);
        } else {
            bt_aws_mce_report_send_DL_SW_gain_info((uint8_t*)(&info),sizeof(LR_balance_para_t));
        }
    }
    return BT_STATUS_SUCCESS;
}
#endif


bool DL_SW_gain_set_lr_volume_balance_param(LR_balance_para_t *para)
{
    //check para
    audio_src_srv_report("DL_SW_gain_set_lr_volume_balance_param, status=%d, id=%d", 2, para->LR_balance_status, para->LR_balance_ID);
    if((para->LR_balance_status == 1) && 
        ((para->LR_balance_ID < (100 - g_DL_SW_gain_default_para->level_num)) || (para->LR_balance_ID > (100 + g_DL_SW_gain_default_para->level_num)))){
        audio_src_srv_err("DL_SW_gain_set_lr_volume_balance_param, error, status=%d, id=%d", 2, para->LR_balance_status, para->LR_balance_ID);
        return FALSE;
    }
    //write to nvkey
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_LR_VOLUME_BALANCE;
    status = nvkey_write_data(nvkey_id, (uint8_t*)(para), sizeof(LR_balance_para_t));
    
    #ifdef MTK_AWS_MCE_ENABLE
    //send to partner
    if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT){
       bt_aws_mce_report_send_DL_SW_gain_info((uint8_t *)para, sizeof(LR_balance_para_t));
    }
    #endif

    if(status == NVKEY_STATUS_OK){
        // set param
        memcpy(&g_DL_SW_gain_lr_balance_para, para, sizeof(LR_balance_para_t));
        set_volume_to_DL_SW_gain(para);
        return  TRUE;
    } else {
        audio_src_srv_err("DL_SW_gain_set_lr_volume_balance_param, write nvkey fail nvkey_id[0x%x], st=%d", 2, nvkey_id, status);
        return FALSE;

    }
}

bool DL_SW_gain_get_lr_volume_balance_param(LR_balance_para_t *para)
{
    uint32_t size = sizeof(LR_balance_para_t);
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_LR_VOLUME_BALANCE;
    status = nvkey_read_data(nvkey_id, (uint8_t*)(para), &size);
    if (status != NVKEY_STATUS_OK) {
        audio_src_srv_err("DL_SW_gain_get_lr_volume_balance_param, read nvkey fail nvkey_id[0x%x], st=%d", 2, nvkey_id, status);
        return FALSE;
    }
    audio_src_srv_report("DL_SW_gain_set_lr_volume_balance_param, status=%d, id=%d", 2, para->LR_balance_status, para->LR_balance_ID);
    return TRUE;
}

void DL_SW_gain_set_fade_in_param(dl_sw_gain_fade_para_t para)
{
    uint32_t fade_param = ((uint32_t)para.new_step)<<16 | (((uint32_t)para.new_samples_per_step)<<16)>>16;
    switch (para.index)
    {
    case HAL_AUDIO_STREAM_OUT1:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_1_FADE_IN_PARAM, fade_param, false);
        break;
    case HAL_AUDIO_STREAM_OUT2:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_2_FADE_IN_PARAM, fade_param, false);
        break;
    case HAL_AUDIO_STREAM_OUT3:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_3_FADE_IN_PARAM, fade_param, false);
        break;
    default:
        AUDIO_ASSERT(0 && "[AM][ERROR]DL_SW_gain_set_fade_out_param gain_index not support.");
    }
}
void DL_SW_gain_set_fade_out_param(dl_sw_gain_fade_para_t para)
{
    uint32_t fade_param = ((uint32_t)para.new_step)<<16 | (((uint32_t)para.new_samples_per_step)<<16)>>16;
    switch (para.index)
    {
    case HAL_AUDIO_STREAM_OUT1:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_1_FADE_OUT_PARAM, fade_param, false);
        break;
    case HAL_AUDIO_STREAM_OUT2:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_2_FADE_OUT_PARAM, fade_param, false);
        break;
    case HAL_AUDIO_STREAM_OUT3:
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN, DL_SW_GAIN_SET_MAINGAIN_3_FADE_OUT_PARAM, fade_param, false);
        break;
    default:
        AUDIO_ASSERT(0 && "[AM][ERROR]DL_SW_gain_set_fade_out_param gain_index not support.");
    }
}

void DL_SW_gain_init(void)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_DEFAULT;
    uint32_t size = sizeof(dl_sw_gain_default_para_t);
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(nvkey_id, &tableSize);
    if (status || !tableSize) {
        AUDIO_ASSERT(0 && "[AM][ERROR][DL_SW_GAIN] default key not find.");
    }
    g_DL_SW_gain_default_para = (dl_sw_gain_default_para_t *)pvPortMalloc(tableSize);
    status = nvkey_read_data(nvkey_id, (uint8_t *)g_DL_SW_gain_default_para, &tableSize);
    if (status != NVKEY_STATUS_OK){
        AUDIO_ASSERT(0 && "[AM][ERROR][DL_SW_GAIN] default key not find.");
    }
    
    if(tableSize != (g_DL_SW_gain_default_para->ch_num * (g_DL_SW_gain_default_para->level_num * 2 + 1)*sizeof(int16_t) + sizeof(dl_sw_gain_default_para_t) - sizeof(int16_t))){
        AUDIO_ASSERT(0 && "[AM][ERROR][DL_SW_GAIN] default key size not right.");
    }

    g_DL_SW_gain_GL = (g_DL_SW_gain_default_para->offset_value);
    g_DL_SW_gain_GR = g_DL_SW_gain_default_para->offset_value + (g_DL_SW_gain_default_para->level_num * 2 + 1);

    nvkey_id = NVKEYID_DSP_PARA_DL_SW_GAIN_LR_VOLUME_BALANCE;
    size = sizeof(LR_balance_para_t);
    status = nvkey_read_data(nvkey_id, (uint8_t *)&g_DL_SW_gain_lr_balance_para, &size);
    if (status != NVKEY_STATUS_OK){
        audio_src_srv_report("[DL_SW_GAIN] key not find, set with default value", 0);
        status = nvkey_write_data(nvkey_id, (uint8_t*)&(g_DL_SW_gain_default_para->LR_balance_para_default), sizeof(LR_balance_para_t));
        if (status != NVKEY_STATUS_OK){
            AUDIO_ASSERT(0 && "[AM][ERROR][DL_SW_GAIN] can`t write key to DL_SW_GAIN_LR_VOLUME_BALANCE.");
        }
    }

    audio_src_srv_report("[DL_SW_GAIN] init , enable %d,%d,%d,%d,%d,%d,%d,%d,%d,  %d,%d, ",
                                                            11, g_DL_SW_gain_default_para->enable_vp, g_DL_SW_gain_default_para->enable_a2dp, 
                                                            g_DL_SW_gain_default_para->enable_hfp, g_DL_SW_gain_default_para->enable_ble_music,
                                                            g_DL_SW_gain_default_para->enable_ble_call, g_DL_SW_gain_default_para->enable_ull_music,
                                                            g_DL_SW_gain_default_para->enable_ull_call, g_DL_SW_gain_default_para->enable_line_in,
                                                            g_DL_SW_gain_default_para->enable_usb_in, 
                                                            g_DL_SW_gain_default_para->ch_num, g_DL_SW_gain_default_para->level_num);
    void *p_param_share = hal_audio_dsp_controller_put_paramter(g_DL_SW_gain_default_para, sizeof(dl_sw_gain_default_para_t), AUDIO_MESSAGE_TYPE_COMMON);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DL_SW_GAIN_DEFAULT_PARA, 0, (uint32_t)p_param_share, true);

    set_volume_to_DL_SW_gain(&g_DL_SW_gain_lr_balance_para);
}
#endif
