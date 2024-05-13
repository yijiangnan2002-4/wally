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
 * File: apps_config_event_list.h
 *
 * Description: This file defines the enum of key actions.
 *
 */

#ifndef __APPS_EVENT_LIST_H__
#define __APPS_EVENT_LIST_H__

/** @brief
 * This enum defines the key action.
 */
typedef enum {
    KEY_ACTION_INVALID = 0,                         /**< invalid key action id. */

    KEY_ACTION_BASE  = 0x0001,                      /**< Declare event base, all active events must larger than it. */


    KEY_DISCOVERABLE = 0x0002,                      /**< Start BT discoverable. */
    KEY_CANCEL_DISCOVERABLE = 0x0003,               /**< Set bt to invisible. */

    KEY_VOICE_UP = 0x000A,                          /**< Volume up. */
    KEY_VOICE_DN = 0x000B,                          /**< Volume down. */
    KEY_BT_OFF = 0x0016,                            /**< BT off. */
    KEY_POWER_ON = 0x0017,                          /**< It's a SW power on, so power on BT. */
    KEY_POWER_OFF = 0x0018,                         /**< System power off, but if device is not support power key, may do BT off only. */
    KEY_SYSTEM_REBOOT = 0x0019,                     /**< Trigger system reboot. */
    KEY_RESET_PAIRED_DEVICES = 0x001A,              /**< Unpair all devices. */
    KEY_RECONNECT_LAST_DEVICE = 0x001E,             /**< Actively reconnect last device. */

    KEY_WAKE_UP_VOICE_ASSISTANT = 0x0020,           /**< Trigger voice assistant. */
    KEY_WAKE_UP_VOICE_ASSISTANT_CONFIRM = 0x0021,   /**< Used with KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY. When user press
                                                        key long enough and releaes key after he listened the notify VP,
                                                        use the action to trigger voice assistant. */
    KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY = 0x0022, /**< Notify user the press time is enough to trigger voice assistant. */
    KEY_INTERRUPT_VOICE_ASSISTANT = 0x0023,       /**< Interrupt the voice assistant. */
    KEY_ULL_UL_VOL_UP = 0x0024,               /**< ULL UL Volume up. */
    KEY_ULL_UL_VOL_DN = 0x0025,               /**< ULL UL Volume down. */

    KEY_REDIAL_LAST_CALL = 0x0034,          /**< Redail last call. */
    KEY_CANCEL_OUT_GOING_CALL = 0x0037,     /**< Cancel outgoing call. */
    KEY_REJCALL = 0x0038,                   /**< Reject incoming call. */
    KEY_REJCALL_SECOND_PHONE,               /**< Reject the new incoming call in 3-way calling. */
    KEY_ONHOLD_CALL = 0x003A,               /**< Hold current call or resume from hold status. */
    KEY_ACCEPT_CALL = 0x003B,               /**< Accept incoming call. */
    KEY_END_CALL = 0x003C,                  /**< End current call. */

    KEY_SWITCH_AUDIO_PATH = 0x3D,           /**< Switch the audio path of call through Smart phone speaker or headset. */
    KEY_MUTE_MIC = 0x3E,                    /**< Mute or unMute micphone. */
    KEY_WM_CONTROL_LOCAL_RECORDER = 0x3F,    /**< Wireless mic control the local of microphone recorder. */

    KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER = 0x0040, /**< Accept the new incoming call and hold current call. */
    KEY_WM_SWITCH_USB_TYPE            = 0x0041, /**< Wireless mic switch the device type of USB. */
    KEY_AVRCP_PLAY = 0x0053,                    /**< Start or resume music playing. */
    KEY_AVRCP_PAUSE = 0x0055,                   /**< Pause music playing. */
    KEY_AVRCP_FORWARD = 0x005A,                 /**< When playing music, play next track. */
    KEY_AVRCP_BACKWARD = 0x005B,                /**< When playing music, play last track. */
    KEY_AVRCP_FAST_FORWARD_PRESS = 0x005C,      /**< When playing music, fast forward start. */
    KEY_AVRCP_FAST_FORWARD_RELEASE = 0x005D,    /**< When playing music, fast forward end. */
    KEY_AVRCP_FAST_REWIND_PRESS = 0x005E,       /**< When playing music, fast rewind start. */
    KEY_AVRCP_FAST_REWIND_RELEASE = 0x005F,     /**< When playing music, fast rewind end. */

    KEY_PASS_THROUGH = 0x0090, /**< Passthrough on and off */
    KEY_ANC = 0x0091,          /**< ANC on and off */
    KEY_SWITCH_ANC_AND_PASSTHROUGH = 0x0092,  /**< Switch sequence is: All off->Passthrough on->ANC on */
    KEY_BETWEEN_ANC_PASSTHROUGH = 0x0093,   /**< Switch between ANC on and Passthrough on. */
    KEY_ADVANCED_PASSTHROUGH_SWITCH = 0x0094,   /**< Switch hearing aid. */
    KEY_FACTORY_RESET = 0x0095,             /**< Do factory reset and reboot. */
    KEY_FACTORY_RESET_AND_POWEROFF = 0x96,  /**< Do factory reset and power off. */
    KEY_AIR_PAIRING = 0x0097,           /**< Start Air pairing. */
    KEY_STOP_FIND_ME = 0x0098,          /**< Stop the find me ringtone and LED. */
    KEY_ULL_AIR_PAIRING = 0x0099,       /**< Start the pairing between device and ultra low latency dongle. */
    KEY_ULL_RECONNECT = 0x009A,         /**< Reconnect ULL dongle or smart phone. */
    KEY_ULL_SWITCH_LINK_MODE = 0x009B,  /**< Switch the ULL link mode */
    KEY_ULL_SWITCH_GAME_MODE = 0x009C,  /**< Switch the ULL gaming mode */
    KEY_AIR_UNPAIRING = 0x009E,         /**< Start AWS air unpairing. */

    KEY_ANC_GAIN = 0x00A0,              /**< ANC Gain change */
    KEY_ANC_ON = 0x00A1,                /**< ANC ON */
    KEY_PASSTHROUGH_ON = 0x00A2,         /**< Passthrough ON */
    KEY_ANC_OFF = 0x00A3,               /**< ANC and Passthrough OFF */
    KEY_GAMEMODE_ON = 0x00A4,           /**< Game mode off. */
    KEY_GAMEMODE_OFF = 0x00A5,          /**< Game mode on. */
    KEY_GAMEMODE_TOGGLE = 0x00A6,       /**< Game mode on off switch. */
    KEY_LE_AUDIO_SCAN = 0x00A8,         /**< LE audio dongle start scan. */
    KEY_LE_ULL_PAIRING = 0x00A9,        /**< LE ULL 2.0 dongle start pairing new headset. */
    KEY_DONGLE_COMMON_PAIRING = 0x00AA, /**< LE ULL + LEA + BT dongle start pairing. */

    KEY_GSOUND_ENDPOINTING = 0x00FF,    /**< GSound TTT tap to talk. */
    KEY_GSOUND_PRESS = 0x0100,          /**< GSound special design, must receive press event to process voice query. */
    KEY_GSOUND_RELEASE = 0x0101,        /**< GSound special design, must receive release event to process voice query. */
    KEY_GSOUND_NOTIFY = 0x0102,         /**< GSound trigger notify. */
    KEY_GSOUND_VOICE_QUERY = 0x0103,    /**< GSound trigger voice query. */
    KEY_GSOUND_CANCEL = 0x0104,         /**< GSound cancel notify. */

    KEY_AMA_START = 0x0105,             /**< AMA tap trigger mode, notify user is pressed long enough to trigger. */
    KEY_AMA_START_NOTIFY = 0x0106,      /**< AMA tap trigger mode, start recording audio. */
    KEY_AMA_STOP = 0x0107,              /**< AMA tap trigger mode, stop recording audio. */
    KEY_AMA_MEDIA_CONTROL = 0x0108,

    KEY_RHO_TO_AGENT = 0x010A,          /**< Do RHO to make Agent switch to the pressed side. */
    KEY_RESET_LINK_KEY = 0x010B,        /**< Clear link key. */

    /**
     * @brief Add VA AMA key event support - long press trigger mode
     *
     */
    KEY_AMA_LONG_PRESS_TRIGGER_START = 0x011A,  /**< AMA hold trigger mode, start recording audio. */
    KEY_AMA_LONG_PRESS_TRIGGER_STOP = 0x011B,   /**< AMA hold trigger mode, stop recording audio. */

    KEY_VA_XIAOAI_START = 0x010F,           /**< XiaoAi tap trigger mode, notify user is pressed long enough to trigger. */
    KEY_VA_XIAOAI_START_NOTIFY = 0x0110,    /**< XiaoAi tap trigger mode, start recording audio. */
    KEY_VA_XIAOAI_STOP_PLAY = 0x0111,       /**< XiaoAi tap trigger mode, stop recording audio. */
    /* 0x0113 was used in UT APP.*/
    KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START = 0x0115,   /**< XiaoAi hold trigger mode, start recording audio. */
    KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP = 0x0116,    /**< XiaoAi hold trigger mode, stop recording audio. */

    /**
     * @brief Add VA xiaowei key event support
     *
     */
    KEY_VA_XIAOWEI_START = 0x0120,                      /**< XiaoWei tap trigger mode, notify user is pressed long enough to trigger. */
    KEY_VA_XIAOWEI_START_NOTIFY = 0x0121,               /**< XiaoWei tap trigger mode, start recording audio. */
    KEY_VA_XIAOWEI_STOP_PLAY = 0x0122,                  /**< XiaoWei tap trigger mode, stop recording audio. */
    KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_START = 0x0123,   /**< XiaoWei hold trigger mode, start recording audio. */
    KEY_VA_XIAOWEI_LONG_PRESS_TRIGGER_STOP = 0x0124,    /**< XiaoWei hold trigger mode, stop recording audio. */

    KEY_SHARE_MODE_SWITCH = 0x0130,             /* Switch share mode. */
    KEY_SHARE_MODE_FOLLOWER_SWITCH = 0x0131,    /* Switch share mode follower. */

    KEY_MS_TEAMS_BTN_INVOKE = 0x0140,         /* Teams BTN press. */
    KEY_MS_TEAMS_BTN_RELEASE = 0x0141,    /* Teams BTN release. */
    KEY_MS_TEAMS_BTN_LONG_PRESS = 0x0142, /* Teams long press. */

    KEY_SPOTIFY_TAP_TRIGGER = 0x014A, /* Spotify tap trigger */

    KEY_LE_AUDIO_BIS_SCAN = 0x0150,             /* LE_Audio BIS Scan. */
    KEY_LE_AUDIO_BIS_STOP = 0x0151,             /* LE_Audio BIS Stop. */
    KEY_LE_AUDIO_BIS_NEXT = 0x0152,             /* LE_Audio BIS Next. */

    KEY_HEAR_THROUGH_TOGGLE                             = 0x015F, /**< Hear Through Toggle */

    KEY_HEARING_AID_BEGIN                               = 0x0160,
    KEY_HEARING_AID_HA_TOGGLE                           = KEY_HEARING_AID_BEGIN,        /**< Hearing Aid Toggle, 0x0160 */
    KEY_HEARING_AID_LEVEL_UP_NOTIFY                     = KEY_HEARING_AID_BEGIN + 1,    /**< Hearing Aid Level Up Notify, 0x0161 */
    KEY_HEARING_AID_LEVEL_UP                            = KEY_HEARING_AID_BEGIN + 2,    /**< Hearing Aid Level Up, 0x0162 */
    KEY_HEARING_AID_LEVEL_DOWN_NOTIFY                   = KEY_HEARING_AID_BEGIN + 3,    /**< Hearing Aid Level Down Notify, 0x0163 */
    KEY_HEARING_AID_LEVEL_DOWN                          = KEY_HEARING_AID_BEGIN + 4,    /**< Hearing Aid Level Down, 0x0164 */
    KEY_HEARING_AID_LEVEL_UP_CIRCULAR_NOTIFY            = KEY_HEARING_AID_BEGIN + 5,    /**< Hearing Aid Level Circular Notify, 0x0165 */
    KEY_HEARING_AID_LEVEL_UP_CIRCULAR                   = KEY_HEARING_AID_BEGIN + 6,    /**< Hearing Aid Level Circular, 0x0166 */
    KEY_HEARING_AID_MODE_UP_NOTIFY                      = KEY_HEARING_AID_BEGIN + 7,    /**< Hearing Aid Mode Up Notify, 0x0167 */
    KEY_HEARING_AID_MODE_UP                             = KEY_HEARING_AID_BEGIN + 8,    /**< Hearing Aid Mode Up, 0x0168 */
    KEY_HEARING_AID_MODE_DOWN_NOTIFY                    = KEY_HEARING_AID_BEGIN + 9,    /**< Hearing Aid Mode Down Notify, 0x0169 */
    KEY_HEARING_AID_MODE_DOWN                           = KEY_HEARING_AID_BEGIN + 10,   /**< Hearing Aid Mode Down, 0x016A */
    KEY_HEARING_AID_MODE_UP_CIRCULAR_NOTIFY             = KEY_HEARING_AID_BEGIN + 11,   /**< Hearing Aid Mode Up Circular Notify, 0x016B */
    KEY_HEARING_AID_MODE_UP_CIRCULAR                    = KEY_HEARING_AID_BEGIN + 12,   /**< Hearing Aid Mode Up Circular, 0x016C */
    KEY_HEARING_AID_VOLUME_UP_NOTIFY                    = KEY_HEARING_AID_BEGIN + 13,   /**< Hearing Aid Volume Up Notify, 0x016D */
    KEY_HEARING_AID_VOLUME_UP                           = KEY_HEARING_AID_BEGIN + 14,   /**< Hearing Aid Volume Up, 0x016E */
    KEY_HEARING_AID_VOLUME_DOWN_NOTIFY                  = KEY_HEARING_AID_BEGIN + 15,   /**< Hearing Aid Volume Down Notify, 0x016F */
    KEY_HEARING_AID_VOLUME_DOWN                         = KEY_HEARING_AID_BEGIN + 16,   /**< Hearing Aid Volume Down, 0x0170 */
    KEY_HEARING_AID_VOLUME_UP_CIRCULAR_NOTIFY           = KEY_HEARING_AID_BEGIN + 17,   /**< Hearing Aid Volume Up Circular Notify, 0x0171 */
    KEY_HEARING_AID_VOLUME_UP_CIRCULAR                  = KEY_HEARING_AID_BEGIN + 18,   /**< Hearing Aid Volume Up Circular, 0x0172 */
    KEY_HEARING_AID_TUNING_MODE_TOGGLE                  = KEY_HEARING_AID_BEGIN + 19,   /**< Hearing Aid Tunning Mode Toggle, 0x0173 */
    KEY_HEARING_AID_BF_SWITCH_TOGGLE                    = KEY_HEARING_AID_BEGIN + 20,   /**< Hearing Aid Beam forming Switch Toggle, 0x0174 */
    KEY_HEARING_AID_AEA_SWITCH_TOGGLE                   = KEY_HEARING_AID_BEGIN + 21,   /**< Hearing Aid AEA Switch Toggle, 0x0175 */
    KEY_HEARING_AID_MASTER_MIC_CHANNEL_TOGGLE           = KEY_HEARING_AID_BEGIN + 22,   /**< Hearing Aid Master Mic Channel Toggle, 0x0176 */
    KEY_HEARING_AID_DEDICATE_DEVICE_PLAY_PAUSE_TOGGLE   = KEY_HEARING_AID_BEGIN + 23,   /**< Hearing Aid dongle dedicate device play pause toggle, 0x0177 */
    KEY_HEARING_AID_END                                 = KEY_HEARING_AID_DEDICATE_DEVICE_PLAY_PAUSE_TOGGLE,

    KEY_DONGLE_CONTROL_RECORD = 0x01A0,                 /* On dongle side control headset side record. */
    KEY_DONGLE_CONTROL_MUTE_MIC = 0x01A1,               /* On dongle side control headset side mute mic. */
    KEY_DONGLE_SWITCH_SPLIT_MERGE_MODE = 0x01A8,        /* On dongle side to switch MIC split mode and merge mode. */
    KEY_DONGLE_SWITCH_SAFETY_MODE = 0x01A9,             /* On dongle side to switch safety mode. */

    KEY_DONGLE_CONTROL_SWITCH_DONGLE_MODE= 0x01AA,      /* Control gaming/xbox/enterprise/... mode. */

    /* Add for line-in feature */
    KEY_LINE_IN_SWITCH  = 0x0200,   /* Switch audio input path is line in or BT. */
    KEY_I2S_IN_SWITCH = 0x0201,

    KEY_AUDIO_MIX_RATIO_GAME_ADD = 0x210,
    KEY_AUDIO_MIX_RATIO_CHAT_ADD = 0x211,
    KEY_AUDIO_SIDE_TONE_VOLUME_UP = 0x212,
    KEY_AUDIO_SIDE_TONE_VOLUME_DOWN = 0x213,
    KEY_AUDIO_CHANNEL_SWAP = 0x214,
    KEY_AUDIO_PEQ_SWITCH = 0x0220, /** < For PEQ switch key event >*/

    KEY_TEST_MODE_ENTER_DUT_MODE = 0x0300,  /* For test mode, press key to enter DUT mode. */
    KEY_TEST_MODE_ENTER_RELAY_MODE = 0x0301,    /* For test mode, press key to enter Relay mode. */

    KEY_DEVICE_SWITCH_ACTIVE = 0x0400, /*For switch active device*/
    KEY_WIRELESS_MIC_VOLUME_DET_SWITCH = 0x0401, /* For switch the volume detection of wireless mic. */
    KEY_FIND_MY_STOP_RING = 0x0402, /* For stop the ring of find my function. */
    
    KEY_AUDIO_CHANNEL_SWITCH = 0x0500, /*For switch device audio channel*/

    KEY_BROADCAST_AGENT = 0x1011,
    KEY_BROADCAST_CLIENT = 0x1012,
    KEY_DOUBLE_AGENT = 0x1013,
    KEY_DOUBLE_PARTNER = 0x1014,
    KEY_SPK_SINGLE = 0x1015,
    KEY_SPK_UNGROUP = 0x1016,

    KEY_CUSTOMER_DEFINE_ACTIONS = 0xF000,   /**< Value larger than it is for customization. */

	//richard for customer UI spec.(customerized key)
	KEY_PSENSOR_TEST = 0xF001,
	KEY_ENABLE_DUT_TEST = 0xF002,
	KEY_VOLUME_SET = 0xF003,
	KEY_SPOTIFY_TAP = 0xF004,

	KEY_MINI_UI_SHORT_CLICK =0xF005,
	KEY_MINI_UI_DOUBLE_CLICK =0xF006,
	KEY_MINI_UI_TRIPLE_CLICK =0xF007,
	KEY_CUSTOMER_LONG_PRESS1 =0xF008,
	KEY_CUSTOMER_LONG_PRESS2 =0xF009,
	KEY_CUSTOMER_SLONG_PRESS =0xF00A,

	KEY_TEST_FACTORY_RESET = 0xF00B,
	KEY_TEST_FACTORY_RESET_AND_POWEROFF = 0xF00C,
	KEY_SIGNAL_DISCOVERABLE = 0xF00D,

	KEY_CUSTOMER_LONG_PRESS1_RELEASE =0xF00E,
    KEY_CUSTOMER_SLONG_PRESS_RELEASE = 0xF00F,
    KEY_TEST_TWS_CLEAN = 0xF010,
    KEY_SWITCH_ANC_AND_PASSTHROUGH1= 0xF011,
    
} apps_config_key_action_t;

#endif /* __APPS_EVENT_LIST_H__ */
