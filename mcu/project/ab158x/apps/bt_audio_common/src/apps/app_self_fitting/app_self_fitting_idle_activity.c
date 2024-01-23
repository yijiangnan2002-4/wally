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

#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2

#include "app_self_fitting_idle_activity.h"
#include "app_self_fitting_nvkey_struct.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#include "app_advance_passthrough.h"

#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bis.h"
#endif

#include "bt_system.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#endif

#ifdef HAL_AUDIO_MODULE_ENABLED
#include "hal_audio_message_struct.h"
#endif

#ifdef MTK_NVDM_ENABLE
#include "nvkey_id_list.h"
#include "nvkey.h"
#endif

#include "psap_api.h"


#define LOG_TAG "[APP_SELF_FITTING]"

typedef struct {
    uint8_t     recipient;
    uint16_t    geq_band_count;
    uint32_t    geq_params[0];
} PACKED app_self_fitting_qeq_recipient_t;

typedef struct {
    uint16_t                            recipient_count;
    uint16_t                            geq_lib_version;
    app_self_fitting_qeq_recipient_t     recipient[0];
} PACKED app_self_fitting_geq_t;

static app_self_fitting_context_t s_app_self_fitting_context;


static void app_self_fitting_get_config(void)
{
    audio_channel_t channel                   = ami_get_audio_channel();
    bt_aws_mce_role_t role                    = bt_connection_manager_device_local_info_get_aws_role();
    int16_t  temp_volume = 0;

    advanced_passthrough_runtime_get_setting(&s_app_self_fitting_context.ah_transparency_mode,
                                             &temp_volume,
                                             &s_app_self_fitting_context.ah_noise_local,
                                             &s_app_self_fitting_context.ah_conversation_boost_local,
                                             &s_app_self_fitting_context.ah_tone_local,
                                             NULL, NULL);
    if (channel == AUDIO_CHANNEL_L) {
        s_app_self_fitting_context.ah_volume_left  = temp_volume;
    } else if (channel == AUDIO_CHANNEL_R) {
        s_app_self_fitting_context.ah_volume_right = temp_volume;
    }

    APPS_LOG_MSGID_I(LOG_TAG" get_config: role=0x%x, channel=%d, mode=%d, volume=%d, noise=%d, conv_boost=%d,tone=%d",
                     7, role, channel, s_app_self_fitting_context.ah_transparency_mode, temp_volume,
                     s_app_self_fitting_context.ah_noise_local, s_app_self_fitting_context.ah_conversation_boost_local,
                     s_app_self_fitting_context.ah_tone_local);
}

static bool app_self_fitting_idle_proc_ui_shell_group(ui_shell_activity_t *self,
                                                      uint32_t event_id,
                                                      void *extra_data,
                                                      size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            self->local_context = &s_app_self_fitting_context;
            app_self_fitting_get_config();
            break;
        }
        default:
            break;
    }
    return ret;
}


#ifdef MTK_RACE_CMD_ENABLE
static bool app_self_fitting_idle_proc_race_set_config(ui_shell_activity_t *self,
                                                       uint32_t event_id,
                                                       void *extra_data,
                                                       size_t data_len)
{
    if (extra_data == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" race_set_config: extra_data is NULL", 0);
        return true;
    }
    app_self_fitting_context_t *local_context = (app_self_fitting_context_t *)(self->local_context);
    self_fitting_config_cmd_t *param          = (self_fitting_config_cmd_t *)extra_data;
    audio_channel_t channel                   = ami_get_audio_channel();
    bt_aws_mce_role_t role                    = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" race_set_config: role=0x%x, config_type=%d, length=%d, channel=%d",
                     4, role, param->config_type, param->data_lenth, channel);


#if 0//def MTK_AWS_MCE_ENABLE
    if (role == BT_AWS_MCE_ROLE_AGENT && BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
        APPS_LOG_MSGID_W(LOG_TAG" race_set_config:  Agent not AWS Connected", 0);
        race_dsprealtime_self_fitting_status_response(true, SELF_FITTING_CONFIG_AWS_NOT_CONNTECT, param->config_type, 0, NULL);
        return true;
    }
#endif

    audio_psap_status_t          set_status        = AUDIO_PSAP_STATUS_NONE;
    self_fitting_config_status   set_config_status = SELF_FITTING_CONFIG_SUCCESS;

    switch (param->config_type) {
        case SELF_FITTING_AH_MODE_ENABLE: {
            local_context->ah_transparency_mode = param->data[0];
            bool  advance_pt_enable            = app_advance_passthrough_is_enable();
            bool  switch_status                 = false;
            if (local_context->ah_transparency_mode == 1) {
                if (!advance_pt_enable) {
                    switch_status = app_advance_passthrough_switch();
                    if (switch_status) {
                        set_status = advanced_passthrough_runtime_custom_mode_enable(local_context->ah_transparency_mode);
                    } else {
                        set_status = AUDIO_PSAP_STATUS_FAIL;
                    }
                } else {
                    set_status = advanced_passthrough_runtime_custom_mode_enable(local_context->ah_transparency_mode);
                }

            } else {
                set_status = advanced_passthrough_runtime_custom_mode_enable(local_context->ah_transparency_mode);
                if (advance_pt_enable) {
                    switch_status = app_advance_passthrough_switch();
                    if (!switch_status) {
                        set_status = AUDIO_PSAP_STATUS_FAIL;
                    }
                }
            }
            APPS_LOG_MSGID_I(LOG_TAG" race_set_config_mode: mode=%d, advance_pt_enable=%d, set_status=%d, switch_success=%d",
                             4, local_context->ah_transparency_mode, advance_pt_enable, set_status, switch_status);
            break;
        }
        case SELF_FITTING_AH_MODE_VOLUME: {
            local_context->ah_volume_left = ((int16_t)param->data[1]) << 8 | (int16_t)param->data[0];
            local_context->ah_volume_right  = ((int16_t)param->data[3]) << 8 | (int16_t)param->data[2];
            if (channel == AUDIO_CHANNEL_L) {
                set_status = advanced_passthrough_runtime_set_volume(local_context->ah_volume_left);
            } else if (channel == AUDIO_CHANNEL_R) {
                set_status = advanced_passthrough_runtime_set_volume(local_context->ah_volume_right);
            }

            break;
        }
        case SELF_FITTING_AH_MODE_TONE: {
            local_context->ah_tone_local = ((int16_t)param->data[1]) << 8 | (int16_t)param->data[0];
            set_status = advanced_passthrough_runtime_set_tone(local_context->ah_tone_local);
            break;
        }
        case SELF_FITTING_AH_MODE_NOISE: {
            local_context->ah_noise_local = ((int16_t)param->data[1]) << 8 | (int16_t)param->data[0];
            set_status = advanced_passthrough_runtime_set_NR(local_context->ah_noise_local);
            break;
        }
        case SELF_FITTING_AH_MODE_CONVERSATION: {
            local_context->ah_conversation_boost_local = param->data[0];
            set_status = advanced_passthrough_runtime_conv_boost_enable(local_context->ah_conversation_boost_local);
            break;
        }
        case SELF_FITTING_AH_MODE_REALTIME_GEQ: {
            app_self_fitting_geq_t *self_fitting_peq = (app_self_fitting_geq_t *)pvPortMalloc(param->data_lenth);
            if (self_fitting_peq != NULL) {
                memset(self_fitting_peq, 0, param->data_lenth);
                memcpy(self_fitting_peq, param->data, param->data_lenth);

                if (self_fitting_peq->recipient_count == 1) {
                    APPS_LOG_MSGID_I(LOG_TAG" race_set_config_GEQ: recipient_count=%d, recipient=%d, geq_band_count=%d",
                                     3, self_fitting_peq->recipient_count, self_fitting_peq->recipient[0].recipient,
                                     self_fitting_peq->recipient[0].geq_band_count);
                    if (channel == AUDIO_CHANNEL_L && self_fitting_peq->recipient[0].recipient == 0) {
                        set_status = advanced_passthrough_runtime_set_GEQ(self_fitting_peq->recipient[0].geq_band_count, self_fitting_peq->recipient[0].geq_params);
                    } else if (channel == AUDIO_CHANNEL_R && self_fitting_peq->recipient[0].recipient == 1) {
                        set_status = advanced_passthrough_runtime_set_GEQ(self_fitting_peq->recipient[0].geq_band_count, self_fitting_peq->recipient[0].geq_params);
                    }
                } else if (self_fitting_peq->recipient_count == 2) {
                    uint8_t recipient1_offset = sizeof(app_self_fitting_qeq_recipient_t) + (self_fitting_peq->recipient[0].geq_band_count * sizeof(uint32_t));
                    app_self_fitting_qeq_recipient_t *recipient1_ptr = (app_self_fitting_qeq_recipient_t *)(((uint8_t *)self_fitting_peq->recipient) + recipient1_offset);
                    APPS_LOG_MSGID_I(LOG_TAG" race_set_config_GEQ: recipient_count=%d, recipient1_location=%d, recipient0=%d, geq_band_count0=%d, recipient1=%d, geq_band_count1=%d",
                                     6, self_fitting_peq->recipient_count, recipient1_offset,
                                     self_fitting_peq->recipient[0].recipient, self_fitting_peq->recipient[0].geq_band_count,
                                     recipient1_ptr->recipient, recipient1_ptr->geq_band_count);
                    if (channel == AUDIO_CHANNEL_L) {
                        if (self_fitting_peq->recipient[0].recipient == 0) {
                            set_status = advanced_passthrough_runtime_set_GEQ(self_fitting_peq->recipient[0].geq_band_count, self_fitting_peq->recipient[0].geq_params);
                        } else if (recipient1_ptr->recipient == 0) {
                            set_status = advanced_passthrough_runtime_set_GEQ(recipient1_ptr->geq_band_count, recipient1_ptr->geq_params);
                        }
                    } else if (channel == AUDIO_CHANNEL_R) {
                        if (self_fitting_peq->recipient[0].recipient == 1) {
                            set_status = advanced_passthrough_runtime_set_GEQ(self_fitting_peq->recipient[0].geq_band_count, self_fitting_peq->recipient[0].geq_params);
                        } else if (recipient1_ptr->recipient == 1) {
                            set_status = advanced_passthrough_runtime_set_GEQ(recipient1_ptr->geq_band_count, recipient1_ptr->geq_params);
                        }
                    }
                }

                vPortFree(self_fitting_peq);
            } else {
                APPS_LOG_MSGID_W(LOG_TAG" race_set_config: GEQ malloc memory fail!", 0);
            }

            break;
        }
        default:
            break;
    }

    if (set_status == AUDIO_PSAP_STATUS_FAIL) {
        set_config_status = SELF_FITTING_CONFIG_UNKNOWN_ERROR;
    }

#ifdef MTK_AWS_MCE_ENABLE
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        bt_status_t sync_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                                 APPS_EVENTS_INTERACTION_SYNC_SELF_FITTING_CONFIG,
                                                                 (void *)param, data_len);
        if ((sync_status != BT_STATUS_SUCCESS) && (set_status == AUDIO_PSAP_STATUS_NONE)) {
            set_config_status = SELF_FITTING_CONFIG_UNKNOWN_ERROR;
        }

        APPS_LOG_MSGID_I(LOG_TAG" race_set_config [%02x] sync to partner: status=0x%x", 2, role, sync_status);
        race_dsprealtime_self_fitting_status_response(true, set_config_status, param->config_type, 0, NULL);
    }
#else
    race_dsprealtime_self_fitting_status_response(true, set_config_status, param->config_type, 0, NULL);
#endif

    return true;
}

static bool app_self_fitting_idle_proc_race_get_config(ui_shell_activity_t *self,
                                                       uint32_t event_id,
                                                       void *extra_data,
                                                       size_t data_len)
{
    if (extra_data == NULL) {
        APPS_LOG_MSGID_I(LOG_TAG" race_get_config: extra_data is NULL", 0);
        return true;
    }
    app_self_fitting_context_t *local_context = (app_self_fitting_context_t *)(self->local_context);
    self_fitting_config_cmd_t *param          = (self_fitting_config_cmd_t *)extra_data;
    APPS_LOG_MSGID_I(LOG_TAG" race_get_config:  config_type=%d", 1, param->config_type);

    self_fitting_config_rsp_t *rsp = NULL;


    switch (param->config_type) {
        case SELF_FITTING_AH_MODE_ENABLE: {
            rsp = (self_fitting_config_rsp_t *)pvPortMalloc(sizeof(self_fitting_config_rsp_t) + 1);
            if (rsp != NULL) {
                rsp->status       = SELF_FITTING_CONFIG_SUCCESS;
                rsp->config_type  = param->config_type;
                rsp->data_lenth   = 1;
                rsp->data[0]      = local_context->ah_transparency_mode;
            }
            break;
        }
        case SELF_FITTING_AH_MODE_VOLUME: {
            rsp = (self_fitting_config_rsp_t *)pvPortMalloc(sizeof(self_fitting_config_rsp_t) + 4);
            if (rsp != NULL) {
                rsp->status       = SELF_FITTING_CONFIG_SUCCESS;
                rsp->config_type  = param->config_type;
                rsp->data_lenth   = 4;
                rsp->data[0]      = (int8_t)local_context->ah_volume_left;
                rsp->data[1]      = (int8_t)(local_context->ah_volume_left >> 8);
                rsp->data[2]      = (int8_t)local_context->ah_volume_right;
                rsp->data[3]      = (int8_t)(local_context->ah_volume_right >> 8);
            }
            break;
        }
        case SELF_FITTING_AH_MODE_TONE: {
            rsp = (self_fitting_config_rsp_t *)pvPortMalloc(sizeof(self_fitting_config_rsp_t) + 2);
            if (rsp != NULL) {
                rsp->status       = SELF_FITTING_CONFIG_SUCCESS;
                rsp->config_type  = param->config_type;
                rsp->data_lenth   = 2;
                rsp->data[0]      = (int8_t)local_context->ah_tone_local;
                rsp->data[1]      = (int8_t)(local_context->ah_tone_local >> 8);
            }
            break;
        }
        case SELF_FITTING_AH_MODE_NOISE: {
            rsp = (self_fitting_config_rsp_t *)pvPortMalloc(sizeof(self_fitting_config_rsp_t) + 2);
            if (rsp != NULL) {
                rsp->status       = SELF_FITTING_CONFIG_SUCCESS;
                rsp->config_type  = param->config_type;
                rsp->data_lenth   = 2;
                rsp->data[0]      = (int8_t)local_context->ah_noise_local;
                rsp->data[1]      = (int8_t)(local_context->ah_noise_local >> 8);
            }
            break;
        }
        case SELF_FITTING_AH_MODE_CONVERSATION: {
            rsp = (self_fitting_config_rsp_t *)pvPortMalloc(sizeof(self_fitting_config_rsp_t) + 1);
            if (rsp != NULL) {
                rsp->status       = SELF_FITTING_CONFIG_SUCCESS;
                rsp->config_type  = param->config_type;
                rsp->data_lenth   = 1;
                rsp->data[0]      = local_context->ah_conversation_boost_local;
            }
            break;
        }
        default:
            break;
    }

    if (rsp != NULL) {
        race_dsprealtime_self_fitting_status_response(false, rsp->status, rsp->config_type, rsp->data_lenth, rsp->data);
        vPortFree(rsp);
    }

    return true;
}

static bool app_self_fitting_idle_proc_race_group(ui_shell_activity_t *self,
                                                  uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    bool anc_enable = false;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    APPS_LOG_MSGID_I(LOG_TAG" [ANC_GAIN_RACE event] [%02X] event_id=%d, data_len=%d",
                     3, role, event_id, data_len);

    switch (event_id) {
        case RACE_EVENT_TYPE_SET_SELF_FITTING_CONFIG: {
            app_self_fitting_idle_proc_race_set_config(self, event_id, extra_data, data_len);
            break;
        }
        case RACE_EVENT_TYPE_GET_SELF_FITTING_CONFIG: {
            app_self_fitting_idle_proc_race_get_config(self, event_id, extra_data, data_len);
            break;
        }
    }
    return true;
}

#endif

#ifdef MTK_AWS_MCE_ENABLE
static bool app_self_fitting_proc_aws_data_group(ui_shell_activity_t *self,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group = 0;
        uint32_t aws_event_id = 0;
        void    *extra = NULL;
        uint32_t extra_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &aws_event_id, (void *)&extra, &extra_len);
        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
            && aws_event_id == APPS_EVENTS_INTERACTION_SYNC_SELF_FITTING_CONFIG) {
            if (extra == NULL) {
                return ret;
            }
            self_fitting_config_cmd_t *aws_parm = (self_fitting_config_cmd_t *)extra;
            APPS_LOG_MSGID_I(LOG_TAG" type=%d, extra_len=%d, data_len=%d",
                             3, aws_parm->config_type, extra_len,  aws_parm->data_lenth);
#ifdef MTK_RACE_CMD_ENABLE
            app_self_fitting_idle_proc_race_set_config(self, aws_parm->config_type, extra, extra_len);
#endif
            ret = TRUE;
        }
    }
    return ret;
}
#endif




bool app_self_fitting_idle_activity_proc(ui_shell_activity_t *self,
                                         uint32_t event_group,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_self_fitting_idle_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_RACE_CMD_ENABLE
        case EVENT_GROUP_UI_SHELL_SELF_FITTING: {
            ret = app_self_fitting_idle_proc_race_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            ret = app_self_fitting_proc_aws_data_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}

#endif
