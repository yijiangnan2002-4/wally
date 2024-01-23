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

#include "bt_system.h"
//#include "bt_sink_srv.h"
#include "bt_gap_le.h"
#include "bt_connection_manager_internal.h"
#include "bt_connection_manager_utils.h"
#include "bt_device_manager_power.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_os_layer_api.h"
#include "hal_trng.h"
#include "bt_callback_manager.h"
#include "bt_ull_service.h"
#include "bt_ull_utility.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif


#define BT_ULL_AIR_PAIRING_FLAG_STARTED         (0x01)
#define BT_ULL_AIR_PAIRING_FLAG_INITIATED       (0x02)
#define BT_ULL_AIR_PAIRING_FLAG_INQUIRING       (0x04)
#define BT_ULL_AIR_PAIRING_FLAG_COMPLETE        (0x08)
typedef uint8_t bt_ull_air_pairing_flag_t;

typedef struct {
    bt_ull_air_pairing_flag_t flags;
    bt_ull_pairing_info_t      cnt;
    bt_bd_addr_t                            remote_address;
} bt_ull_air_pairing_cnt_t;

static bt_ull_air_pairing_cnt_t g_ultra_low_latency_air_pairing_cnt;

static void         bt_ull_pairing_info_timeout_callback(void *params);
static bt_status_t  bt_ull_air_pairing_event_handle(bt_msg_type_t msg, bt_status_t status, void *buffer);

static bt_status_t  bt_ull_air_pairing_bt_reset_callback(bt_device_manager_power_reset_progress_t type, void *user_data)
{
    ull_report("[BT_ULL][AIR_PAIRING][E] Air pairing reset type %d, flag:0x%x", 2, type, g_ultra_low_latency_air_pairing_cnt.flags);
    if (BT_DEVICE_MANAGER_POWER_RESET_PROGRESS_COMPLETE == type) {
        if (g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_STARTED) {
            bt_hci_iac_lap_t iac = 0x9E8B15;
            g_ultra_low_latency_air_pairing_cnt.flags |= BT_ULL_AIR_PAIRING_FLAG_INITIATED;
            bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                  (uint32_t)MODULE_MASK_GAP, (void *)bt_ull_air_pairing_event_handle);
            bt_gap_write_inquiry_access_code(&iac, 1);
            bt_cm_timer_start(BT_SINK_SRV_CM_TERMINATE_AIR_PAIRING_TIMER_ID,
                              g_ultra_low_latency_air_pairing_cnt.cnt.duration * 1000, bt_ull_pairing_info_timeout_callback, (void *)0);
        } else {
            bt_ull_pairing_complete_ind_t air_pairing_ind;
            air_pairing_ind.result = (bool)user_data;
            memcpy((void *)air_pairing_ind.remote_address, (void *)&g_ultra_low_latency_air_pairing_cnt.remote_address, sizeof(bt_bd_addr_t));
            bt_ull_event_callback(BT_ULL_EVENT_PAIRING_COMPLETE_IND, (void *)&air_pairing_ind, sizeof(air_pairing_ind));
#ifdef MTK_AWS_MCE_ENABLE
            bt_cm_register_callback_notify(BT_AWS_MCE_SRV_EVENT_AIR_PAIRING_COMPLETE, &air_pairing_ind, sizeof(air_pairing_ind));
#endif
        }
    }
    return BT_STATUS_SUCCESS;
}

static void         bt_ull_pairing_info_timeout_callback(void *params)
{
    uint32_t data = (uint32_t)params;
    bt_hci_iac_lap_t iac = BT_HCI_IAC_LAP_TYPE_GIAC;
    g_ultra_low_latency_air_pairing_cnt.flags = 0;
    bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_CONNECTABLE_ONLY);
    bt_cm_write_scan_mode_internal(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
    bt_gap_write_inquiry_access_code(&iac, 1);
    if (0 == data) {
        bt_device_manager_power_reset(BT_DEVICE_MANAGER_POWER_RESET_TYPE_AIR_PAIRING_COMPLETE, bt_ull_air_pairing_bt_reset_callback, (void *)false);
    } else {
        bt_device_manager_power_reset(BT_DEVICE_MANAGER_POWER_RESET_TYPE_AIR_PAIRING_COMPLETE, bt_ull_air_pairing_bt_reset_callback, (void *)true);
    }
}

static void         bt_ull_air_pairing_inq_ind_handle(bt_gap_inquiry_ind_t *inq_ret)
{
    if (NULL == inq_ret || NULL == inq_ret->rssi) {
        ull_report_error("[BT_ULL][AIR_PAIRING][E] Inquiry indicate buffer is NULL !!!", 0);
        return;
    }
    ull_report("[BT_ULL][AIR_PAIRING][I] Recevice rssi %d, need rssi %d flags %d", 3,
               *(int8_t *)(inq_ret->rssi), g_ultra_low_latency_air_pairing_cnt.cnt.rssi_threshold, g_ultra_low_latency_air_pairing_cnt.flags);
    if ((*(int8_t *)(inq_ret->rssi) > g_ultra_low_latency_air_pairing_cnt.cnt.rssi_threshold) &&
        (g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_INQUIRING)) {
        bt_os_layer_aes_buffer_t decrypted_data;
        bt_os_layer_aes_buffer_t plain_text;
        bt_os_layer_aes_buffer_t aes_key;
        uint8_t air_pairing_length = sizeof(bt_ull_role_t) + sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
        uint8_t air_pairing_rsp[air_pairing_length];

        decrypted_data.buffer = (uint8_t *) & (air_pairing_rsp[1]);
        decrypted_data.length = sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
        air_pairing_rsp[0] = inq_ret->eir[4];
        plain_text.buffer = (uint8_t *) & (inq_ret->eir[5]);
        plain_text.length = sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
        aes_key.buffer = g_ultra_low_latency_air_pairing_cnt.cnt.key;
        aes_key.length = sizeof(bt_key_t);
        bt_os_layer_aes_decrypt(&decrypted_data, &plain_text, &aes_key);
        ull_report("[BT_ULL][AIR_PAIRING][I] defult role: 0x%x, receive role: 0x%x", 2, g_ultra_low_latency_air_pairing_cnt.cnt.role, air_pairing_rsp[0]);
        if (g_ultra_low_latency_air_pairing_cnt.cnt.role == air_pairing_rsp[0]) {
            ull_report("[BT_ULL][AIR_PAIRING][E] Recevied the same role", 0);
        } else if (0 == memcmp(air_pairing_rsp + sizeof(bt_ull_role_t), (uint8_t *)g_ultra_low_latency_air_pairing_cnt.cnt.info,
                               sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info))) {
            g_ultra_low_latency_air_pairing_cnt.flags |= BT_ULL_AIR_PAIRING_FLAG_COMPLETE;
            bt_gap_cancel_inquiry();
            ull_report("[BT_ULL][AIR_PAIRING][I] Air pairing success !!!", 0);
            memcpy((void *)&g_ultra_low_latency_air_pairing_cnt.remote_address, (void *)inq_ret->address, sizeof(bt_bd_addr_t));
            bt_cm_timer_stop(BT_SINK_SRV_CM_TERMINATE_AIR_PAIRING_TIMER_ID);
            bt_cm_timer_start(BT_SINK_SRV_CM_END_AIR_PAIRING_TIMER_ID,
                              BT_SINK_SRV_CM_END_AIR_PAIRING_TIMER_DUR, bt_ull_pairing_info_timeout_callback, (void *)1);
        }
    }
}

static void         bt_ull_air_pairing_set_eir_cnf_handle(void)
{
    hal_trng_status_t ret = HAL_TRNG_STATUS_OK;
    /* 0.625 ms * 36(0x24) = 22.5 ms */
    uint16_t interval = 0x24;
    /* 0.625 ms * 18(0x12) = 11.25 ms */
    uint16_t window = 0x12;
    uint32_t random_seed;
    bt_ull_role_t role = g_ultra_low_latency_air_pairing_cnt.cnt.role;
    ret = hal_trng_init();
    if (HAL_TRNG_STATUS_OK != ret) {
        ull_report_error("[BT_ULL][AIR_PAIRING][E] generate random inquiry init fail", 0);
    } else {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            ull_report_error("[BT_ULL][AIR_PAIRING][E] generate random inquiry window error", 0);
        } else {
            if (BT_ULL_ROLE_SERVER == role) {
                /* interval max: 0.625 ms * 80(0x50) = 50 ms */
                /* interval min: 0.625 ms * 36(0x24) = 22.5 ms */
                interval = (random_seed & (0x50 - 0x24)) + 0x24;
                ull_report("[BT_ULL][AIR_PAIRING][I] left interval %d", 1, interval);
            } else {
                /* interval max: 0.625 ms * 160(0xA0) = 100 ms */
                /* interval min: 0.625 ms * 96(0x60) = 60 ms */
                interval = 0x60 + (random_seed & (0x40 - 0x24)) + 0x24;
                ull_report("[BT_ULL][AIR_PAIRING][I] right interval %d", 1, interval);
            }
        }
    }
    hal_trng_deinit();
    ull_report("[BT_ULL][AIR_PAIRING][I] generate random inquiry scan interval: %d, window: %d", 2, interval, window);
    bt_gap_write_inquiry_scan_activity(interval, window);
    bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_DISCOVERABLE_ONLY);
}

static void         bt_ull_air_pairing_write_iac_cnf(void)
{
    bt_os_layer_aes_buffer_t encryped_key;
    bt_os_layer_aes_buffer_t plain_text;
    bt_os_layer_aes_buffer_t aes_key;
    uint8_t air_info_length = sizeof(bt_ull_role_t) + sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
    uint8_t air_info[air_info_length];
    uint8_t eir_data[air_info_length + 4];
    memset(eir_data, 0, sizeof(eir_data));
    memcpy(&air_info, &g_ultra_low_latency_air_pairing_cnt.cnt.role, sizeof(bt_ull_role_t));
    memcpy(air_info + sizeof(bt_ull_role_t), &g_ultra_low_latency_air_pairing_cnt.cnt.info,
           sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info));
    ull_report("[BT_ULL][AIR_PAIRING][I] air info 1 0x%x", 1, *(uint32_t *)air_info);
    bt_hci_enable_t filter_enable = BT_HCI_DISABLE;
    encryped_key.buffer = eir_data + 4 + sizeof(bt_ull_role_t);
    encryped_key.length = sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
    plain_text.buffer = air_info + sizeof(bt_ull_role_t);
    plain_text.length = sizeof(g_ultra_low_latency_air_pairing_cnt.cnt.info);
    aes_key.buffer = g_ultra_low_latency_air_pairing_cnt.cnt.key;
    aes_key.length = sizeof(bt_key_t);
    bt_os_layer_aes_encrypt(&encryped_key, &plain_text, &aes_key);
    eir_data[0] = (sizeof(eir_data) - 1);
    eir_data[1] = 0xFF;
    eir_data[2] = 0x94;
    eir_data[3] = 0x00;
    eir_data[4] = g_ultra_low_latency_air_pairing_cnt.cnt.role;
    ull_report("[BT_ULL][AIR_PAIRING] eir data info 2 0x%x", 1, *(uint32_t *)(eir_data + 4));
    bt_hci_send_vendor_cmd(0xFDC9, &filter_enable, sizeof(filter_enable));
    bt_gap_set_extended_inquiry_response(eir_data, sizeof(eir_data));
}

static bt_status_t  bt_ull_air_pairing_event_handle(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t result = BT_STATUS_SUCCESS;
    if (!(g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_INITIATED)) {
        return result;
    }
    ull_report("[BT_ULL][AIR_PAIRING] msg:0x%x, status:0x%x", 2, msg, status);
    switch (msg) {
        case BT_GAP_SET_SCAN_MODE_CNF: {
            if (status == BT_STATUS_SUCCESS) {
                uint8_t inquiry_duration = ((g_ultra_low_latency_air_pairing_cnt.cnt.duration - 4) * 100) / 128;
                bt_gap_inquiry_ext(inquiry_duration, 0, 0x9E8B15);
            } else {
                bt_ull_air_pairing_stop();
            }
        }
        break;
        case BT_GAP_INQUIRY_CNF: {
            if (status == BT_STATUS_SUCCESS) {
                g_ultra_low_latency_air_pairing_cnt.flags |= BT_ULL_AIR_PAIRING_FLAG_INQUIRING;
            } else {
                bt_ull_air_pairing_stop();
            }
        }
        break;
        case BT_GAP_INQUIRY_IND: {
            bt_ull_air_pairing_inq_ind_handle((bt_gap_inquiry_ind_t *)buffer);
        }
        break;
        case BT_GAP_SET_EIR_CNF: {
            if (BT_STATUS_SUCCESS == status) {
                bt_ull_air_pairing_set_eir_cnf_handle();
            } else {
                bt_ull_air_pairing_stop();
            }
        }
        break;
        case BT_GAP_WRITE_INQUIRY_ACCESS_CODE_CNF: {
            if (BT_STATUS_SUCCESS == status) {
                bt_ull_air_pairing_write_iac_cnf();
            } else {
                bt_ull_air_pairing_stop();
            }
        }
        break;
        case BT_GAP_INQUIRY_COMPLETE_IND: {
            g_ultra_low_latency_air_pairing_cnt.flags &= (~BT_ULL_AIR_PAIRING_FLAG_INQUIRING);
            bt_ull_air_pairing_stop();
        }
        break;
        case BT_GAP_CANCEL_INQUIRY_CNF: {
            g_ultra_low_latency_air_pairing_cnt.flags &= (~BT_ULL_AIR_PAIRING_FLAG_INQUIRING);
            if (!(g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_COMPLETE)) {
                bt_ull_air_pairing_stop();
            } else {
                g_ultra_low_latency_air_pairing_cnt.flags &= (~BT_ULL_AIR_PAIRING_FLAG_COMPLETE);
            }
        }
        break;
        default:
            ull_report("[BT_ULL][AIR_PAIRING][I] Unexcepted msg:%x", 1, msg);
            break;
    }
    return result;
}

bt_status_t bt_ull_air_pairing_start(const bt_ull_pairing_info_t *param)
{
    if (NULL == param) {
        return BT_STATUS_FAIL;
    }
    if (g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_STARTED) {
        return BT_STATUS_PENDING;
    }
    g_ultra_low_latency_air_pairing_cnt.flags |= BT_ULL_AIR_PAIRING_FLAG_STARTED;
    bt_timer_ext_stop(BT_ULL_CONFLICT_RECONNECT_TIMER_ID);
    ull_report("[BT_ULL][AIR_PAIRING][I] Air pairing start duration %d, default role 0x%02X, rssi %d info 0x%x", 4,
               param->duration, param->role, param->rssi_threshold, *(uint32_t *)(param->info));
    bt_ull_event_callback(BT_ULL_EVENT_PAIRING_STARTED_IND, NULL, 0);
#ifdef MTK_AWS_MCE_ENABLE
    bt_cm_register_callback_notify(BT_AWS_MCE_SRV_EVENT_AIR_PAIRING_STARTED, NULL, 0);
#endif
    memcpy(&(g_ultra_low_latency_air_pairing_cnt.cnt), param, sizeof(*param));
    bt_device_manager_power_reset(BT_DEVICE_MANAGER_POWER_RESET_TYPE_AIR_PAIRING_START, bt_ull_air_pairing_bt_reset_callback, NULL);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_air_pairing_stop(void)
{
    ull_report("[BT_ULL][AIR_PAIRING][I] Cancel air pairing cur flags 0x%02x", 1, g_ultra_low_latency_air_pairing_cnt.flags);
    if (!(g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_STARTED)) {
        return BT_STATUS_FAIL;
    }
    bt_cm_timer_stop(BT_SINK_SRV_CM_END_AIR_PAIRING_TIMER_ID);
    bt_cm_timer_stop(BT_SINK_SRV_CM_TERMINATE_AIR_PAIRING_TIMER_ID);
    if (g_ultra_low_latency_air_pairing_cnt.flags & BT_ULL_AIR_PAIRING_FLAG_INQUIRING) {
        bt_gap_cancel_inquiry();
    } else {
        bt_hci_iac_lap_t iac = BT_HCI_IAC_LAP_TYPE_GIAC;
        bt_cm_write_scan_mode_internal(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
        bt_gap_write_inquiry_access_code(&iac, 1);
        g_ultra_low_latency_air_pairing_cnt.flags = 0;
        bt_callback_manager_deregister_callback(bt_callback_type_app_event, (void *)bt_ull_air_pairing_event_handle);
        bt_device_manager_power_reset(BT_DEVICE_MANAGER_POWER_RESET_TYPE_AIR_PAIRING_COMPLETE, bt_ull_air_pairing_bt_reset_callback, (void *)0);
    }
    return BT_STATUS_SUCCESS;
}


