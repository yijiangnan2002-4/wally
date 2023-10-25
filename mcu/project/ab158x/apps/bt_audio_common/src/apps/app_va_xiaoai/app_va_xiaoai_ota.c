
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
 * File: app_va_xiaoai_ota.c
 *
 * Description: This file is XiaoAI OTA Code.
 *
 */

#ifdef AIR_XIAOAI_ENABLE

#include "app_va_xiaoai_ota.h"

#include "app_va_xiaoai_ble_adv.h"
#include "app_va_xiaoai_config.h"

#include "xiaoai.h"

#include "apps_aws_sync_event.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "bt_aws_mce_report.h"
#endif
#include "bt_connection_manager.h"
#include "FreeRTOS.h"
#include "fota_flash.h"
#include "fota_multi_info.h"
#include "fota_multi_info_util.h"
#include "timers.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "ui_shell_manager.h"

#define LOG_TAG           "[XIAOAI_VA][OTA]"

static TimerHandle_t        g_xiaoai_slience_ota_timer = NULL;
static bool                 g_xiaoai_both_lid_close    = FALSE;


static void app_va_xiaoai_slience_ota_timer_callback(void *timer_handle)
{
    APPS_LOG_MSGID_I(LOG_TAG" slience_ota_timer_callback", 0);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_XIAOAI,
                        XIAOAI_EVENT_OTA_SLIENCE_OTA_TIMEROUT,
                        NULL, 0, NULL, 0);
}





/**============================================================================*/
/**                                 PUBLIC API                                 */
/**============================================================================*/
void app_va_xiaoai_ota_init(void)
{
    if (g_xiaoai_slience_ota_timer == NULL) {
        g_xiaoai_slience_ota_timer = xTimerCreate("sli_ota",
                                                  (TickType_t)(APP_VA_XIAOAI_SLIENCE_OTA_TIMER / portTICK_PERIOD_MS),
                                                  FALSE, NULL,
                                                  (void *) app_va_xiaoai_slience_ota_timer_callback);
        configASSERT(g_xiaoai_slience_ota_timer != NULL);
    }

    bool enable = FALSE;
//    uint32_t size = sizeof(uint8_t);
//    nvkey_status_t status = nvkey_read_data(NVKEYID_XIAOAI_SLIENCE_OTA, (uint8_t *)&enable, &size);
//    APPS_LOG_MSGID_I(LOG_TAG" ota_init, read status=%d", 1, status);
//    if (status == NVKEY_STATUS_ITEM_NOT_FOUND) {
//        enable = FALSE;
//        size = sizeof(uint8_t);
//        status = nvkey_write_data(NVKEYID_XIAOAI_SLIENCE_OTA, (const uint8_t *)&enable, size);
//        APPS_LOG_MSGID_I(LOG_TAG" ota_init, write status=%d", 1, status);
//    }

    xiaoai_enable_slience_ota(enable);
    APPS_LOG_MSGID_I(LOG_TAG" ota_init, enable=%d", 1, enable);
}

bool app_va_xiaoai_ota_slience_enable(bool enable, bool sync_to_peer)
{
    bool success = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool slience_ota_enable = xiaoai_is_slience_ota();

    if (slience_ota_enable == enable) {
        APPS_LOG_MSGID_E(LOG_TAG" ota_slience_enable fail, same enable=%d", 1, enable);
        return success;
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    if (sync_to_peer && aws_link_type == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" ota_slience_enable fail, must AWS connected", 0);
        return success;
    }
#endif

    xiaoai_enable_slience_ota(enable);
//    nvkey_status_t nvkey_status = nvkey_write_data(NVKEYID_XIAOAI_SLIENCE_OTA,
//                                                   (const uint8_t *)&enable, sizeof(uint8_t));
    APPS_LOG_MSGID_I(LOG_TAG" ota_slience_enable, [%02X] enable=%d sync_to_peer=%d",
                     3, role, enable, sync_to_peer);

#ifdef MTK_AWS_MCE_ENABLE
    if (sync_to_peer) {
        bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                            XIAOAI_EVENT_OTA_SLIENCE_OTA_SYNC,
                                                            (uint8_t *)&enable,
                                                            sizeof(uint8_t));
        if (status != BT_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(LOG_TAG" ota_slience_enable fail, sync_to_peer status=0x%08X", 1, status);
        }
        success = (status == BT_STATUS_SUCCESS);
    }
#endif
    return success;
}

void app_va_xiaoai_ota_check_both_close(bool both_lid_close)
{
    bool old_both_lid_close = g_xiaoai_both_lid_close;
    bool is_slience_ota = xiaoai_is_slience_ota();
    if (old_both_lid_close == both_lid_close || !is_slience_ota) {
        APPS_LOG_MSGID_I(LOG_TAG" check_both_close ignore, both_lid_close=%d is_slience_ota=%d",
                         2, both_lid_close, is_slience_ota);
        return;
    }
    g_xiaoai_both_lid_close = both_lid_close;

    if (both_lid_close) {
        FOTA_ERRCODE check_ret = fota_check_fota_package_integrity(InternalFlash);
        APPS_LOG_MSGID_I(LOG_TAG" check_both_close, check_ret=%d", 1, check_ret);
        if (check_ret == FOTA_ERRCODE_SUCCESS) {
            xTimerReset(g_xiaoai_slience_ota_timer, 0);
            APPS_LOG_MSGID_I(LOG_TAG" check_both_close, start slience_ota timer", 0);
        }
    } else {
        xTimerStop(g_xiaoai_slience_ota_timer, 0);
        APPS_LOG_MSGID_I(LOG_TAG" check_both_close, stop slience_ota timer", 0);
    }
}

void app_va_xiaoai_ota_commit_reboot(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    FOTA_ERRCODE check_ret = FOTA_ERRCODE_FAIL;
    FOTA_ERRCODE commit_ret = FOTA_ERRCODE_FAIL;

    check_ret = fota_check_fota_package_integrity(InternalFlash);
    if (check_ret == FOTA_ERRCODE_SUCCESS) {
        commit_ret = fota_upgrade_flag_set();
    }
    if (commit_ret == FOTA_ERRCODE_SUCCESS) {
        uint32_t delay_ms = 1000;
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                            EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                            NULL, 0, NULL,
                            delay_ms);
    }
    APPS_LOG_MSGID_I(LOG_TAG" ota_commit_reboot, [%02X] check_ret=%d commit_ret=%d",
                     3, role, check_ret, commit_ret);
}



#endif /* AIR_XIAOAI_ENABLE */
