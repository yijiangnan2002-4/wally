/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

/* if it is iic interface,please define this marco */
#ifndef __AIRO_PLATFORM_BUS_H__
#define __AIRO_PLATFORM_BUS_H__

#include <hal.h>
#include <hal_platform.h>

#if 0
#define log_sensor_info(_message, cnt, ...)      log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
#define log_sensor_warn(_message, cnt, ...)      log_hal_msgid_warning(_message,cnt, ##__VA_ARGS__)
#define log_sensor_error(_message,cnt, ...)      log_hal_msgid_error(_message,cnt,   ##__VA_ARGS__)
#else
#define log_sensor_bus_info(_message, cnt, ...)      //printf(_message, ##__VA_ARGS__);
#define log_sensor_bus_warn(_message, cnt, ...)      printf(_message, ##__VA_ARGS__);
#define log_sensor_bus_error(_message, cnt, ...)     printf(_message, ##__VA_ARGS__);
#endif



typedef void (*airo_bus_callback)(void *);


typedef union {
    hal_i2c_config_t            i2c_config;
    hal_spi_master_config_t      spi_config;
} platform_bus_init_config_t;

typedef struct {
    uint8_t     dev_addr;
    uint8_t     reg_addr;
    uint8_t    *buff;
    uint32_t     size;
} platform_transfer_config_t;

typedef struct {
    uint8_t     dev_addr;
    uint8_t     reg_addr;
    uint8_t    *buff;
    uint32_t     size;
    airo_bus_callback    call_back;
    void               *user_data;
} platform_transfer_dma_config_t;

typedef struct {
    airo_bus_callback    call_back;
    void               *user_data;
} platform_bus_private_info_t;

int platform_bus_init(uint32_t handle, platform_bus_init_config_t *config);
int platform_bus_read(uint32_t handle, platform_transfer_config_t *config);
int platform_bus_write(uint32_t handle, platform_transfer_config_t *config);
int platform_bus_read_dma(uint32_t handle, platform_transfer_dma_config_t *config);
int platform_bus_write_dma(uint32_t handle, platform_transfer_dma_config_t *config);
int platform_bus_deinit(uint32_t handle);


#endif
