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
 * File: app_smcharger_off_activity.c
 *
 * Description: This file is the activity to handle action/event in OFF state.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for SmartCharger state machine.
 *
 */


#ifdef AIR_SMART_CHARGER_ENABLE

#include "app_smcharger_off_activity.h"

#ifdef AIR_TILE_ENABLE
#include "app_tile.h"
#include "app_smcharger_startup_activity.h"
#endif

#define LOG_TAG "[SMCharger][OFF]"

static bool smcharger_off_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            //APPS_LOG_MSGID_I(LOG_TAG" create", 0);
            /* Do OFF action - power off device. */
            app_smcharger_state_do_action(STATE_SMCHARGER_OFF);
#ifdef AIR_TILE_ENABLE
            if (app_tile_tmd_is_active()) {
                /* Send restart smart charger case activiry */
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_CHARGER_CASE, SMCHARGER_EVENT_RESTARTUP, NULL, 0, NULL, 3 * 1000);
            }
#endif
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            //APPS_LOG_MSGID_I(LOG_TAG" destroy", 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

bool app_smcharger_off_activity_proc(struct _ui_shell_activity *self, uint32_t event_group, uint32_t event_id,
                                     void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        /* UI Shell internal events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = smcharger_off_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        /* APP SmartCharger events - ignore and block all event. */
        case EVENT_GROUP_UI_SHELL_CHARGER_CASE: {
#ifdef AIR_TILE_ENABLE
            if (event_id == SMCHARGER_EVENT_RESTARTUP) {
                /* Start STARTUP activity (start SmartCharger APP state machine). */
                ui_shell_start_activity(self, app_smcharger_startup_activity_proc, ACTIVITY_PRIORITY_MIDDLE, (void *)NULL, 0);
                ui_shell_finish_activity(self, self);
            }

#else
            APPS_LOG_MSGID_E(LOG_TAG" CHARGER_CASE group, unexpected event_id=%d, ret=TRUE", 1, event_id);
#endif
            ret = true;
            break;
        }
        default:
            break;
    }
    return ret;
}

#endif
