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

#ifndef __SYSLOG_NVKEY_STRUCT_H__
#define __SYSLOG_NVKEY_STRUCT_H__

#include <stdint.h>
#include <stdbool.h>

#define SYSLOG_NVKEY_MAGIC_NUMBER   0x5A

/* share syslog variable define */
#define PORT_SYSLOG_MAX_CPU_NUMBER 16

/* module and filter define */
#define PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER 200
#define PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE (PORT_SYSLOG_MAX_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER) // 16 * 2 + 200 = 232
#define PORT_SYSLOG_MAX_MODULE_FILTER_STATUS_SIZE (PORT_SYSLOG_MAX_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER)  //  16 * 2 + 200 = 232

typedef struct {
    uint8_t cpu_id;
    uint8_t module_number;
} cpu_module_filter_info_t;

typedef struct {
    uint8_t cpu_log_switch;
    uint8_t cpu_log_level;
} cpu_filter_info_t;

typedef struct {
    struct {
        uint8_t port: 7;
        uint8_t dump_enable: 1;
    } syslog_port;
    struct {
        uint8_t cpu0_level: 2;
        uint8_t cpu1_level: 2;
        uint8_t cpu0_switch: 2;
        uint8_t cpu1_switch: 2;
    } syslog_level;
    struct {
        uint8_t high_br_enable: 1;
        uint8_t reserve: 7;
    } baudrate;
    uint8_t magic_number;
} syslog_nvkey_cfg_t;

typedef struct {
    cpu_module_filter_info_t syslog_module_filter_number[PORT_SYSLOG_MAX_CPU_NUMBER];
    uint8_t syslog_module_filter_status[PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER];
} cpu_module_filter_status_t;

typedef struct {
    uint8_t syslog_module_filter_magic;
    cpu_module_filter_status_t cpu_module_filter_status;
} syslog_nvkey_module_filter_t;

#endif

