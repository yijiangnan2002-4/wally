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
#ifdef AIR_HEAD_TRACKER_AIR_ALGO_ENABLE
#include "hal.h"
#include "hal_platform.h"
#include "bsp_imu_sensor.h"
#include "bsp_head_tracker_imu_common.h"
#include "bsp_head_tracker_imu_airoha.h"
#include "bsp_head_tracker_imu_custom_config.h"
#include <string.h>
#include "ht_api.h"

#define     MAX_HEAD_TRACKER_CALI_DATA_NUM   200
#define     ONE_DEGREE_RAD                   0.01745f /*3.1415926/180*/

#define     SENSOR_TYPE             IMU_LSM6DSO
#define     MAX_IMU_BUFF_SZ         8

#define     HEADTRACKER_STATE_INITIAL     1
#define     HEADTRACKER_STATE_ENABLE      2


static uint8_t                          s_op_state = 0;
static bsp_imu_sensor_handle_t          s_imu_sensor_handle = 0;
static bsp_head_tracker_imu_handle_t    s_headtrack_air_handle = 0;
static bsp_imu_sensor_data_t            s_frame_buff[MAX_IMU_BUFF_SZ];
static bsp_head_tracker_imu_config_t    s_init_config;
/**----------------------------------------------------------------------------**/
/**                       Private function implement                           **/
/**----------------------------------------------------------------------------**/

void airo_sensor_enable()
{
    bsp_imu_sensor_init_config_t init_config;
    bsp_imu_sensor_config_t sensor_config;
    bsp_imu_sensor_status_t status;

    init_config.bus_type = BSP_IMU_SENSOR_BUS_TYPE_I2C;
    init_config.bus_port = BSP_HEAD_TRACKER_I2C_PORT;
    init_config.bus_config.i2c_config.frequency = HAL_I2C_FREQUENCY_400K;
    init_config.slv_addr = BSP_HEAD_TRACKER_I2C_ADDR;

    init_config.eint_config.debounce_time = 0;
    init_config.eint_config.trigger_mode  = HAL_EINT_EDGE_RISING;
    init_config.eint_num  = 0xFF;
    init_config.call_back = NULL;
    init_config.user_data = NULL;

    /* configure sensor */
    sensor_config.enable = BSP_IMU_SENSOR_ACCEL_ENABLE | BSP_IMU_SENSOR_GYRO_ENABLE;
    sensor_config.accel_range = BSP_IMU_SENSOR_ACCEL_RANGE_2G;
    sensor_config.gyro_range = BSP_IMU_SENSOR_GYRO_RANGE_2000_DEG_SEC;
    sensor_config.frame_threshold = 1;
    sensor_config.sensor_odr = BSP_IMU_SENSOR_OUTPUT_DATA_RATE_50HZ;

    s_imu_sensor_handle =   bsp_imu_sensor_init(SENSOR_TYPE, &init_config);
    if (s_imu_sensor_handle == 0) {
        bsp_head_tracker_log_e("[bsp][headtracker][air] enable fail, init error (0x%x)!", 1, s_imu_sensor_handle);
        return;
    }
    status = bsp_imu_sensor_config(s_imu_sensor_handle, &sensor_config);
    if (status != 0) {
        bsp_head_tracker_log_e("[bsp][headtracker][air] enable fail, config error(%d)!", 1, status);
        return;
    }
}

void airo_sensor_disable()
{
    bsp_imu_sensor_deinit(s_imu_sensor_handle);
    s_imu_sensor_handle = 0;
}


void airo_algo_process()
{
    static uint32_t cali_cnt = 0;
    float  ftemp[6];
    int n = 0, i = 0, read_num = 0;
    int avail_num = 0, temp;
    bsp_head_tracker_imu_event_t event_type = BSP_HEAD_TRACKER_EVENT_MAX;
    void  *user_data = NULL;
    float *pRawdata = NULL;

    avail_num = bsp_imu_sensor_control(s_imu_sensor_handle, BSP_IMU_SENSOR_CMD_GET_FRAME_CNT, 0);
    bsp_head_tracker_log_d("[bsp][headtracker][air] algo process, available frame %d", 1, avail_num);
    temp = avail_num / MAX_IMU_BUFF_SZ + 1;
    pRawdata = pRawdata;
    for (n = 0; n < temp; n++) {
        read_num = bsp_imu_sensor_get_data(s_imu_sensor_handle, s_frame_buff, MAX_IMU_BUFF_SZ);
        for (i = 0; i < read_num; i++) {
            ftemp[0] = s_frame_buff[i].axis_sensor_data_t.accel_data.x;
            ftemp[1] = s_frame_buff[i].axis_sensor_data_t.accel_data.y;
            ftemp[2] = s_frame_buff[i].axis_sensor_data_t.accel_data.z;
            ftemp[3] = s_frame_buff[i].axis_sensor_data_t.gyro_data.x;
            ftemp[4] = s_frame_buff[i].axis_sensor_data_t.gyro_data.y;
            ftemp[5] = s_frame_buff[i].axis_sensor_data_t.gyro_data.z;
#ifdef HEADTRACKER_DEBUG_EN
            printf("[bsp][headtracker][air] raw sensor data, accl {%f, %f, %f}, gyro {%f, %f, %f}, timestamp %f",
                   (double) ftemp[0], (double) ftemp[1], (double) ftemp[2],
                   (double) ftemp[3], (double) ftemp[4], (double) ftemp[5],
                   (double) s_frame_buff[i].head.system_time_ms
                  );
#endif
            if (s_init_config.spv_mode == false) {
                //covert mdps to rad/s
                ftemp[3]  = ftemp[3] * ONE_DEGREE_RAD;
                ftemp[4]  = ftemp[4] * ONE_DEGREE_RAD;
                ftemp[5]  = ftemp[5] * ONE_DEGREE_RAD;
#ifdef HEADTRACKER_DEBUG_EN
                printf("[bsp][headtracker][air] after cali data, accl {%f, %f, %f}, gyro {%f, %f, %f}, timestamp %f",
                        (double) ftemp[0], (double) ftemp[1], (double) ftemp[2],
                        (double) ftemp[3], (double) ftemp[4], (double) ftemp[5],
                        (double) s_frame_buff[i].head.system_time_ms
                    );
#endif
                imu_api_process((uint32_t) s_frame_buff[i].head.system_time_ms, &ftemp[0], &ftemp[3]);
            } else {
                if ( MAX_HEAD_TRACKER_CALI_DATA_NUM > cali_cnt) {
                    ftemp[3] = ftemp[3] * ONE_DEGREE_RAD;
                    ftemp[4] = ftemp[4] * ONE_DEGREE_RAD;
                    ftemp[5] = ftemp[5] * ONE_DEGREE_RAD;
                    imu_api_calibration(cali_cnt, &ftemp[0], &ftemp[3]);
                } else {
                    imu_api_get_cali_data(&s_init_config.spv_data.spv_value[0]);
                    pRawdata = &s_init_config.spv_data.spv_value[0];
#ifdef HEADTRACKER_DEBUG_EN
                    printf("[bsp][headtracker][air] cali value, accl {%f, %f, %f}, gyro {%f, %f, %f}",
                        (double) pRawdata[0], (double) pRawdata[1], (double) pRawdata[2],
                        (double) pRawdata[3], (double) pRawdata[4], (double) pRawdata[5]
                    );
#endif
                    bsp_head_tracker_log_w("[bsp][headtracker][air] listen, cali succ and will stop service!", 0);
                    head_track_lib_service(HEAD_TRACKER_SERVICE_STOP, 0); /* no need run task so stop service */
                    break;
                }
                cali_cnt++;
            }
        }
    }
    if (s_init_config.spv_mode == false) {
        imu_api_get_attitude(ftemp);
        event_type = BSP_HEAD_TRACKER_EVENT_RAD_VECTOT_RDY;
        user_data = (void *) &ftemp[0];
        s_init_config.callback(s_headtrack_air_handle, event_type, user_data);
    } else if (cali_cnt >= MAX_HEAD_TRACKER_CALI_DATA_NUM) {
        event_type = BSP_HEAD_TRACKER_EVENT_CALI_SUCC;
        user_data = &s_init_config.spv_data.spv_value[0];
        s_init_config.callback(s_headtrack_air_handle, event_type, user_data);
        cali_cnt = 0;
    }
}




static void head_track_airo_lib_service_handle(head_tracker_imu_service_type_t service_id)
{
    bsp_head_tracker_log_d("[bsp][headtracker][air] lib service handle %d!!", 1, service_id);
    switch (service_id) {
        case HEAD_TRACKER_SERVICE_INIT: {
            if (s_init_config.spv_mode == false) {
                imu_api_initial(&s_init_config.spv_data.spv_value[0]);
            }
            bsp_head_tracker_log_i("[bsp][headtracker][air] imu_api_initial done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_ENABLE: {
            airo_sensor_enable();
            bsp_head_tracker_log_i("[bsp][headtracker][air] airo_sensor_enable done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_DISABLE: {
            airo_sensor_disable();
            bsp_head_tracker_log_i("[bsp][headtracker][air] airo_sensor_disable done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_PROCESS: {
            bsp_head_tracker_log_d("[bsp][headtracker][air] airo_algo_process start", 0);
            airo_algo_process();
            bsp_head_tracker_log_i("[bsp][headtracker][air] airo_algo_process done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_DEINIT: {
            airo_sensor_disable();
        }
        break;

        default:
            break;
    }
}

static  int  head_tracker_check_handle(bsp_head_tracker_imu_handle_t handle)
{
    uint32_t magic_id = 0, lib_vendor = 0;

    magic_id = (handle >> 24) & 0xFF;
    lib_vendor = (handle >> 20) & 0xF;
    if (magic_id != BSP_HEAD_TRACKER_MAGIC_ID || lib_vendor != BSP_HEAD_TRACKER_LIB_VENDOR_AIROHA) {
        bsp_head_tracker_log_e("[bsp][headtracker][air] err handle(0x%x)!!", 1, handle);
        return BSP_HEAD_TRACKER_IMU_STATUS_INVALID_HANDLE;
    }
    return BSP_HEAD_TRACKER_IMU_STATUS_OK;
}

/**----------------------------------------------------------------------------**/
/**                       Public function implement                            **/
/**----------------------------------------------------------------------------**/

bsp_head_tracker_imu_handle_t bsp_head_tracker_imu_airoha_init(bsp_head_tracker_imu_config_t *config)
{
    bsp_head_tracker_imu_handle_t handle;

    if (config == NULL) {
        return 0;
    }
    handle = (BSP_HEAD_TRACKER_MAGIC_ID << 24) | ((config->lib_vendor & 0xF) << 20) | ((config->imu_vendor & 0xF) << 16);
    if (0 == s_op_state) {
        memset(&s_init_config, 0, sizeof(s_init_config));
        s_init_config = *config;
        head_track_lib_service_init(head_track_airo_lib_service_handle);
        head_track_lib_service(HEAD_TRACKER_SERVICE_INIT, 0);
        s_op_state = BSP_HEAD_TRACKER_INITIALIZED;
        s_headtrack_air_handle = handle;
    } else {
        handle = 0;
        bsp_head_tracker_log_e("[bsp][headtracker][air] been initialized!!", 0);
    }
    return handle;
}


int bsp_head_tracker_imu_airoha_control(bsp_head_tracker_imu_handle_t handle, bsp_head_tracker_imu_cmd_t command, uint32_t option)
{
    int status = 0;

    status = head_tracker_check_handle(handle);
    if (status != BSP_HEAD_TRACKER_IMU_STATUS_OK) {
        return status;
    }
    switch (command) {
        case BSP_HEAD_TRACKER_CMD_ENABLE: {
            if (BSP_HEAD_TRACKER_ENABLE != s_op_state) {
                head_track_lib_service(HEAD_TRACKER_SERVICE_ENABLE, 0);
                s_op_state = BSP_HEAD_TRACKER_ENABLE;
            }
        }
        break;

        case BSP_HEAD_TRACKER_CMD_DISABLE: {
            if (BSP_HEAD_TRACKER_ENABLE == s_op_state) {
                head_track_lib_service(HEAD_TRACKER_SERVICE_DISABLE, 0);
                s_op_state = BSP_HEAD_TRACKER_INITIALIZED;
            }
        }
        break;

        case BSP_HEAD_TRACKER_CMD_SET_PERIOD: {
            head_track_lib_service(HEAD_TRACKER_SERVICE_CHGTM, (uint32_t) option);
        }
        break;

        case BSP_HEAD_TRACKER_CMD_GET_STATUS: {
            status = s_op_state;
        }
        break;

        default: {
            status = BSP_HEAD_TRACKER_IMU_STATUS_INVALID_PARAMETER;
        }
        break;
    }
    return status;
}

bsp_head_tracker_imu_status_t bsp_head_tracker_imu_airoha_deinit(bsp_head_tracker_imu_handle_t handle)
{
    s_op_state = 0;
    head_track_lib_service_deinit();
    return 0;
}

#endif
