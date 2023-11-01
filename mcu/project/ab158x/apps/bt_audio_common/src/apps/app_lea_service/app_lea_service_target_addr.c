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

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)

#include "app_lea_service_target_addr.h"

#include "apps_debug.h"

#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_gap_le_service.h"
#include "FreeRTOS.h"

#define LOG_TAG     "[LEA][ADV]"

uint8_t      app_lea_adv_target_addr_num = 0;
bt_addr_t    app_lea_adv_target_addr[APP_LEA_MAX_TARGET_NUM] = {0};

extern void app_lea_conn_mgr_get_reconnect_addr(uint8_t sub_mode, bt_addr_t *addr_list, uint8_t *list_num);



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static void app_lea_target_update_white_list(bt_gap_le_set_white_list_op_t op,
                                             bt_addr_type_t addr_type,
                                             const uint8_t *addr)
{
#ifdef AIR_LE_AUDIO_WHITELIST_ADV
    bt_addr_t *bt_addr_param = NULL;
    bt_addr_t bt_addr = {0};
    if (addr != NULL) {
        uint8_t whitelist_adv_addr_type = 0xFF;
        if (addr_type == BT_ADDR_PUBLIC || addr_type == BT_ADDR_PUBLIC_IDENTITY) {
            whitelist_adv_addr_type = 0;
        } else if (addr_type == BT_ADDR_RANDOM || addr_type == BT_ADDR_RANDOM_IDENTITY) {
            whitelist_adv_addr_type = 1;
        }
        bt_addr.type = whitelist_adv_addr_type;
        memcpy(bt_addr.addr, addr, BT_BD_ADDR_LEN);
        bt_addr_param = &bt_addr;
    }
    bt_status_t bt_status = bt_gap_le_srv_operate_white_list(op, bt_addr_param, NULL);
    if (bt_status != BT_STATUS_SUCCESS && bt_status != BT_STATUS_PENDING) {
        APPS_LOG_MSGID_E(LOG_TAG" target_white_list, error bt_status=0x%08X op=0x%04X", 2, bt_status, op);
    }
#endif
}

static void app_lea_print_target_addr(void)
{
//    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
//    APPS_LOG_MSGID_I(LOG_TAG" printf_target_addr, [%02X] num=%d", 2, role, app_lea_adv_target_addr_num);
//    for (int i = 0; i < APP_LEA_MAX_TARGET_NUM; i++) {
//        uint8_t addr_type = app_lea_adv_target_addr[i].type;
//        const uint8_t *addr = app_lea_adv_target_addr[i].addr;
//        APPS_LOG_MSGID_I(LOG_TAG" printf_target_addr, [%d] addr_type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
//                         8, i, addr_type, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
//    }
}

static bool app_lea_update_target_addr_imp(app_le_audio_update_target_mode_t mode,
                                           bt_addr_type_t addr_type,
                                           const uint8_t *addr)
{
    bool ret = FALSE;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    APPS_LOG_MSGID_I(LOG_TAG" update_target_addr, [%02X] mode=%d addr_type=%d addr=%08X%04X",
                     5, role, mode, addr_type, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
    //app_lea_print_target_addr();

    if (mode == APP_LEA_TARGET_ADD_ADDR && app_lea_adv_target_addr_num == APP_LEA_MAX_TARGET_NUM) {
        APPS_LOG_MSGID_E(LOG_TAG" update_target_addr, [%02X] error target_addr_list full", 1, role);
        goto exit;
    } else if ((mode == APP_LEA_TARGET_ADD_ADDR || mode == APP_LEA_TARGET_SET_UNIQUE_ADDR || mode == APP_LEA_TARGET_REMOVE_ADDR)
               && APP_LEA_IS_EMPTY_ADDR(addr, empty_addr)) {
        APPS_LOG_MSGID_E(LOG_TAG" update_target_addr, [%02X] error addr", 1, role);
        goto exit;
    }

    switch (mode) {
        case APP_LEA_TARGET_ADD_ADDR: {
            for (int i = 0; i < APP_LEA_MAX_TARGET_NUM; i++) {
                uint8_t *target_addr = (uint8_t *)app_lea_adv_target_addr[i].addr;
                if (APP_LEA_IS_EMPTY_ADDR(target_addr, empty_addr)) {
                    app_lea_adv_target_addr[i].type = addr_type;
                    memcpy(target_addr, addr, BT_BD_ADDR_LEN);
                    app_lea_adv_target_addr_num += 1;
                    ret = TRUE;
                    break;
                } else if (memcmp(target_addr, addr, BT_BD_ADDR_LEN) == 0) {
                    ret = TRUE;
                    break;
                }
            }
            break;
        }

        case APP_LEA_TARGET_SET_UNIQUE_ADDR: {
            for (int i = 0; i < APP_LEA_MAX_TARGET_NUM; i++) {
                uint8_t *target_addr = (uint8_t *)app_lea_adv_target_addr[i].addr;
                if (i == 0) {
                    app_lea_adv_target_addr[0].type = addr_type;
                    memcpy(target_addr, addr, BT_BD_ADDR_LEN);
                } else {
                    app_lea_adv_target_addr[i].type = 0;
                    memset(target_addr, 0, BT_BD_ADDR_LEN);
                }
            }
            app_lea_adv_target_addr_num = 1;
            ret = TRUE;
            break;
        }

        case APP_LEA_TARGET_REMOVE_ADDR: {
            for (int i = 0; i < APP_LEA_MAX_TARGET_NUM; i++) {
                uint8_t *target_addr = (uint8_t *)app_lea_adv_target_addr[i].addr;
                if (memcmp(addr, target_addr, BT_BD_ADDR_LEN) == 0) {
                    APPS_LOG_MSGID_I(LOG_TAG" update_target_addr, [%02X] clear index=%d", 2, role, i);
                    app_lea_adv_target_addr[i].type = 0;
                    memset(target_addr, 0, BT_BD_ADDR_LEN);
                    app_lea_adv_target_addr_num -= 1;
                    ret = TRUE;
                    break;
                }
            }
            break;
        }
    }

exit:
    configASSERT(app_lea_adv_target_addr_num <= APP_LEA_MAX_TARGET_NUM);

    if (ret) {
        app_lea_print_target_addr();
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" update_target_addr, [%02X] error mode=%d", 2, role, mode);
    }
    return ret;
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_lea_clear_target_addr(bool clear_whitelist)
{
    //bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    //APPS_LOG_MSGID_I(LOG_TAG" [%02X] clear_target_addr", 1, role);
    app_lea_adv_target_addr_num = 0;
    memset(app_lea_adv_target_addr, 0, sizeof(bt_addr_t) * APP_LEA_MAX_TARGET_NUM);
    if (clear_whitelist) {
        app_lea_target_update_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, 0, NULL);
    }
}

void app_lea_update_target_add_white_list(bool need_targeted_flag)
{
    app_lea_clear_target_addr(TRUE);

    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    bt_addr_t addr_list[APP_LEA_MAX_TARGET_NUM] = {0};
    uint8_t list_num = APP_LEA_MAX_TARGET_NUM;
    if (need_targeted_flag) {
        app_lea_conn_mgr_get_reconnect_addr(APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT, addr_list, &list_num);
    } else {
        app_lea_conn_mgr_get_reconnect_addr(APP_LEA_ADV_SUB_MODE_UNACTIVE, addr_list, &list_num);
    }

    for (int i = 0; i < list_num; i++) {
        uint8_t *target_addr = (uint8_t *)addr_list[i].addr;
        uint8_t target_addr_type = (uint8_t)addr_list[i].type;
        if (memcmp(target_addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
            continue;
        }
        app_lea_update_target_addr_imp(APP_LEA_TARGET_ADD_ADDR, target_addr_type, target_addr);
        app_lea_target_update_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, target_addr_type, target_addr);
    }
}

void app_lea_add_white_list(void)
{
    app_lea_target_update_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, 0, NULL);

    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < app_lea_adv_target_addr_num; i++) {
        uint8_t *target_addr = (uint8_t *)app_lea_adv_target_addr[i].addr;
        uint8_t target_addr_type = (uint8_t)app_lea_adv_target_addr[i].type;
        if (memcmp(target_addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
            continue;
        }
        app_lea_target_update_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, target_addr_type, target_addr);
    }
}

bool app_lea_update_target_addr(app_le_audio_update_target_mode_t mode,
                                bt_addr_type_t addr_type,
                                const uint8_t *addr)
{
    return app_lea_update_target_addr_imp(mode, addr_type, addr);
}

#endif
