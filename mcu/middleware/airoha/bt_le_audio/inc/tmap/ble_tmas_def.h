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

#ifndef __BLE_TMAS_DEF_H__
#define __BLE_TMAS_DEF_H__

#include "bt_type.h"

/**
 * @brief The TMAS service UUID.
 */
#define BT_SIG_UUID16_TMAS       (0x1855)    /**< Telephony and media audio service. */

/**
 * @brief The TMAP role.
 */
#define BLE_TMAP_ROLE_MASK_CG    0x0001  /* Call Gateway */
#define BLE_TMAP_ROLE_MASK_CT    0x0002  /* Call Terminal */
#define BLE_TMAP_ROLE_MASK_UMS   0x0004  /* Unicast Media Sender */
#define BLE_TMAP_ROLE_MASK_UMR   0x0008  /* Unicast Media Receiver */
#define BLE_TMAP_ROLE_MASK_BMS   0x0010  /* Broadcast Media Sender */
#define BLE_TMAP_ROLE_MASK_BMR   0x0020  /* Broadcast Media Receiver */
#define BLE_TMAP_ROLE_MASK_BSA   0x0040  /* Broadcast Scan Assistant */
#define BLE_TMAP_ROLE_MASK_BSD   0x0080  /* Broadcast Scan Delegator */
#define BLE_TMAP_ROLE_MASK_RFU   0xFF00  /* RFU */
typedef uint16_t ble_tmap_role_t;

/**
 * @brief The TMAS UUID type definitions.
 */
#define BLE_TMAS_UUID_TYPE_TMAS                           0x00    /**< TMAS. */
#define BLE_TMAS_UUID_TYPE_ROLE                           0x01    /**< Role. */
#define BLE_TMAS_UUID_TYPE_MAX_NUM                        0x02    /**< The max number of TMAS UUID type.*/
#define BLE_TMAS_UUID_TYPE_INVALID                        0xFF    /**< The invalid TMAS UUID type.*/
typedef uint8_t ble_tmas_uuid_t;                                  /**< The TMAS UUID type.*/

#define BLE_TMAS_UUID_TYPE_CHARC_START       BLE_TMAS_UUID_TYPE_ROLE

/**
 * @brief The TMAS max number of characteristics.
 */
#define BLE_TMAS_MAX_CHARC_NUMBER            (BLE_TMAS_UUID_TYPE_MAX_NUM - 1)  /**< The max number of TMAS characteristics.*/

#endif  /* __BLE_TMAS_DEF_H__ */

