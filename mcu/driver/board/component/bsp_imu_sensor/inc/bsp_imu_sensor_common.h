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

#ifndef __BSP_IMU_SENSOR_COMMON_H__
#define __BSP_IMU_SENSOR_COMMON_H__
#include <stdint.h>

#define        BSP_SENSOR_MAGIC_ID              (0x47)
#define        BSP_SENSOR_MAGICID_OFS           (24)
#define        BSP_SENSOR_BUS_TYPE_OFS          (20)
#define        BSP_SENSOR_BUS_PORT_OFS          (16)
#define        BSP_SENSOR_VENDOR_OFS            (12)
#define        BSP_SENSOR_USER_ID_OFS           (8)
#define        BSP_SENSOR_EINT_NUM_OFS          (0)

#define        BSP_SENSOR_GET_MAGICID(handle)           ((handle >> BSP_SENSOR_MAGICID_OFS)  & 0xFF)
#define        BSP_SENSOR_GET_BUS_TYPE(handle)          ((handle >> BSP_SENSOR_BUS_TYPE_OFS) & 0xF)
#define        BSP_SENSOR_GET_BUS_PORT(handle)          ((handle >> BSP_SENSOR_BUS_PORT_OFS) & 0xF)
#define        BSP_SENSOR_GET_VENDOR(handle)            ((handle >> BSP_SENSOR_VENDOR_OFS)   & 0xF)
#define        BSP_SENSOR_GET_USER_ID(handle)           ((handle >> BSP_SENSOR_USER_ID_OFS)  & 0xF)
#define        BSP_SENSOR_GET_EINT_NUM(handle)          ((handle >> BSP_SENSOR_EINT_NUM_OFS)  & 0xFF)

#define        BSP_SENSOR_SET_MAGICID(handle, type)         (handle |= ((type   & 0xFF) << BSP_SENSOR_MAGICID_OFS))
#define        BSP_SENSOR_SET_BUS_TYPE(handle, type)        (handle |= ((type   & 0xF) << BSP_SENSOR_BUS_TYPE_OFS))
#define        BSP_SENSOR_SET_BUS_PORT(handle, port)        (handle |= ((port   & 0xF) << BSP_SENSOR_BUS_PORT_OFS))
#define        BSP_SENSOR_SET_VENDOR(handle, vendor)        (handle |= ((vendor & 0xF) << BSP_SENSOR_VENDOR_OFS))
#define        BSP_SENSOR_SET_USER_ID(handle, user)         (handle |= ((user   & 0xF) << BSP_SENSOR_USER_ID_OFS))
#define        BSP_SENSOR_SET_EINT_NUM(handle, user)        (handle |= ((user   & 0xFF) << BSP_SENSOR_EINT_NUM_OFS))

#if 1
#define log_imu_sensor_d(_message, cnt, ...)            LOG_MSGID_D(bsp_imu_sensor, _message, cnt, ##__VA_ARGS__)
#define log_imu_sensor_i(_message, cnt, ...)            LOG_MSGID_I(bsp_imu_sensor, _message, cnt, ##__VA_ARGS__)
#define log_imu_sensor_w(_message, cnt, ...)            LOG_MSGID_W(bsp_imu_sensor, _message, cnt, ##__VA_ARGS__)
#define log_imu_sensor_e(_message,cnt, ...)             LOG_MSGID_E(bsp_imu_sensor, _message, cnt, ##__VA_ARGS__)
#else
#define log_imu_sensor_d(_message, cnt, ...)            printf(_message, ##__VA_ARGS__);
#define log_imu_sensor_i(_message, cnt, ...)            printf(_message, ##__VA_ARGS__);
#define log_imu_sensor_w(_message, cnt, ...)            printf(_message, ##__VA_ARGS__);
#define log_imu_sensor_e(_message, cnt, ...)            printf(_message, ##__VA_ARGS__);
#endif

typedef struct {
    bool   imu_init_status;
    bool   imu_is_intx_mode; /* IMU use INTx to notify hosts or not */
} private_imu_sensor_info_t;



#endif // __BSP_MULTI_AXIS_SENSOR_COMMON_H__

