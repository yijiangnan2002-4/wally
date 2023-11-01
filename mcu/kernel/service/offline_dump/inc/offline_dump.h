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

#ifndef __OFFLINE_DUMP_H__
#define __OFFLINE_DUMP_H__

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "offline_dump_port.h"
#include "exception_handler.h"

/* system build version & build time */
extern char build_date_time_str[];
extern char sw_verno_str[];

/* porting define */
#define OFFLINE_FOTA_ADDRESS             offline_dump_query_FOTA_address()
#define OFFLINE_FOTA_RESERVE             offline_dump_query_FOTA_reserve()
#define SERIAL_FLASH_BLOCK_SIZE          4096

#define OFFLINE_BUILD_TIME_COUNT         1
#define OFFLINE_BUILD_INFO_SIZE          48
#define OFFLINE_BUILD_INFO_TOTAL_SIZE    OFFLINE_BUILD_TIME_COUNT * OFFLINE_BUILD_INFO_SIZE

#define OFFLINE_DUMP_CUST_SDK_VERSION_LEN   64

/* offline dump cell header struct */
typedef struct {
    uint8_t status;
    uint8_t rtc_status;
    uint8_t reserved[2];
    uint16_t header_checksum;
    uint16_t header_size;
    uint8_t dump_version;
    uint8_t payload_type;
    uint8_t sdk_ver_cnt;
    uint8_t build_time_cnt;
    uint8_t magic_name[32];
    uint32_t payload_size;
    uint32_t payload_real_size;
    uint32_t seq_number;
    uint8_t rtc_time[8];
    uint8_t sdk_version[OFFLINE_BUILD_INFO_SIZE];
    uint8_t build_time[OFFLINE_BUILD_INFO_TOTAL_SIZE];
    uint8_t customized_sdk_version[OFFLINE_DUMP_CUST_SDK_VERSION_LEN];
    uint32_t timestamp;
} offline_dump_header_t;

/* Offline dump version */
#define OFFLINE_REGION_VERSION                    0x01

/* exception log information */
#define OFFLINE_REGION_EXCEPTION_LOG_BASE_ADDR      (OFFLINE_FOTA_ADDRESS)
#define OFFLINE_REGION_EXCEPTION_LOG_CELL_COUNT     1
#define OFFLINE_REGION_EXCEPTION_LOG_CELL_SIZE      (8 * SERIAL_FLASH_BLOCK_SIZE)
#define OFFLINE_REGION_EXCEPTION_LOG_REGION_SIZE    (OFFLINE_REGION_EXCEPTION_LOG_CELL_COUNT * OFFLINE_REGION_EXCEPTION_LOG_CELL_SIZE)

/* offline log information */
#define OFFLINE_REGION_OFFLINE_LOG_BASE_ADDR        (OFFLINE_REGION_EXCEPTION_LOG_BASE_ADDR + OFFLINE_REGION_EXCEPTION_LOG_REGION_SIZE)
#define OFFLINE_REGION_OFFLINE_LOG_CELL_COUNT       32
#define OFFLINE_REGION_OFFLINE_LOG_CELL_SIZE        (1 * SERIAL_FLASH_BLOCK_SIZE)
#define OFFLINE_REGION_OFFLINE_LOG_REGION_SIZE      (OFFLINE_REGION_OFFLINE_LOG_CELL_COUNT * OFFLINE_REGION_OFFLINE_LOG_CELL_SIZE)

/* mini dump information */
#define OFFLINE_REGION_MINI_DUMP_BASE_ADDR          (OFFLINE_REGION_OFFLINE_LOG_BASE_ADDR + OFFLINE_REGION_OFFLINE_LOG_REGION_SIZE)
#define OFFLINE_REGION_MINI_DUMP_CELL_COUNT         1
#if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3) 
    #define OFFLINE_REGION_MINI_DUMP_CELL_SIZE      ((OFFLINE_FOTA_RESERVE - OFFLINE_REGION_EXCEPTION_LOG_REGION_SIZE - OFFLINE_REGION_OFFLINE_LOG_REGION_SIZE) / OFFLINE_REGION_MINI_DUMP_CELL_COUNT)
#else
    #define OFFLINE_REGION_MINI_DUMP_CELL_SIZE      (2 * SERIAL_FLASH_BLOCK_SIZE)
#endif
#define OFFLINE_REGION_MINI_DUMP_REGION_SIZE        (OFFLINE_REGION_MINI_DUMP_CELL_COUNT * OFFLINE_REGION_MINI_DUMP_CELL_SIZE)

typedef void (*offline_callback_t)(void);

/* offline dump region enum */
typedef enum {
    OFFLINE_REGION_EXCEPTION_LOG = 0,
    OFFLINE_REGION_MINI_DUMP,
    OFFLINE_REGION_OFFLINE_LOG,
    OFFLINE_REGION_MAX,
} offline_dump_region_type_t;

/* offline dump status */
typedef enum {
    OFFLINE_STATUS_OK,                          /**<  status ok*/
    OFFLINE_STATUS_ERROR,                       /**<  status error*/
    OFFLINE_STATUS_ERROR_PARAMETER,             /**<  status error parameter*/
    OFFLINE_STATUS_ERROR_NOT_INIT,              /**<  status uninitialized*/
    OFFLINE_STATUS_ERROR_NOT_ALLOW,             /**<  status of flash access error*/
    OFFLINE_STATUS_ERROR_BUSY,                  /**<  status of busy*/
    OFFLINE_STATUS_CELL_INVALID,                /**<  status of invalid cell*/
    OFFLINE_STATUS_CELL_NOT_INVALID,            /**<  status of not invalid cell*/
} offline_dump_status_t;

offline_dump_status_t offline_dump_region_init(void);

offline_dump_status_t offline_dump_region_alloc(offline_dump_region_type_t region_type, uint32_t *p_start_addr);

offline_dump_status_t offline_dump_region_write(offline_dump_region_type_t region_type, uint32_t curr_addr, uint8_t *data, uint32_t length);

offline_dump_status_t offline_dump_region_write_end(offline_dump_region_type_t region_type, uint32_t total_length);

offline_dump_status_t offline_dump_region_read(offline_dump_region_type_t region_type, uint32_t curr_addr, uint8_t *buf, uint32_t length);

offline_dump_status_t offline_dump_region_query_seq_range(offline_dump_region_type_t region_type, uint32_t *p_min_seq, uint32_t *p_max_seq);

offline_dump_status_t offline_dump_region_query_by_seq(offline_dump_region_type_t region_type, uint32_t seq, uint32_t *p_start_addr, uint32_t *p_total_length);

offline_dump_status_t offline_dump_region_query_cell_size(offline_dump_region_type_t region_type, uint32_t *p_cell_size);

offline_dump_status_t offline_dump_region_query_cell_valid_size(offline_dump_region_type_t region_type, uint32_t *p_valid_size);

offline_dump_status_t offline_dump_region_mark_invalid_cell(offline_dump_region_type_t region_type, uint32_t seq);

offline_dump_status_t offline_dump_region_query_cell_invalid_state(offline_dump_region_type_t region_type, uint32_t seq);

offline_dump_status_t offline_dump_register_callback(offline_dump_region_type_t region_type, offline_callback_t user_callback);

offline_dump_status_t offline_dump_region_write_with_race_header(exception_log_type_t log_type, uint32_t *curr_addr, uint8_t *data, uint32_t length);

offline_dump_status_t offline_dump_fill_customer_sdk_version(uint8_t *data, uint32_t length);

#endif

