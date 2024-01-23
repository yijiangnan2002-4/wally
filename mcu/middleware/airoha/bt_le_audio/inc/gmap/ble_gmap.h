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

#ifndef __BLE_GMAP_H__
#define __BLE_GMAP_H__

#include "bt_type.h"

/**
 * @defgroup Bluetoothble_GMAP_define Define
 * @{
 * This section defines the GMAP opcode and error codes.
 */

#define BLE_GMAP_READ_ROLE_CNF                        0x01    /**< The result of role, with #ble_gmap_discover_role_cnf_t as the payload in the callback function. */
#define BLE_GMAP_DISCOVER_SERVICE_COMPLETE_NOTIFY     0x02    /**< The notification when discover all GMAS instances complete, with #ble_gmap_discover_service_complete_t as the payload in the callback function. */
typedef uint8_t ble_gmap_event_t;                             /**< The type of GMAP events.*/

/**
 *  @brief This structure defines the parameter data type for event #BLE_GMAP_DISCOVER_SERVICE_COMPLETE_NOTIFY.
 */
typedef struct {
    bt_handle_t handle;         /**< Connection handle. */
    bt_status_t status;         /**< Event status. */
} ble_gmap_discover_service_complete_t;

typedef void (*ble_gmap_callback_t)(ble_gmap_event_t event, void *msg);

/**
 *  @brief This structure defines the parameter data type for GMAP event #BLE_GMAP_READ_ROLE_CNF.
 */
typedef struct {
    bt_handle_t handle;         /**< Connection handle. */
    uint8_t role;               /**< The role.*/
    uint8_t ugg_feature;        /**< The UGG feature.*/
    uint8_t ugt_feature;        /**< The UGT feature.*/
    uint8_t bgs_feature;        /**< The BGS feature.*/
    uint8_t bgr_feature;        /**< The BGR feature.*/
} ble_gmap_read_role_cnf_t;


bt_status_t ble_gmap_init(ble_gmap_callback_t callback, uint8_t max_link_num);

#endif  /* __BLE_GMAP_H__ */
