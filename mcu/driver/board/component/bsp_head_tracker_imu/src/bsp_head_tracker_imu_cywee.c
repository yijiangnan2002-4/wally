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

#ifdef AIR_HEAD_TRACKER_CWM_ALGO_ENABLE
#include "hal.h"
#include "hal_platform.h"
#include "hal_gpt.h"
#include "bsp_head_tracker_imu_cywee.h"
#include "bsp_head_tracker_imu_common.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "cwm_api.h"
#include "cwm_lib.h"
#include "cwm_lib_dml.h"


/**----------------------------------------------------------------------------**/
/**                       Private varaible define                              **/
/**----------------------------------------------------------------------------**/
static bool s_cwm_initialized = 0;
static bool s_cwm_enable = 0;
static bsp_head_tracker_imu_config_t s_cwm_init_config;
static bsp_head_tracker_imu_handle_t s_cwm_lib_handle = 0;

/**----------------------------------------------------------------------------**/
/**                       Private function implement                           **/
/**----------------------------------------------------------------------------**/

static void    head_track_cywee_listen_callback(pSensorEVT_t sensorEVT)
{
    bsp_head_tracker_imu_event_t event_type = BSP_HEAD_TRACKER_EVENT_MAX;
    void    *user_data = NULL;

    switch (sensorEVT->sensorType) {
        case CUSTOM_HEAD_TRACKER: {
            event_type = BSP_HEAD_TRACKER_EVENT_RAD_VECTOT_RDY;
            user_data  = (void *) &sensorEVT->fData[1];
        }
        break;

        case IDX_ALGO_SPV: {
            float *spv_data = NULL;
            spv_data = sensorEVT->fData;
            if (spv_data == NULL) {
                break;
            }
            if (spv_data[0] == 1) {
                if (spv_data[1] == 1) {
                    event_type = BSP_HEAD_TRACKER_EVENT_CALI_SUCC;
                    user_data  = (void *) &spv_data[4];
                    bsp_head_tracker_log_w("[bsp][headtracker][cwm] listen, cali succ and will stop service!", 0);
                } else {
                    event_type = BSP_HEAD_TRACKER_EVENT_CALI_FAIL;
                    user_data  = (void *) &spv_data[2];
                    bsp_head_tracker_log_e("[bsp][headtracker][cwm] listen, cali fail %d!!", 1, (int) spv_data[2]);
                }
                head_track_lib_service(HEAD_TRACKER_SERVICE_STOP, 0);/* no need run task so stop service */
            }
        }
        break;
    }
    if (event_type != BSP_HEAD_TRACKER_EVENT_MAX) {
        s_cwm_init_config.callback(s_cwm_lib_handle, event_type, user_data);
    }
    bsp_head_tracker_log_d("[bsp][headtracker][cwm] timer listen, type %d!!", 1, sensorEVT->sensorType);
}

static void head_track_cywee_lib_service_handle(head_tracker_imu_service_type_t service_id)
{
    bsp_head_tracker_log_d("[bsp][headtracker][cwm] lib service handle %d!!", 1, service_id);
    bsp_head_tracker_log_d("[bsp][headtracker][cwm] dvfs freq %d", 1, hal_dvfs_get_cpu_frequency());
    switch (service_id) {
        case HEAD_TRACKER_SERVICE_INIT: {
            if (s_cwm_init_config.spv_mode == true) {
                cwm_dml_spv_init(head_track_cywee_listen_callback);
            } else {
                cwm_dml_init(head_track_cywee_listen_callback, s_cwm_init_config.spv_data.spv_value);
            }
            bsp_head_tracker_log_i("[bsp][headtracker][cwm] cwm_dml_init done, spv mode = %d!", 1, s_cwm_init_config.spv_mode);
        }
        break;

        case HEAD_TRACKER_SERVICE_ENABLE: {
            if (s_cwm_init_config.spv_mode == true) {
                CWM_Sensor_Enable(IDX_ALGO_SPV);
            } else {
                CWM_Sensor_Enable(CUSTOM_HEAD_TRACKER);
            }
            bsp_head_tracker_log_i("[bsp][headtracker][cwm] cwm_sensor_enable done, spv mode = %d!!", 1, s_cwm_init_config.spv_mode);
        }
        break;

        case HEAD_TRACKER_SERVICE_DISABLE: {
            if (s_cwm_init_config.spv_mode == true) {
                CWM_Sensor_Disable(IDX_ALGO_SPV);
            } else {
                CWM_Sensor_Disable(CUSTOM_HEAD_TRACKER);
            }
            bsp_head_tracker_log_i("[bsp][headtracker][cwm] cwm_sensor_disable done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_PROCESS: {
            bsp_head_tracker_log_d("[bsp][headtracker][cwm] cwm_dml_process start", 0);
            CWM_Dml_process();
            bsp_head_tracker_log_i("[bsp][headtracker][cwm] cwm_dml_process done!", 0);
        }
        break;

        case HEAD_TRACKER_SERVICE_DEINIT: {
            CWM_Sensor_Disable(CUSTOM_HEAD_TRACKER);
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
    if (magic_id != BSP_HEAD_TRACKER_MAGIC_ID || lib_vendor != BSP_HEAD_TRACKER_LIB_VENDOR_CYWEE) {
        bsp_head_tracker_log_e("[bsp][headtracker][cwm] err handle(0x%x)!!", 1, handle);
        return BSP_HEAD_TRACKER_IMU_STATUS_INVALID_HANDLE;
    }
    return BSP_HEAD_TRACKER_IMU_STATUS_OK;
}

/**----------------------------------------------------------------------------**/
/**                       Public function implement                            **/
/**----------------------------------------------------------------------------**/

bsp_head_tracker_imu_handle_t bsp_head_tracker_imu_cywee_init(bsp_head_tracker_imu_config_t *config)
{
    bsp_head_tracker_imu_handle_t handle;

    if (config == NULL) {
        return 0;
    }
    handle = (BSP_HEAD_TRACKER_MAGIC_ID << 24) | ((config->lib_vendor & 0xF) << 20) | ((config->imu_vendor & 0xF) << 16);
    if (false == s_cwm_initialized) {
        s_cwm_init_config = *config;
        head_track_lib_service_init(head_track_cywee_lib_service_handle);
        head_track_lib_service(HEAD_TRACKER_SERVICE_INIT, 0);
        s_cwm_lib_handle = handle;
        s_cwm_initialized = true;
    } else {
        handle = 0;
        bsp_head_tracker_log_e("[bsp][headtracker][cwm] been initialized!!", 0);
    }
    return handle;
}

int bsp_head_tracker_imu_cywee_control(bsp_head_tracker_imu_handle_t handle, bsp_head_tracker_imu_cmd_t command, uint32_t option)
{
    int status = 0;

    status = head_tracker_check_handle(handle);
    if (status != BSP_HEAD_TRACKER_IMU_STATUS_OK) {
        return status;
    }
    switch (command) {
        case BSP_HEAD_TRACKER_CMD_ENABLE: {
            if (s_cwm_enable == false) {
                head_track_lib_service(HEAD_TRACKER_SERVICE_ENABLE, 0);
                s_cwm_enable = true;
            }
        }
        break;

        case BSP_HEAD_TRACKER_CMD_DISABLE: {
            if (s_cwm_enable == true) {
                s_cwm_enable = false;
                head_track_lib_service(HEAD_TRACKER_SERVICE_DISABLE, 0);
            }
        }
        break;

        case BSP_HEAD_TRACKER_CMD_SET_PERIOD: {
            head_track_lib_service(HEAD_TRACKER_SERVICE_CHGTM, (uint32_t) option);
        }
        break;

        case BSP_HEAD_TRACKER_CMD_GET_STATUS: {
            if (s_cwm_enable == true) {
                status = BSP_HEAD_TRACKER_ENABLE;
            } else if (s_cwm_initialized == true) {
                status = BSP_HEAD_TRACKER_INITIALIZED;
            }
        }
        break;

        default: {
            status = BSP_HEAD_TRACKER_IMU_STATUS_INVALID_PARAMETER;
        }
        break;
    }
    return status;
}

bsp_head_tracker_imu_status_t bsp_head_tracker_imu_cywee_deinit(bsp_head_tracker_imu_handle_t handle)
{
    uint32_t status = BSP_HEAD_TRACKER_IMU_STATUS_OK;

    status = head_tracker_check_handle(handle);
    if (status != BSP_HEAD_TRACKER_IMU_STATUS_OK) {
        return status;
    }
    s_cwm_initialized = false;
    head_track_lib_service_deinit();
    return BSP_HEAD_TRACKER_IMU_STATUS_OK;
}

#endif /* AIR_HEAD_TRACKER_CWM_ALGO_ENABLE */
