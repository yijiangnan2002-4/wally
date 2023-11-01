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

#ifndef __BT_SOURCE_SRV_UTILS_H__
#define __BT_SOURCE_SRV_UTILS_H__
#include <stdint.h>
#include <stdbool.h>
#include <syslog.h>
#include "FreeRTOSConfig.h"
#include "bt_type.h"
#include "syslog.h"

#define AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE 1

#define bt_source_srv_assert configASSERT

#define BT_SOURCE_SRV_NODE_FRONT   0x0
#define BT_SOURCE_SRV_NODE_BACK    0x1
typedef uint32_t bt_source_srv_linknode_position_t;

typedef struct _bt_source_srv_linknode_t {
    struct _bt_source_srv_linknode_t *front;
} bt_source_srv_linknode_t;

void *bt_source_srv_memset(void *ptr, int32_t value, uint32_t num);

void *bt_source_srv_memcpy(void *dest, const void *src, uint32_t size);

int32_t bt_source_srv_memcmp(const void *dest, const void *src, uint32_t count);

void *bt_source_srv_memory_alloc(uint32_t size);

void bt_source_srv_memory_free(void *point);

uint32_t bt_source_srv_strlen(char *string);

void bt_source_srv_mutex_lock(void);

void bt_source_srv_mutex_unlock(void);

void bt_source_srv_linknode_insert_node(bt_source_srv_linknode_t *head, bt_source_srv_linknode_t *dest, bt_source_srv_linknode_position_t position);

bt_source_srv_linknode_t *bt_source_srv_linknode_remove_node(bt_source_srv_linknode_t *head, bt_source_srv_linknode_t *dest);
#endif
