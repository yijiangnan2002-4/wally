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
 * File: nvdm_config_factory_reset.c
 *
 * Description: This file is the implementation of factory reset.
 *
 */

#include <stdio.h>
#include "nvdm.h"
#include "nvdm_config_factory_reset.h"
#include "syslog.h"
#include <string.h>
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "nvdm_id_list.h"
#include "nvkey_dspalg.h"
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#ifdef MTK_PEQ_ENABLE
#include "bt_sink_srv_ami.h"
#endif

/**
 *  @brief This structure defines the NVDM item info.
 */
typedef struct {
    char *group_name;    /**<  NVDM group name. */
    char *item_name;     /**<  NVDM item name. */
} factory_rst_reserved_nvdm_item_t;

static const factory_rst_reserved_nvdm_item_t g_factory_rst_clear_nvdm_items[] = FACTORY_RST_CLEAR_NVDM_ITEM_LIST;  /* NVDM item reserve list in factory reset. */
static const uint16_t g_factory_rst_clear_nvkey_items[] = FACTORY_RST_CLEAR_NVKEY_ITEM_LIST;  /* NVKEY item reserve list in factory reset. */


void factory_rst_reserved_nvdm_item_list_check(void)
{
    uint32_t i, size;
    uint8_t factrst_flag;;
    nvdm_status_t status = NVDM_STATUS_OK;
    nvkey_status_t nvkey_status;
    char group_name[64];
    char data_item_name[64];
    char item_name[5];

    size = sizeof(factrst_flag);
    /* Read factory reset flag in NVDM. This flag is not in reserve list and will be deleted after re-organize. */
    nvkey_status = nvkey_read_data(NVID_SYS_FACTORY_RESET_FLAG, &factrst_flag, &size);
    if (nvkey_status == NVKEY_STATUS_OK) {
        nvkey_status = nvkey_delete_data_item(NVID_SYS_FACTORY_RESET_FLAG);
        if (nvkey_status != NVKEY_STATUS_OK) {
            LOG_MSGID_E(common, "Delete factory reset flag fail: %d", 1, nvkey_status);
            return;
        }
    } else {
        return;
    }
    if (factrst_flag == FACTORY_RESET_FLAG) {
        /* Flag set to be factory reset, all NVDM items except for those in FACTORY_RST_RESERVED_NVDM_ITEM_LIST will be deleted. */
        LOG_MSGID_I(common, "Detect Factory Reset, re-organize the NVDM region", 1);


        /* Clear nvdm item. */
        for (i = 0; i < (sizeof(g_factory_rst_clear_nvdm_items) / sizeof(factory_rst_reserved_nvdm_item_t)); i++) {
            status = nvdm_delete_data_item(g_factory_rst_clear_nvdm_items[i].group_name, g_factory_rst_clear_nvdm_items[i].item_name);
            if (status != NVDM_STATUS_OK) {
                LOG_MSGID_E(common, "nvdm_delete_data_item fail: group_name(%s), data_item_name(%s), %d", 3,
                            g_factory_rst_clear_nvdm_items[i].group_name, g_factory_rst_clear_nvdm_items[i].item_name, status);
            }
            LOG_MSGID_I(common, "delete item: group_name = %s, data_item_name = %s", 2,
                        g_factory_rst_clear_nvdm_items[i].group_name, g_factory_rst_clear_nvdm_items[i].item_name);
        }

        /* Clear nvkey item. */
        for (i = 0; i < (sizeof(g_factory_rst_clear_nvkey_items) / sizeof(uint16_t)); i++) {
            snprintf(item_name, sizeof(item_name), "%X", g_factory_rst_clear_nvkey_items[i]);
            status = nvdm_delete_data_item("AB15", item_name);
            if (status != NVDM_STATUS_OK) {
                LOG_MSGID_E(common, "nvdm_delete_data_item fail: group_name(AB15), data_item_name(%s), %d", 2, item_name, status);
            }
            LOG_MSGID_I(common, "delete item: group_name = AB15, data_item_name = %s", 1, item_name);
        }

#if 0
        /* Scan all the NVDM items from the beginning of NVDM region. */
        status = nvdm_query_begin();
        if (status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_begin fail %d", 1, status);
            return;
        }
        while (nvdm_query_next_group_name(group_name) == NVDM_STATUS_OK) {
            /* Scan group by group. */
            while (nvdm_query_next_data_item_name(data_item_name) == NVDM_STATUS_OK) {
                /* Scan item by item. */
                item_is_match = false;
                /* Scan nvdm item. */
                for (i = 0; i < (sizeof(g_factory_rst_reserved_nvdm_items) / sizeof(factory_rst_reserved_nvdm_item_t)); i++) {
                    if ((!strcmp(g_factory_rst_reserved_nvdm_items[i].group_name, group_name)) &&
                        (!strcmp(g_factory_rst_reserved_nvdm_items[i].item_name, data_item_name))) {
                        /* The item is in the reserve list. */
                        item_is_match = true;
                        break;
                    }
                }
                /* Scan nvkey item. */
                for (i = 0; i < (sizeof(g_factory_rst_reserved_nvkey_items) / sizeof(uint16_t)); i++) {
                    snprintf(item_name, sizeof(item_name), "%X", g_factory_rst_reserved_nvkey_items[i]);
                    if ((!strcmp("AB15", group_name)) &&
                        (!strcmp(item_name, data_item_name))) {
                        /* The item is in the reserve list. */
                        item_is_match = true;
                        break;
                    }
                }
                if (item_is_match == false) {
                    /* If it's not in the reserve list, it'll be deleted here. */
                    status = nvdm_delete_data_item(group_name, data_item_name);
                    if (status != NVDM_STATUS_OK) {
                        LOG_E(common, "nvdm_delete_data_item fail: group_name(%s), data_item_name(%s), %d", group_name, data_item_name, status);
                        return;
                    }
                    //LOG_I(common, "delete item: group_name = %s, data_item_name = %s", group_name, data_item_name);
                }
            }
        }
        status = nvdm_query_end();
        if (status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_end fail %d", 1, status);
            return;
        }
#endif
        /* Some NVDM item cannot be delete, but need to be reset to default value. */
#ifdef MTK_ANC_ENABLE
        if (!app_anc_service_reinit_nvdm()) {
            LOG_MSGID_E(common, "anc_reinit_nvdm fail", 0);
        }
#endif
#ifdef MTK_PEQ_ENABLE
        if (aud_peq_reinit_nvdm() != 0) {
            LOG_MSGID_E(common, "aud_peq_reinit_nvdm fail", 0);
        }
#endif

        LOG_MSGID_I(common, "dump reserved item list in NVDM region after Factory Reset", 0);
        /* Dump from the beginning for debug. */
        status = nvdm_query_begin();
        if (status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_begin fail %d", 1, status);
            return;
        }
        while (nvdm_query_next_group_name(group_name) == NVDM_STATUS_OK) {
            while (nvdm_query_next_data_item_name(data_item_name) == NVDM_STATUS_OK) {
                LOG_I(common, "group_name = %s, data_item_name = %s", group_name, data_item_name);
            }
        }
        status = nvdm_query_end();
        if (status != NVDM_STATUS_OK) {
            LOG_MSGID_E(common, "nvdm_query_end fail %d", 1, status);
            return;
        }
    } else if (factrst_flag == FACTORY_RESET_LINK_KEY) {
        /* Flag set to be link key reset, only delete link key related 2 items. */
        nvkey_status = nvkey_delete_data_item(NVID_BT_HOST_REMOTE_INFO_01);
        LOG_MSGID_I(common, "delete item NVID_BT_HOST_REMOTE_INFO_01, ret = %d", 1, nvkey_status);
        nvkey_status = nvkey_delete_data_item(NVID_BT_HOST_REMOTE_SEQUENCE);
        LOG_MSGID_I(common, "delete item NVID_BT_HOST_REMOTE_SEQUENCE, ret = %d", 1, nvkey_status);
    }
}

