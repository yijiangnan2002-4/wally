/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
 * File: app_call_rx_eq.c
 *
 * Description:
 * This activity to set and get the rx eq of the call.
 *
 */


#include "app_rx_eq.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_srv.h"
#endif

#include "bt_type.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_call.h"
#include "bt_device_manager.h"
#include "bt_connection_manager.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le_cap_audio_manager.h"
#endif
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
#include "bt_sink_srv_ami.h"
#endif

#include "hal_gpt.h"
#include "hal_audio_message_struct_common.h"

#ifdef MTK_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif

#define LOG_TAG     "[CALL_RX_EQ]"

//#define APP_CALL_RX_EQ_ENABLE

typedef struct {
    app_call_rx_eq_type_t   call_rx_eq_type;
    bt_sink_srv_state_t     curr_sink_state;
    bt_sink_srv_device_t    call_type;
} app_rx_eq_context_t;

static app_rx_eq_context_t s_app_rx_eq_context;

static void app_call_rx_eq_type_load(void)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t size = sizeof(app_call_rx_eq_type_t);
    nvkey_status_t status = nvkey_read_data(NVID_APP_CALL_RX_EQ_TYPE, (uint8_t *) & (s_app_rx_eq_context.call_rx_eq_type), &size);
    if (status == NVKEY_STATUS_ITEM_NOT_FOUND) {
        s_app_rx_eq_context.call_rx_eq_type = APP_CALL_RX_EQ_TYPE_0;
        app_call_rx_eq_type_t temp_rx_eq_type = s_app_rx_eq_context.call_rx_eq_type;
        nvkey_write_data(NVID_APP_CALL_RX_EQ_TYPE, (const uint8_t *)&temp_rx_eq_type, size);
    }
#endif
    APPS_LOG_MSGID_I(LOG_TAG" eq_type_load: type=%d", 1, s_app_rx_eq_context.call_rx_eq_type);
}

static void app_call_rx_eq_type_save(void)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t size = sizeof(app_call_rx_eq_type_t);
    app_call_rx_eq_type_t temp_rx_eq_type = s_app_rx_eq_context.call_rx_eq_type;
    nvkey_write_data(NVID_APP_CALL_RX_EQ_TYPE, (const uint8_t *)&temp_rx_eq_type, size);
#endif
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_call_rx_eq_type_sync(app_call_rx_eq_type_t type, bt_clock_t bt_clk)
{
    bool ret = false;
    app_call_rx_eq_sync_t sync_data = {0};
    sync_data.rx_eq_type            = type;
    sync_data.bt_clk                = bt_clk;

    if (apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_SYNC_CALL_RX_EQ_STATUS,
                                       (void *)&sync_data, sizeof(app_call_rx_eq_sync_t)) == BT_STATUS_SUCCESS) {
        ret = true;
    }
    APPS_LOG_MSGID_I(LOG_TAG" rx_eq_type_sync: ret=%d", 1, ret);
    return ret;
}
#endif

static bool app_call_rx_eq_type_local_set(app_call_rx_eq_type_t type, uint32_t target_gpt)
{
    bool ret = true;
    uint32_t gpt_count = 0;
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    bool set_eq_ret = false;
    set_eq_ret = voice_set_RX_EQ_mode(MCU2DSP_SYNC_REQUEST_HFP, type);
    set_eq_ret = voice_set_RX_EQ_mode(MCU2DSP_SYNC_REQUEST_BLE, type);
    if (!set_eq_ret) {
        return false;
    }
#endif
    s_app_rx_eq_context.call_rx_eq_type = type;

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    bt_sink_srv_am_audio_sync_capability_t cap = {0};
#endif

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    APPS_LOG_MSGID_I(LOG_TAG" local set gpt(0x%08x), current(0x%08x)", 2, target_gpt, gpt_count);
    if (gpt_count >= target_gpt) {
        target_gpt = gpt_count;
    }

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    if (target_gpt != 0) {
        cap.sync_scenario_type = (s_app_rx_eq_context.call_type == BT_SINK_SRV_DEVICE_EDR) ? MCU2DSP_SYNC_REQUEST_HFP : MCU2DSP_SYNC_REQUEST_BLE;
        cap.sync_action_type   = MCU2DSP_SYNC_REQUEST_SET_RX_EQ;
        cap.target_gpt_cnt     = target_gpt;
        bt_sink_srv_am_id_t aud_id = AUD_ID_INVALID;

        if (s_app_rx_eq_context.call_type == BT_SINK_SRV_DEVICE_EDR) {
            aud_id = bt_sink_srv_call_get_playing_audio_id();
        } else if (s_app_rx_eq_context.call_type == BT_SINK_SRV_DEVICE_LE) {
#ifdef AIR_LE_AUDIO_ENABLE
            aud_id = bt_sink_srv_cap_am_get_aid();
#endif
        }

        APPS_LOG_MSGID_I(LOG_TAG" local set: scenario_type=%d, aud_id=%d", 2, cap.sync_scenario_type, aud_id);
        if (aud_id == AUD_ID_INVALID) {
            return false;
        }
        if (bt_sink_srv_ami_audio_request_sync(aud_id, &cap) != AUD_EXECUTION_SUCCESS) {
            ret = false;
        }
    }
#endif

    return ret;
}


app_call_rx_eq_set_status_t app_call_rx_eq_type_set(app_call_rx_eq_type_t type)
{
    app_call_rx_eq_set_status_t set_status = APP_CALL_RX_EQ_SET_ERROR;


    // if (s_app_rx_eq_context.call_type == BT_SINK_SRV_DEVICE_INVALID) {
    //     goto exit;
    // }

#ifdef MTK_AWS_MCE_ENABLE
    bt_clock_t bt_clk = {0};
    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        if (bt_sink_srv_bt_clock_addition(&bt_clk, 0, APP_CALL_RX_EQ_SYNC_DELAY * 1000) != BT_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(LOG_TAG" set_and_sync, get bt_clk fail", 0);
            return set_status;
        }

        bool sync_ret = app_call_rx_eq_type_sync(type, bt_clk);
        if (sync_ret == true) {
            uint32_t local_gpt = 0;
            if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (bt_clk), (uint32_t *)&local_gpt) == BT_STATUS_SUCCESS) {
                if (app_call_rx_eq_type_local_set(type, local_gpt)) {
                    set_status = APP_CALL_RX_EQ_SET_SUCCESS;
                }

            } else {
                APPS_LOG_MSGID_E(LOG_TAG" set_and_sync, bt_clk to gpt fail", 0);
            }
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" set_and_sync, sync eq type fail", 0);
        }
    } else {
        if (app_call_rx_eq_type_local_set(type, 0)) {
            set_status = APP_CALL_RX_EQ_SET_SUCCESS;
        }
    }
#else
    if (app_call_rx_eq_type_local_set(type, 0)) {
        set_status = APP_CALL_RX_EQ_SET_SUCCESS;
    }
#endif

    APPS_LOG_MSGID_I(LOG_TAG" set_rx_eq: type=%d, status=%d, call_type=%d", 3, type, set_status, s_app_rx_eq_context.call_type);

    return set_status;
}

app_call_rx_eq_get_status_t app_call_rx_eq_type_get(app_call_rx_eq_type_t *type)
{
    app_call_rx_eq_get_status_t get_status = APP_CALL_RX_EQ_GET_SUCCESS;
    app_call_rx_eq_type_t *get_type = type;

    if (get_type == NULL) {
        return APP_CALL_RX_EQ_GET_ERROR;
    }

    *get_type = s_app_rx_eq_context.call_rx_eq_type;

    APPS_LOG_MSGID_I(LOG_TAG" get_rx_eq=%d", 1, s_app_rx_eq_context.call_rx_eq_type);

    return get_status;
}

static bool app_rx_eq_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(LOG_TAG", create.", 0);
            self->local_context = &s_app_rx_eq_context;
            memset(self->local_context, 0, sizeof(app_rx_eq_context_t));
            app_call_rx_eq_type_load();
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
            voice_set_RX_EQ_mode(MCU2DSP_SYNC_REQUEST_HFP, s_app_rx_eq_context.call_rx_eq_type);
            voice_set_RX_EQ_mode(MCU2DSP_SYNC_REQUEST_BLE, s_app_rx_eq_context.call_rx_eq_type);
#endif
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            APPS_LOG_MSGID_I(LOG_TAG", destroy", 0);
            app_call_rx_eq_type_save();
            break;
        }

        default:
            ret = false;
            break;
    }
    return ret;
}


#ifdef MTK_AWS_MCE_ENABLE
static bool app_rx_eq_proc_aws_data(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        //apps_aws_sync_event_decode(aws_data_ind, &event_group, &action);
        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action,
                                         &p_extra_data, &extra_data_len);
        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
            && action == APPS_EVENTS_INTERACTION_SYNC_CALL_RX_EQ_STATUS) {
            if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
                app_call_rx_eq_sync_t *rx_eq_sync_data = (app_call_rx_eq_sync_t *)p_extra_data;
                app_rx_eq_context_t *rx_eq_context = (app_rx_eq_context_t *)self->local_context;
                if (rx_eq_context == NULL || rx_eq_sync_data == NULL) {
                    return ret;
                }
                uint32_t target_gpt = 0;
                rx_eq_context->call_rx_eq_type = rx_eq_sync_data->rx_eq_type;

                APPS_LOG_MSGID_I(LOG_TAG" [AWS_DATA] rx_eq_type=%d, bt_clk 0x%08x 0x%08x",
                                 3, rx_eq_context->call_rx_eq_type, rx_eq_sync_data->bt_clk.nclk, rx_eq_sync_data->bt_clk.nclk_intra);
                if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *)&rx_eq_sync_data->bt_clk, &target_gpt) == BT_STATUS_SUCCESS) {
                    app_call_rx_eq_type_local_set(rx_eq_context->call_rx_eq_type, target_gpt);
                } else {
                    app_call_rx_eq_type_local_set(rx_eq_context->call_rx_eq_type, 0);
                }
            }
        }
    }
    return ret;
}
#endif

static void app_rx_eq_update_call_type(void)
{
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_device_state_t curr_device_state = {0};
    bt_status_t get_ret = bt_sink_srv_get_playing_device_state(&curr_device_state);
    if (get_ret == BT_STATUS_SUCCESS) {
        s_app_rx_eq_context.call_type = curr_device_state.type;
    }
    APPS_LOG_MSGID_I(LOG_TAG" update_call_type: ret=0x%x, call_type=%d.", 2, get_ret, s_app_rx_eq_context.call_type);
#else
    APPS_LOG_MSGID_I(LOG_TAG" update_call_type fail.", 0);
#endif
}

bool app_rx_eq_activity_proc(ui_shell_activity_t *self,
                             uint32_t event_group,
                             uint32_t event_id,
                             void *extra_data,
                             size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* ui_shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_rx_eq_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            app_rx_eq_context_t *rx_eq_context = (app_rx_eq_context_t *)(self->local_context);
            switch (event_id) {
                case BT_SINK_SRV_EVENT_STATE_CHANGE: {
                    bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;
                    if (param == NULL) {
                        return ret;
                    }
                    if (param->current > BT_SINK_SRV_STATE_STREAMING) {
                        rx_eq_context->curr_sink_state = param->current;
                        APPS_LOG_MSGID_I(LOG_TAG" sink state change: state=0x%x.", 1, rx_eq_context->curr_sink_state);
                        app_rx_eq_update_call_type();
                    }
                    break;
                }
                case BT_SINK_SRV_EVENT_HF_SCO_STATE_UPDATE: {
                    bt_sink_srv_sco_state_update_t *sco_state = (bt_sink_srv_sco_state_update_t *)extra_data;
                    if (sco_state != NULL) {
                        APPS_LOG_MSGID_I(LOG_TAG", SCO_STATE: %d", 1, sco_state->state);
                        if (sco_state->state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                            app_rx_eq_update_call_type();
                        }
                    }

                    break;
                }
            }
            break;
        }
#if defined(MTK_AWS_MCE_ENABLE)
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
                app_rx_eq_context_t *rx_eq_context = (app_rx_eq_context_t *)(self->local_context);
                bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
                if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                    if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                        && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                        bt_clock_t bt_clk = {0};
                        app_call_rx_eq_type_sync(rx_eq_context->call_rx_eq_type, bt_clk);
                    }
                }

            }
            break;
        }
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            /* Handle the key event sync from the partner side. */
            ret = app_rx_eq_proc_aws_data(self, event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}

