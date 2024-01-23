/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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
 * File: apps_events_interaction_event.h
 *
 * Description: This file defines the enum of event ids of EVENT_GROUP_UI_SHELL_APP_INTERACTION group events.
 *
 */

#ifndef __APPS_EVENTS_INTERACTION_EVENT_H__
#define __APPS_EVENTS_INTERACTION_EVENT_H__

/** @brief
 * This enum defines the event ids of EVENT_GROUP_UI_SHELL_APP_INTERACTION group events.
 */
enum {
    /* The events below is used to support RHO features */
    APPS_EVENTS_INTERACTION_TRIGGER_RHO,    /**< Any APPs can send the event to app_rho_idle_activity to trigger RHO */
    APPS_EVENTS_INTERACTION_RHO_STARTED,    /**< app_rho_idle_activity send the event when RHO started */
    APPS_EVENTS_INTERACTION_RHO_END,        /**< app_rho_idle_activity send the event when RHO ended */
    APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT,    /**< app_home_screen_idle_activity in Partner send the event to Agent for triggering RHO */

    /* The events below is used to support power off, reboot or disable/enable BT */
    APPS_EVENTS_INTERACTION_POWER_OFF_WAIT_TIMEOUT,     /**< app_home_screen_idle_activity send the event to itself with a short delay time when do power off */
    APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF,      /**< Any APPs can send the event to app_home_screen_idle_activity to trigger power off */
    APPS_EVENTS_INTERACTION_REQUEST_IMMEDIATELY_POWER_OFF,  /**< Any APPs can send the event to app_home_screen_idle_activity to trigger power off */

    APPS_EVENTS_INTERACTION_REQUEST_REBOOT,         /**< Any APPs can send the event to app_home_screen_idle_activity to trigger reboot */
    APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,      /**< Any APPs can send the event to app_home_screen_idle_activity to enable or disable BT */
    APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF,  /**< Any APPs can send the event to app_home_screen_idle_activity to disable EDR */
    APPS_EVENTS_INTERACTION_CLASSIC_OFF_TO_BT_OFF,  /**< When Classic BT OFF, send the event to trigger a timer to start BT OFF */
    APPS_EVENTS_INTERACTION_REQUEST_OFF_BT_WO_RHO,

    /* The events below is used to support sleep after no connection and wait long enough */
    APPS_EVENTS_INTERACTION_REFRESH_SLEEP_TIME,         /**< app_home_screen_idle_activity in Partner send the event to Agent for trigger it reset sleep timer */
    APPS_EVENTS_INTERACTION_GO_TO_SLEEP,        /**< app_home_screen_idle_activity send the event to itself with a delay time to trigger sleep flow.
                                                   The agent sends the event to partner to make partner sleep at the same time. */
    APPS_EVENTS_INTERACTION_SLEEP_WAIT_TIMEOUT, /**< app_home_screen_idle_activity send the event to itself with a short delay time to sleep. */

    /* The events below is used to support update states */
    APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE,   /**< Activities update mmi state when it receive the event. If an activity want to set MMI state, it should
                                                   returns true when receives the event to avoid next activity receives the event. */
    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN,  /**< Activities update LED background pattern when it receive the event. If an activity want to set LED background
                                                       pattern, it should returns true when receives the event to avoid next activity receives the event. */
    /* The events below is used to support find me feature */
    APPS_EVENTS_INTERACTION_FINISH_FIND_ME, /**< Reserved, not useful now. */
    APPS_EVENTS_INTERACTION_PLAY_FIND_ME_TONE,  /**< To play find me tone continually, app_fm_activity send the event to itself to trigger play voice prompt with a delay */
    /* The events below is used to support GSOUND feature */
    APPS_EVENTS_INTERACTION_GSOUND_ACTION_REJECTED, /**< Some key actions (e.g. KEY_AVRCP_PLAY) need be processed by GSOUND lib. If GSOUND lib ignore the events, it sends
                                                       the event. */
    APPS_EVENTS_INTERACTION_INCREASE_BLE_ADV_INTERVAL, /**< Use a small interval in first 30 seconds, and bt_app_common sends the interval to ask APPs to stop ble adv
                                                        when timeout. */
    APPS_EVENTS_INTERACTION_RELOAD_KEY_ACTION_FROM_NVKEY,   /**< Read NVkey to refresh the key-action table */
    APPS_EVENTS_INTERACTION_BT_VISIBLE_STATE_CHANGE,    /**< Agent send current bt visible state to partner */
    APPS_EVENTS_INTERACTION_SET_BT_VISIBLE,   /**< Partner send the aws event to agent to start bt visible */
    APPS_EVENTS_INTERACTION_AUTO_START_BT_VISIBLE,  /**< Send the event to start auto bt visible with a delay time */
    APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT, /**< Send the event to stop bt visible with a delay time */
    APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY,      /**< Send the event to notify BT visible state */
    APPS_EVENTS_INTERACTION_BT_RECONNECT_TIMEOUT,   /**< Send the event to stop bt reconnect with a delay time */
    APPS_EVENTS_INTERACTION_BT_RECONNECT_DEVICE,    /**< Send the event to reconnecte the reconnected device. */
    APPS_EVENTS_INTERACTION_BT_RETRY_POWER_ON_OFF,  /**< Send the event when call BT standby or active fail, need retry. */

    APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT,  /**< The driver callbac will send this event when in-ear status changed. */
    APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA, /**< The in-ear app will send this event when in-ear status changed. */
    APPS_EVENTS_INTERACTION_IN_EAR_CFG_UPDATE, /**< The in-ear's configuration updated. */

    APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER,   /**< This event come from race cmd handler */
    APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP,           /**< This event is used for vp */
    APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_STOP,         /**< This event is used to notify LD stopped. */
    APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_STATUS,       /**< This event is used to notify LD status. */

    APPS_EVENTS_INTERACTION_SWITCH_AUDIO_PATH,  /* This event is used for audio path switch*/
    APPS_EVENTS_INTERACTION_LINE_IN_PLUG_STATE,  /* This event is notify line in plug in or out. */
    APPS_EVENTS_INTERACTION_USB_AUDIO_OP,    /* app_usb_audio_idle_activity send the event to self to trigger the usb audio start or stop. */
    APPS_EVENTS_INTERACTION_USB_PLUG_STATE, /* This event is notify USB plug in or out. */

    APPS_EVENTS_INTERACTION_POWER_OFF,      /**< app_home_screen_idle_activity send the event when the system power off. */
    APPS_EVENTS_INTERACTION_MULTI_VA_REMOVE_PAIRING_DONE,   /**< Other app call multi_voice_assistant_manager_va_remove_pairing() and wait the event to do next action */
    APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST,  /**< This event is used for factory reset. */
    APPS_EVENTS_INTERACTION_SHARE_MODE_STA_UPDATE,  /* This event indicates that the state of share mode has changed. */
    APPS_EVENTS_INTERACTION_PLAY_SYNC_VP,    /**< Timer to wait AWS connect and play sync VP in __CONN_VP_SYNC_STYLE_TWO__. */
    APPS_EVENTS_INTERACTION_PLAY_VP,    /**< To play a VP for testing. */
    APPS_EVENTS_INTERACTION_SIMULATE_TIMER,     /**< This event simulate timer, user can register callback when sending it. */
    APPS_EVENTS_INTERACTION_SET_LED,    /**< To set an LED pattern for testing. */
    APPS_EVENTS_INTERACTION_TEST_BLE_ADV,    /* To start/stop a test ble adv */
    APPS_EVENTS_INTERACTION_MULTIPOINT_SWITCH_AWS,  /* This event is a timer for partner multipoint aws switch */
    APPS_EVENTS_INTERACTION_SYNC_HFP_STATE_TO_PARTNER,  /* This event  for agent hfp state to partner */
    APPS_EVENTS_INTERACTION_LE_AUDIO_ADV,     /* This event is used to notify start/stop LE Aduio ADV */
    APPS_EVENTS_INTERACTION_ADAPTIVE_ANC_STATE_CHANGE,    /* Sync app_adaptive_anc state to another ear. */
    APPS_EVENTS_INTERACTION_AUDIO_STATE_CHANGE,   /* The state of AM changed */
    APPS_EVENTS_INTERACTION_INCOMING_CALL_DELAY,  /* This event for agent sync incoming call vp to partner. */
    APPS_EVENTS_INTERACTION_SYNC_INCOMING_CALL_VP, /*This event for agent sync incoming call vp flag to partner. */
    APPS_EVENTS_INTERACTION_SYNC_TOUCH_KEY_STATUS, /*This event for agent sync touch key status to partner. */
    APPS_EVENTS_INTERACTION_PLAY_TWC_INCOMING_CALL_VP, /**< To loop twc incoming call vp playback. */
    APPS_EVENTS_INTERACTION_SYNC_ADDR_LIST, /**< Sync connected addr list to new agent when rho ends. */
    APPS_EVENTS_INTERACTION_AUTO_ACCEPT_INCOMING_CALL, /* This event for auto accept incoming call. */
    APPS_EVENTS_INTERACTION_SYNC_AUTO_ACCEPT_STATUS,   /* This event for sync hfp auto accept incoming call CFG . */
    APPS_EVENTS_INTERACTION_SYNC_MUSIC_IN_EAR_CFG,     /* This event for sync music in ear CFG. */
    APPS_EVENTS_INTERACTION_FIND_ME_APP_SYNC_FIND_ME_ACTION,   /* This event for sync find me action. */
    APPS_EVENTS_INTERACTION_ULL_DONGLE_CONNECTION_CHANGE, /**< This event is used for notify ULL dongle connection change. */

    APPS_EVENTS_INTERACTION_SWITCH_MIC, /**< This event is used for switch MIC. */
    APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT,  /**< This event is to indicate the reconnecting timeout in ULL project. */
    APPS_EVENTS_INTERACTION_ULL_LINK_MODE_CHANGE,    /**< This event is to notify the link mode from agent to partner. */
    APPS_EVENTS_INTERACTION_SYNC_MIC_MUTE_STATUS,   /**< This event to sync microphone mute status to peer. */
    APPS_EVENTS_INTERACTION_SILENCE_DETECT_CHANGE,  /**< This event to notify silence detect status change. */
    APPS_EVENTS_INTERACTION_SYNC_ADVANCE_PASSTHROUGH, /**< This event to sync advance passthrough event to partner. */
    APPS_EVENTS_INTERACTION_SYNC_SPEAKER_MUTE_STATUS, /**< This event is used to sync speaker mute state.*/
    APPS_EVENTS_INTERACTION_SYNC_SIDETONE,          /**< This event to sync sidetone to partner. */
#ifdef AIR_TILE_ENABLE
    APPS_EVENTS_INTERACTION_STOP_FAST_PAIR_ADV, /**< This event is used to stop ble adv if tile is ctivated. */
    APPS_EVENTS_INTERACTION_TILE_ADV_TIMEOUT, /**< This event is used to stop tile ble adv if tile is not ativated. */
#endif
    APPS_EVENTS_INTERACTION_AWS_SYNC_REBOOT,    /* Agent and partner reboot sync. */
    APPS_EVENTS_INTERACTION_NOTIFY_FIND_ME_STATE, /**< This event to notify find me state change. */
    APPS_EVENTS_INTERACTION_SET_ULL_MIX_RATIO, /**< This event is used to control mix ratio in ULL activity */
    APPS_EVENTS_INTERACTION_SET_ULL_MIC_VOL, /**< This event is used to control mic vol in ULL activity */
    APPS_EVENTS_INTERACTION_SYNC_VA_STATE,   /**< Agent send the event to partner for sync VA state. */
    APPS_EVENTS_INTERACTION_MULTI_POINT_ENABLE,     /*< Event to set the multi point feature enable. */
    APPS_EVENTS_INTERACTION_MULTI_POINT_STATE_CHANGED, /*< Event to notify multi point state changed. */
    APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD,   /**< Send the event with a timer to clear the record of active switch audio source. */
    APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW,    /**< Send the event with a timer to clear the temp_allow_switch_audio_addr. */
    APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_UPDATE_CONNECTION_STATUS,    /**< Send the event to update connection status. */
    APPS_EVENTS_INTERACTION_SYNC_AEQ_CONTROL_TO_PARTNER,
    APPS_EVENTS_INTERACTION_SYNC_AEQ_INDEX_TO_AGENT,    /**< partner send the adaptive eq index to agent. */
    APPS_EVENTS_INTERACTION_AEQ_START_TIMER,            /*This event to Anti-Shake adaptive eq Quick Start Pause*/
    APPS_EVENTS_INTERACTION_SYNC_BT_AVRCP_STATUS_TO_PEER, /**< Agent sync avrcp status to partner.*/
    APPS_EVENTS_INTERACTION_SYNC_RECORDED_AVRCP_STATUS_TO_PEER,/**< Agent sync the local recorded avrcp status to partner.*/
    APPS_EVENTS_INTERACTION_BATTERY_APP_STATE_CHANGED, /**< This event is battery app state changed. */
    APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR, /**< This event is used to sync takeover LE address. */
    APPS_EVENTS_INTERACTION_SYNC_SELF_FITTING_CONFIG, /**< This event is used to sync the self fitting config. */
	// richard for customer UI spec.
	APPS_EVENTS_INTERACTION_INSIDE_EAR, 				/* Indicate a inside ear event */
	APPS_EVENTS_INTERACTION_OUTSIDE_EAR,				/* Indicate a outside ear event */
	APPS_EVENTS_INTERACTION_PSENSOR_STATUS_UPDATE, 
    APPS_EVENTS_INTERACTION_WIRELESS_MIC_AUDIO_DATA,   /**< This event to send recorder audio data to ui shell task. */
    APPS_EVENTS_INTERACTION_DELAY_PLAY_POWER_ON_VP,     /**< This event to send to delay play the power on VP. */
    APPS_EVENTS_INTERACTION_LE_ULL_CONNECT_TIMEOUT,  /**< This event will be send when ULL connect timeout after adv start. */
    APPS_EVENTS_INTERACTION_WIRELESS_MIC_READY_TO_POWER_OFF, /**< This event for wireless mic to tirgger power off. */
    APPS_EVENTS_INTERACTION_WIRELESS_MIC_VOLUME_DET, /**< This event to trigger wireless mic send volume data. */
    APPS_EVENTS_INTERACTION_USB_IN_RESUME_AFTER_VP, /**< This event will be send when VP play end. */
    APPS_EVENTS_INTERACTION_SET_USB_MODE,   /**< This event is battery app state changed. */
    APPS_EVENTS_INTERACTION_USB_AUDIO_EN, /**< This event is used to enable/disable usb audio. */
//new add
    APPS_EVENTS_INTERACTION_DCHS_NOTIFY_BATTERY_PERCENT,
    APPS_EVENTS_INTERACTION_DCHS_NOTIFY_SHUTDOWN_STATE,
    APPS_EVENTS_INTERACTION_TEAMS_DELAY_TO_RECONNECT, /**< This event is used to reconnect Teams if not handshake done with Teams. */
    APPS_EVENTS_INTERACTION_SYNC_CALL_RX_EQ_STATUS, /**< This event is used to sync the rx eq status of call. */
    APPS_EVENTS_INTERACTION_FAST_PAIR_GENERATE_ADV_ADDR,    /**< Send the event create a timer to generate adv address. */
    APPS_EVENTS_INTERACTION_SPOT_RING_OVER,    /**< Send the event to notify ring over. */
    APPS_EVENTS_INTERACTION_SPOT_GEN_EID_IN_UNWANTED_TRACKING_MODE,    /**< Send the event to gen eid. */
    APPS_EVENTS_INTERACTION_APP_ANC_SRV_ANC_ACTION,                  /**< Send the event to ANC action. */
    APPS_EVENTS_INTERACTION_APP_ANC_SRV_GAIN_ACTION,                  /**< Send the event to GAIN action. */
    APPS_EVENTS_INTERACTION_APP_ANC_SRV_SYNC_INFO,                     /**< Send ANC sync info to partner via APP ANC SRV. */
    APPS_EVENTS_INTERACTION_ULL_APP_SWITCH_VER, /**< Send to switch the version. */
    APPS_EVENTS_INTERACTION_VOICE_ACTIVE_WHILE_MUTED_UPDATE, /**< Send this event while voice active during mic muted. */
    APPS_EVENTS_INTERACTION_ESCO_CRC_RATE, /**< This event is used to report esco crc error cnt. */
    APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR, /**< This event is used to report esco crc error by cycle. */
    APPS_EVENTS_INTERACTION_LINK_QUALITY_ESCO_CRC_READ_ONCE, /**< This event is used to read esco crc once. */
    APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK,           /**< Send the event to update ull2 unlik. */
    APPS_EVENTS_INTERACTION_SWITCH_TO_ULL2_UPLINK, /**< Do the uplink switch. */
    APPS_EVENTS_INTERACTION_SYNC_MUSIC_APP_STREAMING_STATE_TO_PEER, /**< Agent sync music streaming state to partner.*/
    APPS_EVENTS_INTERACTION_TEAMS_DELAY_GET_CONN_NUMS, /**< Get the connected num with debounce. */
    APPS_EVENTS_INTERACTION_UPDATE_TEAMS_LED_BG_PATTERN, /**< Update the teams led pattern */
    APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER,
    APPS_EVENTS_INTERACTION_TAKEOVER_FOR_SASS,
};

#endif /* __APPS_EVENTS_INTERACTION_EVENT_H__ */
