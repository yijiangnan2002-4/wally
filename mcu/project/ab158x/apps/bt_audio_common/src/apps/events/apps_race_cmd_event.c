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

#ifdef MTK_RACE_CMD_ENABLE

#include "apps_race_cmd_event.h"

#include "apps_config_key_remapper.h"
#include "apps_config_audio_helper.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"
#include "bt_device_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "nvdm.h"
#include "nvdm_id_list.h"
#include "bt_sink_srv_ami.h"
#include "apps_aws_sync_event.h"
#include "app_home_screen_idle_activity.h"
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
#include "app_advance_passthrough.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_ull_idle_activity.h"
#endif
#include "app_bt_state_service.h"
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_music_utils.h"
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef AIR_USB_ENABLE
#include "apps_events_usb_event.h"
#endif

#include "race_cmd.h"
#include "race_cmd_relay_cmd.h"
#include "race_noti.h"
#include "race_bt.h"

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif

#include "apps_control_touch_key_status.h"

#ifdef AIR_USB_HID_ENABLE
#include "usb_custom.h"
#endif

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "app_hear_through_race_cmd_handler.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#include "voice_prompt_api.h"
#include "app_rx_eq.h"
#include "bt_app_common.h"
#include "avm_external.h"

extern void vPortFree(void *);
extern void *pvPortMalloc(size_t);

#define LOG_TAG     "[apps_race_event]"

#define RACE_ID_APPLICATION_BEGIN 0x2C10
#define RACE_ID_APPLICATION_END 0x2CCF

#define RACE_ID_APP_POWER_SAVING_SET_CFG 0x2C15
#define RACE_ID_APP_POWER_SAVING_GET_CFG 0x2C16

#define RACE_ID_APP_IN_EAR_SET_MUSIC_CFG 0x2C17
#define RACE_ID_APP_IN_EAR_GET_MUSIC_CFG 0x2C18

#ifdef MTK_RACE_CMD_ENABLE
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
static void *apps_race_cmd_power_saving(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint16_t enable_flag;
        uint16_t timeout;
        uint16_t silence_detect_enable;
        uint16_t silence_detect_timeout;
    } PACKED power_saving_cfg_set_req;

    typedef struct {
        uint8_t result;
    } PACKED RSP;

    typedef struct {
        uint8_t result;
        uint16_t enable_flag;
        uint16_t timeout;
        uint16_t silence_detect_enable;
        uint16_t silence_detect_timeout;
    } PACKED QUERY_RSP;

    void *pEvt = NULL;

    switch (p_race_package->hdr.id) {
        /* Nnable music control */
        case RACE_ID_APP_POWER_SAVING_SET_CFG: {
            power_saving_cfg_set_req *pcmd = (power_saving_cfg_set_req *)p_race_package;
            pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                         RACE_TYPE_RESPONSE,
                                         RACE_ID_APP_POWER_SAVING_SET_CFG,
                                         sizeof(RSP),
                                         channel_id);
            if (pEvt) {
                ((RSP *)pEvt)->result = RACE_ERRCODE_FAIL;
                if (pcmd == NULL) {
                    return pEvt;
                }

                /* sync msg to partner. */
#ifdef MTK_AWS_MCE_ENABLE
                RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    ret = bt_send_aws_mce_race_cmd_data(p_race_package, pcmd->Hdr.length + (uint32_t) & (((RACE_COMMON_HDR_STRU *)0)->id), channel_id, RACE_CMD_RELAY_FROM_AGENT, 0);
                    if (RACE_ERRCODE_SUCCESS != ret) {
                        APPS_LOG_MSGID_I(LOG_TAG"set power saving cfg, sync to partner failed,", 0);
                        ((RSP *)pEvt)->result = ret;
                        return pEvt;
                    }
                }
#endif
                uint16_t data_len = pcmd->Hdr.length - 2;
                app_power_saving_cfg cfg;
                memset(&cfg, 0, sizeof(cfg));
                if (data_len >= 2 * sizeof(uint16_t)) {
                    cfg.power_saving_enable = pcmd->enable_flag;
                    cfg.timeout = pcmd->timeout;
                } else {
                    return pEvt;
                }
                if (data_len >= 4 * sizeof(uint16_t)) {
                    /* Old version not support silence detect. so only 2 type.*/
                    cfg.silence_detect_enable = pcmd->silence_detect_enable;
                    cfg.silence_detect_timeout = pcmd->silence_detect_timeout;
                }
                /* Notify app the cfg updated. */
                APPS_LOG_MSGID_I(LOG_TAG"set power saving cfg, en: %d, t:%d, silence_en: %d, t: %d, len:%d", 5, cfg.power_saving_enable, cfg.timeout, cfg.silence_detect_enable, cfg.silence_detect_timeout, length);
                if ((pcmd->enable_flag && pcmd->timeout < 1) || (cfg.silence_detect_enable && cfg.silence_detect_timeout < 1)) {
                    return pEvt;
                }
                int32_t r = app_power_save_utils_set_cfg(&cfg);
                if (r == 0) {
                    app_power_save_utils_cfg_updated_notify();
                    ((RSP *)pEvt)->result = RACE_ERRCODE_SUCCESS;
                } else {
                    return pEvt;
                }
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
            }
            break;
        }
        /* Get the music control state */
        case RACE_ID_APP_POWER_SAVING_GET_CFG: {
            pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                         RACE_TYPE_RESPONSE,
                                         RACE_ID_APP_POWER_SAVING_GET_CFG,
                                         sizeof(QUERY_RSP),
                                         channel_id);
            if (pEvt) {
                app_power_saving_cfg *cfg = app_power_saving_get_cfg();
                ((QUERY_RSP *)pEvt)->result = RACE_ERRCODE_SUCCESS;
                ((QUERY_RSP *)pEvt)->enable_flag = cfg->power_saving_enable;
                ((QUERY_RSP *)pEvt)->timeout = cfg->timeout;
                ((QUERY_RSP *)pEvt)->silence_detect_enable = cfg->silence_detect_enable;
                ((QUERY_RSP *)pEvt)->silence_detect_timeout = cfg->silence_detect_timeout;
                APPS_LOG_MSGID_I(LOG_TAG"get power saving cfg, en: %d, t:%d", 2, cfg->power_saving_enable, cfg->timeout);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
            }
            break;
        }
    }

    return pEvt;
}
#endif

#ifdef MTK_IN_EAR_FEATURE_ENABLE
static void *apps_race_cmd_in_ear(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t req[1];
    } PACKED in_ear_cfg_set_req;

    typedef struct {
        uint8_t result;
    } PACKED RSP;

    typedef struct {
        uint8_t result;
        uint8_t enable;
    } PACKED QUERY_RSP;

    void *pEvt = NULL;

    switch (p_race_package->hdr.id) {
        /* Nnable music control */
        case RACE_ID_APP_IN_EAR_SET_MUSIC_CFG: {
            in_ear_cfg_set_req *pcmd = (in_ear_cfg_set_req *)p_race_package;
            pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                         RACE_TYPE_RESPONSE,
                                         RACE_ID_APP_IN_EAR_SET_MUSIC_CFG,
                                         sizeof(RSP),
                                         channel_id);
            if (pEvt) {
                ((RSP *)pEvt)->result = RACE_ERRCODE_FAIL;
                /* sync msg to partner. */
#ifdef MTK_AWS_MCE_ENABLE
                RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    ret = bt_send_aws_mce_race_cmd_data(p_race_package, sizeof(RACE_COMMON_HDR_STRU) + 1, channel_id, RACE_CMD_RELAY_FROM_AGENT, 0);
                    if (RACE_ERRCODE_SUCCESS != ret) {
                        APPS_LOG_MSGID_I(LOG_TAG"set music ctrl, sync to partner failed,", 0);
                        ((RSP *)pEvt)->result = ret;
                        return pEvt;
                    }
                }
#endif
                /* Notify app the cfg updated. */
                uint8_t *p_data = (uint8_t *)pvPortMalloc(sizeof(uint8_t));
                if (p_data != NULL) {
                    ui_shell_status_t status = UI_SHELL_STATUS_OK;
                    *p_data = pcmd->req[0];
                    status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                 APPS_EVENTS_INTERACTION_IN_EAR_CFG_UPDATE,
                                                 p_data, sizeof(uint8_t), NULL, 0);
                    if (status != UI_SHELL_STATUS_OK) {
                        vPortFree(p_data);
                        return pEvt;
                    }
                    ((RSP *)pEvt)->result = RACE_ERRCODE_SUCCESS;
                }
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
            }

            break;
        }
        /* Get the music control state */
        case RACE_ID_APP_IN_EAR_GET_MUSIC_CFG: {
            pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                         RACE_TYPE_RESPONSE,
                                         RACE_ID_APP_IN_EAR_GET_MUSIC_CFG,
                                         sizeof(QUERY_RSP),
                                         channel_id);
            if (pEvt) {
                ((QUERY_RSP *)pEvt)->result = RACE_ERRCODE_SUCCESS;
                uint8_t temp_music_in_ear_cfg = app_music_get_in_ear_control_state();
                uint8_t enabled = temp_music_in_ear_cfg ? 1 : 0;
                ((QUERY_RSP *)pEvt)->enable = enabled;
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
            }
            break;
        }
    }

    return pEvt;
}
#endif
#endif

void app_race_send_notify(uint16_t cfg_type, int8_t *data, uint32_t len)
{
    uint8_t channel_id;
    RACE_ERRCODE race_notify_ret;
    uint32_t serial_port;
    serial_port_dev_t dev;
    bt_handle_t handle;

    typedef struct {
        uint16_t config_type;
        uint8_t data[1];
    } PACKED NOTI;

    handle = race_bt_get_ble_conn_hdl();
    APPS_LOG_MSGID_I(LOG_TAG" send notiy, type=0x%04X data=0x%8X len=%d", 3, cfg_type, data, len);

    if (handle != BT_HANDLE_INVALID) {
        dev = SERIAL_PORT_DEV_BT_LE;
    } else {
        dev = SERIAL_PORT_DEV_BT_SPP;
    }

    serial_port = race_get_serial_port_handle(dev);

    channel_id = (uint8_t)race_get_channel_id_by_port_handle(serial_port);
    NOTI *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_COMMAND_WITHOUT_RSP,
                                  (uint16_t)RACE_SET_APP_COMMON_CONFIG,
                                  (uint16_t)(sizeof(uint16_t) + len),
                                  channel_id);
    if (noti == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG"claim packet failed.", 0);
        return;
    }

    noti->config_type = cfg_type;
    memcpy(&noti->data[0], data, len);

    race_notify_ret = race_noti_send(noti, channel_id, false);
    APPS_LOG_MSGID_I(LOG_TAG"notify data resule: %d.", 1, race_notify_ret);
    if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
        RACE_FreePacket((void *)noti);
    }
}

void app_race_notify_mmi_state(uint32_t mmi_state)
{
    /* If connected ULL dongle, need notify to dongle->HID->PC. */
    uint8_t dev[2] = { 0 };
    uint32_t i;
    uint32_t dev_count = 0;
    uint8_t channel_id;
    RACE_ERRCODE race_notify_ret;

    typedef struct {
        uint8_t mmi_state;
    } PACKED NOTI;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
#ifdef MTK_AWS_MCE_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
        || bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_NONE)
#endif
    {
        bt_bd_addr_t addr_list[1];
        uint32_t list_num = 1;
        list_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), addr_list, list_num);
        if (list_num > 0 &&
            BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR) & bt_cm_get_connected_profile_services(addr_list[0])) {
            dev[dev_count] = RACE_SERIAL_PORT_TYPE_SPP;
            dev_count++;
            APPS_LOG_MSGID_I(LOG_TAG"Add SPP in notify mmi state port", 0);
        }
    }
#endif
#if defined(APPS_USB_AUDIO_SUPPORT) && defined(AIR_USB_HID_ENABLE)
    dev[dev_count] = RACE_SERIAL_PORT_TYPE_USB;
    dev_count++;
    APPS_LOG_MSGID_I(LOG_TAG"Add USB in notify mmi state port", 0);
#endif
    for (i = 0; i < dev_count; i++) {
        channel_id = race_get_channel_id_by_port_type(dev[i]);
        NOTI *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_COMMAND_WITHOUT_RSP,
                                      (uint16_t)RACE_GET_MMI_STATE,
                                      (uint16_t)(sizeof(NOTI)),
                                      channel_id);
        if (noti == NULL) {
            APPS_LOG_MSGID_E(LOG_TAG"notify_mmi_state[%d] claim packet failed", 1, i);
            return;
        }
        noti->mmi_state = mmi_state;
        race_notify_ret = race_noti_send(noti, channel_id, false);
        APPS_LOG_MSGID_I(LOG_TAG"notify_mmi_state resule: %d.", 1, race_notify_ret);
        if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)noti);
        }
    }
}

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
uint8_t app_ull_get_mic_vol(void);
uint8_t app_ull_get_mix_ratio(void);
#endif

static void *apps_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    void *pEvt = NULL;
    APPS_LOG_MSGID_I(LOG_TAG"apps_race_cmd_handler, type[0x%X], id[0x%X],len[0x%X],", 3, p_race_package->hdr.type, p_race_package->hdr.id,p_race_package->hdr.length);    
    APPS_LOG_MSGID_I(LOG_TAG"apps_race_cmd_handler, payload[0][0x%X],", 1, p_race_package->payload[0]);
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif

    if (p_race_package->hdr.type == RACE_TYPE_COMMAND ||
        p_race_package->hdr.type == RACE_TYPE_COMMAND_WITHOUT_RSP) {
        switch (p_race_package->hdr.id) {
            case RACE_GET_MMI_STATE: {
                typedef struct {
                    uint8_t mmi_state;
                } PACKED RSP;
                pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                             RACE_TYPE_RESPONSE,
                                             RACE_GET_MMI_STATE,
                                             sizeof(RSP),
                                             channel_id);
                if (pEvt) {
                    ((RSP *)pEvt)->mmi_state = apps_config_key_get_mmi_state();
                } else {
                    APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                }
                break;
            }
            case RACE_PLAY_VOICE_PROMPT: {
                typedef struct {
                    uint8_t status;
                } PACKED RSP;
                typedef struct {
                    RACE_COMMON_HDR_STRU cmdhdr;
                    uint16_t vp_index;
                    uint8_t sync;
                    uint8_t loop;
                } PACKED CMD;
                CMD *cmd = (CMD *)p_race_package;

                if (cmd->cmdhdr.length != 6) {
                    APPS_LOG_MSGID_E(LOG_TAG" PLAY_VP cmd length wrong %d", 1, cmd->cmdhdr.length);
                    break;
                }
                APPS_LOG_MSGID_I(LOG_TAG" PLAY_VP index %d, sync %d, loop %d", 3, cmd->vp_index, cmd->sync, cmd->loop);

                if (cmd->vp_index == 0xFF) {
                    voice_prompt_clear_all();
                } else {
                    voice_prompt_param_t vp = {0};
                    vp.vp_index = (uint32_t)(cmd->vp_index);
                    if (cmd->sync) {
                        vp.control |= VOICE_PROMPT_CONTROL_MASK_SYNC;
                    }
                    if (cmd->loop) {
                        vp.control |= VOICE_PROMPT_CONTROL_MASK_LOOP;
                    }
                    voice_prompt_play(&vp, NULL);
                }

                pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                             RACE_TYPE_RESPONSE,
                                             RACE_PLAY_VOICE_PROMPT,
                                             sizeof(RSP),
                                             channel_id);
                if (pEvt) {
                    ((RSP *)pEvt)->status = 0;
                }  else {
                    APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                }
                break;
            }

            case RACE_SET_LED_PATTERN: {
                typedef struct {
                    uint8_t status;
                } PACKED RSP;
                typedef struct {
                    RACE_COMMON_HDR_STRU cmdhdr;
                    uint32_t data;
                } PACKED CMD;
                CMD *cmd = (CMD *)p_race_package;
                ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_SET_LED,
                                    (void *)(uint32_t)(cmd->data), 0, NULL, 0);
                pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                             RACE_TYPE_RESPONSE,
                                             RACE_SET_LED_PATTERN,
                                             sizeof(RSP),
                                             channel_id);
                if (pEvt) {
                    ((RSP *)pEvt)->status = 0;
                }
                break;
            }
            case RACE_SET_APP_COMMON_CONFIG: {
                typedef struct {
                    uint8_t status;
                    uint16_t config_type;
                } PACKED RSP;
                typedef struct {
                    RACE_COMMON_HDR_STRU cmdhdr;
                    uint16_t config_type;
                    uint8_t config_data[0];
                } PACKED CMD;
                CMD *cmd = (CMD *)p_race_package;
                pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                             RACE_TYPE_RESPONSE,
                                             RACE_SET_APP_COMMON_CONFIG,
                                             sizeof(RSP),
                                             channel_id);
                if (pEvt == NULL) {
                    APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                    break;
                }

                uint16_t data_len = cmd->cmdhdr.length - 4;
                APPS_LOG_MSGID_I(LOG_TAG" race set app common config: handler_id=0x%04X, config_type=0x%04X, data_len=%d",
                                 3, p_race_package->hdr.id, cmd->config_type, data_len);
                switch (cmd->config_type) {
#ifdef AIR_PROMPT_SOUND_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_VP_LANGUAGE: {
                        uint8_t *vp_index = (uint8_t *)(cmd->config_data);
                        APPS_LOG_MSGID_I(LOG_TAG" race set app vp language index %d", 1, *vp_index);
                        if (VP_STATUS_SUCCESS == voice_prompt_set_language(*vp_index, TRUE) && NULL != pEvt) {
                            ((RSP *)pEvt)->status = 0;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_VP_LANGUAGE;
                        }
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE: {

                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
#ifdef MTK_AWS_MCE_ENABLE
                            if ((BT_AWS_MCE_ROLE_AGENT == role) || (BT_AWS_MCE_ROLE_NONE == role)) {
#endif
                                APPS_LOG_MSGID_I(LOG_TAG" set touch key enable: 0x%02X", 1, *(uint8_t *)p_data);
                                uint8_t ret = apps_set_touch_control_enable(*(uint8_t *)p_data, false);
                                if (1 == ret) {
                                    ((RSP *)pEvt)->status = 0;
                                    ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE;
                                } else {
                                    ((RSP *)pEvt)->status = 1;
                                    ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE;
                                }
#ifdef MTK_AWS_MCE_ENABLE
                            }
#endif
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                        }
                        break;
                    }
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
                    case APPS_RACE_CMD_CONFIG_TYPE_POWER_SAVING_CFG: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_POWER_SAVING_CFG;
                            /* sync msg to partner. */
#ifdef MTK_AWS_MCE_ENABLE
                            RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
                            if (role == BT_AWS_MCE_ROLE_AGENT) {
                                ret = bt_send_aws_mce_race_cmd_data(p_race_package, p_race_package->hdr.length + (uint32_t) & (((RACE_COMMON_HDR_STRU *)0)->id), channel_id, RACE_CMD_RELAY_FROM_AGENT, 0);
                                if (RACE_ERRCODE_SUCCESS != ret) {
                                    APPS_LOG_MSGID_I(LOG_TAG"set power saving cfg, sync to partner failed. ", 0);
                                    ((RSP *)pEvt)->status = ret;
                                    break;
                                }
                                APPS_LOG_MSGID_I(LOG_TAG"set power saving cfg, sync to partner success. ", 0);
                            }
#endif
                            /* Notify app the cfg updated. */
                            app_power_saving_cfg cfg;
                            memset(&cfg, 0, sizeof(cfg));
                            if (data_len >= 2 * sizeof(uint16_t)) {
                                cfg.power_saving_enable = *(uint16_t *)p_data;
                                cfg.timeout = *((uint16_t *)p_data + 1);
                            }
                            if (data_len >= 4 * sizeof(uint16_t)) {
                                cfg.silence_detect_enable = *((uint16_t *)p_data + 2);
                                cfg.silence_detect_timeout = *((uint16_t *)p_data + 3);
                            }
                            APPS_LOG_MSGID_I(LOG_TAG" set power saving cfg, en: %d, t:%d, en_silence:%d, t:%d, len:%d",
                                             5, cfg.power_saving_enable, cfg.timeout, cfg.silence_detect_enable, cfg.silence_detect_timeout, length);

                            if ((cfg.power_saving_enable && cfg.timeout < 1) || (cfg.silence_detect_enable && cfg.silence_detect_timeout < 1)) {
                                break;
                            }

                            int32_t r = app_power_save_utils_set_cfg(&cfg);
                            if (r == 0) {
                                APPS_LOG_MSGID_I(LOG_TAG" set power saving cfg success. ", 0);
                                app_power_save_utils_cfg_updated_notify();
                                ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            } else {
                                APPS_LOG_MSGID_I(LOG_TAG" set power saving cfg fail. ", 0);
                                ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                                break;
                            }
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                        }
                        break;
                    }
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
                    /* Enable music in ear control */
                    case APPS_RACE_CMD_CONFIG_TYPE_IN_EAR_MUSIC_CFG: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_IN_EAR_MUSIC_CFG;
#ifdef MTK_AWS_MCE_ENABLE
                            RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
                            if (role == BT_AWS_MCE_ROLE_AGENT) {
                                ret = bt_send_aws_mce_race_cmd_data(p_race_package, sizeof(RACE_COMMON_HDR_STRU) + 3, channel_id, RACE_CMD_RELAY_FROM_AGENT, 0);
                                if (RACE_ERRCODE_SUCCESS != ret) {
                                    APPS_LOG_MSGID_I(LOG_TAG"set music in ear ctrl, sync to partner failed. ", 0);
                                    ((RSP *)pEvt)->status = ret;
                                    break;
                                }
                                APPS_LOG_MSGID_I(LOG_TAG"set music in ear ctrl, sync to partner success. ", 0);
                            }
#endif
                            /* Notify app the cfg updated. */
                            uint8_t *p_noti = (uint8_t *)pvPortMalloc(sizeof(uint8_t));
                            if (p_noti != NULL) {
                                ui_shell_status_t status = UI_SHELL_STATUS_OK;
                                *p_noti = *(uint8_t *)p_data;
                                status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                             APPS_EVENTS_INTERACTION_IN_EAR_CFG_UPDATE,
                                                             p_noti, sizeof(uint8_t), NULL, 0);
                                if (status != UI_SHELL_STATUS_OK) {
                                    APPS_LOG_MSGID_I(LOG_TAG" notify music app the in ear cfg updated fail.", 0);
                                    vPortFree(p_noti);
                                    break;
                                }
                                APPS_LOG_MSGID_I(LOG_TAG" notify music app the in ear cfg updated success.", 0);
                                ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            } else {
                                APPS_LOG_MSGID_I(LOG_TAG" notify music app the in ear cfg updated malloc failed.", 0);
                            }
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
                        }
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_LEVEL: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_LEVEL;
                            int32_t sidetone_value = *(int16_t *)p_data;
                            if (AUD_EXECUTION_SUCCESS == apps_config_audio_helper_set_sidetone_value(sidetone_value)) {
                                ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            }
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_ENABLE: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_ENABLE;
                            uint8_t sidetone_enable = *(int16_t *)p_data;
                            if (AUD_EXECUTION_SUCCESS == apps_config_audio_helper_set_sidetone_enable(sidetone_enable)) {
                                ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            }
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_ADVANCED_PASSTHROUGH: {
                        uint8_t status = RACE_ERRCODE_SUCCESS;
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
                        void *p_data = &(cmd->config_data);
                        uint8_t enable = *(uint8_t *)p_data;
                        bool cur_enable = app_advance_passthrough_is_enable();
                        APPS_LOG_MSGID_I(LOG_TAG" [ADVANCE_PT] switch via RACE, enable=%d->%d", 2, cur_enable, enable);
                        if (cur_enable != enable) {
                            bool success = app_advance_passthrough_switch();
                            if (!success) {
                                status = RACE_ERRCODE_FAIL;
                            }
                        }
#else
                        status = RACE_ERRCODE_NOT_SUPPORT;
#endif //AIR_ADVANCED_PASSTHROUGH_ENABLE or AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
                        if (pEvt) {
                            ((RSP *)pEvt)->status = status;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ADVANCED_PASSTHROUGH;
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_USB_LOGGING_MODE: {
                        void *p_data = &(cmd->config_data);
                        uint8_t enable = *((uint8_t *)p_data);
#ifdef AIR_USB_HID_ENABLE
                        //USB_Set_LoggingMode(enable == 1);
#endif
                        nvkey_status_t nvk_sta = NVKEY_STATUS_OK;
                        if (NVKEY_STATUS_OK != (nvk_sta = nvkey_write_data(NVID_APP_USB_LOGGING_MODE, (const uint8_t *)&enable, sizeof(uint8_t)))) {
                            APPS_LOG_MSGID_I(LOG_TAG" set usb logging model fail, sta=%d, en=%d", 2, nvk_sta, enable);
                            break;
                        }
                        APPS_LOG_MSGID_I(LOG_TAG" set usb logging model=%d", 1, enable);
                        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                                            NULL, 0);
                        break;
                    }
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    case APPS_RACE_CMD_CONFIG_TYPE_ULL_MIX_RATIO: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ULL_MIX_RATIO;
                        }
                        uint8_t *p_noti = (uint8_t *)pvPortMalloc(sizeof(uint8_t));
                        if (p_noti != NULL) {
                            ui_shell_status_t status = UI_SHELL_STATUS_OK;
                            *p_noti = *(uint8_t *)p_data;
                            status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                         APPS_EVENTS_INTERACTION_SET_ULL_MIX_RATIO, p_noti, sizeof(uint8_t), NULL, 0);
                            if (status != UI_SHELL_STATUS_OK) {
                                vPortFree(p_noti);
                                break;
                            }
                            ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_ULL_MIC_VOL: {
                        void *p_data = &(cmd->config_data);
                        if (pEvt) {
                            ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ULL_MIC_VOL;
                        }
                        uint8_t *p_noti = (uint8_t *)pvPortMalloc(sizeof(uint8_t));
                        if (p_noti != NULL) {
                            ui_shell_status_t status = UI_SHELL_STATUS_OK;
                            *p_noti = *(uint8_t *)p_data;
                            status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                         APPS_EVENTS_INTERACTION_SET_ULL_MIC_VOL, p_noti, sizeof(uint8_t), NULL, 0);
                            if (status != UI_SHELL_STATUS_OK) {
                                vPortFree(p_noti);
                                break;
                            }
                            ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                        }
                        break;
                    }
#endif
#ifdef AIR_MULTI_POINT_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_MULTI_POINT_ENABLE: {
                        if (pEvt) {
                            ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_MULTI_POINT_ENABLE;
                            ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
                            if ((role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE)
                                || BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
                                if (pEvt) {
                                    ((RSP *)pEvt)->status = RACE_ERRCODE_FAIL;
                                }
                                break;
                            }
#endif
                        }
                        void *p_data = &(cmd->config_data);
                        uint32_t enable = *(uint8_t *)p_data;
                        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_MULTI_POINT_ENABLE,
                                            (void *)enable, 0, NULL, 0);
                        break;
                    }
#endif
#ifdef AIR_USB_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_USB_MODE: {
                        ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_USB_MODE;
                        uint32_t mode = cmd->config_data[0];
                        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_SET_USB_MODE,
                                            (void *)mode, 0, NULL, 0);
                        ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                        break;
                    }
#endif
#ifdef APPS_USB_AUDIO_SUPPORT
                    case APPS_RACE_CMD_CONFIG_TYPE_USB_AUDIO_EN: {
                        extern void app_usb_audio_enable(bool en);
                        ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_USB_AUDIO_EN;
                        bool en = (bool)cmd->config_data[0];
                        app_usb_audio_enable(en);
                        ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                        break;
                    }
#endif
#if defined(AIR_DCHS_MODE_MASTER_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    case APPS_RACE_CMD_CONFIG_TYPE_ULL2_SAMPLE_SET: {
                        extern void app_le_ull_set_ull_sample_rate(uint8_t type);
                        ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ULL2_SAMPLE_SET;
                        app_le_ull_set_ull_sample_rate(cmd->config_data[0]);
                        ((RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_CALL_RX_EQ_CFG: {
                        ((RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_CALL_RX_EQ_CFG;
                        void *p_data = &(cmd->config_data);
                        if (p_data == NULL
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
                           || (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE)
#endif
                        ) {
                            ((RSP *)pEvt)->status = APP_CALL_RX_EQ_SET_ERROR;
                            break;
                        }

                        app_call_rx_eq_type_t received_eq_type = *(app_call_rx_eq_type_t *)p_data;
                        APPS_LOG_MSGID_I(LOG_TAG" [CALL_RX_EQ] received_eq_type=%d", 1, received_eq_type);
                        app_call_rx_eq_set_status_t set_result = app_call_rx_eq_type_set(received_eq_type);
                        ((RSP *)pEvt)->status      = set_result;
                        break;
                    }

                    default:
                        if (pEvt) {
                            RACE_FreePacket(pEvt);
                            pEvt = NULL;
                        }
                        break;
                }
                break;
            }
            case RACE_GET_APP_COMMON_CONFIG: {
                typedef struct {
                    RACE_COMMON_HDR_STRU cmdhdr;
                    uint16_t config_type;
                    uint8_t config_data[0];
                } PACKED CMD;
                CMD *cmd = (CMD *)p_race_package;
                APPS_LOG_MSGID_I(LOG_TAG" race get app common config: handler_id=0x%04X, config_type=0x%04X",
                                 2, p_race_package->hdr.id, cmd->config_type);
                switch (cmd->config_type) {
#ifdef AIR_PROMPT_SOUND_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_VP_LANGUAGE: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t index;
                        } PACKED VPLANG_RSP;
                        uint8_t vp_index = voice_prompt_get_current_language();
                        APPS_LOG_MSGID_I(LOG_TAG" race get app vp language index %d", 1, vp_index);
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(VPLANG_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((VPLANG_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_VP_LANGUAGE;
                            ((VPLANG_RSP *)pEvt)->status = 0x00;
                            ((VPLANG_RSP *)pEvt)->index = vp_index;
                        }
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE;
                            uint8_t ret = apps_get_touch_control_status();
                            if (0xFF == ret) {
                                ((QUERY_RSP *)pEvt)->status = 0xFF;
                                ((QUERY_RSP *)pEvt)->enable = 0xFF;
                                APPS_LOG_MSGID_I(LOG_TAG" get touch key status: 0x%02X", 1, ((QUERY_RSP *)pEvt)->status);
                            } else {
                                ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                                ((QUERY_RSP *)pEvt)->enable = ret;
                                APPS_LOG_MSGID_I(LOG_TAG" get touch key status: 0x%02X", 1, ((QUERY_RSP *)pEvt)->status);
                            }
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" get touch key status pevt return NULL", 0);
                        }
                        break;
                    }
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
                    case APPS_RACE_CMD_CONFIG_TYPE_POWER_SAVING_CFG: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint16_t enable_flag;
                            uint16_t timeout;
                            uint16_t silence_detect_enable;
                            uint16_t silence_detect_timeout;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_POWER_SAVING_CFG;
                            app_power_saving_cfg *cfg = app_power_saving_get_cfg();
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            ((QUERY_RSP *)pEvt)->enable_flag = cfg->power_saving_enable;
                            ((QUERY_RSP *)pEvt)->timeout = cfg->timeout;
                            ((QUERY_RSP *)pEvt)->silence_detect_enable = cfg->silence_detect_enable;
                            ((QUERY_RSP *)pEvt)->silence_detect_timeout = cfg->silence_detect_timeout;
                            APPS_LOG_MSGID_I(LOG_TAG"get power saving cfg, en: %d, t:%d, en_silence: %d, t:%d", 4, cfg->power_saving_enable, cfg->timeout, cfg->silence_detect_enable, cfg->silence_detect_timeout);
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" get power saving pevt return NULL", 0);
                        }
                        break;
                    }
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_IN_EAR_MUSIC_CFG: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_IN_EAR_MUSIC_CFG;
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            uint8_t temp_music_in_ear_cfg = app_music_get_in_ear_control_state();
                            uint8_t enabled = (temp_music_in_ear_cfg != APP_MUSIC_IN_EAR_DISABLE) ? 1 : 0;
                            ((QUERY_RSP *)pEvt)->enable = enabled;
                            APPS_LOG_MSGID_I(LOG_TAG"get music in ear cfg, status=%d, enable=%d", 2, ((QUERY_RSP *)pEvt)->status, enabled);
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" get music in ear cfg pevt return NULL", 0);
                        }
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_LEVEL: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            int16_t sidetone_value;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_LEVEL;
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            ((QUERY_RSP *)pEvt)->sidetone_value = (int16_t)(apps_config_audio_helper_get_sidetone_data()->value);
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_ENABLE: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t sidetone_enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_SIDETONE_ENABLE;
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            ((QUERY_RSP *)pEvt)->sidetone_enable = apps_config_audio_helper_get_sidetone_data()->enable;
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_ADVANCED_PASSTHROUGH: {
                        APPS_LOG_MSGID_I(LOG_TAG" [ADVANCE_PT] get via RACE", 0);
                        bool enable = false;
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ADVANCED_PASSTHROUGH;
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
                            enable = app_advance_passthrough_is_enable();
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
#else
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_NOT_SUPPORT;
#endif
                            ((QUERY_RSP *)pEvt)->enable = enable;
                            APPS_LOG_MSGID_I(LOG_TAG" [ADVANCE_PT] get via RACE, enable=%d", 1, enable);
                        } else {
                            APPS_LOG_MSGID_E(LOG_TAG" [ADVANCE_PT] get via RACE, error", 0);
                        }
                        break;
                    }

                    case APPS_RACE_CMD_CONFIG_TYPE_USB_LOGGING_MODE: {
                        typedef struct {
                            uint8_t enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
#ifdef AIR_USB_HID_ENABLE
                            ((QUERY_RSP *)pEvt)->enable = (uint8_t)USB_Get_LoggingMode();
#endif
                            APPS_LOG_MSGID_I(LOG_TAG" get usb logging mode: %d", 1, ((QUERY_RSP *)pEvt)->enable);
                        } else {
                            APPS_LOG_MSGID_I(LOG_TAG" get usb logging mode status pevt return NULL", 0);
                        }
                        break;
                    }
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    case APPS_RACE_CMD_CONFIG_TYPE_ULL_MIX_RATIO:
                    case APPS_RACE_CMD_CONFIG_TYPE_ULL_MIC_VOL: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t value;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt) {
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            if (cmd->config_type == APPS_RACE_CMD_CONFIG_TYPE_ULL_MIX_RATIO) {
                                ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ULL_MIX_RATIO;
                                ((QUERY_RSP *)pEvt)->value = app_ull_get_mix_ratio();
                            } else {
                                ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_ULL_MIC_VOL;
                                ((QUERY_RSP *)pEvt)->value = app_ull_get_mic_vol();
                            }
                        }
                        break;
                    }
#endif
#ifdef AIR_MULTI_POINT_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_MULTI_POINT_ENABLE: {
                        bool enable = app_bt_emp_is_enable();
                        APPS_LOG_MSGID_I(LOG_TAG" get current multi point enable %d", 1, enable);
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t enable;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt != NULL) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_MULTI_POINT_ENABLE;
                            ((QUERY_RSP *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            ((QUERY_RSP *)pEvt)->enable = enable;
                        }
                        break;
                    }
#endif
#ifdef AIR_USB_ENABLE
                    case APPS_RACE_CMD_CONFIG_TYPE_USB_MODE: {
                        typedef struct {
                            uint8_t status;
                            uint16_t config_type;
                            uint8_t mode;
                        } PACKED get_usb_mode_response_t;
                        uint8_t mode = apps_usb_event_get_current_usb_mode();

                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(get_usb_mode_response_t),
                                                     channel_id);
                        if (pEvt) {
                            ((get_usb_mode_response_t *)pEvt)->status = RACE_ERRCODE_SUCCESS;
                            ((get_usb_mode_response_t *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_USB_MODE;
                            ((get_usb_mode_response_t *)pEvt)->mode = mode;
                        }
                        break;
                    }
#endif
                    case APPS_RACE_CMD_CONFIG_TYPE_CALL_RX_EQ_CFG: {
                        app_call_rx_eq_type_t get_rx_eq_type;
                        app_call_rx_eq_get_status_t get_rx_eq_rst = app_call_rx_eq_type_get(&get_rx_eq_type);
                        typedef struct {
                            uint8_t   status;
                            uint16_t  config_type;
                            uint8_t   rx_eq_type;
                        } PACKED QUERY_RSP;
                        pEvt = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                                     RACE_TYPE_RESPONSE,
                                                     RACE_GET_APP_COMMON_CONFIG,
                                                     sizeof(QUERY_RSP),
                                                     channel_id);
                        if (pEvt) {
                            ((QUERY_RSP *)pEvt)->config_type = APPS_RACE_CMD_CONFIG_TYPE_CALL_RX_EQ_CFG;
                            ((QUERY_RSP *)pEvt)->status      = get_rx_eq_rst;
                            ((QUERY_RSP *)pEvt)->rx_eq_type  = get_rx_eq_type;
                        }
                        break;
                    }
                    case APPS_RACE_CMD_CONFIG_TYPE_LINK_QUALITY: {
                        APPS_LOG_MSGID_I(LOG_TAG" APPS_RACE_CMD_CONFIG_TYPE_LINK_QUALITY TYPE=%d", 1, cmd->config_data[0]);
                        switch (cmd->config_data[0]) {
                            case BT_APP_COMMON_LINK_QUALITY_SPP_RSSI: {
                                bt_bd_addr_t connected_address[3];
                                uint32_t connected_num = 3;
#ifdef AIR_HEADSET_ENABLE
                                connected_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                                            connected_address, connected_num);
#else
                                connected_num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                                            connected_address, connected_num);
#endif
                                APPS_LOG_MSGID_I(LOG_TAG" read connected edr nums=%d", 1, connected_num);
                                if (connected_num > 0) {
                                    bt_gap_connection_handle_t gap_handle = bt_gap_get_handle_by_address((const bt_bd_addr_t*) &connected_address[0]);
                                    bt_gap_read_rssi(gap_handle);
                                }
                                break;
                            }
#ifdef AIR_LE_AUDIO_ENABLE
                            case BT_APP_COMMON_LINK_QUALITY_BLE_RSSI: {
                                extern bt_handle_t app_lea_conn_mgr_get_handle(uint8_t index);
                                bt_hci_cmd_read_rssi_t read_rssi = {
                                    .handle = app_lea_conn_mgr_get_handle(0),
                                };
                                bt_gap_le_read_rssi(&read_rssi);
                                break;
                            }
#endif
                            case 0xff:
                            case BT_APP_COMMON_LINK_QUALITY_BT_CRC: {
                                extern void bt_pka_clear_esco_iso_statistic(void);
                                bt_pka_clear_esco_iso_statistic();
                                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_LINK_QUALITY_ESCO_CRC_READ_ONCE, NULL, 0,
                                    NULL, 1000);
                                if (cmd->config_data[0] == BT_APP_COMMON_LINK_QUALITY_BT_CRC) {
                                    break;
                                }
                            }
                            case BT_APP_COMMON_LINK_QUALITY_RSSI: {
                                bt_bd_addr_t connected_address[3];
                                uint32_t connected_num = 3;
#ifdef AIR_HEADSET_ENABLE
                                connected_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                                            connected_address, connected_num);
#else
                                connected_num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                                            connected_address, connected_num);
#endif
                                APPS_LOG_MSGID_I(LOG_TAG" read connected edr nums=%d", 1, connected_num);
                                if (connected_num > 0) {
                                    bt_gap_connection_handle_t gap_handle = bt_gap_get_handle_by_address((const bt_bd_addr_t*) &connected_address[0]);
                                    bt_gap_read_rssi(gap_handle);
                                    break;
                                }
#ifdef AIR_LE_AUDIO_ENABLE
                                extern bt_handle_t app_lea_conn_mgr_get_handle(uint8_t index);
                                bt_hci_cmd_read_rssi_t read_rssi = {
                                    .handle = app_lea_conn_mgr_get_handle(0),
                                };
                                bt_gap_le_read_rssi(&read_rssi);
#endif
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }

                break;
            }

                /* In-ear app set or get cfg. */
                /* In-ear app set or get cfg. */
#ifdef MTK_RACE_CMD_ENABLE
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            case RACE_ID_APP_IN_EAR_SET_MUSIC_CFG:
            case RACE_ID_APP_IN_EAR_GET_MUSIC_CFG:
                return apps_race_cmd_in_ear(p_race_package, length, channel_id);
#endif
                /* Power saving */
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            case RACE_ID_APP_POWER_SAVING_SET_CFG:
            case RACE_ID_APP_POWER_SAVING_GET_CFG:
                return apps_race_cmd_power_saving(p_race_package, length, channel_id);
#endif
#endif

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
            case RACE_ID_APP_HEAR_THROUGH_CONFIG:
                return app_hear_through_race_cmd_handler(p_race_package, length, channel_id);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

            default:
                break;
        }
    }
    return pEvt;
}

void apps_race_cmd_event_init(void)
{
#if defined(MTK_RACE_CMD_ENABLE)
    RACE_HANDLER handler = {
        .id_start = RACE_ID_APPLICATION_BEGIN,
        .id_end = RACE_ID_APPLICATION_END,
        .handler = apps_race_cmd_handler
    };
    race_status_t ret = RACE_Register_Handler(&handler);
    APPS_LOG_MSGID_I(LOG_TAG" app race register fail: ret=%d", 1, ret);
#endif
}

#endif /* #ifdef MTK_RACE_CMD_ENABLE */
