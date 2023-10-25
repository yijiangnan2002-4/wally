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

#include "bt_type.h"
#include "bt_source_srv_hfp.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_call.h"
#ifdef MTK_BT_CM_SUPPORT
#include "bt_connection_manager.h"
#endif
#include "bt_device_manager_internal.h"
#include "bt_source_srv_call_pseduo_dev.h"

static const uint8_t g_source_call_volume_step_table[] = {8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};

bt_status_t bt_source_srv_call_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t result = BT_STATUS_SUCCESS;
    uint32_t module = msg & 0xFF000000;

    switch (module) {
        case BT_MODULE_HFP:
            result = bt_source_srv_hfp_common_callback(msg, status, buffer);
            break;
        default:
            break;
    }
    return result;
}

bt_status_t bt_source_srv_call_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length)
{
    return bt_source_srv_hfp_send_action(action, parameter, length);;
}

static bt_status_t bt_source_srv_call_read_nvdm(bt_bd_addr_t *bd_address, bt_source_srv_call_stored_data_t *stored_data)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef BT_SOURCE_SRV_AG_STORAGE_SIZE
    bt_device_manager_db_remote_profile_info_t record = {{0}};
    status = bt_device_manager_remote_find_profile_info((*bd_address), &record);
    if (status != BT_STATUS_SUCCESS) {
        return status;
    }
    bt_source_srv_memcpy(stored_data, record.ag_info, sizeof(bt_source_srv_call_stored_data_t));
#endif
    return status;
}

static bt_status_t bt_source_srv_call_write_nvdm(bt_bd_addr_t *bd_address, bt_source_srv_call_stored_data_t *stored_data)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef BT_SOURCE_SRV_AG_STORAGE_SIZE
    bt_device_manager_db_remote_profile_info_t record = {{0}};
    bt_device_manager_remote_find_profile_info((*bd_address), &record);
    bt_source_srv_memcpy(record.ag_info, stored_data, sizeof(bt_source_srv_call_stored_data_t));
    status = bt_device_manager_remote_update_profile_info((*bd_address), &record);
    bt_device_manager_db_flush_all(BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
#endif
    return status;
}

bt_status_t bt_source_srv_call_audio_gain_update(bt_source_srv_call_gain_t gain_type, bt_bd_addr_t *bd_address, void *device,
        bt_source_srv_call_audio_volume_t gain_level)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_call_stored_data_t stored_data = {0};
    if (bt_source_srv_call_read_nvdm(bd_address, &stored_data) != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[AG][CALL] device = %02x gain type = %02x update fail", 2, device, gain_type);
        return status;
    }

    switch (gain_type) {
        case BT_SOURCE_SRV_CALL_GAIN_LOCAL_SPEAKER: {
            stored_data.local_speaker_volume = gain_level;
            bt_source_srv_call_psd_set_speaker_volume(device, stored_data.local_speaker_volume);
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_SPEAKER: {
            stored_data.speaker_volume = gain_level;
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_MIC: {
            stored_data.mic_gain = gain_level;
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER: {
            stored_data.audio_source_speaker_volume = gain_level;
        }
        break;
        default:
            return status;
    }
    LOG_MSGID_I(source_srv, "[AG][CALL] device = %02x gain type = %02x update level = %02x", 3, device, gain_type, gain_level);
    return bt_source_srv_call_write_nvdm(bd_address, &stored_data);
}

bt_status_t bt_source_srv_call_audio_gain_init(bt_bd_addr_t *bd_address, void *device)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_assert(bd_address && "volume init bd_address is NULL");
    bt_source_srv_call_stored_data_t stored_data = {0};
    if (bt_source_srv_call_read_nvdm(bd_address, &stored_data) == BT_STATUS_SUCCESS) {
        LOG_MSGID_I(source_srv, "[AG][CALL] read nvdm local speker volume = %02x, speaker volume = %02x, mic gain = %02x audio source gain = %02x", 4,
                    stored_data.local_speaker_volume, stored_data.speaker_volume, stored_data.mic_gain, stored_data.audio_source_speaker_volume);
    } else {
        LOG_MSGID_W(source_srv, "[AG][CALL] read nvdm speaker volume and mic gain fail", 0);
        stored_data.local_speaker_volume = BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME;
        stored_data.speaker_volume = BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME;
        stored_data.mic_gain = BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME;
        stored_data.audio_source_speaker_volume = BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME;
        status = bt_source_srv_call_write_nvdm(bd_address, &stored_data);
    }
    bt_source_srv_call_psd_set_speaker_volume(device, stored_data.local_speaker_volume);
    return status;
}

uint32_t bt_source_srv_call_get_audio_gain_level(bt_source_srv_call_gain_t gain_type, bt_bd_addr_t *bd_address)
{
    uint32_t gain_level = 0;
    bt_source_srv_call_stored_data_t stored_data = {0};
    if (bt_source_srv_call_read_nvdm(bd_address, &stored_data) != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[AG][CALL] get gain type = %02x update fail", 1, gain_type);
        return gain_level;
    }

    switch (gain_type) {
        case BT_SOURCE_SRV_CALL_GAIN_LOCAL_SPEAKER: {
            gain_level = stored_data.local_speaker_volume;
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_SPEAKER: {
            gain_level = stored_data.speaker_volume;
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_MIC: {
            gain_level = stored_data.mic_gain;
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER: {
            gain_level = stored_data.audio_source_speaker_volume;
        }
        break;
        default:
            break;
    }
    LOG_MSGID_I(source_srv, "[AG][CALL] get gain type = %02x level = %02x", 2, gain_type, gain_level);
    return gain_level;
}

uint32_t bt_source_srv_call_convert_volume(bt_source_srv_call_volume_scale_t vcf_type, uint32_t volume_value, bool is_forward)
{
    if (is_forward) {
        return (uint32_t)(volume_value * ((double)BT_SOURCE_SRV_CALL_AUDIO_MAX_VOLUME / vcf_type)  + 0.5);
    }
    return (uint32_t)(volume_value * ((double)vcf_type / BT_SOURCE_SRV_CALL_AUDIO_MAX_VOLUME) + 0.5);
}

uint32_t bt_source_srv_call_convert_volume_step(uint32_t min_volume, uint32_t max_volume)
{
    bt_source_srv_common_feature_config_t common_feature_config = {0};
    uint32_t volume_step = 0;

    if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_NONE, &common_feature_config) == BT_STATUS_SUCCESS) {
        if (common_feature_config.host_volume_scale == BT_SOURCE_SRV_CALL_VOLUME_SCALE_15) {
            return (max_volume - min_volume);
        }
    }

    for (uint32_t i = min_volume; i < max_volume; i++) {
        volume_step += g_source_call_volume_step_table[i];
    }
    return (volume_step >> 1);
}

bt_status_t bt_source_srv_call_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_call_port_action_t action)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    status = bt_source_srv_hfp_audio_port_update(audio_port, action);
#endif
    return status;
}

uint8_t bt_source_srv_call_get_playing_device_codec(void)
{
    return bt_source_srv_hfp_get_playing_device_codec();
}

bt_status_t bt_source_srv_call_init(bt_source_srv_call_init_parameter_t *init_param)
{
    bt_source_srv_hfp_init(init_param->hfp_init_param);
    return BT_STATUS_SUCCESS;
}
