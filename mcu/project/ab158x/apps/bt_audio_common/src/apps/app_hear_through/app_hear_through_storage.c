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


#include "app_hear_through_storage.h"
#include "apps_debug.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "string.h"
#include "FreeRTOS.h"
#include "assert.h"
#include "anc_control_api.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#define APP_HEAR_THROUGH_STG_TAG "[HearThrough][Storage]"

#define APP_HEAR_THROUGH_ANC_MODE_HYBRID            0x00
#define APP_HEAR_THROUGH_ANC_MODE_FF                0x01
#define APP_HEAR_THROUGH_ANC_MODE_FB                0x02
#define APP_HEAR_THROUGH_ANC_MODE_OFF               0x04

#define APP_HEARING_AID_IN_EAR_TURN_ON_HA_DELAY_TIME_UNIT           (100)   // 100ms
#define APP_HEARING_AID_BLE_ADV_STOP_TIME_UINT                      (10 * 1000) // 10s
#define APP_HEARING_AID_RSSI_POWER_OFF_TIME_UNIT                    (60 * 1000) // 1min
#define APP_HEARING_AID_POWER_ON_TURN_ON_HA_DELAY_TIME_UNIT         (100) // 100ms

typedef struct {
    uint8_t                     reserved_1;
    uint8_t                     reserved_2 : 1;
    uint8_t                     reserved_3 : 7;
    uint8_t                     turn_on_hear_through_timeout_after_boot_up;
    uint8_t                     in_ear_det_turn_on_ha_delay_time; // 100ms
    uint8_t                     ha_ble_adv_timeout; // 10s
    int8_t                      ha_rssi_threshold;
    uint8_t                     ha_rssi_power_off_timeout;  // 1min
    uint8_t                     dedicate_mac_address[6];
    uint8_t                     anc_mode_with_vivid_pt; // ANC mode, 0x00 : Hybrid, 0x01 : FF, 0x02 : FB, 0x04 : off
    uint8_t                     anc_mode_with_ha_psap; // ANC mode, 0x00 : Hybrid, 0x01 : FF, 0x02 : FB, 0x04 : off
    uint8_t                     reserved_4[5];
    uint8_t                     ha_on_mode_vp_switch : 1;
    uint8_t                     ha_level_up_circular_max_vp_switch : 1;
    uint8_t                     ha_mode_up_circular_max_vp_switch : 1;
    uint8_t                     ha_vol_up_circular_max_vp_switch : 1;
    uint8_t                     reserved_5 : 4;
    int8_t                      ha_mic_ad_gain_l;
    int8_t                      ha_mic_ad_gain_r;
    uint8_t                     reserved_6[9];
} __attribute__((packed)) app_hear_through_const_configuration_t;

app_hear_through_const_configuration_t app_hear_through_const_config;

#define APP_HEAR_THROUGH_CONST_CONFIGURATION_NVKEY_LEN         (sizeof(app_hear_through_const_configuration_t))

bool app_hear_through_storage_read_data(uint16_t nvkey_id, uint8_t expected_len, uint8_t *data_to_fill_in)
{
    uint32_t read_data_len = 0;
    uint32_t actual_read_len = 0;
    nvkey_status_t read_status = NVKEY_STATUS_OK;

    if ((expected_len == 0) || (data_to_fill_in == NULL)) {
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_read_data] Parameter error, %d, 0x%04x",
                            2,
                            expected_len,
                            data_to_fill_in);
        return false;
    }

    read_status = nvkey_data_item_length(nvkey_id, &read_data_len);

    if ((read_status != NVKEY_STATUS_OK) || (read_data_len != expected_len)) {
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_read_data] Failed to read nvkey, ID : 0x%04x, status : %d, length : %d - %d",
                            4,
                            nvkey_id,
                            read_status,
                            read_data_len,
                            expected_len);
        return false;
    }

    actual_read_len = read_data_len;
    read_status = nvkey_read_data(nvkey_id, data_to_fill_in, &actual_read_len);

    if ((read_status != NVKEY_STATUS_OK) || (read_data_len != actual_read_len)) {

        memset(data_to_fill_in, 0, expected_len);

        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_read_data] read failed, status : %d, len : %d - %d",
                            3,
                            read_status,
                            read_data_len,
                            actual_read_len);

        return false;
    }

    return true;
}

void app_hear_through_storage_load_const_configuration()
{
    memset(&app_hear_through_const_config, 0, APP_HEAR_THROUGH_CONST_CONFIGURATION_NVKEY_LEN);

    app_hear_through_storage_read_data(NVID_DSP_ALG_HA_SYS,
                                        APP_HEAR_THROUGH_CONST_CONFIGURATION_NVKEY_LEN,
                                        (uint8_t *)&app_hear_through_const_config);
}

uint32_t app_hear_through_storage_get_ha_ble_advertising_timeout()
{
    return app_hear_through_const_config.ha_ble_adv_timeout * APP_HEARING_AID_BLE_ADV_STOP_TIME_UINT;
}

uint32_t app_hear_through_storage_get_hear_through_turn_on_after_boot_up_timeout()
{
    return app_hear_through_const_config.turn_on_hear_through_timeout_after_boot_up * APP_HEARING_AID_POWER_ON_TURN_ON_HA_DELAY_TIME_UNIT;
}

uint32_t app_hear_through_storage_get_ha_in_ear_det_turn_on_delay_time()
{
    return app_hear_through_const_config.in_ear_det_turn_on_ha_delay_time * APP_HEARING_AID_IN_EAR_TURN_ON_HA_DELAY_TIME_UNIT;
}

int8_t app_hear_through_storage_get_ha_rssi_threshold()
{
    return app_hear_through_const_config.ha_rssi_threshold;
}

uint32_t app_hear_through_storage_get_ha_rssi_power_off_timeout()
{
    return app_hear_through_const_config.ha_rssi_power_off_timeout * APP_HEARING_AID_RSSI_POWER_OFF_TIME_UNIT;
}

bool app_hear_through_storage_get_ha_mode_on_vp_switch()
{
    return app_hear_through_const_config.ha_on_mode_vp_switch;
}

bool app_hear_through_storage_get_ha_level_up_circular_max_vp_switch()
{
    return app_hear_through_const_config.ha_level_up_circular_max_vp_switch;
}

bool app_hear_through_storage_get_ha_mode_up_circular_max_vp_switch()
{
    return app_hear_through_const_config.ha_mode_up_circular_max_vp_switch;
}

bool app_hear_through_storage_get_ha_vol_up_circular_max_vp_switch()
{
    return app_hear_through_const_config.ha_vol_up_circular_max_vp_switch;
}

uint8_t app_hear_through_storage_get_anc_mode_with_vivid_pt()
{
    return app_hear_through_const_config.anc_mode_with_vivid_pt;
}

uint8_t app_hear_through_storage_get_anc_mode_with_ha_psap()
{
    return app_hear_through_const_config.anc_mode_with_ha_psap;
}

static uint32_t app_hear_through_storage_get_anc_path_mask(uint8_t anc_mode)
{
    if (anc_mode == APP_HEAR_THROUGH_ANC_MODE_HYBRID) {
        return AUDIO_ANC_CONTROL_RAMP_FF_L | AUDIO_ANC_CONTROL_RAMP_FB_L;
    } else if (anc_mode == APP_HEAR_THROUGH_ANC_MODE_FF) {
        return AUDIO_ANC_CONTROL_RAMP_FF_L;
    } else if (anc_mode == APP_HEAR_THROUGH_ANC_MODE_FB) {
        return AUDIO_ANC_CONTROL_RAMP_FB_L;
    } else {
        return 0;
    }
}

uint32_t app_hear_through_storage_get_anc_path_mask_for_ha_psap()
{
    return app_hear_through_storage_get_anc_path_mask(app_hear_through_const_config.anc_mode_with_ha_psap);
}

uint32_t app_hear_through_storage_get_anc_path_mask_for_vivid_pt()
{
    return app_hear_through_storage_get_anc_path_mask(app_hear_through_const_config.anc_mode_with_vivid_pt);
}

typedef struct {
    uint8_t     hear_through_mode; // 0 : vivid_pt, 1 : HA/PSAP
    uint8_t     hear_through_switch; // 0 : OFF, 1 : ON
    uint8_t     reserved_1[4];
} __attribute__((packed)) app_hear_through_user_configuration_t;

#define APP_HEAR_THROUGH_USER_CONFIGURATION_LEN     (sizeof(app_hear_through_user_configuration_t))

app_hear_through_user_configuration_t app_ht_user_config;
app_hear_through_user_configuration_t app_ht_user_config_from_nvkey;

void app_hear_through_storage_load_user_configuration()
{
    app_hear_through_user_configuration_t default_config;

    memset(&default_config, 0, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);
    memset(&app_ht_user_config, 0, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);

    bool read_user_config_result = app_hear_through_storage_read_data(NVID_APP_HEAR_THROUGH_USER_CONFIGURATION,
                                                                        APP_HEAR_THROUGH_USER_CONFIGURATION_LEN,
                                                                        (uint8_t *)&app_ht_user_config);

    bool read_config_result = app_hear_through_storage_read_data(NVID_APP_HEAR_THROUGH_CONFIGURATION,
                                                                    APP_HEAR_THROUGH_USER_CONFIGURATION_LEN,
                                                                    (uint8_t *)&default_config);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_load_user_configuration] read_user_config_result : %d, read_config_result : %d",
                        2,
                        read_user_config_result,
                        read_config_result);

    if (read_user_config_result == false) {
        if (read_config_result == false) {
            assert( 0 && "Read Hear through configuration failed" );
        } else {
            memcpy(&app_ht_user_config, &default_config, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);
        }
    }

    memcpy(&app_ht_user_config_from_nvkey, &app_ht_user_config, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_load_user_configuration] hear_through_mode : %d, hear_through_switch : %d",
                        2,
                        app_ht_user_config.hear_through_mode,
                        app_ht_user_config.hear_through_switch);

    if ((app_ht_user_config.hear_through_mode != APP_HEAR_THROUGH_MODE_VIVID_PT)
            && (app_ht_user_config.hear_through_mode != APP_HEAR_THROUGH_MODE_HA_PSAP)) {
        assert( 0 && "Do not supported hear through mode" );
    }
}

app_hear_through_mode_t app_hear_through_storage_get_hear_through_mode()
{
    return app_ht_user_config.hear_through_mode;
}

bool app_hear_through_storage_set_hear_through_mode(app_hear_through_mode_t new_mode)
{
    if ((new_mode != APP_HEAR_THROUGH_MODE_VIVID_PT)
            && (new_mode != APP_HEAR_THROUGH_MODE_HA_PSAP)) {
        return false;
    }

    if (app_ht_user_config.hear_through_mode != new_mode) {
        app_ht_user_config.hear_through_mode = new_mode;
    }

    return true;
}
bool app_hear_through_storage_get_hear_through_switch()
{
    return app_ht_user_config.hear_through_switch;
}

bool app_hear_through_storage_set_hear_through_switch(bool enable)
{
    if (app_ht_user_config.hear_through_switch != enable) {
        app_ht_user_config.hear_through_switch = enable;
    }

    return true;
}

bool app_hear_through_storage_save_user_configuration()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_save_user_configuration] hear_through_mode [new(%d) - old(%d)], hear_through_switch [new(%d) - old(%d)]",
                        4,
                        app_ht_user_config.hear_through_mode,
                        app_ht_user_config_from_nvkey.hear_through_mode,
                        app_ht_user_config.hear_through_switch,
                        app_ht_user_config_from_nvkey.hear_through_switch);

    if (memcmp(&app_ht_user_config, &app_ht_user_config_from_nvkey, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN) == 0) {
        return true;
    }

    nvkey_status_t write_result = nvkey_write_data(NVID_APP_HEAR_THROUGH_USER_CONFIGURATION,
                                                    (uint8_t *)&app_ht_user_config,
                                                    APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);

    if (write_result != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_STG_TAG"[app_hear_through_storage_save_user_configuration] Write user configuration failed, %d",
                            1,
                            write_result);
        return false;
    }

    /**
     * @brief Fix issue - 47470
     * If the configuration already save to nvkey succeed, need update the parameter to make sure
     * the save flow working correctly if the configuration change again.
     */
    memcpy(&app_ht_user_config_from_nvkey, &app_ht_user_config, APP_HEAR_THROUGH_USER_CONFIGURATION_LEN);

    return true;
}

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

