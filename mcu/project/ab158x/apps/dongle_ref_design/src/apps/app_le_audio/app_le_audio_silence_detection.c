/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#include "bt_sink_srv_ami.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_common_nvkey_struct.h"
#include "app_le_audio_silence_detection.h"
#include "audio_log.h"
#include "FreeRTOS.h"
#include "bt_le_audio_msglog.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define AIR_LE_AUDIO_SILENCE_DETECTION_STOP_DELAY_TIMER_PERIOD    (293000) /* 4min53s */


/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/

extern app_le_audio_ucst_link_info_t g_lea_ucst_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];
extern app_le_audio_ctrl_t g_lea_ctrl;
extern app_le_audio_ucst_ctrl_t g_lea_ucst_ctrl;


/**************************************************************************************************
* Prototype
**************************************************************************************************/
static uint32_t app_le_audio_silence_detection_get_delay_stop_timer_period(void);
static void app_le_audio_silence_detection_stop_delay_timer_callback(TimerHandle_t timer_handle, void *user_data);
static void app_le_audio_silence_detection_handle_remote_device_bredr_connection_update(void *parameter);
static void app_le_audio_silence_detection_handle_disconnect_cis_for_silence_update(void *parameter);
static void app_le_audio_silence_detection_handle_port_disabled(void *parameter);
static bt_status_t app_le_audio_silence_detection_set_status_by_port(app_le_audio_stream_port_t port, app_le_audio_silence_detection_status_enum status);
static app_le_audio_silence_detection_status_enum app_le_audio_silence_detection_get_status_by_port(app_le_audio_stream_port_t port);
static bool app_le_audio_silence_detection_is_detecting_by_port(app_le_audio_stream_port_t port);
static void app_le_audio_silence_detection_callback(audio_scenario_type_t scenario_type, bool silence_flag);


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static uint32_t app_le_audio_silence_detection_get_delay_stop_timer_period(void)
{
    return g_lea_ctrl.silence_detection.delay_stop_timer_period;
}

static void app_le_audio_silence_detection_stop_delay_timer_callback(TimerHandle_t timer_handle, void *user_data)
{
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    LE_AUDIO_MSGLOG_I("[APP] sd timer expired handle:%x sd_mode:%x timer:%x", 3, timer_handle, silence_detection_mode, g_lea_ctrl.silence_detection.delay_stop_timer_handle);

    if (g_lea_ctrl.silence_detection.delay_stop_timer_handle) {
        /* The timer will be deleted after callback returns. */
        g_lea_ctrl.silence_detection.delay_stop_timer_handle = NULL;

        if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) {
            app_le_audio_ucst_stop(FALSE);
        }
    }
}

static void app_le_audio_silence_detection_handle_remote_device_bredr_connection_update(void *parameter)
{
    app_le_audio_silence_detection_status_enum silence_detection_status = app_le_audio_silence_detection_get_status();
    bool disconnect_cis = app_le_audio_silence_detection_disconnect_cis_for_silence();
    app_le_audio_ucst_ctrl_t *ucst_ctrl = app_le_audio_ucst_get_ctrl();
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    /* Only process the event when silence detection is on-going.
        * When currently data is detected, do nothing.
        * When currently silcence is detected, if disconnecting CIS is needed, disconnect CIS if CIS is not disconnected.
        * If disconnecting CIS is not needed, reconnect CIS if CIS is disconnected.
        */
    if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NONE != silence_detection_mode &&
        APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA == silence_detection_status) {
        if (disconnect_cis &&
            !app_le_audio_silence_detection_is_speical_silence_detection_ongoing()) {
            app_le_audio_ucst_stop(FALSE);
        } else if (!disconnect_cis &&
                   (app_le_audio_silence_detection_is_speical_silence_detection_ongoing() ||
                    APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == ucst_ctrl->next_target)) {
            app_le_audio_ucst_start();
        }
    }
}


static void app_le_audio_silence_detection_handle_disconnect_cis_for_silence_update(void *parameter)
{
    app_le_audio_silence_detection_handle_remote_device_bredr_connection_update(parameter);
}


static void app_le_audio_silence_detection_handle_port_disabled(void *parameter)
{
    app_le_audio_ucst_ctrl_t *ucst_ctrl = app_le_audio_ucst_get_ctrl();
    uint8_t streaming_port = app_le_audio_ucst_get_streaming_port();
    app_le_audio_stream_port_t port = (app_le_audio_usb_port_t)parameter;

    if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == ucst_ctrl->curr_target) {
        if (!streaming_port) {
            app_le_audio_ucst_stop(FALSE);
        } else {
            app_le_audio_silence_detection_stop_by_port(port);
            app_le_audio_stop_and_deinit_audio_transmitter(port);
        }
    }
}

static bt_status_t app_le_audio_silence_detection_set_status_by_port(app_le_audio_stream_port_t port, app_le_audio_silence_detection_status_enum status)
{
    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }
    g_lea_ctrl.silence_detection_status[port] = status;

    return BT_STATUS_SUCCESS;
}

/* Get detecting status */
static app_le_audio_silence_detection_status_enum app_le_audio_silence_detection_get_status_by_port(app_le_audio_stream_port_t port)
{
    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return APP_LE_AUDIO_SILENCE_DETECTION_STATUS_MAX;
    }
    return g_lea_ctrl.silence_detection_status[port];
}

static bool app_le_audio_silence_detection_is_detecting_by_port(app_le_audio_stream_port_t port)
{
    app_le_audio_silence_detection_status_enum port_status = app_le_audio_silence_detection_get_status_by_port(port);

    if (APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE == port_status ||
        APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA == port_status) {
        return TRUE;
    }

   return FALSE;
}

static void app_le_audio_silence_detection_callback(audio_scenario_type_t scenario_type, bool silence_flag)
{
    app_le_audio_stream_port_t port = APP_LE_AUDIO_STREAM_PORT_SPK_0, other_port = APP_LE_AUDIO_STREAM_PORT_SPK_1;
    app_le_audio_silence_detection_status_enum port_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_MAX;
    app_le_audio_silence_detection_status_enum other_port_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_MAX;
    app_le_audio_silence_detection_status_enum old_device_status = app_le_audio_silence_detection_get_status();
    app_le_audio_silence_detection_status_enum new_device_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_MAX;
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    if (AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0 != scenario_type &&
        AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1 != scenario_type) {
        LE_AUDIO_MSGLOG_W("[APP][SD] wrong scenario_type:%x", 1, scenario_type);
        return;
    }
#if 0//defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    //TBD:LINE_IN,I2S_IN silence detection check other port
#endif
    if (AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1 == scenario_type) {
        port = APP_LE_AUDIO_STREAM_PORT_SPK_1;
        other_port = APP_LE_AUDIO_STREAM_PORT_SPK_0;
    }

    if (!app_le_audio_silence_detection_is_detecting_by_port(port)) {
        LE_AUDIO_MSGLOG_W("[APP][SD] Not detecting:%x", 1, port);
        return;
    }

    /* Update silence detection status of port */
    if (silence_flag) {
        /* Silence detected */
        port_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA;
    } else {
        /* Data detected */
        port_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE;
    }
    app_le_audio_silence_detection_set_status_by_port(port, port_status);

    other_port_status = app_le_audio_silence_detection_get_status_by_port(other_port);

    /* Update silence detection status of device */
    if (APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA != other_port_status &&
        APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE != other_port_status) {
        /* Only one port starts silence detection. */
        app_le_audio_silence_detection_set_status(port_status);
    } else {
        if (APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA == port_status &&
            APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA == other_port_status) {
            /* Only when both ports detected silence, will the status of device be slience detected. */
            app_le_audio_silence_detection_set_status(APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA);
        } else {
            /* As long as at least one port detected data, the status of device will be data detected. */
            app_le_audio_silence_detection_set_status(APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE);
        }
    }

    new_device_status = app_le_audio_silence_detection_get_status();

    LE_AUDIO_MSGLOG_I("[APP][SD] s_flag:%x p:%x ps:%x o_p:%x o_ps:%x ds:%x->%x sd_mode:%x handle:%x", 9,
                      silence_flag, port, port_status, other_port, other_port_status,
                      old_device_status, new_device_status, silence_detection_mode,
                      g_lea_ctrl.silence_detection.delay_stop_timer_handle);

    // if (old_device_status != new_device_status)
    {
        if (APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA == new_device_status) {
            /* Silence detected */
            if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.next_target) {
                return;
            }

            if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) {
                if (app_le_audio_silence_detection_disconnect_cis_for_silence()) {
                    app_le_audio_ucst_stop(FALSE);
                } else {
                    uint32_t timer_period = app_le_audio_silence_detection_get_delay_stop_timer_period();

                    if (timer_period) {
                        /* Start a timer to stop non-special silence detection mode latter. */
                        if (!g_lea_ctrl.silence_detection.delay_stop_timer_handle) {
                            app_le_audio_timer_start(&g_lea_ctrl.silence_detection.delay_stop_timer_handle,
                                                     timer_period,
                                                     app_le_audio_silence_detection_stop_delay_timer_callback,
                                                     NULL);
                        }
                    } else {
                        /* Stop immediately. */
                        app_le_audio_ucst_stop(FALSE);
                    }
                }
            }
        } else if (APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE == new_device_status) {
            /* Data detected */
            app_le_audio_silence_detection_stop_delay_stop_timer();
            if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
                app_le_audio_ucst_start();
            }
        }
    }
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
app_le_audio_silence_detection_mode_enum app_le_audio_silence_detection_get_silence_detection_mode(void)
{
    app_le_audio_ucst_target_t curr_target = app_le_audio_ucst_get_curr_target();

    if (APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == curr_target ||
        APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE == curr_target) {
        return APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL;
    } else if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == curr_target ||
               APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE == curr_target) {
        return APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL;
    } else {
        return APP_LE_AUDIO_SILENCE_DETECTION_MODE_NONE;
    }
}

/* Set the silence detecting status of the device. No need to set status to IDLE. It will be updated automatically when getting status. */
void app_le_audio_silence_detection_set_status(app_le_audio_silence_detection_status_enum status)
{
    g_lea_ctrl.silence_detection.silence_detection_status = status;
}

app_le_audio_silence_detection_status_enum app_le_audio_silence_detection_get_status(void)
{
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    /* There is no time slot for normal silence detection changing to special silence detection. */
    if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NONE == silence_detection_mode &&
        APP_LE_AUDIO_SILENCE_DETECTION_STATUS_IDLE != g_lea_ctrl.silence_detection.silence_detection_status) {
        app_le_audio_silence_detection_set_status(APP_LE_AUDIO_SILENCE_DETECTION_STATUS_IDLE);
    }

    return g_lea_ctrl.silence_detection.silence_detection_status;
}

/*
TimerHandle_t app_le_audio_silence_detection_get_delay_stop_timer(void)
{
    return g_lea_ctrl.silence_detection.delay_stop_timer_handle;
}
*/

void app_le_audio_silence_detection_stop_delay_stop_timer(void)
{
    if (g_lea_ctrl.silence_detection.delay_stop_timer_handle) {
        app_le_audio_timer_stop(g_lea_ctrl.silence_detection.delay_stop_timer_handle);
        g_lea_ctrl.silence_detection.delay_stop_timer_handle = NULL;
    }
}

void app_le_audio_silence_detection_handle_event(app_le_audio_silence_detection_event_enum event_id, void *parameter)
{
    app_le_audio_ucst_ctrl_t *ucst_ctrl = app_le_audio_ucst_get_ctrl();
    uint8_t streaming_port = app_le_audio_ucst_get_streaming_port();

    LE_AUDIO_MSGLOG_I("[APP][SD] event_id:%x streaming_port:%x target:%x->%x", 4,
                      event_id,
                      streaming_port,
                      ucst_ctrl->curr_target,
                      ucst_ctrl->next_target);

    /* Process events using their handlers. */
    if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_REMOTE_DEVICE_BREDR_STATUS_UPDATE == event_id) {
        app_le_audio_silence_detection_handle_remote_device_bredr_connection_update(parameter);
        return;
    } else if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_DISCONNECT_CIS_FOR_SILENCE_UPDATE == event_id) {
        app_le_audio_silence_detection_handle_disconnect_cis_for_silence_update(parameter);
        return;
    } else if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_PORT_DISABLED == event_id) {
        app_le_audio_silence_detection_handle_port_disabled(parameter);
        return;
    }

    /* Process events which update curr_target and/or next_target and continue to process based on curr_target/next_target updated.  */
    if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_OTHER_MODE == event_id ||
        APP_LE_AUDIO_SILENCE_DETECTION_EVENT_STOP_ANY_MODE == event_id) {
        bool restart = FALSE;

        if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == ucst_ctrl->curr_target) {
            ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE;
        }

        if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_STOP_ANY_MODE == event_id) {
            restart = (bool)parameter;
        }

        ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_OTHER_MODE == event_id ||
            (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_STOP_ANY_MODE == event_id && restart)) {
            if (APP_LE_AUDIO_USB_PORT_MASK_MIC_0 & streaming_port) {
                ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE;
            }
            else {
                ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE;
            }
        }
    }
    else if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_SPECIAL_SILENCE_DETECTION == event_id) {
        /* Only start the special silence mode if there is any speaker USB port enabled and any LE link connected. */
        if (app_le_audio_ucst_get_link_num() &&
            (APP_LE_AUDIO_USB_PORT_MASK_SPK_0 & streaming_port ||
             APP_LE_AUDIO_USB_PORT_MASK_SPK_1 & streaming_port
#if 0//defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            || APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN & streaming_port
            || APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN & streaming_port
#endif
             )) {
            ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE;
        } else {
            ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        }
        ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
    } else if (APP_LE_AUDIO_SILENCE_DETECTION_EVENT_SPECIAL_SILENCE_DETECTION_STOPPED == event_id) {
        if (APP_LE_AUDIO_UCST_TARGET_NONE == ucst_ctrl->next_target ||
            APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == ucst_ctrl->next_target ||
            APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == ucst_ctrl->next_target) {
            ucst_ctrl->curr_target = ucst_ctrl->next_target;
        } else {
            /* Unexpected next_target. */
            ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        }
        ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
    }

    LE_AUDIO_MSGLOG_I("[APP][SD] target:%x->%x", 2, ucst_ctrl->curr_target, ucst_ctrl->next_target);

    /* Process based on curr_target updated above */
    if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == ucst_ctrl->curr_target) {
        /* Open audio transmitter if needed */
        if (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == ucst_ctrl->curr_stream_state &&
            APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == ucst_ctrl->next_stream_state) {
            /* Set silence detection mode first. Open audio transmitter will check the silence detection mode. */
            ucst_ctrl->next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM;
            if (BT_STATUS_SUCCESS != app_le_audio_open_audio_transmitter(FALSE, streaming_port)) {
                ucst_ctrl->next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
                return;
            }
        }
    } else if (APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE == ucst_ctrl->curr_target) {
        /* Close audio transmitter if needed */
        if (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE < ucst_ctrl->curr_stream_state &&
            APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING >= ucst_ctrl->curr_stream_state &&
            APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM != ucst_ctrl->next_stream_state) {
            ucst_ctrl->next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
            if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
                ucst_ctrl->next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
                ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE;
                return;
            }
        }
    } else if (APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == ucst_ctrl->curr_target ||
               APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == ucst_ctrl->curr_target) {
        ucst_ctrl->curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_BCST;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_le_audio_start_broadcast();
        } else {
            if (app_le_audio_ucst_check_pause_stream() ||
                ucst_ctrl->release) {
                return;
            }

            app_le_audio_ucst_start();
        }
    } else if (APP_LE_AUDIO_UCST_TARGET_NONE == ucst_ctrl->curr_target) {
        /* There should be no case of NONE curr_target with non-NONE next_target. */
        ucst_ctrl->next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_BCST;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_le_audio_start_broadcast();
        }
    }
}


/* silence detection mode is set after curr_target is set to APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE. */
bool app_le_audio_silence_detection_is_speical_silence_detection_ongoing(void)
{
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    return silence_detection_mode == APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL;
}

void app_le_audio_silence_detection_start_by_port(app_le_audio_stream_port_t port)
{
    app_le_audio_ucst_target_t curr_target = app_le_audio_ucst_get_curr_target();
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

    if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode ||
        APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
        LE_AUDIO_MSGLOG_I("[APP][SD] start silence detection port:%x sd_mode:%x", 2, port, silence_detection_mode);
        if (APP_LE_AUDIO_STREAM_PORT_SPK_0 == port) {
            audio_silence_detection_scenario_start(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0, app_le_audio_silence_detection_callback);
        } else if (APP_LE_AUDIO_STREAM_PORT_SPK_1 == port) {
            audio_silence_detection_scenario_start(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1, app_le_audio_silence_detection_callback);
        }
#if 0//defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        else if (APP_LE_AUDIO_STREAM_PORT_LINE_IN == port) {
            audio_silence_detection_scenario_start(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_XXX, app_le_audio_silence_detection_callback);
        } else if (APP_LE_AUDIO_STREAM_PORT_I2S_IN == port) {
            audio_silence_detection_scenario_start(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_XXX, app_le_audio_silence_detection_callback);
        }
#endif

        //if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) {
        if (APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == curr_target) {
            app_le_audio_silence_detection_set_status_by_port(port, APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE);
        } else {
            app_le_audio_silence_detection_set_status_by_port(port, APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA);
        }
    }
}


void app_le_audio_silence_detection_stop_by_port(app_le_audio_stream_port_t port)
{
    if (app_le_audio_silence_detection_is_detecting_by_port(port)) {
        LE_AUDIO_MSGLOG_I("[APP][SD] stop silence detection port:%x", 1, port);
        if (APP_LE_AUDIO_STREAM_PORT_SPK_0 == port) {
            audio_silence_detection_scenario_stop(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0);
        } else if (APP_LE_AUDIO_STREAM_PORT_SPK_1 == port) {
            audio_silence_detection_scenario_stop(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1);
        }
#if 0//defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        else if (APP_LE_AUDIO_STREAM_PORT_LINE_IN == port) {
            audio_silence_detection_scenario_stop(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_XXX);
        } else if (APP_LE_AUDIO_STREAM_PORT_I2S_IN == port) {
            audio_silence_detection_scenario_stop(AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_XXX);
        }
#endif
        app_le_audio_silence_detection_set_status_by_port(port, APP_LE_AUDIO_SILENCE_DETECTION_STATUS_IDLE);
    }
}

void app_le_audio_silence_detection_init(void)
{
    nvkey_status_t nvkey_status = NVKEY_STATUS_OK;
    app_silence_detection_nvkey_struct silence_detection_nvkey = {0};
    uint32_t size = sizeof(app_silence_detection_nvkey_struct);

    nvkey_status = nvkey_read_data(NVID_APP_SILENCE_DETECTION, (uint8_t *)&silence_detection_nvkey, &size);
    if (NVKEY_STATUS_OK == nvkey_status) {
        g_lea_ctrl.silence_detection.delay_stop_timer_period = silence_detection_nvkey.delay_stop_timer_period;
    } else {
        g_lea_ctrl.silence_detection.delay_stop_timer_period = AIR_LE_AUDIO_SILENCE_DETECTION_STOP_DELAY_TIMER_PERIOD;
    }

    LE_AUDIO_MSGLOG_I("[APP][SD] period:%x status:%x size:%x", 3,
                      g_lea_ctrl.silence_detection.delay_stop_timer_period,
                      nvkey_status,
                      size);
}


void app_le_audio_silence_detection_handle_bt_off(void)
{
    if (g_lea_ctrl.silence_detection.delay_stop_timer_handle) {
        app_le_audio_timer_stop(g_lea_ctrl.silence_detection.delay_stop_timer_handle);
        g_lea_ctrl.silence_detection.delay_stop_timer_handle = NULL;
    }
    g_lea_ctrl.silence_detection.silence_detection_status = APP_LE_AUDIO_SILENCE_DETECTION_STATUS_IDLE;
}

bt_status_t app_le_audio_ucst_set_remote_device_bredr_connection_status(bool bredr_connected, bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = app_le_audio_ucst_get_link_info(handle);

    if (!p_info) {
        return BT_STATUS_FAIL;
    }

    p_info->remote_device_bredr_connected = bredr_connected;
    return BT_STATUS_SUCCESS;
}


bt_status_t app_le_audio_ucst_get_remote_device_bredr_connection_status(bool *bredr_connected, bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = app_le_audio_ucst_get_link_info(handle);

    if (!bredr_connected || !p_info) {
        return BT_STATUS_FAIL;
    }

    *bredr_connected = p_info->remote_device_bredr_connected;
    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_ucst_set_disconnect_cis_for_silence(bool disconnect_cis, bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = app_le_audio_ucst_get_link_info(handle);

    if (!p_info) {
        return BT_STATUS_FAIL;
    }

    p_info->disconnect_cis_for_silence = disconnect_cis;
    return BT_STATUS_SUCCESS;
}


bt_status_t app_le_audio_ucst_get_disconnect_cis_for_silence(bool *disconnect_cis, bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = app_le_audio_ucst_get_link_info(handle);

    if (!disconnect_cis || !p_info) {
        return BT_STATUS_FAIL;
    }

    *disconnect_cis = p_info->disconnect_cis_for_silence;
    return BT_STATUS_SUCCESS;
}


bool app_le_audio_silence_detection_is_remote_device_connected_bredr_device(void)
{
    /* bredr is considered to be connected if there is at least one remote device connected with any bredr device. */
    for (uint32_t i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i; i++) {
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            if (g_lea_ucst_link_info[i].remote_device_bredr_connected
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                && app_le_audio_ucst_is_active_group_by_handle(g_lea_ucst_link_info[i].handle)
#endif
                ) {
                return TRUE;
            }
        }
    }

    return FALSE;
}


bool app_le_audio_silence_detection_disconnect_cis_for_silence(void)
{
    uint32_t i = 0;
    bool remote_bredr_connected = app_le_audio_silence_detection_is_remote_device_connected_bredr_device();
    bool disconnect_cis = TRUE;

    /* If there is any bredr device connected to the remote device, disconnect CIS always to let A2DP be allowed
        * when silence is deteced in the dongle side.
        */
    if (!remote_bredr_connected) {
        /* If there is at least one device setting it to FALSE, do not disconnect CIS.
               * (The setting may be reported latter for the late connected LEA device).
               * The devices in the same coordinated set should have the same setting.
               */
        for (i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i; i++) {
            if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
                if (!g_lea_ucst_link_info[i].disconnect_cis_for_silence) {
                    disconnect_cis = FALSE;
                    break;
                }
            }
        }
    }

    /* If there is no bredr device connecting to the remote LEA device, the decision depends on the setting
        * of the remote device.
        */
    LE_AUDIO_MSGLOG_I("[APP][SD] disconnect cis for silence:%x", 1, disconnect_cis);
    return disconnect_cis;
}

#endif
#endif

