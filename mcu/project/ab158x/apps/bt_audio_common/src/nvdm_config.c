/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

/**
 * File: nvdm_config.c
 *
 * Description: This file implements the functions defined in the nvdm_config.h file.
 * Their function is to perform some trimming of the data items in NVDM for the first boot after FOTA.
 *
 */

#if defined(MTK_NVDM_ENABLE) && defined(MTK_FOTA_VIA_RACE_CMD)
#include "nvdm.h"
#include "nvkey.h"
#include "nvdm_config.h"
#include "fota.h"
#include "syslog.h"
#include <string.h>
#include "FreeRTOS.h"

#include "nvdm_id_list.h"

#ifndef __weak
#define __weak   __attribute__((weak))
#endif


#define NV_DATA_UPDATE_REQUEST_FLAG 0x55
#define NV_DATA_UPDATE_REQUEST_DONE 0xAA

/**
 *  @brief This structure defines enough information for NVDM to find the specified entry.
 */
typedef struct {
    char *group_name;     /**< The group name of the data item, such as "Bluetooth". */
    char *item_name;      /**< The name of the data item, such as "address" of the "Bluetooth". */
} fota_reserved_nvdm_item_t;

/* In this array, the NVDM data items that need to be deleted specified by the user are saved. */
static const fota_reserved_nvdm_item_t g_fota_clear_nvdm_items[] = FOTA_NVDM_ITEM_CLEAR_LIST;
static const uint16_t g_fota_clear_nvkey_items[] = FOTA_NVKEY_ITEM_CLEAR_LIST;


/**
 * @brief    Users can add the processing logic for the specified NVDM data items in this function.
 * @note     This function has the weak attribute. If you define other functions with the same name,
 *           this function will be masked.
 */
__weak void reserved_nvdm_item_list_modify(void)
{
#if 0	// richard for UI
	const uint8_t ha_mp_data[] ={0x00, 0x77, 0x77, 0x71, 0x78, 0x73, 0x79, 0x72, 0x78, 0x77, 0x72, 0x78, 0x79, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7B, 0x7A, 0x7B, \
		0x7B, 0x7B, 0x7A, 0x7A, 0x7B, 0x7A, 0x7C, 0x7B, 0x79, 0x7D, 0x7C, 0x7C, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, \
		0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x00, 0x78, 0x78, 0x74, 0x7B, 0x76, 0x7E, 0x7A, 0x82, 0x82, 0x79, 0x7C, 0x7A, 0x78, 0x76, 0x75, 0x75, 0x74, 0x75, 0x75, \
		0x75, 0x75, 0x76, 0x77, 0x79, 0x7A, 0x7B, 0x7B, 0x7A, 0x79, 0x77, 0x75, 0x73, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71, \
		0x71, 0x71, 0x71, 0x71, 0x71, 0x71, 0x00, 0x77, 0x77, 0x72, 0x78, 0x72, 0x79, 0x72, 0x77, 0x77, 0x71, 0x78, 0x79, 0x79, 0x7A, 0x78, 0x7A, 0x79, 0x7A, \
		0x7B, 0x7B, 0x7C, 0x7C, 0x7B, 0x7D, 0x7A, 0x7B, 0x7D, 0x79, 0x7A, 0x7E, 0x7A, 0x7C, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, \
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x77, 0x78, 0x74, 0x76, 0x82, 0x74, 0x7A, 0x71, 0x71, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	nvkey_write_data(NVID_DSP_ALG_HA_MP_DATA, ha_mp_data, sizeof(ha_mp_data) / sizeof(uint8_t));

	const uint8_t audio_IO_cfg[] = {0x03, 0x02, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0xF4, 0x01, 0x00, 0x00, 0x00, 0x00,\
		0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,\
		0x00, 0x01, 0x01, 0x07, 0x02, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	nvkey_write_data(NVID_DSP_FW_AUDIO_HW_IO_CFG, audio_IO_cfg, sizeof(audio_IO_cfg) / sizeof(uint8_t));
#endif
}


/**
 * @brief    Write a flag so that some unnecessary NVDM data items can be deleted
 *           at the first boot after FOTA.
 */
void reserved_nvdm_item_list_ask_check(void)
{
    nvkey_status_t status = 0;
    uint8_t nv_data_update_req = 0;
    uint32_t size = 0;

    nv_data_update_req = NV_DATA_UPDATE_REQUEST_FLAG;
    size = sizeof(nv_data_update_req);
    status = nvkey_write_data(NVID_SYS_NV_DATA_MODIFY_REQUEST, &nv_data_update_req, size);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_E(common, "Mark NVID_SYS_NV_DATA_MODIFY_REQUEST as flag fail: %d", 1, status);
        return;
    }
}


/**
 *  @brief This API is used to check whether all non-reserved NVDM items should be deleted after FOTA done.
 *  @note  It should be called during system initial phase.
 */
void reserved_nvdm_item_list_check(void)
{
    uint32_t i, size;
    uint8_t nv_data_update_req;
    nvkey_status_t status;
    nvdm_status_t nvdm_op_status;
    char item_name[5];
    //char group_name[64];
    //char data_item_name[64];
    bool fota_upgrade_result = FALSE;
    char *group_name;
    char *data_item_name;

    LOG_MSGID_I(common, "Check the FOTA status", 0);

    group_name = (char *) pvPortMalloc(64);
    data_item_name = (char *) pvPortMalloc(64);
    if (group_name == NULL ||  data_item_name == NULL) {
        LOG_MSGID_E(common, "reserved_nvdm_item_list_check(): malloc failed", 0);
        goto free_ret;
    }

    fota_get_upgrade_result(&fota_upgrade_result);

    size = sizeof(nv_data_update_req);
    status = nvkey_read_data(NVID_SYS_NV_DATA_MODIFY_REQUEST, &nv_data_update_req, &size);

    /* If the FOTA upgrade is successful, and the flag previously written to NVDM is the correct value. */
    if ((status == NVKEY_STATUS_OK) && fota_upgrade_result && (nv_data_update_req == NV_DATA_UPDATE_REQUEST_FLAG)) {
        LOG_MSGID_I(common, "Detect FOTA update, re-organize the NVDM region", 0);

        /* Clear nvdm item. */
        for (i = 0; i < (sizeof(g_fota_clear_nvdm_items) / sizeof(fota_reserved_nvdm_item_t)); i++) {
            nvdm_op_status = nvdm_delete_data_item(g_fota_clear_nvdm_items[i].group_name, g_fota_clear_nvdm_items[i].item_name);
            if (nvdm_op_status != NVDM_STATUS_OK) {
                LOG_MSGID_E(common, "nvdm_delete_data_item fail: group_name(%s), data_item_name(%s), %d", 3,
                            g_fota_clear_nvdm_items[i].group_name, g_fota_clear_nvdm_items[i].item_name, nvdm_op_status);
            }
            LOG_MSGID_I(common, "delete item: group_name = %s, data_item_name = %s", 2,
                        g_fota_clear_nvdm_items[i].group_name, g_fota_clear_nvdm_items[i].item_name);
        }

        /* Clear nvkey item. */
        for (i = 0; i < (sizeof(g_fota_clear_nvkey_items) / sizeof(uint16_t)); i++) {
            snprintf(item_name, sizeof(item_name), "%X", g_fota_clear_nvkey_items[i]);
            nvdm_op_status = nvdm_delete_data_item("AB15", item_name);
            if (nvdm_op_status != NVDM_STATUS_OK) {
                LOG_MSGID_E(common, "nvdm_delete_data_item fail: group_name(AB15), data_item_name(%s), %d", 2, item_name, nvdm_op_status);
            }
            LOG_MSGID_I(common, "delete item: group_name = AB15, data_item_name = %s", 1, item_name);
        }

        if (fota_nvdm_is_incompatible_flag_set() == true) {
            LOG_MSGID_I(common, "Trigger user's customize modification for reserved item list in NVDM region after FOTA", 0);
            reserved_nvdm_item_list_modify();
            fota_nvdm_clear_incompatible_flag();
        }

        /* Dump group name and item name of all items stored in NVDM. */
        LOG_MSGID_I(common, "dump reserved item list in NVDM region after FOTA", 0);
        nvdm_op_status = nvdm_query_begin();
        if (nvdm_op_status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_begin fail %d", 1, nvdm_op_status);
            goto free_ret;
        }
        while (nvdm_query_next_group_name(group_name) == NVDM_STATUS_OK) {
            while (nvdm_query_next_data_item_name(data_item_name) == NVDM_STATUS_OK) {
                LOG_I(common, "group_name = %s, data_item_name = %s", group_name, data_item_name);
            }
        }
        nvdm_op_status = nvdm_query_end();
        if (nvdm_op_status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_end fail %d", 1, nvdm_op_status);
            goto free_ret;
        }

        /* Since this is only required for the first boot after FOTA, this item needs to be modified. */
        nv_data_update_req = NV_DATA_UPDATE_REQUEST_DONE;
        size = sizeof(nv_data_update_req);
        status = nvkey_write_data(NVID_SYS_NV_DATA_MODIFY_REQUEST, &nv_data_update_req, size);
        if (status != NVKEY_STATUS_OK) {
            LOG_MSGID_E(common, "Mark NVID_SYS_NV_DATA_MODIFY_REQUEST as done fail: %d", 1, status);
            goto free_ret;
        }
    }
free_ret:
    vPortFree(group_name);
    vPortFree(data_item_name);
}

#endif

