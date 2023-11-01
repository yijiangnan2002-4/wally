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
#ifndef __LSM6DSO_H__
#define __LSM6DSO_H__

#include "hal.h"
#include "hal_platform.h"
#include "bsp_imu_sensor.h"
#include "bsp_imu_sensor_common.h"
#include "lsm6dso_reg.h"


typedef struct {
    float       temperature;
    float       f_sys_tm_ms;
    uint32_t    pre_sys_tick;
    uint32_t    system_tick;
    uint32_t    timestamp_us;
    uint32_t    packet_len;
    bsp_imu_sensor_callback_t user_callback;
    void        *user_data;
    lsm6dso_odr_xl_t    accr_odr;
    lsm6dso_odr_g_t     gyro_odr;
} lsm6dso_privte_info_t;


typedef union {
    int16_t i16bit[3];
    uint8_t u8bit[6];
} axis3bit16_t;


int      lsm6dso_init(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_init_config_t *config);
int      lsm6dso_config(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_config_t *config);
int      lsm6dso_deinit(bsp_imu_sensor_handle_t handle);
int      lsm6dso_read_format_data(bsp_imu_sensor_data_t *user_buff, uint32_t user_buff_len);
uint32_t lsm6dso_get_frame_cnt();
int      lsm6dso_self_test(bsp_imu_sensor_enable_t sensor_en);
void     lsm6dso_eint_irq_handle(bsp_imu_sensor_handle_t handle);
#endif
