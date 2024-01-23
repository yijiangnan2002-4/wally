/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
#include "hal_wdt.h"
#include "bt_type.h"
#include "bt_system.h"
#include "bt_utils.h"
#include "bt_device_manager_internal.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_utils.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager_power.h"
#include "bt_aws_mce_srv.h"
#include "bt_device_manager_test_mode.h"

void default_bt_cm_poweroff_notify()
{
}

void bt_cm_poweroff_notify();

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_cm_poweroff_notify=_default_bt_cm_poweroff_notify")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_cm_poweroff_notify = default_bt_cm_poweroff_notify
#else
#error "Unsupported Platform"
#endif

#define BT_CM_POWER_FLAG_FORCE              (0x01)
#define BT_CM_POWER_FLAG_DUT_MODE           (0x02)
//#define BT_CM_POWER_FLAG_DEINIT_PREPAIRE    (0x03)
#define BT_CM_POWER_FLAG_TEST_SYS_OFF       (0x08)
#define BT_CM_POWER_FLAG_TEST_SYS_RESET     (0x10)
typedef uint8_t bt_cm_power_flag_t;

static struct {
    bt_cm_power_state_t cur_state;
    bt_cm_power_state_t target_state;
    bt_cm_power_flag_t  flags;
} g_bt_cm_edr_power_cnt = {
    .cur_state = BT_CM_POWER_STATE_OFF,
    .target_state = BT_CM_POWER_STATE_OFF,
    .flags = 0
};

bt_device_manager_power_status_t g_bt_cm_edr_power_status = BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS;

bt_cm_power_state_t bt_cm_power_get_state()
{
    return g_bt_cm_edr_power_cnt.cur_state;
}

void                bt_cm_power_update(void *params)
{
    bt_cmgr_report_id("[BT_CM][POWER][I] BT power update, state target:0x%x, cur:0x%x, flag:0x%x", 3,
                      g_bt_cm_edr_power_cnt.target_state, g_bt_cm_edr_power_cnt.cur_state, g_bt_cm_edr_power_cnt.flags);
    if (BT_CM_POWER_STATE_OFF == g_bt_cm_edr_power_cnt.cur_state && BT_CM_POWER_STATE_ON == g_bt_cm_edr_power_cnt.target_state) {
        g_bt_cm_edr_power_cnt.flags = 0;
        g_bt_cm_edr_power_cnt.cur_state = BT_CM_POWER_STATE_ON;
        bt_cmgr_report_id("[BT_CM][POWER][I] BT POWER ON cnf", 0);
        bt_cm_power_state_update_ind_t state = {
            .power_state = BT_CM_POWER_STATE_ON
        };
        if (BT_DEVICE_MANAGER_POWER_STATE_RESTING != bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_CLASSIC)) {
            bt_cm_event_callback(BT_CM_EVENT_POWER_STATE_UPDATE, &state, sizeof(state));
        }
        if (BT_DEVICE_MANAGER_POWER_STATUS_ROLE_RECOVERY != g_bt_cm_edr_power_status) {
            bt_cm_register_callback_notify(BT_CM_EVENT_POWER_STATE_UPDATE, &state, sizeof(state));
        }
        bt_cm_power_on_cnf(g_bt_cm_edr_power_status);
        bt_device_manager_dev_set_power_state(BT_DEVICE_TYPE_CLASSIC, BT_DEVICE_MANAGER_POWER_STATE_ACTIVE);
    } else if ((BT_CM_POWER_STATE_ON == g_bt_cm_edr_power_cnt.cur_state && BT_CM_POWER_STATE_OFF == g_bt_cm_edr_power_cnt.target_state) ||
               BT_CM_POWER_STATE_OFF_PENDING == g_bt_cm_edr_power_cnt.cur_state) {
        bt_cm_poweroff_notify();

        g_bt_cm_edr_power_cnt.cur_state = BT_CM_POWER_STATE_OFF_PENDING;
        if (NULL == params && BT_STATUS_SUCCESS != bt_cm_prepare_power_deinit(false, g_bt_cm_edr_power_status)) {
            if (!bt_cm_timer_is_exist(BT_CM_FORCE_POWER_OFF_TIMER_ID)) {
                bt_cm_timer_start(BT_CM_FORCE_POWER_OFF_TIMER_ID, 10000, bt_cm_power_update, (void *)1);
            }
            return;
        }
        bt_cm_timer_stop(BT_CM_FORCE_POWER_OFF_TIMER_ID);
        if (BT_STATUS_SUCCESS == bt_cm_prepare_power_deinit(true, g_bt_cm_edr_power_status)) {
            return;
        }
        if (NULL == params) {
            bt_cm_timer_start(BT_CM_FORCE_POWER_OFF_TIMER_ID, 1, bt_cm_power_update, (void *)1);
            return;
        }
        if (0 != bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, NULL, 0)) {
            bt_cmgr_report_id("[BT_CM][POWER][I] there are still connection exit!!! ", 0);
            bt_cm_timer_start(BT_CM_FORCE_POWER_OFF_TIMER_ID, 500, bt_cm_power_update, (void *)1);
            return;
        }
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_aws_local_info_update();
#endif
        g_bt_cm_edr_power_cnt.flags = 0;
        bt_cmgr_report_id("[BT_CM][POWER][I] BT POWER OFF cnf", 0);
        bt_cm_power_off_cnf(g_bt_cm_edr_power_status);
        bt_cm_power_state_update_ind_t state = {
            .power_state = BT_CM_POWER_STATE_OFF
        };
        g_bt_cm_edr_power_cnt.cur_state = BT_CM_POWER_STATE_OFF;
        if (BT_DEVICE_MANAGER_POWER_STATE_RESTING != bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_CLASSIC)) {
            bt_cm_event_callback(BT_CM_EVENT_POWER_STATE_UPDATE, &state, sizeof(state));
        }
        if (BT_DEVICE_MANAGER_POWER_STATUS_ROLE_RECOVERY != g_bt_cm_edr_power_status) {
            bt_cm_register_callback_notify(BT_CM_EVENT_POWER_STATE_UPDATE, &state, sizeof(state));
        }
        bt_device_manager_db_flush_all(BT_DEVICE_MANAGER_DB_FLUSH_BLOCK);
        bt_device_manager_dev_set_power_state(BT_DEVICE_TYPE_CLASSIC, BT_DEVICE_MANAGER_POWER_STATE_STANDBY);
    }
}

static bt_status_t  bt_cm_power_device_manager_callback(bt_device_manager_power_event_t evt, bt_device_manager_power_status_t status, void *data, uint32_t data_length)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_ACTIVE: {
            bt_cmgr_report_id("[BT_CM][POWER][I] Prepare active, current state:%d, status:%d", 2, g_bt_cm_edr_power_cnt.cur_state, status);
            if (BT_DEVICE_MANAGER_TEST_MODE_NONE == bt_device_manager_get_test_mode()) {
                g_bt_cm_edr_power_status = status;
                g_bt_cm_edr_power_cnt.target_state = BT_CM_POWER_STATE_ON;
                if (BT_CM_POWER_STATE_ON == g_bt_cm_edr_power_cnt.cur_state) {
                    return BT_STATUS_SUCCESS;
                } else if (BT_CM_POWER_STATE_OFF == g_bt_cm_edr_power_cnt.cur_state) {
                    bt_cm_timer_start(BT_CM_FORCE_POWER_OFF_TIMER_ID, 1, bt_cm_power_update, NULL);
                }
                return BT_STATUS_PENDING;
            } else {
                return BT_STATUS_SUCCESS;
            }
        }
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_STANDBY: {
            bt_cmgr_report_id("[BT_CM][POWER][I] Prepare standby, current state:%d, status:%d", 2, g_bt_cm_edr_power_cnt.cur_state, status);
            if (BT_CM_POWER_STATE_OFF == g_bt_cm_edr_power_cnt.cur_state) {
                return BT_STATUS_SUCCESS;
            }
            bt_utils_mutex_lock();
            g_bt_cm_edr_power_cnt.target_state = BT_CM_POWER_STATE_OFF;
            g_bt_cm_edr_power_status = status;
            if (BT_CM_POWER_STATE_ON == g_bt_cm_edr_power_cnt.cur_state) {
#ifdef MTK_AWS_MCE_ENABLE
                if ((BT_DEVICE_MANAGER_POWER_STATUS_AIR_PAIRING_COMPLETE != status) && (BT_DEVICE_MANAGER_POWER_STATUS_AIR_PAIRING_START != status)) {
                    bt_aws_mce_srv_air_pairing_stop();
                }
#endif
                if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_UNKNOW);
                }
                bt_cm_write_scan_mode_internal(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
                g_bt_cm_edr_power_cnt.cur_state = BT_CM_POWER_STATE_OFF_PENDING;
                bt_cm_timer_start(BT_CM_FORCE_POWER_OFF_TIMER_ID, 1, bt_cm_power_update, NULL);
            }
            bt_utils_mutex_unlock();
            return BT_STATUS_PENDING;
        }
        default:
            break;
    }
    return ret;
}

#if 0
static void         bt_cm_power_test_sys_excute()
{
    return;
}

static bt_status_t  bt_cm_power_sys_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    return BT_STATUS_SUCCESS;
}
#endif

bt_status_t         bt_cm_power_active()
{
    return BT_STATUS_SUCCESS;
}

bt_status_t         bt_cm_power_standby(bool force)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t         bt_cm_power_reset(bool force)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t         bt_cm_power_reset_ext(bool force, bt_cm_power_reset_callback_t cb, void *user_data)
{
    return BT_STATUS_SUCCESS;
}

void                bt_cm_power_init()
{
    bt_device_manager_dev_register_callback(BT_DEVICE_TYPE_CLASSIC, (bt_device_manager_power_callback_t)bt_cm_power_device_manager_callback);
}

void                bt_cm_power_deinit()
{
    bt_device_manager_dev_register_callback(BT_DEVICE_TYPE_CLASSIC, NULL);
}

//Just for UT/IT test.
void                bt_cm_power_test_sys(bt_cm_power_test_sys_t type)
{
    return;
}

void                bt_cm_power_standby_ready()
{
    return;
}

