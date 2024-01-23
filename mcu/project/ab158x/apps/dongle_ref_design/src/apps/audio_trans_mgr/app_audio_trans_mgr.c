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
 * File: app_audio_trans_mgr_idle_activity.c
 *
 * Description: This file implement the audio management logic.
 *
 */

#include "audio_source_control.h"
#include "app_audio_trans_mgr.h"
#include "app_audio_trans_mgr_idle_activity.h"
#include "apps_debug.h"
#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
#include "audio_src_srv_resource_manager_config.h"
#endif
#if defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#include "apps_race_cmd_co_sys_event.h"
#endif
#include "audio_src_srv.h"
#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "bt_sink_srv_ami.h"
#include "hal_gpio.h"
#include "ept_gpio_var.h"
#include <string.h>

#define APP_AUDIO_TRANS_MGR_LOG_I(msg, ...)     APPS_LOG_MSGID_I("[APP AUDIO TRANS MGR]"msg, ##__VA_ARGS__)
#define APP_AUDIO_TRANS_MGR_LOG_E(msg, ...)     APPS_LOG_MSGID_E("[APP AUDIO TRANS MGR]"msg, ##__VA_ARGS__)
#define APP_AUDIO_TRANS_MGR_LOG_D(msg, ...)     APPS_LOG_MSGID_D("[APP AUDIO TRANS MGR]"msg, ##__VA_ARGS__)

typedef enum {
    APP_AUDIO_TRANS_MGR_TRANS_STA_NONE = 0,
    APP_AUDIO_TRANS_MGR_TRANS_STA_INIT = 1,
    APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE = 2,
    APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING = 3,
    APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING = 4,
    APP_AUDIO_TRANS_MGR_TRANS_STA_STOPPING = 5,
    APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND = 6,
} app_audio_trans_mgr_trans_sta_t;

/* only init/start/stop/deinit cmd will be queued now. */
#define APP_AUDIO_TRANS_MGR_CMD_INIT 0
#define APP_AUDIO_TRANS_MGR_CMD_START 1
#define APP_AUDIO_TRANS_MGR_CMD_STOP 2
#define APP_AUDIO_TRANS_MGR_CMD_DEINIT 3
#define APP_AUDIO_TRANS_MGR_CMD_SET_VOLUME 4
#define APP_AUDIO_TRANS_MGR_CMD_SET_MIX_RATIO 5
#define APP_AUDIO_TRANS_MGR_CMD_SET_MUTE 6
#define APP_AUDIO_TRANS_MGR_CMD_NEXT_CMD 7
#define APP_AUDIO_TRANS_MGR_CMD_COSYS_PLAY 8
#define APP_AUDIO_TRANS_MGR_CMD_INVALID 0xff
typedef uint8_t app_audio_trans_mgr_cmd_t;

typedef enum {
    APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_START = 0,
    APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_STOP  = 1,
    APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_REJECT  = 2,
    APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_SUSPEND = 3,
    APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_EXCEPTION = 4,
} app_audio_trans_mgr_pseudo_event_type_t;

#define MGR_CMD_QUEUE_SIZE 16
typedef struct {
    app_audio_trans_mgr_usr_t usr_info;
    app_audio_trans_mgr_trans_sta_t state;
    app_audio_trans_mgr_cmd_t cmd_queue[MGR_CMD_QUEUE_SIZE];
    uint8_t queued_cmd_nums;
    audio_transmitter_id_t trans_id;
    bool suspended; /* just for usb audio because usb in1 & usb in2 share same instance. */
#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
    audio_src_srv_resource_manager_handle_t *resource_manager_handle;
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    void *audio_source_ctrl_handle;
#endif
    audio_src_srv_handle_t *pseudo_device_handle;
} app_audio_trans_mgr_usr_ctx_t;

typedef struct {
    app_audio_trans_mgr_usr_ctx_t usrs[APP_AUDIO_TRANS_MGR_USR_MAX_NUM];
} app_audio_trans_mgr_ctx_t;

static app_audio_trans_mgr_ctx_t s_ctx;

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
static void __app_audio_trans_mgr_set_transmitter_volume(app_audio_trans_mgr_usr_ctx_t *u_ctx);
#endif

static void __set_trans_state(app_audio_trans_mgr_usr_ctx_t *u_ctx, app_audio_trans_mgr_trans_sta_t new_sta)
{
    APP_AUDIO_TRANS_MGR_LOG_I("usr=%d, old_sta=%d, new_sta=%d", 3, u_ctx->usr_info.type, u_ctx->state, new_sta);
    u_ctx->state = new_sta;
}

static void __send_audio_trans_mgr_event(uint32_t event_id, app_audio_trans_mgr_usr_type_t usr, uint32_t event)
{
    app_audio_trans_mgr_interaction_event_t *ev = (app_audio_trans_mgr_interaction_event_t *)pvPortMalloc(sizeof(app_audio_trans_mgr_interaction_event_t));
    if (ev == NULL) {
        return;
    }

    ev->usr = usr;
    ev->event = event;
    ui_shell_status_t status = ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR, event_id,
                                                   (void *)ev, sizeof(app_audio_trans_mgr_interaction_event_t),
                                                   NULL, 0);
    if (UI_SHELL_STATUS_OK != status) {
        vPortFree(ev);
        APP_AUDIO_TRANS_MGR_LOG_E("usr=%d, ev=%d, send ui event fail=%d", 3, usr, event, status);
    }
}

static void __send_self_cmd(app_audio_trans_mgr_usr_ctx_t *u_ctx, app_audio_trans_mgr_cmd_t cmd)
{
    if (u_ctx == NULL) {
        return;
    }

    __send_audio_trans_mgr_event((uint32_t)APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD, u_ctx->usr_info.type, (uint32_t)cmd);
}

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
static app_audio_trans_mgr_cmd_t __get_next_cmd(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    if (u_ctx != NULL && u_ctx->queued_cmd_nums != 0) {
        app_audio_trans_mgr_cmd_t next_command = u_ctx->cmd_queue[0];
        u_ctx->queued_cmd_nums--;
        APP_AUDIO_TRANS_MGR_LOG_I("get next command=%d, queue_len=%d, queued_cmd=%d", 3,
                                  next_command, u_ctx->queued_cmd_nums, u_ctx->cmd_queue[0]);
        for (uint32_t i = 0; i < u_ctx->queued_cmd_nums && i < MGR_CMD_QUEUE_SIZE - 1; i++) {
            u_ctx->cmd_queue[i] = u_ctx->cmd_queue[i + 1];
        }
        return next_command;
    }
    return APP_AUDIO_TRANS_MGR_CMD_INVALID;
}
#endif

static void app_audio_trans_mgr_transmitter_event_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)user_data;
    if (u_ctx == NULL) {
        return;
    }

    if (event != AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION && event != AUDIO_TRANSMITTER_EVENT_DATA_DIRECT) {
        APP_AUDIO_TRANS_MGR_LOG_I("transmitter callback, usr=%d, event=%d, ctrl=%d, sta=%d", 4,
                                  u_ctx->usr_info.type, event, u_ctx->usr_info.ctrl_type, u_ctx->state);
        __send_audio_trans_mgr_event(APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_TRANSMITTER_EV, u_ctx->usr_info.type, (uint32_t)event);
    }
}

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
static void app_audio_trans_mgr_transmitter_event_proc(app_audio_trans_mgr_usr_ctx_t *u_ctx, audio_transmitter_event_t event)
{
    if (u_ctx == NULL) {
        return;
    }

    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS:
            __app_audio_trans_mgr_set_transmitter_volume(u_ctx);
            if (u_ctx->usr_info.en_side_tone) {
                am_audio_side_tone_enable();
            }
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_OUT &&
                u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL) {
                audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                         AUDIO_SOURCE_CONTROL_CMD_START_TRANSMITTER, AUDIO_SOURCE_CONTROL_CMD_DEST_REMOTE);
            } else {
                __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING);
            }
#else
            __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING);
#endif
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                hal_gpio_set_output(BSP_LINE_IN_SWITCH_PIN, HAL_GPIO_DATA_LOW);
            }
            break;
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
            break;
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                hal_gpio_set_output(BSP_LINE_IN_SWITCH_PIN, HAL_GPIO_DATA_HIGH);
            }

            app_audio_trans_mgr_trans_sta_t now_sta = u_ctx->state;
#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
            if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL) {
                /* special process, usb in1 & usb in2 use same instance */
                if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                    bool release = true;
                    bool add_waiting = false;
                    if ((u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1) && (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].state >= APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING)) {
                        release = false;
                    }
                    if ((u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) && (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].state >= APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING)) {
                        release = false;
                    }
                    if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].state == APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND ||
                        s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].state == APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND) {
                        add_waiting = true;
                    }
                    if (release) {
                        audio_src_srv_resource_manager_give(u_ctx->resource_manager_handle);
                    } else {
                        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                    }
                    if (add_waiting) {
                        audio_src_srv_resource_manager_add_waiting_list(u_ctx->resource_manager_handle);
                    }
                } else {
                    audio_src_srv_resource_manager_give(u_ctx->resource_manager_handle);
                    if (now_sta == APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND) {
                        audio_src_srv_resource_manager_add_waiting_list(u_ctx->resource_manager_handle);
                    }
                }
            }
#endif
            if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE) {
                audio_src_srv_update_state(u_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                audio_src_srv_update_state(u_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_READY);
                if (now_sta == APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND) {
                    audio_src_srv_add_waiting_list(u_ctx->pseudo_device_handle);
                }
            }
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL) {
                audio_source_control_cmd(u_ctx->audio_source_ctrl_handle, AUDIO_SOURCE_CONTROL_CMD_STOP_TRANSMITTER, AUDIO_SOURCE_CONTROL_CMD_DEST_REMOTE);
                if (now_sta) {
                    audio_source_control_cmd(u_ctx->audio_source_ctrl_handle, AUDIO_SOURCE_CONTROL_CMD_ADD_WAITTING_LIST, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
                }
            }
#endif
            if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_NONE) {
                __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            }
            if (u_ctx->usr_info.en_side_tone) {
                am_audio_side_tone_disable();
            }

#ifdef AIR_DCHS_MODE_MASTER_ENABLE
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 ||
                u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2 ||
                u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                uint16_t extra_data = (uint8_t)u_ctx->usr_info.type;
                extra_data <<= 8;
                extra_data |= APP_AUDIO_TRANS_MGR_CMD_STOP;
                app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR, APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD,
                                               &extra_data, sizeof(uint16_t), false);
            }
#endif
            break;
        }
    }

    /* start/stop completed, check cmd queue. */
    if (event == AUDIO_TRANSMITTER_EVENT_START_SUCCESS || event == AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS) {
        __send_self_cmd(u_ctx, APP_AUDIO_TRANS_MGR_CMD_NEXT_CMD);
    }
}
#endif

#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
static void app_audio_trans_mgr_judgement_callback_handler(struct _audio_src_srv_resource_manager_handle_t *current_handle,
                                                           audio_src_srv_resource_manager_event_t event)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = NULL;
    for (uint32_t i = 0; i < APP_AUDIO_TRANS_MGR_USR_MAX_NUM; i++) {
        if (s_ctx.usrs[i].resource_manager_handle == current_handle) {
            u_ctx = &s_ctx.usrs[i];
            break;
        }
    }
    if (u_ctx == NULL) {
        return;
    }

    APP_AUDIO_TRANS_MGR_LOG_I("judgement callback, usr=%d, event=%d", 2, u_ctx->usr_info.type, event);
    __send_audio_trans_mgr_event(APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_RESOURCE_CTRL_EV, u_ctx->usr_info.type, (uint32_t)event);
}

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
static void app_audio_trans_mgr_judgement_event_proc(app_audio_trans_mgr_usr_ctx_t *u_ctx,
                                                     audio_src_srv_resource_manager_event_t event)
{
    if (u_ctx == NULL) {
        return;
    }

    switch (event) {
        case AUDIO_SRC_SRV_EVENT_NONE:
            break;
        case AUDIO_SRC_SRV_EVENT_TAKE_ALREADY:
        case AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS: {
            /* usb in1 & usb in2 share same handle */
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING || s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].suspended) {
                    audio_transmitter_start(s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].trans_id);
                    u_ctx->suspended = false;
                }
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING || s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].suspended) {
                    audio_transmitter_start(s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].trans_id);
                    u_ctx->suspended = false;
                }
            } else {
                audio_transmitter_start(u_ctx->trans_id);
            }
            break;
        }
        case AUDIO_SRC_SRV_EVENT_GIVE_ERROR:
            break;
        case AUDIO_SRC_SRV_EVENT_TAKE_REJECT:
            u_ctx->suspended = true;
            audio_src_srv_resource_manager_add_waiting_list(u_ctx->resource_manager_handle);
            //__set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING) {
                    s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].suspended = true;
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                }
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
                    s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].suspended = true;
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                }
            } else {
                __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            }
            break;
        case AUDIO_SRC_SRV_EVENT_GIVE_SUCCESS:
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                __set_trans_state(&s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1], APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                __set_trans_state(&s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2], APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            } else {
                __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            }
            break;
        case AUDIO_SRC_SRV_EVENT_SUSPEND:
            /* usb in1 & usb in2 share same handle */
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
                    s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].suspended = true;
                    __set_trans_state(&s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1], APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
                    audio_transmitter_stop(s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].trans_id);
                }
                if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
                    s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].suspended = true;
                    __set_trans_state(&s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2], APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
                    audio_transmitter_stop(s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].trans_id);
                }
            } else {
                __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
                audio_transmitter_stop(u_ctx->trans_id);
            }
            /* release it when stop success. */
            //audio_src_srv_resource_manager_give(u_ctx->resource_manager_handle);
            //audio_src_srv_resource_manager_add_waiting_list(u_ctx->resource_manager_handle);
            break;
    }
}
#endif
#endif

static void __send_pseudo_event(struct _audio_src_srv_handle_t *handle, app_audio_trans_mgr_pseudo_event_type_t event)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = NULL;
    for (uint32_t i = 0; i < APP_AUDIO_TRANS_MGR_USR_MAX_NUM; i++) {
        if (s_ctx.usrs[i].pseudo_device_handle == handle) {
            u_ctx = &s_ctx.usrs[i];
            break;
        }
    }
    if (u_ctx == NULL) {
        return;
    }

    __send_audio_trans_mgr_event(APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_PSEUDO_DEV_EV, u_ctx->usr_info.type, (uint32_t)event);
    APP_AUDIO_TRANS_MGR_LOG_I("__send_pseudo_event, usr=%d, event=%d", 2, u_ctx->usr_info.type, event);
}

static void pseudo_play_callback(struct _audio_src_srv_handle_t *handle)
{
    __send_pseudo_event(handle, APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_START);
}

static void pseudo_stop_callback(struct _audio_src_srv_handle_t *handle)
{
    __send_pseudo_event(handle, APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_STOP);
}

static void pseudo_suspend_callback(struct _audio_src_srv_handle_t *handle, struct _audio_src_srv_handle_t *int_hd)
{
    __send_pseudo_event(handle, APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_SUSPEND);
}

static void pseudo_reject_callback(struct _audio_src_srv_handle_t *handle)
{
    __send_pseudo_event(handle, APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_REJECT);
}

static void pseudo_exception_handle(struct _audio_src_srv_handle_t *handle, int32_t event, void *param)
{
    __send_pseudo_event(handle, APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_EXCEPTION);
    APP_AUDIO_TRANS_MGR_LOG_I("pseudo_exception_handle exception=%d", 1, event);
}

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
static void app_audio_trans_mgr_pseudo_event_proc(app_audio_trans_mgr_usr_ctx_t *u_ctx, app_audio_trans_mgr_pseudo_event_type_t event)
{
    if (u_ctx == NULL) {
        return;
    }

    switch (event) {
        case APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_START:
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
            if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 ||
                u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2 ||
                u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                uint16_t extra_data = (uint8_t)u_ctx->usr_info.type;
                extra_data <<= 8;
                extra_data |= APP_AUDIO_TRANS_MGR_CMD_START;
                app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR, APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD,
                                               &extra_data, sizeof(uint16_t), false);
                break;
            }
#endif
            audio_transmitter_start(u_ctx->trans_id);
            break;
        case APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_STOP:
            __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            break;
        case APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_REJECT:
            //__set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
            __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
            audio_src_srv_add_waiting_list(u_ctx->pseudo_device_handle);
            break;
        case APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_SUSPEND:
            __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
            audio_transmitter_stop(u_ctx->trans_id);
            //audio_src_srv_update_state(u_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_READY);
            break;
        case APP_AUDIO_TRANS_MGR_PSEUDO_EVENT_EXCEPTION:
            break;
    }
}
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
static void audio_trans_mgr_audio_source_callback(audio_source_control_event_type type, uint32_t sub_event, void *usr_data)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)usr_data;
    if (u_ctx == NULL) {
        return;
    }
    uint32_t event = ((type << 16) & 0xFFFF0000) | (sub_event & 0xFFFF);
    __send_audio_trans_mgr_event(APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_AUD_SRC_CTRL_EV, u_ctx->usr_info.type, (uint32_t)event);
    APP_AUDIO_TRANS_MGR_LOG_I("audio source ctrl callback, usr=%d, event=%d", 2, u_ctx->usr_info.type, event);
}

static void audio_trans_mgr_audio_source_event_proc(app_audio_trans_mgr_usr_ctx_t *u_ctx, uint32_t event)
{
    if (u_ctx == NULL) {
        return;
    }

    audio_source_control_event_type ev_type = (audio_source_control_event_type)((event >> 16) & 0xFFFF);
    switch (ev_type) {
        case AUDIO_SOURCE_CONTROL_EVENT_RES_CTRL: {
            audio_src_srv_resource_manager_event_t res_ev = (audio_src_srv_resource_manager_event_t)(event & 0xFFFF);
            switch (res_ev) {
                case AUDIO_SRC_SRV_EVENT_TAKE_ALREADY:
                case AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS:
                    if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                        audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                                 AUDIO_SOURCE_CONTROL_CMD_START_TRANSMITTER, AUDIO_SOURCE_CONTROL_CMD_DEST_REMOTE);
                    } else {
                        audio_transmitter_start(u_ctx->trans_id);
                    }
                    break;
                case AUDIO_SRC_SRV_EVENT_TAKE_REJECT:
                    //__set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                    audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                             AUDIO_SOURCE_CONTROL_CMD_ADD_WAITTING_LIST, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
                    break;
                case AUDIO_SRC_SRV_EVENT_GIVE_SUCCESS:
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                    break;
                case AUDIO_SRC_SRV_EVENT_SUSPEND:
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_SUSPEND);
                    audio_transmitter_stop(u_ctx->trans_id);
                    /* release it when stop success. */
#if 0
                    audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                             AUDIO_SOURCE_CONTROL_CMD_RELEASE_RES, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
                    audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                             AUDIO_SOURCE_CONTROL_CMD_ADD_WAITTING_LIST, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
#endif
                    break;
            }
            break;
        }
        case AUDIO_SOURCE_CONTROL_EVENT_TRANSMITTER: {
            audio_transmitter_event_t trans_ev = (audio_transmitter_event_t)(event & 0xFFFF);
            switch (trans_ev) {
                /* The event must come from remote, it audio source ctrl case. */
                case AUDIO_TRANSMITTER_EVENT_START_SUCCESS:
                    if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                        audio_transmitter_start(u_ctx->trans_id);
                    } else if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_LINE_OUT) {
                        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING);
                    }
                    break;
                case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS:
                    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE);
                    audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                             AUDIO_SOURCE_CONTROL_CMD_RELEASE_RES, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
                    break;
            }
            break;
        }
        case AUDIO_SOURCE_CONTROL_EVENT_CMD_FAIL: {
            break;
        }
    }
}

#endif

void __app_audio_trans_mgr_init_audio(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    if (u_ctx == NULL || u_ctx->usr_info.cfg_callback == NULL) {
        return;
    }

    app_audio_trans_mgr_usr_cfg_t cfg;
    memset(&cfg, 0, sizeof(app_audio_trans_mgr_usr_cfg_t));
    u_ctx->usr_info.cfg_callback(u_ctx->usr_info.type, &cfg);

    cfg.trans_cfg.msg_handler = app_audio_trans_mgr_transmitter_event_callback;
    cfg.trans_cfg.user_data = u_ctx;
    //u_ctx->queued_cmd_nums = 0;
    u_ctx->trans_id = audio_transmitter_init(&cfg.trans_cfg);
    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_INIT);

#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
    if (u_ctx->resource_manager_handle == NULL && u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL) {
        /* for special case, usb in & usb in2 use same handle */
        if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 &&
            s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].resource_manager_handle != NULL) {
            u_ctx->resource_manager_handle = s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].resource_manager_handle;
        } else if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2 &&
                   s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].resource_manager_handle != NULL) {
            u_ctx->resource_manager_handle = s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].resource_manager_handle;
        } else {
            u_ctx->resource_manager_handle = audio_src_srv_resource_manager_construct_handle(
                                                 cfg.resource_type, (const char *)u_ctx->usr_info.name);
            if (u_ctx->resource_manager_handle == NULL) {
                APP_AUDIO_TRANS_MGR_LOG_E("init source manager fail, usr=%d, trans_id=%d", 2, u_ctx->usr_info.type, u_ctx->trans_id);
                return;
            }
            u_ctx->resource_manager_handle->callback_func = app_audio_trans_mgr_judgement_callback_handler;
            u_ctx->resource_manager_handle->priority = cfg.priority;
        }
    }
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    if (u_ctx->audio_source_ctrl_handle == NULL && u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL) {
        /* for special case, usb in & usb in2 use same handle */
        if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1) {
            if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].audio_source_ctrl_handle != NULL) {
                u_ctx->audio_source_ctrl_handle = s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2].audio_source_ctrl_handle;
            }
        } else if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
            if (s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].audio_source_ctrl_handle != NULL) {
                u_ctx->audio_source_ctrl_handle = s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1].audio_source_ctrl_handle;
            }
        } else {
            audio_source_control_cfg_t aud_src_ctr_cfg = {
                cfg.resource_type,
                cfg.priority,
                cfg.source_ctrl_type,
                (uint8_t *)u_ctx->usr_info.name,
                (void *)u_ctx,
                audio_trans_mgr_audio_source_callback
            };
            u_ctx->audio_source_ctrl_handle = audio_source_control_register(&aud_src_ctr_cfg);
            if (u_ctx->audio_source_ctrl_handle == NULL) {
                APP_AUDIO_TRANS_MGR_LOG_E("init source ctrl fail, usr=%d, trans_id=%d", 2, u_ctx->usr_info.type, u_ctx->trans_id);
            }
        }
    }
#endif
    if (u_ctx->pseudo_device_handle == NULL && u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE) {
        u_ctx->pseudo_device_handle = audio_src_srv_construct_handle(cfg.pseudo_type);
        if (u_ctx->pseudo_device_handle == NULL) {
            APP_AUDIO_TRANS_MGR_LOG_E("init seudo device fail, usr=%d, trans_id=%d", 2, u_ctx->usr_info.type, u_ctx->trans_id);
            return;
        }
        u_ctx->pseudo_device_handle->type = cfg.pseudo_type;
        u_ctx->pseudo_device_handle->priority = cfg.priority;
        u_ctx->pseudo_device_handle->dev_id = 0;//??
        u_ctx->pseudo_device_handle->play       = pseudo_play_callback;
        u_ctx->pseudo_device_handle->stop       = pseudo_stop_callback;
        u_ctx->pseudo_device_handle->suspend    = pseudo_suspend_callback;
        u_ctx->pseudo_device_handle->reject     = pseudo_reject_callback;
        u_ctx->pseudo_device_handle->exception_handle = pseudo_exception_handle;
        audio_src_srv_update_state(u_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_READY);
    }
    APP_AUDIO_TRANS_MGR_LOG_I("init, usr=%d, trans_id=%d", 2, u_ctx->usr_info.type, u_ctx->trans_id);
}

void __app_audio_trans_mgr_start_audio(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    if (u_ctx == NULL) {
        return;
    }

#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL) {
        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING);
        audio_src_srv_resource_manager_take(u_ctx->resource_manager_handle);
    }
#endif
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE) {
        audio_src_srv_update_state(u_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING);
    }
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL) {
        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING);
        audio_source_control_result_t result = audio_source_control_cmd(u_ctx->audio_source_ctrl_handle,
                                                                        AUDIO_SOURCE_CONTROL_CMD_REQUEST_RES, AUDIO_SOURCE_CONTROL_CMD_DEST_LOCAL);
        APP_AUDIO_TRANS_MGR_LOG_I("audio_source_control_cmd request audio result: %d.", 1, result);
    }
#endif
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_NONE) {
        __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING);
        audio_transmitter_start(u_ctx->trans_id);
    }
}

void __app_audio_trans_mgr_stop_audio(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    if (u_ctx == NULL) {
        return;
    }
    /* special process for usb audio due to usb in1 & usb in2 share same instance!!!!!! */
    if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1 || u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
        u_ctx->suspended = false;
    }

#ifdef APP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL) {
        audio_src_srv_resource_manager_delete_waiting_list((audio_src_srv_resource_manager_handle_t *)u_ctx->pseudo_device_handle);
    }
#endif
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE) {
        audio_src_srv_del_waiting_list(u_ctx->pseudo_device_handle);
    }
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    if (u_ctx->usr_info.ctrl_type == APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL && u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_STOPPING) {
        audio_source_control_cmd(u_ctx->audio_source_ctrl_handle, AUDIO_SOURCE_CONTROL_CMD_DEL_WAITTING_LIST, AUDIO_SOURCE_CONTROL_CMD_DEST_REMOTE);
    }
#endif

    if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE) {
        /* just delete waiting list in this case. */
        return;
    }

    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STOPPING);
    audio_transmitter_stop(u_ctx->trans_id);
}

void __app_audio_trans_mgr_deinit_audio(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    if (u_ctx == NULL) {
        return;
    }

    audio_transmitter_deinit(u_ctx->trans_id);
    __set_trans_state(u_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_NONE);
}

#ifndef AIR_DCHS_MODE_SLAVE_ENABLE
#define USB_AUDIO_MAX_LEVEL 15
#define USB_AUDIO_MAX_VOLUME 100
static void __app_audio_trans_mgr_set_transmitter_volume(app_audio_trans_mgr_usr_ctx_t *u_ctx)
{
    audio_transmitter_status_t sta = AUDIO_TRANSMITTER_STATUS_FAIL;
    audio_transmitter_runtime_config_t config;

    memset(&config, 0, sizeof(audio_transmitter_runtime_config_t));

    if (u_ctx->usr_info.rconfig_type == WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC || u_ctx->usr_info.rconfig_type == WIRED_AUDIO_CONFIG_OP_VOL_DB_VOICE) {
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        if (u_ctx->usr_info.type == APP_AUDIO_TRANS_MGR_USR_USB_IN1) {
            extern int32_t *apps_event_usb_8ch_volumes(void);
            int32_t *volumes_tbls = apps_event_usb_8ch_volumes();
            for (uint32_t i = 0; i < 8; i++) {
                config.wired_audio_runtime_config.vol_db.vol_db[i] = volumes_tbls[i];
            }
        }
#else
        config.wired_audio_runtime_config.vol_db.vol_db[0] = u_ctx->usr_info.mute > 0 ? -1440000 : u_ctx->usr_info.volume_db_l;
        config.wired_audio_runtime_config.vol_db.vol_db[1] = u_ctx->usr_info.mute > 0 ? -1440000 : u_ctx->usr_info.volume_db_r;
        for (uint32_t i = 2; i < 8; i++) {
            config.wired_audio_runtime_config.vol_db.vol_db[i] = config.wired_audio_runtime_config.vol_db.vol_db[1];
        }
#endif
        APP_AUDIO_TRANS_MGR_LOG_I("volueme set, usr=%d, mute=%d, l=%d, r=%d", 4,
                              u_ctx->usr_info.type, u_ctx->usr_info.mute, u_ctx->usr_info.volume_db_l, u_ctx->usr_info.volume_db_r);
        config.wired_audio_runtime_config.vol_db.vol_ratio = u_ctx->usr_info.mix_ratio;
    } else {
        config.wired_audio_runtime_config.vol_level.vol_level[0] = u_ctx->usr_info.mute > 0 ? 0 : u_ctx->usr_info.volume_l;
        config.wired_audio_runtime_config.vol_level.vol_level[1] = u_ctx->usr_info.mute > 0 ? 0 : u_ctx->usr_info.volume_r;
        memset(&config.wired_audio_runtime_config.vol_level.vol_level[2], u_ctx->usr_info.mute > 0 ? 0 : u_ctx->usr_info.volume_r, 6);
        config.wired_audio_runtime_config.vol_level.vol_ratio = u_ctx->usr_info.mix_ratio;
    }

    sta = audio_transmitter_set_runtime_config(u_ctx->trans_id, u_ctx->usr_info.rconfig_type, &config);
    APP_AUDIO_TRANS_MGR_LOG_I("volueme set, usr=%d, mute=%d, l=%d, r=%d, ret=%d", 5,
                              u_ctx->usr_info.type, u_ctx->usr_info.mute, u_ctx->usr_info.volume_l, u_ctx->usr_info.volume_r, sta);
}

static void app_audio_trans_mgr_self_cmd_proc(app_audio_trans_mgr_usr_ctx_t *u_ctx, app_audio_trans_mgr_cmd_t cmd)
{
    if (u_ctx == NULL) {
        return;
    }

    APP_AUDIO_TRANS_MGR_LOG_I("cmd proc, cmd=%d, sta=%d, queue_len=%d, next_cmd=%d", 4,
                              cmd, u_ctx->state, u_ctx->queued_cmd_nums, u_ctx->cmd_queue[0]);

    if (u_ctx->queued_cmd_nums > 0 && cmd <= APP_AUDIO_TRANS_MGR_CMD_DEINIT) {
        if (u_ctx->queued_cmd_nums < MGR_CMD_QUEUE_SIZE) {
            u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = cmd;
        }
        return;
    }

    if (cmd == APP_AUDIO_TRANS_MGR_CMD_NEXT_CMD) {
        cmd = __get_next_cmd(u_ctx);
    }

    /* init & deinit should update cmd queue due to no event trigger back. */
    switch (cmd) {
        case APP_AUDIO_TRANS_MGR_CMD_INIT:
            if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_NONE) {
                __app_audio_trans_mgr_init_audio(u_ctx);
                __send_self_cmd(u_ctx, APP_AUDIO_TRANS_MGR_CMD_NEXT_CMD);
            } else {
                u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = cmd;
            }
            break;
        case APP_AUDIO_TRANS_MGR_CMD_START:
            if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_INIT || u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE) {
                __app_audio_trans_mgr_start_audio(u_ctx);
            } else if (u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING && u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
                u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = cmd;
            }
            break;
        case APP_AUDIO_TRANS_MGR_CMD_STOP:
            if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING || u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE) {
                __app_audio_trans_mgr_stop_audio(u_ctx);
            } else if (u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_STOPPING &&
                       u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_INIT) {
                u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = cmd;
            }
            break;
        case APP_AUDIO_TRANS_MGR_CMD_DEINIT:
            if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE || u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_INIT) {
                __app_audio_trans_mgr_deinit_audio(u_ctx);
                __send_self_cmd(u_ctx, APP_AUDIO_TRANS_MGR_CMD_NEXT_CMD);
            } else {
                u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = cmd;
            }
            break;
        case APP_AUDIO_TRANS_MGR_CMD_SET_VOLUME:
        case APP_AUDIO_TRANS_MGR_CMD_SET_MIX_RATIO:
        case APP_AUDIO_TRANS_MGR_CMD_SET_MUTE:
            if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
                __app_audio_trans_mgr_set_transmitter_volume(u_ctx);
            }
            break;
    }
}
#endif

static void app_audio_trans_mgr_init()
{
    memset(&s_ctx, 0, sizeof(app_audio_trans_mgr_ctx_t));
    APP_AUDIO_TRANS_MGR_LOG_I("app_audio_trans_mgr_init", 0);
}

#if defined(AIR_DCHS_MODE_SLAVE_ENABLE)
static app_audio_trans_mgr_usr_type_t s_request_usrs[3] = {APP_AUDIO_TRANS_MGR_USR_MAX_NUM, APP_AUDIO_TRANS_MGR_USR_MAX_NUM, APP_AUDIO_TRANS_MGR_USR_MAX_NUM};
static bool s_get_resource = false;
static void cosys_seudo_play_callback(struct _audio_src_srv_handle_t *handle)
{
    APP_AUDIO_TRANS_MGR_LOG_I("cosys_seudo_play_callback", 0);
    for (uint32_t i = 0; i < 3; i++) {
        if (s_request_usrs[i] != APP_AUDIO_TRANS_MGR_USR_MAX_NUM) {
            uint16_t extra_data = (uint8_t)s_request_usrs[i];
            extra_data <<= 8;
            extra_data |= APP_AUDIO_TRANS_MGR_CMD_COSYS_PLAY;
            app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR, APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SALVE_EV,
                                           &extra_data, sizeof(uint16_t), false);

            s_request_usrs[i] = APP_AUDIO_TRANS_MGR_USR_MAX_NUM;
        }
    }

    s_get_resource = true;
}

static void cosys_seudo_stop_callback(struct _audio_src_srv_handle_t *handle)
{
    APP_AUDIO_TRANS_MGR_LOG_I("cosys_seudo_stop_callback", 0);
    s_get_resource = false;
}

static void cosys_seudo_suspend_callback(struct _audio_src_srv_handle_t *handle, struct _audio_src_srv_handle_t *int_hd)
{
    APP_AUDIO_TRANS_MGR_LOG_I("cosys_seudo_suspend_callback", 0);
}

static void cosys_seudo_reject_callback(struct _audio_src_srv_handle_t *handle)
{
    APP_AUDIO_TRANS_MGR_LOG_I("cosys_seudo_reject_callback", 0);
}

static void cosys_seudo_exception_handle(struct _audio_src_srv_handle_t *handle, int32_t event, void *param)
{
    APP_AUDIO_TRANS_MGR_LOG_I("cosys_seudo_exception_handle", 0);
}

static bool app_audio_trans_mgr_event_proc(ui_shell_activity_t *self,
                                           uint32_t event_id,
                                           void *extra_data,
                                           size_t data_len)
{
    if (event_id != APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD) {
        return false;
    }
    static audio_src_srv_handle_t *s_usr_handle = NULL;
    static uint32_t s_play_counter = 0;
    uint16_t data;
    memcpy(&data, extra_data, sizeof(uint16_t));

    app_audio_trans_mgr_usr_type_t usr = (app_audio_trans_mgr_usr_type_t)(data >> 8);
    app_audio_trans_mgr_cmd_t cmd = (app_audio_trans_mgr_cmd_t)(data & 0xff);

    APP_AUDIO_TRANS_MGR_LOG_I("app_audio_trans_mgr_event_proc, usr=%d, cmd=%d", 2, usr, cmd);

    /* init */
    if (s_usr_handle == NULL) {
        s_usr_handle = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_LINE_IN);
        s_usr_handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_LINE_IN;
        s_usr_handle->priority = 7;
        s_usr_handle->dev_id = 0;//??
        s_usr_handle->play       = cosys_seudo_play_callback;
        s_usr_handle->stop       = cosys_seudo_stop_callback;
        s_usr_handle->suspend    = cosys_seudo_suspend_callback;
        s_usr_handle->reject     = cosys_seudo_reject_callback;
        s_usr_handle->exception_handle = cosys_seudo_exception_handle;
        audio_src_srv_update_state(s_usr_handle, AUDIO_SRC_SRV_EVT_READY);
    }

    if (cmd == APP_AUDIO_TRANS_MGR_CMD_START) {
        if (s_get_resource) {
            uint16_t send_data = (uint8_t)usr;
            send_data <<= 8;
            send_data |= APP_AUDIO_TRANS_MGR_CMD_COSYS_PLAY;
            app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR, APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SALVE_EV,
                                           &send_data, sizeof(uint16_t), false);

        } else {
            for (uint32_t i = 0; i < 3; i++) {
                if (s_request_usrs[i] == APP_AUDIO_TRANS_MGR_USR_MAX_NUM) {
                    s_request_usrs[i] = usr;
                    if (i == 0) {
                        audio_src_srv_update_state(s_usr_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
                    }
                    break;
                }
            }
        }
        s_play_counter++;
    } else if (cmd == APP_AUDIO_TRANS_MGR_CMD_STOP) {
        if (s_play_counter > 0) {
            s_play_counter--;
            if (s_play_counter == 0) {
                audio_src_srv_update_state(s_usr_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                audio_src_srv_update_state(s_usr_handle, AUDIO_SRC_SRV_EVT_READY);
            }
        }
    }

    return false;
}

#else

static bool app_audio_trans_mgr_event_proc(ui_shell_activity_t *self,
                                           uint32_t event_id,
                                           void *extra_data,
                                           size_t data_len)
{
    bool ret = false;
#if defined(AIR_DCHS_MODE_MASTER_ENABLE)
    if (event_id == APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SALVE_EV) {
        uint16_t data;
        memcpy(&data, extra_data, sizeof(uint16_t));

        app_audio_trans_mgr_usr_type_t usr = (app_audio_trans_mgr_usr_type_t)(data >> 8);
        app_audio_trans_mgr_cmd_t cmd = (app_audio_trans_mgr_cmd_t)(data & 0xff);
        if (cmd == APP_AUDIO_TRANS_MGR_CMD_COSYS_PLAY) {
            app_audio_trans_mgr_usr_ctx_t *cu_ctx = NULL;
            if (usr == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
                cu_ctx = &s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_LINE_IN];
            } else if (usr == APP_AUDIO_TRANS_MGR_USR_USB_IN1) {
                cu_ctx = &s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN1];
            } else if (usr == APP_AUDIO_TRANS_MGR_USR_USB_IN2) {
                cu_ctx = &s_ctx.usrs[APP_AUDIO_TRANS_MGR_USR_USB_IN2];
            } else {
                return false;
            }
#if 0
            audio_src_srv_update_state(cu_ctx->pseudo_device_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
            __set_trans_state(cu_ctx, APP_AUDIO_TRANS_MGR_TRANS_STA_STARTING);
#endif
            audio_transmitter_start(cu_ctx->trans_id);
        }
        return ret;
    }
#endif
    app_audio_trans_mgr_interaction_event_t *ev = (app_audio_trans_mgr_interaction_event_t *)extra_data;

    if (ev == NULL) {
        return false;
    }

    app_audio_trans_mgr_usr_ctx_t *u_ctx = &s_ctx.usrs[ev->usr];

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_TRANSMITTER_EV:
            app_audio_trans_mgr_transmitter_event_proc(u_ctx, (audio_transmitter_event_t)ev->event);
            break;
        case APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_RESOURCE_CTRL_EV:
            app_audio_trans_mgr_judgement_event_proc(u_ctx, (audio_src_srv_resource_manager_event_t)ev->event);
            break;
        case APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD:
            app_audio_trans_mgr_self_cmd_proc(u_ctx, (app_audio_trans_mgr_cmd_t)ev->event);
            break;
        case APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_PSEUDO_DEV_EV:
            app_audio_trans_mgr_pseudo_event_proc(u_ctx, (app_audio_trans_mgr_pseudo_event_type_t)ev->event);
            break;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        case APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_AUD_SRC_CTRL_EV:
            audio_trans_mgr_audio_source_event_proc(u_ctx, ev->event);
            break;
#endif
    }

    return ret;
}
#endif

bool app_audio_trans_mgr_idle_activity_proc(struct _ui_shell_activity *self,
                                            uint32_t event_group,
                                            uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            if (event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE) {
                app_audio_trans_mgr_init();
                return true;
            }
            break;
        case EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANS_MGR:
            ret = app_audio_trans_mgr_event_proc(self, event_id, extra_data, data_len);
            break;
    }
    return ret;
}

void *app_audio_trans_mgr_register(app_audio_trans_mgr_usr_t *usr)
{
    if (usr == NULL || usr->type >= APP_AUDIO_TRANS_MGR_USR_MAX_NUM) {
        return NULL;
    }

    memset(&s_ctx.usrs[usr->type], 0, sizeof(app_audio_trans_mgr_usr_ctx_t));
    memcpy(&s_ctx.usrs[usr->type].usr_info, usr, sizeof(app_audio_trans_mgr_usr_t));
    return (void *) &s_ctx.usrs[usr->type];
}

void app_audio_trans_mgr_init_audio(void *usr)
{
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_INIT);
}

void app_audio_trans_mgr_start_audio(void *usr)
{
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_START);
}

void app_audio_trans_mgr_set_volume(void *usr, uint8_t l, uint8_t r)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)usr;
    if (u_ctx == NULL) {
        return;
    }

    u_ctx->usr_info.volume_l = l;
    u_ctx->usr_info.volume_r = r;
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_SET_VOLUME);
}

void app_audio_trans_mgr_set_db(void* usr, int32_t l, int32_t r) {
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)usr;
    if (u_ctx == NULL) {
        return;
    }

    u_ctx->usr_info.volume_db_l = l;
    u_ctx->usr_info.volume_db_r = r;
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_SET_VOLUME);
}

void app_audio_trans_mgr_set_mix_ratio(void *usr, uint8_t ratio)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)usr;
    if (u_ctx == NULL) {
        return;
    }

    u_ctx->usr_info.mix_ratio = ratio;
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_SET_MIX_RATIO);
}

void app_audio_trans_mgr_set_mute(void *usr, uint8_t mute)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *)usr;
    if (u_ctx == NULL) {
        return;
    }

    u_ctx->usr_info.mute = mute;
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_SET_MUTE);
}

void app_audio_trans_mgr_stop_audio(void *usr)
{
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_STOP);
}

void app_audio_trans_mgr_deinit_audio(void *usr)
{
    __send_self_cmd((app_audio_trans_mgr_usr_ctx_t *)usr, APP_AUDIO_TRANS_MGR_CMD_DEINIT);
}

bool app_audio_trans_mgr_wired_audio_playing()
{
    for (uint32_t i = 0; i < APP_AUDIO_TRANS_MGR_USR_MAX_NUM; i++) {
        if (s_ctx.usrs[i].state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING) {
            return true;
        }
    }
    return false;
}

/* workaround for a2dp suspend xx out. */
void app_audio_trans_mgr_stop_audio_unsafe(void *usr)
{
    app_audio_trans_mgr_usr_ctx_t *u_ctx = (app_audio_trans_mgr_usr_ctx_t *) usr;
    if (u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_STREAMING || u_ctx->state == APP_AUDIO_TRANS_MGR_TRANS_STA_IDLE) {
        __app_audio_trans_mgr_stop_audio(u_ctx);
    } else if (u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_STOPPING &&
               u_ctx->state != APP_AUDIO_TRANS_MGR_TRANS_STA_INIT) {
        u_ctx->cmd_queue[u_ctx->queued_cmd_nums++] = APP_AUDIO_TRANS_MGR_CMD_STOP;
    }
}
/* workaround over */

