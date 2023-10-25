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

#include "bt_utils.h"
#include "FreeRTOS.h"

extern void bt_os_take_stack_mutex(void);
extern void bt_os_give_stack_mutex(void);

BT_UTILS_LOG_MODULE_CREATE(log_create_module);

void       *bt_utils_memcpy(void *dest, const void *src, uint32_t size)
{
    if (NULL == dest || NULL == src || 0 == size) {
        return dest;
    }
    return memcpy(dest, src, size);
}

void       *bt_utils_memset(void *ptr, int32_t value, uint32_t num)
{
    if (NULL == ptr || 0 == num) {
        return ptr;
    }
    return memset(ptr, value, num);
}

int32_t     bt_utils_memcmp(const void *dest, const void *src, uint32_t count)
{
    if (NULL == dest || NULL == src || 0 == count) {
        return -1;
    }
    return memcmp(dest, src, count);
}

char       *bt_utils_strcpy(char *dest, const char *src)
{
    return strncpy(dest, src, strlen(src) + 1);
}

uint32_t    bt_utils_strlen(const char *string)
{
    return strlen(string);
}

void       *bt_utils_memory_alloc(uint32_t size)
{
    void *memory = (void *)pvPortMalloc(size);
    if (NULL != memory) {
        bt_utils_memset(memory, 0, size);
#ifdef MTK_SUPPORT_HEAP_DEBUG
extern void vPortUpdateBlockHeaderLR(void* addr, uint32_t lr);
        vPortUpdateBlockHeaderLR(memory, (uint32_t)__builtin_return_address(0));
#endif
    }
    return memory;
}

void        bt_utils_memory_free(void *point)
{
    if (point) {
        vPortFree(point);
    }
}

void        bt_utils_mutex_lock(void)
{
    bt_os_take_stack_mutex();
}

void        bt_utils_mutex_unlock(void)
{
    bt_os_give_stack_mutex();
}

void        bt_utils_get_address_from_string(const char *addr_str, bt_bd_addr_t *address)
{
#if 0
    if (NULL == addr_str || NULL == address) {
        return;
    }
    int16_t result = 0;
    uint32_t total_num = bt_utils_strlen(addr_str), bt_count = bt_data_len, bt_bit = 1;
    const char *temp_index = addr_str;
    bt_utils_memset(address, 0, sizeof(bt_bd_addr_t));
    while (total_num) {
        if (*temp_index <= '9' && *temp_index >= '0') {
            (uint8_t *)(address)[bt_count - 1] += ((*temp_index - '0') * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }

        } else if (*temp_index <= 'F' && *temp_index >= 'A') {
            (uint8_t *)(address)[bt_count - 1] += ((*temp_index - 'A' + 10) * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }
        } else if (*temp_index <= 'f' && *temp_index >= 'a') {
            (uint8_t *)(address)[bt_count - 1] += ((*temp_index - 'a' + 10) * (bt_bit * 15 + 1));
            if (bt_bit == 0) {
                bt_count--;
                bt_bit = 1;
            } else {
                bt_bit--;
            }
        }
        if (!bt_count) {
            break;
        }
        total_num--;
        temp_index++;
    }

    if (bt_count) {
        bt_utils_memset((uint8_t *)(address), 0, sizeof(bt_bd_addr_t));
        result = -1;
        bt_cmgr_report_id("[BT_CM][ATCI]convert fail:%d", 1, result);
    }

    if (bt_data_len == sizeof(bt_bd_addr_t)) {
        bt_cmgr_report_id("[BT_CM][ATCI]BT addr: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                          bt_data[5], bt_data[4], bt_data[3], bt_data[2], bt_data[1], bt_data[0]);
    } else if (bt_data_len == sizeof(bt_key_t)) {
        bt_cmgr_report_id("[BT_CM][ATCI]BT key: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 16,
                          bt_data[15], bt_data[14], bt_data[13], bt_data[12], bt_data[11], bt_data[10],
                          bt_data[9], bt_data[8], bt_data[7], bt_data[6], bt_data[5], bt_data[4], bt_data[3],
                          bt_data[2], bt_data[1], bt_data[0]);
    }
    return result;
#endif
}

void bt_utils_srv_linknode_insert_node(bt_utils_linknode_t *head, bt_utils_linknode_t *dest, bt_utils_linknode_position_t position)
{
    bt_utils_linknode_t *tmp = head;
    if (position == BT_UTILS_SRV_NODE_BACK) {
        while (tmp->front != NULL) {
            tmp = tmp->front;
        }
    }
    dest->front = tmp->front;
    tmp->front = dest;
}

bt_utils_linknode_t *bt_utils_srv_linknode_remove_node(bt_utils_linknode_t *head, bt_utils_linknode_t *dest)
{
    bt_utils_linknode_t *tmp;
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

bt_utils_linknode_t *bt_utils_linknode_travel_node(bt_utils_linknode_t *head, bt_utils_linknode_cmp_t func, const void *data)
{
    bt_utils_linknode_t *tmp;
    tmp = head;
    while (tmp) {
        if (false != func(tmp, data)) {
            return tmp;
        }
        tmp = tmp->front;
        if (tmp == head) {
            return NULL;
        }
    }
    return NULL;
}
