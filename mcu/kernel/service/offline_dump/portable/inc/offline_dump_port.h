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

#ifndef __OFFLINE_DUMP_PORT_H__
#define __OFFLINE_DUMP_PORT_H__

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <time.h>

typedef struct {
    uint16_t milli_sec;             /**< millisecond               - [0,999] */
    uint8_t sec;                    /**< Seconds after minutes     - [0,59]  */
    uint8_t min;                    /**< Minutes after the hour    - [0,59]  */
    uint8_t hour;                   /**< Hours after midnight      - [0,23]  */
    uint8_t day;                    /**< Day of the month          - [1,31]  */
    uint8_t mon;                    /**< Months                    - [1,12]  */
    uint8_t week;                   /**< Days in a week            - [0,6]   */
    uint8_t year;                   /**< Years                     - [0,127] */
} offline_dump_rtc_t;

extern const uint8_t PORT_FLASH_BLOCK_SIZE_4K;
extern const uint8_t PORT_FLASH_BLOCK_SIZE_32K;
extern const uint8_t PORT_FLASH_BLOCK_SIZE_64K;

uint32_t offline_dump_query_FOTA_address(void);
uint32_t offline_dump_query_FOTA_reserve(void);
bool offline_dump_region_is_busy(void);
bool offline_dump_query_region_info(uint8_t region_type, uint32_t *base_address, uint32_t *region_size, uint32_t *cell_size);
bool offlined_dump_core_is_exception(void);
int32_t offlined_dump_feed_wdt(void);
bool offline_dump_query_rtc_time(offline_dump_rtc_t *time);
bool offline_dump_set_rtc_time_unix(time_t timestamp);
bool offline_dump_query_unix_time(time_t *timestamp);
uint32_t offline_dump_query_current_timestamp(void);

/* flash API */
int32_t PORT_FLASH_READ(uint32_t address, uint8_t *buffer, uint32_t length);
int32_t PORT_FLASH_WRITE(uint32_t address, const uint8_t *data, uint32_t length);
int32_t PORT_FLASH_ERASE(uint32_t address, uint8_t block_size);

/* log API */
extern void offline_dump_log_port_info(const char *message, ...);
extern void offline_dump_log_port_warning(const char *message, ...);
extern void offline_dump_log_port_error(const char *message, ...);
extern void offline_dump_log_port_msgid_info(const char *message, uint32_t arg_cnt, ...);
extern void offline_dump_log_port_msgid_warning(const char *message, uint32_t arg_cnt, ...);
extern void offline_dump_log_port_msgid_error(const char *message, uint32_t arg_cnt, ...);

/* lib msgid log API */
#define OFFLINE_DUMP_LIB_I(fmt, arg...)               offline_dump_log_port_info(fmt, ##arg)
#define OFFLINE_DUMP_LIB_W(fmt, arg...)               offline_dump_log_port_warning(fmt, ##arg)
#define OFFLINE_DUMP_LIB_E(fmt, arg...)               offline_dump_log_port_error(fmt, ##arg)
#define OFFLINE_DUMP_LIB_MSGID_I(fmt, cnt, arg...)    offline_dump_log_port_msgid_info(fmt, cnt, ##arg)
#define OFFLINE_DUMP_LIB_MSGID_W(fmt, cnt, arg...)    offline_dump_log_port_msgid_warning(fmt, cnt, ##arg)
#define OFFLINE_DUMP_LIB_MSGID_E(fmt, cnt, arg...)    offline_dump_log_port_msgid_error(fmt, cnt, ##arg)
/* lib msgid log */
extern const char offline_dump_001[];
extern const char offline_dump_002[];
#endif

