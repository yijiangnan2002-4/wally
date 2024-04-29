/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
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

/* Common Internal Adapter API */
#include "atci_adapter.h"
/* Free RTOS */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "hal_nvic_internal.h"
#include "syslog.h"
#include "hal.h"


uint32_t atci_queue_create(uint32_t queue_length, uint32_t item_size)
{
    QueueHandle_t q_id = NULL;

    q_id = xQueueCreate(queue_length, item_size);
    return (uint32_t)q_id;
}

atci_status_t atci_queue_send(uint32_t q_id, void *data)
{
    BaseType_t ret = 0;
    BaseType_t xHigherPriorityTaskWoken;

    if (0 == q_id) {
        return ATCI_STATUS_ERROR;
    }

//  if( 0 == hal_nvic_query_exception_number())
    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        ret = xQueueSend((QueueHandle_t)q_id, data, 0);
    } else {
        ret = xQueueSendFromISR((QueueHandle_t)q_id, data, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (pdFAIL != ret) {
        return ATCI_STATUS_OK;
    } else {
        return ATCI_STATUS_ERROR;
    }
}


atci_status_t atci_queue_send_from_isr(uint32_t q_id, void *data)
{
    BaseType_t ret = 0;
    BaseType_t xHigherPriorityTaskWoken;

    ret = xQueueSendFromISR((QueueHandle_t)q_id, data, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if (pdPASS == ret) {
        return ATCI_STATUS_OK;
    } else {
        return ATCI_STATUS_ERROR;
    }
}


int32_t atci_queue_receive_no_wait(uint32_t q_id, void *data)
{
    BaseType_t ret = -1;
    if (0 == q_id) {
        return 0;
    }
    ret = xQueueReceive((QueueHandle_t)q_id, data, 0);
    return (int32_t)ret;
}

int32_t atci_queue_receive_wait(uint32_t q_id, void *data, uint32_t delay_time)
{
    BaseType_t ret = -1;
    ret = xQueueReceive((QueueHandle_t)q_id, data, delay_time / portTICK_PERIOD_MS);
    return (int32_t)ret;
}

uint16_t atci_queue_get_item_num(uint32_t q_id)
{
    uint16_t queue_item_num = 0;
    if (0 == q_id) {
        return 0;
    }
    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        queue_item_num = (uint16_t)uxQueueMessagesWaiting((QueueHandle_t) q_id);
    } else {
        queue_item_num = (uint16_t)uxQueueMessagesWaitingFromISR((QueueHandle_t) q_id);
    }
    //queue_item_num = (uint16_t)uxQueueMessagesWaiting((QueueHandle_t) q_id);
    return queue_item_num;
}

uint32_t atci_mutex_create(void)
{
    return (uint32_t)xSemaphoreCreateMutex();
}

uint32_t atci_mutex_lock(uint32_t mutex_id)
{
    if (mutex_id == 0) {
        return 0;
    }
    return (uint32_t)xSemaphoreTake((SemaphoreHandle_t)mutex_id, portMAX_DELAY);
}

uint32_t atci_mutex_unlock(uint32_t mutex_id)
{
    if (mutex_id == 0) {
        return 0;
    }

    return (uint32_t)xSemaphoreGive((SemaphoreHandle_t)mutex_id);
}

void *atci_mem_alloc(uint32_t size)
{
    void *pvReturn = NULL;
    uint32_t  free_size;
    free_size = xPortGetFreeHeapSize();
    if (free_size > size) {
        pvReturn = pvPortMalloc(size);
    }

    return pvReturn;
}

void atci_mem_free(void *buf)
{
    if (buf != NULL) {
        vPortFree(buf);
    }
}

uint32_t atci_semaphore_create(uint32_t uxMaxCount, uint32_t uxInitialCount)
{
    return (uint32_t)xSemaphoreCreateCounting((UBaseType_t)uxMaxCount, (UBaseType_t)uxInitialCount);
}

atci_status_t atci_semaphore_take(uint32_t semaphore_id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return ATCI_STATUS_OK;
    }

    if (NULL == (void *)semaphore_id) {
        return ATCI_STATUS_ERROR;
    }

    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        ret = (uint32_t)xSemaphoreTake((SemaphoreHandle_t)semaphore_id, portMAX_DELAY);
    } else {
        ret = xSemaphoreTakeFromISR((SemaphoreHandle_t)semaphore_id, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return (ret == pdTRUE) ? ATCI_STATUS_OK : ATCI_STATUS_ERROR;
}

atci_status_t atci_semaphore_give(uint32_t semaphore_id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return ATCI_STATUS_OK;
    }

    if (NULL == (void *)semaphore_id) {
        return ATCI_STATUS_ERROR;
    }

    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        ret = (uint32_t)xSemaphoreGive((SemaphoreHandle_t)semaphore_id);
    } else {
        ret = xSemaphoreGiveFromISR((SemaphoreHandle_t)semaphore_id, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return (ret == pdTRUE) ? ATCI_STATUS_OK : ATCI_STATUS_ERROR;
}



uint16_t atci_get_parameter_list(atci_parse_cmd_param_t *atcmd, uint8_t **list_out, uint16_t list_cnt)
{
    if (NULL == atcmd || NULL == list_out || 0 >= list_cnt) {
     log_hal_msgid_info("atci_cmd_hdlr_test return 1111\r\n ",0);
       return 0;
    }
    if (ATCI_CMD_MODE_EXECUTION != atcmd->mode || atcmd->parse_pos >= atcmd->string_len) {
     log_hal_msgid_info("atci_cmd_hdlr_test return 2222\r\n ",0);
        return 0;
    }
    uint8_t *para_str = (uint8_t *)&atcmd->string_ptr[atcmd->parse_pos];
    uint32_t rest_len = atcmd->string_len - atcmd->parse_pos;
    uint16_t param_cnt = 0;
    uint8_t *param = para_str;
    uint32_t i;
    bool parse_end = false;
    
    log_hal_msgid_info("atci_cmd_hdlr_test atcmd->string_len=%d,->parse_pos=%d,->name_len=%d\r\n ",3,atcmd->string_len,atcmd->parse_pos,atcmd->name_len);

    for (i = 0; i < rest_len; i++) {
    log_hal_msgid_info("atci_cmd_hdlr_test rest_len=%d,para_str[%d]=%d\r\n ",3,rest_len,i,para_str[i]);
        switch(para_str[i]) {
            case '\0':
            case '\r':
            case '\n': {
              if(para_str[i]=='\0') { log_hal_msgid_info("atci_cmd_hdlr_test str=xiegan zero,para_str[i]=%d\r\n ",1,para_str[i]);}
              else if(para_str[i]=='\r') { log_hal_msgid_info("atci_cmd_hdlr_test str=xiegan r,para_str[i]=%d\r\n ",1,para_str[i]);}
              else if(para_str[i]=='\n') { log_hal_msgid_info("atci_cmd_hdlr_test str=xiegan n,para_str[i]=%d\r\n ",1,para_str[i]);}
                parse_end = true;
                }
            case ',': {
               { log_hal_msgid_info("atci_cmd_hdlr_test str=douhao,para_str[%d]=%d\r\n ",2,i,para_str[i]);}
                para_str[i] = '\0';
                if (param != &para_str[i]) {
                    list_out[param_cnt] = param;
                    param_cnt++;
                    if (param_cnt >= list_cnt) {
                        parse_end = true;
                        break;
                    }
                }
                param = &para_str[i+1];
                break;
            }
            default:
                break;
        }

        if (true == parse_end) {
            break;
        }
    }

    return param_cnt;
}


void atci_dump_data(uint8_t *data, uint16_t len, const char *log_msg)
{
    uint16_t real_len = len > 25 ? 25 : len;
    LOG_HEXDUMP_I(atci, log_msg, data, real_len);
}


