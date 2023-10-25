/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#include "bt_sink_srv_state_manager.h"
#include "bt_sink_srv_state_manager_internal.h"

static void bt_sink_srv_state_manager_get_device_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_device_state_t *device_state);

static bt_status_t default_bt_sink_srv_get_config(
    bt_sink_srv_get_config_t type,
    bt_sink_srv_get_config_param_t *param,
    bt_sink_srv_config_t *config);

#if _MSC_VER >= 1500
#pragma comment(linker, "/altername:_bt_sink_srv_get_config=_default_bt_sink_srv_get_config")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma weak bt_sink_srv_get_config = default_bt_sink_srv_get_config
#else
#error "Unsupported Platform"
#endif

bt_status_t bt_sink_srv_state_manager_get_config(
    bt_sink_srv_get_config_t type,
    bt_sink_srv_state_manager_get_config_t *param,
    bt_sink_srv_config_t *config)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    switch (type) {
        case BT_SINK_SRV_GET_REJECT_CONFIG: {
            bt_sink_srv_get_reject_config_param_t config_param = {0};
            bt_sink_srv_state_manager_device_t *current_device = NULL;
            bt_sink_srv_state_manager_device_t *reject_device = NULL;
            const audio_src_srv_handle_t *current_psedev = audio_src_srv_get_runing_pseudo_device();

            /* 1. Find device. */
            current_device = bt_sink_srv_state_manager_get_device_by_psedev(
                context,
                (audio_src_srv_handle_t *)current_psedev);

            reject_device = bt_sink_srv_state_manager_get_device(
                context,
                param->get_reject_config.type,
                &param->get_reject_config.address);

            bt_sink_srv_report_id("[Sink][StaMgr]get reject config, current_device: 0x%x reject_device: 0x%x",
                                  2, current_device, reject_device);

            /* 2. Get config from Application. */
            if (NULL != current_device) {
                bt_sink_srv_state_manager_get_device_state(
                    context,
                    current_device,
                    &config_param.current_device_state);
            } else if (NULL != current_psedev) {
                if (BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(current_psedev->type) ||
                    AUDIO_SRC_SRV_PRIORITY_HIGH > current_psedev->priority) {
                    /* For non Sink Service device, assume it streaming if priority lower than high. */
                    config_param.current_device_state.music_state = BT_SINK_SRV_STATE_STREAMING;
                }
            }

            if (NULL != reject_device) {
                bt_sink_srv_state_manager_get_device_state(
                    context,
                    reject_device,
                    &config_param.reject_device_state);
            } else {
                config_param.reject_device_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            }

            status = bt_sink_srv_get_config(type, (bt_sink_srv_get_config_param_t *)&config_param, config);

            bt_sink_srv_report_id("[Sink][StaMgr]get reject config, operation: 0x%x will_resume: %d",
                                  2, config->reject_config.reject_operation, config->reject_config.will_reject_resume);

            break;
        }

        case BT_SINK_SRV_GET_SUSPEND_CONFIG:
        {
            bt_sink_srv_get_suspend_config_param_t config_param = {0};
            bt_sink_srv_state_manager_device_t *suspend_device = NULL;
            bt_sink_srv_state_manager_device_t *coming_device = NULL;

            /* 1. Find device. */
            suspend_device = bt_sink_srv_state_manager_get_device(
                context,
                param->get_suspend_config.type,
                &param->get_suspend_config.address);

            coming_device = bt_sink_srv_state_manager_get_device_by_psedev(
                context,
                param->get_suspend_config.suspend_handle);

            bt_sink_srv_report_id("[Sink][StaMgr]get suspend config, suspend_device: 0x%x coming_device: 0x%x",
                                  2, suspend_device, coming_device);

            /* 2. Get config from Application. */
            if (NULL != suspend_device) {
                bt_sink_srv_state_manager_get_device_state(
                    context,
                    suspend_device,
                    &config_param.suspend_device_state);
            } else {
                config_param.suspend_device_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            }

            if (NULL != coming_device) {
                bt_sink_srv_state_manager_get_device_state(
                    context,
                    coming_device,
                    &config_param.coming_device_state);
            } else if (NULL != param->get_suspend_config.suspend_handle) {
                if (BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(param->get_suspend_config.suspend_handle->type) ||
                    AUDIO_SRC_SRV_PRIORITY_HIGH > param->get_suspend_config.suspend_handle->priority) {
                    /* For non Sink Service device, assume it streaming if priority lower than high. */
                    config_param.coming_device_state.music_state = BT_SINK_SRV_STATE_STREAMING;
                }
            }

            status = bt_sink_srv_get_config(type, (bt_sink_srv_get_config_param_t *)&config_param, config);

            bt_sink_srv_report_id("[Sink][StaMgr]get suspend config, operation: 0x%x will_resume: %d timeout: %d",
                                  3,
                                  config->suspend_config.suspend_operation,
                                  config->suspend_config.will_suspend_resume,
                                  config->suspend_config.suspend_resume_timeout);

            break;
        }

        case BT_SINK_SRV_GET_RESUME_CONFIG:
        {
            bt_sink_srv_get_resume_config_param_t config_param = {0};
            bt_sink_srv_state_manager_device_t *resume_device = NULL;

            /* 1. Find device. */
            resume_device = bt_sink_srv_state_manager_get_device(
                context,
                param->get_resume_config.type,
                &param->get_resume_config.address);

            bt_sink_srv_report_id("[Sink][StaMgr]get resume config, resume_device: 0x%x",
                                  1, resume_device);

            /* 2. Get config from Application. */
            if (NULL != resume_device) {
                bt_sink_srv_state_manager_get_device_state(
                    context,
                    resume_device,
                    &config_param.resume_device_state);
            } else {
                config_param.resume_device_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            }

            status = bt_sink_srv_get_config(type, (bt_sink_srv_get_config_param_t *)&config_param, config);

            bt_sink_srv_report_id("[Sink][StaMgr]get resume config, operation: 0x%x",
                                  1, config->resume_config.resume_operation);

            break;
        }

        default: {
            break;
        }
    }

    return status;
}

static void bt_sink_srv_state_manager_get_device_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_device_state_t *device_state)
{
    device_state->type = device->type;
    device_state->call_state = device->call_state;
    device_state->sco_state = device->call_audio_state;
    device_state->music_state = device->media_state;
    bt_sink_srv_memcpy(device_state->address, device->address, sizeof(bt_bd_addr_t));
}

static bt_status_t default_bt_sink_srv_get_config(
    bt_sink_srv_get_config_t type,
    bt_sink_srv_get_config_param_t *param,
    bt_sink_srv_config_t *config)
{
    switch (type) {
        case BT_SINK_SRV_GET_REJECT_CONFIG: {
            config->reject_config.reject_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;
            config->reject_config.will_reject_resume = false;

            /* If 2nd media try to interrupt 1st media, 2nd media should resume after 1st paused. */
            /* If 2nd media try to interrupt 1st call,  2nd media should resume after 1st call ended. */
            if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(param->reject_config_param.reject_device_state.music_state)) {
                config->reject_config.will_reject_resume = true;
            }

            /* If 2nd call  try to interrupt 1st call,  transfer call audio to SP. */
            if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(param->reject_config_param.reject_device_state.call_state) ||
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->reject_config_param.reject_device_state.sco_state) {
                config->reject_config.reject_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_TRANSFER_CALL_AUDIO; 
                if (BT_SINK_SRV_DEVICE_LE == param->reject_config_param.reject_device_state.type) {
                    config->reject_config.will_reject_resume = true; /* LE should resume because cannnot transfer call audio. */
                }
            }

            break;
        }

        case BT_SINK_SRV_GET_SUSPEND_CONFIG: {
            config->suspend_config.suspend_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;
            config->suspend_config.will_suspend_resume = false;
            config->suspend_config.suspend_resume_timeout = 0;

            /* If 2nd call try to interrupt 1st media, media should resume after call ended. */
            if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(param->suspend_config_param.suspend_device_state.music_state)) {
                config->suspend_config.suspend_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC;
                config->suspend_config.will_suspend_resume = true;
            }

            break;
        }

        case BT_SINK_SRV_GET_RESUME_CONFIG: {
            config->resume_config.resume_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;

            /* If 2nd call  has been rejected, do nothing while it can be resumed. */
            /* If 1st media has beed suspended, media should play after call ended. */
            if (BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(param->resume_config_param.resume_device_state.music_state)) {
                config->resume_config.resume_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PLAY_MUSIC;
            }

            break;
        }

        default: {
            break;
        }
    }

    return BT_STATUS_SUCCESS;
}

