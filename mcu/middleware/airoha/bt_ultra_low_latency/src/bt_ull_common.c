/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
#include "bt_type.h"
#include "bt_ull_service.h"
#include "bt_ull_utility.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#include "bt_ull_le_utility.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "bt_ull_le_hid_service.h"
#include "bt_ull_le_utility.h"
#endif
log_create_module(ULL, PRINT_LEVEL_INFO);

bt_status_t bt_ull_init(bt_ull_role_t role, bt_ull_callback callback)
{
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    /* init bt ull 1.0 module */
    bt_ull_srv_init(role, callback);
#endif
  
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    /* init bt ull 2.0 module */
    bt_ull_le_srv_init(role, callback);
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    bt_ull_le_hid_srv_init(role, callback);
#endif

    /* init atci cmd module */
    bt_ull_atci_init();

    return BT_STATUS_SUCCESS;
}


bt_status_t bt_ull_action(bt_ull_action_t action, const void *param, uint32_t param_len)
{    
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_srv_action(action, param, param_len);
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_le_srv_action(action, param, param_len);
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        bt_ull_le_hid_srv_action(action, param, param_len);
#endif


    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info)
{
    bt_status_t status = BT_STATUS_SUCCESS;

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    if (bt_ull_le_service_is_connected()) {
        status = bt_ull_le_srv_get_streaming_info(streaming, info);
        return status;
    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    status = bt_ull_srv_get_streaming_info(streaming, info);
#endif

    return status;
}


void bt_ull_lock_streaming(bool lock)
{
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_le_srv_lock_streaming(lock);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_srv_lock_streaming(lock);
#endif
}


bt_status_t bt_ull_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length)
{
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    if (bt_ull_le_service_is_connected()) {
        return bt_ull_le_srv_write_raw_pcm_data(streaming, data, length);
    }
#endif
    
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (ctx->is_ull_connected) {
        return bt_ull_srv_write_raw_pcm_data(streaming, data, length);
    }
#endif
    return BT_STATUS_FAIL;
}

uint32_t bt_ull_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length)
{
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    if (bt_ull_le_service_is_connected()) {
        return bt_ull_le_srv_get_raw_pcm_data(streaming, buffer, buffer_length);
    }
#endif
        
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (ctx->is_ull_connected) {
        return bt_ull_srv_get_raw_pcm_data(streaming, buffer, buffer_length);
    }
#endif
    return 0;
}

bool bt_ull_is_transmitter_start(bt_ull_streaming_t *streaming)
{
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    if (bt_ull_le_service_is_connected()) {
        return bt_ull_le_srv_is_transmitter_start(streaming);
    }
#endif
        
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (ctx->is_ull_connected) {
        return bt_ull_srv_is_transmitter_start(streaming);
    }
#endif
    return false;
}


