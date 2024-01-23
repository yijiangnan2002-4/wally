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


#ifndef __BT_GAP_EXT_H__
#define __BT_GAP_EXT_H__

#include "bt_type.h"

#include "bt_gap.h"

#define BT_GAP_SCO_TYPE_SCO         (0x00)   /**< SCO link type. */
#define BT_GAP_SCO_TYPE_ESCO        (0x02)   /**< ESCO link type. */
typedef uint8_t bt_gap_sco_type_t;

#define BT_GAP_SCO_STATUS_INACTIVE  (0)
#define BT_GAP_SCO_STATUS_ACTIVE    (1)
typedef uint8_t bt_gap_sco_status_t;

#define BT_GAP_mHDT_RATE_1M      (0x01)
#define BT_GAP_mHDT_RATE_2M      (0x02)
#define BT_GAP_mHDT_RATE_3M      (0x04)
#define BT_GAP_mHDT_RATE_EDR4    (0x08)
#define BT_GAP_mHDT_RATE_EDR6    (0x10)
#define BT_GAP_mHDT_RATE_EDR8    (0x20)

#define BT_GAP_SCO_STATUS_DISCONNECTED    (0x00)
#define BT_GAP_SCO_STATUS_DISCONNECTNG    (0x01)
#define BT_GAP_SCO_STATUS_CONNECTNG       (0x02)
#define BT_GAP_SCO_STATUS_CONNECTED       (0x03)

BT_PACKED(
typedef struct {
    uint32_t transmit_band_with;
    uint32_t receive_band_with;
    uint16_t max_latency;
    uint16_t voice_setting;
    uint8_t  retransmission_effort;
    uint16_t packet_type;
    uint8_t  tx_coding_format_byte_0;             //for esco only
    uint32_t tx_coding_format_byte_1_4;           //for esco only
    uint8_t  rx_coding_format_byte_0;             //for esco only
    uint32_t rx_coding_format_byte_1_4;           //for esco only
}) bt_gap_accept_sco_params_t;

typedef struct {
    bt_bd_addr_t           *address;
    bt_handle_t             handle;
    bt_gap_sco_type_t       sco_type;
    bt_gap_sco_status_t     active;
    uint8_t                 air_mode;
} bt_gap_sco_info_t;

typedef enum {
    BT_GAP_SCO_EVT_CONNECTED = 1,
    BT_GAP_SCO_EVT_PARAM_REQ,
    BT_GAP_SCO_EVT_DISCONNECTED,
    BT_GAP_SCO_EVT_INACTIVE,
    BT_GAP_SCO_EVT_ACTIVE,
    BT_GAP_SCO_EVT_CONNECT_FAIL,
    BT_GAP_SCO_EVT_REQUEST
} bt_gap_sco_evt_t;

typedef struct {
    bt_handle_t handle;
    uint8_t tx_rates;
    uint8_t rx_rates;
} bt_tci_mHDT_mode_t;


typedef struct {
    bt_handle_t handle;
    uint16_t mHDT_timeout;
} bt_tci_write_mHDT_timeout_t;

BT_PACKED(
typedef struct {
    uint8_t status;    
    bt_handle_t handle;
    uint8_t remote_feature;
})bt_tci_mHDT_remote_feature_t;


typedef struct {
    uint8_t status;
    bt_handle_t handle;
    uint8_t mHDT_mode;
    uint8_t tx_rates;
    uint8_t rx_rates;
}bt_tci_mHDT_data_rate_change_t;


typedef struct {
    uint8_t status;
    bt_handle_t handle;
    uint16_t timeout;
}bt_tci_mHDT_timeout_change_t;

BT_PACKED(
typedef struct {
    uint8_t status;
    bt_handle_t handle;
    uint32_t feature;
})bt_tci_mHDT_le_read_remote_feature_complete_t;

typedef struct {
    uint8_t status;
    bt_handle_t handle;
    uint16_t timeout;
}bt_tci_mHDT_read_timeout_complete_t;

typedef struct {
    uint8_t status;
    uint32_t feature;
}bt_tci_mHDT_le_read_loacal_feature_t;

bt_status_t bt_gap_disconnect_sco(bt_handle_t sco_handle);
bt_status_t bt_gap_connect_sco(const bt_bd_addr_t *address, bt_gap_sco_type_t type);
bt_status_t bt_gap_set_sco_status(bt_handle_t hci_handle, bt_gap_sco_status_t active_status);
bt_handle_t bt_gap_find_sco_handle_by_connection(bt_gap_connection_handle_t conn_handle);
bt_status_t bt_gap_sniff_lock(bt_gap_connection_handle_t conn_handle);
bt_status_t bt_gap_sniff_unlock(bt_gap_connection_handle_t conn_handle);
uint8_t     bt_gap_get_local_mHDT_feature();
uint8_t     bt_gap_get_remote_mHDT_feature_by_address(const bt_bd_addr_t *addr);

bt_status_t bt_hsp_sco_callback(bt_gap_sco_evt_t evt, const bt_gap_sco_info_t *sco_status, bt_gap_accept_sco_params_t *param);
bt_status_t bt_hfp_sco_callback(bt_gap_sco_evt_t evt, const bt_gap_sco_info_t *sco_status, bt_gap_accept_sco_params_t *param);

bt_status_t bt_gap_tci_read_local_mHDT_feature();
bt_status_t bt_gap_tci_read_remote_mHDT_feature(bt_handle_t handle);
bt_status_t bt_gap_tci_mHDT_mode(bt_tci_mHDT_mode_t mHDT_param);
bt_status_t bt_gap_tci_exit_mHDT_mode(bt_handle_t handle);
bt_status_t bt_gap_tci_write_mHDT_timeout(bt_tci_write_mHDT_timeout_t write_timeout);
bt_status_t bt_gap_tci_read_mHDT_timeout(bt_handle_t handle);
bt_status_t bt_gap_tci_le_read_local_mtk_proprietary_feature();
bt_status_t bt_gap_tci_le_read_remote_mtk_proprietary_feature(bt_handle_t handle);
bt_status_t bt_gap_tci_mHDT_callback(uint32_t is_timeout, uint32_t timer_id, uint32_t data, const void *arg);

#endif /* __BT_GAP_EXT_H__ */


