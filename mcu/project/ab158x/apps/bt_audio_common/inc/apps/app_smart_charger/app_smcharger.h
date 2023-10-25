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

/**
 * File: app_smcharger_utils.h
 *
 * Description: This file defines the interface of app_smcharger_utils.c.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine.
 *
 */


#ifndef __APP_SMCHARGER_H__
#define __APP_SMCHARGER_H__

#ifdef AIR_SMART_CHARGER_ENABLE

#include "stdbool.h"
#include "stdint.h"

/**================================================================================*/
/**                           PUBLIC API for other APP                             */
/**================================================================================*/

#define APP_SMCHARGER_DELAY_OFF_BT_CLASSIC_TIMER            (500)

/**
 *  @brief This enum defines the SmartCharger internal UI_Shell Event.
 *  Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine and internal event.
 */
enum {
    SMCHARGER_EVENT_NONE = 0,                /**<  0. None Event ID. */
    SMCHARGER_EVENT_POWER_KEY_BOOT,          /**<  The event to trigger Startup->Out_of_Case, means device power on via long_press Power_Key in out_of_case. */
    SMCHARGER_EVENT_CHARGER_IN_BOOT,         /**<  The event to trigger Startup->Lid_open, means device power on via charging in the SmartCharger case. */
    SMCHARGER_EVENT_CHARGER_IN,              /**<  The event to trigger Out_of_Case->Lid_open, means device is put into SmartCharger case. */
    SMCHARGER_EVENT_CHARGER_COMPLETE,        /**<  Unused. */
    SMCHARGER_EVENT_CHARGER_OUT,             /**<  5. The event to trigger Lid_open/Lid_close->Out_of_Case, means device is take out from SmartCharger case. */
    SMCHARGER_EVENT_CHARGER_OFF,             /**<  The event to trigger Lid_open/Lid_close->Off, means device charging completely or SmartCharger case low battery. */
    SMCHARGER_EVENT_CHARGER_KEY,             /**<  The event means SmartCharger key is clicked. */
    SMCHARGER_EVENT_LID_OPEN,                /**<  The event to trigger Lid_close->Lid_open, means SmartCharger open lid (device in the case). */
    SMCHARGER_EVENT_LID_CLOSE,               /**<  The event means SmartCharger close lid. */
    SMCHARGER_EVENT_LID_CLOSE_COMPLETE,      /**<  10. The event means SmartCharger close lid 3 sec. */
    SMCHARGER_EVENT_USER_DATA,               /**<  The event means SmartCharger send USER_DATA. */
    SMCHARGER_EVENT_CASE_BATTERY_REPORT,     /**<  The event means case battery report. */
    SMCHARGER_EVENT_SYNC_STATE,              /**<  The event means partner will sync own state to agent and agent will check RHO. */
    SMCHARGER_EVENT_PREPARE_RHO,             /**<  The event means device will send APP RHO event. */
    SMCHARGER_EVENT_RESTARTUP,               /**<  15. (For TILE) The event means SmartCharger send EVENT_ID_SMCHARGER_RESTARTUP_EVENT to restart. */
    SMCHARGER_EVENT_SYNC_DATA_TO_PEER,       /**<  The event means SmartCharger sync battery/case_bat/state to peer. */
    SMCHARGER_EVENT_LID_CLOSE_OUT_TIMER,     /**<  The event means SmartCharger out_of_case timer timeout in LID_CLOSE state. */

    // PUBLIC Event for other APP
    SMCHARGER_EVENT_NOTIFY_ACTION,           /**<  Public Event. The event means SmartCharger APP will notify public event with action (app_smcharger_action_t) to other APP. */
    SMCHARGER_EVENT_NOTIFY_BOTH_IN_OUT,      /**<  Public Event. The event means SmartCharger APP will notify Charging changed event to other APP. */
    SMCHARGER_EVENT_NOTIFY_BOTH_CHANGED,     /**<  Public Event. Agent will notify other APP when own/peer/case battery/charging changed/AWS disconnect/LID open/close. */
};

/**
 *  @brief This enum defines the states of SmartCharger main state machine.
 *  Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine.
 */
typedef enum {
    STATE_SMCHARGER_NONE = 0,           /**<  None state. */
    STATE_SMCHARGER_STARTUP,            /**<  SmartCharger Startup state. */
    STATE_SMCHARGER_LID_CLOSE,          /**<  SmartCharger lid_close state. */
    STATE_SMCHARGER_LID_OPEN,           /**<  SmartCharger lid_open state. */
    STATE_SMCHARGER_OUT_OF_CASE,        /**<  SmartCharger out_of_case state. */
    STATE_SMCHARGER_OFF                 /**<  SmartCharger off state. */
} app_smcharger_state;

/**
 *  @brief This enum defines the action of SmartCharger.
 *  SmartCharger will notify app_smcharger_public_event_para_t with action to other APP.
 */
typedef enum {
    SMCHARGER_CHARGER_IN_ACTION = 0,        /**<  The action to device charging (in the case). */
    SMCHARGER_CHARGER_COMPLETE_ACTION,      /**<  The action to device charging complete. */
    SMCHARGER_OPEN_LID_ACTION,              /**<  The action to SmartCharger open lid. */
    SMCHARGER_CLOSE_LID_ACTION,             /**<  The action to SmartCharger close lid. */
    SMCHARGER_CHARGER_OUT_ACTION,           /**<  The action to device not charging (out of case). */
    SMCHARGER_CHARGER_KEY_ACTION,           /**<  The action to SmartCharger click key. */
    SMCHARGER_USER_DATA_ACTION,             /**<  The action to SmartCharger send user_data. */
    SMCHARGER_CASE_BATTERY_REPORT_ACTION    /**<  The action to SmartCharger report battery percent. */
} app_smcharger_action_t;

/**
 *  @brief This structure defines the context of app_smcharger_public_event_para_t. (see SMCHARGER_EVENT_NOTIFY_ACTION)
 */
typedef struct {
    app_smcharger_action_t action;          /**<  The action is used to notify other APP from SmartCharger APP. */
    uint8_t                data;            /**<  Action data (default 0), only for BATTERY_REPORT_ACTION, CHARGER_KEY_ACTION, USER_DATA action. */
} app_smcharger_public_event_para_t;

/**
 * @brief      This function is used to update Left/Right/Agent/Partner/Case battery and set to other APP.
 *             Now, only XiaoAI used.
 */
void app_smcharger_update_bat();

typedef enum {
    APP_SMCHARGER_NONE = 0,
    APP_SMCHARGER_IN,
    APP_SMCHARGER_OUT
} app_smcharger_in_out_t;

/**
 * @brief      This function is used to get local charging flag.
 * @return     app_smcharger_in_out_t.
 */
app_smcharger_in_out_t app_smcharger_is_charging();

/**
 * @brief      This function is used to get peer charging flag.
 * @return     app_smcharger_in_out_t.
 */
app_smcharger_in_out_t app_smcharger_peer_is_charging();

/**
 * @brief      This function is used to get charger state.
 * @return     app_smcharger_state.
 */
app_smcharger_state app_smcharger_get_state();

/**
 * @brief      This function is used to get all battery info.
 * @param[in]  own_battery, SmartCharger own battery.
 * @param[in]  peer_battery, SmartCharger peer battery.
 * @param[in]  case_battery, SmartCharger case battery.
 */
void app_smcharger_get_battery_percent(uint8_t *own_battery, uint8_t *peer_battery, uint8_t *case_battery);

/**
 * @brief      This function is used to read current charger case battery.
 * @return     saved SmartCharger case battery, default 0.
 */
uint8_t app_smcharger_read_case_battery_nvkey();

#endif

#endif
