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

#ifndef __BT_DEVICE_MANAGER_GATT_CACHE_H__
#define __BT_DEVICE_MANAGER_GATT_CACHE_H__
/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_DM_Gatt_Cache BT_Device_Manager_Gatt_Cache
 * @{
 * This section defines the Bluetooth BT_Gatt_Cache API to store or find GATT service during discovery.
 * With the GATT caching, time is saved and a significant number of packets need not be exchanged between the client and server.
 * One GATT cache corresponds to one device.
 *
 * @section device_manager_api_usage How to use this module
 * - Step 1. Initialize the device manager GATT cache at boot up.
 *  - Sample Code:
 *     @code
 *                bt_device_manager_gatt_cache_init();
 *     @endcode
 * - Step 2. Update the device manager GATT cache.
 *  - Sample code:
 *     @code
 *               bt_bd_addr_t address = {0x00, 0x33, 0x63, 0x56, 0xE0, 0x4C};
 *               const bt_bd_addr_t *peer_addr = &address;
 *               bt_gattc_discovery_service_t service;
 *               bt_device_manager_gatt_cache_update(peer_addr, &service);
 *     @endcode
 * - Step 3. Store device manager GATT cache.
 *  - Sample code:
 *     @code
 *               bt_bd_addr_t address = {0x00, 0x33, 0x63, 0x56, 0xE0, 0x4C};
 *               const bt_bd_addr_t *peer_addr = &address;
 *               bt_device_manager_gatt_cache_store(peer_addr);
 *     @endcode
 */

#include "bt_type.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_config.h"
#include "bt_gattc_discovery.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @defgroup BT_device_manager_GATT_cache_define Define
 * @{
 */

/**
 * @brief Define Bluetooth device manager GATT cache values.
 */
#define BT_DM_GATT_CACHE_MAX_RECORD_NUM         (0x04)    /**< The maximum number of NVDM records. */
#define BT_DM_GATT_CACHE_MAX_SIZE               (1000)    /**< The maximum size of each GATT cache. */

/**
 * @brief Define the GATT cache status.
 */
#define BT_GATT_CACHE_STATUS_SUCCESS            (0)       /**< The result of GATT cache operation is success. */
#define BT_GATT_CACHE_STATUS_ERROR              (1)       /**< The result of GATT cache operation is error. */
#define BT_GATT_CACHE_STATUS_NOT_COMPLETE       (2)       /**< The GATT cache process for finding the UUID is not complete, the same UUID still exists in the GATT cache. */
#define BT_GATT_CACHE_STATUS_NOT_FIND_ADDR      (3)       /**< Address not found during GATT caching. */
#define BT_GATT_CACHE_STATUS_NOT_FIND_UUID      (4)       /**< UUID not found during GATT caching. */
typedef uint8_t bt_gatt_cache_status_t;
/**
 * @}
*/

/**
 * @brief         This function is used to initialize the GATT cache when Bluetooth powers on successfully, it reads the sequence and GATT cache stored in NVDM to a global list.
 * @return        None.
 */
void bt_device_manager_gatt_cache_init(void);

/**
 * @brief      This function is used to update the GATT service context to the global list when a service is discovered.
 * @param[in]  peer_addr    is the address of the device that needs to update the GATT cache.
 * @param[in]  service      is the GATT service context during discovery.
 * @return     bt_gatt_cache_status_t.
 */
bt_gatt_cache_status_t bt_device_manager_gatt_cache_update(const bt_bd_addr_t *addr, const bt_gattc_discovery_service_t *service);

/**
 * @brief      This function is used to store GATT cache and sequence to NVDM when user discovery is complete.
 * @param[in]  addr    is the address of the device that needs to store the GATT cache.
 * @return     bt_gatt_cache_status_t.
 */
bt_gatt_cache_status_t bt_device_manager_gatt_cache_store(const bt_bd_addr_t *addr);

/**
 * @brief      This function is used to delete the GATT cache and sequence in NVDM which is contained in matching address information.
 * @param[in]  addr    is the address of the device that needs to delete the GATT cache.
 * @return     bt_gatt_cache_status_t.
 */
bt_gatt_cache_status_t bt_device_manager_gatt_cache_delete(const bt_bd_addr_t *addr);

/**
 * @brief      This function is used to find and return the context of service by the peer address and UUID.
 * @param[in]  addr    is the device address that needs to be found and returned to the GATT cache.
 * @param[in]  uuid    is the service UUID which GATT cache needs to be find.
 * @param[out]     service    is a pointer to the service, used to fill service information in the GATT cache.
 * @param[in/out]  seq_addr   is a pointer to the read sequence, if bt_gatt_cache_status_t is BT_GATT_CACHE_STATUS_NOT_FINISH, the value of the seq_addr will be updated and the user should use this API again.
 * @return     bt_gatt_cache_status_t.
 */
bt_gatt_cache_status_t bt_device_manager_gatt_cache_find(const bt_bd_addr_t *addr, const ble_uuid_t *uuid, bt_gattc_discovery_service_t *service, uint8_t *seq_addr);

/**
 * @brief      This function is used to clear the GATT cache and sequence in NVDM.
 * @param[in]  none.
 * @return     BT_GATT_CACHE_STATUS_SUCCESS   the GATT cache and sequeue is clear successful.
 *                                            otherwise the operation failed.
 */
bt_gatt_cache_status_t bt_device_manager_gatt_cache_clear(void);
/**
 * @}
 */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __BT_DEVICE_MANAGER_GATT_CACHE_H__ */

