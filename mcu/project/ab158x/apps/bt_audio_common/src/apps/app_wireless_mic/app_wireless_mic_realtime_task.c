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

#include "app_wireless_mic_realtime_task.h"
#include "app_wireless_mic_idle_activity.h"
#include "app_wireless_mic_fatfs.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_nvic.h"
#include "syslog.h"

static QueueHandle_t s_app_wireless_mic_realtime_queue = NULL;

log_create_module(WIRELESS_MIC_REALTIME, PRINT_LEVEL_INFO);

void app_wireless_mic_realtime_task(void *arg)
{
    app_wireless_mic_realtime_msg_t msg;

    LOG_MSGID_I(WIRELESS_MIC_REALTIME, "app_wireless_mic_realtime_task: task create", 0);

    s_app_wireless_mic_realtime_queue = xQueueCreate(20, sizeof(app_wireless_mic_realtime_msg_t));

    if (s_app_wireless_mic_realtime_queue == NULL) {
        LOG_MSGID_I(WIRELESS_MIC_REALTIME, "app_wireless_mic_realtime_task: app_vp_queue create fail", 0);
        return;
    }

    while (1) {
        /* Wait msg, blocking. */
        if (xQueueReceive((QueueHandle_t)s_app_wireless_mic_realtime_queue, &msg, portMAX_DELAY) == pdFALSE) {
            continue;
        }

        switch (msg.type) {
            case APP_WIRELESS_MIC_REALTIME_MSG_TYPE_RECORDER_DATA: {
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
                app_wireless_mic_idle_proc_audio_data();
#endif
                break;
            }
            case APP_WIRELESS_MIC_REALTIME_MSG_TYPE_START_RECORDER: {
                app_wireless_mic_start_local_recorder();
                break;
            }
            case APP_WIRELESS_MIC_REALTIME_MSG_TYPE_STOP_RECORDER: {
                app_wireless_mic_fatfs_close_wav_file();
                if (app_wireless_mic_get_ready_power_off_state()) {
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
                                        EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_WIRELESS_MIC_READY_TO_POWER_OFF,
                                        NULL, 0, NULL, 0);
                    app_wireless_mic_set_ready_power_off_state(false);
                }
                break;
            }
            default:
                break;
        }

    }
}


void app_wireless_mic_realtime_send_msg(app_wireless_mic_realtime_msg_type_t type, uint16_t id, void *data)
{
    app_wireless_mic_realtime_msg_t msg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    msg.type = type;
    msg.id = id;
    msg.data = data;

    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0) {
        ret = xQueueSendFromISR((QueueHandle_t)s_app_wireless_mic_realtime_queue, &msg, &xHigherPriorityTaskWoken);
    } else {
        ret = xQueueSend((QueueHandle_t)s_app_wireless_mic_realtime_queue, &msg, 0);
    }

    LOG_MSGID_I(WIRELESS_MIC_REALTIME, "wireless_mic_realtime_send_msg, type %d, ID %d, ret %d", 3, type, id, ret);

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
