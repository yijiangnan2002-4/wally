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
#ifndef __BLE_GMAP_DISCOVERY_H__
#define __BLE_GMAP_DISCOVERY_H__

#include "ble_gmas_def.h"
#include "bt_gattc_discovery.h"

/**
 * @brief The GMAP set attribute callback.
 */
typedef void (*ble_gmap_set_attribute_callback_t)(void);

/**
 *  @brief This structure defines the initial conditions for the service.
 */
typedef struct {
    uint16_t uuid;                  /**< UUID of characteristic.*/
    uint16_t value_handle;          /**< The handle of characteristic value.*/
    uint16_t desc_handle;           /**< The handle of descriptor.*/
} ble_gmap_characteristic_t;

/**
* @brief The parameter of #ble_gmap_set_service_attribute.
*/
typedef struct {
    uint16_t start_handle;          /**< The start handle of service.*/
    uint16_t end_handle;            /**< The end handle of service.*/
    bool is_complete;               /**< Indicates the discovery flow is completed or not. */
    uint8_t charc_num;              /**< The total count of characteristics in service.*/
    ble_gmap_characteristic_t *charc;  /**< The characteristic information of GMAS. */
    ble_gmap_set_attribute_callback_t callback;  /**< The callback is invoked when finish setting GMAP attribute. */
} ble_gmap_set_service_attribute_parameter_t;

/**
 * @}
 */

/**
 * @brief                       This function sets the service attribute of GMAP.
 * @param[in] handle            is the connection handle of GMAS.
 * @param[in] params            is the attribute information of GMAS.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_gmap_set_service_attribute(bt_gattc_discovery_event_t *event);

/**
 * @}
 * @}
 * @}
 */

#endif  /* __BLE_GMAP_DISCOVERY_H__ */
