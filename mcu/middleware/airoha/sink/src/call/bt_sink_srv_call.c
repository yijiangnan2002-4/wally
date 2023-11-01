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

#include "bt_sink_srv.h"
#include "bt_sink_srv_common.h"
#include "bt_sink_srv_call_pseudo_dev.h"
#include "bt_sink_srv_call_pseudo_dev_mgr.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_call.h"
#include "bt_sink_srv_hf.h"
#include "bt_sink_srv_hsp.h"
#include "bt_sink_srv_aws_mce_call.h"
#include "bt_sink_srv_state_manager.h"

static bt_sink_srv_call_context_t g_sink_srv_call_context;

extern bt_status_t bt_sink_srv_hf_action_handler(bt_sink_srv_action_t action, void *parameters);
extern bt_status_t bt_sink_srv_hf_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
extern bt_status_t bt_sink_srv_hf_set_mute(bt_sink_srv_mute_t type, bool mute);

extern bt_status_t bt_sink_srv_hsp_action_handler(bt_sink_srv_action_t action, void *parameters);
extern bt_status_t bt_sink_srv_hsp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
extern bt_status_t bt_sink_srv_hsp_set_mute(bt_sink_srv_mute_t type, bool mute);

extern bt_status_t bt_sink_srv_aws_mce_call_action_handler(bt_sink_srv_action_t action, void *parameters);
extern bt_status_t bt_sink_srv_aws_mce_call_set_mute(bt_sink_srv_mute_t type, bool mute);
extern bt_sink_srv_hf_call_state_t bt_sink_srv_aws_mce_call_transfer_aws_call_state(bt_sink_srv_aws_mce_call_state_t state);

#ifdef MTK_BT_CM_SUPPORT
extern bt_status_t  bt_sink_srv_hf_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data);
extern bt_status_t  bt_sink_srv_hsp_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data);
#endif

bt_status_t default_bt_sink_srv_hf_action_handler(bt_sink_srv_action_t action, void *parameters)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_sink_srv_hsp_action_handler(bt_sink_srv_action_t action, void *parameters)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_sink_srv_aws_mce_call_action_handler(bt_sink_srv_action_t action, void *parameters)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_sink_srv_hf_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_sink_srv_hf_gap_callback(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_sink_srv_hsp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    return BT_STATUS_SUCCESS;
}

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_sink_srv_hf_action_handler=_default_bt_sink_srv_hf_action_handler")
#pragma comment(linker, "/alternatename:_bt_sink_srv_hsp_action_handler=_default_bt_sink_srv_hsp_action_handler")
#pragma comment(linker, "/alternatename:_bt_sink_srv_aws_mce_call_action_handler=_default_bt_sink_srv_aws_mce_call_action_handler")
#pragma comment(linker, "/alternatename:_bt_sink_srv_hf_common_callback=_default_bt_sink_srv_hf_common_callback")
#pragma comment(linker, "/alternatename:_bt_sink_srv_hf_gap_callback=_default_bt_sink_srv_hf_gap_callback")
#pragma comment(linker, "/alternatename:_bt_sink_srv_hsp_common_callback=_default_bt_sink_srv_hsp_common_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_srv_hf_action_handler = default_bt_sink_srv_hf_action_handler
#pragma weak bt_sink_srv_hsp_action_handler = default_bt_sink_srv_hsp_action_handler
#pragma weak bt_sink_srv_aws_mce_call_action_handler = default_bt_sink_srv_aws_mce_call_action_handler
#pragma weak bt_sink_srv_hf_common_callback = default_bt_sink_srv_hf_common_callback
#pragma weak bt_sink_srv_hf_gap_callback = default_bt_sink_srv_hf_gap_callback
#pragma weak bt_sink_srv_hsp_common_callback = default_bt_sink_srv_hsp_common_callback
#else
#error "Unsupported Platform"
#endif

#ifdef MTK_BT_CM_SUPPORT
void bt_sink_srv_call_init(void)
{
    bt_cm_profile_service_register(BT_CM_PROFILE_SERVICE_HFP, (bt_cm_profile_service_handle_callback_t)bt_sink_srv_hf_cm_callback_handler);
    bt_cm_profile_service_register(BT_CM_PROFILE_SERVICE_HSP, (bt_cm_profile_service_handle_callback_t)bt_sink_srv_hsp_cm_callback_handler);
}
#endif


bt_sink_srv_call_context_t *bt_sink_srv_call_get_context(void)
{
    return &g_sink_srv_call_context;
}

bt_status_t bt_sink_srv_call_action_handler(bt_sink_srv_action_t action, void *parameters)
{
    bt_sink_srv_report_id("[CALL]Action:0x%x", 1, action);
    switch (action) {
        case BT_SINK_SRV_ACTION_PROFILE_INIT: {
            bt_sink_srv_call_psd_init();
            bt_sink_srv_hf_action_handler(action, parameters);
            bt_sink_srv_hsp_action_handler(action, parameters);
            bt_sink_srv_aws_mce_call_action_handler(action, parameters);
        }
        break;

        case BT_SINK_SRV_ACTION_PROFILE_DEINIT: {
            bt_sink_srv_call_psd_deinit();
            bt_sink_srv_hf_action_handler(action, parameters);
            bt_sink_srv_hsp_action_handler(action, parameters);
            bt_sink_srv_aws_mce_call_action_handler(action, parameters);
        }
        break;

        case BT_SINK_SRV_ACTION_PROFILE_CONNECT:
        case BT_SINK_SRV_ACTION_PROFILE_DISCONNECT: {
            bt_sink_srv_mutex_lock();
            bt_sink_srv_hf_action_handler(action, parameters);
            bt_sink_srv_hsp_action_handler(action, parameters);
            bt_sink_srv_aws_mce_call_action_handler(action, parameters);
            bt_sink_srv_mutex_unlock();
        }
        break;

        default: {
            if (BT_SINK_MODULE_HFP_ACTION == (action & 0xFFFFF000)) {
#ifdef MTK_BT_CM_SUPPORT
                if (bt_cm_power_get_state() == BT_CM_POWER_STATE_ON) {
#else
                if (bt_sink_srv_get_state() != BT_SINK_SRV_STATE_NONE) {
#endif
                    bt_sink_srv_mutex_lock();
                    bt_sink_srv_hf_action_handler(action, parameters);
                    bt_sink_srv_hsp_action_handler(action, parameters);
                    bt_sink_srv_aws_mce_call_action_handler(action, parameters);
                    bt_sink_srv_mutex_unlock();
                } else {
                    bt_sink_srv_report_id("[CALL]BT is not powered on.", 0);
                }
            } else {
                bt_sink_srv_report_id("[CALL]Not call action.", 0);
            }
        }
        break;
}
return BT_STATUS_SUCCESS;
}

bt_status_t bt_sink_srv_call_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{

    bt_status_t result = BT_STATUS_SUCCESS;
    uint32_t moduel = msg & 0xFF000000;

    switch (moduel) {
        case BT_MODULE_HFP:
            result = bt_sink_srv_hf_common_callback(msg, status, buffer);
            break;

        case BT_MODULE_HSP:
            result = bt_sink_srv_hsp_common_callback(msg, status, buffer);
            break;

        default:
            break;
    }
    return result;
}

bt_status_t bt_sink_srv_call_get_device_state(bt_sink_srv_device_state_t *device_state)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    switch (role) {
        case BT_AWS_MCE_ROLE_NONE:
        case BT_AWS_MCE_ROLE_AGENT: {
            bt_sink_srv_hf_context_t *hf_context = bt_sink_srv_hf_get_context_by_address(&device_state->address);
            bt_sink_srv_hsp_context_t *hsp_context = bt_sink_srv_hsp_get_context_by_address(&device_state->address);

            if ((hf_context != NULL) && (bt_sink_srv_hf_check_is_connected_by_context(hf_context))) {
                device_state->call_state = hf_context->link.call_state;
                device_state->sco_state = (hf_context->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) ?
                                          BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED : BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            } else if ((hsp_context != NULL) && (bt_sink_srv_hsp_check_is_connected_by_context(hsp_context))) {
                device_state->call_state = BT_SINK_SRV_STATE_NONE; /* HSP has no call state. */
                device_state->sco_state = hsp_context->sco_state;
            } else {
                device_state->call_state = BT_SINK_SRV_STATE_NONE;
                device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            }

            break;
        }

        case BT_AWS_MCE_ROLE_PARTNER: {
#ifdef MTK_AWS_MCE_ENABLE
            bt_sink_srv_aws_mce_call_context_t *aws_context
                = bt_sink_srv_aws_mce_call_get_context_by_address((const bt_bd_addr_t *)&device_state->address);

            if ((aws_context != NULL) && (aws_context->aws_handle) &&
                (aws_context->device != NULL) && (bt_sink_srv_call_psd_is_ready(aws_context->device))) {
                device_state->call_state = bt_sink_srv_aws_mce_call_transfer_aws_call_state(aws_context->call_info.call_state);
                device_state->sco_state = (aws_context->call_info.sco_state != BT_SINK_SRV_AWS_MCE_SCO_STATE_DISCONNECTED) ?
                                          BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED : BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            } else {
                device_state->call_state = BT_SINK_SRV_STATE_NONE;
                device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            }
#endif
            break;
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_sink_srv_call_set_mute(bt_sink_srv_mute_t type, bool mute)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
        if (BT_STATUS_SUCCESS == bt_sink_srv_hf_set_mute(type, mute)) {
            status = BT_STATUS_SUCCESS;
        }
#if defined(MTK_BT_HSP_ENABLE)
        if (BT_STATUS_SUCCESS == bt_sink_srv_hsp_set_mute(type, mute)) {
            status = BT_STATUS_SUCCESS;
        }
#endif
    } else {
#if defined(MTK_AWS_MCE_ENABLE)
        if (BT_STATUS_SUCCESS == bt_sink_srv_aws_mce_call_set_mute(type, mute)) {
            status = BT_STATUS_SUCCESS;
        }
#endif
    }

    return status;
}

bt_status_t bt_sink_srv_state_manager_call_callback(bt_sink_srv_state_manager_event_t event, bt_bd_addr_t *address, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    switch (event) {
        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE: {
            bt_sink_srv_device_state_t *device_state = (bt_sink_srv_device_state_t *)parameter;
            if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
                bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_context_by_address(address);
                if (NULL != context) {
                    status = BT_STATUS_SUCCESS;
                    device_state->call_state = context->link.call_state;
                    if (context->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) {
                        device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED;
                    } else {
                        device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
                    }
                }
            } else {
            #ifdef MTK_AWS_MCE_ENABLE
                bt_sink_srv_aws_mce_call_context_t *context = bt_sink_srv_aws_mce_call_get_context_by_address(address);
                if (NULL != context) {
                    status = BT_STATUS_SUCCESS;
                    device_state->call_state = bt_sink_srv_aws_mce_call_transfer_aws_call_state(context->call_info.call_state);
                    if (NULL != context->device && BT_HFP_CODEC_TYPE_NONE != bt_sink_srv_call_psd_get_codec_type(context->device)) {
                        device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED;
                    } else {
                        device_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
                    }
                }
            #endif
            }
            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE: {
            bt_sink_srv_call_pseudo_dev_t *device = NULL;
            audio_src_srv_handle_t **pseudo_device = (audio_src_srv_handle_t **)parameter;
            if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
                bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_context_by_address(address);
                if (NULL != context) {
                    device = context->device;
                }
            } else {
            #ifdef MTK_AWS_MCE_ENABLE
                bt_sink_srv_aws_mce_call_context_t *context = bt_sink_srv_aws_mce_call_get_context_by_address(address);
                if (NULL != context) {
                    device = context->device;
                }
            #endif
            }
            if (NULL != device) {
                status = BT_STATUS_SUCCESS;
                *pseudo_device = device->audio_src;
            }
            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT: {
            bool *support_inband = (bool *)parameter;
            if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
                bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_context_by_address(address);
                if ((NULL != context) && (context->link.ag_featues & BT_HFP_AG_FEATURE_IN_BAND_RING)) {
                    status = BT_STATUS_SUCCESS;
                    *support_inband = true;
                }
            }
            break;
        }

        default: {
            break;
        }
    }

    return status;
}

bt_sink_srv_am_id_t bt_sink_srv_call_get_playing_audio_id(void)
{
    bt_sink_srv_am_id_t audio_id = AUD_ID_INVALID;
    bt_sink_srv_call_pseudo_dev_t *device = NULL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_AGENT) {
        bt_sink_srv_hf_context_t *context
            = bt_sink_srv_hf_get_context_by_flag(BT_SINK_SRV_HF_FLAG_SCO_ACTIVE);
        if (context != NULL) {
            device = context->device;
        }
    } else {
#ifdef __MTK_AWS_MCE_ENABLE__
        bt_sink_srv_aws_mce_call_context_t *context
            = bt_sink_srv_aws_mce_call_get_context_by_sco_state(BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED);
        if (context != NULL) {
            device = context->device;
        }
#endif
    }

    if (device != NULL) {
        audio_id = device->audio_id;
    }

    bt_sink_srv_report_id("[CALL]Playing audio id: %d", 1, audio_id);
    return audio_id;
}

#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
extern uint8_t bt_avm_esco_stream_control(bool value);
bt_status_t bt_sink_srv_call_audio_switch_handle(bool value, void *playing_dev, void *paly_idle_dev)
{
    uint8_t avm_status = 0;
    bt_sink_srv_call_context_t *call_context = bt_sink_srv_call_get_context();

    if(call_context == NULL) {
        bt_sink_srv_report_id("[CALL][SWITCH] call context is null", 0);
        return BT_STATUS_FAIL;
    }

    if(value) {
        if (!BT_SINK_SRV_CALL_IS_FLAG_EXIST(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH)) {
            bt_sink_srv_report_id("[CALL][SWITCH] there is not set audio switch", 0);
            return BT_STATUS_FAIL;
        }
        BT_SINK_SRV_CALL_REMOVE_FLAG(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH);
        
        if (paly_idle_dev != NULL) {
            bt_sink_srv_report_id("[CALL][SWITCH] hfp_context is palying idle", 0);
            bt_sink_srv_call_psd_state_event_notify(paly_idle_dev, BT_SINK_SRV_CALL_EVENT_HF_SWITCH_START, NULL);
            avm_status = bt_avm_esco_stream_control(value);
            bt_sink_srv_report_id("[CALL][SWITCH] avm esco stream control status = %d", 1,avm_status);
        }
    } else {
        if (BT_SINK_SRV_CALL_IS_FLAG_EXIST(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH)) {
            bt_sink_srv_report_id("[CALL][SWITCH] already stop", 0);
            return BT_STATUS_FAIL;
        }
        BT_SINK_SRV_CALL_SET_FLAG(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH);

        if (playing_dev != NULL) {
            bt_sink_srv_report_id("[CALL][SWITCH] there is palying device = %02x", 1, playing_dev);
            /* Codec stop,sub->state:palying_idle. */
            bt_sink_srv_call_psd_state_event_notify(playing_dev, BT_SINK_SRV_CALL_EVENT_HF_SWITCH_STOP, NULL);
            avm_status = bt_avm_esco_stream_control(value);
            bt_sink_srv_report_id("[CALL][SWITCH] avm esco stream control status = %d", 1,avm_status);
        } else {
            bt_sink_srv_report_id("[CALL][SWITCH] there is no palying device", 0);
        }
    }

    return BT_STATUS_SUCCESS;
}
#endif