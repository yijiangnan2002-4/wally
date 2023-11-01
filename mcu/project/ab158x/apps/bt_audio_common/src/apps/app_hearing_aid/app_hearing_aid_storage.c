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


#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#include "app_hearing_aid_storage.h"
#include "app_hear_through_storage.h"
#include "apps_debug.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "assert.h"
#include "stdlib.h"
#include "string.h"

#define APP_HEARING_AID_STG_TAG "[HearingAid][Storage]"

typedef struct {
    uint8_t     level_sync_switch : 1;
    uint8_t     volume_sync_switch : 1;
    uint8_t     in_ear_detection_switch : 1;
    uint8_t     reserved_1 : 5;
    uint8_t     a2dp_mix_switch : 1;
    uint8_t     sco_mix_switch : 1;
    uint8_t     vp_mix_switch : 1;
    uint8_t     reserved_2 : 5;
    uint8_t     reserved_bytes[4];
} __attribute__((packed)) app_hearing_aid_user_configuration_t;

#define APP_HEARING_AID_USER_CONFIG_LEN     (sizeof(app_hearing_aid_user_configuration_t))

static app_hearing_aid_user_configuration_t user_config_from_nvkey;
static app_hearing_aid_user_configuration_t user_config;
static app_hearing_aid_user_configuration_t default_config;

bool app_hearing_aid_storage_load()
{
    memset(&default_config, 0, APP_HEARING_AID_USER_CONFIG_LEN);
    memset(&user_config_from_nvkey, 0, APP_HEARING_AID_USER_CONFIG_LEN);
    memset(&user_config, 0, APP_HEARING_AID_USER_CONFIG_LEN);

    bool read_default_value = app_hear_through_storage_read_data(NVID_APP_HEARING_AID_CONFIGURATION,
                                                                    APP_HEARING_AID_USER_CONFIG_LEN,
                                                                    (uint8_t *)&default_config);

    bool read_user_value = app_hear_through_storage_read_data(NVID_APP_HEARING_AID_USER_CONFIGURATION,
                                                                    APP_HEARING_AID_USER_CONFIG_LEN,
                                                                    (uint8_t *)&user_config);

    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_load] read_default_value : %d, read_user_value : %d",
                        2,
                        read_default_value,
                        read_user_value);

    if (read_user_value == false) {
        if (read_default_value == false) {
            assert( 0 && "Read hearing aid configuration failed" );
        } else {
            memcpy(&user_config, &default_config, APP_HEARING_AID_USER_CONFIG_LEN);
        }
    }

    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_load][SYNC_SWITCH] level : %d, volume : %d, in_ear : %d, a2dp_mix : %d, sco_mix : %d, vp_mix : %d",
                        6,
                        user_config.level_sync_switch,
                        user_config.volume_sync_switch,
                        user_config.in_ear_detection_switch,
                        user_config.a2dp_mix_switch,
                        user_config.sco_mix_switch,
                        user_config.vp_mix_switch);

    memcpy(&user_config_from_nvkey, &user_config, APP_HEARING_AID_USER_CONFIG_LEN);

    return true;
}

bool app_hearing_aid_storage_get_level_sync_switch()
{
    return user_config.level_sync_switch;
}

bool app_hearing_aid_storage_get_volume_sync_switch()
{
    return user_config.volume_sync_switch;
}

bool app_hearing_aid_storage_get_in_ear_detection_switch()
{
    return user_config.in_ear_detection_switch;
}

bool app_hearing_aid_storage_get_a2dp_mix_switch()
{
    return user_config.a2dp_mix_switch;
}

bool app_hearing_aid_storage_get_sco_mix_switch()
{
    return user_config.sco_mix_switch;
}

bool app_hearing_aid_storage_get_vp_mix_switch()
{
    return user_config.vp_mix_switch;
}

void app_hearing_aid_storage_set_level_sync_switch(bool level_sync)
{
    if (user_config.level_sync_switch != level_sync) {
        user_config.level_sync_switch = level_sync;
    }
}

void app_hearing_aid_storage_set_volume_sync_switch(bool volume_sync)
{
    if (user_config.volume_sync_switch != volume_sync) {
        user_config.volume_sync_switch = volume_sync;
    }
}

void app_hearing_aid_storage_set_in_ear_detection_switch(bool in_ear_detection_switch)
{
    if (user_config.in_ear_detection_switch != in_ear_detection_switch) {
        user_config.in_ear_detection_switch = in_ear_detection_switch;
    }
}

bool app_hearing_aid_storage_set_a2dp_mix_switch(bool a2dp_mix)
{
    if (user_config.a2dp_mix_switch != a2dp_mix) {
        user_config.a2dp_mix_switch = a2dp_mix;
        return true;
    }
    return false;
}

bool app_hearing_aid_storage_set_sco_mix_switch(bool sco_mix)
{
    if (user_config.sco_mix_switch != sco_mix) {
        user_config.sco_mix_switch = sco_mix;
        return true;
    }
    return false;
}

bool app_hearing_aid_storage_set_vp_mix_switch(bool vp_mix)
{
    if (user_config.vp_mix_switch != vp_mix) {
        user_config.vp_mix_switch = vp_mix;
        return true;
    }
    return false;
}

bool app_hearing_aid_storage_save_configuration()
{
    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_save_configuration] level : %d, volume : %d, in_ear : %d, a2dp_mix : %d, sco_mix : %d, vp_mix : %d",
                        6,
                        user_config.level_sync_switch,
                        user_config.volume_sync_switch,
                        user_config.in_ear_detection_switch,
                        user_config.a2dp_mix_switch,
                        user_config.sco_mix_switch,
                        user_config.vp_mix_switch);

    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_save_configuration] - nvkey level : %d, volume : %d, in_ear : %d, a2dp_mix : %d, sco_mix : %d, vp_mix : %d",
                        6,
                        user_config_from_nvkey.level_sync_switch,
                        user_config_from_nvkey.volume_sync_switch,
                        user_config_from_nvkey.in_ear_detection_switch,
                        user_config_from_nvkey.a2dp_mix_switch,
                        user_config_from_nvkey.sco_mix_switch,
                        user_config_from_nvkey.vp_mix_switch);

    if (memcmp(&user_config, &user_config_from_nvkey, APP_HEARING_AID_USER_CONFIG_LEN) == 0) {
        return true;
    }

    nvkey_status_t write_result = nvkey_write_data(NVID_APP_HEARING_AID_USER_CONFIGURATION,
                                                    (uint8_t *)&user_config,
                                                    APP_HEARING_AID_USER_CONFIG_LEN);

    if (write_result != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_save_configuration] Write user configuration failed, %d",
                            1,
                            write_result);
        return false;
    }

    /**
     * @brief Fix issue - 47470
     * If the configuration already save to nvkey succeed, need update the parameter to make sure
     * the save flow working correctly if the configuration change again.
     */
    memcpy(&user_config_from_nvkey, &user_config, APP_HEARING_AID_USER_CONFIG_LEN);

    return true;
}

bool app_hearing_aid_storage_restore(bool *need_update)
{
    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_restore][Restore] level : %d, volume : %d, in_ear : %d, a2dp_mix : %d, sco_mix : %d, vp_mix : %d",
                        6,
                        default_config.level_sync_switch,
                        default_config.volume_sync_switch,
                        default_config.in_ear_detection_switch,
                        default_config.a2dp_mix_switch,
                        default_config.sco_mix_switch,
                        default_config.vp_mix_switch);

    *need_update = false;
    if (memcmp(&user_config, &default_config, APP_HEARING_AID_USER_CONFIG_LEN) != 0) {
        *need_update = true;
    }

    memcpy(&user_config, &default_config, APP_HEARING_AID_USER_CONFIG_LEN);
    memcpy(&user_config_from_nvkey, &default_config, APP_HEARING_AID_USER_CONFIG_LEN);

    nvkey_status_t write_result = nvkey_write_data(NVID_APP_HEARING_AID_USER_CONFIGURATION,
                                                    (uint8_t *)&user_config,
                                                    APP_HEARING_AID_USER_CONFIG_LEN);

    if (write_result != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_restore][Restore] restore configuration failed, %d",
                            1,
                            write_result);
        return false;
    }

    return true;
}

void app_hearing_aid_storage_sync_user_configuration(uint8_t *buf, uint8_t buf_len)
{
    if ((buf == NULL) || (buf_len == 0)) {
        APPS_LOG_MSGID_E(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_sync_user_configuration] parameter error, 0x%x, %d",
                            2,
                            buf,
                            buf_len);
        return;
    }

    if (buf_len != APP_HEARING_AID_USER_CONFIG_LEN) {
        APPS_LOG_MSGID_E(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_sync_user_configuration] length error, %d, %d",
                            2,
                            buf_len,
                            APP_HEARING_AID_USER_CONFIG_LEN);
        return;
    }

    memcpy(&user_config, buf, APP_HEARING_AID_USER_CONFIG_LEN);
    // memcpy(&user_config_from_nvkey, buf, APP_HEARING_AID_USER_CONFIG_LEN);

    APPS_LOG_MSGID_I(APP_HEARING_AID_STG_TAG"[app_hearing_aid_storage_sync_user_configuration][SYNC] level : %d, volume : %d, in_ear : %d, a2dp_mix : %d, sco_mix : %d, vp_mix : %d",
                        6,
                        user_config.level_sync_switch,
                        user_config.volume_sync_switch,
                        user_config.in_ear_detection_switch,
                        user_config.a2dp_mix_switch,
                        user_config.sco_mix_switch,
                        user_config.vp_mix_switch);
}

uint8_t *app_hearing_aid_storage_get_user_configuration(uint8_t *config_len)
{
    *config_len = APP_HEARING_AID_USER_CONFIG_LEN;
    return (uint8_t *)&user_config;
}

#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
