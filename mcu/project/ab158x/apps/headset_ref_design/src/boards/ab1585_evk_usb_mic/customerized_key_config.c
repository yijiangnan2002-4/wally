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

#include "apps_config_key_remapper.h"
#include "hal_keypad_table.h"

const apps_config_configurable_event_status_table_t s_key_config_event_sta_maps[] = {
    {
        KEY_AVRCP_PLAY,
        (1 << APP_CONNECTED)
    },
    {
        KEY_AVRCP_PAUSE,
        (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
    },
    {
        KEY_AVRCP_FORWARD,
        (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
    },
    {
        KEY_AVRCP_BACKWARD,
        (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
    },
    {
        KEY_VOICE_UP,
        (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING) | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        KEY_VOICE_DN,
        (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING) | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        KEY_WAKE_UP_VOICE_ASSISTANT,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_WAKE_UP_VOICE_ASSISTANT_CONFIRM,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY, /* To support play a "press" VP to notify user long press time, but confirm the action after user release the key */
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING)
        | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_INTERRUPT_VOICE_ASSISTANT, /* Interrupt VA via short click. */
        (1 << APP_STATE_VA)
    },
#ifdef AIR_MULTI_POINT_ENABLE
    {

        KEY_SWITCH_ANC_AND_PASSTHROUGH,
        (1 << APP_BT_OFF) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
#else
    {
        KEY_SWITCH_ANC_AND_PASSTHROUGH,
        (1 << APP_BT_OFF) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
#endif
    {
        KEY_VA_XIAOWEI_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_VA_XIAOWEI_START_NOTIFY,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_VA_XIAOAI_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_VA_XIAOAI_START_NOTIFY,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_AMA_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_AMA_START_NOTIFY, /* To support play a "press" VP to notify user long press time, but confirm the action after user release the key */
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_GSOUND_PRESS,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING)
    },
    {
        KEY_GSOUND_RELEASE,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_INCOMING) | (1 << APP_HFP_OUTGOING)
        | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_GSOUND_VOICE_QUERY,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_POWER_OFF,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING)
        | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE)
        | (1 << APP_STATE_FIND_ME) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        KEY_ACTION_INVALID,
        0xFFFFFFFF
    },
    {
        KEY_PASS_THROUGH,
        (1 << APP_BT_OFF) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        KEY_ANC,
        (1 << APP_BT_OFF) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        KEY_GSOUND_NOTIFY,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
    {
        KEY_GSOUND_CANCEL,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
    {
        KEY_AUDIO_PEQ_SWITCH,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
    {
        KEY_AMA_LONG_PRESS_TRIGGER_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_AMA_LONG_PRESS_TRIGGER_STOP,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY) | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
    {
        KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
    {
        KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP,
        (1 << APP_STATE_VA)
    },
    {
        KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
    {
        KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_STOP,
        (1 << APP_STATE_VA)
    },
    {
        KEY_SHARE_MODE_SWITCH,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
    {
        KEY_SHARE_MODE_FOLLOWER_SWITCH,
        (1 << APP_CONNECTABLE) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    }
};

const apps_config_key_event_map_t temp_key_release_configs[] = {
    {
        EINT_KEY_0,
        KEY_AVRCP_FAST_FORWARD_RELEASE,
        (1 << APP_A2DP_PLAYING)
    },
    {
        EINT_KEY_1,
        KEY_AVRCP_FAST_REWIND_RELEASE,
        (1 << APP_A2DP_PLAYING)
    },
};

const apps_config_key_event_map_t temp_key_short_click_configs[] = {
    {
        DEVICE_KEY_POWER,
        KEY_ACCEPT_CALL,
        (1 << APP_HFP_INCOMING)
    },
    {
        DEVICE_KEY_POWER,
        KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        (1 << APP_HFP_TWC_INCOMING)
    },
    {
        DEVICE_KEY_POWER,
        KEY_END_CALL,
        (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_MULTIPARTY_CALL)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_HFP_TWC_OUTGOING)
    },
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    {
        DEVICE_KEY_POWER,
        KEY_LE_AUDIO_BIS_NEXT,
        (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
#endif
    {
        EINT_KEY_0,
        KEY_VOICE_UP,
        (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING) | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
    {
        EINT_KEY_1,
        KEY_VOICE_DN,
        (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING) | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE)
        | (1 << APP_HFP_TWC_INCOMING) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_STATE_HELD_ACTIVE) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
};

const apps_config_key_event_map_t temp_key_double_click_configs[] = {
    {
        DEVICE_KEY_POWER,
        KEY_REJCALL,
        (1 << APP_HFP_INCOMING)
    },
    {
        DEVICE_KEY_POWER,
        KEY_ONHOLD_CALL,
        (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_STATE_HELD_ACTIVE)
    },
    {
        DEVICE_KEY_POWER,
        KEY_REJCALL_SECOND_PHONE,
        (1 << APP_HFP_TWC_INCOMING)
    },
    {
        DEVICE_KEY_POWER,
        KEY_DISCOVERABLE,
#if defined(AIR_MULTI_POINT_ENABLE) || defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_LE_AUDIO_BIS_PLAYING)
#else
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
#endif
    },
#ifdef AIR_MCSYNC_SHARE_ENABLE
    {
        EINT_KEY_0,
        KEY_SHARE_MODE_SWITCH,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
    {
        EINT_KEY_1,
        KEY_SHARE_MODE_FOLLOWER_SWITCH,
        (1 << APP_CONNECTABLE) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
#else
    {
        EINT_KEY_0,
        KEY_DEVICE_SWITCH_ACTIVE,
        (1 << APP_CONNECTED)
    }

#endif
};

const apps_config_key_event_map_t temp_key_triple_click_configs[] = {
#if defined(MTK_AWS_MCE_ENABLE)
    {
        DEVICE_KEY_POWER,
        KEY_AIR_PAIRING,
        (1 << APP_DISCONNECTED)
    },
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    {
        DEVICE_KEY_POWER,
        KEY_LE_AUDIO_BIS_SCAN,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED)
    },
    {
        DEVICE_KEY_POWER,
        KEY_LE_AUDIO_BIS_STOP,
        (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
#endif
#if defined(SUPPORT_ROLE_HANDOVER_SERVICE) && defined(MTK_AWS_MCE_ENABLE)
    {
        DEVICE_KEY_POWER,
        KEY_RHO_TO_AGENT,
        (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING)
    },
#endif
#ifdef APPS_LINE_IN_SUPPORT
    {
        EINT_KEY_0,
        KEY_LINE_IN_SWITCH,
        0xffffffff /* For all key event state, only handle the line-in switch */
    },
#endif /* APPS_LINE_IN_SUPPORT */
#ifdef AIR_SPOTIFY_TAP_ENABLE
    {
        EINT_KEY_1,
        KEY_SPOTIFY_TAP_TRIGGER,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
#endif /* AIR_SPOTIFY_TAP_ENABLE */
};

const apps_config_key_event_map_t temp_key_long_press1_configs[] = {
    {
        EINT_KEY_0,
        KEY_AVRCP_FAST_FORWARD_PRESS,
        (1 << APP_A2DP_PLAYING)
    },
    {
        EINT_KEY_1,
        KEY_AVRCP_FAST_REWIND_PRESS,
        (1 << APP_A2DP_PLAYING)
    },
};

const apps_config_key_event_map_t temp_key_long_press2_configs[] = {

};

const apps_config_key_event_map_t temp_key_long_press3_configs[] = {
    {
        DEVICE_KEY_POWER,
        KEY_POWER_ON,
        (1 << APP_BT_OFF)
    },
    {
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING)
        | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE)
        | (1 << APP_STATE_FIND_ME) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
        | (1 << APP_LE_AUDIO_BIS_PLAYING) | (1 << APP_STATE_VA)
    },
};

const apps_config_key_event_map_t temp_key_slong_configs[] = {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    {
        DEVICE_KEY_POWER,
        KEY_ULL_AIR_PAIRING,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
#else
    {
        DEVICE_KEY_POWER,
        KEY_FACTORY_RESET_AND_POWEROFF,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED)
    },
#endif
    {
        DEVICE_KEY_POWER,
        KEY_AUDIO_PEQ_SWITCH,
        (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING)
        | (1 << APP_LE_AUDIO_BIS_PLAYING)
    },
};

const apps_config_key_event_map_t temp_key_dlong_configs[] = {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    {
        DEVICE_KEY_POWER,
        KEY_ULL_RECONNECT,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING)
    }
#endif
};

const apps_config_key_event_map_t temp_key_repeat_configs[] = {

};

const apps_config_key_event_map_t temp_key_press_configs[] = {

};

const apps_config_key_event_map_t temp_key_long_press_release1_configs[] = {

};

const apps_config_key_event_map_t temp_key_long_press_release2_configs[] = {

};

const apps_config_key_event_map_t temp_key_long_press_release3_configs[] = {

};

const apps_config_key_event_map_t temp_key_slong_release_configs[] = {

};

const apps_config_key_event_map_t temp_key_dlong_release_configs[] = {

};

#define temp_left_key_release_configs temp_key_release_configs
#define temp_left_key_short_click_configs temp_key_short_click_configs
#define temp_left_key_double_click_configs temp_key_double_click_configs
#define temp_left_key_triple_click_configs temp_key_triple_click_configs
#define temp_left_key_long_press1_configs temp_key_long_press1_configs
#define temp_left_key_long_press2_configs temp_key_long_press2_configs
#define temp_left_key_long_press3_configs temp_key_long_press3_configs
#define temp_left_key_slong_configs temp_key_slong_configs
const apps_config_key_event_map_t temp_left_key_dlong_configs[] = {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    {
        DEVICE_KEY_POWER,
        KEY_ULL_SWITCH_LINK_MODE,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
        | (1 << APP_ULTRA_LOW_LATENCY_PLAYING),
    }
#endif
};
#define temp_left_key_repeat_configs temp_key_repeat_configs
#define temp_left_key_press_configs temp_key_press_configs
#define temp_left_key_long_press_release1_configs temp_key_long_press_release1_configs
#define temp_left_key_long_press_release2_configs temp_key_long_press_release2_configs
#define temp_left_key_long_press_release3_configs temp_key_long_press_release3_configs
#define temp_left_key_slong_release_configs temp_key_slong_release_configs
#define temp_left_key_dlong_release_configs temp_key_dlong_release_configs

// For right side or default configurable mapping table
const static apps_config_configurable_table_t default_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_FORWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,
        DEVICE_KEY_POWER,
        KEY_WAKE_UP_VOICE_ASSISTANT_CONFIRM,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_2,
        DEVICE_KEY_POWER,
        KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY, /* To support play a "press" VP to notify user long press time, but confirm the action after user release the key */
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_INTERRUPT_VOICE_ASSISTANT, /* Interrupt VA via short click. */
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
};

const static apps_config_configurable_table_t left_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_SWITCH_ANC_AND_PASSTHROUGH,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },

    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,
        DEVICE_KEY_POWER,
        KEY_WAKE_UP_VOICE_ASSISTANT_CONFIRM,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_2,
        DEVICE_KEY_POWER,
        KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY, /* To support play a "press" VP to notify user long press time, but confirm the action after user release the key */
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_INTERRUPT_VOICE_ASSISTANT, /* Interrupt VA via short click. */
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
};

#ifdef AIR_XIAOWEI_ENABLE
const static apps_config_configurable_table_t xiaowei_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_FORWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    /**
     * @brief Default tap to trigger xiaowei
     */
    {
        APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOWEI_START,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_2,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOWEI_START_NOTIFY,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        0
    },
#if 0
    /**
     * @brief for Xiaowei long press trigger.
     * When long press trigger enabled, the power off operation should be disabled.
     * #APPS_CONFIG_KEY_LONG_PRESS_3
     */
    {
        APPS_CONFIG_KEY_LONG_PRESS_1,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING) | (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE)
    },
    {
        APPS_CONFIG_KEY_RELEASE,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_STOP,
        (1 << APP_STATE_VA)
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_ACTION_INVALID,
        (1 << APP_DISCONNECTED) | (1 << APP_CONNECTABLE) | (1 << APP_CONNECTED) | (1 << APP_HFP_INCOMING)
        | (1 << APP_HFP_OUTGOING) | (1 << APP_HFP_CALL_ACTIVE) | (1 << APP_HFP_CALL_ACTIVE_WITHOUT_SCO) | (1 << APP_HFP_TWC_INCOMING)
        | (1 << APP_HFP_TWC_OUTGOING) | (1 << APP_HFP_MULTIPARTY_CALL) | (1 << APP_A2DP_PLAYING) | (1 << APP_STATE_HELD_ACTIVE)
        | (1 << APP_STATE_FIND_ME) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING)
    },
#endif
};
#endif /* AIR_XIAOWEI_ENABLE */

#ifdef MTK_VA_XIAOAI_ENABLE
const static apps_config_configurable_table_t xiaoai_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_FORWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
#if 0
    /**
     * @brief for XiaoAI long press trigger.
     * When long press trigger enabled, the power off operation should be disabled.
     * #APPS_CONFIG_KEY_LONG_PRESS_3
     */
    {
        APPS_CONFIG_KEY_LONG_PRESS_1,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START,
        (1 << APP_CONNECTED) | (1 << APP_A2DP_PLAYING)
    },
    {
        APPS_CONFIG_KEY_RELEASE,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP,
        (1 << APP_STATE_VA)
    },
#else
    /**
     * @brief Default tap to trigger XiaoAI
     */
    {
        APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOAI_START,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_2,
        DEVICE_KEY_POWER,
        KEY_VA_XIAOAI_START_NOTIFY,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        0
    },
#endif
};
#endif /* MTK_VA_XIAOAI_ENABLE */

#ifdef AIR_AMA_ENABLE

const static apps_config_configurable_table_t ama_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_FORWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0 /* Set to 0, it will be set in apps_config_key_remaper_init_configurable_table by the s_key_config_event_sta_maps */
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,
        DEVICE_KEY_POWER,
        KEY_AMA_START,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_2,
        DEVICE_KEY_POWER,
        KEY_AMA_START_NOTIFY, /* To support play a "press" VP to notify user long press time, but confirm the action after user release the key */
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_POWER_OFF,
        0
    },
};

#endif /* AIR_AMA_ENABLE */

#ifdef AIR_GSOUND_ENABLE

const static apps_config_configurable_table_t gsound_configurable_table[] = {
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PLAY,
        0
    },
    {
        APPS_CONFIG_KEY_SHORT_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_PAUSE,
        0
    },
    {
        APPS_CONFIG_KEY_DOUBLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_FORWARD,
        0
    },
    {
        APPS_CONFIG_KEY_TRIPLE_CLICK,
        DEVICE_KEY_POWER,
        KEY_AVRCP_BACKWARD,
        0
    },
    {
        APPS_CONFIG_KEY_PRESS,
        DEVICE_KEY_POWER,
        KEY_GSOUND_PRESS,
        0
    },
    {
        APPS_CONFIG_KEY_RELEASE,
        DEVICE_KEY_POWER,
        KEY_GSOUND_RELEASE,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_1,
        DEVICE_KEY_POWER,
        KEY_GSOUND_VOICE_QUERY,
        0
    },
    {
        APPS_CONFIG_KEY_LONG_PRESS_3,
        DEVICE_KEY_POWER,
        KEY_ACTION_INVALID,
        0
    },
};

#endif /* AIR_GSOUND_ENABLE */
