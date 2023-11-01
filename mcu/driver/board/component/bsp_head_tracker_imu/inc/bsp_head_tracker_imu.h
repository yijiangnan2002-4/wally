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

#ifndef _BSP_HEAD_TRACKER_IMU_H_
#define _BSP_HEAD_TRACKER_IMU_H_
#include "hal.h"
#include "hal_platform.h"

typedef enum {
    BSP_HEAD_TRACKER_IMU_STATUS_BUSY = -5,
    BSP_HEAD_TRACKER_IMU_STATUS_UNSUPPORT = -4,
    BSP_HEAD_TRACKER_IMU_STATUS_INVALID_HANDLE = -3,
    BSP_HEAD_TRACKER_IMU_STATUS_INVALID_PARAMETER = -2,
    BSP_HEAD_TRACKER_IMU_STATUS_ERROR = -1,
    BSP_HEAD_TRACKER_IMU_STATUS_OK = 0,
} bsp_head_tracker_imu_status_t;


typedef enum {
    BSP_HEAD_TRACKER_IDLE = 0,
    BSP_HEAD_TRACKER_INITIALIZED,
    BSP_HEAD_TRACKER_ENABLE,
} bsp_head_tracker_op_state_t;


typedef enum {
    BSP_HEAD_TRACKER_LIB_VENDOR_CYWEE,
    BSP_HEAD_TRACKER_LIB_VENDOR_AIROHA,
    BSP_HEAD_TRACKER_LIB_VENDOR_NONE
} bsp_head_tracker_lib_vendor_t;

typedef enum {
    BSP_HEAD_TRACKER_IMU_TYPE_LSM6DSOW,
} bsp_head_tracker_imu_type_t;

typedef enum {
    BSP_HEAD_TRACKER_CMD_ENABLE,
    BSP_HEAD_TRACKER_CMD_DISABLE,
    BSP_HEAD_TRACKER_CMD_SET_PERIOD,
    BSP_HEAD_TRACKER_CMD_GET_STATUS,
} bsp_head_tracker_imu_cmd_t;

typedef enum {
    BSP_HEAD_TRACKER_EVENT_RAD_VECTOT_RDY = 0,
    BSP_HEAD_TRACKER_EVENT_CALI_SUCC,
    BSP_HEAD_TRACKER_EVENT_CALI_FAIL,
    BSP_HEAD_TRACKER_EVENT_MAX,
} bsp_head_tracker_imu_event_t;

typedef uint32_t bsp_head_tracker_imu_handle_t;

typedef void (*bsp_head_tracker_imu_callback_t)(bsp_head_tracker_imu_handle_t handle, bsp_head_tracker_imu_event_t event_type, void *args);

typedef struct {
    int16_t rotation_vector_rad[3];
    int16_t rotation_vector_rad_sec[3];
} bsp_head_tracker_imu_data_t;

typedef struct {
    float spv_value[6];
} bsp_head_tracker_imu_cali_t;



typedef struct {
    bsp_head_tracker_lib_vendor_t   lib_vendor;
    bsp_head_tracker_imu_type_t     imu_vendor;
    bsp_head_tracker_imu_callback_t callback;
    bool                            spv_mode;
    bsp_head_tracker_imu_cali_t     spv_data;
} bsp_head_tracker_imu_config_t;

bsp_head_tracker_imu_handle_t bsp_head_tracker_imu_init(bsp_head_tracker_imu_config_t *config);
int bsp_head_tracker_imu_control(bsp_head_tracker_imu_handle_t handle, bsp_head_tracker_imu_cmd_t command, uint32_t option);
bsp_head_tracker_imu_status_t bsp_head_tracker_imu_deinit(bsp_head_tracker_imu_handle_t handle);

#endif
