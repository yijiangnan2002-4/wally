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

/**
 * File: app_gsound_multi_va.c
 *
 * Description: This file is the activity to handle the action from multi-VA module.
 */

#ifdef AIR_GSOUND_ENABLE

#include "app_gsound_battery_ohd.h"

#include "gsound_api.h"

#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
#include "apps_events_battery_event.h"
#endif


#define LOG_TAG "[GS][APP][BAT]"

extern bool app_gsound_device_action_request_state(void);

typedef struct {
    bool           local_is_charging;
    bool           peer_is_charging;
    bool           case_is_charging;
    uint8_t        local_percent;
    uint8_t        peer_percent;
    uint8_t        case_percent;
} app_gsound_battery_info_t;

static app_gsound_battery_info_t app_gsound_battery_info = {0};



/**================================================================================*/
/**                                    Battery/OHD API                             */
/**================================================================================*/
static void app_gsound_battery_notify(void)
{
#ifdef MTK_AWS_MCE_ENABLE
    uint8_t local_percent = app_gsound_battery_info.local_percent;
    uint8_t peer_percent = app_gsound_battery_info.peer_percent;
    uint8_t min_percent = (local_percent <= peer_percent ? local_percent : peer_percent);
#else
    uint8_t min_percent = app_gsound_battery_info.local_percent;
#endif

    bool is_charging = app_gsound_battery_info.local_is_charging;
    gsound_battery_notify_info(is_charging, min_percent);
    GSOUND_LOG_I(LOG_TAG" notify, percent=%d is_charging=%d", 2, min_percent, is_charging);
}

void app_gsound_battery_set_info(uint8_t info_type, uint8_t data)
{
    bool update = FALSE;
    switch (info_type) {
        case APP_GSOUND_BATTERY_INFO_LOCAL_PERCENT:
            if (app_gsound_battery_info.local_percent != data) {
                app_gsound_battery_info.local_percent = data;
                update = TRUE;
                app_gsound_battery_notify();
            }
            break;
        case APP_GSOUND_BATTERY_INFO_LOCAL_IS_CHARGING:
            if (app_gsound_battery_info.local_is_charging != data) {
                app_gsound_battery_info.local_is_charging = data;
                update = TRUE;
                app_gsound_battery_notify();
            }
            break;
        case APP_GSOUND_BATTERY_INFO_PEER_PERCENT:
            if (app_gsound_battery_info.peer_percent != data) {
                app_gsound_battery_info.peer_percent = data;
                update = TRUE;
                app_gsound_battery_notify();
            }
            break;
        case APP_GSOUND_BATTERY_INFO_PEER_IS_CHARGING:
            if (app_gsound_battery_info.peer_is_charging != data) {
                app_gsound_battery_info.peer_is_charging = data;
                update = TRUE;
            }
            break;
        case APP_GSOUND_BATTERY_INFO_CASE_PERCENT:
            if (app_gsound_battery_info.case_percent != data) {
                app_gsound_battery_info.case_percent = data;
                update = TRUE;
            }
            break;
        case APP_GSOUND_BATTERY_INFO_CASE_IS_CHARGING:
            if (app_gsound_battery_info.case_is_charging != data) {
                app_gsound_battery_info.case_is_charging = data;
                update = TRUE;
            }
            break;
    }

    if (update) {
        //GSOUND_LOG_I(LOG_TAG" battery_set_info, update info_type=%d data=%d", 2, info_type, data);
        app_gsound_device_action_request_state();
    }
}

uint8_t app_gsound_battery_get_info(uint8_t info_type)
{
    uint8_t data = 0;
    switch (info_type) {
        case APP_GSOUND_BATTERY_INFO_LOCAL_PERCENT:
            data = app_gsound_battery_info.local_percent;
            break;
        case APP_GSOUND_BATTERY_INFO_LOCAL_IS_CHARGING:
            data = app_gsound_battery_info.local_is_charging;
            break;
        case APP_GSOUND_BATTERY_INFO_PEER_PERCENT:
            data = app_gsound_battery_info.peer_percent;
            break;
        case APP_GSOUND_BATTERY_INFO_PEER_IS_CHARGING:
            data = app_gsound_battery_info.peer_is_charging;
            break;
        case APP_GSOUND_BATTERY_INFO_CASE_PERCENT:
            data = app_gsound_battery_info.case_percent;
            break;
        case APP_GSOUND_BATTERY_INFO_CASE_IS_CHARGING:
            data = app_gsound_battery_info.case_is_charging;
            break;
    }
    GSOUND_LOG_I(LOG_TAG" battery_get_info, info_type=%d data=%d", 2, info_type, data);
    return data;
}

void app_gsound_battery_handle_peer_battery(uint8_t peer_data)
{
    //GSOUND_LOG_I(LOG_TAG" handle_peer_battery, peer_data=%d", peer_data);
    uint8_t peer_percent = peer_data & 0x7F;
    bool peer_is_charging = (peer_data & 0x80) ? TRUE : FALSE;

    GSOUND_LOG_I(LOG_TAG" handle_peer_battery, peer_data=%d is_charging=%d percent=%d",
                 3, peer_data, peer_is_charging, peer_percent);
    app_gsound_battery_set_info(APP_GSOUND_BATTERY_INFO_PEER_PERCENT, peer_percent);
    app_gsound_battery_set_info(APP_GSOUND_BATTERY_INFO_PEER_IS_CHARGING, peer_is_charging);
}

void app_gsound_battery_init(void)
{
    memset(&app_gsound_battery_info, 0, sizeof(app_gsound_battery_info_t));

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    uint32_t percent = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
    bool is_charging = (battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST) > 0);
#else
    uint32_t percent = 100;
    bool is_charging = FALSE;
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
    percent = apps_events_get_optimal_battery();
#endif
    app_gsound_battery_info.local_percent = percent;
    app_gsound_battery_info.local_is_charging = is_charging;
    //GSOUND_LOG_I(LOG_TAG" init, percent=%d is_charging=%d", 2, percent, is_charging);

    gsound_battery_notify_info(is_charging, percent);
}

gsound_ohd_state app_gsound_ohd_get_state()
{
    gsound_ohd_state ohd_state = GSOUND_OHD_DISABLE;
#ifdef MTK_IN_EAR_FEATURE_ENABLE
    app_in_ear_ohd_state_t in_ear = app_in_ear_get_ohd_state();
    switch (in_ear) {
        case APP_IN_EAR_OHD_BOTH_DETECTED:
            ohd_state = GSOUND_OHD_BOTH_DETECTED;
            break;
        case APP_IN_EAR_OHD_DETECTED:
            ohd_state = GSOUND_OHD_SINGULAR_DETECTED;
            break;
        case APP_IN_EAR_OHD_NONE_DETECTED:
            ohd_state = GSOUND_OHD_NONE_DETECTED;
            break;
        case APP_IN_EAR_OHD_LEFT_DETECTED:
            ohd_state = GSOUND_OHD_LEFT_DETECTED;
            break;
        case APP_IN_EAR_OHD_RIGHT_DETECTED:
            ohd_state = GSOUND_OHD_RIGHT_DETECTED;
            break;
        default:
            ohd_state = GSOUND_OHD_UNSET;
            break;
    }
#endif
    //GSOUND_LOG_I(LOG_TAG" [OHD] app_gsound_get_ohd_state, %d", 1, ohd_state);
    return ohd_state;
}

#endif /* AIR_GSOUND_ENABLE */
