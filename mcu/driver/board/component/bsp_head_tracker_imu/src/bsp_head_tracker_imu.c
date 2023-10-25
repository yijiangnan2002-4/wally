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
#include "bsp_head_tracker_imu_cywee.h"
#include "bsp_head_tracker_imu_airoha.h"
#include "bsp_head_tracker_imu_common.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "syslog.h"

/**----------------------------------------------------------------------------**/
/**                       Private variable declare                             **/
/**----------------------------------------------------------------------------**/
typedef enum {
    HEAD_TRACKER_SAMAPHORE_TAKE = 0,
    HEAD_TRACKER_SAMAPHORE_GIVE = 1,
} headtracker_semaphore_control_t;

#define  MAX_SEMAPHORE_BLOCK_TICK   500
#if defined(HAL_DVFS_312M_SOURCE)
#define  HEADTRACK_DVFS_LEVEL       DVFS_156M_SPEED
#elif defined(HAL_DVFS_416M_SOURCE)
#define  HEADTRACK_DVFS_LEVEL       HAL_DVFS_HIGH_SPEED_208M
#else
#define  HEADTRACK_DVFS_LEVEL       HAL_DVFS_OPP_HIGH
#endif
/**----------------------------------------------------------------------------**/
/**                       log module create                                     **/
/**----------------------------------------------------------------------------**/
log_create_module(BSP_HEAD_TRACKER_IMU, PRINT_LEVEL_INFO);


/**----------------------------------------------------------------------------**/
/**                       Private variable declare                             **/
/**----------------------------------------------------------------------------**/
static SemaphoreHandle_t s_headtracker_imu_semaphr = NULL;
static bool s_headtracker_imu_lock_dvfs = false;
static uint8_t  s_headtracker_sleep_handle = 0xFF;


void head_tracker_imu_lock()
{
    if (false == s_headtracker_imu_lock_dvfs) {
        s_headtracker_imu_lock_dvfs = true;
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_status_t status = HAL_DVFS_STATUS_ERROR;

        status = hal_dvfs_lock_control(HEADTRACK_DVFS_LEVEL, HAL_DVFS_LOCK);
        status = status;
        bsp_head_tracker_log_w("[bsp][headtracker][imu] lock dvfs, status %d, freq %d", 2, status, hal_dvfs_get_cpu_frequency());
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
        hal_sleep_manager_lock_sleep(s_headtracker_sleep_handle);
#endif
    }
}

void head_tracker_imu_unlock()
{
    if (true == s_headtracker_imu_lock_dvfs) {
        s_headtracker_imu_lock_dvfs = false;
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_status_t status = HAL_DVFS_STATUS_ERROR;

        status = hal_dvfs_lock_control(HEADTRACK_DVFS_LEVEL, HAL_DVFS_UNLOCK);
        status = status;
        bsp_head_tracker_log_w("[bsp][headtracker][imu] unlock dvfs, status %d", 1, status);
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
        hal_sleep_manager_unlock_sleep(s_headtracker_sleep_handle);
#endif
    }
}

int head_tracker_imu_semaphore_control(SemaphoreHandle_t handle, headtracker_semaphore_control_t ctrl)
{
    int mode;
    BaseType_t xStatus;

    mode = HAL_NVIC_QUERY_EXCEPTION_NUMBER;
    switch(ctrl) {
        case HEAD_TRACKER_SAMAPHORE_TAKE: {
            if(mode == 0) {
                xStatus = xSemaphoreTake(handle, 0xFFFF);
            } else {
                xStatus = xSemaphoreTakeFromISR(handle, NULL);
            }
            if (xStatus != pdPASS) {
                return -1;
            }
        } break;
        case HEAD_TRACKER_SAMAPHORE_GIVE: {
            if(mode == 0) {
                xStatus = xSemaphoreGive(handle);
            } else {
                xStatus = xSemaphoreGiveFromISR(handle, NULL);
            }
            if (xStatus != pdPASS) {
                return -1;
            }
        } break;
    }
    return 0;
}

/**----------------------------------------------------------------------------**/
/**                       Public function implement                            **/
/**----------------------------------------------------------------------------**/
bsp_head_tracker_imu_handle_t bsp_head_tracker_imu_init(bsp_head_tracker_imu_config_t *config)
{
    bsp_head_tracker_imu_handle_t handle = 0;

    /*create mute for multi-thread*/
    if (s_headtracker_imu_semaphr == NULL) {
        s_headtracker_imu_semaphr = xSemaphoreCreateMutex();
        if (s_headtracker_imu_semaphr == NULL) {
            bsp_head_tracker_log_e("[bsp][headtracker][imu] init fail, create semaph error!", 0);
            return 0;
        }
#ifdef HAL_SLEEP_MANAGER_ENABLED
        s_headtracker_sleep_handle = hal_sleep_manager_set_sleep_handle("headtracker");
#endif
    }
    /* take semaphore for init*/
    if (head_tracker_imu_semaphore_control(s_headtracker_imu_semaphr, HEAD_TRACKER_SAMAPHORE_TAKE) != 0) {
        bsp_head_tracker_log_e("[bsp][headtracker][imu] init fail, take semaph err!", 0);
        return 0;
    }
#ifdef AIR_HEAD_TRACKER_CWM_ALGO_ENABLE
    handle = bsp_head_tracker_imu_cywee_init(config);
#elif defined(AIR_HEAD_TRACKER_AIR_ALGO_ENABLE)
    handle = bsp_head_tracker_imu_airoha_init(config);
#else
    bsp_head_tracker_log_e("[bsp][headtracker][imu] init fail, none algo available!", 0);
#endif
    head_tracker_imu_semaphore_control(s_headtracker_imu_semaphr, HEAD_TRACKER_SAMAPHORE_GIVE);
    return handle;
}

int bsp_head_tracker_imu_control(bsp_head_tracker_imu_handle_t handle, bsp_head_tracker_imu_cmd_t command, uint32_t option)
{
    int status;

    if (s_headtracker_imu_semaphr == NULL) {
        bsp_head_tracker_log_e("[bsp][headtracker][imu] control fail, semaph is null!", 0);
        return BSP_HEAD_TRACKER_IMU_STATUS_ERROR;
    }
    if (head_tracker_imu_semaphore_control(s_headtracker_imu_semaphr, HEAD_TRACKER_SAMAPHORE_TAKE) != 0) {
        bsp_head_tracker_log_e("[bsp][headtracker][imu] init fail, take semaph err!", 0);
        return 0;
    }
#ifdef AIR_HEAD_TRACKER_CWM_ALGO_ENABLE
    status = bsp_head_tracker_imu_cywee_control(handle, command, option);
#elif defined(AIR_HEAD_TRACKER_AIR_ALGO_ENABLE)
    status = bsp_head_tracker_imu_airoha_control(handle, command, option);
#else
    bsp_head_tracker_log_e("[bsp][headtracker][imu] control fail, none algo available!", 0);
    status = BSP_HEAD_TRACKER_IMU_STATUS_UNSUPPORT;
#endif
    bsp_head_tracker_log_d("[bsp][headtracker][imu] control, command %d, status %d", 2, command, status);
    if (status == BSP_HEAD_TRACKER_IMU_STATUS_OK) {
        static uint8_t  lock_st = 0;
        hal_sleep_manager_status_t  slp_st = 0;

        if (command == BSP_HEAD_TRACKER_CMD_ENABLE && lock_st == 0) {
            head_tracker_imu_lock();
            lock_st = 1;
        } else if (command == BSP_HEAD_TRACKER_CMD_DISABLE && lock_st == 1) {
            head_tracker_imu_unlock();
            lock_st = 0;
        }

        if (lock_st < 2) {
            bsp_head_tracker_log_w("[bsp][headtracker][imu] control, lock sleep:%d, status %d", 2, lock_st, slp_st);
        }
    }
    head_tracker_imu_semaphore_control(s_headtracker_imu_semaphr, HEAD_TRACKER_SAMAPHORE_GIVE);
    return status;
}

bsp_head_tracker_imu_status_t bsp_head_tracker_imu_deinit(bsp_head_tracker_imu_handle_t handle)
{
    bsp_head_tracker_imu_status_t status = BSP_HEAD_TRACKER_IMU_STATUS_ERROR;

    if (s_headtracker_imu_semaphr == NULL) {
        bsp_head_tracker_log_e("[bsp][headtracker][imu] de-init fail, semaph is null!", 0);
        return BSP_HEAD_TRACKER_IMU_STATUS_ERROR;
    }
    head_tracker_imu_unlock();
#ifdef AIR_HEAD_TRACKER_CWM_ALGO_ENABLE
    status = bsp_head_tracker_imu_cywee_deinit(handle);
#elif defined(AIR_HEAD_TRACKER_AIR_ALGO_ENABLE)
    status = bsp_head_tracker_imu_airoha_deinit(handle);
#else
    bsp_head_tracker_log_e("[bsp][headtracker][imu] de-init fail, none algo available!", 0);
    status = BSP_HEAD_TRACKER_IMU_STATUS_UNSUPPORT;
#endif
    vSemaphoreDelete(s_headtracker_imu_semaphr);
    s_headtracker_imu_semaphr = NULL;
    return status;
}




