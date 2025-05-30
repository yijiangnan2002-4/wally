/* Copyright Statement:
 *
 * (C) 2005-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "hal_nvic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "audio_codec.h"
#include <stdint.h>
#include <string.h>

#include "task_def.h"
#include "audio_log.h"

QueueHandle_t audio_codec_queue_handle = NULL;
TaskHandle_t audio_codec_task_handle = NULL;
SemaphoreHandle_t audio_codec_semaphore_handle = NULL;
audio_codec_internal_callback_t audio_codec_queue_cb_array[AUDIO_CODEC_MAX_FUNCTIONS];
audio_codec_media_handle_t *audio_codec_queue_handle_array[AUDIO_CODEC_MAX_FUNCTIONS];

static void audio_codec_task_main(void *arg)
{
    audio_codec_queue_event_t event;

    while (1) {
        if (xQueueReceive(audio_codec_queue_handle, &event, portMAX_DELAY)) {
            // log_hal_info("uxQueueSpacesAvailable %d \n", uxQueueSpacesAvailable(audio_codec_queue_handle));
            audio_codec_media_handle_t *handle = event.handle;
            uint32_t id_idx;
            for (id_idx = 0; id_idx < AUDIO_CODEC_MAX_FUNCTIONS; id_idx++) {
                if (audio_codec_queue_handle_array[id_idx] == handle) {
                    audio_codec_mutex_lock();
                    audio_codec_queue_cb_array[id_idx](handle, event.parameter);
                    audio_codec_mutex_unlock();
                    break;
                }
            }
            // AUD_LOG_I("[TEST] stack size max [%d]", 1, uxTaskGetStackHighWaterMark(audio_codec_task_handle));
        }
    }
}


audio_codec_status_t audio_codec_event_register_callback(audio_codec_media_handle_t *handle, audio_codec_internal_callback_t callback)
{
    {   /* temp */
        if (audio_codec_queue_handle == NULL) {
            audio_codec_queue_handle = xQueueCreate(AUDIO_CODEC_QUEUE_LENGTH, sizeof(audio_codec_queue_event_t));
            {   /* Initialize queue registration */
                uint32_t id_idx;
                for (id_idx = 0; id_idx < AUDIO_CODEC_MAX_FUNCTIONS; id_idx++) {
                    audio_codec_queue_handle_array[id_idx] = NULL;
                }
            }

            xTaskCreate(audio_codec_task_main,
                        AUDIO_CODEC_TASK_NAME, 512 * 5 / sizeof(StackType_t), NULL, TASK_PRIORITY_SOFT_REALTIME, &audio_codec_task_handle);
        }
    }
    uint32_t id_idx;
    audio_codec_status_t status = AUDIO_CODEC_RETURN_ERROR;
    for (id_idx = 0; id_idx < AUDIO_CODEC_MAX_FUNCTIONS; id_idx++) {
        if (audio_codec_queue_handle_array[id_idx] == NULL) {
            audio_codec_queue_handle_array[id_idx] = handle;
            audio_codec_queue_cb_array[id_idx] = callback;
            status = AUDIO_CODEC_RETURN_OK;
            break;
        }
    }
    return status;
}

audio_codec_status_t audio_codec_event_deregister_callback(audio_codec_media_handle_t *handle)
{
    uint32_t id_idx;
    audio_codec_status_t status = AUDIO_CODEC_RETURN_ERROR;
    for (id_idx = 0; id_idx < AUDIO_CODEC_MAX_FUNCTIONS; id_idx++) {
        if (audio_codec_queue_handle_array[id_idx] == handle) {
            audio_codec_queue_handle_array[id_idx] = NULL;
            status = AUDIO_CODEC_RETURN_OK;
            break;
        }
    }

    {   /* temp */
        // bool needDelete = true;
        // for (id_idx = 0; id_idx < AUDIO_CODEC_MAX_FUNCTIONS; id_idx++) {
        //     if (audio_codec_queue_handle_array[id_idx] != NULL) {
        //         needDelete = false;
        //         break;
        //     }
        // }
        // if ( needDelete && audio_codec_task_handle != NULL) {
        //     vTaskDelete(audio_codec_task_handle);
        //     audio_codec_task_handle = NULL;
        // }

        // if ( needDelete && audio_codec_queue_handle != NULL) {
        //     vQueueDelete(audio_codec_queue_handle);
        //     audio_codec_queue_handle = NULL;
        // }
    }
    return status;
}

audio_codec_status_t audio_codec_event_send_from_isr(audio_codec_media_handle_t *handle, void *parameter)
{
    // log_hal_info("audio_codec_event_send_from_isr + \n");
    audio_codec_status_t status = AUDIO_CODEC_RETURN_OK;
    audio_codec_queue_event_t event;
    event.handle    = handle;
    event.parameter = parameter;

    if (handle != NULL) {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) { // task level
            if (xQueueSend(audio_codec_queue_handle, &event, 0) != pdTRUE) {
                AUD_LOG_I("[Audio Codec] Send queue error. Queue full (Drop: 0x%x)\r\n", 1, event.handle);
            }
            status = AUDIO_CODEC_RETURN_ERROR;
        } else { // interrupt level
            if (pdFALSE == xQueueIsQueueFullFromISR(audio_codec_queue_handle)) {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xQueueSendFromISR(audio_codec_queue_handle, &event, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                status = AUDIO_CODEC_RETURN_OK;
            } else {
                AUD_LOG_I("[Audio Codec] Send queue error. Queue full (Drop: 0x%x)\r\n", 1, event.handle);
                status = AUDIO_CODEC_RETURN_ERROR;
            }
        }
    } else {
        // AUD_LOG_W("[Audio Codec] Warning: handle NULL\r\n", 0);
        status = AUDIO_CODEC_RETURN_ERROR;
    }
    return status;
}

/* global semaphore to protect play/stop/close/pause/resume flow */
audio_codec_status_t audio_codec_mutex_create(void)
{
    audio_codec_status_t status = AUDIO_CODEC_RETURN_OK;

    if (audio_codec_semaphore_handle == NULL) {
        audio_codec_semaphore_handle = xSemaphoreCreateMutex();  /*Old FreeRTOS version work.*/
        // vSemaphoreCreateBinary(audio_codec_semaphore_handle);      /*In New FreeRTOS version It would assert if Mutex take & give are different task.*/
    } else {
        status = AUDIO_CODEC_RETURN_ERROR;
    }

    return status;
}

audio_codec_status_t audio_codec_mutex_delete(void)
{
    audio_codec_status_t status = AUDIO_CODEC_RETURN_OK;

    if (audio_codec_semaphore_handle != NULL) {
        vSemaphoreDelete(audio_codec_semaphore_handle);
        audio_codec_semaphore_handle = NULL;
    } else {
        status = AUDIO_CODEC_RETURN_ERROR;
    }

    return status;
}

audio_codec_status_t audio_codec_mutex_lock(void)
{
    audio_codec_status_t status = AUDIO_CODEC_RETURN_OK;
    if (audio_codec_semaphore_handle == NULL) {
        audio_codec_semaphore_handle = xSemaphoreCreateMutex();
    }
    if (audio_codec_semaphore_handle != NULL) {
        // AUD_LOG_I("[Audio Codec] audio_codec_mutex_lock() +\r\n", 0);
        xSemaphoreTake(audio_codec_semaphore_handle, portMAX_DELAY);
        // AUD_LOG_I("[Audio Codec] audio_codec_mutex_lock() +\r\n", 0);
    } else {
        status = AUDIO_CODEC_RETURN_ERROR;
    }

    return status;
}

audio_codec_status_t audio_codec_mutex_unlock(void)
{
    audio_codec_status_t status = AUDIO_CODEC_RETURN_OK;

    if (audio_codec_semaphore_handle != NULL) {
        xSemaphoreGive(audio_codec_semaphore_handle);
        // AUD_LOG_I("[Audio Codec] audio_codec_mutex_unlock()\r\n", 0);
    } else {
        status = AUDIO_CODEC_RETURN_ERROR;
    }

    return status;
}
