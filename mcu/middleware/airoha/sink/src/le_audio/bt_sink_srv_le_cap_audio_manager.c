/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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




#include "FreeRTOS.h"
#include "bt_type.h"
#include "bt_le_audio_sink.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_cap_audio_manager.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_sink_srv_le_volume.h"
#include "bt_sink_srv_le_volume_internal.h"
#include "ble_ascs.h"
#include "ble_bap.h"
#include "ble_bass.h"
#include "bt_sink_srv_ami.h"
#include "audio_src_srv.h"
#if defined(MTK_AVM_DIRECT)
#include "bt_avm.h"
#endif
#ifdef BT_SINK_DUAL_ANT_ENABLE
#include "bt_sink_srv_dual_ant.h"
#include "mux_ll_uart_latch.h"
#endif
#include "bt_le_audio_msglog.h"
#include "bt_le_audio_def.h"
#ifdef AIR_BT_SINK_MUSIC_ENABLE
#include "bt_sink_srv_music.h"
#endif
#include "bt_utils.h"
#include "bt_device_manager_le.h"
#include "bt_timer_external.h"
#include "bt_sink_srv_state_manager.h"

/**************************************************************************************************
* Define
**************************************************************************************************/

#if defined(__AFE_HS_DC_CALIBRATION__)
#define CAP_AUDIO_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HANDSET
#else
#define CAP_AUDIO_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HEADSET
#endif
#ifdef MTK_DEVELOPMENT_BOARD_HDK
#define CAP_AUDIO_INPUT_DEVICE HAL_AUDIO_DEVICE_MAIN_MIC
#else
#define CAP_AUDIO_INPUT_DEVICE HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC
#endif

#define CAP_INVALID_UINT8   0xFF
#define BROADCAST_MUSIC_RESUME  1

/**************************************************************************************************
* Enum
**************************************************************************************************/
enum {
    CAP_AM_STATE_OFF,
    CAP_AM_STATE_IDLE,
    CAP_AM_STATE_PREPARE_PLAY,
    CAP_AM_STATE_WAIT_PLAY,
    CAP_AM_STATE_PLAYING,
    CAP_AM_STATE_SUSPENDING,
    CAP_AM_STATE_SUSPEND_PREPARE_STOP,
    CAP_AM_STATE_SUSPEND_WAIT_STOP,
    CAP_AM_STATE_PREPARE_STOP,
    CAP_AM_STATE_WAIT_STOP,
};

enum {
    CAP_AM_SUB_STATE_TAKING_SPK_RES,
    CAP_AM_SUB_STATE_TAKING_MIC_RES,
};


/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct
{
    audio_src_srv_handle_t *p_handle;
    uint8_t state;
    bt_sink_srv_am_id_t cap_aid;
    uint8_t current_mode;
    uint8_t sub_state;
    bool stop_needed:1;
    bool deinit_needed:1;
    bool mcs_play_needed:1;
    bool resume_needed:1;
}cap_audio_manager_control_t;

typedef struct
{
    audio_src_srv_handle_t *handle;
    audio_src_srv_handle_t *int_hd;
}cap_audio_manager_suspend_callback_on_bt_task_data;


/**************************************************************************************************
* Variables
**************************************************************************************************/
static cap_audio_manager_control_t g_cap_am_ctl[CAP_MAX_DEVICE_NUM];
static bt_sink_srv_cap_am_mode g_cap_am_current_mode = CAP_INVALID_UINT8;
static bt_sink_srv_cap_am_mode g_cap_am_reject_mode = CAP_INVALID_UINT8;
//static TimerHandle_t g_cap_am_reject_timer = NULL;
static bt_sink_srv_cap_am_mode g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
static bt_sink_srv_cap_am_mode g_cap_am_restarting_dsp_mode = CAP_INVALID_UINT8;
static bt_sink_srv_cap_am_mode g_cap_am_switch_suspending_mode = CAP_INVALID_UINT8;
bool g_cap_am_switch_suspending = false;

bool g_cap_am_local_mute = 0;/*To record mute state set by upper layer, priority: APP > VCS*/

bool g_cap_am_streaming_with_conversational_context = 0;/*To record DSP streaming context*/

cap_audio_manager_suspend_callback_on_bt_task_data g_cap_am_suspend_callback_on_bt_task = {NULL, NULL};

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static void bt_sink_srv_cap_am_para_init(void);
static void bt_sink_srv_cap_am_function_init(void);
static void bt_sink_srv_cap_am_callback_register(void);
static void bt_sink_srv_cap_am_callback_deregister(bt_sink_srv_cap_am_mode mode);
static void bt_sink_srv_cap_am_action_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
static void bt_sink_srv_cap_am_play_callback(audio_src_srv_handle_t *handle);
static void bt_sink_srv_cap_am_stop_callback(audio_src_srv_handle_t *handle);
static void bt_sink_srv_cap_am_suspend_callback(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_sink_srv_cap_am_reject_callback(audio_src_srv_handle_t *handle);
static void bt_sink_srv_cap_am_exception_callback(audio_src_srv_handle_t *handle, int32_t event, void *param);
static void bt_sink_srv_cap_am_send_event_and_set_state(bt_sink_srv_cap_am_mode mode, audio_src_srv_event_t event_id);
static void bt_sink_srv_cap_am_set_state(bt_sink_srv_cap_am_mode mode, uint8_t state);
static uint8_t bt_sink_srv_cap_am_get_state(bt_sink_srv_cap_am_mode mode);
static bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_mode_by_audio_id(bt_sink_srv_am_id_t aud_id);
bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_mode_by_audio_handle(audio_src_srv_handle_t *p_handle);
static bool bt_sink_srv_cap_am_reject_timer_start(bt_sink_srv_cap_am_mode mode);
static void bt_sink_srv_cap_am_reject_timer_callback(uint32_t timer_id, uint32_t data);
static bool bt_sink_srv_cap_am_reject_timer_stop(void);
static uint8_t bt_sink_srv_cap_am_convert_sampling_freq(uint8_t config_freq);
static void bt_sink_srv_cap_am_suspend_callback_timer_start(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_sink_srv_cap_am_suspend_callback_timer_callback(uint32_t timer_id, uint32_t data);

#if 1 //def BT_SINK_MUSIC_NEW_VP_INTERRUPT_SOLUTION
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
static bool bt_sink_srv_cap_am_resume_timer_start(bt_sink_srv_cap_am_mode mode, uint32_t duration_ms);
#endif
static bool bt_sink_srv_cap_am_resume_timer_stop(bt_sink_srv_cap_am_mode mode);
#endif

extern bt_status_t le_audio_sink_inform_app(bt_le_audio_sink_event_t event, void *msg);
extern void le_sink_srv_set_streaming_state(bool is_streaming);
#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
extern void bt_ble_set_special_device(bool is_special);
#endif
extern bool bt_sink_srv_call_get_sidetone_enable_config(void);

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void bt_sink_srv_cap_am_para_init(void)
{
#ifdef AIR_BT_CODEC_BLE_ENABLED
    memset(&g_cap_am_ctl, 0, sizeof(g_cap_am_ctl));
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        g_cap_am_ctl[i].cap_aid = AUD_ID_INVALID;
        g_cap_am_ctl[i].p_handle = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE);
        g_cap_am_ctl[i].p_handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE;
        g_cap_am_ctl[i].p_handle->dev_id = 0x0;
        g_cap_am_ctl[i].p_handle->priority = ((i <= CAP_AM_UNICAST_CALL_MODE_END) ? AUDIO_SRC_SRV_PRIORITY_HIGH : AUDIO_SRC_SRV_PRIORITY_NORMAL);
    }
#endif
}

static void bt_sink_srv_cap_am_function_init(void)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (!g_cap_am_ctl[i].p_handle) {
            le_audio_log("[CAP][AM] function init FAIL, mode:%d", 1, i);
            bt_utils_assert(0);
            return;
        }

        g_cap_am_ctl[i].p_handle->play             = bt_sink_srv_cap_am_play_callback;
        g_cap_am_ctl[i].p_handle->stop             = bt_sink_srv_cap_am_stop_callback;
        g_cap_am_ctl[i].p_handle->suspend          = bt_sink_srv_cap_am_suspend_callback_timer_start;
        g_cap_am_ctl[i].p_handle->reject           = bt_sink_srv_cap_am_reject_callback;
        g_cap_am_ctl[i].p_handle->exception_handle = bt_sink_srv_cap_am_exception_callback;
    }
}

static void bt_sink_srv_cap_am_callback_register(void)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if(g_cap_am_ctl[i].cap_aid != AUD_ID_INVALID)
        {
            bt_sink_srv_ami_audio_close(g_cap_am_ctl[i].cap_aid);
        }

        g_cap_am_ctl[i].cap_aid = bt_sink_srv_ami_audio_open(((i <= CAP_AM_UNICAST_CALL_MODE_END) ? AUD_HIGH : AUD_MIDDLE), bt_sink_srv_cap_am_action_callback);
        le_audio_log("[CAP][AM] register callback, index:%d, aid:%d", 2, i, g_cap_am_ctl[i].cap_aid);

        g_cap_am_ctl[i].sub_state = CAP_INVALID_UINT8;
        bt_sink_srv_cap_am_send_event_and_set_state(i, AUDIO_SRC_SRV_EVT_READY);
    }
}

static void bt_sink_srv_cap_am_callback_deregister(bt_sink_srv_cap_am_mode mode)
{
    if (g_cap_am_ctl[mode].cap_aid != AUD_ID_INVALID) {
        bt_sink_srv_ami_audio_close(g_cap_am_ctl[mode].cap_aid);
        g_cap_am_ctl[mode].cap_aid = AUD_ID_INVALID;
    }
    audio_src_srv_destruct_handle(g_cap_am_ctl[mode].p_handle);
    le_audio_log("[CAP][AM] deregister callback, mode:%d", 1, mode);
}

static void bt_sink_srv_cap_am_action_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_id(aud_id);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    bool ul_only = bt_sink_srv_cap_stream_is_source_ase_only(conn_handle, true);

#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
    bt_ble_set_special_device(false);
#endif
        le_audio_log("[CAP][AM] action callback, mode:%d, msg:%d, sub_msg:%d, state:%d, need stop:%d, need deinit:%d, ul_only:%d", 7,
        mode, msg_id, sub_msg, state, g_cap_am_ctl[mode].stop_needed, g_cap_am_ctl[mode].deinit_needed, ul_only);

    bt_sink_srv_mutex_lock();

    switch (msg_id) {
        case AUD_SINK_OPEN_CODEC: //DSP on
            le_audio_log("[CAP][AM] AUD_SINK_OPEN_CODEC", 0);

            if (mode == g_cap_am_switch_suspending_mode) {
                le_audio_log("[CAP][AM] switch to resuming", 0);
                g_cap_am_switch_suspending_mode = CAP_INVALID_UINT8;
                if (mode <= CAP_AM_UNICAST_CALL_MODE_END && !ul_only) {
                    bt_sink_srv_cap_am_lea_control_sidetone(true);
                }
                bt_sink_srv_cap_stream_restarting_complete_response(mode);
                break;
            } else if (mode == g_cap_am_restarting_dsp_mode) {
                le_audio_log("[CAP][AM] DSP restarting finished", 0);
                g_cap_am_restarting_dsp_mode = CAP_INVALID_UINT8;
                if (mode <= CAP_AM_UNICAST_CALL_MODE_END && !ul_only) {
                    bt_sink_srv_cap_am_lea_control_sidetone(true);
                }
                bt_sink_srv_cap_stream_restarting_complete_response(mode);
                break;
            }

            if (state == CAP_AM_STATE_WAIT_PLAY) {
#ifdef AIR_LE_AUDIO_CIS_ENABLE
                if (mode <= CAP_AM_UNICAST_MODE_END) {
                    /*Unicast*/
                    if (sub_msg == AUD_CMD_FAILURE || g_cap_am_ctl[mode].stop_needed || g_cap_am_ctl[mode].deinit_needed) {
                        //bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                        g_cap_am_ctl[mode].resume_needed = false;
                        /*bt_sink_srv_cap_am_set_state(mode,
                            (resume ? CAP_AM_STATE_SUSPEND_WAIT_STOP : CAP_AM_STATE_PREPARE_STOP));
                        g_cap_am_ctl[mode].resume_needed = false;

                        if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                            if (resume) {
                                audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);
                            }
                        } else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
                            le_sink_srv_set_streaming_state(false);
                        }*/

                        le_audio_log("[CAP][AM] enabling_response first ase_id", 0);
#if (RELEASE_LE_MUSIC_WHEN_INTERRUPT)
                        bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);
#else
                        bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
#endif
                        if(le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                            //respond 2nd ASE Characteristic
#if (RELEASE_LE_MUSIC_WHEN_INTERRUPT)
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, false,
                                (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));
#else
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, true,
                                (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));
#endif
                        }

                        if (mode <= CAP_AM_UNICAST_CALL_MODE_END && !ul_only) {
                            //respond both ASE Characteristic
                            le_audio_log("[CAP][AM] enabling_response source ase_id", 0);
#if (RELEASE_LE_CALL_WHEN_INTERRUPT)
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
#else
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
#endif
                        } else if (mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
                            bt_le_audio_sink_action_param_t le_param = {
                                .service_idx = 0xFF,
                            };
                            bt_le_audio_sink_send_action(conn_handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PAUSE, &le_param);
                        }

                    } else if (sub_msg == AUD_HFP_OPEN_CODEC_DONE) {
                        //bt_clock_t play_clk = {0};

                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PLAYING);

                        //bt_sink_srv_ami_audio_set_mute(aud_id, (volume_state.mute ? true : false), STREAM_OUT);

                        bt_sink_srv_cap_stream_set_service_ble_link(conn_handle);
                        g_cap_am_current_mode = mode;
                        bt_sink_srv_le_volume_state_t volume_state = bt_sink_srv_le_volume_get_state(conn_handle, mode);
                        bt_sink_srv_le_volume_set_volume(BT_SINK_SRV_LE_STREAM_TYPE_OUT, volume_state.volume);
                        bt_sink_srv_le_volume_set_mute(BT_SINK_SRV_LE_STREAM_TYPE_OUT, volume_state.mute);
                        #if BT_SINK_LE_MIC_VOLUME_ADJUST_ENABLE
                        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                            bt_sink_srv_le_volume_state_t mic_vol = bt_sink_srv_le_volume_get_mic_volume_state(conn_handle);
                            bt_sink_srv_le_volume_set_mute_and_volume_level(g_cap_am_ctl[mode].cap_aid, BT_SINK_SRV_LE_STREAM_TYPE_IN, mic_vol.mute, mic_vol.volume);
                        }
                        #endif

                        if (mode == g_cap_am_restarting_psedev_mode) {
                            /* Restart AM success, no need to send ASE notification, just set data path */
                            bt_sink_srv_cap_stream_restarting_complete_response(mode);
                            g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
                            break;
                        }

                        //bt_sink_srv_cap_stream_am_timer_start();
                        le_audio_log("[CAP][AM] enabling_response first ase_id", 0);
                        bt_sink_srv_cap_stream_enabling_response(conn_handle, true,
                            (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));

                        if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                            //respond 2nd ASE Characteristic
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
                        }

                        if (mode <= CAP_AM_UNICAST_CALL_MODE_END && !ul_only) {
                            //respond both ASE Characteristic
                            bt_sink_srv_cap_am_lea_control_sidetone(true);
                            bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
                        }
                        bt_sink_srv_cap_stream_set_all_cis_data_path(conn_handle);
                    } else {
                        bt_utils_assert(0);
                    }
                }
#endif

                if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {
                    /*Broadcast*/
                    if (sub_msg == AUD_CMD_FAILURE || g_cap_am_ctl[mode].stop_needed || g_cap_am_ctl[mode].deinit_needed) {
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                        bt_sink_srv_cap_stream_broadcast_enabling_response(false);

                    } else if (sub_msg != AUD_CMD_FAILURE) {
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PLAYING);
                        g_cap_am_current_mode = mode;

                        if (mode == g_cap_am_restarting_psedev_mode) {
                            /* Restart AM success, set BIS data path or create BIG sync */
                            bt_sink_srv_cap_stream_restarting_complete_response(mode);
                            g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
                            break;
                        }
                        bt_sink_srv_cap_stream_broadcast_enabling_response(true);
                    }
                }
            }
            break;

        case AUD_SELF_CMD_REQ: {//DSP off
            le_audio_log("[CAP][AM] AUD_SELF_CMD_REQ", 0);
#if 0 //To inform APP that LE audio streaming stop, APP may need this event to apply different DSP algorithms.
			bt_sink_srv_lea_streaming_stop_t notify = {0};
            notify.handle = conn_handle;
            bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_LE_LEA_STREAMING_STOP, &notify, sizeof(bt_sink_srv_lea_streaming_stop_t));
#endif
            g_cap_am_streaming_with_conversational_context = false;

            if (mode == g_cap_am_switch_suspending_mode) {
                le_audio_log("[CAP][AM] switch to suspending", 0);
                break;
            } else if (mode == g_cap_am_restarting_dsp_mode) {
                le_audio_log("[CAP][AM] DSP restarting", 0);
                bt_sink_srv_cap_am_play_callback(g_cap_am_ctl[mode].p_handle);
                break;
            }

            if (state == CAP_AM_STATE_WAIT_STOP) {
#ifdef AIR_LE_AUDIO_CIS_ENABLE
                if (mode <= CAP_AM_UNICAST_MODE_END) {
                    /*Unicast*/
                    if (sub_msg == AUD_CMD_COMPLETE) {
                        //bt_sink_srv_cap_stream_reset_avm_buffer();
#ifdef BT_SINK_DUAL_ANT_ENABLE
#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE
                        bt_sink_srv_cap_am_take_give_resource(mode, false);
#endif
#endif
                        bt_sink_srv_le_volume_set_mute_ex(BT_SINK_SRV_LE_STREAM_TYPE_OUT, 0, false);
                        g_cap_am_current_mode = CAP_INVALID_UINT8;
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                        bt_sink_srv_cap_stream_clear_service_ble_link(conn_handle);


                        if (g_cap_am_ctl[mode].deinit_needed) {
                            g_cap_am_ctl[mode].deinit_needed = false;
                            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
                            bt_sink_srv_cap_am_callback_deregister(mode);
                        } else if (mode == g_cap_am_restarting_psedev_mode) {
                            /* Restart AM without sending ASE notification */
                            bt_sink_srv_cap_am_audio_start(mode);
                            break;
                        }

                        bt_sink_srv_cap_stream_disabling_response_all(conn_handle, true,
                            (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));
                        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                            bt_sink_srv_cap_stream_disabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
                        }
                    } else {
                        bt_sink_srv_cap_stream_disabling_response_all(conn_handle, false,
                            (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));

                        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                            bt_sink_srv_cap_stream_disabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
                        }
                    }
                }
#endif

                if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {
                    /*Broadcast*/
                    bt_sink_srv_cap_stream_service_big_t *service_big = bt_sink_srv_cap_stream_get_service_big();
                    //bt_sink_srv_ami_audio_set_mute(aud_id, false, STREAM_OUT);
                    //bt_sink_srv_cap_stream_reset_avm_buffer();
                    g_cap_am_current_mode = CAP_INVALID_UINT8;
                    bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);

                    if (g_cap_am_ctl[mode].deinit_needed) {
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
                        bt_sink_srv_cap_am_callback_deregister(mode);
                        g_cap_am_ctl[mode].deinit_needed = false;
                    } else if (mode == g_cap_am_restarting_psedev_mode) {
                        /* Restart AM without sending ASE notification */
                        bt_sink_srv_cap_am_audio_start(mode);
                        break;
                    }

                    if ((NULL != service_big) && (service_big->big_handle)) {
                        bt_sink_srv_cap_stream_broadcast_disabling_response(true);
                        bt_sink_srv_cap_stream_clear_service_big(service_big->big_handle);
                    }
                }

            } else if (state == CAP_AM_STATE_SUSPEND_WAIT_STOP && sub_msg == AUD_CMD_COMPLETE) {
                //bt_sink_srv_cap_stream_reset_avm_buffer();
#ifdef BT_SINK_DUAL_ANT_ENABLE
#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE
                bt_sink_srv_cap_am_take_give_resource(mode, false);
#endif
#endif
                bt_sink_srv_le_volume_set_mute_ex(BT_SINK_SRV_LE_STREAM_TYPE_OUT, 0, false);
                g_cap_am_current_mode = CAP_INVALID_UINT8;
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);

#ifdef AIR_LE_AUDIO_CIS_ENABLE
                if (mode <= CAP_AM_UNICAST_MODE_END && conn_handle != BT_HANDLE_INVALID) {
                    bt_sink_srv_cap_stream_disabling_response_all(conn_handle, true,
                        (ul_only ? AUDIO_DIRECTION_SOURCE : AUDIO_DIRECTION_SINK));
                    if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                        bt_sink_srv_cap_stream_disabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
                    }
                    //bt_sink_srv_cap_stream_clear_service_ble_link(conn_handle);
                }
#endif

                if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {
                    bt_sink_srv_cap_stream_service_big_t *service_big = bt_sink_srv_cap_stream_get_service_big();
                    if ((NULL != service_big) && (service_big->big_handle)) {
                        bt_sink_srv_cap_stream_broadcast_disabling_response(true);
                        bt_sink_srv_cap_stream_clear_service_big(service_big->big_handle);
                    }
                }

            } else if (state == CAP_AM_STATE_WAIT_PLAY && sub_msg == AUD_CMD_COMPLETE) {
                //bt_sink_srv_cap_stream_reset_avm_buffer();
#ifdef BT_SINK_DUAL_ANT_ENABLE
#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE
                bt_sink_srv_cap_am_take_give_resource(mode, false);
#endif
#endif
                bt_sink_srv_le_volume_set_mute_ex(BT_SINK_SRV_LE_STREAM_TYPE_OUT, 0, false);
                g_cap_am_current_mode = CAP_INVALID_UINT8;
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
                bt_sink_srv_cap_am_callback_deregister(mode);
            }
            break;
        }
        default:
            break;
    }

    if (mode < CAP_MAX_DEVICE_NUM) {
        g_cap_am_ctl[mode].stop_needed = false;
    }
    bt_sink_srv_mutex_unlock();
}

static void bt_sink_srv_cap_am_play_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    bt_sink_srv_cap_stream_config_info_t config_info = {{0}}, ul_config = {{0}};
    bool ul_only = bt_sink_srv_cap_stream_is_source_ase_only(conn_handle, true);
    config_info = bt_sink_srv_cap_stream_get_config_info(mode, AUDIO_DIRECTION_SINK);
    ble_bap_ase_id_list_t ase_list = bt_sink_srv_cap_stream_find_streaming_ase_id_list(conn_handle, true);
    bt_le_audio_direction_t direction = bt_sink_srv_cap_stream_find_streaming_ase_direction(conn_handle, true);
    bt_sink_srv_cap_stream_service_big_t *service_big = bt_sink_srv_cap_stream_get_service_big();

    le_audio_log("[CAP][AM] play callback, mode:%d, state:%d, priority:%d, switch_suspending:%d, switch_suspending_mode:%d, dsp_restarting_mode:%d",
            6, mode, state, g_cap_am_ctl[mode].p_handle->priority, g_cap_am_switch_suspending, g_cap_am_switch_suspending_mode, g_cap_am_restarting_dsp_mode);

    //To reset priority
    g_cap_am_ctl[mode].p_handle->priority = ((mode <= CAP_AM_UNICAST_CALL_MODE_END) ? AUDIO_SRC_SRV_PRIORITY_HIGH : AUDIO_SRC_SRV_PRIORITY_NORMAL);

    if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
        g_cap_am_ctl[mode].p_handle->priority = AUDIO_SRC_SRV_PRIORITY_HIGH;
    } else if (mode < CAP_AM_MODE_NUM) {
        g_cap_am_ctl[mode].p_handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
    }

#ifdef AIR_BT_CODEC_BLE_ENABLED
    if (g_cap_am_switch_suspending && g_cap_am_switch_suspending_mode != mode && mode < CAP_AM_MODE_NUM) {
        /*Just respond without play DSP*/
        le_audio_log("[CAP][AM] play callback in switch suspending state", 0);

        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_WAIT_PLAY);
        bt_sink_srv_cap_am_action_callback(g_cap_am_ctl[mode].cap_aid, AUD_SINK_OPEN_CODEC, AUD_HFP_OPEN_CODEC_DONE, NULL);
        g_cap_am_switch_suspending_mode = mode;
        /*if (mode <= CAP_AM_UNICAST_MUSIC_MODE_1) {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PLAYING);
            bt_sink_srv_cap_stream_set_service_ble_link(conn_handle);
            g_cap_am_current_mode = mode;

            le_audio_log("[CAP][AM] enabling_response first ase_id", 0);
            bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);

            if(le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                //respond 2nd ASE Characteristic
                le_audio_log("[CAP][AM] enabling_response second ase_id", 0);
                bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
            }

            if (mode <= CAP_AM_UNICAST_CALL_MODE_1) {
                le_audio_log("[CAP][AM] enabling_response source ase_id", 0);
                bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
            }
        }*/
        return;
    }

#ifdef BT_SINK_DUAL_ANT_ENABLE
#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE
    if ((mode <= CAP_AM_UNICAST_CALL_MODE_END || ul_only) && g_cap_am_ctl[mode].sub_state == CAP_INVALID_UINT8) {
        /*Call mode, wait event*/
        bt_sink_srv_cap_am_take_give_resource(mode, true);
        return;
    } else {
        bt_sink_srv_cap_am_take_give_resource(mode, true);
    }
#endif
#endif

    if (state == CAP_AM_STATE_SUSPENDING) {

        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
        bt_sink_srv_config_t config ;
        bt_sink_srv_state_manager_get_reject_config_t get_resume_config;
        memset(&config, 0, sizeof(bt_sink_srv_config_t));
        memset(&get_resume_config, 0, sizeof(bt_sink_srv_state_manager_get_resume_config_t));
        get_resume_config.type = BT_SINK_SRV_DEVICE_LE;

        if (conn_info != NULL) {
            bt_sink_srv_memcpy(&(get_resume_config.address),&(conn_info->peer_addr.addr),sizeof(bt_bd_addr_t));
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_RESUME_CONFIG, (void*)(&get_resume_config), &config);
#endif
        }

        le_audio_log("[CAP][AM] resume, resume_operation:%x", 1, config.resume_config.resume_operation);

        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            /*Call resume*/

            if (config.resume_config.resume_operation & BT_SINK_SRV_INTERRUPT_OPERATION_UNHOLD_CALL) {
                bt_sink_srv_cap_stream_retrieve_call(conn_handle);
            }

            if (ase_list.num_of_ase && direction == AUDIO_DIRECTION_SOURCE) {
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_PLAY);
                state = CAP_AM_STATE_PREPARE_PLAY;
            } else {
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_STOP);
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                return;
            }

        } else {
            /*Music resume*/
            if (mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {

#if 1//def BT_SINK_MUSIC_NEW_VP_INTERRUPT_SOLUTION
                    bt_sink_srv_cap_am_resume_timer_stop(mode);
#endif
                    /*TO DO MCP PLAY*/
                    if (config.resume_config.resume_operation & BT_SINK_SRV_INTERRUPT_OPERATION_PLAY_MUSIC) {
                        if (g_cap_am_ctl[mode].mcs_play_needed) {
                            g_cap_am_ctl[mode].mcs_play_needed = false;
                            bt_le_audio_sink_action_param_t le_param = {
                                .service_idx = BLE_MCP_GMCS_INDEX,
                            };
                            bt_le_audio_sink_send_action(conn_handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PLAY, &le_param);
                        }
                    }

	                le_audio_sink_inform_app(BT_LE_AUDIO_SINK_EVENT_MEDIA_RESUME, &conn_handle);

#if (RELEASE_LE_MUSIC_WHEN_INTERRUPT)
                    bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_STOP);
                    bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                    return;
#else
                    if (ase_list.num_of_ase && direction == AUDIO_DIRECTION_SINK) {
                        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_PLAY);
                        state = CAP_AM_STATE_PREPARE_PLAY;
                    } else {
                        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_STOP);
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                        return;
                    }
#endif
            } else {

                    bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_STOP);
                    bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
#if (BROADCAST_MUSIC_RESUME)
                    bt_sink_srv_cap_stream_bmr_scan_param_ex_t scan_param = {0};
                    bt_addr_t temp_addr = {0};
                    bt_sink_srv_cap_stream_get_default_bmr_scan_info(&scan_param);

                    if (memcmp(&temp_addr, &scan_param.bms_address, sizeof(bt_addr_t))) {
                        scan_param.duration = DEFAULT_SCAN_TIMEOUT;
                        scan_param.bis_sync_state = BT_BASS_BIS_SYNC_NO_PREFERENCE;
                        scan_param.scan_type= BT_HCI_SCAN_FILTER_ACCEPT_ONLY_ADVERTISING_PACKETS_IN_WHITE_LIST;
                        bt_sink_srv_cap_stream_scan_broadcast_source_ex(&scan_param);
                    }
#endif
                return;
            }

        }
    }

    if(handle->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE && (state == CAP_AM_STATE_PREPARE_PLAY ||
        (state == CAP_AM_STATE_PLAYING && (g_cap_am_switch_suspending_mode == mode || g_cap_am_restarting_dsp_mode == mode))))
    {
        bt_sink_srv_cap_am_reject_timer_stop();
        uint8_t codec_id[AUDIO_CODEC_ID_SIZE] = CODEC_ID_LC3;
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        uint8_t lc3plus_cbr_id[AUDIO_CODEC_ID_SIZE] = CODEC_ID_LC3PLUS_CBR;
        uint8_t lc3plus_vbr_id[AUDIO_CODEC_ID_SIZE] = CODEC_ID_LC3PLUS_VBR;
#endif
        bt_sink_srv_am_audio_capability_t aud_cap = {0};
        bt_sink_srv_am_result_t am_result = AUD_EXECUTION_FAIL;
        uint32_t context_type = AUDIO_CONTENT_TYPE_UNSPECIFIED;
        le_audio_log("[CAP][AM] config_info [sdu_interval %x %x %x]", 3, config_info.sdu_interval[0], config_info.sdu_interval[1], config_info.sdu_interval[2]);

        uint8_t *p_preferred_audio_contexts = ble_ascs_get_ltv_value_from_metadata(METADATA_TYPE_PREFERRED_AUDIO_CONTEXTS, config_info.metadata_length, config_info.metadata);
        if (NULL != p_preferred_audio_contexts){
            context_type = p_preferred_audio_contexts[0] |(p_preferred_audio_contexts[1] << 8);
        }

        uint8_t *p_streaming_audio_contexts = ble_ascs_get_ltv_value_from_metadata(METADATA_TYPE_STREAMING_AUDIO_CONTEXTS, config_info.metadata_length, config_info.metadata);
        if (NULL != p_streaming_audio_contexts){
            context_type = p_streaming_audio_contexts[0] |(p_streaming_audio_contexts[1] << 8);
        }
#ifdef AIR_BT_LATENCY_TEST_MODE_ENABLE
        uint8_t *p_vendor_specific = ble_ascs_get_ltv_value_from_metadata(METADATA_TYPE_VENDOR_SPECIFIC, config_info.metadata_length, config_info.metadata);
        if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END &&
            NULL != p_vendor_specific && ((p_vendor_specific[0]|(p_vendor_specific[1] << 8)) == 0x46)) {
            /*WALT Mode*/
            bt_ble_set_special_device(true);
        }
#endif
        if (context_type & AUDIO_CONTENT_TYPE_CONVERSATIONAL) {
            g_cap_am_streaming_with_conversational_context = true;;
        }

        aud_cap.codec.ble_format.ble_codec.context_type = context_type;
        le_audio_log("[CAP][AM] context_type:%d", 1, context_type);

        bt_sink_srv_le_volume_state_t volume_state = {0};
#if BT_SINK_LE_MIC_VOLUME_ADJUST_ENABLE
#ifdef AIR_LE_AUDIO_CIS_ENABLE
        bt_sink_srv_le_volume_state_t mic_vol = {0};
        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            mic_vol = bt_sink_srv_le_volume_get_mic_volume_state(conn_handle);
        }
#endif
#endif
        if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {
            volume_state.volume = bt_sink_srv_cap_stream_get_broadcast_volume();
            volume_state.mute = bt_sink_srv_cap_stream_is_broadcast_mute();
        } else {
#ifdef AIR_LE_AUDIO_CIS_ENABLE
            volume_state = bt_sink_srv_le_volume_get_state(conn_handle, mode);
#endif

            if (mode <= CAP_AM_UNICAST_CALL_MODE_END || ul_only ) {
                /*Call mode/UL only, get configuration for UL*/
                ul_config = bt_sink_srv_cap_stream_get_config_info(mode, AUDIO_DIRECTION_SOURCE);
                uint32_t ul_context_type = AUDIO_CONTENT_TYPE_UNSPECIFIED;

                aud_cap.codec.ble_format.ble_codec.ul_sample_rate = bt_sink_srv_cap_am_convert_sampling_freq(ul_config.sampling_frequency);
                aud_cap.codec.ble_format.ble_codec.ul_channel_num = CHANNEL_MONO;
                aud_cap.codec.ble_format.ble_codec.ul_frame_payload_length = ul_config.frame_payload_length;

                uint8_t *p_ul_preferred_audio_contexts = ble_ascs_get_ltv_value_from_metadata(METADATA_TYPE_PREFERRED_AUDIO_CONTEXTS, ul_config.metadata_length, ul_config.metadata);
                if (NULL != p_ul_preferred_audio_contexts){
                    ul_context_type = p_ul_preferred_audio_contexts[0] |(p_ul_preferred_audio_contexts[1] << 8);
                }

                uint8_t *p_ul_streaming_audio_contexts = ble_ascs_get_ltv_value_from_metadata(METADATA_TYPE_STREAMING_AUDIO_CONTEXTS, ul_config.metadata_length, ul_config.metadata);
                if (NULL != p_ul_streaming_audio_contexts){
                    ul_context_type = p_ul_streaming_audio_contexts[0] |(p_ul_streaming_audio_contexts[1] << 8);
                }

                aud_cap.codec.ble_format.ble_codec.ul_context_type = ul_context_type;

                if ((ul_config.sdu_interval[0] + (ul_config.sdu_interval[1] << 8) + (ul_config.sdu_interval[2] << 16)) == 0x002710) {
                    aud_cap.codec.ble_format.ble_codec.frame_duration = FRAME_MS_10;
                }
                else if((ul_config.sdu_interval[0] + (ul_config.sdu_interval[1] << 8) + (ul_config.sdu_interval[2] << 16)) == 0x001D4C) {
                    aud_cap.codec.ble_format.ble_codec.frame_duration = FRAME_MS_7P5;
                }

                le_audio_log("[CAP][AM] ul_context_type:%d", 1, ul_context_type);
                le_audio_log("[CAP][AM] UL config_info [codec:%X][sampling_freq:%X][frame_payload_length:%04X]", 3,
                    ul_config.codec[0], ul_config.sampling_frequency, ul_config.frame_payload_length);
            }
        }

        aud_cap.codec.ble_format.ble_codec.sample_rate = bt_sink_srv_cap_am_convert_sampling_freq(config_info.sampling_frequency);

        le_audio_log("[CAP][AM] DL config_info [codec:%X][sampling_freq:%X][frame_payload_length:%04X][volume:%x][mute:%d]", 5,
           config_info.codec[0], config_info.sampling_frequency, config_info.frame_payload_length, volume_state.volume, volume_state.mute);

        aud_cap.type = BLE;
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        if (ul_only)
        {
            if (ul_config.codec[0] == codec_id[0]) {
                aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3;
            } else if (0 == memcmp(ul_config.codec, lc3plus_cbr_id, AUDIO_CODEC_ID_SIZE)) {
                aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3PLUS_CBR;
            } else if (0 == memcmp(ul_config.codec, lc3plus_vbr_id, AUDIO_CODEC_ID_SIZE)) {
                aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3PLUS_VBR;
            }
        }
        else
        {
#endif
        if (config_info.codec[0] == codec_id[0]) {
            aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3;
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        } else if (0 == memcmp(config_info.codec, lc3plus_cbr_id, AUDIO_CODEC_ID_SIZE)) {
            aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3PLUS_CBR;
        } else if (0 == memcmp(config_info.codec, lc3plus_vbr_id, AUDIO_CODEC_ID_SIZE)) {
            aud_cap.codec.ble_format.ble_codec.codec                = CODEC_LC3PLUS_VBR;
#endif
        }
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        }
#endif
        if (config_info.is_stereo == true) {
            aud_cap.codec.ble_format.ble_codec.channel_num = CHANNEL_STEREO;
        } else {
            if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                if ((direction == AUDIO_DIRECTION_SOURCE && ase_list.num_of_ase == 3) ||
                    (direction == AUDIO_DIRECTION_SINK && ase_list.num_of_ase == 2) ||
                    (mode == CAP_AM_BROADCAST_MUSIC_MODE && service_big->num_bis > 1)) {
                    aud_cap.codec.ble_format.ble_codec.channel_num = CHANNEL_DUAL_MONO;
                } else {
                    aud_cap.codec.ble_format.ble_codec.channel_num = CHANNEL_MONO;
                }
            } else {
                aud_cap.codec.ble_format.ble_codec.channel_num = CHANNEL_MONO;
            }
        }

        if (ul_only) {
            aud_cap.codec.ble_format.ble_codec.channel_mode = CHANNEL_MODE_UL_ONLY;
        } else if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            aud_cap.codec.ble_format.ble_codec.channel_mode = CHANNEL_MODE_DL_UL_BOTH;
        } else {
            aud_cap.codec.ble_format.ble_codec.channel_mode = CHANNEL_MODE_DL_ONLY;
        }
        le_audio_log("[CAP][AM] play callback, channel_num:%d, channel_mode:%d", 2,
            aud_cap.codec.ble_format.ble_codec.channel_num, aud_cap.codec.ble_format.ble_codec.channel_mode);

        if ((config_info.sdu_interval[0] + (config_info.sdu_interval[1] << 8) + (config_info.sdu_interval[2] << 16)) == 0x002710) {
            aud_cap.codec.ble_format.ble_codec.frame_duration = FRAME_MS_10;
        } else if ((config_info.sdu_interval[0] + (config_info.sdu_interval[1] << 8) + (config_info.sdu_interval[2] << 16)) == 0x001D4C) {
            aud_cap.codec.ble_format.ble_codec.frame_duration = FRAME_MS_7P5;
        }

        aud_cap.codec.ble_format.ble_codec.frame_payload_length = config_info.frame_payload_length;
        aud_cap.codec.ble_format.ble_event = BT_CODEC_MEDIA_REQUEST;
        aud_cap.audio_stream_in.audio_device  = CAP_AUDIO_INPUT_DEVICE;
#if BT_SINK_LE_MIC_VOLUME_ADJUST_ENABLE
#ifdef AIR_LE_AUDIO_CIS_ENABLE
        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            aud_cap.audio_stream_in.audio_volume = mic_vol.volume;
            aud_cap.audio_stream_in.audio_mute = mic_vol.mute;
        } else
#endif
#endif
        {
            aud_cap.audio_stream_in.audio_volume  = AUD_VOL_IN_LEVEL0;
        }
        aud_cap.audio_stream_out.audio_device = CAP_AUDIO_OUTPUT_DEVICE;
        aud_cap.audio_stream_out.audio_volume = (bt_sink_srv_am_volume_level_out_t)(volume_state.volume);
        aud_cap.audio_stream_out.audio_mute = g_cap_am_local_mute ? g_cap_am_local_mute : volume_state.mute;
        /*g_cap_am_local_mute is set by upper layer, which has higher priority than VCS*/

        if ((mode >= CAP_AM_UNICAST_MUSIC_MODE_START && !config_info.frame_payload_length) ||
            (mode <= CAP_AM_UNICAST_CALL_MODE_END && !ul_config.frame_payload_length)) {
            /*No config info, stop and set ready*/
            bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_STOP);
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            return;
        }

        if (g_cap_am_switch_suspending_mode != mode && mode < CAP_AM_MODE_NUM && g_cap_am_restarting_dsp_mode != mode) {
            bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_WAIT_PLAY);
        }

        if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
            le_sink_srv_set_streaming_state(true);
        }

        am_result = bt_sink_srv_ami_audio_play(g_cap_am_ctl[mode].cap_aid, &aud_cap);
#if BT_SINK_LE_MIC_VOLUME_ADJUST_ENABLE
#ifdef AIR_LE_AUDIO_CIS_ENABLE
        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            bt_sink_srv_le_volume_set_mute_and_volume_level(g_cap_am_ctl[mode].cap_aid, BT_SINK_SRV_LE_STREAM_TYPE_IN, mic_vol.mute, mic_vol.volume);
        }
#endif
#endif
        le_audio_log("[CAP][AM] play callback, play result:%d, mode:%d, switch_suspending_mode:%d", 3, am_result, mode, g_cap_am_switch_suspending_mode);

        if (am_result != AUD_EXECUTION_SUCCESS && g_cap_am_switch_suspending_mode != mode) {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);

#ifdef AIR_LE_AUDIO_CIS_ENABLE
            bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);

            if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                //respond 2nd ASE Characteristic
                bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);
            }

            if (mode <= CAP_AM_UNICAST_CALL_MODE_END || ul_only) {
                //respond both ASE Characteristic
                bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
            }
#endif
        }

        //#ifdef AIR_LE_AUDIO_ENABLE
        //bt_sink_srv_cap_am_action_callback(g_cap_am_ctl.cap_aid, AUD_SINK_OPEN_CODEC, AUD_CMD_COMPLETE, NULL);
        //#endif
    } else {
        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    }
#endif
}

static void bt_sink_srv_cap_am_stop_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);

    le_audio_log("[CAP][AM] stop callback, mode:%d, state:%d, switch_suspending:%d, switch_suspending_mode:%d, restarting_dsp:%d",
        5, mode, state, g_cap_am_switch_suspending, g_cap_am_switch_suspending_mode, g_cap_am_restarting_dsp_mode);
#ifdef AIR_BT_CODEC_BLE_ENABLED
    if (handle->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE && (state == CAP_AM_STATE_PREPARE_STOP || state == CAP_AM_STATE_SUSPEND_PREPARE_STOP)) {
        bt_sink_srv_cap_am_set_state(mode, (state == CAP_AM_STATE_PREPARE_STOP ? CAP_AM_STATE_WAIT_STOP : CAP_AM_STATE_SUSPEND_WAIT_STOP));

        if (g_cap_am_switch_suspending && g_cap_am_switch_suspending_mode == mode) {
            le_audio_log("[CAP][AM] stop callback in switch suspending state", 0);
            g_cap_am_switch_suspending_mode = CAP_INVALID_UINT8;
            bt_sink_srv_cap_am_action_callback(g_cap_am_ctl[mode].cap_aid, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
            return;
        }

        if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
            bt_sink_srv_cap_am_lea_control_sidetone(false);
        }

        if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
            /*Audio resource is not in used*/
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
		} else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
            le_sink_srv_set_streaming_state(false);
        }

        //#ifdef AIR_LE_AUDIO_ENABLE
        //bt_sink_srv_cap_am_action_callback(g_cap_am_ctl.cap_aid, AUD_SELF_CMD_REQ, AUD_CMD_COMPLETE, NULL);
        //#endif
    } else if (state == CAP_AM_STATE_SUSPENDING) {
        if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
            /*Audio resource is not in used*/
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
            audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);
		} else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
            le_sink_srv_set_streaming_state(false);
        }
    } else if ((state == CAP_AM_STATE_PLAYING &&
    (g_cap_am_switch_suspending_mode == mode || g_cap_am_restarting_dsp_mode == mode))) {
        /*Switch to suspending, just stop DSP without change ASE/AM state*/
		bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid);
    }
#endif
}

static void bt_sink_srv_cap_am_suspend_callback(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    bt_sink_srv_config_t config ;
    bt_sink_srv_state_manager_get_suspend_config_t get_suspend_config;
    memset(&config, 0, sizeof(bt_sink_srv_config_t));
    memset(&get_suspend_config, 0, sizeof(bt_sink_srv_state_manager_get_suspend_config_t));
    get_suspend_config.type = BT_SINK_SRV_DEVICE_LE;
    get_suspend_config.suspend_handle = int_hd;

    if (conn_info != NULL) {
        bt_sink_srv_memcpy(&(get_suspend_config.address),&(conn_info->peer_addr.addr),sizeof(bt_bd_addr_t));
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_SUSPEND_CONFIG, (void*)(&get_suspend_config), &config);
#endif
    }

    le_audio_log("[CAP][AM] suspend callback, mode:%d, state:%d, int_priority:%d, suspend_operation:%x, will_suspend_resume:%d", 5,
        mode, state, int_hd->priority, config.suspend_config.suspend_operation, config.suspend_config.will_suspend_resume);
#ifdef AIR_BT_CODEC_BLE_ENABLED
    if (handle->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE) {
        if (state == CAP_AM_STATE_PLAYING) {

            /*Check currnt media state to judge resuming or not*/
            //bool resume = (BLE_MCS_MEDIA_STATE_PLAYING == bt_le_audio_sink_media_get_state(conn_handle, BLE_MCP_GMCS_INDEX) ? true : false);

            if (mode == g_cap_am_restarting_psedev_mode) {
                /*Restarting is interrupted*/
                g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
            }

            if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {

                bt_sink_srv_cap_am_set_state(mode,
                    (BROADCAST_MUSIC_RESUME) ? CAP_AM_STATE_SUSPEND_WAIT_STOP : CAP_AM_STATE_WAIT_STOP);

                bt_sink_srv_cap_stream_service_big_t *big_info = bt_sink_srv_cap_stream_get_service_big();
                if (NULL != big_info) {
                    ble_bap_stop_syncing_to_periodic_advertising(big_info->sync_handle);
                    bt_sink_srv_cap_stream_remove_all_bis_data_path(big_info->big_handle);
                }
                if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                    //audio_src_srv_del_waiting_list(g_cap_am_ctl[mode].p_handle);
                    bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                }
            } else {

                if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {

                    if (config.suspend_config.suspend_operation & BT_SINK_SRV_INTERRUPT_OPERATION_HOLD_CALL) {
                        bt_sink_srv_cap_stream_hold_call(conn_handle);
                    }
                    //bt_le_audio_sink_call_hold_all_active_accept_others(conn_handle, BLE_CCP_GTBS_INDEX);
                    bt_sink_srv_cap_am_lea_control_sidetone(false);
#if (RELEASE_LE_CALL_WHEN_INTERRUPT)
                    /*Call mode*/
                    //bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_WAIT_STOP);
                    bt_sink_srv_cap_stream_release_autonomously(conn_handle, 0xFF, config.suspend_config.will_suspend_resume, 0);
#else
                    /*Call modes co-exist, no need to release ASE*/
                    bt_sink_srv_cap_stream_remove_all_cis_data_path(conn_handle);
                    bt_sink_srv_cap_am_set_state(mode,
                        config.suspend_config.will_suspend_resume ? CAP_AM_STATE_SUSPEND_WAIT_STOP : CAP_AM_STATE_WAIT_STOP);

                    if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                        /*Audio resource is not in used*/
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                        if (config.suspend_config.will_suspend_resume) {
                            audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);
                        }
                    }
#endif
                } else {
                    /*Check currnt media state to judge resuming or not*/
                    bool mcs_play = (BLE_MCS_MEDIA_STATE_PLAYING == bt_le_audio_sink_media_get_state(conn_handle, BLE_MCP_GMCS_INDEX) ? true : false);
                    if (mcs_play) {
                        //Set mcs_play_needed flag to resume music after call end
                        g_cap_am_ctl[mode].mcs_play_needed = true;

                    }

                    if (config.suspend_config.suspend_operation & BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC) {

                        bt_le_audio_sink_action_param_t le_param = {
                            .service_idx = BLE_MCP_GMCS_INDEX,
                        };
                        bt_le_audio_sink_send_action(conn_handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PAUSE, &le_param);
                    }

#if (RELEASE_LE_MUSIC_WHEN_INTERRUPT)
                    bt_sink_srv_cap_stream_release_autonomously(conn_handle, 0xFF, config.suspend_config.will_suspend_resume, 0);
#else
                    bt_sink_srv_cap_stream_remove_all_cis_data_path(conn_handle);

                    bt_sink_srv_cap_am_set_state(mode,
                        config.suspend_config.will_suspend_resume ? CAP_AM_STATE_SUSPEND_WAIT_STOP : CAP_AM_STATE_WAIT_STOP);

                    if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                        /*Audio resource is not in used*/
                        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                        if (config.suspend_config.will_suspend_resume) {
                            audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);
                            if (config.suspend_config.suspend_resume_timeout) {
                                bt_sink_srv_cap_am_resume_timer_start(mode, config.suspend_config.suspend_resume_timeout);
                                //bt_sink_srv_music_start_vp_detection(a2dp_dev->handle, config.suspend_config.suspend_resume_timeout);
                            }
                        }
                    } else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
                        le_sink_srv_set_streaming_state(false);
                    }
#endif
                }
            }
        } else if (state == CAP_AM_STATE_WAIT_PLAY) {
            //bt_handle_t conn_handle = bt_sink_srv_cap_stream_get_service_ble_link();
            if (config.suspend_config.will_suspend_resume) {
                g_cap_am_ctl[mode].resume_needed = true;
            }
            bt_sink_srv_cap_am_audio_stop(mode);

            /*if (conn_handle != BT_HANDLE_INVALID) {
                bt_sink_srv_cap_stream_enabling_response(false);
                if (g_cap_am_ctl.current_mode == UNICAST_CALL_MODE) {
                    bt_sink_srv_cap_stream_enabling_response(false);
                }
                bt_sink_srv_cap_stream_clear_service_ble_link(conn_handle);
            }*/
        }
    }
#endif
}

static void bt_sink_srv_cap_am_reject_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);

    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    bt_sink_srv_config_t config ;
    bt_sink_srv_state_manager_get_reject_config_t get_reject_config;
    memset(&config, 0, sizeof(bt_sink_srv_config_t));
    memset(&get_reject_config, 0, sizeof(bt_sink_srv_state_manager_get_reject_config_t));
    get_reject_config.type = BT_SINK_SRV_DEVICE_LE;

    if (conn_info != NULL) {
        bt_sink_srv_memcpy(&(get_reject_config.address),&(conn_info->peer_addr.addr),sizeof(bt_bd_addr_t));
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_REJECT_CONFIG, (void*)(&get_reject_config), &config);
#endif
    }

    le_audio_log("[CAP][AM] reject callback, mode:%d, state:%d, will_reject_resume:%x", 3,
        mode, state, config.reject_config.will_reject_resume);

#ifdef AIR_BT_CODEC_BLE_ENABLED
    if (handle->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE && state == CAP_AM_STATE_PREPARE_PLAY) {
        const audio_src_srv_handle_t *rej_handle = NULL;
        rej_handle = audio_src_srv_get_runing_pseudo_device();
        (void)rej_handle;

        le_audio_log("[CAP][AM] reject callback, mode:%d, RejDevId:%d, RejDevType:%d, BleDevId:%d", 4, mode, rej_handle->dev_id, rej_handle->type, handle->dev_id);

        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);

        if (mode == CAP_AM_BROADCAST_MUSIC_MODE) {
            bt_sink_srv_cap_stream_broadcast_enabling_response(false);

        } else if (conn_handle != BT_HANDLE_INVALID) {
#if (REJECT_ASE_ENABLE_WHEN_AM_REJECT)
            bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_PLAY);
            if (config.reject_config.will_reject_resume) {
    			audio_src_srv_add_waiting_list(handle);
            }
            bt_sink_srv_cap_am_reject_timer_start(mode);
#else
            /*Allow ASE enable operation without taking AM and playing DSP*/
            if (config.reject_config.will_reject_resume) {
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_SUSPENDING);
    			audio_src_srv_add_waiting_list(handle);
            }

            bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);

            if(le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
            }

            if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
            }
            bt_sink_srv_cap_am_reject_timer_start(mode);

#endif
#ifdef AIR_LE_AUDIO_CIS_ENABLE
            /*bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);
            if (mode <= CAP_AM_UNICAST_CALL_MODE_1) {
                bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
            }*/
#endif

        }
        //bt_sink_srv_cap_am_send_event_and_set_state(AUDIO_SRC_SRV_EVT_UNAVAILABLE);
    }
#endif
}

static void bt_sink_srv_cap_am_exception_callback(audio_src_srv_handle_t *handle, int32_t event, void *param)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    (void)mode;
    le_audio_log("[CAP][AM] exception callback, mode:%d, state:%d", 2, mode, bt_sink_srv_cap_am_get_state(mode));
#ifdef AIR_BT_CODEC_BLE_ENABLED
    if (handle->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE) {

    }
#endif
}

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE

static void bt_sink_srv_cap_am_lea_int_user_conf_reject_response(bt_sink_srv_cap_am_mode mode)
{
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);

    if (BLE_MCS_MEDIA_STATE_PLAYING == bt_le_audio_sink_media_get_state(conn_handle, BLE_MCP_GMCS_INDEX) ? true : false) {

        bt_le_audio_sink_action_param_t le_param = {
            .service_idx = BLE_MCP_GMCS_INDEX,
        };
        bt_le_audio_sink_send_action(conn_handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PAUSE, &le_param);
    }

#if (REJECT_ASE_ENABLE_WHEN_AM_REJECT)
    bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);

    if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
        bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);
    }

    if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
        bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
    }
#else
    /*Allow ASE enable operation without taking AM and playing DSP*/
    bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);

    if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
        bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
    }

    if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
        bt_sink_srv_cap_stream_enabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
    }
#endif
}

static void bt_sink_srv_cap_am_lea_int_user_conf_timer_callback(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_cap_am_mode mode = (bt_sink_srv_cap_am_mode)data;

    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_lea_int_user_conf_timer_callback, mode:%d, ", 1, mode);

    if (mode < CAP_AM_MODE_NUM && g_cap_am_ctl[mode].state == CAP_AM_STATE_PREPARE_PLAY) {

        bt_sink_srv_cap_am_lea_int_user_conf_reject_response(mode);
    }

}

bt_status_t bt_sink_srv_cap_am_lea_int_user_conf_set(uint8_t val, const bt_bd_addr_t *address)
{
    bt_bd_addr_t *temp_addr = NULL;
    bt_status_t ret = BT_STATUS_FAIL;
    bt_handle_t handle = bt_gap_le_srv_get_conn_handle_by_address(address);
    if (!handle) {
        bt_device_manager_le_bonded_info_t *le_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)address);
        if (le_bond_info != NULL) {
            temp_addr = &(le_bond_info->bt_addr.addr);
            handle = bt_gap_le_srv_get_conn_handle_by_address(temp_addr);
        }
    }
    bt_sink_srv_cap_am_mode mode = CAP_AM_UNICAST_MUSIC_MODE_START + bt_sink_srv_cap_get_link_index(handle);
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);

    le_audio_log("[CAP][AM]bt_sink_srv_cap_am_lea_int_user_conf_set, mode:%d, state:%d, val:%d", 3, mode, state, val);
    bt_timer_ext_stop(BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER);

    if (mode < CAP_AM_MODE_NUM && state == CAP_AM_STATE_PREPARE_PLAY) {
        if (val) {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, CAP_AM_STATE_PREPARE_PLAY);
        } else {
            bt_sink_srv_cap_am_lea_int_user_conf_reject_response(mode);
        }
        ret = BT_STATUS_SUCCESS;
    }

    return ret;
}

extern int32_t bt_sink_srv_cap_am_lea_int_user_conf_get(bt_bd_addr_ptr_t coming_addr, bt_bd_addr_ptr_t cur_addr);
static int32_t default_bt_sink_srv_cap_am_lea_int_user_conf_get(bt_bd_addr_ptr_t coming_addr, bt_bd_addr_ptr_t cur_addr)
{
    return BT_SINK_SRV_LEA_INT_USER_CONF_ALLOW;
}

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:bt_sink_srv_a2dp_int_user_conf_get=default_bt_sink_srv_a2dp_int_user_conf_get")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_srv_cap_am_lea_int_user_conf_get = default_bt_sink_srv_cap_am_lea_int_user_conf_get
#else
#error "Unsupported Platform"
#endif

uint32_t bt_sink_srv_cap_am_lea_int_user_conf_check(bt_sink_srv_cap_am_mode mode)
{
    uint32_t ret = BT_SINK_SRV_LEA_INT_USER_CONF_ALLOW;
    //bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_device_state_t cur_dev;
    bt_sink_srv_memset(&cur_dev, 0, sizeof(bt_sink_srv_device_state_t));
    bt_status_t playing_status = bt_sink_srv_get_playing_device_state(&cur_dev);
    bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);

    if (conn_info != NULL && playing_status == BT_STATUS_SUCCESS && (!cur_dev.sco_state && cur_dev.call_state < BT_SINK_SRV_STATE_INCOMING)) {
        ret = bt_sink_srv_cap_am_lea_int_user_conf_get((bt_bd_addr_ptr_t)(conn_info->peer_addr.addr), (bt_bd_addr_ptr_t)(cur_dev.address));
        if (BT_SINK_SRV_LEA_INT_USER_CONF_PENDING == ret) {
            bt_timer_ext_t *timer_ctx = bt_timer_ext_find(BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER);
            if (timer_ctx == NULL) {
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_PREPARE_PLAY);
                bt_timer_ext_start(BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER, (uint32_t)mode,
                                           BT_SINK_SRV_LEA_INT_WAIT_USER_TIMER_DUR, bt_sink_srv_cap_am_lea_int_user_conf_timer_callback);
            } else if (timer_ctx->data != (uint32_t)mode) {
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);
                bt_timer_ext_stop(BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER);
            }

            /*bt_timer_ext_t *timer_ctx = bt_timer_ext_find(BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER);

            if (!timer_ctx) {
                bt_timer_ext_start(BT_SINK_SRV_A2DP_INT_USER_CONF_TIMER_ID, (uint32_t)dev,
                                   BT_SINK_SRV_A2DP_INT_WAIT_USER_TIMER_DUR, bt_sink_srv_a2dp_int_user_conf_timer);
            } else if (timer_ctx->data != (uint32_t)dev) {
                bt_timer_ext_stop(BT_SINK_SRV_A2DP_INT_USER_CONF_TIMER_ID);
            }*/
        } else if (BT_SINK_SRV_LEA_INT_USER_CONF_DISALLOW == ret) {
            bt_sink_srv_cap_am_lea_int_user_conf_reject_response(mode);
        }
    }
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_lea_int_user_conf_check, mode:%d, playing_status:%x, ret:0x%x", 3, mode, playing_status, ret);
    return ret;
}
#endif

static void bt_sink_srv_cap_am_send_event_and_set_state(bt_sink_srv_cap_am_mode mode, audio_src_srv_event_t event_id)
{
    uint8_t cur_state = bt_sink_srv_cap_am_get_state(mode);

    le_audio_log("[CAP][AM] send to am and set state, mode:%d, event:%d, state:%d", 3, mode, event_id, cur_state);
    if (mode >= CAP_AM_MODE_NUM) {
        return;
    }

    switch (event_id) {
        case AUDIO_SRC_SRV_EVT_UNAVAILABLE:
            cur_state = CAP_AM_STATE_OFF;
            break;
        case AUDIO_SRC_SRV_EVT_READY:
            if (cur_state == CAP_AM_STATE_SUSPEND_WAIT_STOP) {
                cur_state = CAP_AM_STATE_SUSPENDING;
            } else if (cur_state != CAP_AM_STATE_SUSPENDING) {
                cur_state = CAP_AM_STATE_IDLE;
            }
            break;
        case AUDIO_SRC_SRV_EVT_PREPARE_PLAY:
            cur_state = CAP_AM_STATE_PREPARE_PLAY;
            break;
        case AUDIO_SRC_SRV_EVT_PREPARE_STOP:
            if (cur_state != CAP_AM_STATE_SUSPENDING) {
                cur_state = (g_cap_am_ctl[mode].resume_needed ? CAP_AM_STATE_SUSPEND_PREPARE_STOP : CAP_AM_STATE_PREPARE_STOP);
            }
            break;
        case AUDIO_SRC_SRV_EVT_PLAYING:
            cur_state = CAP_AM_STATE_PLAYING;
            break;

        default:
            break;
    }
    bt_sink_srv_cap_am_set_state(mode, cur_state);

    audio_src_srv_update_state(g_cap_am_ctl[mode].p_handle, event_id);
}

void bt_sink_srv_cap_am_disable_waiting_list(void)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (bt_sink_srv_cap_am_get_state(i) == CAP_AM_STATE_SUSPENDING) {

            le_audio_log("[CAP][AM] disable suspending mode [%d]", 1, i);
            audio_src_srv_del_waiting_list(g_cap_am_ctl[i].p_handle);
        }
    }
}

void bt_sink_srv_cap_am_enable_waiting_list(void)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (bt_sink_srv_cap_am_get_state(i) == CAP_AM_STATE_SUSPENDING) {

            le_audio_log("[CAP][AM] enable suspending mode [%d]", 1, i);
            audio_src_srv_add_waiting_list(g_cap_am_ctl[i].p_handle);
        }
    }
}

static void bt_sink_srv_cap_am_set_state(bt_sink_srv_cap_am_mode mode, uint8_t state)
{
    le_audio_log("[CAP][AM] set state, mode:%d, state:%d", 2, mode, state);
    g_cap_am_ctl[mode].state = state;
}

static uint8_t bt_sink_srv_cap_am_get_state(bt_sink_srv_cap_am_mode mode)
{
    return g_cap_am_ctl[mode].state;
}

static bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_mode_by_audio_id(bt_sink_srv_am_id_t aud_id)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (g_cap_am_ctl[i].cap_aid == aud_id) {
            return i;
        }
    }
    return CAP_INVALID_UINT8;
}

bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_mode_by_audio_handle(audio_src_srv_handle_t *p_handle)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (g_cap_am_ctl[i].p_handle == p_handle) {
            return i;
        }
    }
    return CAP_INVALID_UINT8;
}

static bool bt_sink_srv_cap_am_reject_timer_start(bt_sink_srv_cap_am_mode mode)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_reject_timer_start, mode:%d, g_cap_am_reject_mode:%x", 2, mode, g_cap_am_reject_mode);

    /*if (g_cap_am_reject_timer) {
        return false;
    } else {
        g_cap_am_reject_timer = xTimerCreate("cap_am_reject_timer", (1500 * portTICK_PERIOD_MS), pdFALSE, NULL, bt_sink_srv_cap_am_reject_timer_callback);
    }

    if (!g_cap_am_reject_timer) {
        return false;
    }

    xTimerStart(g_cap_am_reject_timer, 0);
    g_cap_am_reject_mode = mode;
    return true;*/


    if(bt_timer_ext_find(BT_SINK_SRV_CAP_AM_REJECT_TIMER) == NULL) {
        g_cap_am_reject_mode = mode;
        bt_timer_ext_start(BT_SINK_SRV_CAP_AM_REJECT_TIMER, (uint32_t)mode, 1500, bt_sink_srv_cap_am_reject_timer_callback);
        return true;
    }
    return false;
}

static void bt_sink_srv_cap_am_reject_timer_callback(uint32_t timer_id, uint32_t data)
{
    uint8_t state;
    bt_handle_t conn_handle = BT_HANDLE_INVALID;

    bt_sink_srv_cap_am_mode mode = (bt_sink_srv_cap_am_mode)(data);

    le_audio_log("[CAP][AM] reject timer timeout, g_cap_am_reject_mode:%d, timer_id:%x", 2, mode,  timer_id);

    /*if (pdPASS == xTimerDelete(g_cap_am_reject_timer, 0)) {
            g_cap_am_reject_timer = NULL;
    }

    if (CAP_INVALID_UINT8 == g_cap_am_reject_mode) {
        return;
    }*/

    state = bt_sink_srv_cap_am_get_state(mode);
    conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);

    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    bt_sink_srv_config_t config ;
    bt_sink_srv_state_manager_get_reject_config_t get_reject_config;
    memset(&config, 0, sizeof(bt_sink_srv_config_t));
    memset(&get_reject_config, 0, sizeof(bt_sink_srv_state_manager_get_reject_config_t));
    get_reject_config.type = BT_SINK_SRV_DEVICE_LE;

    if (conn_info != NULL) {
        bt_sink_srv_memcpy(&(get_reject_config.address),&(conn_info->peer_addr.addr),sizeof(bt_bd_addr_t));
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_REJECT_CONFIG, (void*)(&get_reject_config), &config);
#endif
    }

    le_audio_log("[CAP][AM] reject timer timeout, mode:%d, state:%d", 3, mode, state, config.reject_config.reject_operation);

    if (state == CAP_AM_STATE_PREPARE_PLAY) {
        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);

        if (conn_handle != BT_HANDLE_INVALID) {
#if (REJECT_ASE_ENABLE_WHEN_AM_REJECT)
            bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);

            if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET) {
                //respond 2nd ASE Characteristic
                bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SINK);
            }

            if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
                //respond both ASE Characteristic
                bt_sink_srv_cap_stream_enabling_response(conn_handle, false, AUDIO_DIRECTION_SOURCE);
            }

#endif
        }
    }
    if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END &&
        config.reject_config.reject_operation & BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC) {
        /*TO DO MCP PAUSE*/
        bt_le_audio_sink_action_param_t le_param = {
            .service_idx = BLE_MCP_GMCS_INDEX,
        };
        bt_le_audio_sink_send_action(conn_handle, BT_LE_AUDIO_SINK_ACTION_MEDIA_PAUSE, &le_param);
    } else if (config.reject_config.reject_operation & BT_SINK_SRV_INTERRUPT_OPERATION_HOLD_CALL) {
        bt_sink_srv_cap_stream_hold_call(conn_handle);
    }
	//audio_src_srv_del_waiting_list(g_cap_am_ctl[g_cap_am_reject_mode].p_handle);
    g_cap_am_reject_mode = CAP_INVALID_UINT8;
}

static bool bt_sink_srv_cap_am_reject_timer_stop(void)
{
    le_audio_log("[CAP][AM] reject timer stop, g_cap_am_reject_mode:%d", 1, g_cap_am_reject_mode);

    if (g_cap_am_reject_mode == CAP_INVALID_UINT8) {
        return false;
    }

    if (BT_TIMER_EXT_STATUS_SUCCESS == bt_timer_ext_stop(BT_SINK_SRV_CAP_AM_REJECT_TIMER)) {
        if (g_cap_am_reject_mode != CAP_INVALID_UINT8) {
            audio_src_srv_del_waiting_list(g_cap_am_ctl[g_cap_am_reject_mode].p_handle);
            g_cap_am_reject_mode = CAP_INVALID_UINT8;
        }
        return true;
    }

    return false;
}

static uint8_t bt_sink_srv_cap_am_convert_sampling_freq(uint8_t config_freq)
{
    uint8_t am_freq = SAMPLING_RATE_NUM;

    switch (config_freq) {

        case CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ:
            am_freq = SAMPLING_RATE_16K;
            break;

        case CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ:
            am_freq = SAMPLING_RATE_24K;
            break;

        case CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ:
            am_freq = SAMPLING_RATE_32K;
            break;

        case CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ:
            am_freq = SAMPLING_RATE_44_1K;
            break;

        case CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ:
            am_freq = SAMPLING_RATE_48K;
            break;
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
        case CODEC_CONFIGURATION_SAMPLING_FREQ_96KHZ:
            am_freq = SAMPLING_RATE_96K;
            break;
#endif
        default:
            break;
    }
    return am_freq;
}

#if 1//def BT_SINK_MUSIC_NEW_VP_INTERRUPT_SOLUTION
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
static bool bt_sink_srv_cap_am_resume_timer_start(bt_sink_srv_cap_am_mode mode, uint32_t duration_ms)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_resume_timer_start mode:%d", 1, mode);

    if (mode >= CAP_MAX_DEVICE_NUM) {
        return false;
    }

#ifdef AIR_BT_SINK_MUSIC_ENABLE
    bt_sink_srv_music_start_vp_detection(g_cap_am_ctl[mode].p_handle, duration_ms);
#endif
    return true;
}
#endif

static bool bt_sink_srv_cap_am_resume_timer_stop(bt_sink_srv_cap_am_mode mode)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_resume_timer_stop mode:%d", 1, mode);

    if (mode >= CAP_MAX_DEVICE_NUM) {
        return false;
    }

    if (CAP_AM_STATE_SUSPENDING == bt_sink_srv_cap_am_get_state(mode)) {
        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);
    }
#ifdef AIR_BT_SINK_MUSIC_ENABLE
    bt_sink_srv_music_stop_vp_detection(g_cap_am_ctl[mode].p_handle);
#endif
    return true;
}
#endif

static void bt_sink_srv_cap_am_suspend_callback_timer_start(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    /*This API used to transfer from AM task to BT task, delay is set to 0 */
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);

    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_suspend_callback_timer_start, mode:%d", 1, mode);

    if(bt_timer_ext_find(BT_SINK_SRV_AM_SUSPEND_CALLBACK_TIMER) == NULL) {
        g_cap_am_suspend_callback_on_bt_task.handle = handle;
        g_cap_am_suspend_callback_on_bt_task.int_hd = int_hd;
        bt_timer_ext_start(BT_SINK_SRV_AM_SUSPEND_CALLBACK_TIMER, 0, 0, bt_sink_srv_cap_am_suspend_callback_timer_callback);
    }
}

static void bt_sink_srv_cap_am_suspend_callback_timer_callback(uint32_t timer_id, uint32_t data)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_suspend_callback_timer_callback timer timeout, timer_id:0x%x", 1, timer_id);

    cap_audio_manager_suspend_callback_on_bt_task_data param = g_cap_am_suspend_callback_on_bt_task;
    g_cap_am_suspend_callback_on_bt_task.handle = NULL;
    g_cap_am_suspend_callback_on_bt_task.int_hd = NULL;

    bt_sink_srv_cap_am_suspend_callback(param.handle, param.int_hd);
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void bt_sink_srv_cap_am_init()
{
    le_audio_log("[CAP][AM] init, current mode:%d", 1, g_cap_am_current_mode);

    if (g_cap_am_current_mode == CAP_INVALID_UINT8) {
        bt_sink_srv_cap_am_para_init();
        bt_sink_srv_cap_am_function_init();
        bt_sink_srv_cap_am_callback_register();
        //bt_sink_srv_cap_am_send_event_and_set_state(AUDIO_SRC_SRV_EVT_READY);
    }
}

void bt_sink_srv_cap_am_deinit(void)
{
    uint8_t state = 0;
    for (uint8_t mode = 0; mode < CAP_MAX_DEVICE_NUM; mode++) {
        state = bt_sink_srv_cap_am_get_state(mode);
        le_audio_log("[CAP][AM] deinit, mode:%d, state:%d", 2, mode, state);

        if ((g_cap_am_ctl[mode].cap_aid == AUD_ID_INVALID) || (state == CAP_AM_STATE_OFF)) {
            continue;
        }

        if (state == CAP_AM_STATE_PLAYING) {
            g_cap_am_ctl[mode].deinit_needed = true;
            bt_sink_srv_cap_am_audio_stop(mode);
            continue;
        } else if (state == CAP_AM_STATE_IDLE || state == CAP_AM_STATE_SUSPENDING) {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
            bt_sink_srv_cap_am_callback_deregister(mode);
        } else if (state == CAP_AM_STATE_PREPARE_PLAY || state == CAP_AM_STATE_WAIT_PLAY ||
                   state == CAP_AM_STATE_PREPARE_STOP || state == CAP_AM_STATE_WAIT_STOP) {
            bt_sink_srv_cap_am_reject_timer_stop();
            g_cap_am_ctl[mode].deinit_needed = true;
        }
    }

    ///
    /*uint8_t state = bt_sink_srv_cap_am_get_state();
    le_audio_log("[CAP][AM] deinit, aid:%d, state:%d", 2, g_cap_am_ctl.cap_aid, state);

    if((g_cap_am_ctl.cap_aid == AUD_ID_INVALID) || (state == CAP_AM_STATE_OFF) || (state == CAP_AM_STATE_SUSPENDING))
    {
        return;
    }

    if (state == CAP_AM_STATE_PLAYING && g_cap_am_ctl.current_mode == BROADCAST_MUSIC_MODE && reason == CAP_AM_DEINIT_REASON_LE_DISCONNECT) {
        return;
    }

    if (state == CAP_AM_STATE_PLAYING) {
        g_cap_am_ctl.deinit_needed = true;
        bt_sink_srv_cap_am_audio_stop();
        return;
    } else if (state == CAP_AM_STATE_IDLE) {
        bt_sink_srv_cap_am_send_event_and_set_state(AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        bt_sink_srv_cap_am_callback_deregister();
    } else if (state == CAP_AM_STATE_PREPARE_PLAY || state == CAP_AM_STATE_WAIT_PLAY ||
        state == CAP_AM_STATE_PREPARE_STOP || state == CAP_AM_STATE_WAIT_STOP) {
        g_cap_am_ctl.deinit_needed = true;
    }*/

}

void bt_sink_srv_cap_am_audio_start(bt_sink_srv_cap_am_mode mode)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        le_audio_log("[CAP][AM] Invalid mode:%d", 1, mode);
        return;
    }
    uint8_t state = bt_sink_srv_cap_am_get_state(mode);
    le_audio_log("[CAP][AM] audio start, mode:%d, state:%d", 2, mode, state);

    if(state == CAP_AM_STATE_IDLE || state == CAP_AM_STATE_SUSPENDING || state == CAP_AM_STATE_PREPARE_PLAY) {
        g_cap_am_ctl[mode].stop_needed = false;

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
            /*Check music mode with call state, should modify priority to take audio resource in order to trigger music resume*/
            if (bt_sink_srv_le_get_call_state_by_handle(bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode)) != BT_SINK_SRV_STATE_NONE) {
                g_cap_am_ctl[mode].p_handle->priority = AUDIO_SRC_SRV_PRIORITY_HIGH;
            } else {
                g_cap_am_ctl[mode].p_handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
            }
        }
#endif
        bt_sink_srv_cap_am_resume_timer_stop(mode);

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
        if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
            if (BT_SINK_SRV_LEA_INT_USER_CONF_ALLOW == bt_sink_srv_cap_am_lea_int_user_conf_check(mode)) {
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
            }
        } else {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
        }
#else
        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
#endif
    } else if (state == CAP_AM_STATE_WAIT_STOP) {
        /*DSP is closing, set flag to restart*/
        g_cap_am_restarting_psedev_mode = mode;
    }
}

bool bt_sink_srv_cap_am_audio_stop(bt_sink_srv_cap_am_mode mode)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        le_audio_log("[CAP][AM] Invalid mode:%d", 1, mode);
        return false;
    }

    uint8_t state = bt_sink_srv_cap_am_get_state(mode);

    le_audio_log("[CAP][AM] audio stop, mode:%d, state:%d", 2, mode, state);

    if (mode == g_cap_am_restarting_psedev_mode) {
        /*Restarting is interrupted*/
        g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
    }

    switch (state) {
        case CAP_AM_STATE_PLAYING: {
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            return true;
        }

        case CAP_AM_STATE_PREPARE_PLAY:
        case CAP_AM_STATE_WAIT_PLAY: {
            if (mode == g_cap_am_reject_mode) {
                /*Reject timer is proccessing*/
                bt_sink_srv_cap_am_reject_timer_stop();
                bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);
            } else {
                g_cap_am_ctl[mode].stop_needed = true;
            }
            return true;
        }

        case CAP_AM_STATE_IDLE:
        case CAP_AM_STATE_SUSPENDING: {
            if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                //audio_src_srv_del_waiting_list(g_cap_am_ctl[mode].p_handle);
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
            } else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
                le_sink_srv_set_streaming_state(false);
            }
            return false;
        }

        case CAP_AM_STATE_WAIT_STOP: {
            /*Already stop, wait stop event*/
            return true;
        }

        default:
            break;


    }

    /*if(state == CAP_AM_STATE_PLAYING)
    {
        bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    }
    else if(state == CAP_AM_STATE_PREPARE_PLAY || state == CAP_AM_STATE_WAIT_PLAY ||
        state == CAP_AM_STATE_PREPARE_STOP || state == CAP_AM_STATE_WAIT_STOP)
    {
        g_cap_am_ctl[mode].stop_needed = true;
    }
    else if(state == CAP_AM_STATE_SUSPENDING)
    {
        bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid);
    }
    else if(state == CAP_AM_STATE_IDLE)
    {
        return;

        bt_handle_t conn_handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
        switch (mode) {
            case CAP_AM_UNICAST_CALL_MODE_0:
            case CAP_AM_UNICAST_CALL_MODE_1:
                bt_sink_srv_cap_stream_disabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
                bt_sink_srv_cap_stream_disabling_response(conn_handle, true, AUDIO_DIRECTION_SOURCE);
                break;

            case CAP_AM_UNICAST_MUSIC_MODE_0:
            case CAP_AM_UNICAST_MUSIC_MODE_1:
                bt_sink_srv_cap_stream_disabling_response(conn_handle, true, AUDIO_DIRECTION_SINK);
                break;

            default:
                break;
        }
    }*/
    return false;
}

void bt_sink_srv_cap_am_audio_suspend(bt_sink_srv_cap_am_mode mode)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        le_audio_log("[CAP][AM] Invalid mode:%d", 1, mode);
        return;
    }

    uint8_t state = bt_sink_srv_cap_am_get_state(mode);

    le_audio_log("[CAP][AM] audio suspend, mode:%d, state:%d", 2, mode, state);

    if (mode == g_cap_am_restarting_psedev_mode) {
        /*Restarting is interrupted*/
        g_cap_am_restarting_psedev_mode = CAP_INVALID_UINT8;
    }

    switch (state) {
        case CAP_AM_STATE_PLAYING: {
            bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_SUSPEND_PREPARE_STOP);
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            break;
        }

        case CAP_AM_STATE_PREPARE_PLAY:
        case CAP_AM_STATE_WAIT_PLAY: {
            g_cap_am_ctl[mode].stop_needed = true;
            bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            break;
        }

        case CAP_AM_STATE_SUSPEND_WAIT_STOP: {
            if (AUD_EXECUTION_FAIL == bt_sink_srv_ami_audio_stop(g_cap_am_ctl[mode].cap_aid)) {
                /*Audio resource is not in used*/
                bt_sink_srv_cap_am_send_event_and_set_state(mode, AUDIO_SRC_SRV_EVT_READY);
                audio_src_srv_add_waiting_list(g_cap_am_ctl[mode].p_handle);
            } else if (mode >= CAP_AM_UNICAST_MUSIC_MODE_START && mode <= CAP_AM_UNICAST_MUSIC_MODE_END) {
                le_sink_srv_set_streaming_state(false);
            }
            break;
        }

        default:
            break;
    }
}

bool bt_sink_srv_cap_am_audio_restart(bool only_dsp)
{
    le_audio_log("[CAP][AM] audio restart, current current_mode:%d, restarting_mode:%d, only_dsp:%d", 3,
        g_cap_am_current_mode, g_cap_am_restarting_psedev_mode, only_dsp);

    if (g_cap_am_current_mode == CAP_INVALID_UINT8 || g_cap_am_restarting_psedev_mode != CAP_INVALID_UINT8) {
        return false;
    }

    if (CAP_AM_STATE_PLAYING != bt_sink_srv_cap_am_get_state(g_cap_am_current_mode)) {
        return false;
    }

    if (only_dsp) {
        g_cap_am_restarting_dsp_mode = g_cap_am_current_mode;
        bt_sink_srv_cap_am_stop_callback(g_cap_am_ctl[g_cap_am_current_mode].p_handle);
    } else {
        g_cap_am_restarting_psedev_mode = g_cap_am_current_mode;
        bt_sink_srv_cap_am_send_event_and_set_state(g_cap_am_restarting_psedev_mode, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    }
    return true;
}

bool bt_sink_srv_cap_am_switch_psedev(bt_sink_srv_cap_am_mode new_mode)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_switch_psedev, current current_mode:%d, new_mode:%d, ", 2,
        g_cap_am_current_mode, new_mode);

    if (g_cap_am_current_mode >= CAP_AM_MODE_NUM || new_mode >= CAP_AM_MODE_NUM || g_cap_am_current_mode == new_mode) {
        return false;
    }

    if (CAP_AM_STATE_PLAYING != bt_sink_srv_cap_am_get_state(g_cap_am_current_mode)) {
        return false;
    }


    bt_sink_srv_cap_am_set_state(new_mode, CAP_AM_STATE_PREPARE_PLAY);
    g_cap_am_ctl[new_mode].p_handle->priority = AUDIO_SRC_SRV_PRIORITY_HIGH + 1;//Temporary set highest priority to resume this psedev first
    audio_src_srv_add_waiting_list(g_cap_am_ctl[new_mode].p_handle);
    bt_sink_srv_cap_am_audio_stop(g_cap_am_current_mode);
    return true;
}

bool bt_sink_srv_cap_am_audio_switch(bool switch_on)
{
    if (g_cap_am_current_mode >= CAP_AM_MODE_NUM) {
        return false;
    }

    if (switch_on) {
        bt_sink_srv_cap_am_play_callback(g_cap_am_ctl[g_cap_am_current_mode].p_handle);
    } else {
        g_cap_am_switch_suspending_mode = g_cap_am_current_mode;
        bt_sink_srv_cap_am_stop_callback(g_cap_am_ctl[g_cap_am_current_mode].p_handle);
    }
    return true;
}

int8_t bt_sink_srv_cap_am_get_aid(void)
{
    if (g_cap_am_current_mode < CAP_MAX_DEVICE_NUM) {
        return g_cap_am_ctl[g_cap_am_current_mode].cap_aid;
    } else {
        return AUD_ID_INVALID;
    }
}

audio_src_srv_handle_t *bt_sink_srv_cap_am_get_audio_handle(bt_sink_srv_cap_am_mode mode)
{
    le_audio_log("[CAP][AM] Get audio handle, mode:%d", 1, mode);

    if (mode >= CAP_MAX_DEVICE_NUM) {
        return NULL;
    }

    return g_cap_am_ctl[mode].p_handle;
}

bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_current_mode(void)
{
    le_audio_log("[CAP][AM] Get current streaming mode:%d", 1, g_cap_am_current_mode);
    return g_cap_am_current_mode;
}

bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_restarting_psedev_mode(void)
{
    le_audio_log("[CAP][AM] Get restarting sedev mode:%d", 1, g_cap_am_restarting_psedev_mode);
    return g_cap_am_restarting_psedev_mode;
}

bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_restarting_dsp_mode(void)
{
    le_audio_log("[CAP][AM] Get restarting dsp mode:%d", 1, g_cap_am_restarting_dsp_mode);
    return g_cap_am_restarting_dsp_mode;
}

bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_switch_suspending_mode(void)
{
    le_audio_log("[CAP][AM] Get switch suspending mode:%d", 1, g_cap_am_switch_suspending_mode);
    return g_cap_am_switch_suspending_mode;
}

bool bt_sink_srv_cap_am_is_psedev_streaming(bt_sink_srv_cap_am_mode mode)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        return false;
    }
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_is_psedev_streaming, mode:%d, state:%d", 2, mode, g_cap_am_ctl[mode].state);
    return ((g_cap_am_ctl[mode].state == CAP_AM_STATE_PLAYING || g_cap_am_ctl[mode].state == CAP_AM_STATE_WAIT_PLAY) ? true : false);
}


bool bt_sink_srv_cap_am_is_dsp_streaming_with_conversational_context(void)
{
    le_audio_log("[CAP][AM] streaming_with_conversational_context:%d", 1, g_cap_am_streaming_with_conversational_context);

    return g_cap_am_streaming_with_conversational_context;
}

#if 1//def BT_SINK_MUSIC_NEW_VP_INTERRUPT_SOLUTION
bool bt_sink_srv_cap_am_resume_timer_callback_handler(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(handle);
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_resume_timer_callback_handler mode:%d", 1, mode);
    if (mode >= CAP_MAX_DEVICE_NUM) {
        return false;
    }

    if (CAP_AM_STATE_SUSPENDING == bt_sink_srv_cap_am_get_state(mode)) {
        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);
    }
    return true;
}
#endif

bool bt_sink_srv_cap_am_reset_suspending(bt_sink_srv_cap_am_mode mode)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        return false;
    }

    le_audio_log("[CAP][AM] reset suspending mode[%d], state:%d", 2, mode, g_cap_am_ctl[mode].state);

    if (g_cap_am_ctl[mode].state == CAP_AM_STATE_SUSPENDING) {
        bt_sink_srv_cap_am_set_state(mode, CAP_AM_STATE_IDLE);
    }
    audio_src_srv_del_waiting_list(g_cap_am_ctl[mode].p_handle);
    return true;
}

bool bt_sink_srv_cap_am_lea_control_sidetone(bool enable)
{
    le_audio_log("[CAP][AM] bt_sink_srv_cap_am_control_sidetone, current_mode:%d, enable:%d", 2, g_cap_am_current_mode, enable);

    if (enable) {
        if (g_cap_am_current_mode > CAP_AM_UNICAST_CALL_MODE_END || !bt_sink_srv_call_get_sidetone_enable_config()) {
            return false;
        }

        am_audio_side_tone_enable();
    } else {
        am_audio_side_tone_disable();
    }
    return true;
}

#ifdef BT_SINK_DUAL_ANT_ENABLE
bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_is_taking_resource(void)
{
    for (uint8_t i = 0; i < CAP_MAX_DEVICE_NUM; i++) {
        if (g_cap_am_ctl[i].sub_state == CAP_AM_SUB_STATE_TAKING_SPK_RES || g_cap_am_ctl[i].sub_state == CAP_AM_SUB_STATE_TAKING_MIC_RES) {
            le_audio_log("[CAP][AM] is taking resource, mode:%d, sub_state:%d", 2, i, g_cap_am_ctl[i].sub_state);
            return i;
        }
    }
    return CAP_INVALID_UINT8;
}

void bt_sink_srv_cap_am_take_give_resource(bt_sink_srv_cap_am_mode mode, bool take)
{
    if (mode >= CAP_MAX_DEVICE_NUM) {
        return;
    }

    bt_sink_srv_dual_ant_data_t notify;

    if (mode <= CAP_AM_UNICAST_CALL_MODE_END) {
        g_cap_am_ctl[mode].sub_state = (take ? CAP_AM_SUB_STATE_TAKING_MIC_RES : CAP_INVALID_UINT8);

        notify.type = BT_SINK_DUAL_ANT_TYPE_CALL;
        notify.call_info.esco_state = take;
        bt_sink_srv_dual_ant_notify(false, &notify);
    } else {
        g_cap_am_ctl[mode].sub_state = (take ? CAP_AM_SUB_STATE_TAKING_SPK_RES : CAP_INVALID_UINT8);

        notify.type = BT_SINK_DUAL_ANT_TYPE_MUSIC;
        notify.music_info.a2dp_state = take;
        bt_sink_srv_dual_ant_notify(false, &notify);
    }
    le_audio_log("[CAP][AM] Take/Give resource:%d, mode:%d", 2, take, mode);
}

void bt_sink_srv_cap_am_take_resource_response(void)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_is_taking_resource();
    if (mode >= CAP_MAX_DEVICE_NUM) {
        return;
    }
    if (mode <= CAP_AM_UNICAST_CALL_MODE_END && g_cap_am_ctl[mode].sub_state == CAP_AM_SUB_STATE_TAKING_MIC_RES) {
        bt_sink_srv_cap_am_play_callback(g_cap_am_ctl[mode].p_handle);
        g_cap_am_ctl[mode].sub_state = CAP_INVALID_UINT8;
    }
}
#endif


