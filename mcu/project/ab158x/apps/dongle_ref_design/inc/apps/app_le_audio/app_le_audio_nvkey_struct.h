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

#ifndef __APP_LE_AUDIO_NVKEY_STRUCT_H__
#define __APP_LE_AUDIO_NVKEY_STRUCT_H__

#include "app_le_audio_ucst_utillity.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
/* NvkeyDefine NVID_BT_LEA_BONDED_LIST */
/* NVID_BT_LEA_BONDED_LIST: 36 = | bonded_device * APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM (4) | */
/* bonded_device: | addr (7 bytes) + group_id (1 byte) + group_size(1 byte) | */
#define APP_LE_AUDIO_NVKEY_BONDED_LIST_LEN  36

/* NvkeyDefine NVID_BT_LEA_ACTIVE_GROUP */
/* NVID_BT_LEA_ACTIVE_GROUP: 1 = | active_group (1 byte) | */
#define APP_LE_AUDIO_NVKEY_ACTIVE_GROUP_LEN 1

/* NvkeyDefine NVID_BT_LEA_SIRK_LIST */
/* NVID_BT_LEA_SIRK_LIST: 34 = (| SIRK (16 bytes) + group_id (1 byte) |) * 2 */
#define APP_LE_AUDIO_NVKEY_SIRK_LIST_LEN        34
#define APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM    2

/**************************************************************************************************
* Structure
**************************************************************************************************/
/* NvkeyDefine NVID_BT_LEA_BONDED_LIST*/
BT_PACKED(
typedef struct {
    bt_addr_t addr;
    uint8_t group_id;
    uint8_t group_size;
}) bt_lea_bonded_list_nvkey_t;

BT_PACKED(
typedef struct {
    bt_lea_bonded_list_nvkey_t device[APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM];    /* bonded device list */
}) bt_lea_ucst_bonded_list_nvkey_t;

BT_PACKED(
typedef struct {
    bt_key_t sirk;
    uint8_t group_id;
}) bt_lea_sink_info_nvkey_t;

BT_PACKED(
typedef struct {
    bt_lea_sink_info_nvkey_t sirk_info[APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM];
}) bt_lea_ucst_sirk_list_nvkey_t;


#endif /*__APP_LE_AUDIO_NVKEY_STRUCT_H__*/

