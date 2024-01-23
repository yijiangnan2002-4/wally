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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "assert.h"
#include "hal_uart.h"
#include "hal_log.h"
#include "hal_gpt.h"
#include "dsp_audio_msg.h"
#include <stdio.h>
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include "task_def.h"
#include "memory_attribute.h"
#include <string.h>
#include "hal_audio_common.h"
#include "hal_nvic.h"
#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif

#define CALLER_RX_ACK   (1)
#define CALLER_RX_TASK  (2)
#define CALLER_RX_HDR   (3)
#define CALLER_TX_TASK  (4)
#define CALLER_TX_HDR   (5)
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
#define CALLER_NONREALTIME_RX_ACK   (6)
#define CALLER_NONREALTIME_RX_TASK  (7)
#define CALLER_NONREALTIME_RX_HDR   (8)
#endif
#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
#define CALLER_IRQ_LEVEL_RX_ACK   (9)
#endif

static volatile aud_msg_node_t *rx_q_front = NULL;
static volatile aud_msg_node_t *rx_q_rear = NULL;
static volatile aud_msg_node_t *tx_q_front = NULL;
static volatile aud_msg_node_t *tx_q_rear = NULL;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
static volatile aud_msg_node_t *nonrealtime_rx_q_front = NULL;
static volatile aud_msg_node_t *nonrealtime_rx_q_rear = NULL;
#endif

static volatile TaskHandle_t aud_msg_rx_handle;
static volatile TaskHandle_t aud_msg_tx_handle;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
static volatile TaskHandle_t aud_msg_nonrealtime_rx_handle;
#endif

#ifdef MTK_CM4_RECORD_ENABLE //workaround for BTA-26914
bool volatile record_ccni_msg_existed = false; //MSG_DSP2MCU_RECORD_DATA_NOTIFY
#endif


/** AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
  *  define: allocate aud_msg_cb_node_t from pre-allocated array.
  *  undefine: allocate aud_msg_cb_node_t from FreeRTOS heap
  *  - This would decide to let rx_rx_register_callback be allocated with efficient memory usage or not.
  *  - Please always define it in dsp_audio_msg.h.
  */

/** AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
  *  define: allocate aud_msg_cb_node_t from pre-allocated array in static memory.
  *  undefine: allocate aud_msg_cb_node_t from pre-allocated array in FreeRTOS heap memory.
  *  - Please choose which behavior would fit user's request.
  */

/** AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_CUSTOM_SIZE
  *  define: The size of pre-allocated memory depended on how many CCNI msg that user want to handle.
  *  undefine: SDK default maximum size.
  *  - Please choose which behavior would fit user's request.
  */

/** AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_SIZE_LIMIT_ASSERT
  *  define: Over size of the maximum pre-allocated array would assert.
  *  undefine: Over size would not assert but use original method(inefficient memory usage) to allocate memory.
  *  - Please choose which behavior would fit user's request.
  */

//#define AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
#ifndef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_CUSTOM_SIZE
#define RX_CB_MAX_SDK_DEFAULT (123)
#define TX_CB_MAX_SDK_DEFAULT (5)
#define RX_TX_CB_MAX (RX_CB_MAX_SDK_DEFAULT + TX_CB_MAX_SDK_DEFAULT)
#else
#define RX_TX_CB_MAX (AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_CUSTOM_SIZE)
#endif
static uint16_t gRx_Tx_cb_count = 0;
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
static aud_msg_cb_node_t rx_tx_cb_mem_pool[RX_TX_CB_MAX];
#else
static aud_msg_cb_node_t *rx_tx_cb_mem_pool = NULL;
#endif
#endif
static aud_msg_cb_node_t *rx_cb_front = NULL;
static aud_msg_cb_node_t *tx_ack_cb_front = NULL;

static ccni_memo_t ccni_memo_rx;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
static ccni_memo_t ccni_memo_nonrealtime_rx;
#endif
static ccni_memo_t ccni_memo_tx;
static uint16_t ccni_debug_tx = 0;

static SemaphoreHandle_t xAud_TxMsgSem = 0;
static SemaphoreHandle_t xAud_TxTaskSem = 0;
static SemaphoreHandle_t xAud_RxTaskSem = 0;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
static SemaphoreHandle_t xAud_Nonrealtime_RxTaskSem = 0;
//static SemaphoreHandle_t xAud_Nonrealtime_Rx_TxMsgSem = 0;
#endif

#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
#define IRQ_LEVEL_RX_CB_MAX (5)
static uint16_t IRQ_Level_Rx_cb_count = 0;
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
static aud_msg_cb_node_t IRQ_Level_Rx_cb_mem_pool[IRQ_LEVEL_RX_CB_MAX];
#else
static aud_msg_cb_node_t *IRQ_Level_Rx_cb_mem_pool = NULL;
#endif
#endif
static aud_msg_cb_node_t *IRQ_Level_rx_cb_front = NULL;
static ccni_memo_t ccni_memo_IRQ_Level_rx;
#endif

#ifndef DSP_AUD_MSG_USE_MSGID_LOG
#define DSP_AUD_MSG_USE_MSGID_LOG
#endif
log_create_module(dsp_aud_msg, PRINT_LEVEL_INFO);
#ifdef DSP_AUD_MSG_USE_MSGID_LOG
#define DSP_AUD_MSG_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(dsp_aud_msg,_message, arg_cnt, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(dsp_aud_msg,_message, arg_cnt, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(dsp_aud_msg,_message, arg_cnt, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(dsp_aud_msg,_message, arg_cnt, ##__VA_ARGS__)
#else
#define DSP_AUD_MSG_LOG_E(_message, arg_cnt, ...)  LOG_E(dsp_aud_msg,_message, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_W(_message, arg_cnt, ...)  LOG_W(dsp_aud_msg,_message, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_I(_message, arg_cnt, ...)  LOG_I(dsp_aud_msg,_message, ##__VA_ARGS__)
#define DSP_AUD_MSG_LOG_D(_message, arg_cnt, ...)  LOG_D(dsp_aud_msg,_message, ##__VA_ARGS__)
#endif


extern void DSP_LOCAL_TX_HANDEL(hal_ccni_message_t msg);

static BOOL audio_sem_take(uint8_t caller, SemaphoreHandle_t sem, BOOL fromISR)
{
    BOOL result = true;
    DSP_AUD_MSG_LOG_D("audio sem take, caller:%d sem:0x%x\r\n", caller, (uint32_t)sem);

    if (fromISR == false) {
        if (xSemaphoreTake(sem, portMAX_DELAY) == pdFALSE) {
            result = false;
        }
        portYIELD();
    } else {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xSemaphoreTakeFromISR(sem, &xHigherPriorityTaskWoken) == pdFALSE) {
            result = false;
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    if (result == false) {
        DSP_AUD_MSG_LOG_E("audio sem take FAIL, from_ISR %d, caller:%d sem:0x%x\r\n", 3, fromISR, caller, (uint32_t)sem);
    }
    UNUSED(caller);
    return result;
}

static void audio_sem_give(uint8_t caller, SemaphoreHandle_t sem, BOOL fromISR)
{
    BOOL result = true;
    DSP_AUD_MSG_LOG_D("audio sem give, caller:%d sem:0x%x\r\n", caller, (uint32_t)sem);

    if (fromISR == false) {
        if (xSemaphoreGive(sem) == pdFALSE) {
            result = false;
        }
        portYIELD();
    } else {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken) == pdFALSE) {
            result = false;
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    if (result == false) {
        DSP_AUD_MSG_LOG_E("audio sem give FAIL, from_ISR %d, caller:%d sem:0x%x\r\n", 3, fromISR, caller, (uint32_t)sem);
    }
    UNUSED(caller);
}

static void msg_enqueue(aud_msg_node_t *tx, aud_msg_queue_t queue)
{
    aud_msg_node_t *temp = NULL;
    aud_msg_node_t **front_ptr = NULL;
    aud_msg_node_t **rear_ptr = NULL;
    aud_msg_node_t *obj = NULL;
    DSP_AUD_MSG_LOG_D("msg enqueue, queue_id %d\r\n", 1, queue);

    if (queue == AUDIO_MSG_TX) {
        front_ptr = (aud_msg_node_t **)&tx_q_front;
        rear_ptr = (aud_msg_node_t **)&tx_q_rear;
    } else if (queue == AUDIO_MSG_RX) {
        front_ptr = (aud_msg_node_t **)&rx_q_front;
        rear_ptr = (aud_msg_node_t **)&rx_q_rear;
    }
    #ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
    else if (queue == AUDIO_MSG_NONREALTIME_RX) {
        front_ptr = (aud_msg_node_t **)&nonrealtime_rx_q_front;
        rear_ptr = (aud_msg_node_t **)&nonrealtime_rx_q_rear;
    }
    #endif

    if (tx->msg.ccni_message[0] & 0x00800000) {   /* put the msg to the front of queue */
        temp = *front_ptr;
        *front_ptr = tx;
        obj = (*front_ptr);
        obj->next = temp;
        if (*rear_ptr == NULL) {
            *rear_ptr = *front_ptr;
        }
    } else {                                     /* put the msg to the rear of queue */
        if (*rear_ptr == NULL) {
            *front_ptr = tx;
            obj = (*front_ptr);
            obj->next = NULL;
            *rear_ptr = *front_ptr;
        } else {
            obj = (*rear_ptr);
            obj->next = tx;
            *rear_ptr = tx;
            obj = (*rear_ptr);
            obj->next = NULL;
        }
    }
}


static hal_ccni_message_t msg_dequeue(aud_msg_queue_t queue)
{
    aud_msg_node_t *temp = NULL;
    aud_msg_node_t **front_ptr = NULL;
    aud_msg_node_t **rear_ptr = NULL;
    aud_msg_node_t *obj = NULL;
    hal_ccni_message_t cur;
    DSP_AUD_MSG_LOG_D("msg dequeue, queue_id %d\r\n", 1, queue);

    if (queue == AUDIO_MSG_TX) {
        front_ptr = (aud_msg_node_t **)&tx_q_front;
        rear_ptr = (aud_msg_node_t **)&tx_q_rear;
    } else if (queue == AUDIO_MSG_RX) {
        front_ptr = (aud_msg_node_t **)&rx_q_front;
        rear_ptr = (aud_msg_node_t **)&rx_q_rear;
    }
    #ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
    else if (queue == AUDIO_MSG_NONREALTIME_RX) {
        front_ptr = (aud_msg_node_t **)&nonrealtime_rx_q_front;
        rear_ptr = (aud_msg_node_t **)&nonrealtime_rx_q_rear;
    }
    #endif

    obj = (*front_ptr);
    cur.ccni_message[0] = obj->msg.ccni_message[0];
    cur.ccni_message[1] = obj->msg.ccni_message[1];
    temp = obj->next;
    vPortFree((void *)obj);
    *front_ptr = temp;

    if (*front_ptr == NULL) { /* There is no object in the queue */
        *rear_ptr = *front_ptr;
    }

    return cur;
}

static aud_msg_cb_node_t *_msg_queue_get_middle(aud_msg_cb_node_t *front, aud_msg_cb_node_t *rear)
{
    if (front == NULL) {
        return NULL;
    }

    aud_msg_cb_node_t *step1 = front;
    aud_msg_cb_node_t *step2 = front->next;

    while (step2 != rear) {
        step2 = step2->next;
        if (step2 != rear) {
            step1 = step1->next;
            step2 = step2->next;
        }
    }

    return step1;
}

static aud_msg_cb_info_t *_msg_queue_search(aud_msg_cb_node_t *front, uint16_t ID)
{
    /* Simple binary search */

    aud_msg_cb_node_t *cb;
    aud_msg_cb_node_t *rear = NULL;

    do {
        cb = _msg_queue_get_middle(front, rear);

        if (cb == NULL) {
            return NULL;
        } else if (cb->cb_info.msg_id == ID) {
            return &(cb->cb_info);
        } else if (cb->cb_info.msg_id < ID) {
            front = cb->next;
        } else {
            rear = cb;
        }

    } while (front != rear);

    return NULL;
}

#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
static aud_msg_cb_node_t *AUD_MSG_GET_CB_NODE(void)
{
    aud_msg_cb_node_t *cb_node_ptr = NULL;
    if (gRx_Tx_cb_count < RX_TX_CB_MAX) {
        cb_node_ptr = rx_tx_cb_mem_pool + gRx_Tx_cb_count;
        gRx_Tx_cb_count ++;
    } else {
        DSP_AUD_MSG_LOG_E("Audio RX_TX_msg register callback over size cur:(%d) max:(%d).\r\n", 2, gRx_Tx_cb_count, RX_TX_CB_MAX);
        if (gRx_Tx_cb_count < 65535) { //UINT16_MAX
            gRx_Tx_cb_count ++;
        }
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_SIZE_LIMIT_ASSERT
        platform_assert("[AUD_MSG] Audio RX_TX_msg register callback over size.", __func__, __LINE__); //Assert if need
#else
        cb_node_ptr = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
    }
    return cb_node_ptr;
}
#endif

#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
static aud_msg_cb_node_t *AUD_MSG_IRQ_LEVEL_GET_CB_NODE(void)
{
    aud_msg_cb_node_t *cb_node_ptr = NULL;
    if (IRQ_Level_Rx_cb_count < IRQ_Level_Rx_cb_count) {
        cb_node_ptr = IRQ_Level_Rx_cb_mem_pool + IRQ_Level_Rx_cb_count;
        IRQ_Level_Rx_cb_count ++;
    } else {
        DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] Audio IRQ_LEVEL_RX_msg register callback over size cur:(%d) max:(%d).\r\n", 2, IRQ_Level_Rx_cb_count, IRQ_LEVEL_RX_CB_MAX);
        if (IRQ_Level_Rx_cb_count < 65535) { //UINT16_MAX
            IRQ_Level_Rx_cb_count ++;
        }
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_SIZE_LIMIT_ASSERT
        platform_assert("[AUD_MSG][IRQ_LEVEL_RX] Audio IRQ_LEVEL_RX_msg register callback over size.", __func__, __LINE__); //Assert if need
#else
        cb_node_ptr = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
    }
    return cb_node_ptr;
}
#endif
#endif


/* Assume aud_msg_rx_ack is only called in Task Level */
static void aud_msg_rx_ack(aud_msg_queue_t queue_id, hal_ccni_message_t ack)
{
    uint32_t tx_msg_queue_empty = false;
    uint32_t caller_ack_id = 0;

    switch (queue_id) {
        case AUDIO_MSG_RX:
            caller_ack_id = CALLER_RX_ACK;
            break;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
        case AUDIO_MSG_NONREALTIME_RX:
            caller_ack_id = CALLER_NONREALTIME_RX_ACK;
            break;
#endif
        default:
            DSP_AUD_MSG_LOG_D("[RX_AUD_MSG] aud_msg_rx_ack, unsupport queue_id %d", 1, queue_id);
            return;
            break;
    }

    aud_msg_node_t *rx_ack = (aud_msg_node_t *)pvPortMalloc(sizeof(aud_msg_node_t));
    if (rx_ack == NULL) {
        DSP_AUD_MSG_LOG_E("[RX_AUD_MSG] queue_id %d, AUDIO CCNI MSG ACK malloc failed !!\r\n", 1, queue_id);
        return;
    }
    rx_ack->msg.ccni_message[0] = ack.ccni_message[0] | 0x80000000; /* [31]=1 -> ACK, [23]=1 -> to the front of queue */
    rx_ack->msg.ccni_message[1] = ack.ccni_message[1];

    /* Enter critical section */
    audio_sem_take(caller_ack_id, xAud_TxMsgSem, false);

    /* Chech if TX msg Queue is empty */
    if (tx_q_front == NULL) {
        tx_msg_queue_empty = true;
    }

    DSP_AUD_MSG_LOG_D("[RX_AUD_MSG] queue_id %d, rx_ack pMsg[0]=0x%08x, pMsg[1]=0x%08x, tx_q_front=0x%x\r\n", 4, queue_id, rx_ack->msg.ccni_message[0], rx_ack->msg.ccni_message[1], (int32_t)tx_q_front);

    /* Enqueue msg */
    msg_enqueue(rx_ack, AUDIO_MSG_TX);

    /* Add debug memo */
    memcpy(&ccni_memo_tx.enq_, &ack, sizeof(hal_ccni_message_t));

    /* Leave critical section */
    audio_sem_give(caller_ack_id, xAud_TxMsgSem, false);

    /* Wakeup TX task if necessary */
    if (tx_msg_queue_empty != false) {
        audio_sem_give(caller_ack_id, xAud_TxTaskSem, false);
    }
}

#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
static void aud_msg_IRQ_Level_rx_ack(hal_ccni_message_t ack)
{
    uint32_t tx_msg_queue_empty = false;
    aud_msg_node_t *rx_ack = (aud_msg_node_t *)pvPortMalloc(sizeof(aud_msg_node_t));
    if (rx_ack == NULL) {
        DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] AUDIO CCNI MSG ACK malloc failed !!\r\n", 0);
        return;
    }
    rx_ack->msg.ccni_message[0] = ack.ccni_message[0] | 0x80000000 | 0x00800000; /* [31]=1 -> ACK, [23]=1 -> put front*/
    rx_ack->msg.ccni_message[1] = ack.ccni_message[1];

    /* Enter critical section */
    audio_sem_take(CALLER_IRQ_LEVEL_RX_ACK, xAud_TxMsgSem, true);

    /* Chech if TX msg Queue is empty */
    if (tx_q_front == NULL) {
        tx_msg_queue_empty = true;
    }

    DSP_AUD_MSG_LOG_D("[IRQ_LEVEL_RX] rx_ack pMsg[0]=0x%08x, pMsg[1]=0x%08x, tx_q_front=0x%x\r\n", 3, rx_ack->msg.ccni_message[0], rx_ack->msg.ccni_message[1], (int32_t)tx_q_front);

    /* Enqueue msg */
    msg_enqueue(rx_ack, AUDIO_MSG_TX);

    /* Add debug memo */
    memcpy(&ccni_memo_tx.enq_, &ack, sizeof(hal_ccni_message_t));

    /* Leave critical section */
    audio_sem_give(CALLER_IRQ_LEVEL_RX_ACK, xAud_TxMsgSem, true);

    /* Wakeup TX task if necessary */
    if (tx_msg_queue_empty != false) {
        audio_sem_give(CALLER_IRQ_LEVEL_RX_ACK, xAud_TxTaskSem, true);
    }
}
#endif

static uint32_t aud_msg_get_task_max_runtime(aud_msg_queue_t queue_id)
{
    switch (queue_id) {
        case AUDIO_MSG_TX:
            return TX_TASK_MAX_RUNTIME;
            break;
        case AUDIO_MSG_RX:
            return REALTIME_RX_TASK_MAX_RUNTIME;
            break;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
        case AUDIO_MSG_NONREALTIME_RX:
            return NONREALTIME_RX_TASK_MAX_RUNTIME;
            break;
#endif
        default:
            DSP_AUD_MSG_LOG_D("[RX_AUD_MSG] aud_msg_get_task_max_runtime, unsupport queue_id %d", 1, queue_id);
            return 50000;
            break;
    }
}


static void aud_msg_rx_task_ack_process(aud_msg_queue_t queue_id/*, hal_ccni_message_t* cur*/)
{
    hal_ccni_message_t cur;
    hal_ccni_message_t ack;
    uint16_t msg_id = 0;
    aud_msg_cb_info_t *cb;
    aud_rx_msg_callback_t rx_cb;
    aud_tx_msg_callback_t tx_ack_cb;

    ccni_memo_t* ccni_memo;

    switch (queue_id) {
        case AUDIO_MSG_RX:
            ccni_memo = &ccni_memo_rx;
            break;
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
        case AUDIO_MSG_NONREALTIME_RX:
            ccni_memo = &ccni_memo_nonrealtime_rx;
            break;
#endif
        default:
            DSP_AUD_MSG_LOG_D("[RX_AUD_MSG] aud_msg_rx_task_ack_process, unsupport queue_id %d", 1, queue_id);
            return;
            break;
    }

    hal_ccni_mask_event(CCNI_CM4_TO_DSP0_RX_EVENT);    /* to avoid race condition (rx queue)*/
    //hal_ccni_mask_event(AUDIO_N9_RX_EVENT);
    //hal_ccni_mask_event(CCNI_N9_TO_DSP0_EVENT1);
    cur = msg_dequeue(queue_id);
    memcpy(&ccni_memo->deq_, &cur, sizeof(hal_ccni_message_t));
    //hal_ccni_unmask_event(CCNI_N9_TO_DSP0_EVENT1);
    //hal_ccni_unmask_event(AUDIO_N9_RX_EVENT);
    hal_ccni_unmask_event(CCNI_CM4_TO_DSP0_RX_EVENT);

    msg_id = (cur.ccni_message[0] & 0x7FFF0000) >> 16;

    if (cur.ccni_message[0] & 0x80000000) { /* If ccni_message[0] bit 31 = 1, it stands for an ACK */
        cb = _msg_queue_search(tx_ack_cb_front, msg_id);

        if (cb != NULL) {
            if (cb->cb_func != NULL) {
                tx_ack_cb.callback = cb->cb_func;
                tx_ack_cb.callback(cur);
            }
        }
    } else {
        cb = _msg_queue_search(rx_cb_front, msg_id);
        if (cb == NULL) {   /* ack only */
            memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
            ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */

            aud_msg_rx_ack(queue_id, ack);
        } else {
            uint32_t start_time, end_time;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_time);
            if (cb->ack_option & 0x1) { /* NO ACK */
                if (cb->cb_func != NULL) {
                    rx_cb.callback = cb->cb_func;
                    rx_cb.callback(cur, NULL);
                    memcpy(&ccni_memo->exec_, &cur, sizeof(hal_ccni_message_t));
                }
            } else if (cb->ack_option & 0x2) {   /* execute callback function before response ACK. */
                memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
                ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */

                if (cb->cb_func != NULL) {
                    rx_cb.callback = cb->cb_func;
                    rx_cb.callback(cur, &ack);
                    memcpy(&ccni_memo->exec_, &cur, sizeof(hal_ccni_message_t));
                }
                aud_msg_rx_ack(queue_id, ack);
            } else {
                memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
                ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */
                aud_msg_rx_ack(queue_id, ack);

                if (cb->cb_func != NULL) {
                    rx_cb.callback = cb->cb_func;
                    rx_cb.callback(cur, &ack);
                    memcpy(&ccni_memo->exec_, &cur, sizeof(hal_ccni_message_t));
                }
            }
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_time);

            if ((uint32_t)(end_time - start_time) > aud_msg_get_task_max_runtime(queue_id)) {
                DSP_AUD_MSG_LOG_W("[RX_AUD_MSG] queue_id %d, long callback time:%d us from rx msg:0x%x", 3, queue_id, (uint32_t)(end_time - start_time), (uint32_t)msg_id);
            }
        }

    }

}

void aud_msg_rx_task(void)
{
    volatile aud_msg_node_t **p;

    while (1) {
        p = &rx_q_front;
        if (*p != NULL) {
            DSP_AUD_MSG_LOG_D("[REALTIME_RX] rx queue is not empty\r\n", 0);

            aud_msg_rx_task_ack_process(AUDIO_MSG_RX);
        } else {
            DSP_AUD_MSG_LOG_D("[REALTIME_RX] rx queue is empty\r\n", 0);
            audio_sem_take(CALLER_RX_TASK, xAud_RxTaskSem, false);
        }
    }
}


#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
void aud_msg_nonrealtime_rx_task(void)
{
    volatile aud_msg_node_t **p;

    while (1) {
        p = &nonrealtime_rx_q_front;
        if (*p != NULL) {
            DSP_AUD_MSG_LOG_D("[NONREALTIME_RX] rx queue is not empty\r\n", 0);

            aud_msg_rx_task_ack_process(AUDIO_MSG_NONREALTIME_RX);
        } else {
            DSP_AUD_MSG_LOG_D("[NONREALTIME_RX] rx queue is empty\r\n", 0);
            audio_sem_take(CALLER_NONREALTIME_RX_TASK, xAud_Nonrealtime_RxTaskSem, false);
        }
    }
}
#endif

#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
void aud_msg_IRQ_Level_Rx_execute(hal_ccni_message_t *irq_rx)
{

    hal_ccni_message_t cur;
    hal_ccni_message_t ack;

    uint16_t msg_id = 0;

    aud_msg_cb_info_t *cb;
    aud_rx_msg_callback_t rx_cb;
    //aud_tx_msg_callback_t tx_ack_cb;
    //volatile aud_msg_node_t **p;

    cur = (*irq_rx);
    msg_id = (cur.ccni_message[0] & 0x7FFF0000) >> 16;

    cb = _msg_queue_search(IRQ_Level_rx_cb_front, msg_id);

    if (cb == NULL) {   /* ack only */
        memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
        ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */
        DSP_AUD_MSG_LOG_D("[IRQ_LEVEL_RX] aud_msg_IRQ_Level_Rx_execute, CB NULL\r\n", 0);
        aud_msg_IRQ_Level_rx_ack(ack);
    } else {
        uint32_t start_time, end_time;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_time);
        if (cb->ack_option & 0x1) { /* NO ACK */
            if (cb->cb_func != NULL) {
                rx_cb.callback = cb->cb_func;
                rx_cb.callback(cur, NULL);
            }
        } else if (cb->ack_option & 0x2) {   /* execute callback function before response ACK. */
            memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
            ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */
            if (cb->cb_func != NULL) {
                rx_cb.callback = cb->cb_func;
                rx_cb.callback(cur, &ack);

            }
            aud_msg_IRQ_Level_rx_ack(ack);
        } else {
            memset((void *)&ack, 0, sizeof(hal_ccni_message_t));
            ack.ccni_message[0] = cur.ccni_message[0] & 0x7FFF0000;    /* copy ID to ack */
            aud_msg_IRQ_Level_rx_ack(ack);
            if (cb->cb_func != NULL) {
                rx_cb.callback = cb->cb_func;
                rx_cb.callback(cur, &ack);
            }
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_time);

        if ((uint32_t)(end_time - start_time) > 100) {
            DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] long callback time:%d us from rx msg:0x%x", 2, (uint32_t)(end_time - start_time), (uint32_t)msg_id);
            platform_assert("[IRQ_LEVEL_RX] IRQ_Level_rx callback execution time > 100us!!", __func__, __LINE__);
        }
    }
}

#endif



void aud_msg_tx_task(void)
{
    hal_ccni_message_t cur;
    uint32_t i;
    uint32_t msg_ID;
    uint32_t tx_event = 0;
    uint32_t gpt_timer0, gpt_timer1;
    hal_ccni_status_t ret;

    while (1) {
        if (tx_q_front != NULL) {
            DSP_AUD_MSG_LOG_D("tx queue is not empty\r\n", 0);

            audio_sem_take(CALLER_TX_TASK, xAud_TxMsgSem, false);
            cur = msg_dequeue(AUDIO_MSG_TX);
            memcpy(&ccni_memo_tx.deq_, &cur, sizeof(hal_ccni_message_t));
            #ifdef MTK_CM4_RECORD_ENABLE //workaround for BTA-26914
            if(((cur.ccni_message[0] & 0x7FFF0000) >> 16) == (MSG_DSP2MCU_RECORD_DATA_NOTIFY)){
                if (record_ccni_msg_existed) {
                    record_ccni_msg_existed = false;
                    DSP_AUD_MSG_LOG_W("[Record_Notify_Special_Case] MSG 0x%x is sent, clear flag %d\r\n", 2, ((cur.ccni_message[0] & 0x7FFF0000) >> 16), record_ccni_msg_existed);
                }
            }
            #endif
            audio_sem_give(CALLER_TX_TASK, xAud_TxMsgSem, false);

            msg_ID = (cur.ccni_message[0] & 0x7FFF0000) >> 16;

            if ((msg_ID & MSG_DSP2LOCAL_AUDIO_PROCESS) != MSG_DSP2LOCAL_AUDIO_PROCESS) {
                if (msg_ID < 0x3000) {          /* to CM4 */
                    tx_event = CCNI_DSP0_TO_CM4_TX_EVENT;
#if 0
                } else if (msg_ID < 0x4000) {   /* to N9 */
                    tx_event = AUDIO_N9_TX_EVENT;
#endif
                } else if (msg_ID < 0x5000) {   /* to DSP1 */
                } else if (msg_ID < 0x6000) {   /* Reserved */
                } else if (msg_ID < 0x7000) {   /* Customer, to CM4 */
                } else if (msg_ID < 0x8000) {   /* Customer, to N9 */
                }
                gpt_timer0 = 0;
                for (i = 0; (ret = hal_ccni_set_event(tx_event, &cur)) != HAL_CCNI_STATUS_OK; i++) {
                    if (!gpt_timer0) {
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer0);
                    }
                    if (i >= AUDIO_TX_RETRY_TIMES) {
                        DSP_AUD_MSG_LOG_E("DSP msg tx retry timeout ret(%d)", 1, ret);
                        ccni_debug_tx = ((uint8_t)ret << 8) | (uint8_t)i;
                    }
#ifdef AIR_ICE_DEBUG_ENABLE
                    if (hal_ice_debug_is_enabled() == false) {
                        AUDIO_ASSERT(i < AUDIO_TX_RETRY_TIMES);
                    }
#else
                    AUDIO_ASSERT(i < AUDIO_TX_RETRY_TIMES);
#endif
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                if (gpt_timer0) {
                    gpt_timer1 = 0;
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer1);
                    DSP_AUD_MSG_LOG_I("DSP msg tx event busy ID 0x%x pending time: %d", 2, msg_ID, gpt_timer1 - gpt_timer0);
                    if ((uint32_t)(gpt_timer1 - gpt_timer0) > TX_TASK_MAX_RUNTIME) {
                        DSP_AUD_MSG_LOG_W("DSP Tx long callback time:%d us from tx msg:0x%x", 2, (uint32_t)(gpt_timer1 - gpt_timer0), msg_ID);
                    }
                }
            } else {
                DSP_AUD_MSG_LOG_I("DSP TX task special process(0x%x).", 1, cur.ccni_message[0]);
                DSP_LOCAL_TX_HANDEL(cur);
            }
        } else {
            DSP_AUD_MSG_LOG_D("tx queue is empty\r\n", 0);
            audio_sem_take(CALLER_TX_TASK, xAud_TxTaskSem, false);
        }
    }
}

static int msg_rx_task_init(void)
{
    int ret;
#ifdef MTK_SLT_AUDIO_HW
    ret = xTaskCreate((void *)aud_msg_rx_task, "(RX) Audio Message Handler", 3072 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_SOFT_REALTIME, (void *)&aud_msg_rx_handle);
#else
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    ret = xTaskCreate((void *)aud_msg_rx_task, "(RX) Audio Message Handler", 2560 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_HIGH, (void *)&aud_msg_rx_handle);
#else
#if defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE)
    ret = xTaskCreate((void *)aud_msg_rx_task, "(RX) Audio Message Handler", 3072 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_SOFT_REALTIME, (void *)&aud_msg_rx_handle);
#else
    ret = xTaskCreate((void *)aud_msg_rx_task, "(RX) Audio Message Handler", 2560 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_SOFT_REALTIME, (void *)&aud_msg_rx_handle);
#endif //AIR_DCHS_MODE_ENABLE
#endif
#endif
    if (ret != 1) {
        DSP_AUD_MSG_LOG_D("Audio MSG RX task create fail !!\r\n", 0);
        return 0;
    }
    return 1;
}

#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
static int msg_nonrealtime_rx_task_init(void)
{
    int ret;
    ret = xTaskCreate((void *)aud_msg_nonrealtime_rx_task, "(NONREALTIME_RX) Audio Message Handler", 2560 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_LOW, (void *)&aud_msg_nonrealtime_rx_handle);

    if (ret != 1) {
        DSP_AUD_MSG_LOG_D("[NONREALTIME_RX] Audio MSG NONREALTIME_RX task create fail !!\r\n", 0);
        return 0;
    }
    return 1;
}
#endif


static int msg_tx_task_init(void)
{
    int ret;
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    ret = xTaskCreate((void *)aud_msg_tx_task, "(TX) Audio Message Handler", 2048 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_HIGH, (void *)&aud_msg_tx_handle);
#else
    ret = xTaskCreate((void *)aud_msg_tx_task, "(TX) Audio Message Handler", 2048 / sizeof(StackType_t), (void *)NULL, TASK_PRIORITY_SOFT_REALTIME, (void *)&aud_msg_tx_handle);
#endif
    if (ret != 1) {
        DSP_AUD_MSG_LOG_D("Audio MSG TX task create fail !!\r\n", 0);
        return 0;
    }

    return 1;
}

void aud_msg_rx_handler(hal_ccni_event_t event, void *msg)
{
    hal_ccni_status_t status;
    SemaphoreHandle_t Sem = xAud_RxTaskSem;
    uint32_t Caller = CALLER_RX_HDR;
    volatile aud_msg_node_t *q_front = rx_q_front;
    uint32_t Queue = AUDIO_MSG_RX;
    ccni_memo_t ccni_memo = ccni_memo_rx;

    uint32_t *pMsg = (uint32_t *)msg;

    if(pMsg[0] & 0x40000000){ /* If ccni_message[0] bit 30 = 1, it stands for an IRQ Level CCNI */
        #ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
        status = hal_ccni_mask_event(event);
        aud_msg_node_t irq_rx;
        memset(&irq_rx, 0, sizeof(aud_msg_node_t));
        irq_rx.msg.ccni_message[0] = pMsg[0];
        irq_rx.msg.ccni_message[1] = pMsg[1];

        aud_msg_IRQ_Level_Rx_execute((hal_ccni_message_t *)&irq_rx);
        //msg_enqueue(rx, Queue); //AUDIO_MSG_RX,
        memcpy(&ccni_memo_IRQ_Level_rx.enq_, &(irq_rx.msg.ccni_message), sizeof(hal_ccni_message_t)); //&ccni_memo_rx.enq_
        DSP_AUD_MSG_LOG_D("[IRQ_LEVEL_RX] Rx_handler end\r\n", 0);
        goto _Rx_Error_Handleing;
        #endif

    //}else if((pMsg[0] & 0x20000000)){ /* If ccni_message[0] bit 29 = 1, it stands for an Realtime Level CCNI */

    }else if(!(pMsg[0] & 0x20000000)){ /* If ccni_message[0] bit 29 = 0, it stands for an Non-Realtime Level CCNI */
        #ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
        Sem = xAud_Nonrealtime_RxTaskSem;
        Caller = CALLER_NONREALTIME_RX_HDR;
        q_front = nonrealtime_rx_q_front;
        Queue = AUDIO_MSG_NONREALTIME_RX;
        ccni_memo = ccni_memo_nonrealtime_rx;
        #endif
    }

    if (Sem == NULL) { //xAud_RxTaskSem
        DSP_AUD_MSG_LOG_E("error!!!! aud_msg_rx_handler: xAud_RxTaskSem NULL\n", 0);
        hal_ccni_clear_event(event);
        hal_ccni_unmask_event(event);
        return;
    }

    status = hal_ccni_mask_event(event);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_AUD_MSG_LOG_D("CM4 CCNI mask event: 0x%x something wrong, return is %d\r\n", 2, event, status);
    }

    status = hal_ccni_mask_event(CCNI_CM4_TO_DSP0_RX_EVENT);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_AUD_MSG_LOG_D("CM4 CCNI mask event something wrong, return is %d\r\n", 1, status);
    }
#if 0
    status = hal_ccni_mask_event(AUDIO_N9_RX_EVENT);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_MW_LOG_E("N9 CCNI mask event something wrong, return is %d\r\n", 1, status);
    }
#endif

    aud_msg_node_t *rx = (aud_msg_node_t *)pvPortMalloc(sizeof(aud_msg_node_t));

    if (rx == NULL) {
        DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG malloc failed !!\r\n", 0);
        goto _Rx_Error_Handleing;
    }

    DSP_AUD_MSG_LOG_D("Rx_handler pMsg[0]=0x%08x, pMsg[1]=0x%08x\r\n", 2, pMsg[0], pMsg[1]);

    /*****/
    rx->msg.ccni_message[0] = pMsg[0];
    rx->msg.ccni_message[1] = pMsg[1];
    /*****/

    if (!q_front) { //rx_q_front, CALLER_RX_HDR, xAud_RxTaskSem
        audio_sem_give(Caller, Sem, true); //may fail to give sem because rx task is handling a msg right now, it's false alarm.
    }
    msg_enqueue(rx, Queue); //AUDIO_MSG_RX
    memcpy(&ccni_memo.enq_, &(rx->msg.ccni_message), sizeof(hal_ccni_message_t)); //&ccni_memo_rx.enq_

_Rx_Error_Handleing:
    status = hal_ccni_clear_event(event);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_AUD_MSG_LOG_D("CCNI clear event something wrong, return is %d\r\n", 1, status);
    }
#if 0
    status = hal_ccni_unmask_event(AUDIO_N9_RX_EVENT);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_MW_LOG_E("N9 CCNI unmask event something wrong, return is %d\r\n", 1, status);
    }
#endif
    status = hal_ccni_unmask_event(CCNI_CM4_TO_DSP0_RX_EVENT);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_AUD_MSG_LOG_D("CM4 CCNI unmask event something wrong, return is %d\r\n", 1, status);
    }

    status = hal_ccni_unmask_event(event);
    if (status != HAL_CCNI_STATUS_OK) {
        DSP_AUD_MSG_LOG_D("CM4 CCNI unmask event: 0x%x something wrong, return is %d\r\n", 2, event, status);
    }

    DSP_AUD_MSG_LOG_D("Rx_handler end\r\n", 0);
}

/* When call aud_msg_tx_handler from ISR, need to check returned aud_msg_status
    If aud_msg_status is false, then try again */
aud_msg_status_t aud_msg_tx_handler(hal_ccni_message_t msg, uint8_t to_the_front, uint8_t from_ISR)
{
    uint32_t tx_msg_queue_empty = false;
    aud_msg_node_t *tx = (aud_msg_node_t *)pvPortMalloc(sizeof(aud_msg_node_t));
    if (tx == NULL) {
        DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG malloc failed !!\r\n", 0);
        return AUDIO_MSG_STATUS_ERROR;
    }
    tx->msg.ccni_message[0] = msg.ccni_message[0] | (to_the_front << 23);
    tx->msg.ccni_message[1] = msg.ccni_message[1];



    /* Enter critical section */
    if (audio_sem_take(CALLER_TX_HDR, xAud_TxMsgSem, (BOOL)from_ISR) == true) {

        /* Chech if TX msg Queue is empty */
        if (tx_q_front == NULL) {
            tx_msg_queue_empty = true;
        }

        #ifdef MTK_CM4_RECORD_ENABLE //workaround for BTA-26914
        if (msg.ccni_message[0] == (MSG_DSP2MCU_RECORD_DATA_NOTIFY << 16)) {
            if (record_ccni_msg_existed) {
                DSP_AUD_MSG_LOG_W("[Record_Notify_Special_Case] MSG 0x%x existed in TX queue, ignore the new one.\r\n", 1, MSG_DSP2MCU_RECORD_DATA_NOTIFY);
                /* Leave critical section */
                audio_sem_give(CALLER_TX_HDR, xAud_TxMsgSem, (BOOL)from_ISR);
                /* Wakeup TX task if necessary */
                if (tx_msg_queue_empty != false) {
                    audio_sem_give(CALLER_TX_HDR, xAud_TxTaskSem, (BOOL)from_ISR);
                }
                vPortFree(tx);
                return AUDIO_MSG_STATUS_ERROR;
            } else { //first in TX queue
                record_ccni_msg_existed = true;
                DSP_AUD_MSG_LOG_W("[Record_Notify_Special_Case] Receive MSG 0x%x, set flag %d.\r\n", 2, MSG_DSP2MCU_RECORD_DATA_NOTIFY, record_ccni_msg_existed);
            }
        }
        #endif

        DSP_AUD_MSG_LOG_D("tx_handler pMsg[0]=0x%08x, pMsg[1]=0x%08x, tx_q_front=0x%x\r\n", 3, tx->msg.ccni_message[0], tx->msg.ccni_message[1], (int32_t)tx_q_front);

        /* Enqueue msg */
        msg_enqueue(tx, AUDIO_MSG_TX);

        /* Add debug memo */
        memcpy(&ccni_memo_tx.enq_, &msg, sizeof(hal_ccni_message_t));

        /* Leave critical section */
        audio_sem_give(CALLER_TX_HDR, xAud_TxMsgSem, (BOOL)from_ISR);

        /* Wakeup TX task if necessary */
        if (tx_msg_queue_empty != false) {
            audio_sem_give(CALLER_TX_HDR, xAud_TxTaskSem, (BOOL)from_ISR);
        }
    } else {
        DSP_AUD_MSG_LOG_W("tx_handler pMsg[0]=0x%08x, pMsg[1]=0x%08x, tx_q_front=0x%x NG\r\n", 3, tx->msg.ccni_message[0], tx->msg.ccni_message[1], (int32_t)tx_q_front);
        vPortFree(tx);
        return AUDIO_MSG_STATUS_ERROR;
    }

    DSP_AUD_MSG_LOG_D("tx_handler end\r\n", 0);
    return AUDIO_MSG_STATUS_OK;
}


#ifdef AIR_AUDIO_IRQ_LEVEL_RX_ENABLE
aud_msg_status_t aud_msg_IRQ_level_rx_register_callback(mcu2dsp_audio_msg_t ID, void *rx_callback, uint8_t ack_opt)
{
    DSP_AUD_MSG_LOG_I("[IRQ_LEVEL_RX] aud_msg_IRQ_level_rx_register_callback, ID 0x%x\r\n", 1, ID);
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
    if (IRQ_Level_Rx_cb_count == 0) { //AUD_MSG_GET_CB_NODE!!
#ifndef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
        IRQ_Level_Rx_cb_mem_pool = (aud_msg_cb_node_t *)pvPortMalloc(IRQ_LEVEL_RX_CB_MAX * sizeof(aud_msg_cb_node_t));
        if (IRQ_Level_Rx_cb_mem_pool == NULL) {
            DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB malloc mem poll failed !!\r\n", 0);
            platform_assert("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB malloc mem poll failed !!", __func__, __LINE__);
            return AUDIO_MSG_STATUS_ERROR;
        }
#endif
        memset(IRQ_Level_Rx_cb_mem_pool, 0, IRQ_LEVEL_RX_CB_MAX * sizeof(aud_msg_cb_node_t));
    }
#endif

    aud_msg_cb_node_t *cur = IRQ_Level_rx_cb_front;
    aud_msg_cb_node_t *tmp;

    if (cur == NULL) {  /* Nothing in the queue */
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_IRQ_LEVEL_GET_CB_NODE();
#if 1//#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB RX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, IRQ_Level_Rx_cb_count, IRQ_LEVEL_RX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }

        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = rx_callback;
        cb->cb_info.ack_option = ack_opt;
        cb->next = NULL;
        IRQ_Level_rx_cb_front = cb;
        return AUDIO_MSG_STATUS_OK;
    }

    while ((cur->next != NULL) && (ID >= cur->next->cb_info.msg_id)) {  /* Search the the closest node */
        cur = cur->next;
    }

    /* Replace or insert the node */
    if (ID == cur->cb_info.msg_id) {
        cur->cb_info.cb_func = rx_callback;
        cur->cb_info.ack_option = ack_opt;
        return AUDIO_MSG_STATUS_OK;
    } else {
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_IRQ_LEVEL_GET_CB_NODE();
#if 1 //#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB RX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, IRQ_Level_Rx_cb_count, IRQ_LEVEL_RX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("[IRQ_LEVEL_RX] AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = rx_callback;
        cb->cb_info.ack_option = ack_opt;

        if (ID > cur->cb_info.msg_id) {
            tmp = cur->next;
            cur->next = cb;
            cb->next = tmp;
        } else {
            tmp = cur;
            cur = cb;
            cb->next = tmp;
        }

        /*  Updated the front of queue pointer */
        if (IRQ_Level_rx_cb_front->cb_info.msg_id > cb->cb_info.msg_id) {
            IRQ_Level_rx_cb_front = cb;
        }
    }

    return AUDIO_MSG_STATUS_OK;
}
#endif

aud_msg_status_t aud_msg_rx_register_callback(mcu2dsp_audio_msg_t ID, void *rx_callback, uint8_t ack_opt)
{
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
    if (gRx_Tx_cb_count == 0) {
#ifndef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
        rx_tx_cb_mem_pool = (aud_msg_cb_node_t *)pvPortMalloc(RX_TX_CB_MAX * sizeof(aud_msg_cb_node_t));
        if (rx_tx_cb_mem_pool == NULL) {
            platform_assert("AUDIO CCNI MSG CB malloc mem poll failed !!", __func__, __LINE__);
            return AUDIO_MSG_STATUS_ERROR;
        }
#endif
        memset(rx_tx_cb_mem_pool, 0, RX_TX_CB_MAX * sizeof(aud_msg_cb_node_t));
    }
#endif

    aud_msg_cb_node_t *cur = rx_cb_front;
    aud_msg_cb_node_t *tmp;

    if (cur == NULL) {  /* Nothing in the queue */
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_GET_CB_NODE();
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("AUDIO CCNI MSG CB RX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, gRx_Tx_cb_count, RX_TX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }

        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = rx_callback;
        cb->cb_info.ack_option = ack_opt;
        cb->next = NULL;
        rx_cb_front = cb;
        return AUDIO_MSG_STATUS_OK;
    }

    while ((cur->next != NULL) && (ID >= cur->next->cb_info.msg_id)) {  /* Search the the closest node */
        cur = cur->next;
    }

    /* Replace or insert the node */
    if (ID == cur->cb_info.msg_id) {
        cur->cb_info.cb_func = rx_callback;
        cur->cb_info.ack_option = ack_opt;
        return AUDIO_MSG_STATUS_OK;
    } else {
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_GET_CB_NODE();
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("AUDIO CCNI MSG CB RX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, gRx_Tx_cb_count, RX_TX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = rx_callback;
        cb->cb_info.ack_option = ack_opt;

        if (ID > cur->cb_info.msg_id) {
            tmp = cur->next;
            cur->next = cb;
            cb->next = tmp;
        } else {
            tmp = cur;
            cur = cb;
            cb->next = tmp;
        }

        /*  Updated the front of queue pointer */
        if (rx_cb_front->cb_info.msg_id > cb->cb_info.msg_id) {
            rx_cb_front = cb;
        }
    }

    return AUDIO_MSG_STATUS_OK;
}

aud_msg_status_t aud_msg_tx_ack_register_callback(mcu2dsp_audio_msg_t ID, void *tx_ack_callback)
{
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
    if (gRx_Tx_cb_count == 0) {
#ifndef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_IN_STATIC
        rx_tx_cb_mem_pool = (aud_msg_cb_node_t *)pvPortMalloc(RX_TX_CB_MAX * sizeof(aud_msg_cb_node_t));
        if (rx_tx_cb_mem_pool == NULL) {
            platform_assert("AUDIO CCNI MSG TX CB malloc mem poll failed !!", __func__, __LINE__);
            return AUDIO_MSG_STATUS_ERROR;
        }
#endif
        memset(rx_tx_cb_mem_pool, 0, RX_TX_CB_MAX * sizeof(aud_msg_cb_node_t));
    }
#endif
    aud_msg_cb_node_t *cur = tx_ack_cb_front;
    aud_msg_cb_node_t *tmp;

    if (cur == NULL) {  /* Nothing in the queue */
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_GET_CB_NODE();
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("AUDIO CCNI MSG CB TX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, gRx_Tx_cb_count, RX_TX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }

        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = tx_ack_callback;
        cb->next = NULL;
        tx_ack_cb_front = cb;
        return AUDIO_MSG_STATUS_OK;
    }

    while ((cur->next != NULL) && (ID >= cur->next->cb_info.msg_id)) {  /* Search the the closest node */
        cur = cur->next;
    }

    /* Replace or insert the node */
    if (ID == cur->cb_info.msg_id) {
        cur->cb_info.cb_func = tx_ack_callback;
        return AUDIO_MSG_STATUS_OK;
    } else {
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_ENABLE
        aud_msg_cb_node_t *cb = AUD_MSG_GET_CB_NODE();
#ifdef AIR_RX_TX_CALLBACK_MEMORY_DEDICATE_DEBUG
        DSP_AUD_MSG_LOG_I("AUDIO CCNI MSG CB TX cur:(%d) max:(%d) id(0x%x) ptr(0x%x).\r\n", 4, gRx_Tx_cb_count, RX_TX_CB_MAX, ID, cb); //debug
#endif
#else
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
#endif
        if (cb == NULL) {
            DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG CB malloc failed !!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        cb->cb_info.msg_id = ID;
        cb->cb_info.cb_func = tx_ack_callback;

        if (ID > cur->cb_info.msg_id) {
            tmp = cur->next;
            cur->next = cb;
            cb->next = tmp;
        } else {
            tmp = cur;
            cur = cb;
            cb->next = tmp;
        }

        /*  Updated the front of queue pointer */
        if (tx_ack_cb_front->cb_info.msg_id > cb->cb_info.msg_id) {
            tx_ack_cb_front = cb;
        }
    }

    return AUDIO_MSG_STATUS_OK;
}

void aud_msg_ack(mcu2dsp_audio_msg_t ID, uint8_t from_ISR)
{
    if (ID == MSG_DSP_NULL_REPORT) {
        return;
    }
    ID |= 0x8000;//ACK_CCNI_BIT;
    hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = ID << 16;
    aud_msg_tx_handler(msg, 0, from_ISR);
}

aud_msg_status_t aud_msg_init(void)
{
    int ret = 0;
    vSemaphoreCreateBinary(xAud_TxMsgSem);
    if (xAud_TxMsgSem == NULL) {
        platform_assert("Audio TX_Msg_Sem init failed", __func__, __LINE__);
        return AUDIO_MSG_STATUS_ERROR;
    }
    vSemaphoreCreateBinary(xAud_TxTaskSem);
    if (xAud_TxTaskSem == NULL) {
        platform_assert("Audio TX_Task_Sem init failed", __func__, __LINE__);
        return AUDIO_MSG_STATUS_ERROR;
    }
    vSemaphoreCreateBinary(xAud_RxTaskSem);
    if (xAud_RxTaskSem == NULL) {
        platform_assert("Audio xAud_RxTaskSem init failed", __func__, __LINE__);
        return AUDIO_MSG_STATUS_ERROR;
    }

#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
    vSemaphoreCreateBinary(xAud_Nonrealtime_RxTaskSem);
    if (xAud_Nonrealtime_RxTaskSem == NULL) {
        platform_assert("[NONREALTIME_RX] Audio xAud_Nonrealtime_RxTaskSem init failed", __func__, __LINE__);
        return AUDIO_MSG_STATUS_ERROR;
    }
#endif

    ret = msg_tx_task_init();
    if (ret == 0) {
        return AUDIO_MSG_STATUS_ERROR;
    }

    ret = msg_rx_task_init();
    if (ret == 0) {
        return AUDIO_MSG_STATUS_ERROR;
    }

#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
    ret = msg_nonrealtime_rx_task_init();
    if (ret == 0) {
        return AUDIO_MSG_STATUS_ERROR;
    }
#endif

    hal_ccni_init();
    memset(&ccni_memo_tx, 0, sizeof(ccni_memo_t));
    memset(&ccni_memo_rx, 0, sizeof(ccni_memo_t));
#ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
    memset(&ccni_memo_nonrealtime_rx, 0, sizeof(ccni_memo_t));
#endif

    //write core status as active here to notify cm4 that dsp ccni is ready. (originally write after system init done)
    hal_core_status_write(HAL_CORE_DSP0, HAL_CORE_ACTIVE);
    DSP_AUD_MSG_LOG_I("hal_core_status_write done in aud_msg_init.\n", 0);
    return AUDIO_MSG_STATUS_OK;
}

// DSP send msg by-self to do in rx task.
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void aud_msg_rx_handler_virtual(void *msg)
{
    SemaphoreHandle_t Sem                     = xAud_RxTaskSem;
    uint32_t          Caller                  = CALLER_RX_HDR;
    volatile          aud_msg_node_t *q_front = rx_q_front;
    uint32_t          Queue                   = AUDIO_MSG_RX;
    uint32_t savedmask;
    uint32_t *pMsg = (uint32_t *)msg;
    if(!(pMsg[0] & 0x20000000)){ /* If ccni_message[0] bit 29 = 0, it stands for an Non-Realtime Level CCNI */
        #ifdef AIR_AUDIO_NONREALTIME_RX_ENABLE
        Sem = xAud_Nonrealtime_RxTaskSem;
        Caller = CALLER_NONREALTIME_RX_HDR;
        q_front = nonrealtime_rx_q_front;
        Queue = AUDIO_MSG_NONREALTIME_RX;
        #endif
    }
    aud_msg_node_t *rx = (aud_msg_node_t *)pvPortMalloc(sizeof(aud_msg_node_t));
    if (rx == NULL) {
        DSP_AUD_MSG_LOG_E("AUDIO CCNI MSG malloc failed !!\r\n", 0);
    }
    rx->msg.ccni_message[0] = pMsg[0];
    rx->msg.ccni_message[1] = pMsg[1];

    if (!q_front) { //rx_q_front, CALLER_RX_HDR, xAud_RxTaskSem
        audio_sem_give(Caller, Sem, true); //may fail to give sem because rx task is handling a msg right now, it's false alarm.
    }
    hal_nvic_save_and_set_interrupt_mask(&savedmask);
    msg_enqueue(rx, Queue); //AUDIO_MSG_RX
    hal_nvic_restore_interrupt_mask(savedmask);
}
