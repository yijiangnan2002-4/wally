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
 * File: app_smcharger_utils.c
 *
 * Description: This file provides many utility function for SmartCharger APP.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine.
 *
 */


#ifdef AIR_SMART_CHARGER_ENABLE

/* Enable SmartCharger APP AT-CMD for test. */
//#define AIR_SMART_CHARGER_TEST

#include "app_smcharger_utils.h"

#include "apps_aws_sync_event.h"
#include "apps_config_audio_helper.h"
#include "apps_config_event_list.h"
#include "apps_events_key_event.h"
#include "battery_management.h"
#include "hal_pmu.h"
#include "smartcharger.h"
#include "smartcase.h"
#include "bt_power_on_config.h"
#include "bt_sink_srv_ami.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"

#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_le_audio_util.h"
#include "bt_sink_srv_le_volume.h"
#endif
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service.h"
#endif
#ifdef AIR_XIAOAI_ENABLE
#include "xiaoai.h"
#endif
#ifdef AIR_PROMPT_SOUND_ENABLE
#include "prompt_control.h"
#endif

#ifdef AIR_TILE_ENABLE
#include "app_tile.h"
#include "app_bt_state_service.h"
#endif

// richard for customer UI spec.
//#include "app_customer_common.h"
#include "apps_config_state_list.h"
//#include "app_psensor_px31bf_activity.h"
//#include "app_hall_sensor_activity.h"
#include "apps_config_key_remapper.h"

/* SmartCharger APP must use Battery Manager module. */
#if !defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#error "For SmartCharger feature, please enable MTK_BATTERY_MANAGEMENT_ENABLE"
#endif
/* SmartCharger APP must use AWS_MCE module (AIR_TWS_ENABLE) on earbuds project. */
#ifndef AIR_TWS_ENABLE
#error "For SmartCharger feature, please enable AIR_TWS_ENABLE"
#endif

#define LOG_TAG                  "[SMCharger][Util]"

#ifdef AIR_SMART_CHARGER_TEST
#include "atci.h"

/* Define SmartCharger APP AT-CMD structure. */
static atci_status_t smcharger_atci_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_cmd_hdlr_item_t smcharger_atci_cmd[] = {
    {
        .command_head = "AT+SMC",
        .command_hdlr = smcharger_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};
#endif

/* SmartCharger APP global context. */
static app_smcharger_context_t *g_smcharger_context = NULL;
static app_smcharger_action_status_t app_smcharger_switch_bt(bool on, bool direct_off);

bool app_smcharger_ui_shell_event(bool from_isr, int event_id, void *data, size_t data_len)
{
    APPS_LOG_MSGID_I(LOG_TAG" send_event, from_isr=%d event_id=%d", 2, from_isr, event_id);
    ui_shell_status_t ret = ui_shell_send_event(from_isr, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_CHARGER_CASE,
                                                event_id, data, data_len, NULL, 0);
    return (ret == UI_SHELL_STATUS_OK);
}

static void app_smcharger_ui_shell_public_event(bool from_isr, app_smcharger_action_t action, uint8_t data)
{
    app_smcharger_public_event_para_t *event_para = (app_smcharger_public_event_para_t *)
                                                    pvPortMalloc(sizeof(app_smcharger_public_event_para_t));
    if (event_para == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" shell_public_event, malloc fail", 0);
        return;
    }

    event_para->action = action;
    event_para->data = data;
    bool ret = app_smcharger_ui_shell_event(from_isr, SMCHARGER_EVENT_NOTIFY_ACTION,
                                            (void *)event_para, sizeof(app_smcharger_public_event_para_t));
    APPS_LOG_MSGID_I(LOG_TAG" shell_public_event, from_isr=%d action=%d data=%d ret=%d",
                     4, from_isr, action, data, ret);
}

/**
* @brief      SmartCharger APP handle case battery report.
* @param[in]  from_isr, event callback from ISR.
* @param[in]  data, data from SmartCharger case.
*/
static void app_smcharger_handle_case_battery_report(bool from_isr, uint8_t data)
{
    // case_battery_percent include bit7 (charging_flag)
    // cannot be 0, because case low battery will send charger_off -> earbuds power off
    uint8_t case_battery_percent = data;

//    APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, case_battery_percent %d->%d", 2,
//                     (g_smcharger_context != NULL ? g_smcharger_context->case_battery_percent : 0),
//                     case_battery_percent);

    /* Update SmartCharger APP context->case_battery_percent and send internal event. */
    if (g_smcharger_context != NULL
        && g_smcharger_context->case_battery_percent != case_battery_percent
        && (case_battery_percent <= 100 || (case_battery_percent > 128 && case_battery_percent <= 228))) {
        g_smcharger_context->case_battery_percent = case_battery_percent;
        app_smcharger_ui_shell_event(from_isr, SMCHARGER_EVENT_CASE_BATTERY_REPORT,
                                     (void *)(uint32_t)case_battery_percent, 0);
    }
}

/**
* @brief      SmartCharger middleware callback function.
* @param[in]  drv_event, SmartCharger middleware event, see middleware/MTK/smart_charger/inc/smartcharger.h.
* @param[in]  from_isr, event callback from ISR.
* @param[in]  data, data from SmartCharger case.
* @param[in]  data_len, data length.
*/
static void app_smcharger_driver_callback(uint8_t drv_event, uint8_t from_isr, uint32_t data, uint16_t data_len)
{
    APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, drv_event=%d from_isr=%d data=%d data_len=%d",
                     4, drv_event, from_isr, data, data_len);
    bool need_send_event = FALSE;
    int event_id = SMCHARGER_EVENT_NONE;
    switch (drv_event) {
        /* SmartCharger middleware LID_OPEN Command. */
        case SMCHG_LID_OPEN: {
            event_id = SMCHARGER_EVENT_LID_OPEN;
            if (g_smcharger_context != NULL && g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_OPEN) {
                APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, ignore LID_OPEN in LID_OPEN State", 0);
            } else {
                need_send_event = TRUE;
            }
            if (data_len == 1) {
                app_smcharger_handle_case_battery_report(from_isr, (uint8_t)data);
            }
            break;
        }
        /* SmartCharger middleware USER1 data Command (PRE_CLOSE Command) - start to close lid. */
        case SMCHG_LID_CLOSE: {
            event_id = SMCHARGER_EVENT_LID_CLOSE;
            if (g_smcharger_context != NULL && g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_CLOSE) {
                APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, ignore LID_CLOSE in LID_CLOSE State", 0);
            } else {
                need_send_event = TRUE;
            }
            break;
        }
        /* SmartCharger middleware LID_CLOSE complete Command - The command is sent after close lid and wait 3 sec. */
        case SMCHG_LID_CLOSE_DONE: {
            // event_id = EVENT_ID_SMCHARGER_LID_CLOSE_COMPLETE_CMD;
            // need_send_event = TRUE;
            APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, LID_CLOSE_COMPLETE_CMD", 0);
            break;
        }
        /* SmartCharger middleware CHARGER_OFF Command. */
        case SMCHG_CHG_OFF: {
            event_id = SMCHARGER_EVENT_CHARGER_OFF;
            if (g_smcharger_context != NULL && g_smcharger_context->smcharger_state == STATE_SMCHARGER_OFF) {
                APPS_LOG_MSGID_I(LOG_TAG" [DRV]callback, ignore OFF in OFF State", 0);
            } else {
                need_send_event = TRUE;
            }
            if (data_len == 1) {
                app_smcharger_handle_case_battery_report(from_isr, data);
            }
            break;
        }
        /* SmartCharger middleware CHARGER_IN, i.e. battery_manager charger_exist is 1. */
        case SMCHG_CHG_IN: {
            event_id = SMCHARGER_EVENT_CHARGER_IN;
            need_send_event = TRUE;
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CHARGER_CASE, event_id);
            break;
        }
        case SMCHG_CHG_COMPL: {
            need_send_event = FALSE;
            app_smcharger_ui_shell_public_event(from_isr, SMCHARGER_CHARGER_COMPLETE_ACTION, 0);
            break;
        }
        /* SmartCharger middleware CHARGER_OUT. */
        case SMCHG_CHG_OUT: {
            event_id = SMCHARGER_EVENT_CHARGER_OUT;
            need_send_event = TRUE;
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CHARGER_CASE, event_id);
            break;
        }
        /* SmartCharger middleware CHARGER_KEY Command. */
        case SMCHG_CHG_KEY: {
            event_id = SMCHARGER_EVENT_CHARGER_KEY;
            if (data_len == 1) {
                need_send_event = TRUE;
            }
            break;
        }
        /* SmartCharger middleware USER DATA Command. */
        case SMCHG_USER_DATA: {
            event_id = SMCHARGER_EVENT_USER_DATA;
            need_send_event = TRUE;
            break;
        }
	// richard for UI spec
		case DRV_CHARGER_EVENT_BATTERY_LEVEL: {
			APPS_LOG_MSGID_I(" driver_callback[battery level]", 0);
			break;
		}
		case DRV_CHARGER_EVENT_CHARGER_STATE: {
			APPS_LOG_MSGID_I(" driver_callback[charger state]", 0);
			break;
		}
		case DRV_CHARGER_EVENT_REVERSION_REPORT: {
			log_hal_msgid_info(" driver_callback[casing version] = 0x%x", 1, data);
			app_set_charger_case_version((uint8_t)data);
			break;	
		}
		case DRV_CHARGER_EVENT_CHARGING_CURRENT_LIMIT: {
			log_hal_msgid_info(" driver_callback[set limit status] = 0x%x", 1, data);
			app_smcharger_set_current_limit_status((uint8_t)data);
			break;
		}
		case DRV_CHARGER_EVENT_SHIPPING_MODE_ENABLE: {
			log_hal_msgid_info(" driver_callback[shipping mode] = 0x%x", 1, data);
			app_nvkey_shipping_mode_set(0x01);
			break;	
		}
		case DRV_CHARGER_EVENT_EOC_CHECKING: {
			log_hal_msgid_info(" driver_callback[eoc checking] = 0x%x", 1, data);
			break;					
		}
        default: {
            break;
        }
    }

    /* Need to send internal event when need_send_event is 1. */
    if (need_send_event) {
        app_smcharger_ui_shell_event(from_isr, event_id, (void *)(uint32_t)data, 0);
    }
}

/**
* @brief      SmartCharger APP check system boot reason and send event.
*/
static app_smcharger_action_status_t app_smcharger_handle_system_boot()
{
    extern uint8_t pmu_get_power_on_reason();
    int32_t charger_exitst = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
    uint8_t reason = pmu_get_power_on_reason();
    APPS_LOG_MSGID_I(LOG_TAG" handle_system_boot reason=0x%02X charger_exitst=%d", 2, reason, charger_exitst);

	// richard for clean shipping mode
	if(app_get_shipping_mode_state())
	{
		app_set_shipping_mode_state(false);
	}	
	
    if (charger_exitst) {
        /* Send CHARGER_IN_BOOT event if bit[2] is 1 (STS_CHRIN). */
        app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_CHARGER_IN_BOOT, NULL, 0);
    } else {
        /* Send POWER_KEY_BOOT event if bit[0] is 1 (STS_PWRKEY) or other. */
        app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_POWER_KEY_BOOT, NULL, 0);
        /* Play "power_on" VP and foreground led pattern if device power on in out_of_case state. */
// #if !(defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE))
        voice_prompt_play_vp_power_on();
// #endif /* !(AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE) */
        apps_config_set_foreground_led_pattern(LED_INDEX_POWER_ON, 30, FALSE);
    }
    return APP_SMCHARGER_OK;
}

static void app_smcharger_send_key_action(uint16_t action)
{
    uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    if (key_action != NULL) {
        *key_action = action;
        ui_shell_status_t ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                                    EVENT_GROUP_UI_SHELL_KEY,
                                                    INVALID_KEY_EVENT_ID,
                                                    key_action, sizeof(uint16_t),
                                                    NULL, 0);
        APPS_LOG_MSGID_I(LOG_TAG" send_key_action %d ret=%d", 2, action, ret);
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" send_key_action malloc fail", 0);
    }
}

/**
* @brief      SmartCharger APP enable BT discoverable mode via SmartCharger Case Key
*/
static void app_smcharger_bt_enter_discoverable()
{
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    APPS_LOG_MSGID_I(LOG_TAG" [%02X] bt_enter_discoverable", 1, role);
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        app_smcharger_send_key_action(KEY_DISCOVERABLE);
    }
}

/**
* @brief      This function is used to clear and enter discoverable.
* @param[in]  step, SmartCharger BT clear step.
*/
void app_smcharger_bt_clear_enter_discoverable(app_smcharger_clear_bt_step_t step)
{
    if (g_smcharger_context == NULL || !g_smcharger_context->bt_clear_ongoing) {
        return;
    }

    bt_cm_power_state_t power_state = bt_cm_power_get_state();
    APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR power_state=%d step=%d", 2, power_state, step);

    if (step == APP_SMCHARGER_REQUEST_BT_OFF) {
        if (power_state == BT_CM_POWER_STATE_OFF) {
            step = APP_SMCHARGER_REQUEST_BT_CLEAR_ALL;
        } else if (power_state == BT_CM_POWER_STATE_OFF_PENDING) {
            step = APP_SMCHARGER_REQUEST_NONE;
        }
    }

    if (step == APP_SMCHARGER_REQUEST_NONE) {
        //APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR - no action", 0);
    } else if (step == APP_SMCHARGER_REQUEST_BT_OFF) {
        app_smcharger_switch_bt(FALSE, TRUE);
        //APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR - OFF action", 0);
    } else if (step == APP_SMCHARGER_REQUEST_BT_CLEAR_ALL) {
        bt_status_t status = bt_device_manager_unpair_all();
        APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR - unpair_all=%d", 1, status);
        if (status == BT_STATUS_SUCCESS) {
#ifdef APP_CONN_MGR_RECONNECT_CONTROL
            nvkey_delete_data_item(NVID_APP_BT_RECONNECT_INFO);
#endif
            app_smcharger_bt_clear_enter_discoverable(APP_SMCHARGER_REQUEST_BT_ON);
        } else {
            //APPS_LOG_MSGID_E(LOG_TAG" BT_CLEAR error", 0);
            g_smcharger_context->bt_clear_ongoing = FALSE;
        }
    } else if (step == APP_SMCHARGER_REQUEST_BT_ON) {
        //APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR - ON action", 0);
        app_smcharger_switch_bt(TRUE, FALSE);
    } else if (step == APP_SMCHARGER_REQUEST_BT_DISCOVERABLE) {
        //APPS_LOG_MSGID_I(LOG_TAG" BT_CLEAR - discoverable action", 0);
        app_smcharger_bt_enter_discoverable();
        g_smcharger_context->bt_clear_ongoing = FALSE;
    }
}

/**
* @brief      SmartCharger APP enable BT air pairing mode via SmartCharger Case Key.
*/
static void app_smcharger_bt_air_pairing()
{
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_stop_advertising(FALSE);
#endif
    APPS_LOG_MSGID_I(LOG_TAG" [%02X] bt_air_pairing", 1, role);
    app_smcharger_send_key_action(KEY_AIR_PAIRING);
}

/**
* @brief      SmartCharger APP power on/off BT.
* @param[in]  on, TRUE - power on BT, FALSE - power off BT.
* @param[in]  direct_off, TRUE - power off whole BT, no delay.
* @return     If return APP_SMCHARGER_OK, run successfully.
*/
static app_smcharger_action_status_t app_smcharger_switch_bt(bool on, bool direct_off)
{
    bool need_rho = TRUE;
    if (g_smcharger_context != NULL
        && g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_CLOSE
        && g_smcharger_context->peer_smcharger_state == STATE_SMCHARGER_LID_CLOSE) {
        need_rho = FALSE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" switch_bt on=%d need_rho=%d", 2, on, need_rho);

    uint32_t off_classic_delay_ms = APP_SMCHARGER_DELAY_OFF_BT_CLASSIC_TIMER;
    uint32_t delay_ms = 3 * 1000;
    /* Cancel pop-up window for SP XiaoAI/GFP APP. */
    if (!on) {
#if defined(AIR_XIAOAI_ENABLE)
        delay_ms = 5 * 1000;
#elif defined(AIR_BT_FAST_PAIR_ENABLE)
        delay_ms = 10 * 1000;
#endif
    }

    ui_shell_status_t ret = UI_SHELL_STATUS_OK;
    if (BT_POWER_ON_NORMAL == bt_power_on_get_config_type()
        || BT_POWER_ON_DUT == bt_power_on_get_config_type()) {
#ifdef AIR_TILE_ENABLE
        /* If tile is activated, earbuds should enter classic bt mode */
        if (on) {
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                      (void *)on, 0, NULL, delay_ms);
        } else {
            if (!app_tile_tmd_is_active()) {
                ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                          EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                          APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                          (void *)on, 0, NULL, delay_ms);
            } else {
                ret = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                          APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF, NULL, 0, NULL, off_classic_delay_ms);
                /* Notify fast pair to stop ble adv after 3 sec */
                ret = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                          APPS_EVENTS_INTERACTION_STOP_FAST_PAIR_ADV, NULL, 0, NULL, 3000);
            }
        }
#else
        /* Only send event to HomeScreen APP, no need to care BT current state. */
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF);
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT);
        if (on) {
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                      (void *)on, 0, NULL, 0);
        } else if (direct_off) {
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_OFF_BT_WO_RHO,
                                      (void *)on, 0, NULL, 0);
        } else {
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF,
                                      (void *)need_rho, 0, NULL, off_classic_delay_ms);
            ret += ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                       (void *)FALSE, 0, NULL, delay_ms);
        }
#endif
    }
    return (ret == UI_SHELL_STATUS_OK ? APP_SMCHARGER_OK : APP_SMCHARGER_FAILURE);
}

/**
* @brief      SmartCharger APP mute/unmute audio, include BT Audio and VP.
* @param[in]  is_mute, TRUE - mute audio, FALSE - unmute audio.
* @return     If return APP_SMCHARGER_OK, run successfully.
*/
static app_smcharger_action_status_t app_smcharger_mute_audio(bool is_mute)
{
    APPS_LOG_MSGID_E(LOG_TAG" mute_audio, mute=%d (sidetone)", 1, is_mute);
    bt_status_t status = bt_sink_srv_music_set_mute(is_mute);
#ifdef AIR_LE_AUDIO_ENABLE
    bt_sink_srv_le_volume_set_mute_ex(BT_SINK_SRV_LE_STREAM_TYPE_OUT, is_mute, true);
#endif
#ifdef AIR_PROMPT_SOUND_ENABLE
    prompt_control_set_mute(is_mute);
#endif
    apps_config_audio_helper_sidetone_temporary_disable(is_mute);
    return (status == BT_STATUS_SUCCESS ? APP_SMCHARGER_OK : APP_SMCHARGER_FAILURE);
}

// richard for customer UI spec
static uint8_t mute_state = 0xff;
void app_mute_audio(bool is_mute)
{
	if(mute_state != is_mute)
	{	
		mute_state = is_mute;
		app_smcharger_mute_audio(is_mute);
	}
}

/**
* @brief      SmartCharger APP send public_event with action.
* @param[in]  old_state, old SmartCharger APP state.
* @param[in]  state, new SmartCharger APP state.
*/
static void app_smcharger_notify_state_to_app(int old_state, int state)
{
    uint32_t action = 0xFF;
    if (old_state == STATE_SMCHARGER_OUT_OF_CASE && state == STATE_SMCHARGER_LID_OPEN) {
        action = SMCHARGER_CHARGER_IN_ACTION;
    } else if (old_state == STATE_SMCHARGER_LID_CLOSE && state == STATE_SMCHARGER_OFF) {
        if (g_smcharger_context != NULL && g_smcharger_context->battery_percent == 100) {
            action = SMCHARGER_CHARGER_COMPLETE_ACTION;
        }
    } else if (state == STATE_SMCHARGER_LID_OPEN) {
        /* Two case: 1. LID_CLOSE switch to LID_OPEN. */
        /* 2. STARTUP switch to LID_OPEN (power_off device then charger in, device reboot in the lid_open case, open lid after device power_off in lid_close case). */
        action = SMCHARGER_OPEN_LID_ACTION;
    } else if (old_state == STATE_SMCHARGER_STARTUP && state == STATE_SMCHARGER_LID_CLOSE) {
        /* STARTUP->LID_CLOSE for "device boot slowly then close lid quickly". */
        action = SMCHARGER_CLOSE_LID_ACTION;
    } else if (old_state == STATE_SMCHARGER_LID_OPEN && state == STATE_SMCHARGER_LID_CLOSE) {
        action = SMCHARGER_CLOSE_LID_ACTION;
    } else if (state == STATE_SMCHARGER_OUT_OF_CASE) {
        action = SMCHARGER_CHARGER_OUT_ACTION;
    }
    if (action != 0xFF) {
        app_smcharger_ui_shell_public_event(FALSE, action, 0);
    }
}

app_smcharger_action_status_t app_smcharger_power_off(bool normal_off)
{
    /* Enter app_smcharger_power_off with following conditions
       1. app_smcharger_idle_activity.c detect battery state is SMCHARGER_BATTERY_STATE_SHUTDOWN
       2. app_smcharger_out_of_case_activity.c detect battery state is SMCHARGER_BATTERY_STATE_SHUTDOWN
       3. app_smcharger_utils.c detect smart charger state is STATE_SMCHARGER_OFF
     */
    APPS_LOG_MSGID_I(LOG_TAG" power_off %d", 1, normal_off);
    if (normal_off) {
        /* LOW_VOLTAGE power off outside of SmartCharger case, play "power off" VP and foreground led pattern. */
        voice_prompt_play_vp_power_off(VOICE_PROMPT_CONTROL_POWEROFF);
        apps_config_set_foreground_led_pattern(LED_INDEX_POWER_OFF, 30, FALSE);
    }
#ifdef AIR_TILE_ENABLE
    ui_shell_status_t ret = UI_SHELL_STATUS_OK;
    if (normal_off) {
        /* for normall_off is low battery state, tile should stop ble adv and enter power off mode */
        ret = ui_shell_send_event(FALSE,
                                  EVENT_PRIORITY_HIGHEST,
                                  EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                  (normal_off ? APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF
                                   : APPS_EVENTS_INTERACTION_REQUEST_IMMEDIATELY_POWER_OFF),
                                  NULL, 0, NULL, 0);
    } else {
        /* smart charger state is STATE_SMCHARGER_OFF, it should be classic bt off state */
        const app_bt_state_service_status_t *bt_state_srv_status = app_bt_connection_service_get_current_status();
        if (bt_state_srv_status != NULL) {
            APPS_LOG_MSGID_I(LOG_TAG" current power state %d", 1, bt_state_srv_status->current_power_state);
        }
    }
#else
    ui_shell_status_t ret = ui_shell_send_event(FALSE,
                                                EVENT_PRIORITY_HIGHEST,
                                                EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                (normal_off ? APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF
                                                 : APPS_EVENTS_INTERACTION_REQUEST_IMMEDIATELY_POWER_OFF),
                                                NULL, 0, NULL, 0);
#endif
    return (ret == UI_SHELL_STATUS_OK ? APP_SMCHARGER_OK : APP_SMCHARGER_FAILURE);
}

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
/**
* @brief      need_power_saving when SmartCharger APP enable, two cases:
*             1. Single device (AWS no connected) + out_of_case state
*             2. Double device (AWS connected) + Agent + both out_of_case state
* @return     If return APP_POWER_SAVING_TARGET_MODE_BT_OFF, need to power_saving via power off device,
*             otherwise no need to power_saving.
*/
app_power_saving_target_mode_t app_smcharger_get_power_saving_target_mode(void)
{
    app_power_saving_target_mode_t target_mode = APP_POWER_SAVING_TARGET_MODE_SYSTEM_OFF;
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    bool need_power_saving = FALSE;
    if (g_smcharger_context != NULL) {
        if (aws_link_type == BT_AWS_MCE_SRV_LINK_NONE
            && g_smcharger_context->smcharger_state == STATE_SMCHARGER_OUT_OF_CASE) {
            need_power_saving = TRUE;
        } else if (aws_link_type != BT_AWS_MCE_SRV_LINK_NONE
                   && role == BT_AWS_MCE_ROLE_AGENT
                   && g_smcharger_context->smcharger_state == STATE_SMCHARGER_OUT_OF_CASE
                   && g_smcharger_context->peer_smcharger_state == STATE_SMCHARGER_OUT_OF_CASE) {
            need_power_saving = TRUE;
        }
        APPS_LOG_MSGID_I(LOG_TAG" [%02X] aws_link_type=%d need_power_saving=%d",
                         3, role, aws_link_type, need_power_saving);
    }

    if (!need_power_saving) {
        target_mode = APP_POWER_SAVING_TARGET_MODE_NORMAL;
    }
    //APPS_LOG_MSGID_I(LOG_TAG" [POWER_SAVING] target_mode=%d", 1, target_mode);
    return target_mode;
}
#endif

static uint8_t dut_test_flag = 0;	// richard for UI spec
app_smcharger_action_status_t app_smcharger_state_do_action(uint8_t state)
{
    if (g_smcharger_context == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" app_smcharger do_action fail, invalid context", 0);
        return APP_SMCHARGER_FAILURE;
    }
    uint8_t old_state = g_smcharger_context->smcharger_state;

    /* SmartCharger APP send IN_OUT_EVENT. */
    if (state == STATE_SMCHARGER_OUT_OF_CASE
        || (old_state != STATE_SMCHARGER_LID_CLOSE && state == STATE_SMCHARGER_LID_OPEN)) {
        app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_NOTIFY_BOTH_IN_OUT, NULL, 0);
    }

    g_smcharger_context->smcharger_state = state;
    /* Print state switch log for Auto-Test. */
    APPS_LOG_MSGID_I(LOG_TAG" app_smcharger %d -> %d action", 2, old_state, state);

    /* Sync smcharger_state to peer. */
    if (state == STATE_SMCHARGER_STARTUP
        || state == STATE_SMCHARGER_OUT_OF_CASE
        || state == STATE_SMCHARGER_LID_OPEN
        || state == STATE_SMCHARGER_LID_CLOSE
        || state == STATE_SMCHARGER_OFF) {
        app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_SYNC_STATE, NULL, 0);
    }

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
    /* Notify Power_saving APP mode changed, then determine "is_need_power_saving". */
    if (state == STATE_SMCHARGER_OUT_OF_CASE
        || state == STATE_SMCHARGER_LID_OPEN
        || state == STATE_SMCHARGER_LID_CLOSE) {
        app_power_save_utils_notify_mode_changed(FALSE, app_smcharger_get_power_saving_target_mode);
    }
#endif

    /* SmartCharger APP send public_event with action. */
    app_smcharger_notify_state_to_app(old_state, state);

    /* See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine. */
    app_smcharger_action_status_t status = APP_SMCHARGER_OK;
    switch (state) {
        case STATE_SMCHARGER_STARTUP: {
            status += app_smcharger_handle_system_boot();
            break;
        }
        case STATE_SMCHARGER_LID_CLOSE: {
            status += app_smcharger_mute_audio(TRUE);
            status += app_smcharger_switch_bt(FALSE, FALSE);
            break;
        }
        case STATE_SMCHARGER_LID_OPEN: {
            if (old_state == STATE_SMCHARGER_STARTUP) {
                if (BT_POWER_ON_DUT == bt_power_on_get_config_type()) {
                    APPS_LOG_MSGID_I(LOG_TAG" BT_POWER_ON_DUT, No switch bt on", 0);
                } else {
                    status += app_smcharger_switch_bt(TRUE, FALSE);
                }
            } else if (old_state == STATE_SMCHARGER_LID_CLOSE) {
                status += app_smcharger_switch_bt(TRUE, FALSE);
            }
            status += app_smcharger_mute_audio(TRUE);
#ifdef MTK_ANC_ENABLE
            /* Suspend ANC when device in the SmartCharger case. */
#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
            app_anc_service_suspend();
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
            //APPS_LOG_MSGID_I(LOG_TAG" audio_anc_suspend ret=%d", 1, anc_ret);
#endif
		// richard for customer UI spec.
		if(dut_test_flag)
		{//if in dut test will reboot and exit dut
			dut_test_flag = 0;
			app_smcharger_send_key_action(KEY_SYSTEM_REBOOT);
		}

            break;
        }
        case STATE_SMCHARGER_OUT_OF_CASE: {
            status += app_smcharger_switch_bt(TRUE, FALSE);
            status += app_smcharger_mute_audio(FALSE);
#ifdef MTK_ANC_ENABLE
            /* Resume ANC when device outside of the SmartCharger case. */
#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
            app_anc_service_resume();
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
            //APPS_LOG_MSGID_I(LOG_TAG" audio_anc_resume ret=%d", 1, anc_ret);
#endif
            break;
        }
        case STATE_SMCHARGER_OFF: {
			// richard for need enter shipping mode
			if(app_get_shipping_mode_state())
			{
				app_enter_shipping_mode_flag_set(true);
			}
			app_common_add_tracking_log(0x32);
            status += app_smcharger_power_off(FALSE);
            break;
        }
        default: {
            configASSERT(0);
            break;
        }
    }
    if (status != APP_SMCHARGER_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" app_smcharger do_action fail %d", 1, status);
    }
    return status;
}

/**
* @brief      This function is init SmartCharger APP.
*/
void app_smcharger_init()
{
    /* Register callback to SmartCharger middleware. */
    DrvCharger_RegisterSmartCase(app_smcharger_driver_callback);
#ifdef AIR_SMART_CHARGER_TEST
    /* Register AT CMD handler for SmartCharger Test. */
    atci_register_handler(smcharger_atci_cmd, sizeof(smcharger_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    //APPS_LOG_MSGID_I(LOG_TAG" app_smcharger_init atci ret=%d", 1, ret);
#endif

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
    /* Register callback for power_saving. */
    app_power_save_utils_register_get_mode_callback(app_smcharger_get_power_saving_target_mode);
#endif
}

void app_smcharger_set_context(app_smcharger_context_t *smcharger_ctx)
{
    g_smcharger_context = smcharger_ctx;
}

app_smcharger_battery_state_t app_smcharger_get_battery_state()
{
    return (g_smcharger_context != NULL ? g_smcharger_context->battery_state : SMCHARGER_BATTERY_STATE_IDLE);
}

bool app_smcharger_show_led_bg_pattern()
{
    app_smcharger_battery_state_t battery_state = (g_smcharger_context != NULL ?
                                                   g_smcharger_context->battery_state : SMCHARGER_BATTERY_STATE_IDLE);
    bool ret = FALSE;
    /* Update background led pattern according to app_battery_state. */
    switch (battery_state) {
        case SMCHARGER_BATTERY_STATE_IDLE:
        case SMCHARGER_BATTERY_STATE_FULL:
            ret = FALSE;
            break;
        case SMCHARGER_BATTERY_STATE_CHARGING:
            apps_config_set_background_led_pattern(LED_INDEX_CHARGING, FALSE, APPS_CONFIG_LED_AWS_SYNC_PRIO_HIGH);
            ret = TRUE;
            break;
        case SMCHARGER_BATTERY_STATE_CHARGING_FULL:
            apps_config_set_background_led_pattern(LED_INDEX_IDLE, FALSE, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
            ret = TRUE;
            break;
        case SMCHARGER_BATTERY_STATE_LOW_CAP:
        case SMCHARGER_BATTERY_STATE_SHUTDOWN:
            apps_config_set_background_led_pattern(LED_INDEX_LOW_BATTERY, FALSE, APPS_CONFIG_LED_AWS_SYNC_PRIO_HIGH);
            ret = TRUE;
            break;
        case SMCHARGER_BATTERY_STATE_THR:
            apps_config_set_background_led_pattern(LED_INDEX_CHARGING_ERROR, FALSE, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOW);
            ret = TRUE;
            break;
    }

    APPS_LOG_MSGID_I(LOG_TAG" show_led_bg_pattern, battery_state=%d", 1, battery_state);
    return ret;
}

void app_smcharger_handle_case_battery(uint8_t case_battery)
{
    APPS_LOG_MSGID_I(LOG_TAG" handle_case_battery, case_battery=%d", 1, case_battery);
    if (case_battery <= 100
        || (case_battery >= 128 && case_battery <= 228)) {
        /* Use SmartCharger Case battery_percent, such as send it via BLE ADV (XiaoAI MIUI Quick_Connect, Fast Pairing). */
        app_smcharger_ui_shell_public_event(FALSE, SMCHARGER_CASE_BATTERY_REPORT_ACTION, case_battery);
    }
}

void app_smcharger_handle_key_event(uint32_t key_value)
{
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    APPS_LOG_MSGID_I(LOG_TAG" handle_key_event key=%d", 1, key_value);
    if (g_smcharger_context != NULL && key_value <= 255
        && (g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_OPEN
            || g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_CLOSE)) {
        // Default Key function must be both LID_OPEN
        APPS_LOG_MSGID_I(LOG_TAG" handle_key_event state=%d peer=%d aws_link_type=%d", 3,
                         g_smcharger_context->smcharger_state,
                         g_smcharger_context->peer_smcharger_state,
                         aws_link_type);
        if (g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_OPEN
            && key_value == APP_SMCHARGER_KEY_BT_AIR_PAIRING) {
            app_smcharger_bt_air_pairing();
        } else if (g_smcharger_context->smcharger_state == STATE_SMCHARGER_LID_OPEN
/* richard for UI spec                   && g_smcharger_context->peer_smcharger_state == STATE_SMCHARGER_LID_OPEN
                   && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE*/) {
            if (key_value == APP_SMCHARGER_KEY_BT_DISCOVERABLE) {
                app_smcharger_bt_enter_discoverable();
            } else if (key_value == APP_SMCHARGER_KEY_BT_CLEAR) {
                g_smcharger_context->bt_clear_ongoing = TRUE;
                app_smcharger_bt_clear_enter_discoverable(APP_SMCHARGER_REQUEST_BT_OFF);
            }
#if 1	// richard for UI spe.
            else if((key_value == APP_SMCHARGER_KEY_BT_SIGNAL_DISCOVERABLE)/*&&(aws_link_type == BT_AWS_MCE_SRV_LINK_NONE)*/) {
//				app_smcharger_send_key_action(KEY_SIGNAL_DISCOVERABLE);
				app_smcharger_send_key_action(KEY_DISCOVERABLE);
            } else if (key_value == APP_SMCHARGER_KEY_BT_DUT_TEST) {
			if((APP_DISCONNECTED == apps_config_key_get_mmi_state()	|| APP_CONNECTABLE == apps_config_key_get_mmi_state())&& dut_test_flag == 0)
			{
            			app_smcharger_send_key_action(KEY_ENABLE_DUT_TEST);
				dut_test_flag = 1;
			}
            } else if (key_value == APP_SMCHARGER_KEY_TWS_CLEAN) {
			app_smcharger_send_key_action(KEY_TEST_TWS_CLEAN);
            } else if (key_value == APP_SMCHARGER_KEY_FACTORY_RESET) {
			app_smcharger_send_key_action(KEY_TEST_FACTORY_RESET);
            } else if (key_value == APP_SMCHARGER_KEY_ULL_AIR_PAIRING) {
			app_smcharger_send_key_action(KEY_ULL_AIR_PAIRING);
            }
#endif		
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" handle_key_event - No both LID_OPEN/AWS", 0);
        }

        app_smcharger_ui_shell_public_event(FALSE, SMCHARGER_CHARGER_KEY_ACTION, key_value);
    }
}

void app_smcharger_handle_user_data(int state, int event_id, uint8_t user_data)
{
    APPS_LOG_MSGID_I(LOG_TAG" handle_user_data state=%d event_id=%d user_data=0x%02X",
                     3, state, event_id, user_data);

    if ((state == STATE_SMCHARGER_LID_OPEN || state == STATE_SMCHARGER_LID_CLOSE)
        && event_id == SMCHARGER_EVENT_USER_DATA) {
        /* Use USER_DATA according to Case State & user_data. */
        app_smcharger_ui_shell_public_event(FALSE, SMCHARGER_USER_DATA_ACTION, user_data);
    }
}

void app_smcharger_update_case_battery_nvkey(uint8_t case_battery)
{
    uint32_t size = sizeof(uint8_t);
    nvkey_status_t status = nvkey_write_data(NVID_APP_CASE_BATTERY_INFO,
                                             (const uint8_t *)&case_battery, size);
    APPS_LOG_MSGID_I(LOG_TAG" update_case_battery_nvkey, case_battery=%d status=%d",
                     2, case_battery, status);

    // Notify other APP when Agent smcharger_case_battery changed
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        APPS_LOG_MSGID_I(LOG_TAG"harrydbg app_smcharger_update_case_battery_nvkey case_battery=%d,status=%d",2,case_battery , status);
        app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_NOTIFY_BOTH_CHANGED, NULL, 0);
    }
}

/*
 * @brief   SmartCharger APP UT Code by using AT CMD
 * AT+SMC=pwr_key_boot/chg_in_boot/chg_in/chg_out/chg_off/lid_close/lid_open
 */
#ifdef AIR_SMART_CHARGER_TEST

static void smcharger_atci_get_context_str(uint8_t *response_buf)
{
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    memset(response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
    snprintf((char *)response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
             "[%02X] battery_state=%d own=%d[peer=%d] battery=%d[peer=%d] aws_link_type=%d\r\n",
             bt_device_manager_aws_local_info_get_role(),
             g_smcharger_context->battery_state,
             g_smcharger_context->smcharger_state,
             g_smcharger_context->peer_smcharger_state,
             (int)g_smcharger_context->battery_percent,
             (int)g_smcharger_context->peer_battery_percent,
             aws_link_type);
}

static atci_status_t smcharger_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    bool show_status = FALSE;
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            uint8_t data = 0;
            memcpy(cmd, atcmd, strlen(atcmd) - 2);
            if (strstr(cmd, "pwr_key_boot") != NULL) {
                app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_POWER_KEY_BOOT, NULL, 0);
            } else if (strstr(cmd, "chg_in_boot") != NULL) {
                app_smcharger_ui_shell_event(FALSE, SMCHARGER_EVENT_CHARGER_IN_BOOT, NULL, 0);
            } else if (strstr(cmd, "chg_in") != NULL) {
                app_smcharger_driver_callback(SMCHG_CHG_IN, FALSE, 0, 0);
            } else if (strstr(cmd, "chg_out") != NULL) {
                app_smcharger_driver_callback(SMCHG_CHG_OUT, FALSE, 0, 0);
            } else if (strstr(cmd, "chg_off") != NULL) {
                data = 90;    /* Case Battery. */
                app_smcharger_driver_callback(SMCHG_CHG_OFF, FALSE, data, 1);
            } else if (strstr(cmd, "key") != NULL) {
                data = 0;     /* Key value. */
                int x = 0;
                sscanf(cmd, "key,%d", &x);
                data = (uint8_t)x;
                app_smcharger_driver_callback(SMCHG_CHG_KEY, FALSE, data, 1);
            } else if (strstr(cmd, "lid_close") != NULL) {
                app_smcharger_driver_callback(SMCHG_LID_CLOSE, FALSE, 0, 0);
            } else if (strstr(cmd, "lid_open") != NULL) {
                data = 80;    /* Case Battery. */
                app_smcharger_driver_callback(SMCHG_LID_OPEN, FALSE, data, 1);
            } else if (strstr(cmd, "user_data") != NULL) {
                data = 0x11;
                app_smcharger_driver_callback(SMCHG_USER_DATA, FALSE, data, 1);
            } else if (strstr(cmd, "status") != NULL) {
                smcharger_atci_get_context_str(response.response_buf);
                show_status = TRUE;
            } else if (strstr(cmd, "case_bat") != NULL) {
                int x = 0;
                sscanf(cmd, "case_bat,%d", &x);
                app_smcharger_handle_case_battery_report(FALSE, x);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" invalid SMCharger AT-CMD", 0);
            }
            if (!show_status) {
                memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s\r\n", atcmd);
            }
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif



/**================================================================================*/
/**                           PUBLIC API for other APP                             */
/**================================================================================*/
// richard for customer UI spec
app_smcharger_context_t* app_get_smcharger_context(void)
{
	return g_smcharger_context;
}

uint8_t app_smcharger_get_state1(void)
{
	if(g_smcharger_context == NULL)
		return ((app_bt_service_is_air_pairing()<<4)|app_bt_service_is_visible());

	return (((g_smcharger_context->smcharger_state)<<5)|(app_bt_service_is_air_pairing()<<4)|((g_smcharger_context->peer_smcharger_state)<<1)|app_bt_service_is_visible());
}

uint8_t app_smcharger_get_earbud_state(void)
{
	bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();

	if(g_smcharger_context == NULL)
		return aws_link_type;
	
	return ((g_smcharger_context->smcharger_state)<<5) |((g_smcharger_context->peer_smcharger_state)<<2) |aws_link_type;
}

void app_smcharger_update_bat()
{
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    if (role == BT_AWS_MCE_ROLE_AGENT && g_smcharger_context != NULL) {
        unsigned char partner_battery_percent = g_smcharger_context->peer_battery_percent;
        unsigned char left_bat = 0;
        unsigned char right_bat = 0;
        audio_channel_t channel = ami_get_audio_channel();
        if (AUDIO_CHANNEL_L == channel) {
            left_bat = g_smcharger_context->battery_percent;
            right_bat = partner_battery_percent;
        } else if (AUDIO_CHANNEL_R == channel) {
            right_bat = g_smcharger_context->battery_percent;
            left_bat = partner_battery_percent;
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" [XIAOAI_VA] invalid L/R Channel", 0);
        }

        APPS_LOG_MSGID_I(LOG_TAG" [XIAOAI_VA] left=%d right=%d case=%d",
                         3, left_bat, right_bat, g_smcharger_context->case_battery_percent);
#ifdef AIR_XIAOAI_ENABLE
        /* Call XiaoAI API to set battery. */
        xiaoai_set_battery(g_smcharger_context->case_battery_percent,
                           left_bat,
                           right_bat);
#endif
    }
}

app_smcharger_in_out_t app_smcharger_is_charging()
{
    app_smcharger_in_out_t result = APP_SMCHARGER_NONE;
    if (g_smcharger_context != NULL) {
        uint8_t state = g_smcharger_context->smcharger_state;
        if (state == STATE_SMCHARGER_LID_OPEN
            || state == STATE_SMCHARGER_LID_CLOSE
            || state == STATE_SMCHARGER_OFF) {
            result = APP_SMCHARGER_IN;
        } else if (state == STATE_SMCHARGER_OUT_OF_CASE) {
            result = APP_SMCHARGER_OUT;
        }
    }
    return result;
}

app_smcharger_in_out_t app_smcharger_peer_is_charging()
{
    app_smcharger_in_out_t result = APP_SMCHARGER_NONE;
    if (g_smcharger_context != NULL) {
        uint8_t state = g_smcharger_context->peer_smcharger_state;
        if (state == STATE_SMCHARGER_LID_OPEN
            || state == STATE_SMCHARGER_LID_CLOSE
            || state == STATE_SMCHARGER_OFF) {
            result = APP_SMCHARGER_IN;
        } else if (state == STATE_SMCHARGER_OUT_OF_CASE) {
            result = APP_SMCHARGER_OUT;
        }
    }
    return result;
}

app_smcharger_state app_smcharger_get_state()
{
    app_smcharger_state state = STATE_SMCHARGER_NONE;
    if (g_smcharger_context != NULL) {
        state = g_smcharger_context->smcharger_state;
    }
    return state;
}

void app_smcharger_get_battery_percent(uint8_t *own_battery, uint8_t *peer_battery, uint8_t *case_battery)
{
    if (own_battery != NULL) {
        *own_battery = g_smcharger_context->battery_percent;
    }
    if (peer_battery != NULL) {
        *peer_battery = g_smcharger_context->peer_battery_percent;
    }
    if (case_battery != NULL) {
        *case_battery = g_smcharger_context->case_battery_percent;
    }
}

uint8_t app_smcharger_read_case_battery_nvkey()
{
    uint8_t case_battery = 0;
    uint32_t size = sizeof(uint8_t);
    nvkey_status_t status = nvkey_read_data(NVID_APP_CASE_BATTERY_INFO, (uint8_t *)&case_battery, &size);

    if (status == NVKEY_STATUS_ITEM_NOT_FOUND) {
        size = sizeof(uint8_t);
        status = nvkey_write_data(NVID_APP_CASE_BATTERY_INFO, (const uint8_t *)&case_battery, size);
    }

    APPS_LOG_MSGID_I(LOG_TAG" read_case_battery_nvkey, battery=%d status=%d",
                     2, case_battery, status);
    return case_battery;
}

#endif
