/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_RACE_CO_SYS_ENABLE

#include "apps_race_cmd_co_sys_event.h"

#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"
#include "FreeRTOS.h"
#include "verno.h"
#if defined(AIR_DCHS_MODE_ENABLE)
#include "mux_ll_uart_latch.h"
#endif
#ifdef AIRO_KEY_EVENT_ENABLE
#include "hal_keypad_table.h"
#include "apps_events_key_event.h"
#endif

#include "apps_events_battery_event.h"
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#include "apps_detachable_mic.h"
#endif



#define LOG_TAG     "[APP_RACE_DUAL_EVENT]"

void app_race_cmd_co_sys_send_event(uint32_t event_group, uint32_t event_id, void *extra_data, uint32_t extra_data_len, bool is_critical)
{
    uint32_t total_len = sizeof(apps_race_cmd_co_sys_common_cmd_buf_t) + extra_data_len;
    apps_race_cmd_co_sys_common_cmd_buf_t *send_buf = (apps_race_cmd_co_sys_common_cmd_buf_t *)pvPortMalloc(total_len);
    if (send_buf) {
        send_buf->group_id = event_group;
        send_buf->event_id = event_id;
        send_buf->data_len = extra_data ? extra_data_len : 0;
        if (extra_data_len > 0 && extra_data) {
            memcpy(send_buf->data, extra_data, extra_data_len);
        }
        race_cosys_send_data(RACE_COSYS_MODULE_ID_APP_COMMON, is_critical, (uint8_t *)send_buf, total_len);
        vPortFree(send_buf);
    }
}

static void app_race_cmd_co_sys_data_received_callback(bool is_critical, uint8_t *buff, uint32_t len)
{
    apps_race_cmd_co_sys_common_cmd_buf_t *cmd_content = (apps_race_cmd_co_sys_common_cmd_buf_t *)buff;
    void *extra_data = NULL;
    bool process_done = false;  /* Whe it's false, means send ui shell event to other app to process. */
    if (!cmd_content || cmd_content->data_len + sizeof(apps_race_cmd_co_sys_common_cmd_buf_t) != len) {
        APPS_LOG_MSGID_E(LOG_TAG", receive co sys data not correct: buff:0x%x", 1, buff);
        return;
    }
#if defined(AIR_DCHS_MODE_ENABLE)
    if (dchs_get_device_mode() == DCHS_MODE_SINGLE) {
        return;
    }
#endif

    switch (cmd_content->group_id) {
        case EVENT_GROUP_UI_SHELL_DUAL_CHIP_CMD: {
            switch (cmd_content->event_id) {
                case APPS_RACE_CMD_CO_SYS_DUAL_CHIP_EVENT_GET_TIMESTAMP: {
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
                    app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_DUAL_CHIP_CMD, APPS_RACE_CMD_CO_SYS_DUAL_CHIP_EVENT_GET_TIMESTAMP,
                                                   build_date_time_str, strlen(build_date_time_str) + 1, __bool_true_false_are_defined);

                    //report percent of battery to master
                    int32_t local_battery = apps_events_battery_get_current_battery();
                    app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_DCHS_NOTIFY_BATTERY_PERCENT,
                                                   &local_battery, sizeof(int32_t), false);
                    //report shutdown state to master
                    int32_t temp_state = apps_events_battery_get_shutdown_state();
                    app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_DCHS_NOTIFY_SHUTDOWN_STATE,
                                                   &temp_state, sizeof(int32_t), false);
                    APPS_LOG_MSGID_I(LOG_TAG"[DCHS_battery] slave report to master local_battery:%d,shutdown_state:%d ", 2, local_battery, temp_state);
#endif



#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
                    if (cmd_content->data_len > 0) {
                        APPS_LOG_I(LOG_TAG", timestamp. master:%s, slave:%s", build_date_time_str, cmd_content->data);
                    }
#endif
                    process_done = true;
                    break;
                }
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
                case APPS_RACE_CMD_CO_SYS_DUAL_CHIP_EVENT_SYNC_DETACHABLE_MIC_STATUS: {
                    APPS_LOG_MSGID_I(LOG_TAG"[DETACHABLE_MIC] receive mic state=%d ", 1, cmd_content->data);
                    voice_mic_type_t mic_type = *(voice_mic_type_t*)cmd_content->data;
                    detachable_mic_set_mic_type(mic_type);
                    process_done = true;
                    break;
                }
#endif
                default:
                    break;
            }
            break;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
            case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
                switch (cmd_content->event_id) {
                    case APPS_EVENTS_INTERACTION_DCHS_NOTIFY_BATTERY_PERCENT: {
                        if (cmd_content->data) {
                            int32_t temp_battery = *((int32_t *)cmd_content->data);
//                    memcpy(&temp_battery, cmd_content->data, sizeof(int32_t));
                            apps_events_battery_update_dual_chip_another_side_battery(temp_battery);
                        }

                    }
                    break;
                    case APPS_EVENTS_INTERACTION_DCHS_NOTIFY_SHUTDOWN_STATE: {
                        if (cmd_content->data) {
                            int32_t temp_state = *((int32_t *)cmd_content->data);
                            //judge slave battery state to shutdown
                            apps_events_shutdown_by_slave_battery(temp_state);
                        }

                    }
                    break;



                }

            }
            break;
#endif


        }
        default:
            break;
    }

    /* Send ui event. */
    if (!process_done) {
        APPS_LOG_MSGID_I(LOG_TAG", receive co_sys APP data from remote: group:0x%x, event_id: 0x%x, extra_data_len: %d", 3, cmd_content->group_id, cmd_content->event_id, cmd_content->data_len);
        if (cmd_content->data_len > 0) {
            /* Some event use value as a pointer directly. */
            bool directly_use_data = false;

            if (cmd_content->group_id == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                && (cmd_content->event_id == APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF
                    || cmd_content->event_id == APPS_EVENTS_INTERACTION_REQUEST_REBOOT
                    || cmd_content->event_id == APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT
                    || cmd_content->event_id == APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF)) {
                directly_use_data = true;
            }
#if defined(AIR_DCHS_MODE_MASTER_ENABLE) && defined (AIRO_KEY_EVENT_ENABLE)
            else if (cmd_content->group_id == EVENT_GROUP_UI_SHELL_KEY) {
                uint8_t key_id;
                airo_key_event_t key_event;

                /* The extra_data in the key event is valid key_action. */
                app_event_key_event_decode(&key_id, &key_event, cmd_content->event_id);
                if (EINT_KEY_0 <= key_id && EINT_KEY_2 >= key_id) {
                    key_id = EINT_KEY_4 - EINT_KEY_0 + key_id;
                    cmd_content->event_id = (key_id & 0xFF) | ((key_event & 0xFF) << 8);
                }
            }
#endif
            if (directly_use_data) {
                uint32_t data_value = 0;
                memcpy(&data_value, cmd_content->data, cmd_content->data_len);
                cmd_content->data_len = 0;
                extra_data = (void *)data_value;
            } else {
                extra_data = pvPortMalloc(cmd_content->data_len);
                if (extra_data) {
                    memcpy(extra_data, cmd_content->data, cmd_content->data_len);
                } else {
                    return;
                }
            }
        }
        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, cmd_content->group_id, cmd_content->event_id, extra_data, cmd_content->data_len, NULL, 0);
    }
}

void apps_race_cmd_co_sys_event_init(void)
{
    race_cosys_register_data_callback(RACE_COSYS_MODULE_ID_APP_COMMON, app_race_cmd_co_sys_data_received_callback);
}

#endif /* #ifdef MTK_RACE_CMD_ENABLE */
