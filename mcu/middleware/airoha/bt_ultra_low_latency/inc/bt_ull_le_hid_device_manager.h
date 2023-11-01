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
#ifndef __BT_ULL_LE_HID_DM_DEVICE_MANAGER_H__
#define __BT_ULL_LE_HID_DM_DEVICE_MANAGER_H__
#include "bt_type.h"
#include "bt_ull_le_hid_service.h"
#include "bt_ull_utility.h"

#define BT_ULL_LE_HID_DM_UNI_AA_LEN            4
#define BT_ULL_LE_HID_DM_LTK_LEN               16
#define BT_ULL_LE_HID_DM_SKD_LEN               16
#define BT_ULL_LE_HID_DM_IV_LEN                8
#define BT_ULL_LE_HID_DM_DEVICE_NUM_MAX        1


typedef struct {
    uint8_t                 device_type;
    uint8_t                 device_characterization;   /*rename*/
    bt_ull_le_codec_t       codec_type;
    bt_ull_le_srv_client_preferred_codec_param codec_param;
    bt_addr_t               addr;
    uint8_t                 uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN];
    uint8_t                 ltk[BT_ULL_LE_HID_DM_LTK_LEN];
    uint8_t                 skd[BT_ULL_LE_HID_DM_SKD_LEN];
    uint8_t                 iv[BT_ULL_LE_HID_DM_IV_LEN];
} PACKED bt_ull_le_hid_dm_device_info_t;

bt_status_t bt_ull_le_hid_dm_init(void);
bt_ull_le_hid_dm_device_info_t *bt_ull_le_hid_dm_read_device_info(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr);
bt_status_t bt_ull_le_hid_dm_write_device_info(bt_ull_le_hid_dm_device_info_t *info);
bt_status_t bt_ull_le_hid_dm_delete_device_info(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr);
bt_status_t bt_ull_le_hid_dm_shift_device_front(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr);
uint8_t bt_ull_le_hid_dm_get_bonded_device_num(bt_ull_le_hid_srv_device_t device_type);
void bt_ull_le_hid_dm_get_bonded_device_list(bt_ull_le_hid_srv_device_t device_type, uint8_t count, bt_addr_t *list);
bool bt_ull_le_hid_dm_is_bonded_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr);
uint8_t *bt_ull_le_hid_dm_generate_uni_aa(void);
uint8_t *bt_ull_le_hid_dm_get_ltk(void);
uint8_t *bt_ull_le_hid_dm_get_skd(void);
uint8_t *bt_ull_le_hid_dm_get_iv(void);
bt_status_t bt_ull_le_hid_dm_clear_bonded_list(bt_ull_le_hid_srv_device_t device_type);

#endif

