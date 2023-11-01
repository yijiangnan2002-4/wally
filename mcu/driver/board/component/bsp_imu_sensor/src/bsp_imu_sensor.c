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

#include "hal.h"
#include "hal_platform.h"
#include <string.h>
#include "bsp_imu_sensor.h"
#include "bsp_imu_sensor_common.h"
#include "lsm6dso.h"

log_create_module(bsp_imu_sensor, PRINT_LEVEL_WARNING);

static private_imu_sensor_info_t    priv_imu_data = {0};

static void bsp_imu_sensor_eintx_handler(void *user_data)
{
}

bsp_imu_sensor_handle_t    bsp_imu_sensor_init(uint32_t vendor, bsp_imu_sensor_init_config_t *config)
{
    uint32_t handle = 0;
    int      ret = 0;

    if (config == NULL) {
        log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_init fail: null para", 0);
        return 0;
    }
    if(priv_imu_data.imu_init_status == 0) {
        memset(&priv_imu_data, 0, sizeof(priv_imu_data));
    }
    /*set handle*/
    BSP_SENSOR_SET_MAGICID(handle,  BSP_SENSOR_MAGIC_ID);
    BSP_SENSOR_SET_VENDOR(handle,   vendor);
    BSP_SENSOR_SET_BUS_TYPE(handle, config->bus_type);
    BSP_SENSOR_SET_BUS_PORT(handle, config->bus_port);
    BSP_SENSOR_SET_EINT_NUM(handle, config->eint_num);
    switch (vendor) {
        case IMU_LSM6DSO:   ret = lsm6dso_init(handle, config); break;
        default:  break;
    }
    if(ret != BSP_IMU_SENSOR_OK){
        log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_init fail: sensor init error(%d)", 1, (unsigned int) ret);
        return 0;
    }
    if (config->call_back != NULL && config->eint_num < HAL_EINT_NUMBER_MAX) {
        priv_imu_data.imu_is_intx_mode = 1;
        hal_eint_init(config->eint_num, &config->eint_config);
        hal_eint_register_callback(config->eint_num, bsp_imu_sensor_eintx_handler, (void *)handle);
    }
    log_imu_sensor_i("[bsp][axis] bsp_imu_sensor_init succ: handle 0x%x, vendor %d, eint num %d", 3, handle, vendor, config->eint_num);
    return (bsp_imu_sensor_handle_t)handle;
}






bsp_imu_sensor_status_t    bsp_imu_sensor_config(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_config_t *config)
{
    int ret = 0;

    if (BSP_SENSOR_GET_MAGICID(handle) != BSP_SENSOR_MAGIC_ID) {
        return BSP_IMU_SENSOR_INVALID_HANDLE;
    }
    if (config == NULL || config->enable == 0 || (config->enable & BSP_IMU_SENSOR_DISABLE_ALL) ||
        config->frame_threshold == 0) {
        log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_config fail: invalid parameter",0);
        return BSP_IMU_SENSOR_INVALID_PARA;
    }
    switch (BSP_SENSOR_GET_VENDOR(handle)) {
        case IMU_LSM6DSO:{
            ret = lsm6dso_config(handle, config);
            } break;

        default: {
            log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_config fail: vendor invalid(%d)", 1, (unsigned int)BSP_SENSOR_GET_VENDOR(handle));
        }
        break;
    }
    if(ret != BSP_IMU_SENSOR_OK) {
        log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_config fail: config error(%d)", 1, (unsigned int) ret);
        return ret;
    }
    if (priv_imu_data.imu_is_intx_mode == true) {
        uint8_t eint_num;
        eint_num = BSP_SENSOR_GET_EINT_NUM(handle);
        hal_eint_unmask(eint_num);
        log_imu_sensor_i("[bsp][axis] bsp_imu_sensor_config succ: unmask eint %d", 1, (unsigned int) eint_num);
    }
    return ret;
}

uint32_t bsp_imu_sensor_get_data(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_data_t *frame_buff, uint32_t frame_cnt)
{

    if (BSP_SENSOR_GET_MAGICID(handle) != BSP_SENSOR_MAGIC_ID) {
        return BSP_IMU_SENSOR_INVALID_HANDLE;
    }
    if (frame_buff == NULL || frame_cnt == 0) {
        log_imu_sensor_e("[bsp][imu] bsp_imu_sensor_get_data() fail: para invalid", 0);
        return BSP_IMU_SENSOR_INVALID_PARA;
    }
    switch (BSP_SENSOR_GET_VENDOR(handle)) {

        case IMU_LSM6DSO: {
                frame_cnt = lsm6dso_read_format_data(frame_buff, frame_cnt);
            } break;
        default: {
            log_imu_sensor_e("[bsp][imu] bsp_imu_sensor_get_data() fail: vendor invalid(%d)", 1, (unsigned int)BSP_SENSOR_GET_VENDOR(handle));
        } break;
    }
    if (priv_imu_data.imu_is_intx_mode == true) {
        uint8_t eint_num;
        eint_num = BSP_SENSOR_GET_EINT_NUM(handle);
        hal_eint_unmask(eint_num);
    }
    log_imu_sensor_i("[bsp][imu] bsp_imu_sensor_get_data() succ: frame cnt %d", 1, (int) frame_cnt);
    return frame_cnt;
}


int bsp_imu_sensor_control(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_command_t cmd, uint32_t option)
{
    int ret = BSP_IMU_SENSOR_NOT_SUPPORT;

    if (BSP_SENSOR_GET_MAGICID(handle) != BSP_SENSOR_MAGIC_ID) {
        return BSP_IMU_SENSOR_INVALID_HANDLE;
    }

    switch (cmd) {
        case BSP_IMU_SENSOR_CMD_SELFTEST:{
            ret = lsm6dso_self_test(option);
        }
        break;

        case BSP_IMU_SENSOR_CMD_GET_DEVICE_ID:{

        }
        break;

        case BSP_IMU_SENSOR_CMD_GET_FRAME_CNT:{
            ret = lsm6dso_get_frame_cnt();
        }
        break;

        case BSP_IMU_SENSOR_CMD_ENTRT_SLEEP:{

        }
        break;

        case BSP_IMU_SENSOR_CMD_EXIT_SLEEP:{

        }
        break;

        default: break;
    }
    return ret;
}



bsp_imu_sensor_status_t    bsp_imu_sensor_deinit(bsp_imu_sensor_handle_t handle)
{
    int status = 0;
    uint8_t eint_num = 0;

    if (BSP_SENSOR_GET_MAGICID(handle) != BSP_SENSOR_MAGIC_ID) {
        log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_deinit fail: invalid handle", 0);
        return BSP_IMU_SENSOR_INVALID_HANDLE;
    }
    eint_num = BSP_SENSOR_GET_EINT_NUM(handle);
    hal_eint_mask(eint_num);
    hal_eint_deinit(eint_num);
    switch (BSP_SENSOR_GET_VENDOR(handle)) {
        case IMU_LSM6DSO:
            status = lsm6dso_deinit(handle);
            break;
        default: {
            log_imu_sensor_e("[bsp][axis] bsp_imu_sensor_deinit: vendor err(%d)", 1, (unsigned int)BSP_SENSOR_GET_VENDOR(handle));
        }
        break;
    }
    log_imu_sensor_i("[bsp][axis] bsp_imu_sensor_deinit: handle %x", 1, (unsigned int) handle);

    return status;
}
