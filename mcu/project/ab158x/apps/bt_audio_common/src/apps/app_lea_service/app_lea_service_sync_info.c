
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



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
static void app_lea_sync_start_peer_adv(app_lea_sync_info_t *peer_info)
{
    if (peer_info == NULL || peer_info->adv_mode == APP_LEA_ADV_MODE_NONE) {
        return;
    }

    app_lea_service_start_advertising(peer_info->adv_mode, FALSE, peer_info->adv_timeout);
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
    bt_addr_t lea_conn_addr[APP_LE_AUDIO_MAX_LINK_NUM] = {0};
    app_lea_conn_mgr_get_conn_info(&local_info.lea_num, lea_conn_addr);
    app_lea_adv_mgr_get_adv_info(&local_info.adv_mode, &local_info.adv_timeout);
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
    bt_addr_t lea_conn_addr[APP_LE_AUDIO_MAX_LINK_NUM] = {0};
    app_lea_conn_mgr_get_conn_info(&local_info.lea_num, lea_conn_addr);
    app_lea_adv_mgr_get_adv_info(&local_info.adv_mode, &local_info.adv_timeout);
    local_info.bond_num = app_lea_conn_mgr_get_bond_num();

    // Only for Power on Reconnect flow
    // Agent TARGET_MODE, Sync to Partner on Reconnecting Time
    if (role == BT_AWS_MCE_ROLE_PARTNER
        && local_info.adv_mode != APP_LEA_ADV_MODE_TARGET
        && peer_info->adv_mode == APP_LEA_ADV_MODE_TARGET) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] trigger TARGET ADV by peer", 1, role);
        app_lea_sync_start_peer_adv(peer_info);
#if 0
    } else if (role == BT_AWS_MCE_ROLE_PARTNER
               && peer_info->adv_mode == APP_LEA_ADV_MODE_GENERAL
               && peer_info->adv_timeout == APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME
               && local_info.adv_mode != APP_LEA_ADV_MODE_GENERAL) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] Switch_LEA General ADV by peer", 1, role);
        // Only for non-Dual-Mode + MTK HFP AT CMD case
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
#endif
    } else if (local_info.adv_mode != APP_LEA_ADV_MODE_GENERAL
               && peer_info->bond_num > local_info.bond_num) {
        APPS_LOG_MSGID_W(LOG_TAG" sync_info_handle, [%02X] Start General ADV %d ms by peer more bond_info",
                         2, role, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
        // Maybe stop and switch to TARGET_ALL by "Reconnect_Complete" or "Force_update after Bond" (visible=0)
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
    }
#endif
}


#endif /* AIR_LE_AUDIO_ENABLE */
