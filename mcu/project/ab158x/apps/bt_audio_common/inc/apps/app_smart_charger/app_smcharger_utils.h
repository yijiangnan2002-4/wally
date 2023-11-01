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


#ifndef __APP_SMCHARGER_UTILS_H__
#define __APP_SMCHARGER_UTILS_H__

#ifdef AIR_SMART_CHARGER_ENABLE

#include "app_smcharger.h"

#include "ui_shell_manager.h"
#include "ui_shell_activity.h"

#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"
#include "apps_debug.h"
#include "apps_events_battery_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif

#include "bt_aws_mce_srv.h"
#include "bt_sink_srv.h"
#include "bt_power_on_config.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"

#include "FreeRTOS.h"



/**
 *  @brief This enum defines the states of battery_app.
 */
typedef enum {
    SMCHARGER_BATTERY_STATE_SHUTDOWN,         /**<  Shutdown state for LOW VOLTAGE, the device will power off if not charging. */
    SMCHARGER_BATTERY_STATE_LOW_CAP,          /**<  Low battery state, the device will play VP "low battery, please charge!". */
    SMCHARGER_BATTERY_STATE_IDLE,             /**<  Idle state, not charging (out of case). */
    SMCHARGER_BATTERY_STATE_FULL,             /**<  Full battery state, not charging (out of case). */
    SMCHARGER_BATTERY_STATE_CHARGING,         /**<  Charging state. */
    SMCHARGER_BATTERY_STATE_CHARGING_FULL,    /**<  Charging full state. */
    SMCHARGER_BATTERY_STATE_THR               /**<  High temperature state. */
} app_smcharger_battery_state_t;

#define SMCHARGER_BOOT_OUT_FLAG  0xFFF1 /* Special flag in Startup activity. */

/**
 *  @brief This structure defines the context of SmartCharger.
 */
typedef struct {
    uint8_t                 battery_percent;            /**<  The battery percent of current device. */
    uint8_t                 peer_battery_percent;       /**<  Peer battery. */
    uint8_t                 case_battery_percent;       /**<  The battery percent of charger case. */
    uint8_t                 smcharger_state;            /**<  SmartCharger state of current device. */
    uint8_t                 peer_smcharger_state;       /**<  Peer SmartCharger state. */
    battery_event_shutdown_state_t     shutdown_state;  /**<  Shutdown_state in battery_management. */
    app_smcharger_battery_state_t      battery_state;   /**<  The states of battery. */
    bool                    agent_prepare_rho_flag;     /**<  TRUE means prepare to RHO after 1000ms. */
    bool                    bt_clear_ongoing;           /**<  TRUE means BT is clearing info. */
} app_smcharger_context_t;

/**
 *  @brief This enum defines the SmartCharger status.
 */
typedef enum {
    APP_SMCHARGER_FAILURE = -1,
    APP_SMCHARGER_OK = 0,
} app_smcharger_action_status_t;

/**
 * @brief      This function is SmartCharger power off API.
 * @param[in]  normal_off, TRUE - LOW_VOLTAGE power off outside of SmartCharger case, play "power off" VP and foreground led pattern;
 *             FALSE - immediately power off due to charging completely or SmartCharger case low battery.
 * @return     If return APP_SMCHARGER_OK, means successfully, otherwise failure.
 */
app_smcharger_action_status_t app_smcharger_power_off(bool normal_off);

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
/**
 * @brief      This function is used to update power_saving status.
 * @return     app_power_saving_target_mode_t.
 */
app_power_saving_target_mode_t app_smcharger_get_power_saving_target_mode(void);
#endif

/**
 * @brief      This function is used to switch state and do action.
 * @param[in]  state, current SmartCharger APP state.
 * @return     If return APP_SMCHARGER_OK, means successfully, otherwise failure.
 */
app_smcharger_action_status_t app_smcharger_state_do_action(uint8_t state);

/**
 * @brief      SmartCharger APP init function.
 */
void app_smcharger_init();

/**
 * @brief      This function is used to set app_smcharger_context_t.
 * @param[in]  smcharger_ctx, SmartCharger APP context.
 */
void app_smcharger_set_context(app_smcharger_context_t *smcharger_ctx);

/**
 * @brief      This function is used to return battery state.
 * @return     SmartCharger APP battery state.
 */
app_smcharger_battery_state_t app_smcharger_get_battery_state();

/**
 * @brief      This function is used to update background LED pattern.
 * @return     TRUE - the APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN event cannot transfer to the next activity.
 */
bool app_smcharger_show_led_bg_pattern();

/**
 * @brief      This function is used to handle SmartCharger case battery percent.
 * @param[in]  case_battery, SmartCharger case battery percent.
 */
void app_smcharger_handle_case_battery(uint8_t case_battery);

/**
 *  @brief This enum defines the states of battery_app.
 */
 #if 0	// original
 typedef enum {
    APP_SMCHARGER_KEY_SHARE_MODE_AGENT = 0,
    APP_SMCHARGER_KEY_SHARE_MODE_FOLLOWER = 1,
    APP_SMCHARGER_KEY_BT_DISCOVERABLE,
    APP_SMCHARGER_KEY_BT_CLEAR,
    APP_SMCHARGER_KEY_BT_AIR_PAIRING
} app_smcharger_key_event_t;
 #else	// richard for customer UI spec
uint8_t app_smcharger_get_state1(void);
uint8_t app_smcharger_get_earbud_state(void);

typedef enum {
    APP_SMCHARGER_KEY_SHARE_MODE_AGENT = 0,
    APP_SMCHARGER_KEY_SHARE_MODE_FOLLOWER = 1,
    APP_SMCHARGER_KEY_BT_DISCOVERABLE =2,
    APP_SMCHARGER_KEY_BT_CLEAR =3,
    APP_SMCHARGER_KEY_BT_AIR_PAIRING =4,
    APP_SMCHARGER_KEY_BT_SIGNAL_DISCOVERABLE = 5,
    APP_SMCHARGER_KEY_BT_DUT_TEST = 6,
    APP_SMCHARGER_KEY_TWS_CLEAN = 7,
    APP_SMCHARGER_KEY_FACTORY_RESET = 8,
} app_smcharger_key_event_t;
#endif

/**
 * @brief      This function is used to handle SmartCharger Key event and data.
 * @param[in]  key_value, current SmartCharger Key data.
 */
void app_smcharger_handle_key_event(uint32_t key_value);

/**
 * @brief      This function is used to handle SmartCharger user data.
 * @param[in]  state, current SmartCharger APP state.
 * @param[in]  event_id, USER_DATA1 or USER_DATA2 or USER_DATA3 event.
 * @param[in]  user_data, current SmartCharger user data.
 */
void app_smcharger_handle_user_data(int state, int event_id, uint8_t user_data);

void app_mute_audio(bool is_mute);


/**
 *  @brief This enum defines the step of Clear BT.
 */
typedef enum {
    APP_SMCHARGER_REQUEST_NONE = 0,
    APP_SMCHARGER_REQUEST_BT_OFF,
    APP_SMCHARGER_REQUEST_BT_CLEAR_ALL,
    APP_SMCHARGER_REQUEST_BT_ON,
    APP_SMCHARGER_REQUEST_BT_DISCOVERABLE
} app_smcharger_clear_bt_step_t;

/**
 * @brief      This function is used to clear and enter discoverable.
 * @param[in]  step, SmartCharger BT clear step.
 */
void app_smcharger_bt_clear_enter_discoverable(app_smcharger_clear_bt_step_t step);

/**
 * @brief      This function is used to send UI shell event.
 * @param[in]  event_id, EVENT_ID_SMCHARGER_XXX.
 * @param[in]  data, SmartCharger event data.
 * @param[in]  data_len, event data length.
 */
bool app_smcharger_ui_shell_event(bool from_isr, int event_id, void *data, size_t data_len);

/**
 * @brief      This function is used to update and save charger case battery.
 * @param[in]  case_battery, SmartCharger case battery.
 */
void app_smcharger_update_case_battery_nvkey(uint8_t case_battery);

// richard for customer UI spec
app_smcharger_context_t* app_get_smcharger_context(void);
#endif
#endif
