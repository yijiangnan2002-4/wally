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

extern bt_sink_srv_allow_result_t bt_customer_config_allow_play(
    bt_sink_srv_device_state_t *current,
    bt_sink_srv_device_state_t *coming);

static bool bt_sink_srv_state_manager_is_compare_waiting(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current);

static bt_sink_srv_allow_result_t bt_sink_srv_state_manager_compare_waiting(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming);

static bt_sink_srv_state_manager_play_count_t bt_sink_srv_state_manager_get_waiting_play_count(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *handle);

static bt_sink_srv_allow_result_t bt_sink_srv_state_manager_compare_interrupt(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming);

static bt_sink_srv_allow_result_t default_bt_customer_config_allow_play(
    bt_sink_srv_device_state_t *current,
    bt_sink_srv_device_state_t *coming);

#if defined(AIR_LE_AUDIO_ENABLE)
bt_sink_srv_cap_am_mode bt_sink_srv_cap_am_get_mode_by_audio_handle(
    audio_src_srv_handle_t *p_handle);
#endif

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_customer_config_allow_play=_default_bt_customer_config_allow_play")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma weak bt_customer_config_allow_play = default_bt_customer_config_allow_play
#else
#error "Unsupported Platform"
#endif

bt_sink_srv_allow_result_t bt_sink_srv_state_manager_psedev_compare(
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming)
{
    bt_sink_srv_allow_result_t result = BT_SINK_SRV_ALLOW_RESULT_BYPASS;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    bt_sink_srv_state_manager_device_t *coming_device = NULL;
    bt_sink_srv_device_state_t coming_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;

    /* 1. Check parameters. */
    if (current == NULL || !BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(current->type)) {
        return result;
    }

    if (coming == NULL || !BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(coming->type)) {
        return result;
    }

    /* 2. Compare devices. */
    if (bt_sink_srv_state_manager_is_compare_waiting(context, current)) {
        result = bt_sink_srv_state_manager_compare_waiting(context, current, coming);
    } else {
        result = bt_sink_srv_state_manager_compare_interrupt(context, current, coming);
    }

    /* 3. Update result. */
    coming_device = bt_sink_srv_state_manager_get_device_by_psedev(context, coming);

    if (NULL != coming_device) {
        bt_sink_srv_state_manager_get_device_state(context, coming_device, &coming_state);
    } else if (AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP == coming->type ||
               AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP == coming->type) {
#if defined(AIR_BT_SINK_MUSIC_ENABLE)
        bt_sink_srv_music_device_t *music_device
            = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, coming);

        if (NULL != music_device) {
            coming_state.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
            coming_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            bt_sink_srv_memcpy(coming_state.address, music_device->dev_addr, sizeof(bt_bd_addr_t));
        }
#endif
    } else if (AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE == coming->type) {
#if defined(AIR_LE_AUDIO_ENABLE)
        bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(coming);
        bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);

        if (NULL != conn_info) {
            coming_state.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE;
            coming_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            bt_sink_srv_memcpy(coming_state.address, conn_info->peer_addr.addr, sizeof(bt_bd_addr_t));
        }
#endif
    }

    if (BT_SINK_SRV_ALLOW_RESULT_ALLOW == result) {
        if (NULL != coming_device) {
            bt_sink_srv_state_manager_set_focus_device(context, coming_device, true);
        }

        bt_sink_srv_event_callback(
            BT_SINK_SRV_EVENT_PLAYING_DEVICE_CHANGE,
            &coming_state,
            sizeof(bt_sink_srv_device_state_t));
    }

    return result;
}

void bt_sink_srv_state_manager_running_psedev_change(audio_src_srv_handle_t *running)
{
    bt_sink_srv_state_manager_device_t *device = NULL;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL == running || !BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(running->type)) {
        return;
    }

    /* 1. Get device. */
    device = bt_sink_srv_state_manager_get_device_by_psedev(context, running);
    bt_sink_srv_report_id("[Sink][StaMgr]running psedev change, device: 0x%x", 1, device);

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
    if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role() &&
        BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()) {
        return;
    }
#endif

    /* 2. Swap state. */
    if (NULL != device) {
        bt_sink_srv_state_manager_set_focus_device(context, device, true);
        bt_sink_srv_state_manager_notify_state_change(device->type, BT_SINK_SRV_STATE_NONE);
    }
}

bt_status_t bt_sink_srv_get_playing_device_state(bt_sink_srv_device_state_t *device_state)
{
    bt_sink_srv_state_manager_device_t *device = NULL;
    const audio_src_srv_handle_t *psedev = audio_src_srv_get_runing_pseudo_device();
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL != psedev && BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(psedev->type)) {
        device = bt_sink_srv_state_manager_get_device_by_psedev(
                     context, (audio_src_srv_handle_t *)psedev);

        if (NULL != device) {
            bt_sink_srv_state_manager_get_device_state(context, device, device_state);
            return BT_STATUS_SUCCESS;
        }
    }

    return BT_STATUS_FAIL;
}

static bool bt_sink_srv_state_manager_is_compare_waiting(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current)
{
    (void)context;
    return (current != audio_src_srv_get_runing_pseudo_device());
}

static bt_sink_srv_allow_result_t bt_sink_srv_state_manager_compare_waiting(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming)
{
    uint32_t cases = 0x00;
    bt_sink_srv_allow_result_t result = BT_SINK_SRV_ALLOW_RESULT_BYPASS;

    do {
        bt_sink_srv_state_manager_device_t *current_device
            = bt_sink_srv_state_manager_get_device_by_psedev(context, current);

        bt_sink_srv_state_manager_device_t *coming_device
            = bt_sink_srv_state_manager_get_device_by_psedev(context, coming);

        bt_sink_srv_report_id("[Sink][StaMgr]compare waiting, current_device: 0x%x, coming_device: 0x%x", 2,
                              current_device, coming_device);

#if defined(AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE)
        if (NULL != current_device && current_device == coming_device) {
            if (current == current_device->dummy_device) {
                cases = 0x11;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                break;
            }

            if (coming == coming_device->dummy_device) {
                cases = 0x12;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                break;
            }
        }
#endif

        if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(current_device) &&
            !BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(coming_device)) {
            cases = 0x21;
            result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
            break;
        }

        if (!BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(current_device) &&
            BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(coming_device)) {
            cases = 0x22;
            result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
            break;
        }

        if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(current_device) &&
            BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(coming_device)) {
            if (context->focus_device == coming_device) {
                cases = 0x23;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                break;
            } else {
                cases = 0x24;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                break;
            }
        }

        bt_sink_srv_state_manager_play_count_t current_count
            = bt_sink_srv_state_manager_get_waiting_play_count(context, current);

        bt_sink_srv_state_manager_play_count_t coming_count
            = bt_sink_srv_state_manager_get_waiting_play_count(context, coming);

        bt_sink_srv_report_id("[Sink][StaMgr]compare waiting, current_count: %d, coming_count: %d", 2,
                              current_count, coming_count);

        if (BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_INVALID != current_count &&
            BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_INVALID != coming_count) {
            if (current_count < coming_count) {
                cases = 0x41;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                break;
            } else {
                cases = 0x42;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                break;
            }
        }
    } while (0);

    bt_sink_srv_report_id("[Sink][StaMgr]compare waiting, cases: 0x%x, result: 0x%x", 2,
                          cases, result);

    return result;
}

static bt_sink_srv_state_manager_play_count_t bt_sink_srv_state_manager_get_waiting_play_count(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *psedev)
{
    bt_sink_srv_state_manager_play_count_t play_count
        = BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_INVALID;

    bt_sink_srv_state_manager_device_t *device
        = bt_sink_srv_state_manager_get_device_by_psedev(context, psedev);

    switch (psedev->type) {
        case AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP:
        case AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP: {
            if (NULL != device) {
                bt_sink_srv_state_manager_music_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT,
                    &device->address,
                    &play_count);
            } else {
                bt_sink_srv_music_device_t *music_device
                    = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, psedev);

                if (NULL != music_device) {
                    bt_sink_srv_state_manager_music_callback(
                        BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT,
                        &music_device->dev_addr,
                        &play_count);
                }
            }
            break;
        }

        case AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP: {
            if (NULL != device) {
                bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT,
                    &device->address,
                    &play_count);
            }
            break;
        }

        case AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE: {
#if defined(AIR_LE_AUDIO_ENABLE)
            if (NULL != device) {
                bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT,
                    &device->address,
                    &play_count);
            } else {
                bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(psedev);
                bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
                bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);

                if (NULL != conn_info) {
                    bt_sink_srv_state_manager_le_callback(
                        BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT,
                        &conn_info->peer_addr.addr,
                        &play_count);
                }
            }
#endif
            break;
        }

        default: {
            break;
        }
    }

    return play_count;
}

static bt_sink_srv_allow_result_t bt_sink_srv_state_manager_compare_interrupt(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming)
{
    uint32_t cases = 0x00;
    bt_sink_srv_allow_result_t result = BT_SINK_SRV_ALLOW_RESULT_BYPASS;
    bt_sink_srv_device_state_t current_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;
    bt_sink_srv_device_state_t coming_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;

    bt_sink_srv_state_manager_device_t *current_device
        = bt_sink_srv_state_manager_get_device_by_psedev(context, current);

    bt_sink_srv_state_manager_device_t *coming_device
        = bt_sink_srv_state_manager_get_device_by_psedev(context, coming);

    bt_sink_srv_report_id("[Sink][StaMgr]compare interrupt, current_device: 0x%x, coming_device: 0x%x", 2,
                          current_device, coming_device);

    if (NULL != current_device) {
        bt_sink_srv_state_manager_get_device_state(context, current_device, &current_state);
    }

    if (NULL != coming_device) {
        bt_sink_srv_state_manager_get_device_state(context, coming_device, &coming_state);
    } else if (AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP == coming->type ||
               AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP == coming->type) {
#if defined(AIR_BT_SINK_MUSIC_ENABLE)
        bt_sink_srv_music_device_t *music_device
            = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, coming);

        if (NULL != music_device) {
            coming_state.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
            coming_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            bt_sink_srv_memcpy(coming_state.address, music_device->dev_addr, sizeof(bt_bd_addr_t));
        }
#endif
    } else if (AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE == coming->type) {
#if defined(AIR_LE_AUDIO_ENABLE)
        bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_mode_by_audio_handle(coming);
        bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);

        if (NULL != conn_info) {
            coming_state.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE;
            coming_state.music_state = BT_SINK_SRV_STATE_STREAMING;
            bt_sink_srv_memcpy(coming_state.address, conn_info->peer_addr.addr, sizeof(bt_bd_addr_t));
        }
#endif
    }

    do {
        result = bt_customer_config_allow_play(&current_state, &coming_state);
        bt_sink_srv_report_id("[Sink][StaMgr]compare interrupt, customer result: 0x%x", 1, result);

        if (BT_SINK_SRV_ALLOW_RESULT_BYPASS != result) {
            cases = 0x11;
            break;
        }

#if defined(MTK_AWS_MCE_ENABLE)
        if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE == current_state.type &&
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == coming_state.type &&
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != coming_state.sco_state) {
                cases = 0x21;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                break;
            }
        }
#endif

#if defined(AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE)
        if (NULL != current_device && current_device == coming_device) {
            if (current == current_device->dummy_device) {
                cases = 0x41;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                break;
            }

            if (coming == coming_device->dummy_device) {
                cases = 0x42;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                break;
            }
        }
#endif

        if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(current_device)) {
            if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(coming_device)) {
                if (context->focus_device == coming_device) {
                    cases = 0x81;
                    result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
                    break;
                } else {
                    cases = 0x82;
                    result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW; /* Call cannot interrupt call. */
                    break;
                }
            } else {
                cases = 0x83;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW; /* Music cannot interrupt call. */
                break;
            }
        } else {
            if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(coming_device)) {
                cases = 0x84;
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW; /* Call can interrupt music. */
                break;
            } else {
                cases = 0x85;
                result = BT_SINK_SRV_ALLOW_RESULT_DISALLOW; /* Music cannot interrupt music. */
                break;
            }
        }
    } while (0);

    bt_sink_srv_report_id("[Sink][StaMgr]compare interrupt, case: 0x%x result: 0x%x", 2, cases, result);
    return result;
}

static bt_sink_srv_allow_result_t default_bt_customer_config_allow_play(
    bt_sink_srv_device_state_t *current,
    bt_sink_srv_device_state_t *coming)
{
    return BT_SINK_SRV_ALLOW_RESULT_BYPASS;
}
