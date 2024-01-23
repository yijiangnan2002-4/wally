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
  * File: app_bolt_poc_utility.c
  *
  * Description: This file is the utility for bolt poc.
  *
  * Note: todo.
  *
  */

#include "app_bolt_poc_utility.h"
#include "FreeRTOS.h"


void *app_bolt_poc_memory_alloc(uint16_t size)
{
    void *memory = (void *)pvPortMalloc(size);
    if (NULL != memory) {
        memset(memory, 0, size);
    }
    return memory;
}

void app_bolt_poc_memory_free(void *point)
{
    if (point) {
        vPortFree(point);
    }
}

bool app_bolt_poc_cmp_addr(const bt_addr_t *addr1, const bt_addr_t *addr2)
{
    if (NULL == addr1 || NULL == addr2) {
        return false;
    }

    if (addr1->addr == NULL || addr2->addr == NULL) {
        return false;
    }

    return (0 == memcmp(addr1->addr, addr2->addr, BT_BD_ADDR_LEN));
}

void app_bolt_poc_cpy_addr(bt_addr_t *dest, const bt_addr_t *src)
{
    if (NULL == src || NULL == dest) {
        return;
    }

    memcpy(dest, src, sizeof(bt_addr_t));
}

void app_bolt_poc_init_addr(bt_addr_t *addr)
{
    if (NULL == addr) {
        return;
    }

    memset(addr, 0, sizeof(bt_addr_t));
}

bool app_bolt_poc_addr_not_empty(const bt_addr_t *addr)
{
    if (NULL == addr) {
        return false;
    }

    uint8_t i = 0;
    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        if (addr->addr[i] != 0x00) {
            return true;
        }
    }
    return false;
}

void bitcpy(void* dest_, int dest_ofs, const void* src_, int src_ofs, int bit_cnt_)
{
    static const uint8_t bit_mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
#define BIT_SET(p, i, b) ((b) ? ((*(p))|=(bit_mask[i])) : ((*(p))&=(~(bit_mask[i]))))
    const uint8_t* src = (uint8_t*)src_ + (src_ofs / 8);
    uint8_t* dst = (uint8_t*)dest_ + (dest_ofs / 8);

    int s_i = src_ofs % 8;
    int d_i = dest_ofs % 8;

    int bit_cnt = bit_cnt_;
    while (bit_cnt--)
    {
        BIT_SET(dst, d_i, ((*src) & bit_mask[s_i]) ? 1 : 0);

        if (++s_i == 8) {
            src++;
            s_i = 0;
        }

        if (++d_i == 8) {
            dst++;
            d_i = 0;
        }
    }
}

int8_t get_int8(const uint8_t *buffer, int16_t start, int16_t count)
{
    int8_t ret = 0;

    if (count % 8 == 0)
    {
        memcpy(&ret, &buffer[start / 8], count / 8);
    }
    else
    {
        bitcpy(&ret, 0, buffer, start, count);
    }

    return ret;
}

int8_t get_sign_int8(const uint8_t *buffer, int16_t start, int16_t count)
{
    int8_t ret = 0;

    if (count % 8 == 0)
    {
        memcpy(&ret, &buffer[start / 8], count / 8);
    }
    else
    {
        bitcpy(&ret, 0, buffer, start, count);
    }

    int8_t dif = 8 - count;
    if (dif > 0)
    {
        uint8_t sign_bit = (ret >> (count - 1)) & 0x01;
        if (sign_bit)
        {
            int8_t i = 0;
            for (i = 0; i < dif; i++)
            {
                ret |= (1 << (count + i));
            }
        }
    }

    return ret;
}

int16_t get_int16(const uint8_t *buffer, int16_t start, int16_t count)
{
    int16_t ret = 0;

    if (count % 8 == 0)
    {
        memcpy(&ret, &buffer[start / 8], count / 8);
    }
    else
    {
        bitcpy(&ret, 0, buffer, start, count);
    }

    return ret;
}

int16_t get_sign_int16(const uint8_t *buffer, int16_t start, int16_t count)
{
    int16_t ret = 0;

    if (count % 8 == 0)
    {
        memcpy(&ret, &buffer[start / 8], count / 8);
    }
    else
    {
        bitcpy(&ret, 0, buffer, start, count);
    }

    int16_t dif = 16 - count;
    if (dif > 0)
    {
        uint8_t sign_bit = (ret >> (count - 1)) & 0x01;
        if (sign_bit)
        {
            int8_t i = 0;
            for (i = 0; i < dif; i++)
            {
                ret |= (1 << (count + i));
            }
        }
    }

    return ret;
}

