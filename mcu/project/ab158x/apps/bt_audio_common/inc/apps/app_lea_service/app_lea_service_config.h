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

#ifndef __APP_LEA_SERVICE_CONFIG_H__
#define __APP_LEA_SERVICE_CONFIG_H__

/**
 * File: app_lea_service_config.h
 *
 * Description: This file defines the configure option for LE Audio.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include "apps_customer_config.h"

#ifdef __cplusplus
extern "C" {
#endif



// Start general advertising 2 min when enter pairing mode (same as VISIBLE_TIMEOUT)
#define APP_LE_AUDIO_ADV_TIME                   (VISIBLE_TIMEOUT)
// Only start direct ADV & targeted announcement flag for a limited time (same as TIME_TO_STOP_RECONNECTION)
#define APP_LE_AUDIO_ACTIVE_RECONNECT_TIME      (TIME_TO_STOP_RECONNECTION)

// Start temp general ADV time
#define APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME      (20 * 1000)

#define APP_LE_AUDIO_MODE_DEFAULT               0

/*
 * The LE Audio Connection & advertising type.
 * Common Rule
 *     Stop ADV when LEA Link or total_conn_num full;
 *
 * 0: default (APP_LE_AUDIO_MODE_DEFAULT)
 *    MAX_LEA_LINK (EMP ON - 2, OFF - 1, Takeover +1)
 *    Start infinite Target ADV
 *    Start 2 min General ADV when EDR visible
 *    Support Direct ADV for link lost (Intel EVO)
 */
#define APP_LE_AUDIO_ADV_CONN_MODE              APP_LE_AUDIO_MODE_DEFAULT

#if (APP_LE_AUDIO_ADV_CONN_MODE == APP_LE_AUDIO_MODE_DEFAULT)
#define AIR_LE_AUDIO_WHITELIST_ADV
#ifdef MTK_AWS_MCE_ENABLE
#define AIR_LE_AUDIO_BOTH_SYNC_INFO
#endif
#ifdef AIR_BT_TAKEOVER_ENABLE
#define APP_LE_AUDIO_MAX_LINK_NUM               (2 + 1)       // +1 for takeover
#else
#define APP_LE_AUDIO_MAX_LINK_NUM               2
#endif
#endif



#define APP_LEA_ADV_NAME_PREFIX                 "LEA-"

#if !defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#define AIR_LEA_ULL2_KEY_TRIGGER_GENERAL_ADV
#endif

#define APP_LEA_MAX_BOND_NUM                    7             // Same as controller max LE white list, don't change
// If you want to modify the value, please tell us about modify reason
#if defined(AIR_BT_TAKEOVER_ENABLE) && (APP_LEA_MAX_BOND_NUM < 3)
#error "LEA MAX bond num should larger than or equal to 3"
#elif (APP_LEA_MAX_BOND_NUM > 7)
#error "LEA MAX bond num should <= 7"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
typedef enum {
    APP_LEA_FEATURE_MODE_OFF,
    APP_LEA_FEATURE_MODE_ON,
    APP_LEA_FEATURE_MODE_DUAL_MODE,
} app_lea_feature_mode_t;

extern app_lea_feature_mode_t                   app_lea_feature_mode;

#define APP_LE_AUDIO_ADV_INTERVAL_ENHANCE
#endif

typedef enum {
    APP_LEA_ADV_MODE_NONE,
    APP_LEA_ADV_MODE_GENERAL,                   /* General ADV for pairing mode (visible on). */
    APP_LEA_ADV_MODE_TARGET,                    /* Deprecated; Only start 1~2 active reconnect ADV for power on reconnect. */
    APP_LEA_ADV_MODE_TARGET_ALL,                /* Start inactive ADV with general announcement flag & active reconnect ADV (direct ADV, with targeted announcement flag) for all bonded device. */
} app_lea_adv_mgr_mode_t;

typedef enum {
    APP_LEA_ADV_SUB_MODE_NONE,
    APP_LEA_ADV_SUB_MODE_GENERAL,
    APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT,
    APP_LEA_ADV_SUB_MODE_INACTIVE,
    APP_LEA_ADV_SUB_MODE_DIRECT,
    APP_LEA_ADV_SUB_MODE_MAX,
} app_lea_adv_mgr_sub_mode_t;

#define APP_LEA_ADV_SUB_MODE_MASK(sub_mode)     (0x01U << (sub_mode))


#if defined(AIR_WIRELESS_MIC_ENABLE) && defined(AIR_AUDIO_ULD_CODEC_ENABLE)
#define APP_LE_AUDIO_ADV_INTERVAL_MIN           0x0004              /* 2.5 ms */
#define APP_LE_AUDIO_ADV_INTERVAL_MAX           0x0008              /* 5 ms */
#endif

#ifdef AIR_BT_INTEL_EVO_ENABLE
#define APP_LE_AUDIO_ADV_INTERVAL_MIN_S         0x0020              /* 20 ms */
#define APP_LE_AUDIO_ADV_INTERVAL_MAX_S         0x0028              /* 25 ms */
#define APP_LE_AUDIO_ADV_FAST_TIME              (20 * 1000)         /* 20 sec */
#else
#define APP_LE_AUDIO_ADV_INTERVAL_MIN_S         0x0020              /* 20 ms */
#define APP_LE_AUDIO_ADV_INTERVAL_MAX_S         0x0030              /* 30 ms */
#define APP_LE_AUDIO_ADV_FAST_TIME              (30 * 1000)         /* 30 sec */
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#define APP_LE_AUDIO_ADV_INTERVAL_MIN_L         0x00A0              /* 100 ms */
#define APP_LE_AUDIO_ADV_INTERVAL_MAX_L         0x00A0              /* 100 ms */
#else
#define APP_LE_AUDIO_ADV_INTERVAL_MIN_L         0x00F0              /* 150 ms */
#define APP_LE_AUDIO_ADV_INTERVAL_MAX_L         0x00F0              /* 150 ms */
#endif

#define APP_LE_AUDIO_ADV_INTERVAL_M             0x0050              /* 50 ms */


//#define APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME  500              /* 500 ms */
#define APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME  800              /* 800 ms */



// After start "LEA ADV with Targeted announcement flag" for active reconnect 2 min timeout (APP_LE_AUDIO_ACTIVE_RECONNECT_TIME)
// We need to stop "active connecting" operation, similar to EDR stop paging after 2min
// In order to this feature, we set "active_reconnect_type" to "temp_no_need"
// If enable the below option, after open lid or restart BT, we will restore all "temp_no_need" to "active_reconnect_type"
// Default off, because after EDR stop paging, device EDR not paging again (similar to EDR)
//#define APP_LE_AUDIO_ADV_RESTORE_TEMP_NO_NEED_TO_ACTIVE_RECONNECT

// Be similar to EDR "stop paging", cancel "LEA Direct ADV / Targeted announcement flag" when BT power on
// On APP_CONN app_bt_conn_manager_restore, need to set "active reconnect type" for last 1~2 remote source, be similar to initiating to paging EDR
#define APP_LE_AUDIO_ADV_CLEAR_LEA_ACTIVE_RECONNECT_TYPE_AFTER_POWER_ON

#ifdef AIR_LE_AUDIO_USE_DIRECT_ADV_TO_ACTIVE_RECONNECT
// As long as LEA is "active reconnect type", use direct ADV even if there are not other LEA source connected or "no_active_reconnect" type
#define APP_LE_AUDIO_ADV_ACTIVE_RECONNECT_TYPE_ALWAYS_USE_DIRECT_ADV
#endif

#ifndef MTK_AWS_MCE_ENABLE
// Only for headset project
#define APP_LE_AUDIO_ADV_STOP_ADV_WHEN_WIRED_AUDIO
#endif

#if defined(AIR_LE_AUDIO_DIRECT_ADV) && !defined(AIR_GATT_SRV_CLIENT_ENABLE)
#error "LEA Direct ADV need enable AIR_GATT_SRV_CLIENT_ENABLE"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_LEA_SERVICE_CONFIG_H__ */
