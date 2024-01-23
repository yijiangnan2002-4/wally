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

#ifndef __BLE_GMAS_DEF_H__
#define __BLE_GMAS_DEF_H__

#include "bt_type.h"

/**
 * @brief The GMAS service UUID.
 */
#define BT_SIG_UUID16_GMAS                  (0x1858)    /**< Gaming audio service. */
#define BT_SIG_UUID16_GMAS_GMAP_ROLE        (0x2C00)    /**< GMAP role. */

#define BT_SIG_UUID16_GMAS_UGG_FEATURE      (0x2C01)    /**< GMAS UGG Feature. */
#define BT_SIG_UUID16_GMAS_UGT_FEATURE      (0x2C02)    /**< GMAS UGT Feature. */
#define BT_SIG_UUID16_GMAS_BGS_FEATURE      (0x2C03)    /**< GMAS BGS Feature. */
#define BT_SIG_UUID16_GMAS_BGR_FEATURE      (0x2C04)    /**< GMAS BGR Feature. */

/**
 * @brief The GMAP role.
 */
#define BLE_GMAP_ROLE_MASK_UGG   0x01  /* Unicast Game Gateway */
#define BLE_GMAP_ROLE_MASK_UGT   0x02  /* Unicast Game Teminal */
#define BLE_GMAP_ROLE_MASK_BGS   0x04  /* Broadcast Game Sender */
#define BLE_GMAP_ROLE_MASK_BGR   0x08  /* Broadcast Game Receiver */
#define BLE_GMAP_ROLE_MASK       0x0F
typedef uint8_t ble_gmap_role_t;

/**
 * @brief The GMAS UGG Features.
 */
#define BLE_GMAS_UGG_FEATURE_MULTIFRAME        0x01  /* UGG Multiframe feature support */
#define BLE_GMAS_UGG_FEATURE_96K_SOURCE        0x02  /* UGG 96 kbps Source feature support */
#define BLE_GMAS_UGG_FEATURE_MULTI_SINK        0x04  /* UGG Multi Sink feature support */
#define BLE_GMAS_UGG_FEATURE_MASK              0x07

/**
 * @brief The GMAS UGT Features.
 */
#define BLE_GMAS_UGT_FEATURE_SOURCE            0x01  /* UGT Source feature support */
#define BLE_GMAS_UGT_FEATURE_80K_SOURCE        0x02  /* UGT 80 kbps Source feature support */
#define BLE_GMAS_UGT_FEATURE_SINK              0x04  /* UGT Sink feature support */
#define BLE_GMAS_UGT_FEATURE_64K_SINK          0x08  /* UGT 64 kbps Sink feature support */
#define BLE_GMAS_UGT_FEATURE_MULTIFRAME        0x10  /* UGT Multiframe feature support */
#define BLE_GMAS_UGT_FEATURE_MULTI_SINK        0x20  /* UGT Multi Sink feature support */
#define BLE_GMAS_UGT_FEATURE_MULTI_SOURCE      0x40  /* UGT Multi Source feature support */
#define BLE_GMAS_UGT_FEATURE_MASK              0x7F

/**
 * @brief The GMAS BGS Features.
 */
#define BLE_GMAS_BGS_FEATURE_96K               0x01  /* BGS 96 kbps feature support */
#define BLE_GMAS_BGS_FEATURE_MASK              0x01

/**
 * @brief The GMAS BGR Features.
 */
#define BLE_GMAS_BGR_FEATURE_MULTI_SINK        0x01  /* BGR Multi Sink feature support */
#define BLE_GMAS_BGR_FEATURE_MULTIFRAME        0x02  /* BGR Multiframe feature support  */
#define BLE_GMAS_BGR_FEATURE_MASK              0x03

/**
 * @brief The GMAS GATT req.
 */
#define BLE_GMAS_READ_ROLE                     0x00    /**< Read GMAP Role */
#define BLE_GMAS_READ_UGG_FEATURE              0x01    /**< Read UGG Feature */
#define BLE_GMAS_READ_UGT_FEATURE              0x02    /**< Read UGT Feature */
#define BLE_GMAS_READ_BGS_FEATURE              0x03    /**< Read BGS Feature */
#define BLE_GMAS_READ_BGR_FEATURE              0x04    /**< Read BGR Feature */
#define BLE_GMAS_REQ_MAX                       0x05
typedef uint8_t ble_gmas_gatt_request_t;


/**
 * @brief                       This function is using to set the GMAS feature.
 * @param[in] att_hdl           is the ATT handle.
 * @param[in] gmas_role         is the GMAP role.
 * @param[in] data              is the feature.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_gmas_set_feature(uint16_t att_hdl, ble_gmap_role_t role, uint8_t feature);

/**
 * @brief                       This function distributes GMAS request to corresponded handler from specified remote device.
 * @param[in] req               is the type of GMAS request.
 * @param[in] conn_hdl          is the connection handle of the Bluetooth link.
 * @param[in] data              is the data.
 * @param[in] size              is the size of data.
 * @param[in] offset            is the offset.
 * @return                      the size of responded data.
 *
 */
uint32_t ble_gmas_gatt_request_handler(ble_gmas_gatt_request_t req, bt_handle_t conn_hdl, uint8_t *data, uint16_t size, uint16_t offset);

#endif  /* __BLE_GMAS_DEF_H__ */
