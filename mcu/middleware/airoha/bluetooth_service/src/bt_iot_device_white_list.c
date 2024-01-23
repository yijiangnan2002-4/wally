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


#include "bt_device_manager_internal.h"
#include "bt_iot_device_white_list.h"

#define BT_IOT_DEVICE_INVALIDE_ITEM1    (0xFF)
#define BT_IOT_DEVICE_INVALIDE_ITEM     (0xFFFF)
#define BT_IOT_DEVICE_INVALIDE_COD      (0x00000000)

#define BT_IOT_DEVICE_VERSION_MANUFACTORY_LENOVO_X250   (0x0002)
#define BT_IOT_DEVICE_VERSION_SUBVERSION_LENOVO_X250    (0x1000)

#define BT_IOT_DEVICE_PNP_VENDOR_MICROSOFT              (0x0006)
#define BT_IOT_DEVICE_PNP_VENDOR_BROADCOM               (0x000F)
#define BT_IOT_DEVICE_PNP_VENDOR_QUALCOMM               (0x001D)
#define BT_IOT_DEVICE_PNP_VENDOR_MEDIATEK               (0x0046)
#define BT_IOT_DEVICE_PNP_VENDOR_BLUEGIGA               (0x0047)
#define BT_IOT_DEVICE_PNP_VENDOR_GOOGLE                 (0x00E0)
#define BT_IOT_DEVICE_PNP_VENDOR_HISILICON              (0x010F)
#define BT_IOT_DEVICE_PNP_VENDOR_XIAOMI                 (0x038F)
#define BT_IOT_DEVICE_PNP_VENDOR_CAREFREE               (0x054C)
#define BT_IOT_DEVICE_PNP_VENDOR_GN                     (0x0067)
#define BT_IOT_DEVICE_PNP_VENDOR_APPLE                  (0x004C)
#define BT_IOT_DEVICE_PNP_VENDOR_SAMSUNG                (0x0075)


#define BT_IOT_DEVICE_NAME_NW_ZX2       "NW-ZX2"

const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO01 = {0x0001, BT_IOT_DEVICE_PNP_VENDOR_MICROSOFT, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO02 = {0x1200, BT_IOT_DEVICE_PNP_VENDOR_XIAOMI, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO03 = {0x1200, BT_IOT_DEVICE_PNP_VENDOR_QUALCOMM, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO04 = {0x1200, BT_IOT_DEVICE_PNP_VENDOR_BROADCOM, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO05 = {0x1200, BT_IOT_DEVICE_PNP_VENDOR_MEDIATEK, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO06 = {0xf000, BT_IOT_DEVICE_PNP_VENDOR_BLUEGIGA, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO07 = {0x107e, BT_IOT_DEVICE_PNP_VENDOR_HISILICON, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO08 = {0x107e, BT_IOT_DEVICE_PNP_VENDOR_GOOGLE, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO09 = {0x004c, BT_IOT_DEVICE_INVALIDE_ITEM, 0x000d};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0A = {BT_IOT_DEVICE_INVALIDE_ITEM, BT_IOT_DEVICE_PNP_VENDOR_CAREFREE, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0B = {0x2462, BT_IOT_DEVICE_PNP_VENDOR_GOOGLE, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0C = {0x24b9, BT_IOT_DEVICE_PNP_VENDOR_GN, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0D = {0x200B, BT_IOT_DEVICE_PNP_VENDOR_APPLE, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0E = {0x200E, BT_IOT_DEVICE_PNP_VENDOR_APPLE, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO0F = {0x0100, BT_IOT_DEVICE_PNP_VENDOR_SAMSUNG, BT_IOT_DEVICE_INVALIDE_ITEM};
const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO10 = {BT_IOT_DEVICE_INVALIDE_ITEM, BT_IOT_DEVICE_PNP_VENDOR_APPLE, BT_IOT_DEVICE_INVALIDE_ITEM};

const bt_device_manager_db_remote_pnp_info_t BT_IOT_DEVICE_PNP_INFO11 = {BT_IOT_DEVICE_INVALIDE_ITEM, BT_IOT_DEVICE_PNP_VENDOR_GN, BT_IOT_DEVICE_INVALIDE_ITEM};


const bt_device_manager_db_remote_version_info_t BT_IOT_DEVICE_VERSION_INFO01 = {BT_IOT_DEVICE_INVALIDE_ITEM1, BT_IOT_DEVICE_VERSION_MANUFACTORY_LENOVO_X250, BT_IOT_DEVICE_VERSION_SUBVERSION_LENOVO_X250};

typedef struct {
    const char      *device_name;
    const bt_device_manager_db_remote_version_info_t    *version_info;
    const bt_device_manager_db_remote_pnp_info_t        *pnp_info;
    const uint32_t  cod;
    bt_iot_device_white_list_iot_case_t all_iot_case[MAX_IOT_CASE];
} bt_iot_device_all_iot_case_t;

const bt_iot_device_all_iot_case_t g_all_iot_case_list[] = {
    {NULL, NULL, NULL, BT_IOT_DEVICE_INVALIDE_COD, {0}},
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO01, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET, BT_IOT_MUSIC_SET_LATENCY_TO_250, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO02, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET, BT_IOT_MUSIC_SET_LATENCY_TO_250, BT_IOT_MUSIC_SEND_AVRCP_FORCE_PAUSE, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO03, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LATENCY_TO_250}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO04, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LATENCY_TO_250}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO05, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_SPECIAL_MTU, BT_IOT_MUSIC_SET_LATENCY_TO_170}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO06, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LATENCY_TO_ORIGINAL}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO07, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LATENCY_TO_250}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO08, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LATENCY_TO_250}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO09, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_IGNORE_ABSOLUTE_VOLUME}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0A, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_IS_WALKMAN_WITH_SBC_CODEC_AND_NO_HFP, BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION}
    },
    {
        BT_IOT_DEVICE_NAME_NW_ZX2, NULL, NULL, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_SPECIAL_SAMPLE_COUNT_TO_MAGIC_NUMBER, BT_IOT_MUSIC_SET_LOCAL_ASI}
    },
    {
        BT_IOT_DEVICE_NAME_NW_ZX2, NULL, &BT_IOT_DEVICE_PNP_INFO0A, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_SPECIAL_SAMPLE_COUNT_TO_MAGIC_NUMBER, BT_IOT_MUSIC_SET_LOCAL_ASI, BT_IOT_MUSIC_IS_WALKMAN_WITH_SBC_CODEC_AND_NO_HFP, BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0B, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_SET_LOCAL_ASI}
    },
    {
        NULL, &BT_IOT_DEVICE_VERSION_INFO01, NULL, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0C, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_CALL_ESCO_DISCONNECT_WITH_CALL_ACTIVE_RECONNECT}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0D, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_CALL_HEADSET_BLOCK_ESCO_CONNECTION_WITH_RECONNECT, BT_IOT_DEVICE_IDENTIFY_APPLE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0E, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_NO_PRE_NEXT_PASS_THROUGH_CMD, BT_IOT_DEVICE_IDENTIFY_APPLE}
    },
    {
        NULL, NULL, NULL, 0x004000,
        {BT_IOT_IDENTIFY_LEA_SUPPORT_DEVICE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO05, 0x004000,
        {BT_IOT_MUSIC_SET_SPECIAL_MTU, BT_IOT_MUSIC_SET_LATENCY_TO_170, BT_IOT_IDENTIFY_LEA_SUPPORT_DEVICE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO01, 0x004000,
        {BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET, BT_IOT_MUSIC_SET_LATENCY_TO_250, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP, BT_IOT_IDENTIFY_LEA_SUPPORT_DEVICE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO0F, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_IDENTIFY_LEA_SUPPORT_DEVICE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO10, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_DEVICE_IDENTIFY_APPLE}
    },
    {
        NULL, NULL, &BT_IOT_DEVICE_PNP_INFO11, BT_IOT_DEVICE_INVALIDE_COD,
        {BT_IOT_MUSIC_NOT_SUPPORT_STATUS_CHANGE_NOTIFY_DEVICE}
    },

};

uint32_t        bt_iot_device_white_list_get_iot_id(bt_iot_device_identify_information_t *identify_info)
{
    uint32_t list_count = sizeof(g_all_iot_case_list) / sizeof(bt_iot_device_all_iot_case_t);
    uint32_t matched_item = 0;
    uint32_t found_index = 0;
    if (NULL == identify_info) {
        return found_index;
    }
    for (uint32_t index = 1; index < list_count; index++) {
        uint32_t temp_matched = 0;
        if (g_all_iot_case_list[index].pnp_info != NULL &&
            (g_all_iot_case_list[index].pnp_info->product_id == BT_IOT_DEVICE_INVALIDE_ITEM || g_all_iot_case_list[index].pnp_info->product_id == identify_info->pnp_info.product_id) &&
            (g_all_iot_case_list[index].pnp_info->vender_id == BT_IOT_DEVICE_INVALIDE_ITEM || g_all_iot_case_list[index].pnp_info->vender_id == identify_info->pnp_info.vender_id) &&
            (g_all_iot_case_list[index].pnp_info->version == BT_IOT_DEVICE_INVALIDE_ITEM || g_all_iot_case_list[index].pnp_info->version == identify_info->pnp_info.version)) {
            temp_matched += 1;
        }
        if (g_all_iot_case_list[index].cod != BT_IOT_DEVICE_INVALIDE_COD && (g_all_iot_case_list[index].cod == (g_all_iot_case_list[index].cod & identify_info->cod))) {
            temp_matched += 1;
        }
        if (g_all_iot_case_list[index].version_info != NULL &&
            (g_all_iot_case_list[index].version_info->manufacturer_id == BT_IOT_DEVICE_INVALIDE_ITEM || g_all_iot_case_list[index].version_info->manufacturer_id == identify_info->version_info.manufacturer_id) &&
            (g_all_iot_case_list[index].version_info->version == BT_IOT_DEVICE_INVALIDE_ITEM1 || g_all_iot_case_list[index].version_info->version == identify_info->version_info.version) &&
            (g_all_iot_case_list[index].version_info->subversion == BT_IOT_DEVICE_INVALIDE_ITEM || g_all_iot_case_list[index].version_info->subversion == identify_info->version_info.subversion)) {
            temp_matched += 1;
        }
        if (g_all_iot_case_list[index].device_name != NULL && !memcmp(g_all_iot_case_list[index].device_name, identify_info->device_name, strlen(g_all_iot_case_list[index].device_name))) {
            temp_matched += 1;
        }
        if (temp_matched > matched_item) {
            matched_item = temp_matched;
            found_index = index;
        }
    }

    bt_dmgr_report_id("[BT_DM][IOT]matched_item:%d, found_index:%d", 2, matched_item, found_index);
    return found_index;
}

bt_status_t     bt_iot_device_white_list_get_iot_case_list_by_addr(bt_bd_addr_t *addr, bt_iot_device_white_list_iot_case_t ret_list[MAX_IOT_CASE])
{
    if (NULL == addr || NULL == ret_list) {
        return BT_STATUS_FAIL;
    }
    uint32_t iot_id = bt_device_manager_remote_find_iot_id((void *)addr);
    uint32_t list_count = sizeof(g_all_iot_case_list) / sizeof(bt_iot_device_all_iot_case_t);
    if (iot_id >= list_count) {
        return BT_STATUS_FAIL;
    }
    memcpy(ret_list, g_all_iot_case_list[iot_id].all_iot_case, sizeof(g_all_iot_case_list[iot_id].all_iot_case));
    return BT_STATUS_SUCCESS;
}

bt_iot_device_white_list_iot_case_t     *bt_iot_device_white_list_get_iot_case_list_by_addr_2(bt_bd_addr_t *addr)
{
    if (NULL == addr) {
        return NULL;
    }
    uint32_t iot_id = bt_device_manager_remote_find_iot_id((void *)addr);
    uint32_t list_count = sizeof(g_all_iot_case_list) / sizeof(bt_iot_device_all_iot_case_t);
    if (iot_id >= list_count) {
        return NULL;
    }
    return (void *)(g_all_iot_case_list[iot_id].all_iot_case);
}

bool            bt_iot_device_white_list_check_iot_case(bt_bd_addr_t *addr, bt_iot_device_white_list_iot_case_t iot_case)
{
    uint32_t iot_id = bt_device_manager_remote_find_iot_id((void *)addr);
    uint32_t list_count = sizeof(g_all_iot_case_list) / sizeof(bt_iot_device_all_iot_case_t);
    bool ret = false;
    if (iot_id < list_count) {
        for (uint32_t index = 0; index < MAX_IOT_CASE; index++) {
            if (g_all_iot_case_list[iot_id].all_iot_case[index] == iot_case) {
                ret = true;
            } else if (g_all_iot_case_list[iot_id].all_iot_case[index] == 0) {
                break;
            }
        }
    }
    bt_dmgr_report_id("[BT_DM][IOT] Check iot case, index:%d, list_count:%d, iot_case:%d, ret:%d", 4, iot_id, list_count, iot_case, ret);

    return ret;
}

