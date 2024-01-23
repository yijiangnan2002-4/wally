/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "hal.h"
#include "hal_platform.h"
#include "hal_gpt.h"
#include "bsp_head_tracker_imu_cywee.h"
#include "bsp_head_tracker_imu_common.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "task_def.h"
#include "queue.h"
#include <string.h>

/******************************************************************
 *            Macro & Typedef
 ******************************************************************/
enum {
    HEAD_TRACKER_Q_SEND = 0,
    HEAD_TRACKER_Q_RECV = 1
};

#define HEAD_TRACKER_SENSOR_TIMEOUT_TIME       (20) //20ms


/******************************************************************
 *             Private Variable Declare
 ******************************************************************/

static bool             s_is_enabled = false;
static head_track_lib_service_handle_func_t s_lib_event_handle;
static uint32_t         s_timer_period = HEAD_TRACKER_SENSOR_TIMEOUT_TIME;
static QueueHandle_t    s_os_q_handle = NULL;
static uint32_t         s_gpt_handle = 0;
static volatile bool    s_initialized = false;

/******************************************************************
 *             Private Function Implement
 ******************************************************************/
static void   head_track_lib_event_handle_task(void *pargs);
static void   headtrack_gpt_callback(void *user_data);


static int headtrack_q_control(QueueHandle_t handle, int cmd, int option)
{
    BaseType_t xStatus;
    uint32_t mode;

    mode = HAL_NVIC_QUERY_EXCEPTION_NUMBER;
    switch(cmd){
        case HEAD_TRACKER_Q_SEND: {
            bsp_head_tracker_log_d("[bsp][headtracker][comm] headtrack_q_control: send %d to Q", 1, option);
            if(mode == 0) {
                xStatus = xQueueSend(handle, &option, 0xFFFF);
            } else {
                xStatus = xQueueSendFromISR(handle, &option, pdFALSE);
            }
            if (xStatus != pdPASS) {
                bsp_head_tracker_log_e("[bsp][headtracker][comm] send Q timeout, status %d", 1, xStatus);
                return -1;
            }
        } break;

        case HEAD_TRACKER_Q_RECV: {
            if(mode == 0) {
                xStatus = xQueueReceive(handle, &option, 0xFFFF);
            } else {
                xStatus = xQueueReceiveFromISR(handle, &option, NULL);
            }
            if (xStatus != pdPASS) {
                bsp_head_tracker_log_i("[bsp][headtracker][comm] recv Q timeout, status %d", 1, xStatus);
                return -1;
            }
            bsp_head_tracker_log_d("[bsp][headtracker][comm] headtrack_q_control: recv %d from Q", 1, option);
        } break;
    }
    return option;
}



static void head_track_lib_event_handle_task(void *pargs)
{
    int service_id;

    while(1) {
        service_id = headtrack_q_control(s_os_q_handle, HEAD_TRACKER_Q_RECV, 0);
        if(service_id < 0) {
            continue;
        }
        bsp_head_tracker_log_d("[bsp][headtracker][comm] service lib task: service id %d", 1, service_id);

        if (s_lib_event_handle != NULL) {
            s_lib_event_handle(service_id);
        }
        if (service_id == HEAD_TRACKER_SERVICE_ENABLE) {
            hal_gpt_sw_start_timer_ms(s_gpt_handle, s_timer_period, headtrack_gpt_callback, NULL);
        }
    }
}

static void headtrack_gpt_callback(void *user_data)
{
    int event = HEAD_TRACKER_SERVICE_PROCESS;
    bsp_head_tracker_log_d("[bsp][headtracker][comm] headtrack_gpt_callback: triggered", 0);
    if (true == s_is_enabled) {
        headtrack_q_control(s_os_q_handle, HEAD_TRACKER_Q_SEND, event);
        hal_gpt_sw_start_timer_ms(s_gpt_handle, s_timer_period, headtrack_gpt_callback, NULL);
    }
}


/******************************************************************
 *             Public Function Implement
 ******************************************************************/

void head_track_lib_service_init(head_track_lib_service_handle_func_t func)
{
    s_lib_event_handle = func;

    if(s_initialized == false) {
        s_os_q_handle = xQueueCreate(10, 1);
        if(s_os_q_handle == NULL) {
            bsp_head_tracker_log_e("[bsp][headtracker][comm] init service err, create Q fail", 0);
            return;
        }
        xTaskCreate(head_track_lib_event_handle_task, "hdtk", 4096 / sizeof(StackType_t), (void *)0, TASK_PRIORITY_ABOVE_NORMAL, NULL);
        hal_gpt_sw_get_timer(&s_gpt_handle);
        s_initialized = true;
    }
}

void head_track_lib_service(head_tracker_imu_service_type_t type, uint32_t option)
{
    int event = type;

    bsp_head_tracker_log_d("[bsp][headtracker][comm] head_track_lib_service: start sevice %d", 1, type);
    switch (type) {
        case HEAD_TRACKER_SERVICE_ENABLE: {
            s_is_enabled = true;
        } break;
        case HEAD_TRACKER_SERVICE_DISABLE: {
            s_is_enabled = false;
        }
        break;
        case HEAD_TRACKER_SERVICE_CHGTM: {
            s_timer_period = option;
            bsp_head_tracker_log_w("[bsp][headtracker][comm] head_track_lib_service: change period to %dms", 1, option);
        }
        break;
        case HEAD_TRACKER_SERVICE_STOP: {
            hal_gpt_sw_stop_timer_ms(s_gpt_handle);
            s_is_enabled = false;
        }
        break;
        default:
            break;
    }
    headtrack_q_control(s_os_q_handle, HEAD_TRACKER_Q_SEND, event);
}

void head_track_lib_service_deinit()
{
}