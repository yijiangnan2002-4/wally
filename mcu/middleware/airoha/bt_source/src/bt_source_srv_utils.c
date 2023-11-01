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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <string.h>
#include <stdarg.h>
#include "FreeRTOSConfig.h"
#include "bt_source_srv_utils.h"

extern void bt_os_take_stack_mutex(void);
extern void bt_os_give_stack_mutex(void);

log_create_module(source_srv, PRINT_LEVEL_INFO);

void *bt_source_srv_memset(void *ptr, int32_t value, uint32_t num)
{
    return memset(ptr, value, num);
}

void *bt_source_srv_memcpy(void *dest, const void *src, uint32_t size)
{
    return memcpy(dest, src, size);
}

int32_t bt_source_srv_memcmp(const void *dest, const void *src, uint32_t count)
{
    return memcmp(dest, src, count);
}

void *bt_source_srv_memory_alloc(uint32_t size)
{
    void *memory = (void *)pvPortMalloc(size);
    bt_source_srv_memset(memory, 0, size);
    return memory;
}

void bt_source_srv_memory_free(void *point)
{
    if (point != NULL) {
        vPortFree(point);
    }
}

uint32_t bt_source_srv_strlen(char *string)
{
    return strlen(string);
}

void bt_source_srv_mutex_lock(void)
{
    bt_os_take_stack_mutex();
}

void bt_source_srv_mutex_unlock(void)
{
    bt_os_give_stack_mutex();
}

void bt_source_srv_linknode_insert_node(bt_source_srv_linknode_t *head, bt_source_srv_linknode_t *dest, bt_source_srv_linknode_position_t position)
{
    bt_source_srv_linknode_t *tmp = head;
    if (position == BT_SOURCE_SRV_NODE_BACK) {
        while (tmp->front != NULL) {
            tmp = tmp->front;
        }
    }
    dest->front = tmp->front;
    tmp->front = dest;
}

bt_source_srv_linknode_t *bt_source_srv_linknode_remove_node(bt_source_srv_linknode_t *head, bt_source_srv_linknode_t *dest)
{
    bt_source_srv_linknode_t *tmp;
    tmp = head;
    while (tmp) {
        if (tmp->front == dest) {
            tmp->front = dest->front;
            dest->front = NULL;
            break;
        }
        tmp = tmp->front;
        if (tmp == head) {
            return NULL;
        }
    }
    return tmp;
}
