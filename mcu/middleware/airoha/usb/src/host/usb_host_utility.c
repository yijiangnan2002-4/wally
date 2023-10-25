/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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



#ifdef AIR_USB_HOST_ENABLE

#include "usb_dbg.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if FREERTOS_ENABLE
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_def.h"
#endif /* FREERTOS_ENABLE */

#include "usb_host.h"
#include "usb_host_utility.h"



static QueueHandle_t usb_host_queue_handle = NULL;


bool Usb_Host_Message_Queue_Init(void)
{
    /* Queue creation */
    usb_host_queue_handle = xQueueCreate(USB_QUEUE_LENGTH, sizeof(usb_host_msg_t));

    if (usb_host_queue_handle == NULL) {
        LOG_MSGID_E(USB_MAIN, "Create usb host message queue failed!", 0);
        return false;
    } else {
        LOG_MSGID_I(USB_MAIN, "Create usb host message queue successfully", 0);
    }

    return true ;
}

bool Usb_Host_Receieve_Message(usb_host_msg_t *msgs)
{
    if (usb_host_queue_handle == NULL) {
        LOG_MSGID_E(USB_MAIN, "USB Host queue handle is NULL", 0);
        return false;
    }

    if (xQueueReceive(usb_host_queue_handle, msgs, portMAX_DELAY)) {
        //LOG_MSGID_I(USB_MAIN, "USB host Rx message successfully", 0);
        return true;
    }
    return false;
}

void USB_Host_Send_Message_ISR(usb_host_msg_type_t msg, void *data)
{
    usb_host_msg_t msgs;
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t ret;

    if (usb_host_queue_handle == NULL) {
        LOG_MSGID_E(USB_MAIN, "usb message queue not initlize", 0);
        return;
    }

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    msgs.host_msg_id = msg;
    msgs.data = data;

    /*
    if (0 == HAL_NVIC_QUERY_EXCEPTION_NUMBER) {
        ret = xQueueSend(usb_host_queue_handle, &msgs, 0);
        LOG_MSGID_I(USB_MAIN, "Send Queue in Task !! id = %d ", 1, msgs.host_msg_id);
    }
    else
    */
    {
        ret = xQueueSendFromISR(usb_host_queue_handle, &msgs, &xHigherPriorityTaskWoken);
        //LOG_MSGID_I(USB_MAIN, "Send Queue in ISR !! id = %d ", 1, msgs.host_msg_id);
    }

    if (ret != pdTRUE) {
        LOG_MSGID_E(USB_MAIN, "Send Queue fail!! Queue size = %d ", 1, USB_QUEUE_LENGTH);
    }
    // Now the buffer is empty we can switch context if necessary.
    if (xHigherPriorityTaskWoken) {
        // Actual macro used here is port specific.
        portYIELD_FROM_ISR(pdTRUE);
    }
}

#endif /* AIR_USB_HOST_ENABLE */
