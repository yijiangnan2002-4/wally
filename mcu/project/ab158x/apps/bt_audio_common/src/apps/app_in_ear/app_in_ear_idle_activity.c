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
 * File: app_in_ear_idle_activity.c
 *
 * Description:
 * When the earbuds is taken out of the ear or put into the ear, this activity will
 * receive the events which contains the in ear state. This activity will handle these
 * events and notify other apps that the wearing status has changed.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#ifdef MTK_IN_EAR_FEATURE_ENABLE

#include "FreeRTOS.h"
#include "app_in_ear_idle_activity.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "bt_sink_srv.h"
#include "bt_role_handover.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#include "apps_config_key_remapper.h"
#include "nvdm.h"
#include "bt_sink_srv_ami.h"
#include "bt_device_manager.h"
#include "bt_connection_manager.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"


#define APP_INEAR_TAG "[In ear]idle_activity"

static apps_in_ear_local_context_t s_app_in_ear_context;    /* The variable records context. */

static bool app_in_ear_play_press_vp()
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = VP_INDEX_PRESS;
    APPS_LOG_MSGID_I(APP_INEAR_TAG"Play Press Vp", 0);
    return (VP_STATUS_SUCCESS == voice_prompt_play(&vp, NULL));
}


static apps_in_ear_local_context_t *app_in_ear_get_app_context()
{
    return &s_app_in_ear_context;
}

bool app_in_ear_get_own_state()
{
    return s_app_in_ear_context.isInEar;
}

bool app_in_ear_get_peer_state()
{
#ifdef MTK_AWS_MCE_ENABLE
    return s_app_in_ear_context.isPartnerInEar;
#else
    return FALSE;
#endif
}

app_in_ear_state_t app_in_ear_get_state()
{
    apps_in_ear_local_context_t *ctx = app_in_ear_get_app_context();
    return ctx->curState;
}

app_in_ear_ohd_state_t app_in_ear_get_ohd_state()
{
    apps_in_ear_local_context_t *ctx = app_in_ear_get_app_context();
#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
        return APP_IN_EAR_OHD_DISABLED;
    }

    audio_channel_t channel = ami_get_audio_channel();
    switch (ctx->curState) {
        case APP_IN_EAR_STA_BOTH_IN:
            return APP_IN_EAR_OHD_BOTH_DETECTED;
        case APP_IN_EAR_STA_BOTH_OUT:
            return APP_IN_EAR_OHD_NONE_DETECTED;
        case APP_IN_EAR_STA_AIN_POUT:
            return AUDIO_CHANNEL_L == channel ? APP_IN_EAR_OHD_LEFT_DETECTED : APP_IN_EAR_OHD_RIGHT_DETECTED;
        case APP_IN_EAR_STA_AOUT_PIN:
            return AUDIO_CHANNEL_R == channel ? APP_IN_EAR_OHD_LEFT_DETECTED : APP_IN_EAR_OHD_RIGHT_DETECTED;
    }
    return APP_IN_EAR_OHD_DISABLED;
#else
    return APP_IN_EAR_STA_BOTH_IN == ctx->curState ? APP_IN_EAR_OHD_DETECTED : APP_IN_EAR_OHD_NONE_DETECTED;
#endif
}

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
static uint8_t _role_handover_service_get_data_len_callback(const bt_bd_addr_t *addr)
{
    uint8_t data_len = 0;
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == addr)
#endif
    {
        data_len = sizeof(apps_in_ear_local_context_t);
    }
    return data_len;
}


static bt_status_t _role_handover_get_data_callback(const bt_bd_addr_t *addr, void *data)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == addr)
#endif
    {
        apps_in_ear_local_context_t *ctx = app_in_ear_get_app_context();
        if (NULL == ctx) {
            return BT_STATUS_FAIL;
        }
        apps_in_ear_local_context_t *pdata = (apps_in_ear_local_context_t *)data;
        memcpy(pdata, ctx, sizeof(apps_in_ear_local_context_t));
    }
    return BT_STATUS_SUCCESS;
}


static bt_status_t _role_handover_update_data_callback(bt_role_handover_update_info_t *info)
{
    bool needUpdate = false;
    app_in_ear_state_t currentStaTemp;

    apps_in_ear_local_context_t *ctx = app_in_ear_get_app_context();

    //APPS_LOG_MSGID_I(APP_INEAR_TAG" update_data_callback recv, role=0x%02X", 1, info->role);
    if (NULL == ctx) {
        return BT_STATUS_FAIL;
    }
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == info->addr)
#endif
    {
        /* Synchronize the status of the old agent to the new agent. */
        if (info->role == BT_AWS_MCE_ROLE_PARTNER) {
            apps_in_ear_local_context_t *pdata = (apps_in_ear_local_context_t *)info->data;

            APPS_LOG_MSGID_I(APP_INEAR_TAG" update_data_callback old agent[inEar=%d, partnerIn=%d, previousSta=%d, currentSta=%d], new agent[inEar=%d, partnerIn=%d, previousSta=%d, currentSta=%d]",
                             8, pdata->isInEar, pdata->isPartnerInEar, pdata->preState, pdata->curState,
                             ctx->isInEar, ctx->isPartnerInEar, ctx->preState, ctx->curState);

            ctx->isPartnerInEar = pdata->isInEar;
            ctx->preState       = pdata->preState;
            ctx->curState       = pdata->curState;
            ctx->eventDone      = pdata->eventDone;
            ctx->eventOutEnable = pdata->eventOutEnable;
            ctx->rhoEnable      = pdata->rhoEnable;

            /* Check the old partner in-ear status. */
            if (pdata->isPartnerInEar != ctx->isInEar) {
                needUpdate = true;
            }

            /* Check the old agent in-ear status. */
            if (pdata->isInEar &&
                pdata->curState != APP_IN_EAR_STA_AIN_POUT &&
                pdata->curState != APP_IN_EAR_STA_BOTH_IN) {
                needUpdate = true;
            }

            if (!pdata->isInEar &&
                pdata->curState != APP_IN_EAR_STA_AOUT_PIN &&
                pdata->curState != APP_IN_EAR_STA_BOTH_OUT) {
                needUpdate = true;
            }

            /* Because state changes may occur during RHO, the current state needs to be checked and updated. */
            if (needUpdate) {
                APPS_LOG_MSGID_I(APP_INEAR_TAG" update_data_callback1 old agent[inEar=%d, partnerIn=%d, previousSta=%d, currentSta=%d] : new agent[inEar=%d, partnerIn=%d, -previousSta=%d, currentSta=%d]",
                                 8, pdata->isInEar, pdata->isPartnerInEar, pdata->preState, pdata->curState,
                                 ctx->isInEar, ctx->isPartnerInEar, ctx->preState, ctx->curState);
                if (ctx->isInEar) {
                    /* New agent is in the ear. */
                    if (ctx->isPartnerInEar) {
                        currentStaTemp = APP_IN_EAR_STA_BOTH_IN;
                    } else {
                        currentStaTemp = APP_IN_EAR_STA_AIN_POUT;
                    }
                } else {
                    /* New agent is not in the ear. */
                    if (ctx->isPartnerInEar) {
                        currentStaTemp = APP_IN_EAR_STA_AOUT_PIN;
                    } else {
                        currentStaTemp = APP_IN_EAR_STA_BOTH_OUT;
                    }
                }

                ctx->preState = ctx->curState;
                ctx->curState = currentStaTemp;
                ctx->eventDone = false;
            }
        }
    }

    return BT_STATUS_SUCCESS;
}

static void _role_handover_service_status_callback(const bt_bd_addr_t *addr,
                                                   bt_aws_mce_role_t role,
                                                   bt_role_handover_event_t event,
                                                   bt_status_t status)
{

    apps_in_ear_local_context_t *ctx = app_in_ear_get_app_context();
    if (NULL == ctx) {
        return;
    }

    if (BT_ROLE_HANDOVER_COMPLETE_IND == event) {
        ctx->isInRho = false;

        if (role == BT_AWS_MCE_ROLE_PARTNER && BT_STATUS_SUCCESS == status && !ctx->eventDone) {
            /* Update current state after RHO. */
            if ((ctx->curState == APP_IN_EAR_STA_AOUT_PIN) && ctx->isInEar && !ctx->isPartnerInEar) {
                ctx->curState = APP_IN_EAR_STA_AIN_POUT;
            }

            app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)pvPortMalloc(sizeof(app_in_ear_sta_info_t));
            if (sta_info) {
                sta_info->previous = ctx->preState;
                sta_info->current = ctx->curState;
#ifdef IN_EAR_DEBUG
                printStaToSysUART(ctx);
#endif
                /* Notify other apps that the wearing status has changed. */
                ctx->eventDone = true;
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA,
                                    (void *)sta_info, sizeof(app_in_ear_sta_info_t),
                                    NULL, 0);

                APPS_LOG_MSGID_I(APP_INEAR_TAG" rho_service_status_cb send interaction, currentSta=%d, previousSta: %d", 2,
                                 ctx->curState, ctx->preState);
            }
        }

        /* Because state changes may occur during RHO, the new partner must report state to new agent. */
        if (role == BT_AWS_MCE_ROLE_AGENT && BT_STATUS_SUCCESS == status) {
            app_in_ear_send_aws_data(ctx, APP_IN_EAR_EVENT_UPDATE_STA);
        }

        if (BT_STATUS_FAIL == status) {
            APPS_LOG_MSGID_E(APP_INEAR_TAG" rho_service_status_cb rho failed.", 0);
        }
    }
}
#endif


static bool _proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            self->local_context = &s_app_in_ear_context;
            apps_in_ear_local_context_t *ctx = &s_app_in_ear_context;
#ifndef MTK_EINT_KEY_ENABLE
            ctx->isInEar = FALSE;
#else
            ctx->isInEar  = app_in_ear_get_wearing_status();
#endif

#ifdef MTK_AWS_MCE_ENABLE
            if (ctx->isInEar) {
                ctx->curState = APP_IN_EAR_STA_AIN_POUT;
            } else {
                ctx->curState = APP_IN_EAR_STA_BOTH_OUT;
            }
            ctx->isPartnerInEar = false;
#else
            if (ctx->isInEar) {
                ctx->curState = APP_IN_EAR_STA_BOTH_IN;
            } else {
                ctx->curState = APP_IN_EAR_STA_BOTH_OUT;
            }
#endif
            ctx->preState = ctx->curState;
#ifdef MTK_AWS_MCE_ENABLE
            ctx->isInRho = false;
            ctx->rhoEnable = true;
#endif
            ctx->eventDone = true;
            ctx->eventOutEnable = true;

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
            bt_role_handover_callbacks_t role_callbacks = {
                NULL,
                _role_handover_service_get_data_len_callback,
                _role_handover_get_data_callback,
                _role_handover_update_data_callback,
                _role_handover_service_status_callback
            };
            bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_IN_EAR, &role_callbacks);
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_in_ear_proc_bt_cm_events(ui_shell_activity_t *self,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
#ifdef MTK_AWS_MCE_ENABLE
    apps_in_ear_local_context_t *ctx = (apps_in_ear_local_context_t *)self->local_context;
    bt_aws_mce_role_t role;
    role = bt_device_manager_aws_local_info_get_role();
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == ctx || NULL == remote_update) {
                break;
            }
            bt_cm_profile_service_mask_t profile_srv = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
                                                       | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);

            if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role) {
                if (!(remote_update->pre_connected_service & profile_srv) && (remote_update->connected_service & profile_srv)) {
                    APPS_LOG_MSGID_I(APP_INEAR_TAG" app_in_ear_proc_bt_cm_events, connected: rhoEnable=%d, curState=%d, isInRho=%d.",
                                     3, ctx->rhoEnable, ctx->curState, ctx->isInRho);
                    if (ctx->rhoEnable && ctx->curState == APP_IN_EAR_STA_AOUT_PIN) {
                        if (!ctx->isInRho) {
                            app_in_ear_update_status(ctx);
                        }
                    }
                }
            } else if (BT_AWS_MCE_ROLE_PARTNER == role) {
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                        && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    APPS_LOG_MSGID_I(APP_INEAR_TAG" [aws connect] partner report status to agent: in-ear=%d", 1, ctx->isInEar);
                    app_in_ear_send_aws_data(ctx, APP_IN_EAR_EVENT_UPDATE_STA);
                }
            }

            break;
        }
        default:
            break;
    }
#endif

    return false;
}

static bool app_in_ear_proc_apps_internal_events(ui_shell_activity_t *self,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len)
{
    bool ret = false;
    apps_in_ear_local_context_t *ctx = (apps_in_ear_local_context_t *)self->local_context;

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    if ((APPS_EVENTS_INTERACTION_RHO_END == event_id) || (APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT == event_id)) {
        ctx->isInRho = false;
    }
#endif

    if (APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT == event_id) {
        /*
         * extra_data is not NULL means the event is come from the driver callback.
         * extra_data is NULL means the the state already updated, just need to handle it.
         */
        if (extra_data != NULL) {
            bool *pstatus = (bool *)extra_data;
            ctx->isInEar = *pstatus;
            if (ctx->isInEar) {
                app_in_ear_play_press_vp();
            }
        }
        APPS_LOG_MSGID_I(APP_INEAR_TAG" [IN_EAR_UPDATE] isInEar=%d", 1, ctx->isInEar);
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
        /* Events are not handled during RHO, and the new agent will handle them. */
        APPS_LOG_MSGID_I(APP_INEAR_TAG" [IN_EAR_UPDATE] isInRho=%d", 1, ctx->isInRho);
        if (ctx->isInRho) {
            return false;
        }
#endif

        /* For agent, check and update state. For partner, report state to agent */
#ifdef MTK_AWS_MCE_ENABLE
        if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
#ifdef AIR_LE_AUDIO_ENABLE
            if (BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
                app_in_ear_update_status(ctx);
            } else
#endif
            {
                APPS_LOG_MSGID_I(APP_INEAR_TAG" [IN_EAR_UPDATE] partner report status to agent: in-ear=%d", 1, ctx->isInEar);
                app_in_ear_send_aws_data(ctx, APP_IN_EAR_EVENT_UPDATE_STA);
            }
        } else
#endif
        {
            app_in_ear_update_status(ctx);
        }
    }
    return ret;
}


#ifdef MTK_AWS_MCE_ENABLE
static bool app_in_ear_proc_aws_data_events(ui_shell_activity_t *self,
                                            uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    apps_in_ear_local_context_t *ctx = (apps_in_ear_local_context_t *)self->local_context;

    if (aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_IN_EAR) {
        return false;
    }

    app_in_ear_aws_data_t data;
    memcpy(&data, aws_data_ind->param, sizeof(app_in_ear_aws_data_t));
    if (data.event == APP_IN_EAR_EVENT_UPDATE_STA) {
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        if (role == BT_AWS_MCE_ROLE_AGENT) {
            if (!ctx->isInRho) {
                ctx->isPartnerInEar = data.isInEar;
                app_in_ear_update_status(ctx);
            }
        }
        /*
         * In this case, old partner report in ear state at first, and then the old agent start rho.
         * After rho finished, old partner's data received by the new partner.
         */
        APPS_LOG_MSGID_I(APP_INEAR_TAG" aws_data_even [0x%02x] received partner's status: in-ear=%d, isInRho=%d",
                         3, role, data.isInEar, ctx->isInRho);
    }

    return ret;
}
#endif

bool app_in_ear_activity_proc(ui_shell_activity_t *self,
                              uint32_t event_group,
                              uint32_t event_id,
                              void *extra_data,
                              size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            /* BT connect manager events. */
            ret = app_in_ear_proc_bt_cm_events(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            /* Interaction events. */
            ret = app_in_ear_proc_apps_internal_events(self, event_id, extra_data, data_len);
            break;
#if defined(MTK_AWS_MCE_ENABLE)
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            /* The event come from partner. */
            ret = app_in_ear_proc_aws_data_events(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }
    return ret;

}

#endif /*MTK_IN_EAR_FEATURE_ENABLE*/
