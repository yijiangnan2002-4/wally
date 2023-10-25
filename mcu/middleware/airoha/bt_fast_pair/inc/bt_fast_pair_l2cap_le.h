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

#ifndef __BT_FAST_PAIR_L2CAP_LE_H__
#define __BT_FAST_PAIR_L2CAP_LE_H__

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
#include "bt_l2cap_le.h"
#endif
#include "bt_type.h"
#include "bt_fast_pair.h"

#define FAST_PAIR_COC_MAXIMUM                       (0x03)      /**< The maximum COC connection number. */
#define FAST_PAIR_L2CAP_LE_PSM_VALUE                (0x0090)

/**
 *  @brief Defines the fast_pair COC event.
 */
#define BT_FAST_PAIR_L2CAP_LE_CONNECTED_IND                       0   /**< This event indicates the connection oriented channel connected #bt_fast_pair_coc_connected_ind_t. */
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_IND                    2   /**< This event indicates the connection oriented channel disconnected #bt_fast_pair_coc_disconnected_ind_t. */
#define BT_FAST_PAIR_L2CAP_LE_DATA_IN_IND                         3   /**< This event indicates the connection oriented channel data in #bt_fast_pair_coc_data_in_ind_t. */
typedef uint8_t bt_fast_pair_l2cap_le_event_type_t;    /**< The type of fast_pair COC events.*/

/**
 *  @brief Defines the disconnected reason.
 */
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_LOCAL_REQUEST                    0x00    /**< Local request.*/
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_DUPLICATED_CID                   0x01    /**< Duplicated CID.*/
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_CREDIT_OVERFLOW                  0x02    /**< Credit overflow.*/
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_CREDIT_UNDERFLOW                 0x03    /**< Credit underflow.*/
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_INVALID_MTU_CONFIGURATION        0x04    /**< Invalid MTU configuration.*/
#define BT_FAST_PAIR_L2CAP_LE_DISCONNECTED_DUE_TO_REMOTE_REQUEST                   0x05    /**< Remote request.*/
typedef uint8_t bt_fast_pair_l2cap_le_disconnect_reason_t;     /**< The type of disconnected reason.*/

/**
 * @defgroup Bluetoothble_fast_pair_l2cap_le_struct Struct
 * @{
 * This section defines basic data structures for the fast_pair_COC.
 */

/**
 *  @brief This structure defines the parameter data type for event #BT_fast_pair_COC_CONNECTED_IND.
 */
typedef struct {
    bt_bd_addr_t *addr;          /**< Remote device address.*/
} bt_fast_pair_l2cap_le_connected_ind_t;


/**
 *  @brief This structure defines the parameter data type for event #BT_fast_pair_COC_DISCONNECTED_IND.
 */
typedef struct {
    bt_bd_addr_t *addr;          /**< Remote device address.*/
    bt_fast_pair_l2cap_le_disconnect_reason_t  reason;     /**< Disconnected reason.*/
} bt_fast_pair_l2cap_le_disconnected_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BT_fast_pair_COC_DATA_IN_IND.
 */
typedef struct {
    bt_bd_addr_t *addr;          /**< Remote device address.*/
    uint32_t    data_length;    /**< Data length.*/
    uint8_t     *p_data;        /**< Data.*/
} bt_fast_pair_l2cap_le_data_in_ind_t;

typedef void (* bt_fast_pair_l2cap_le_callback_t)(bt_fast_pair_l2cap_le_event_type_t event, const void *parameter);
/**
 * @}
 */

/**
 * @brief                       This function register the l2cap LE transfer profile.
 * @param[in] callback          is the callback.
 * @param[in] max_link          is the supported maximum link.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_fast_pair_l2cap_le_init(bt_fast_pair_l2cap_le_callback_t callback);

/**
 * @brief                       This function send data.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] remote_cid        is the CID of remote.
 * @param[in] data              is the data.
 * @param[in] length            is the data length.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_fast_pair_l2cap_le_send_data(bt_bd_addr_t *addr, void *data, uint32_t length);

#endif  /*__BT_fast_pair_COC_H__ */

