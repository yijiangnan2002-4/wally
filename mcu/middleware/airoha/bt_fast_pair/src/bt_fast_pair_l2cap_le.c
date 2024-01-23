/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
/* Airoha restricted information */

#include "bt_fast_pair_l2cap_le.h"
#include "bt_fast_pair_utility.h"
#include "bt_callback_manager.h"
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
#include "bt_device_manager_le.h"
#include "bt_l2cap_le.h"

typedef struct {
    uint8_t         is_active;
    uint32_t        coc_handle;
    bt_bd_addr_t    address;
} PACKED bt_fast_pair_l2cap_le_cnt_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static bt_fast_pair_l2cap_le_callback_t   bt_fast_pair_l2cap_le_callback = NULL;

static bt_l2cap_le_profile_config_info_t bt_fast_pair_l2cap_le_register_callback;

static bt_fast_pair_l2cap_le_cnt_t        s_fast_pair_coc_cnt[FAST_PAIR_COC_MAXIMUM];
static const bt_bd_addr_t                 empty_address = {0};

/**************************************************************************************************
* Static functions
**************************************************************************************************/
static bt_fast_pair_l2cap_le_cnt_t *bt_fast_pair_l2cap_le_find_empty(void)
{
    for (uint8_t i = 0; i < FAST_PAIR_COC_MAXIMUM; i++) {
        if (s_fast_pair_coc_cnt[i].is_active == 0) {
            return &s_fast_pair_coc_cnt[i];
        }
    }
    return NULL;
}

static bt_fast_pair_l2cap_le_cnt_t *bt_fast_pair_l2cap_le_find_by_handle(uint32_t handle)
{
    for (uint8_t i = 0; i < FAST_PAIR_COC_MAXIMUM; i++) {
        if (handle == s_fast_pair_coc_cnt[i].coc_handle && s_fast_pair_coc_cnt[i].is_active != 0) {
            return &s_fast_pair_coc_cnt[i];
        }
    }
    return NULL;
}

static bt_fast_pair_l2cap_le_cnt_t *bt_fast_pair_l2cap_le_find_by_addr(bt_bd_addr_t *address)
{
    for (uint8_t i = 0; i < FAST_PAIR_COC_MAXIMUM; i++) {
        if ((memcmp(&s_fast_pair_coc_cnt[i].address, address, sizeof(bt_bd_addr_t)) == 0)
            && s_fast_pair_coc_cnt[i].is_active != 0) {
            return &s_fast_pair_coc_cnt[i];
        }
    }
    return NULL;
}

static void bt_fast_pair_l2cap_le_find_ida_addr(bt_bd_addr_t *addr, bt_bd_addr_t *ida_addr)
{
    if (addr == NULL || ida_addr == NULL) {
        return;
    }

    bt_device_manager_le_bonded_info_t *bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(addr);
    if (bond_info != NULL && (memcmp(empty_address, bond_info->info.identity_addr.address.addr, sizeof(bt_bd_addr_t)) != 0)) {
        bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] find_ida_addr not empty.", 0);
        memcpy(*ida_addr, bond_info->info.identity_addr.address.addr, sizeof(bt_bd_addr_t));
    } else {
        bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] find_ida_addr empty.", 0);
        memcpy(*ida_addr, *addr, sizeof(bt_bd_addr_t));
    }


    return;
}

static void bt_fast_pair_l2cap_le_evt_cb(bt_l2cap_le_event_type_t event, const void *parameter)
{
    bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] event %d.", 1, event);
    bt_bd_addr_t ida_addr = {0};
    if (NULL == bt_fast_pair_l2cap_le_callback || NULL == parameter) {
        return;
    }

    switch (event) {
        case BT_L2CAP_LE_CONNECTED_IND: {
            bt_l2cap_le_connected_ind_t *p_evt = (bt_l2cap_le_connected_ind_t *)parameter;
            bt_fast_pair_l2cap_le_connected_ind_t cb_evt;

            memset(&cb_evt, 0, sizeof(bt_fast_pair_l2cap_le_connected_ind_t));
            if (p_evt != NULL) {
                if (BT_HANDLE_INVALID != p_evt->handle) {
                    bt_fast_pair_l2cap_le_cnt_t *temp_dev = bt_fast_pair_l2cap_le_find_empty();
                    if (temp_dev != NULL) {
                        temp_dev->is_active     = 1;
                        temp_dev->coc_handle    = p_evt->handle;
                        bt_fast_pair_l2cap_le_find_ida_addr(&p_evt->address->addr, &ida_addr);
                        memcpy(temp_dev->address, ida_addr, sizeof(bt_bd_addr_t));
                        cb_evt.addr = &ida_addr;
                        bt_fast_pair_l2cap_le_callback(BT_FAST_PAIR_L2CAP_LE_CONNECTED_IND, &cb_evt);
                        bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] connect: handle=0x%x.", 1, p_evt->handle);
                    }
                }
            }
            break;
        }
        case BT_L2CAP_LE_DISCONNECTED_IND: {
            bt_l2cap_le_disconnected_ind_t *p_evt = (bt_l2cap_le_disconnected_ind_t *)parameter;
            bt_fast_pair_l2cap_le_disconnected_ind_t cb_evt;
            memset(&cb_evt, 0, sizeof(bt_fast_pair_l2cap_le_disconnected_ind_t));
            if (p_evt != NULL) {
                bt_fast_pair_l2cap_le_cnt_t *temp_dev = bt_fast_pair_l2cap_le_find_by_handle(p_evt->handle);
                if (temp_dev != NULL) {
                    temp_dev->is_active     = 0;
                    temp_dev->coc_handle    = BT_HANDLE_INVALID;
                    cb_evt.reason = p_evt->reason;
                    cb_evt.addr = &temp_dev->address;
                    bt_fast_pair_l2cap_le_callback(BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_IND, &cb_evt);
                }
                bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] disconnect: reason=0x%x, handle=0x%x.",
                                 2, p_evt->reason, p_evt->handle);
            }
            break;
        }
        case BT_L2CAP_LE_DATA_IND: {
            bt_l2cap_le_data_ind_t *p_evt = (bt_l2cap_le_data_ind_t *)parameter;
            bt_fast_pair_l2cap_le_data_in_ind_t cb_evt;
            bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] data ind: handle=%d, length=%d.", 2, p_evt->handle, p_evt->length);

            bt_fast_pair_l2cap_le_cnt_t *temp_dev = bt_fast_pair_l2cap_le_find_by_handle(p_evt->handle);
            if (temp_dev != NULL) {
                memset(&cb_evt, 0, sizeof(bt_fast_pair_l2cap_le_data_in_ind_t));
                cb_evt.addr = &temp_dev->address;
                cb_evt.data_length  = p_evt->length;
                cb_evt.p_data       = p_evt->data;

                bt_fast_pair_l2cap_le_callback(BT_FAST_PAIR_L2CAP_LE_DATA_IN_IND, &cb_evt);
            }
            break;
        }
        default:
            break;
    }
}
#endif //AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
bt_status_t bt_fast_pair_l2cap_le_init(bt_fast_pair_l2cap_le_callback_t callback)
{
    bt_status_t status = BT_STATUS_FAIL;

    if (NULL == callback) {
        return BT_STATUS_FAIL;
    }
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    bt_fast_pair_l2cap_le_register_callback.psm        = FAST_PAIR_L2CAP_LE_PSM_VALUE;
    bt_fast_pair_l2cap_le_register_callback.mode       = BT_L2CAP_CHANNEL_MODE_CBFCM;
    bt_fast_pair_l2cap_le_register_callback.callback   = bt_fast_pair_l2cap_le_evt_cb;

    if (BT_STATUS_SUCCESS == (status = bt_callback_manager_add_l2cap_le_customer_profile_config(&bt_fast_pair_l2cap_le_register_callback))) {
        bt_fast_pair_l2cap_le_callback = callback;
    }

    bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] init status=0x%x.", 1, status);
#endif //AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    return status;
}

bt_status_t bt_fast_pair_l2cap_le_send_data(bt_bd_addr_t *addr, void *data, uint32_t length)
{
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    bt_status_t ret_status = BT_STATUS_SUCCESS;
    bt_fast_pair_l2cap_le_cnt_t *coc_dev = NULL;

    if (NULL != addr) {
        bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] send by address 0x%x", 1, *(uint32_t *)addr);
        if ((NULL != (coc_dev = bt_fast_pair_l2cap_le_find_by_addr(addr))) && (0 != coc_dev->is_active)) {
            if (BT_STATUS_SUCCESS == (ret_status = bt_l2cap_le_send_data(coc_dev->coc_handle, data, length))) {
                return BT_STATUS_SUCCESS;
            }
            bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] send failed 0x%x", 1, ret_status);
        }
        bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] send found fail can't find device", 0);
        return BT_STATUS_FAIL;
    }

    for (uint8_t i = 0; i < FAST_PAIR_COC_MAXIMUM; i++) {
        if (BT_HANDLE_INVALID != s_fast_pair_coc_cnt[i].coc_handle) {
            if (BT_STATUS_SUCCESS != (ret_status = bt_l2cap_le_send_data(s_fast_pair_coc_cnt[i].coc_handle, data, length))) {
                bt_fast_pair_log("[BT_FAST_PAIR][L2CAP] send failed 0x%x", 1, ret_status);
            }
        }
    }
#endif //AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    //bt_l2cap_le_disconnect(0);
    return BT_STATUS_SUCCESS;
}

