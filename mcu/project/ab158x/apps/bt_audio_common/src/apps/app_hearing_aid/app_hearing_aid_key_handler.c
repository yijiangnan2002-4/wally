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

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#include "app_hearing_aid_key_handler.h"
#include "app_hearing_aid_activity.h"
#include "app_hearing_aid_utils.h"
#include "app_hearing_aid_activity.h"
#include "app_hearing_aid_aws.h"
#include "app_hearing_aid_config.h"
#include "apps_events_event_group.h"
#include "apps_config_event_list.h"
#include "apps_config_vp_index_list.h"
#include "apps_aws_sync_event.h"
#include "apps_debug.h"
#include "bt_device_manager.h"
#include "bt_aws_mce.h"
#include "bt_aws_mce_srv.h"
#include "audio_anc_psap_control.h"
#include "ui_shell_manager.h"
#include "app_hear_through_storage.h"
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "bt_sink_srv_le_cap_stream.h"
#include "apps_events_key_event.h"
#endif /* AIR_LE_AUDIO_BIS_ENABLE */
#include "app_customer_common_activity.h"
#include "App_hear_through_activity.h"

#define APP_HA_KEY_HANDLER_TAG      "[HearingAid][KeyHandler]"


uint8_t anc_key_count;



typedef void (*key_event_handler)();

static const uint8_t app_hearing_aid_beam_forming_index_list[] = {
    VP_INDEX_HEARING_AID_BF_OFF,
    VP_INDEX_HEARING_AID_BF_ON,
};

static const uint8_t app_hearing_aid_aea_index_list[] = {
    VP_INDEX_HEARING_AID_AEA_OFF,
    VP_INDEX_HEARING_AID_AEA_ON,
};

static const uint8_t app_hearing_aid_volume_index_list[] = {
    VP_INDEX_HEARING_AID_VOLUME_DOWN,
    VP_INDEX_HEARING_AID_VOLUME_UP,
};

static void app_hearing_aid_key_handler_proc_level_adjust(bool up, bool circular)
{
    app_hearing_aid_change_value_t change_value = {0};
    bool result = app_hearing_aid_utils_get_level_change_value(up, circular, &change_value);

    if (result == false) {
        return;
    }

    if ((change_value.max == true) || (change_value.min == true)) {
        uint8_t vp_index = 0xFF;
        if (change_value.max == true) {
            if (circular == true) {
                if (app_hear_through_storage_get_ha_level_up_circular_max_vp_switch() == true) {
                    vp_index = VP_INDEX_HEARING_AID_MAX_LEVEL;
                }
            } else {
                vp_index = VP_INDEX_HEARING_AID_MAX_LEVEL;
            }
        } else {
            vp_index = VP_INDEX_HEARING_AID_MIN_LEVEL;
        }

        if (vp_index != 0xFF) {
#ifdef AIR_TWS_ENABLE
            app_hearing_aid_activity_play_vp(vp_index, change_value.sync);
#else
            app_hearing_aid_activity_play_vp(vp_index, false);
#endif /* AIR_TWS_ENABLE */
        }

        if (circular == false) {
            return;
        }
    }

#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_lr_index_change_t change = {0};

        change.l_index = change_value.l_index;
        change.r_index = change_value.r_index;

        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_CHANGE_LEVEL,
                                                    (uint8_t *)&change,
                                                    sizeof(app_hearing_aid_aws_lr_index_change_t),
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_key_handler_adjust_level(change_value.l_index, change_value.r_index, true);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

static void app_hearing_aid_key_handler_proc_volume_adjust(bool up, bool circular)
{
    app_hearing_aid_change_value_t change_value = {0};
    bool result = app_hearing_aid_utils_get_volume_change_value(up, circular, &change_value);

    if (result == false) {
        return;
    }

    if ((change_value.max == true) || (change_value.min == true)) {
        uint8_t vp_index = 0xFF;
        if (change_value.max == true) {
            if (circular == true) {
                if (app_hear_through_storage_get_ha_vol_up_circular_max_vp_switch() == true) {
                    vp_index = VP_INDEX_HEARING_AID_MAX_VOLUME;
                }
            } else {
                vp_index = VP_INDEX_HEARING_AID_MAX_VOLUME;
            }
        } else {
            vp_index = VP_INDEX_HEARING_AID_MIN_VOLUME;
        }

        if (vp_index != 0xFF) {
#ifdef AIR_TWS_ENABLE
            app_hearing_aid_activity_play_vp(vp_index, change_value.sync);
#else
            app_hearing_aid_activity_play_vp(vp_index, false);
#endif /* AIR_TWS_ENABLE */
        }

        if (circular == false) {
            return;
        }
    }

#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_lr_index_with_direction_change_t change = {0};

        change.l_index = change_value.l_index;
        change.r_index = change_value.r_index;
        change.up = up;

        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_CHANGE_VOLUME,
                                                    (uint8_t *)&change,
                                                    sizeof(app_hearing_aid_aws_lr_index_with_direction_change_t),
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_key_handler_adjust_volume(change_value.l_index, change_value.r_index, up, true);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}


static void app_hearing_aid_key_handler_proc_mode_adjust(bool up, bool circular)
{
    uint8_t out_mode = 0;
    bool max = false;
    bool min = false;
    bool get_result = app_hearing_aid_utils_get_mode_change_value(up, circular, &out_mode, &max, &min);
    APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_proc_mode_adjust] anc_eastech_spec=%d,max=%d,min=%d",
                        3,anc_eastech_spec,max,min);

    // if (get_result == false) {
        if ((max == true) || (min == true)) {
            uint8_t vp_index = 0xFF;
            if (max == true) {
                if (circular == true) {
                    if (app_hear_through_storage_get_ha_mode_up_circular_max_vp_switch() == true) {
                        vp_index = VP_INDEX_HEARING_AID_MAX_MODE;
                    }
                } else {
                    vp_index = VP_INDEX_HEARING_AID_MAX_MODE;
                }
            } else {
                vp_index = VP_INDEX_HEARING_AID_MIN_MODE;
            }
    APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_proc_mode_adjust] anc_eastech_spec=%d,vp_index : %d",
                        2,
                        anc_eastech_spec,
                        vp_index);
            if (vp_index != 0xFF) {
#ifdef AIR_TWS_ENABLE
   #if 0 // harry for anc+ha common key
                if(anc_eastech_spec==0)
                {
                    APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_proc_mode_adjust] anc_eastech_spec==0",0);
                    app_hearing_aid_activity_play_vp(vp_index, true);
                }
                else
                {
                    APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_proc_mode_adjust] anc_eastech_spec!=0",0);
                    app_hearing_aid_activity_play_vp(vp_index, false);
                }
   #else
                 app_hearing_aid_activity_play_vp(vp_index, true);
   #endif
#else
                app_hearing_aid_activity_play_vp(vp_index, false);
#endif /* AIR_TWS_ENABLE */
            }
        }

        if (get_result == false) {
            return;
        }
    // }

#ifdef AIR_TWS_ENABLE
#if 0// harry for anc+ha common key
    if ((app_hearing_aid_aws_is_connected() == true)&&(anc_eastech_spec==0))
      #else
    if ((app_hearing_aid_aws_is_connected() == true))
      #endif
      {
        app_hearing_aid_aws_index_change_t change = {0};
        change.index = out_mode;
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_CHANGE_MODE,
                                                (uint8_t *)&change,
                                                sizeof(app_hearing_aid_aws_index_change_t),
                                                true,
                                                APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_proc_mode_adjust] harry single call",0);
        app_hearing_aid_key_handler_adjust_mode(out_mode, true);
#ifdef AIR_TWS_ENABLE
    }
    //anc_eastech_spec=0;// harry for anc+ha common key
#endif /* AIR_TWS_ENABLE */
}


void app_hearing_aid_key_handler_proc_ha_toggle()
{
    bool enable = app_hearing_aid_utils_is_ha_user_switch_on();
    enable = !enable;

    app_hearing_aid_activity_set_user_switch(true, enable);

    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_BUTTON, enable, false);
}

void app_hearing_aid_key_handler_proc_level_up_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_level_up()
{
    app_hearing_aid_key_handler_proc_level_adjust(true, false);
}

void app_hearing_aid_key_handler_proc_level_down_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_level_down()
{
    app_hearing_aid_key_handler_proc_level_adjust(false, false);
}

void app_hearing_aid_key_handler_proc_level_up_circular_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_level_up_circular()
{
    app_hearing_aid_key_handler_proc_level_adjust(true, true);
}

void app_hearing_aid_key_handler_proc_mode_up_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_mode_up()
{
    app_hearing_aid_key_handler_proc_mode_adjust(true, false);
}

void app_hearing_aid_key_handler_proc_mode_down_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

#if 1	// richard for UI spec.
uint8_t get_current_ha_mode(void)
{
	uint8_t mode_index = 0;

	audio_psap_status_t mode_index_status = audio_anc_psap_control_get_mode_index(&mode_index);
	return mode_index;
}
#endif

void app_hearing_aid_key_handler_proc_mode_down()
{
#if 0// harry for anc+ha common key
    uint8_t l_level_index = 0;
    uint8_t r_level_index = 0;
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;
    uint8_t mode_index = 0;

    audio_psap_status_t mode_index_status = audio_anc_psap_control_get_mode_index(&mode_index);
    audio_psap_status_t mode_max_count_status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);
    audio_psap_status_t get_level_index_status = audio_anc_psap_control_get_level_index(&l_level_index, &r_level_index);

    APPS_LOG_MSGID_I("app_hearing_aid_key_handler_proc_mode_down  current mode index:%d, mode max:%d,anc_key_count=%d,vol_max_count=%d",
                     4,
                     mode_index,
                     mode_max_count,
                     anc_key_count,
                     vol_max_count
                     );

    //app_hearing_aid_key_handler_proc_mode_adjust(false, false);
    APPS_LOG_MSGID_I("app_hearing_aid_key_handler_proc_mode_down  l_index : %d, r_index : %d, level_max : %d,anc_key_count=%d",
                     4,
                     l_level_index,
                     r_level_index,
                     level_max_count,
                     anc_key_count
                     );    
    if (anc_key_count)
    {
    #if 1
      app_hear_through_activity_switch_ancon();
    #else
      uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
      if (p_key_action) {
          *p_key_action = 0x0092;//KEY_DISCOVERABLE = 0x0002KEY_DISCOVERABLE;	   
          ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, 0xFFFF, p_key_action,	sizeof(uint16_t), NULL, 0);
        }
    #endif
        anc_key_count=0;  
  	    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ANC_KEY_SYNC,(void*)&anc_key_count ,1);
    }
    else
    {
        //app_hear_through_switch_on_off(true, true);
        app_hear_through_activity_switch_to_hear_through();       
        app_hearing_aid_key_handler_proc_mode_adjust(true, true);
       // if((mode_index+2)>=mode_max_count)
        if((mode_index+2)==mode_max_count)
        {
            anc_key_count=1;  
		    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ANC_KEY_SYNC,(void*)&anc_key_count ,1);
        }
    }
    audio_anc_psap_control_get_mode_index(&mode_index);
    APPS_LOG_MSGID_I("app_hearing_aid_key_handler_proc_mode_down  mode_index:%d", 1, mode_index );
    #else
    app_hearing_aid_key_handler_proc_mode_adjust(false, false);
    #endif

}

void app_hearing_aid_key_handler_proc_mode_up_circular_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_mode_up_circular()
{
    app_hearing_aid_key_handler_proc_mode_adjust(true, true);
}

void app_hearing_aid_key_handler_proc_volume_up_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_volume_up()
{
    app_hearing_aid_key_handler_proc_volume_adjust(true, false);
}

void app_hearing_aid_key_handler_proc_volume_down_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_volume_down()
{
    app_hearing_aid_key_handler_proc_volume_adjust(false, false);
}

void app_hearing_aid_key_handler_proc_volume_up_circular_notify()
{
    app_hearing_aid_activity_play_vp(VP_INDEX_PRESS, true);
}

void app_hearing_aid_key_handler_proc_volume_up_circular()
{
    app_hearing_aid_key_handler_proc_volume_adjust(true, true);
}

void app_hearing_aid_key_handler_proc_tunning_mode_toggle()
{
#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_SWITCH_TUNING_MODE,
                                                    NULL,
                                                    0,
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_utils_hearing_tuning_mode_toggle(false);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

void app_hearing_aid_key_handler_proc_bf_mode_switch_toggle()
{
    bool enable = app_hearing_aid_utils_is_beam_forming_enable();

    enable = !enable;

#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_index_change_t change = {0};
        change.index = enable;
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_SWITCH_BF,
                                                    (uint8_t *)&change,
                                                    sizeof(app_hearing_aid_aws_index_change_t),
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_key_handler_bf_mode_switch(enable, true);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

void app_hearing_aid_key_handler_proc_aea_switch_toggle()
{
    bool enable = app_hearing_aid_utils_is_aea_configure_enable();

    enable = !enable;

#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_index_change_t change = {0};
        change.index = enable;
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_SWITCH_AEA_CONFIGURE,
                                                    (uint8_t *)&change,
                                                    sizeof(app_hearing_aid_aws_index_change_t),
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_key_handler_aea_switch(enable, true);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

void app_hearing_aid_key_handler_proc_dedicate_device_play_pause_toggle()
{
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    bool is_bis_streaming = bt_sink_srv_cap_stream_is_broadcast_streaming();
    uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    uint16_t action = INVALID_KEY_EVENT_ID;
    if (key_action == NULL) {
        APPS_LOG_MSGID_E(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_proc_dedicate_device_play_pause_toggle] Failed to allocate buffer to key action", 0);
        return;
    }
    if (is_bis_streaming == true) {
        action = KEY_LE_AUDIO_BIS_STOP;
    } else {
        action = KEY_LE_AUDIO_BIS_SCAN;
    }
    APPS_LOG_MSGID_E(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_proc_dedicate_device_play_pause_toggle] is_bis_streaming : %d, action : 0x%2x",
                     2,
                     is_bis_streaming,
                     action);

    memcpy(key_action, &action, sizeof(uint16_t));
    ui_shell_send_event(false,
                        EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_KEY,
                        INVALID_KEY_EVENT_ID,
                        key_action,
                        sizeof(uint16_t),
                        NULL,
                        0);
#else
    APPS_LOG_MSGID_E(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_proc_dedicate_device_play_pause_toggle] Not define AIR_LE_AUDIO_BIS_ENABLE", 0);
#endif /* AIR_LE_AUDIO_BIS_ENABLE */
}

void app_hearing_aid_key_handler_proc_master_mic_channel_toggle()
{
    uint8_t channel = app_hearing_aid_utils_get_master_mic_channel();

    if (channel == APP_HEARING_AID_MASTER_MIC_CHANNEL_1) {
        channel = APP_HEARING_AID_MASTER_MIC_CHANNEL_2;
    } else if (channel == APP_HEARING_AID_MASTER_MIC_CHANNEL_2) {
        channel = APP_HEARING_AID_MASTER_MIC_CHANNEL_1;
    } else {
        APPS_LOG_MSGID_E(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_proc_master_mic_channel_toggle] Unknown channel to handle : %d", 1, channel);
        return;
    }

#ifdef AIR_TWS_ENABLE
    if (app_hearing_aid_aws_is_connected() == true) {
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_SWITCH_MASTER_MIC_CHANNEL,
                                                    &channel,
                                                    sizeof(uint8_t),
                                                    true,
                                                    APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_utils_master_mic_channel_switch_toggle(channel);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

ATTR_LOG_STRING_LIB app_hearing_aid_key_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN] = {
    "HA_TOGGLE",
    "HA_LEVEL_UP_NOTIFY",
    "HA_LEVEL_UP",
    "HA_LEVEL_DOWN_NOTIFY",
    "HA_LEVEL_DOWN",
    "HA_LEVEL_UP_CIRCULAR_NOTIFY",
    "HA_LEVEL_UP_CIRCULAR",
    "HA_MODE_UP_NOTIFY",
    "HA_MODE_UP",
    "HA_MODE_DOWN_NOTIFY",
    "HA_MODE_DOWN",
    "HA_MODE_UP_CIRCULAR_NOTIFY",
    "HA_MODE_UP_CIRCULAR",
    "HA_VOLUME_UP_NOTIFY",
    "HA_VOLUME_UP",
    "HA_VOLUME_DOWN_NOTIFY",
    "HA_VOLUME_DOWN",
    "HA_VOLUME_UP_CIRCULAR_NOTIFY",
    "HA_VOLUME_UP_CIRCULAR",
    "HA_TUNING_MODE_TOGGLE",
    "HA_BEAM_FORMING_SWITCH_TOGGLE",
    "HA_AEA_SWITCH_TOGGLE",
    "HA_MASTER_MIC_CHANNEL_TOGGLE",
    "HA_DEDICATE_PLAY_PAUSE_TOGGLE",
};

const key_event_handler key_handler_list[] = {
    app_hearing_aid_key_handler_proc_ha_toggle,
    app_hearing_aid_key_handler_proc_level_up_notify,
    app_hearing_aid_key_handler_proc_level_up,
    app_hearing_aid_key_handler_proc_level_down_notify,
    app_hearing_aid_key_handler_proc_level_down,
    app_hearing_aid_key_handler_proc_level_up_circular_notify,
    app_hearing_aid_key_handler_proc_level_up_circular,
    app_hearing_aid_key_handler_proc_mode_up_notify,
    app_hearing_aid_key_handler_proc_mode_up,
    app_hearing_aid_key_handler_proc_mode_down_notify,
    app_hearing_aid_key_handler_proc_mode_down,
    app_hearing_aid_key_handler_proc_mode_up_circular_notify,
    app_hearing_aid_key_handler_proc_mode_up_circular,
    app_hearing_aid_key_handler_proc_volume_up_notify,
    app_hearing_aid_key_handler_proc_volume_up,
    app_hearing_aid_key_handler_proc_volume_down_notify,
    app_hearing_aid_key_handler_proc_volume_down,
    app_hearing_aid_key_handler_proc_volume_up_circular_notify,
    app_hearing_aid_key_handler_proc_volume_up_circular,
    app_hearing_aid_key_handler_proc_tunning_mode_toggle,
    app_hearing_aid_key_handler_proc_bf_mode_switch_toggle,
    app_hearing_aid_key_handler_proc_aea_switch_toggle,
    app_hearing_aid_key_handler_proc_master_mic_channel_toggle,
    app_hearing_aid_key_handler_proc_dedicate_device_play_pause_toggle,
};

bool app_hearing_aid_key_handler_is_ready_to_process(apps_config_key_action_t key_event)
{
    // bool ha_on = app_hearing_aid_utils_is_ha_user_switch_on();
    bool hearing_tunning_mode = app_hearing_aid_utils_is_hearing_tuning_mode();
    bool mp_test_mode = app_hearing_aid_utils_is_mp_test_mode();

#if 0
    if (ha_on == false) {
        if (key_event == KEY_HEARING_AID_HA_TOGGLE) {
            return true;
        } else {
            return false;
        }
    }
#endif
    if (hearing_tunning_mode == true) {
        if (/*(key_event == KEY_HEARING_AID_HA_TOGGLE)
            || */(key_event == KEY_HEARING_AID_LEVEL_UP)
            || (key_event == KEY_HEARING_AID_LEVEL_DOWN)
            || (key_event == KEY_HEARING_AID_LEVEL_UP_CIRCULAR)
            || (key_event == KEY_HEARING_AID_VOLUME_UP)
            || (key_event == KEY_HEARING_AID_VOLUME_DOWN)
            || (key_event == KEY_HEARING_AID_VOLUME_UP_CIRCULAR)
            || (key_event == KEY_HEARING_AID_MASTER_MIC_CHANNEL_TOGGLE)
            || (key_event == KEY_HEARING_AID_TUNING_MODE_TOGGLE)) {
            return true;
        } else {
            return false;
        }
    }

    if (mp_test_mode == true) {
        if (/*(key_event == KEY_HEARING_AID_HA_TOGGLE)
            || */(key_event == KEY_HEARING_AID_LEVEL_UP)
            || (key_event == KEY_HEARING_AID_LEVEL_DOWN)
            || (key_event == KEY_HEARING_AID_LEVEL_UP_CIRCULAR)
            || (key_event == KEY_HEARING_AID_VOLUME_UP)
            || (key_event == KEY_HEARING_AID_VOLUME_DOWN)
            || (key_event == KEY_HEARING_AID_VOLUME_UP_CIRCULAR)
            || (key_event == KEY_HEARING_AID_MASTER_MIC_CHANNEL_TOGGLE)) {
            return true;
        } else {
            return false;
        }
    }

    return true;
}
extern uint8_t prompt_no_play_flag;	// richard for UI

bool app_hearing_aid_key_handler_processing(apps_config_key_action_t key_event)
{
    // TODO need handle key event from agent or from partner.
    if ((key_event < KEY_HEARING_AID_BEGIN) || (key_event > KEY_HEARING_AID_END)) {
        return false;
    }

    if (app_hearing_aid_activity_is_out_case() == false) {
        APPS_LOG_MSGID_W(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_processing] Ignore the key event cause in charger case", 0);
        return false;
    }

    bool ready_to_handle = app_hearing_aid_key_handler_is_ready_to_process(key_event);
    APPS_LOG_MSGID_I(APP_HA_KEY_HANDLER_TAG"[app_hearing_aid_key_handler_processing] Handle key event : 0x%04x (%s), ready to handle : %d,prompt_no_play_flag=%d",
                     4,
                     key_event,
                     app_hearing_aid_key_string[key_event - KEY_HEARING_AID_BEGIN],
                     ready_to_handle,prompt_no_play_flag);
      prompt_no_play_flag=0; // 双击切HA，需要打开播放HA VP
    if (ready_to_handle == false) {
        return true;
    }

    uint8_t index = key_event - KEY_HEARING_AID_BEGIN;
    key_handler_list[index]();
    return true;
}

void app_hearing_aid_key_handler_adjust_level(uint8_t l_index, uint8_t r_index, bool need_vp)
{
    app_hearing_aid_utils_adjust_level(l_index, r_index);
    if (need_vp == true) {
        app_hearing_aid_activity_play_vp(VP_INDEX_HEARING_AID_LEVEL_CHANGED, true);
    }
}

void app_hearing_aid_key_handler_adjust_volume(uint8_t l_index, uint8_t r_index, bool up, bool need_vp)
{
    app_hearing_aid_utils_adjust_volume(l_index, r_index);
    if (need_vp == true) {
        app_hearing_aid_activity_play_vp(app_hearing_aid_volume_index_list[up], true);
    }
}

void app_hearing_aid_key_handler_adjust_mode(uint8_t target_mode, bool need_vp)
{
         APPS_LOG_MSGID_I("[app_hearing_aid_key_handler_adjust_mode] harry target_mode=%d,need_vp=%d",2,target_mode,need_vp);
   app_hearing_aid_utils_adjust_mode(target_mode);
    if (need_vp == true) {
        app_hearing_aid_activity_play_mode_index_vp(target_mode, true);
    }
}

void app_hearing_aid_key_handler_bf_mode_switch(bool enable, bool need_vp)
{
    app_hearing_aid_utils_beam_forming_switch_toggle(enable);
    if (need_vp == true) {
        app_hearing_aid_activity_play_vp(app_hearing_aid_beam_forming_index_list[enable], true);
    }
}

void app_hearing_aid_key_handler_aea_switch(bool enable, bool need_vp)
{
    app_hearing_aid_utils_aea_switch_toggle(enable);
    if (need_vp == true) {
        app_hearing_aid_activity_play_vp(app_hearing_aid_aea_index_list[enable], true);
    }
}


#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */


