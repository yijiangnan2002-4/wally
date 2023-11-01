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

#ifndef __BT_SINK_SRV_UTILS_H__
#define __BT_SINK_SRV_UTILS_H__
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOSConfig.h"

#if defined(MTK_DEBUG_LEVEL_INFO)
#define __BT_SINK_SRV_DEBUG_INFO__
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define bt_sink_srv_assert configASSERT

#ifdef __BT_SINK_SRV_DEBUG_INFO__
#define bt_sink_srv_report(_message,...) LOG_I(sink_srv, (_message), ##__VA_ARGS__)
#define bt_sink_srv_report_id(_message, art_cnt,...) LOG_MSGID_I(sink_srv, _message, art_cnt, ##__VA_ARGS__)
#define bt_sink_srv_music_report(_message,...) LOG_I(sink_music, (_message), ##__VA_ARGS__)
#else
#define bt_sink_srv_report(_message,...);
#define bt_sink_srv_report_id(_message, art_cnt,...);
#define bt_sink_srv_music_report(_message,...);
#endif

void *bt_sink_srv_memory_alloc(uint16_t size);

void bt_sink_srv_memory_free(void *point);

void *bt_sink_srv_memset(void *ptr, int32_t value, uint32_t num);

void *bt_sink_srv_memcpy(void *dest, const void *src, uint32_t size);

int32_t bt_sink_srv_memcmp(const void *dest, const void *src, uint32_t count);

uint32_t bt_sink_srv_strlen(char *string);

char *bt_sink_srv_strncpy(char *dest, const char *src, uint32_t size);

int32_t bt_sink_srv_strnmp(const char *dest, const char *src, uint32_t size);

uint32_t bt_sink_srv_util_atoi(const uint8_t *a, uint8_t len);

bool bt_sink_srv_util_endian_order_swap(uint8_t *dest, const uint8_t *src, uint8_t len);

void bt_sink_srv_mutex_lock(void);

void bt_sink_srv_mutex_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __BT_SINK_SRV_UTILS_H__ */
