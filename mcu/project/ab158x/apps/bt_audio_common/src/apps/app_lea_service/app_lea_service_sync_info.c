
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 * File: app_le_audio_conn_mgr.c
 *
 * Description: This file provides API for LE Audio connection manage.
 *
 */

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)

#include "app_lea_service_sync_info.h"

#include "app_lea_service.h"
#include "app_lea_service_adv_mgr.h"
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"

#include "app_bt_conn_manager.h"
#include "apps_debug.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "apps_events_event_group.h"
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#endif
#include "bt_connection_manager_adapt.h"

#define LOG_TAG             "[LEA][SYNC]"

#ifdef MTK_AWS_MCE_ENABLE
//static app_lea_sync_info_t             app_lea_peer_sync_info = {0};
#endif

/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
#if 0//defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
static void app_lea_sync_info_print(app_lea_sync_info_t *info)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint8_t lea_num = info->lea_num;
    uint8_t lea_addr_type1 = info->lea_conn_addr[0].type;
    uint8_t lea_addr_type2 = info->lea_conn_addr[1].type;
    uint8_t lea_addr_type3 = info->lea_conn_addr[2].type;
    uint8_t *lea_addr1 = info->lea_conn_addr[0].addr;
    uint8_t *lea_addr2 = info->lea_conn_addr[1].addr;
    uint8_t *lea_addr3 = info->lea_conn_addr[2].addr;
    APPS_LOG_MSGID_I(LOG_TAG" lea_info, lea_num=%d addr1=%d %02X:%02X:%02X:%02X:%02X:%02X",
                     8, lea_num, lea_addr_type1,
                     lea_addr1[5], lea_addr1[4], lea_addr1[3],
                     lea_addr1[2], lea_addr1[1], lea_addr1[0]);
    APPS_LOG_MSGID_I(LOG_TAG" lea_info, [%02X] addr2=%d %02X:%02X:%02X:%02X:%02X:%02X",
                     8, role, lea_addr_type2, lea_addr2[5], lea_addr2[4], lea_addr2[3],
                     lea_addr2[2], lea_addr2[1], lea_addr2[0]);
    APPS_LOG_MSGID_I(LOG_TAG" lea_info, [%02X] addr3=%d %02X:%02X:%02X:%02X:%02X:%02X",
                     8, role, lea_addr_type3, lea_addr3[5], lea_addr3[4], lea_addr3[3],
                     lea_addr3[2], lea_addr3[1], lea_addr3[0]);

    uint8_t adv_target_addr_num = 0;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (memcmp(info->adv_addr[i].addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
            adv_target_addr_num++;
        }
    }
    APPS_LOG_MSGID_I(LOG_TAG" lea_info, [%02X] adv_mode=%d adv_target_addr_num=%d",
                     3, role, info->adv_mode, adv_target_addr_num);
}
#endif

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
static void app_lea_sync_start_peer_adv(app_lea_sync_info_t *peer_info)
{
    if (peer_info == NULL || peer_info->adv_mode == APP_LEA_ADV_MODE_NONE) {
        return;
    }

    if (peer_info->adv_mode == APP_LEA_ADV_MODE_GENERAL) {
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, peer_info->adv_timeout);
    } else {
        uint8_t add_le_num = 0;
        uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            if (memcmp(peer_info->adv_addr[i].addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
                if (add_le_num == 0) {
                    app_lea_adv_mgr_update_target_addr(APP_LEA_TARGET_SET_UNIQUE_ADDR,
                                                       peer_info->adv_addr[i].type,
                                                       peer_info->adv_addr[i].addr);
                } else {
                    app_lea_adv_mgr_update_target_addr(APP_LEA_TARGET_ADD_ADDR,
                                                       peer_info->adv_addr[i].type,
                                                       peer_info->adv_addr[i].addr);
                }
                add_le_num++;
            }
        }

        // Note
        // 1. After support sub-mode and sync IDA/IRK to bond list feature, LEA don't need to pre-add target addr
        //    Clear target addr list when app_lea_update_target_add_white_list
        // 2. On LEA or LEA-ULL2 Target Mode, start ADV with all "active reconnect" addr.
        //    LEA/ULL2 reconnect mode are APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED after BT power off (0x15 reason)
        // 3. The target addr list only available when ULL2 Only + Target Mode
        app_lea_service_start_advertising(peer_info->adv_mode, FALSE, peer_info->adv_timeout);
    }
}
#endif



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_lea_sync_info_send(void)
{
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#ifdef AIR_SPEAKER_ENABLE
    // Single mode - no AWS Link; broadcast mode - BIS;
    bt_aws_mce_srv_mode_t aws_mode = bt_aws_mce_srv_get_mode();
    if (aws_mode != BT_AWS_MCE_SRV_MODE_NORMAL && aws_mode != BT_AWS_MCE_SRV_MODE_DOUBLE) {
        APPS_LOG_MSGID_E(LOG_TAG" sync_info_send, [%02X] invalid mode", 2, role, aws_mode);
        return FALSE;
    }
#endif
    if (bt_aws_mce_srv_get_link_type() == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" sync_info_send, [%02X] AWS Disconnected", 1, role);
        return FALSE;
    }

    app_lea_sync_info_t local_info = {0};
    app_lea_conn_mgr_get_conn_info(&local_info.lea_num, local_info.lea_conn_addr);
    app_lea_adv_mgr_get_adv_info(&local_info.adv_mode, local_info.adv_addr, &local_info.adv_timeout);
    local_info.bond_num = app_lea_conn_mgr_get_bond_num();

    bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                                           EVENT_ID_LE_AUDIO_SYNC_INFO,
                                                           &local_info,
                                                           sizeof(app_lea_sync_info_t));

    APPS_LOG_MSGID_I(LOG_TAG" sync_info_send, [%02X] lea_num=%d adv_mode=%d bond_num=%d bt_status=0x%08X",
                     5, role, local_info.lea_num, local_info.adv_mode, local_info.bond_num, bt_status);
    return TRUE;
#else
    return FALSE;
#endif
}

void app_lea_sync_info_handle(app_lea_sync_info_t *peer_info)
{
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    app_lea_sync_info_t local_info = {0};
    app_lea_conn_mgr_get_conn_info(&local_info.lea_num, local_info.lea_conn_addr);
    app_lea_adv_mgr_get_adv_info(&local_info.adv_mode, local_info.adv_addr, &local_info.adv_timeout);
    local_info.bond_num = app_lea_conn_mgr_get_bond_num();

//    app_lea_sync_info_print(&local_info);
//    app_lea_sync_info_print(peer_info);

    // Only for Power on Reconnect flow
    // Agent TARGET_MODE, Sync to Partner on Reconnecting Time
    if (role == BT_AWS_MCE_ROLE_PARTNER
        && local_info.adv_mode != APP_LEA_ADV_MODE_TARGET
        && peer_info->adv_mode == APP_LEA_ADV_MODE_TARGET) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] trigger TARGET ADV by peer", 1, role);
        app_lea_sync_start_peer_adv(peer_info);
    } else if (role == BT_AWS_MCE_ROLE_PARTNER
               && peer_info->adv_mode == APP_LEA_ADV_MODE_GENERAL
               && peer_info->adv_timeout == APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME
               && local_info.adv_mode != APP_LEA_ADV_MODE_GENERAL) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] Switch_LEA General ADV by peer", 1, role);
        // Only for non-Dual-Mode + MTK HFP AT CMD case
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
    } else if (local_info.adv_mode != APP_LEA_ADV_MODE_GENERAL
               && peer_info->bond_num > local_info.bond_num) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] Start General ADV %d ms by peer more bond_info",
                         2, role, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
        // Maybe stop and switch to TARGET_ALL by "Reconnect_Complete" or "Force_update after Bond" (visible=0)
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
    }

    // Agent not TARGET_MODE, Partner should use Agent ADV mode on Reconnecting Time
#ifdef APP_CONN_MGR_RECONNECT_CONTROL
    else if (role == BT_AWS_MCE_ROLE_PARTNER
             && app_bt_conn_mgr_is_reconnecting()) {
        if (peer_info->adv_mode != APP_LEA_ADV_MODE_NONE
            && peer_info->adv_mode != APP_LEA_ADV_MODE_TARGET) {
            APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] trigger mode-%d ADV by peer",
                             2, role, peer_info->adv_mode);
            app_lea_sync_start_peer_adv(peer_info);
        } else if (peer_info->adv_mode == APP_LEA_ADV_MODE_NONE
                   && local_info.adv_mode == APP_LEA_ADV_MODE_NONE) {
            APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] re-trigger reconnect ADV", 1, role);
            app_bt_conn_mgr_lea_restart_reconnect_adv();
        }
    }
#endif
#endif
}


#endif /* AIR_LE_AUDIO_ENABLE */
