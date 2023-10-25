/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#ifndef _BSP_HEAD_TRACKER_IMU_COMMON_H_
#define _BSP_HEAD_TRACKER_IMU_COMMON_H_
#include "hal.h"
#include "hal_platform.h"
#include "syslog.h"
#include "bsp_head_tracker_imu.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"

#define bsp_head_tracker_log_d(_message, art_cnt,...)     //LOG_MSGID_I(BSP_HEAD_TRACKER_IMU, _message, art_cnt, ##__VA_ARGS__)
#define bsp_head_tracker_log_i(_message, art_cnt,...)     LOG_MSGID_I(BSP_HEAD_TRACKER_IMU, _message, art_cnt, ##__VA_ARGS__)
#define bsp_head_tracker_log_w(_message, art_cnt,...)     LOG_MSGID_W(BSP_HEAD_TRACKER_IMU, _message, art_cnt, ##__VA_ARGS__)
#define bsp_head_tracker_log_e(_message, art_cnt,...)     LOG_MSGID_E(BSP_HEAD_TRACKER_IMU, _message, art_cnt, ##__VA_ARGS__)

#define BSP_HEAD_TRACKER_MAGIC_ID   0x87


typedef enum {
    HEAD_TRACKER_TIMER_CREATE = 0,
    HEAD_TRACKER_TIMER_START,
    HEAD_TRACKER_TIMER_STOP,
    HEAD_TRACKER_TIMER_RESET,
    HEAD_TRACKER_TIMER_MODIFY,
    HEAD_TRACKER_TIMER_DELETE,
    HEAD_TRACKER_TIMER_MAX,
} head_tracker_timer_cmd_t;

typedef enum {
    HEAD_TRACKER_SERVICE_INIT = 0,
    HEAD_TRACKER_SERVICE_DEINIT,
    HEAD_TRACKER_SERVICE_ENABLE,
    HEAD_TRACKER_SERVICE_PROCESS,
    HEAD_TRACKER_SERVICE_DISABLE,
    HEAD_TRACKER_SERVICE_CHGTM,
    HEAD_TRACKER_SERVICE_STOP,
    HEAD_TRACKER_SERVICE_MAX
} head_tracker_imu_service_type_t;

#define MAX_FIFO_SIZE  8

typedef struct {
    uint8_t   head;
    uint8_t   tail;
    int   data[MAX_FIFO_SIZE];
} airo_queue_t;


typedef enum {
    I2C_FIFO_STATUS_OK = 0,
    I2C_FIFO_STATUS_ERROR_INVALID_PARA = -1,
    I2C_FIFO_STATUS_ERROR_FULL     = -2,
    I2C_FIFO_STATUS_ERROR_EMPTY    = -3
} queue_status_t;

typedef void (*head_track_lib_service_handle_func_t)(head_tracker_imu_service_type_t service_id);

void head_track_lib_service_init(head_track_lib_service_handle_func_t func);
void head_track_lib_service(head_tracker_imu_service_type_t type, uint32_t option);
void head_track_lib_service_deinit();
#endif
