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
 * File: app_preproc_activity.c
 *
 * Description: This activity is used to pre-process events before other activities.
 *
 * Note: See doc/AB158X_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#include "app_preproc_activity.h"
#include "apps_config_led_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "bt_app_common.h"
#include "apps_debug.h"
#include "apps_config_key_remapper.h"
#include "voice_prompt_api.h"
#include "apps_events_key_event.h"
#include "app_preproc_sys_pwr.h"
#include "app_bt_state_service.h"
#include "apps_events_test_event.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "apps_race_cmd_event.h"
#endif
#include "nvkey.h"
#include "nvkey_id_list.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#endif
#include "apps_config_audio_helper.h"
#include "apps_config_vp_index_list.h"
#include "bt_device_manager.h"
#include "apps_customer_config.h"
#include "apps_control_touch_key_status.h"
#ifdef AIR_RACE_SCRIPT_ENABLE
#include "race_cmd_script.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
#include "apps_race_cmd_co_sys_event.h"
#endif
#if defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#include "mux_ll_uart_latch.h"
#endif
#ifdef AIR_TILE_ENABLE
#include "app_bt_state_service.h"
#endif
#ifdef MTK_BLE_BAS
//errrrrrrrrrrrrrrrrr
#include "ble_bas_app.h"
#endif
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#if (defined AIRO_KEY_EVENT_ENABLE) && !(defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
#include "hal_keypad_table.h"
static const uint8_t captouch_keys[] = APPS_CAPTOUCH_KEY_IDS;
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "app_dongle_service.h"
#endif

#include "apps_config_event_list.h"
#include "audio_anc_psap_control.h"
#include "app_hear_through_activity.h"


bool key_porc_in_ear_statu; 
static bool _proc_ui_shell_group(ui_shell_activity_t *self,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    bool ret = true;

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
            APPS_LOG_MSGID_I("app_preproc_activity create", 0);
            apps_config_audio_helper_init();
            break;
        default:
            break;
    }
    return ret;
}

// richard for customer UI spec
static uint8_t touch_test_press_times = 0;							 
uint8_t app_touch_key_test_read(void)
{
	return touch_test_press_times;
}

void app_touch_key_test_clean(void)
{
	touch_test_press_times = 0;
}

#ifdef AIRO_KEY_EVENT_ENABLE
static bool pre_proc_key_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
//uint8_t ret=false;
    if (extra_data) {
#ifdef AIR_RACE_SCRIPT_ENABLE
        race_script_key_notify();
#endif
#if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
        uint8_t key_id;
        airo_key_event_t key_event;
        /* The extra_data in the key event is valid key_action. */
        app_event_key_event_decode(&key_id, &key_event, event_id);
#endif

#if defined(AIR_DCHS_MODE_SLAVE_ENABLE)
        if (dchs_get_device_mode() != DCHS_MODE_SINGLE) {
            if (key_id == DEVICE_KEY_POWER) {
                /* DCHS master and slave receive duplicate power key event. */
                return true;
            } else if (INVALID_KEY_EVENT_ID != event_id) {
                app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_KEY, event_id,
                                               extra_data, data_len, false);
                return true;
            } else {
                return false;
            }
        }
#endif
#if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE))
        bool is_captouch = false;

#ifdef AIR_RACE_SCRIPT_ENABLE
        race_script_key_notify();
#endif

        /* The pointer is from p_key_action in apps_events_key_event.c, content is same as s_press_from_power_on. */
        uint16_t *p_key_action = (uint16_t *)extra_data;
        if (INVALID_KEY_EVENT_ID == event_id) {
            /* Key event from CMD, not real key. */
            APPS_LOG_MSGID_I("Receive CMD key event, action: %04x", 1, *p_key_action);
            return false;
        }
        /* The key is from power on, ignore it. */
        if (*p_key_action) {
            APPS_LOG_MSGID_I("The key pressed from power on, do special %04x", 1, event_id);
            return true;
        }

        for (uint8_t i = 0; i < sizeof(captouch_keys); i++) {
            if (captouch_keys[i] == key_id) {
                is_captouch = true;
                break;
            }
        }

        if (key_event == AIRO_KEY_PRESS) {
#ifdef AIR_TILE_ENABLE
errrrrrrrrrrrrrrrr
            const app_bt_state_service_status_t *bt_state_srv_status = app_bt_connection_service_get_current_status();
            if (bt_state_srv_status != NULL && bt_state_srv_status->current_power_state == APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED) {
                APPS_LOG_MSGID_I("AIRO_KEY_PRESS, current_power_state %d", 1, bt_state_srv_status->current_power_state);
            } else
#endif
            {
#ifndef AIR_WIRELESS_MIC_ENABLE
//errrrrrrrrrrrrrrrrr
                APPS_LOG_MSGID_I("pre_proc_key_event_proc AIRO_KEY_PRESS, vpvp press  key_id=%x, key_event=%x",2, key_id, key_event);
                #if 1
                voice_prompt_param_t vp = {0};
                vp.vp_index = VP_INDEX_PRESS;
                vp.control = VOICE_PROMPT_CONTROL_MASK_PREEMPT;
                voice_prompt_play(&vp, NULL);
                #endif
                //apps_config_set_vp_preempt(VP_INDEX_PRESS, false, 0, VOICE_PROMPT_PRIO_HIGH, NULL);
#endif
            }
        }

        /* Control captouch for bisto*/
#if defined(AIR_GSOUND_ENABLE) && defined(MTK_RACE_CMD_ENABLE)
        uint8_t temp_touch_key_status = 0;
        temp_touch_key_status = apps_get_touch_control_status();
        APPS_LOG_MSGID_I("App_pre_pro get touch_key_status=0x%02X, key_id=%x, key_event=%x", 3, temp_touch_key_status, key_id, key_event);
        if (((0 == temp_touch_key_status) || (0xFF == temp_touch_key_status))
            && is_captouch) {
            if ((AIRO_KEY_SHORT_CLICK == key_event) || (AIRO_KEY_DOUBLE_CLICK == key_event) || (AIRO_KEY_TRIPLE_CLICK == key_event)) {
            APPS_LOG_MSGID_I(APP_HFP_IDLE"  VP_INDEX_FAILED lll", 0);
                voice_prompt_play_vp_failed();
            }
            return true;
        }
#endif
        if (is_captouch) {
            /* For QA testing, use power key table to implement . */
            key_id = DEVICE_KEY_POWER;
        }
        if(0)//(key_porc_in_ear_statu==0)// harry for key proc
          {
          APPS_LOG_MSGID_I("pre_proc_key_event_proc, it is outear action", 0);
          }
        else
          {
          //APPS_LOG_MSGID_I("pre_proc_key_event_proc, it is inear action", 0);
        *p_key_action = apps_config_key_event_remapper_map_action(key_id, key_event);
          }

      APPS_LOG_MSGID_I("pre_proc_key_event_proc, action: %04x,key_id=%x,key_event=%x", 3, *p_key_action,key_id,key_event);
     if(*p_key_action==KEY_SWITCH_ANC_AND_PASSTHROUGH)
     {
     #if 0
      uint8_t level_max_count = 0;
      uint8_t mode_max_count = 0;
      uint8_t vol_max_count = 0;
      uint8_t mode_index = 0;

      audio_anc_psap_control_get_mode_index(&mode_index);
      audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);
      APPS_LOG_MSGID_I("pre_proc_key_event_proc  current mode index:%d, mode max:%d,app_hear_through_ctx.mode_index=%d,vol_max_count=%d",
                     4,
                     mode_index,
                     mode_max_count,
                     app_hear_through_ctx.mode_index,
                     vol_max_count
                     );
        if(app_hear_through_ctx.mode_index==APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH)//现在在hear
        {
          if(mode_index!=(mode_max_count-1)) // 现在不是最大mode
          {
            *p_key_action=KEY_HEARING_AID_MODE_UP_CIRCULAR;
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_HEARING_AID_MODE_UP_CIRCULAR", 0);
          }
          else
          {
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_SWITCH_ANC_AND_PASSTHROUGH11", 0);
          }
          
        }
        else
       #endif
        {
          APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_SWITCH_ANC_AND_PASSTHROUGH22", 0);
        }
      }
     else if(*p_key_action==KEY_AVRCP_PAUSE)
      {
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_AVRCP_PAUSE", 0);
      }else if(*p_key_action==KEY_AVRCP_PLAY)
      {
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_AVRCP_PLAY", 0);
      } 
     else if(*p_key_action==KEY_VOICE_DN)
      {
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_VOICE_DN", 0);
            voice_prompt_play_sync_vp_volume_down();
      }else if(*p_key_action==KEY_VOICE_UP)
      {
            voice_prompt_play_sync_vp_volume_up();
            APPS_LOG_MSGID_I("pre_proc_key_event_proc KEY_VOICE_UP", 0);
      }
#endif /* #if !(defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)) */
    }
    return false;
}
#endif

#ifdef AIR_ROTARY_ENCODER_ENABLE
static bool s_rotary_1_control_mix = false;
static bool pre_proc_rotary_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    if (extra_data) {
        bsp_rotary_encoder_port_t port;
        bsp_rotary_encoder_event_t event;
        uint32_t rotary_data;
        uint16_t *p_key_action = (uint16_t *)extra_data;

        app_event_rotary_event_decode(&port, &event, &rotary_data, event_id);
        if (BSP_ROTARY_ENCODER_0 == port) {
            /* ENCODER_0 is for volume change.*/
            if (BSP_ROTARY_ENCODER_EVENT_CW == event) {
                *p_key_action = KEY_VOICE_UP;
            } else {
                *p_key_action = KEY_VOICE_DN;
            }
        } else if (BSP_ROTARY_ENCODER_1 == port && (s_rotary_1_control_mix
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                                                    || app_dongle_service_get_dongle_mode() == APP_DONGLE_SERVICE_DONGLE_MODE_XBOX
#endif
                                                   )) {
            /* ENCODER_0 is for mix ra:tio and side tone. */
            if (BSP_ROTARY_ENCODER_EVENT_CW == event) {
                *p_key_action = KEY_AUDIO_MIX_RATIO_CHAT_ADD;
            } else {
                *p_key_action = KEY_AUDIO_MIX_RATIO_GAME_ADD;
            }
        } else if (BSP_ROTARY_ENCODER_1 == port) {
            if (BSP_ROTARY_ENCODER_EVENT_CW == event) {
                *p_key_action = KEY_AUDIO_SIDE_TONE_VOLUME_UP;
            } else {
                *p_key_action = KEY_AUDIO_SIDE_TONE_VOLUME_DOWN;
            }
        } else {
            *p_key_action = KEY_ACTION_INVALID;
        }
    } else {
        ret = true;
    }
    return ret;
}
#endif

static bool pre_proc_app_interaction_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        /* Increase BLE ADV interval and restart advertise, see bt_app_common_ble_adv_timer_hdlr in bt_app_common.c. */
        case APPS_EVENTS_INTERACTION_INCREASE_BLE_ADV_INTERVAL:
            bt_app_common_trigger_increase_ble_adv();
            ret = true;
            break;
        /* Reload key_remaper when NVKEY changed, see bt_race_reload_nvkey_event_callback in apps_events_bt_event.c. */
        case APPS_EVENTS_INTERACTION_RELOAD_KEY_ACTION_FROM_NVKEY:
            apps_config_key_remaper_init_configurable_table();
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
            apps_config_key_remapper_dual_chip_sync_key_table();
#endif
            ret = true;
            break;
#ifdef AIR_TILE_ENABLE /* Test CMD with tile features. */
        case APPS_EVENTS_INTERACTION_TEST_BLE_ADV: {
            bool on_off = (bool)extra_data;
            apps_events_test_event_ble_adv_start(on_off);
            ret = true;

            break;
        }
#endif
        /*
        case APPS_EVENTS_INTERACTION_PLAY_VP: {
            apps_config_vp_t *vp_extra = (apps_config_vp_t *)extra_data;
            if (vp_extra->type == VP_TYPE_VP) {
                apps_config_set_vp(vp_extra->index, vp_extra->need_sync, vp_extra->delay_time, vp_extra->level, false, NULL);
            } else if (vp_extra->type == VP_TYPE_RT) {
                apps_config_set_voice(vp_extra->index, vp_extra->need_sync, vp_extra->delay_time,
                        vp_extra->level, true, vp_extra->need_repeat, NULL);
            }
            ret = true;
            break;
        }
        */
        case APPS_EVENTS_INTERACTION_SET_LED: {
            typedef struct {
                uint8_t type;
                uint8_t index;
                uint16_t timeout;
            } __attribute__((packed)) LED_DATA;
            LED_DATA *led_data = (LED_DATA *)(&extra_data);
            APPS_LOG_MSGID_I("app_preproc APPS_EVENTS_INTERACTION_SET_LED type %d, index %d, timeout %d", 3,
                             led_data->type, led_data->index, led_data->timeout);
            if (led_data->type == 0x01) {
                /* Foreground. */
                apps_config_set_foreground_led_pattern(led_data->index, led_data->timeout, false);
            } else if (led_data->type == 0x02) {
                /* Background. */
                apps_config_set_background_led_pattern(led_data->index, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_HIGHEST);
            }
            ret = true;
            break;
        }
#if (defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)) && defined(AIR_SILENCE_DETECTION_ENABLE)
        case APPS_EVENTS_INTERACTION_SILENCE_DETECT_CHANGE: {
            bool is_silence = app_events_audio_event_get_silence_detect_flag();
            app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_DUAL_CHIP_CMD, APPS_RACE_CMD_CO_SYS_DUAL_CHIP_EVENT_SILENCE_DETECT, &is_silence, sizeof(is_silence), false);
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_SIMULATE_TIMER: {
            if (extra_data) {
                typedef void (*simulate_timer_callback_t)(void);
                simulate_timer_callback_t timer_callback = (simulate_timer_callback_t)extra_data;
                timer_callback();
            }
            ret = true;
        }
#ifdef MTK_IN_EAR_FEATURE_ENABLE   // harry for key proc
        case APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT: {
            if (extra_data) {
                bool *in_ear = (bool *)extra_data;
                key_porc_in_ear_statu = *in_ear;
             APPS_LOG_MSGID_I("app_preproc APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT inear= %d", 1,key_porc_in_ear_statu);
                if(key_porc_in_ear_statu)
                  {
                  //bt_sink_srv_set_mute(BT_SINK_SRV_MUTE_SPEAKER, FALSE);
			bt_sink_srv_music_set_mute(FALSE);		  
                  }
                else
                  {
                  //bt_sink_srv_set_mute(BT_SINK_SRV_MUTE_SPEAKER, TRUE);
			bt_sink_srv_music_set_mute(TRUE);		  
                  }
           }
            break;
        }
#endif

        
        default:
            break;
    }

    return ret;
}


static bool pre_proc_led_manager_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        /* Check and disable foreground LED when timeout, see _led_fg_time_out_callback in apps_config_led_manager.c. */
        case APPS_EVENTS_LED_FG_PATTERN_TIMEOUT:
            apps_config_check_foreground_led_pattern();
            ret = true;
            break;
#ifdef MTK_AWS_MCE_ENABLE
        /* Sync LED pattern info from Agent or Partner, then handle in UI Shell task, see app_led_sync_callback in apps_config_led_manager.c. */
        case APPS_EVENTS_LED_SYNC_LED_PATTERN:
            app_config_led_sync(extra_data);
            ret = true;
            break;
#endif
        default:
            break;
    }

    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_pre_proc_aws_data(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action,
                                         &p_extra_data, &extra_data_len);

        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION) {
            if (action == APPS_EVENTS_INTERACTION_SYNC_TOUCH_KEY_STATUS) {
                if (BT_AWS_MCE_ROLE_PARTNER == role) {
                    uint8_t touch_key_status = *(uint8_t *)p_extra_data;
                    nvkey_status_t status = 0;
                    if (NVKEY_STATUS_OK != (status = nvkey_write_data(NVID_APP_RACE_TOUCH_KEY_ENABLE,
                                                                      (uint8_t *)&touch_key_status, sizeof(uint8_t)))) {
                        return ret;
                    } else {
                        ret = true;
                    }
                    APPS_LOG_MSGID_I("App_pre_pro partner receive touch_key_status=%x, nvdm_status: %d", 2, touch_key_status, status);
                }
            } else if (action == APPS_EVENTS_INTERACTION_PLAY_VP) {
                /*
                apps_config_vp_t *vp_extra = (apps_config_vp_t *)p_extra_data;
                APPS_LOG_MSGID_I("app_preproc receive set vp, index=%d sync=%d delay=%d level=%d", 4,
                        vp_extra->index, vp_extra->need_sync, vp_extra->delay_time, vp_extra->level);
                if (vp_extra->type == VP_TYPE_VP) {
                    apps_config_set_vp(vp_extra->index, vp_extra->need_sync, vp_extra->delay_time, vp_extra->level, false, NULL);
                } else if (vp_extra->type == VP_TYPE_RT) {
                    apps_config_set_voice(vp_extra->index, vp_extra->need_sync, vp_extra->delay_time,
                            vp_extra->level, true, vp_extra->need_repeat, NULL);
                }
                */
                ret = true;
            }
        }
    }
    return ret;
}
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
static bool app_pre_proc_dual_chip_cmd(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case APPS_RACE_CMD_CO_SYS_DUAL_CHIP_EVENT_SYNC_KEY_TABLE: {
            apps_config_configurable_table_t *table = (apps_config_configurable_table_t *)extra_data;
            apps_config_key_remapper_set_configurable_key_table(table, data_len, false);
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

bool app_preproc_activity_proc(ui_shell_activity_t *self,
                               uint32_t event_group,
                               uint32_t event_id,
                               void *extra_data,
                               size_t data_len)
{
    bool ret = false;

    /* APPS_LOG_MSGID_I("pre-proc receive event_group=%d event_id=0x%08x", 2, event_group, event_id); */

    switch (event_group) {
        /* UI Shell internal events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        /* UI Shell APP interaction events. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = pre_proc_app_interaction_event_proc(self, event_id, extra_data, data_len);
            break;
#ifdef AIRO_KEY_EVENT_ENABLE
        /* UI Shell key events. */
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = pre_proc_key_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
#ifdef AIR_ROTARY_ENCODER_ENABLE
        /* UI Shell rotary encoder events. */
        case EVENT_GROUP_UI_SHELL_ROTARY_ENCODER: {
            ret = pre_proc_rotary_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#endif
        /* UI Shell BT Sink events, see bt_app_common.c. */
        case EVENT_GROUP_UI_SHELL_BT_SINK:
            ret = bt_app_common_sink_event_proc(event_id, extra_data, data_len);
            break;
        /* UI Shell BT CM events, see bt_app_common.c. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            ret = bt_app_common_cm_event_proc(event_id, extra_data, data_len);
            break;
        /* UI Shell LED manager events. */
        case EVENT_GROUP_UI_SHELL_LED_MANAGER:
            ret = pre_proc_led_manager_event_proc(self, event_id, extra_data, data_len);
            break;
        /* UI Shell system_power events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM_POWER:
            ret = sys_pwr_component_event_proc(self, event_id, extra_data, data_len);
            break;
#if (defined(MTK_AWS_MCE_ENABLE))
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            /* Handle the touch key state sync from the agent side. */
            ret = app_pre_proc_aws_data(self, event_id, extra_data, data_len);
            break;
        }
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
        case EVENT_GROUP_UI_SHELL_DUAL_CHIP_CMD:
            /* Handle the dual chip cmd event. */
            app_pre_proc_dual_chip_cmd(self, event_id, extra_data, data_len);
            break;
#endif



        default:
            break;
    }
    if (!ret) {
        /* Handle again if ret is not TRUE, see app_bt_state_service.c. */
        ret = app_bt_state_service_process_events(event_group, event_id, extra_data, data_len);
        if (!ret) {
            ret = apps_config_audio_helper_proc(event_group, event_id, extra_data, data_len);
        }
    }

#ifdef MTK_BLE_BAS
    ble_bas_app_handle_event(event_group, event_id, extra_data, data_len);
#endif
#ifdef MTK_ANC_ENABLE
    app_anc_service_handle_event(event_group, event_id, extra_data, data_len);
#endif

    return ret;
}
