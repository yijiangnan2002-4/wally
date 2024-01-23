/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
  * File: app_bolt_poc_idle_activity.c
  *
  * Description: This file is the idle activity for bolt poc.
  *
  * Note: todo.
  *
  */

#include "app_bolt_poc_idle_activity.h"
#include "app_bolt_poc_data.h"
#include "apps_config_event_list.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"


//#define KEY_TRIGGER_HOGP_TEST


#ifdef KEY_TRIGGER_HOGP_TEST
#include "timers.h"

#define TEST_ITEM_COUNT (4)

static uint32_t g_time_count = 0;

static uint8_t g_test_input_buf[TEST_ITEM_COUNT][HOGP_REPORT_DATA_LENGTH] = {
    {0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
    {0x00, 0x00, 0xE0, 0x0F, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00}
};

static void app_bolt_poc_timer_func(TimerHandle_t xTime)
{
    const uint8_t *dest = (const uint8_t *)(&g_test_input_buf[g_time_count % TEST_ITEM_COUNT]);

    bt_hogp_client_para_t para;
    para.report_id = BT_HOGP_MOUSE_INPUT_REPORT;
    app_bolt_poc_event_input_report_ind(&para, dest, HOGP_REPORT_DATA_LENGTH);

    g_time_count++;
}

static TimerHandle_t g_bolt_timer_handle = NULL;
static bool app_bolt_poc_idle_proc_key_event_group(ui_shell_activity_t* self,
    uint32_t event_id,
    const void* extra_data,
    size_t data_len)
{
    bool ret = false;

    APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_idle_proc_key_event_group: ID(0x%X)", 1, event_id);

    #define KEY11 (0x4081)

    switch (event_id) {
        case KEY11: {
            if (g_bolt_timer_handle == NULL) {
                g_bolt_timer_handle = xTimerCreate("bolt_timer", 250 * portTICK_PERIOD_MS, pdTRUE, NULL, app_bolt_poc_timer_func);
                if (g_bolt_timer_handle != NULL) {
                    APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_idle_proc_key_event_group: timer start", 0);
                    xTimerStart(g_bolt_timer_handle, 0);
                }
            } else {
                APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_idle_proc_key_event_group: timer stop", 0);
                xTimerStop(g_bolt_timer_handle, 0);
                xTimerDelete(g_bolt_timer_handle, 0);
                g_bolt_timer_handle = NULL;
                g_time_count = 0;
            }

            ret = true;
            break;
        }
        default:
            break;
    }

    return ret;
}
#endif

static bool app_bolt_poc_idle_proc_ui_shell_group(ui_shell_activity_t* self,
    uint32_t event_id,
    const void* extra_data,
    size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
            break;
        default:
            break;
    }

    return ret;
}

bool app_bolt_poc_idle_activity_proc(ui_shell_activity_t* self,
    uint32_t event_group,
    uint32_t event_id,
    void* extra_data,
    size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_bolt_poc_idle_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
#ifdef KEY_TRIGGER_HOGP_TEST
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = app_bolt_poc_idle_proc_key_event_group(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }

    return ret;
}

