
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
 * File: app_va_xiaoai_ble_adv.h
 *
 * Description: This file defines the interface of app_va_xiaoai_ble_adv.c.
 *
 */

#ifndef __APP_VA_XIAOAI_BLE_ADV_H__
#define __APP_VA_XIAOAI_BLE_ADV_H__

#ifdef AIR_XIAOAI_ENABLE

#include "stdbool.h"
#include "stdint.h"
#include "bt_type.h"
#include "multi_ble_adv_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief This enum defines the events of group EVENT_GROUP_UI_SHELL_XIAOAI.
 */
typedef enum {
    XIAOAI_EVENT_UNBIND_ACTION = 0x1000,          /**<  The event to start unbind BT EDR. */
    XIAOAI_EVENT_RECONNECT_EDR_ACTION,            /**<  The event to reconnect BT EDR. */
    XIAOAI_EVENT_DISCONNECT_EDR_ACTION,           /**<  The event to disconnect BT EDR. */
    XIAOAI_EVENT_START_BLE_ADV_ACTION = 0x2000,   /**<  The event to start XiaoAI BLE ADV. */
    XIAOAI_EVENT_STOP_BLE_ADV_ACTION,             /**<  The event to stop XiaoAI BLE ADV. */
    XIAOAI_EVENT_AWS_ACTION,                      /**<  The event to AWS action. */
    XIAOAI_EVENT_DEVICE_CONFIG_ACTION,            /**<  The event to device config. */
    XIAOAI_EVENT_DEVICE_LD_CHECK_IN_EAR,          /**<  The event to reply in_ear result before start LD. */
    XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION,
#ifdef AIR_XIAOAI_PTT_ENABLE
    XIAOAI_EVENT_START_PTT_ACTIVITY = 0x3000,      /**<  The event to start PTT activity. */
    XIAOAI_EVENT_FINISH_PTT_ACTIVITY,             /**<  The event to finish PTT activity. */
#endif
#ifdef AIR_XIAOAI_WWE_ENABLE
    XIAOAI_EVENT_START_WWE_ACTION = 0x4000,       /**<  The event to start WWE. */
    XIAOAI_EVENT_STOP_WWE_ACTION,                 /**<  The event to stop WWE. */
    XIAOAI_EVENT_RESTART_WWE_ACTION,              /**<  The event to restart WWE. */
    XIAOAI_EVENT_WWE_DETECTED_ACTION,             /**<  The event to notify WWE notification. */
    XIAOAI_EVENT_WWE_DATA_ACTION,                 /**<  The event to notify Data notification. */
    XIAOAI_EVENT_WWE_HFP_SUSPEND,                 /**<  The event to notify HFP suspend. */
    XIAOAI_EVENT_WWE_LE_CALL_SUSPEND,             /**<  The event to notify LE Call suspend. */
    XIAOAI_EVENT_WWE_LD_SUSPEND,                  /**<  The event to notify LD suspend. */
    XIAOAI_EVENT_WWE_AUDIO_DVFS_UNLOCK,           /**<  The event to notify DVFS UNLOCK after started and DVFS LOCK. */
#endif
    XIAOAI_EVENT_SYNC_LEAKAGE_RESULT = 0x5000,    /**<  The event to sync leakage result (partner to agent). */
    XIAOAI_EVENT_SYNC_SAVING_CONFIG,              /**<  The event to sync saving_config (agent to partner). */
    XIAOAI_EVENT_SYNC_DEVICE_NAME,                /**<  The event to sync device name (agent to partner). */
    XIAOAI_EVENT_SYNC_LD_RESULT,

    XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_TO_AGENT,           // 08/F2 cmd
    XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_RESULT_TO_PARTNER,  // 08/F2 result
    XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_NOTIFY_TO_PEER,     // 0xF4
    XIAOAI_EVENT_LEA_SYNC_AT_CMD_TO_PEER,                   // HFP AT CMD
    XIAOAI_EVENT_LEA_SYNC_NOTIFY_SP_TO_PEER,                // 07/0E

#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    XIAOAI_EVENT_MIUI_FC_PAIRING_REQUEST = 0x6000,
    XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST_TIMER,
    XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST,
    XIAOAI_EVENT_MIUI_FC_ACCOUNT_KEY_REQUEST,
    XIAOAI_EVENT_MIUI_FC_SYNC_ACCOUNT_KEY,
    XIAOAI_EVENT_MIUI_FC_RESET_REQUEST,
#endif

    XIAOAI_EVENT_OTA_SLIENCE_OTA_SYNC = 0x7000,
    XIAOAI_EVENT_OTA_SLIENCE_OTA_TIMEROUT,
} multi_va_event_id_t;

/**
* @brief      This function is used to send UI Shell event to start/stop XiaoAI BLE ADV.
* @param[in]  enable, TRUE - start to advertise, FALSE - stop advertise.
* @param[in]  adv_interval_ms, XiaoAI BLE advertising interval parameter.
* @param[in]  adv_time_ms, XiaoAI BLE ADV duration time, will stop advertising after adv_time_ms.
*/
void app_va_xiaoai_send_ble_adv_event(bool enable, uint16_t adv_interval_ms, int adv_time_ms);

/**
* @brief      This function is used to start/stop XiaoAI BLE ADV.
* @param[in]  enable, TRUE - start to advertise, FALSE - stop advertise.
* @param[in]  adv_interval_ms, XiaoAI BLE advertising interval parameter.
*/
bool app_va_xiaoai_start_ble_adv(bool enable, uint16_t advertising_interval);

#ifdef __cplusplus
}
#endif

#endif /* AIR_XIAOAI_ENABLED */

#endif /* __APP_VA_XIAOAI_BLE_ADV_H__ */

