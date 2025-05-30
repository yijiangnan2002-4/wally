
/**
  * @file    app_customer_common_activity.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/5/27
  *
  * @brief   data process for customer common handle
**/

#include "app_customer_common_activity.h"
#include "app_customer_common.h"
#include "apps_events_event_group.h"
#include "apps_debug.h"
#include "bt_aws_mce_report.h"
#include "apps_aws_sync_event.h"
#include "app_psensor_px31bf_activity.h"
#include "app_customer_nvkey_operation.h"
#include "ui_shell_manager.h"
#include "voice_prompt_api.h"
#include "airo_key_event.h"
#include "apps_config_event_list.h"
#include "bt_sink_srv_a2dp.h"
#include "bsp_px31bf.h"
#include "apps_config_vp_index_list.h"
#include "apps_config_state_list.h"
#include "apps_events_key_event.h"
#include "app_smcharger_utils.h"
//#include "spp_air_adidas_running.h"
#ifdef BLE_ZOUND_ENABLE
#include "gatt_zound_service_notify.h"
#include "gatt_zound_service_tool.h"
#include "gatt_zound_service.h"
#include "gatt_bas.h"
#include "gatt_bas_left_notify.h"
#include "gatt_bas_right_notify.h"
#include "gatt_bas_main_notify.h"
#include <bt_sink_srv_hf.h>
#endif
#include "battery_management.h"
#include "apps_config_key_remapper.h"
#include "app_bt_conn_componet_in_homescreen.h"
//#include "race_cmd_factory_test.h"
#include "bt_app_common_at_cmd.h"
#include "apps_customer_config.h"
#include "app_hearing_aid_key_handler.h"
#include "app_bt_state_service.h"
#include "app_home_screen_idle_activity.h"
#include "app_music_utils.h"
#include "app_hearing_aid_activity.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "app_smcharger_idle_activity.h"
#ifdef BATTERY_HEATHY_ENABLE
//calculate when in case after 5s, so need power on or out of case more than 1min, and keep charge in case more than 5s
#define BATTERY_HEATHY_PERIOD (30*1000)
static TimerHandle_t xCusAPP_battery_heathy_timer;
volatile uint8_t battery_percent_backup = 0;
static uint8_t battery_percent_out = 0;
static uint8_t battery_percent_in = 0;
static uint8_t battery_heathy_state = 0xFF;
#endif

static uint16_t peer_battery_heathy_times = 0;


static apps_customer_common_local_context_t s_app_customer_common_context;
bool normal_ui_short_click();
bool normal_and_mini_ui_double_click(uint8_t mini_ui_flag);
bool normal_and_mini_ui_triple_click(uint8_t mini_ui_flag);
void customer_key_configure_long_press1(void);
bool customer_key_configure_slong_press(void);
bool customer_key_configure_short_click(void);
bool customer_key_configure_double_click(void);
bool customer_key_configure_triple_click(void);
void app_eastech_pair_vp_callback(void );
void customer_key_triger_ocean_vp(void);
void bt_sink_music_change_respone_vp(uint8_t oldvol,uint8_t newvol);
    uint8_t ocean_cnt=0;
   uint8_t	ocean_vp_vol=10;  // DEF= VP DEF
#ifdef BATTERY_HEATHY_ENABLE
static void battery_heathy_timer_callback(TimerHandle_t pxTimer)
{
	if(!battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST))
	{
		battery_percent_backup = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
		APPS_LOG_MSGID_I("app_customer_common_activity battery heathy_timer_callback uncharging:battery_percent %d%", 1, battery_percent_backup);
	}
	else
		APPS_LOG_MSGID_I("app_customer_common_activity battery heathy_timer_callback incharging:battery_percent %d%", 1, battery_percent_backup);

}
#endif

uint16_t app_get_peer_battery_heathy_times(void)
{
	return peer_battery_heathy_times;
}


static bool _proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = true; // UI shell internal event must process by this activity, so default is true
    switch (event_id)
    {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity create current activity", 0);
            self->local_context = &s_app_customer_common_context;

#ifdef BATTERY_HEATHY_ENABLE
			battery_percent_backup = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);

			xCusAPP_battery_heathy_timer = xTimerCreate("battery heathy Timer", BATTERY_HEATHY_PERIOD, pdTRUE, NULL, battery_heathy_timer_callback);
	        if (xTimerStart(xCusAPP_battery_heathy_timer, 0) != pdPASS) {
	            APPS_LOG_MSGID_E("battery heathy Timer xTimerStart fail\n", 0);
	        }
#endif

            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity destroy", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_RESUME:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity resume", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity pause", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity refresh", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_RESULT:
        {
            APPS_LOG_MSGID_I("app_customer_common_activity result", 0);
            break;
        }

        default:
            break;
    }
    return ret;
}

static bool _proc_key_event_group(ui_shell_activity_t *self,
                                  uint32_t event_id,
                                  void *extra_data,
                                  size_t data_len)
{
    bool ret = false;
//    apps_customer_common_local_context_t *local_context = (apps_customer_common_local_context_t *)(self->local_context);

    uint8_t key_id;
    airo_key_event_t key_event;
    app_event_key_event_decode(&key_id, &key_event, event_id);

    apps_config_key_action_t action;
    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }
	uint8_t mini_ui = app_nvkey_ble_mini_ui_read();
	switch (action)
	{
		case KEY_PSENSOR_TEST:
			{
				uint8_t data[7] = {0};

				bsp_px31bf_PsData_read_reg(data);
				//race_debug_print((uint8_t*)data, 7,"psensor data read:");
			}
			ret = true;
			break;
		case KEY_ENABLE_DUT_TEST:
			app_customer_test_enable_dut();
			ret = true;
			break;
		case KEY_VOLUME_SET:
			app_set_a2dp_volume(BT_SINK_SRV_A2DP_MAX_VOL_LEV);
			ret = true;
			break;
		case KEY_TEST_TWS_CLEAN:
			app_customer_common_tws_clean();
			ret = true;
			break;			
		case KEY_MINI_UI_SHORT_CLICK:
			if(mini_ui){
				ret = true;
			}else{
#ifdef BLE_ZOUND_ENABLE			
				ret = customer_key_configure_short_click();
#endif
				if(!ret){
					ret = normal_ui_short_click();
				}
			}
			break;

		case KEY_MINI_UI_DOUBLE_CLICK:
			if(mini_ui){
				ret = normal_and_mini_ui_double_click(mini_ui);
			}
#ifdef BLE_ZOUND_ENABLE
			if(!ret){
				ret = customer_key_configure_double_click();
			}
#endif
			break;

		case KEY_MINI_UI_TRIPLE_CLICK:
			if(mini_ui){
				ret = normal_and_mini_ui_triple_click(mini_ui);
			}
#ifdef BLE_ZOUND_ENABLE
			if(!ret){
				ret = customer_key_configure_triple_click();
			}
#endif
			break;
		case KEY_CUSTOMER_LONG_PRESS1:
			customer_key_configure_long_press1();
			ret = true;
			break;
#ifdef BLE_ZOUND_ENABLE
		case KEY_CUSTOMER_LONG_PRESS1_RELEASE:
			zound_volume_task_send_msg(ZOUND_VOLUME_STOP);
			ret = true;
			break;
		case KEY_CUSTOMER_SLONG_PRESS_RELEASE:
			zound_volume_task_send_msg(ZOUND_VOLUME_STOP);
			ret = true;
			break;
		case KEY_CUSTOMER_SLONG_PRESS:
			ret = customer_key_configure_slong_press();
			break;

#endif
		case KEY_TRIGER_OCEAN_VP:		
			if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
			&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
			{
				APPS_LOG_MSGID_I("customer_key_triger_ocean_vp role=partner sent EVENT_ID_EASTECH_CALLBACK_OCEAN_VP to agent,ocean_cnt=%d ", 1,ocean_cnt);
				apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_CALLBACK_OCEAN_VP,NULL,0);
			}
			else
			{
				if(ocean_cnt==0)		
				{
					APPS_LOG_MSGID_I("customer_key_triger_ocean_vp role=agent play vp ", 0);
					ocean_cnt=1;	
					customer_key_triger_ocean_vp();	
				}
				else	
				{
					APPS_LOG_MSGID_I("customer_key_triger_ocean_vp role=agent stop vp ", 0);
					ocean_cnt=0;	
		       			ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,EVENT_ID_EASTECH_CALLBACK_OCEAN_VP);
			 		voice_prompt_stop(VP_INDEX_Ocean,VOICE_PROMPT_ID_INVALID,false);	
				}
				apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_CALLBACK_OCEAN_VP,&ocean_cnt,1);
			}
			ret = true;
			break;
		case KEY_VP_UP:
			if(ocean_cnt==1)
			{
				if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
				&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
				{
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_UP role=partner sent EVENT_ID_EASTECH_OCEAN_ADJUST_VP to agent,local ocean_vp_vol=%d ", 1,ocean_vp_vol);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_VP,NULL,0);
				}
				else
				{
					if(ocean_vp_vol<15)
					{
						ocean_vp_vol++;	
					}
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_UP role=agent   action vol+,local ocean_vp_vol=%d ", 1,ocean_vp_vol);
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_UP role=agent   sync sent EVENT_ID_EASTECH_OCEAN_ADJUST_VP,ocean_vp_vol to partner",0);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_VP,&ocean_vp_vol,1);
				}
			}
			ret = true;
			break;

		case KEY_VP_DM:
			if(ocean_cnt==1)
			{
				if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
				&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
				{
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_DM role=partner sent EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP to agent,partner-local ocean_vp_vol=%d ", 1,ocean_vp_vol);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP,NULL,0);
				}
				else
				{
					if(ocean_vp_vol>1)
					{
					ocean_vp_vol--;	
					}
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_DM role=agent,local ocean_vp_vol=%d ", 1,ocean_vp_vol);
					APPS_LOG_MSGID_I("_proc_key_event_group KEY_VP_DM role=agent   sync sent EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP to partner,ocean_vp_vol to partner ", 0);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP,&ocean_vp_vol,1);
				}
			}
			ret = true;
			break;

		default:
			break;

	}

	return ret;
}

#if 1	// richard for ab1571d command processing
uint8_t Is_earbuds_agent_proc(void)
{
	if((bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) && (bt_sink_srv_cm_get_aws_connected_device() != NULL))
		return 1;
	else return 0;
}

void key_oceanvp_trige_proc(void)		
{
#if 1
		APPS_LOG_MSGID_I("key_oceanvp_trige_proc", 0);

#else
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); 

	*p_key_action = KEY_TRIGER_OCEAN_VP;
	

	if (p_key_action)
	{
        	ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
  	}
  	else
  	{
      		vPortFree(p_key_action);
  	}	
#endif
}

void key_oceanvp_up_proc(void)		
{
   if(ocean_cnt==1)
   {
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); 
	//*p_key_action = KEY_ACTION_INVALID;
	//uint8_t volkey_val;

	*p_key_action = KEY_VP_UP;
	//volkey_val=2;
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
	&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
	}
	else
	{
	//	if(ocean_vp_vol<15)
		{
	//		ocean_vp_vol++;	
		}

	}

	if (p_key_action)
	{
        	ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
  	}
  	else
  	{
      		vPortFree(p_key_action);
  	}		
}

}
void key_ocean_vpdown_proc(void )		// 0: sp; 1: LP2
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
//	uint8_t volkey_val;
	 if(ocean_cnt==1)
   	{

	*p_key_action = KEY_VP_DM;
	//volkey_val=1;
	if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
	&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
	}
	else
	{

	}	

	if (p_key_action)
	{
        	ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
  	}
  	else
  	{
      		vPortFree(p_key_action);
  	}	
    }
}

extern uint8_t anc_ha_flag;
void key_volumeup_proc(uint8_t volume_up_mode)		// 0: sp; 1: LP2
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	uint8_t volkey_val;   // 1:音量-VP,	2:音量+VP,	3,4:HA默认音量VP.	5,MAX VOL VP.	6,mix vol vp
    uint8_t l_level_index = 0;
	uint8_t r_level_index = 0;
	uint16_t	bt_sink_state;
    uint8_t volume = 0;

     bt_sink_state = bt_sink_srv_get_state();
    if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING) {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_HFP);
    } else {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_A2DP);
    }

    APPS_LOG_MSGID_I("key_volumeup_proc: volume=%d, bt_sink_state=0x%x, a2dp_maxvol=0x%x, hfp_maxvol=0x%x", 4 , volume, bt_sink_state,bt_sink_srv_ami_get_a2dp_max_volume_level(),AUD_VOL_OUT_LEVEL15);

     audio_anc_psap_control_get_volume_index(&l_level_index, &r_level_index);
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	APPS_LOG_MSGID_I("key_volumeup_proc app_mmi_state=%d,anc_ha_flag=%d,music_streaming_state_common=%d,l_level=%d,r_level=%d", 5,app_mmi_state,anc_ha_flag,music_streaming_state_common,l_level_index,r_level_index);
	if(volume_up_mode==0)
	{
		if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY||
		app_mmi_state == APP_HFP_INCOMING || app_mmi_state == APP_HFP_MULTIPARTY_CALL||
		app_mmi_state == APP_HFP_TWC_INCOMING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_LE_AUDIO_BIS_PLAYING||
		app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_HFP_CALL_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
		{
			*p_key_action = KEY_VOICE_UP;
		APPS_LOG_MSGID_I("key_volumeup_proc sent KEY_VOICE_UP 11", 0);
		}
		else if(music_streaming_state_common==1)
		{
			*p_key_action = KEY_VOICE_UP;
		APPS_LOG_MSGID_I("key_volumeup_proc sent KEY_VOICE_UP 22", 0);
		}
		else if (app_mmi_state == APP_DISCONNECTED ||app_mmi_state == APP_CONNECTABLE ||
			app_mmi_state == APP_CONNECTED ||app_mmi_state == APP_STATE_FIND_ME ||
			app_mmi_state == APP_HFP_CALL_ACTIVE_WITHOUT_SCO ||app_mmi_state == APP_STATE_VA)
		{
			*p_key_action = KEY_HEARING_AID_VOLUME_UP;
		APPS_LOG_MSGID_I("key_volumeup_proc sent KEY_HEARING_AID_VOLUME_UP", 0);
		}

		
		if((*p_key_action == KEY_HEARING_AID_VOLUME_UP)&&l_level_index==3)
		{
			volkey_val=4;  // 默认音量
		}
		else if((*p_key_action == KEY_HEARING_AID_VOLUME_UP)&&(l_level_index>=7))
		{
			volkey_val=5;
		}
		else if((*p_key_action == KEY_VOICE_UP)&&(bt_sink_state >= BT_SINK_SRV_STATE_INCOMING)&&(volume >= AUD_VOL_OUT_LEVEL15))
		{
			volkey_val=5;
		}
		else if((*p_key_action == KEY_VOICE_UP)&&(bt_sink_state < BT_SINK_SRV_STATE_INCOMING)&&(volume >=bt_sink_srv_ami_get_a2dp_max_volume_level()))
		{
			volkey_val=5;
		}
		else
		{
			volkey_val=2;
		}
			APPS_LOG_MSGID_I("key_volumeup_proc volkey_val=%d", 1,volkey_val);
		if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
		{
			APPS_LOG_MSGID_I("key_volumeup_proc role=partner sent data to agent", 0);
			apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_VOLUME_VP,(void*)&volkey_val,1);
		}
		else
		{
			if(volkey_val==4)
			{
			APPS_LOG_MSGID_I("key_volumeup_proc  voice_prompt_play_sync_vp_mute22 ", 0);
        			voice_prompt_play_sync_vp_mute();  // mute code actually is default vol vp
			}
			else if(volkey_val==5)
			{
			APPS_LOG_MSGID_I("key_volumeup_proc  voice_prompt_play_sync_vp_maxvol ", 0);
        			voice_prompt_play_sync_vp_maxvol();  
			}
			else
			{
        		voice_prompt_play_sync_vp_volume_up();  
			}
		}
	}
	else
	{
		if(anc_ha_flag)
		{
			*p_key_action = KEY_HEARING_AID_VOLUME_UP;	
		}
	}

	if (p_key_action)
	{
        	ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
  	}
  	else
  	{
      		vPortFree(p_key_action);
  	}		
}

void key_volumedown_proc(uint8_t volume_down_mode)		// 0: SP; 1: LP2
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	uint8_t volkey_val;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
    	uint8_t l_level_index = 0;
    	uint8_t r_level_index = 0;
	uint16_t	bt_sink_state;
    	uint8_t volume = 0;

     bt_sink_state = bt_sink_srv_get_state();
    if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING) {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_HFP);
    } else {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_A2DP);
    }

    APPS_LOG_MSGID_I("key_volumedown_proc: volume=%d, bt_sink_state=0x%x", 2 , volume, bt_sink_state);

     audio_anc_psap_control_get_volume_index(&l_level_index, &r_level_index);
	
	APPS_LOG_MSGID_I("key_volumedown_proc app_mmi_state=%d,anc_ha_flag=%d,music_streaming_state_common=%d,l_level=%d,r_level=%d", 5,app_mmi_state,anc_ha_flag,music_streaming_state_common,l_level_index,r_level_index);

	if(volume_down_mode==0)
	{
		if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY||
		 app_mmi_state == APP_HFP_INCOMING || app_mmi_state == APP_HFP_MULTIPARTY_CALL||
		app_mmi_state == APP_HFP_TWC_INCOMING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_LE_AUDIO_BIS_PLAYING||
		app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_HFP_CALL_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING
		)
		{
			*p_key_action = KEY_VOICE_DN;
		APPS_LOG_MSGID_I("key_volumedown_proc sent KEY_VOICE_DN 11", 0);
		}
		else if(music_streaming_state_common==1)
		{
			*p_key_action = KEY_VOICE_DN;
		APPS_LOG_MSGID_I("key_volumedown_proc sent KEY_VOICE_DN 22", 0);
		}
		else if (app_mmi_state == APP_DISCONNECTED ||app_mmi_state == APP_CONNECTABLE ||
			app_mmi_state == APP_CONNECTED ||app_mmi_state == APP_STATE_FIND_ME ||
			app_mmi_state == APP_HFP_CALL_ACTIVE_WITHOUT_SCO ||app_mmi_state == APP_STATE_VA)
		{
			*p_key_action = KEY_HEARING_AID_VOLUME_DOWN;
		APPS_LOG_MSGID_I("key_volumedown_proc sent KEY_HEARING_AID_VOLUME_DOWN key=0x%x", 1,*p_key_action);
		}
		if((*p_key_action == KEY_HEARING_AID_VOLUME_DOWN)&&l_level_index==5)
		{
			volkey_val=3;
		}
		else if((*p_key_action == KEY_HEARING_AID_VOLUME_DOWN)&&(l_level_index==0))
		{
			volkey_val=6;
		}
		else if((*p_key_action == KEY_VOICE_DN)&&(bt_sink_state >= BT_SINK_SRV_STATE_INCOMING)&&(volume == 0/*AUD_VOL_OUT_LEVEL15*/))
		{
			volkey_val=6;
		}
		else if((*p_key_action == KEY_VOICE_DN)&&(bt_sink_state < BT_SINK_SRV_STATE_INCOMING)&&(0==volume))
		{
			volkey_val=6;
		}
		else
		{
			volkey_val=1;
		}
			APPS_LOG_MSGID_I("key_volumedown_proc volkey_val=%d", 1,volkey_val);
		if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
		{
			APPS_LOG_MSGID_I("key_volumedown_proc role=partner sent data to agent ", 0);
			apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_VOLUME_VP,(void*)&volkey_val,1);
		}
		else
		{
			if(volkey_val==3)
			{
			APPS_LOG_MSGID_I("key_volumedown_proc  voice_prompt_play_sync_vp_mute11 ", 0);
        			voice_prompt_play_sync_vp_mute();  // mute code actually is default vol vp
			}
			else if(volkey_val==6)
			{
        			voice_prompt_play_sync_vp_minvol();  
			}
			else
			{
        			voice_prompt_play_sync_vp_volume_down();  
			}
		}
	}
	else
	{
		if(anc_ha_flag)
		{
			*p_key_action = KEY_HEARING_AID_VOLUME_DOWN;	// 
		}
	}

    	if (p_key_action)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}
void bt_sink_music_change_respone_vp(uint8_t oldvol,uint8_t newvol)
{
	APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp oldvol=0x%x,newvol=0x%x,max=0x%x", 3, oldvol,newvol,AUD_VOL_OUT_MAX);	
	if (oldvol==newvol&&newvol!=AUD_VOL_OUT_LEVEL15&&newvol!=0)
	{
		APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp no need play vp",0);
	return;
	}
	else 
	{
		if(newvol>=AUD_VOL_OUT_MAX){
			APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp play maxvol",0);
			voice_prompt_play_sync_vp_maxvol();  
		}
		else if(newvol==0){
			voice_prompt_play_sync_vp_minvol();  
			APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp play minvol",0);

		}
		else if(newvol>oldvol){
			voice_prompt_play_sync_vp_volume_up();
			APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp play up vp",0);

		}
		else if(newvol<oldvol){
			voice_prompt_play_sync_vp_volume_down();
			APPS_LOG_MSGID_I("bt_sink_music_change_respone_vp play down vp",0);

		}
		else{
			APPS_LOG_MSGID_E("bt_sink_music_change_respone_vp errorr",0);

		}
	}

}

void key_avrcp_next_proc(uint8_t vol_m_flag)			// 0: VOLUP_DP, 1: MBUTTON_DP
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;

	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	if(app_mmi_state == APP_HFP_INCOMING)
	{
		*p_key_action = KEY_REJCALL;
	}
	else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
		||app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
	{
		*p_key_action = KEY_END_CALL;
	}
	else
	{
	 //(1 << APP_A2DP_PLAYING) | (1 << APP_ULTRA_LOW_LATENCY_PLAYING) | (1 << APP_WIRED_MUSIC_PLAY)
		//*p_key_action = KEY_HEARING_AID_MODE_UP_CIRCULAR;
  			*p_key_action = KEY_AVRCP_FORWARD;	
	}

    	if (p_key_action)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_avrcp_prev_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
    	if (p_key_action)
	{
		*p_key_action = KEY_AVRCP_BACKWARD;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_fact_pairing_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
    	if (p_key_action)
	{
		*p_key_action = KEY_DISCOVERABLE;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_send_address_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
    	if (p_key_action)
	{
		*p_key_action = KEY_SEND_ADDRESS;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_write_sirk_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    	if (p_key_action)
	{
		*p_key_action = KEY_WRITE_SIRK_KEY;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 100);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_anc_ha_display_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    	if (p_key_action)
	{
		*p_key_action = KEY_ANC_HA_DISPLAY;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 300);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

void key_switch_anc_and_passthrough_proc(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
    	if (p_key_action)
	{
		*p_key_action = KEY_ANC_AND_HA;
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
    	}
    	else
    	{
        	vPortFree(p_key_action);
    	}		
}

bool key_multifunction_short_click()
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	if(app_mmi_state == APP_HFP_INCOMING)
	{
		*p_key_action = KEY_ACCEPT_CALL;
	}
	else if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
	{
		*p_key_action = KEY_AVRCP_PAUSE;
	}
	else if(app_mmi_state == APP_CONNECTED)
	{
		*p_key_action = KEY_AVRCP_PLAY;
	}

	/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
	}
    	else
    	{
        	vPortFree(p_key_action);
    	}

	return true;
}

#endif

bool normal_ui_short_click()
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	if(app_mmi_state == APP_HFP_INCOMING)
	{
		*p_key_action = KEY_ACCEPT_CALL;
	}
	else if(app_mmi_state == APP_HFP_TWC_INCOMING)
	{
		*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
	}
	else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
			||app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
	{
		*p_key_action = KEY_END_CALL;
	}
	else if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
	{
		*p_key_action = KEY_AVRCP_PAUSE;
	}
	else if(app_mmi_state == APP_CONNECTED)
	{
		*p_key_action = KEY_AVRCP_PLAY;
	}
	else if(app_mmi_state == APP_STATE_VA)
	{
		*p_key_action = KEY_INTERRUPT_VOICE_ASSISTANT;
	}

	/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }

	return true;
}

bool normal_and_mini_ui_double_click(uint8_t mini_ui_flag)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	if(mini_ui_flag)
	{
		//play & pause(mini_ui)
		if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
		{
			*p_key_action = KEY_AVRCP_PAUSE;
		}
		else if(app_mmi_state == APP_CONNECTED)
		{
			*p_key_action = KEY_AVRCP_PLAY;
		}
		//answer & end call
		else if(app_mmi_state == APP_HFP_INCOMING)
		{
			*p_key_action = KEY_ACCEPT_CALL;
		}
		else if(app_mmi_state == APP_HFP_TWC_INCOMING)
		{
			*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
		}
		else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
			|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
		{
			*p_key_action = KEY_END_CALL;
		}
	}
	else
	{
		if(app_mmi_state == APP_HFP_INCOMING)
		{
			*p_key_action = KEY_REJCALL;
		}
		else if(app_mmi_state == APP_HFP_TWC_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE)
		{
			*p_key_action = KEY_ONHOLD_CALL;
		}
		else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO)
		{
			//*p_key_action = KEY_ONHOLD_CALL;
		}
		else if(app_mmi_state == APP_HFP_TWC_INCOMING)
		{
			*p_key_action = KEY_REJCALL_SECOND_PHONE;
		}
		else if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
		{
			*p_key_action = KEY_AVRCP_FORWARD;
		}
	}

	/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }

	return true;

}

bool normal_and_mini_ui_triple_click(uint8_t mini_ui_flag)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();
	if(mini_ui_flag)
	{
		//reject call
		if(app_mmi_state == APP_HFP_INCOMING)
		{
			*p_key_action = KEY_REJCALL;
		}
		else if(app_mmi_state == APP_HFP_TWC_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE)
		{
			*p_key_action = KEY_ONHOLD_CALL;
		}
		else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO)
		{
			//*p_key_action = KEY_ONHOLD_CALL;
		}
		else if(app_mmi_state == APP_HFP_TWC_INCOMING)
		{
			*p_key_action = KEY_REJCALL_SECOND_PHONE;
		}
	}
	else
	{
#ifdef PROJECT_KALLA
		if(app_mmi_state == APP_DISCONNECTED)
		{
			*p_key_action = KEY_AIR_PAIRING;
		}
#endif
/*
	#ifdef AIR_LE_AUDIO_BIS_ENABLE
    {
		if(app_mmi_state == APP_DISCONNECTED || app_mmi_state == APP_CONNECTABLE || app_mmi_state == APP_CONNECTED)
		{
			*p_key_action = KEY_LE_AUDIO_BIS_SCAN;
		}

		if(app_mmi_state == APP_LE_AUDIO_BIS_PLAYING)
		{
			*p_key_action = KEY_LE_AUDIO_BIS_STOP;
		}
    }
	#endif

	#if defined(SUPPORT_ROLE_HANDOVER_SERVICE) && defined(MTK_AWS_MCE_ENABLE)
		if(app_mmi_state == APP_CONNECTED || app_mmi_state == APP_HFP_INCOMING)
		{
			*p_key_action = KEY_RHO_TO_AGENT;
		}
	#endif
*/
		if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state == APP_CONNECTED)
		{
			*p_key_action = KEY_AVRCP_BACKWARD;
		}

	}

	/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }


	return true;
}
#ifdef BLE_ZOUND_ENABLE	
bool customer_key_configure_short_click(void){
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();

	uint8_t* abc = app_nvkey_customer_all().action_button_configuration;
	uint8_t action_press = 0;
	uint8_t action_button_event_notify[3] = { 0 };

	bool ret = true;

	action_button_event_notify[0] = ami_get_audio_channel();
	action_button_event_notify[1] = 0x00;

	if (abc[0] == action_button_event_notify[0]) {
		action_press = abc[1];
	} else {
		action_press = abc[7];
	}
	action_button_event_notify[2] = action_press;
	gatt_zound_service_notify(action_button_event_notify, sizeof(action_button_event_notify), BLE_CHARACTERISTIC_ZOUND_ACTION_BUTTON_EVENT_T);
	if (action_press ==0)
	{
		//ret = normal_ui_short_click();
		ret = true;
	}
	else
	{
		switch (action_press)
		{
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE_ANSWER_ENDCALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_BACK:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_BACKWARD;
				}

				break;
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_FORWARD_ANSWER_END_CALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_FORWARD;
				}else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}				
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			default:
				break;
		}
	}

	if(app_mmi_state == APP_STATE_VA)
	{
		*p_key_action = KEY_INTERRUPT_VOICE_ASSISTANT;
	}	

		/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }
	return ret;
}
bool customer_key_configure_double_click(void){
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();

	uint8_t* abc = app_nvkey_customer_all().action_button_configuration;
	uint8_t action_press = 0;
	uint8_t action_button_event_notify[3] = { 0 };
	//uint8_t mini_ui = app_nvkey_ble_mini_ui_read();

	bool ret = true;

	action_button_event_notify[0] = ami_get_audio_channel();
	action_button_event_notify[1] = 0x01;

	if (abc[0] == action_button_event_notify[0]) {
		action_press = abc[2];
	} else {
		action_press = abc[8];
	}
	action_button_event_notify[2] = action_press;
	gatt_zound_service_notify(action_button_event_notify, sizeof(action_button_event_notify), BLE_CHARACTERISTIC_ZOUND_ACTION_BUTTON_EVENT_T);
	if (action_press ==0)
	{
		//ret = normal_and_mini_ui_double_click(mini_ui);
		ret = true;
	}
	else
	{
		switch (action_press)
		{
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE_ANSWER_ENDCALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}				
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_BACK:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_BACKWARD;
				}

				break;
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_FORWARD_ANSWER_END_CALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_FORWARD;
				}else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			default:
				break;
		}
	}

		/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }

	return ret;
}
bool customer_key_configure_triple_click(void){
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();

	uint8_t* abc = app_nvkey_customer_all().action_button_configuration;
	uint8_t action_press = 0;
	uint8_t action_button_event_notify[3] = { 0 };
	//uint8_t mini_ui = app_nvkey_ble_mini_ui_read();

	bool ret= true;

	action_button_event_notify[0] = ami_get_audio_channel();
	action_button_event_notify[1] = 0x02;

	if (abc[0] == action_button_event_notify[0]) {
		action_press = abc[3];
	} else {
		action_press = abc[9];
	}
	action_button_event_notify[2] = action_press;
	gatt_zound_service_notify(action_button_event_notify, sizeof(action_button_event_notify), BLE_CHARACTERISTIC_ZOUND_ACTION_BUTTON_EVENT_T);
	if (action_press ==0)
	{
		//ret = normal_and_mini_ui_triple_click(mini_ui);
		ret = true;
	}
	else
	{
		switch (action_press)
		{
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE_ANSWER_ENDCALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}				
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_BACK:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_BACKWARD;
				}
				break;
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_FORWARD_ANSWER_END_CALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_FORWARD;
				}else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_TWC_INCOMING)
				{
					*p_key_action = KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
				}
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			default:
				break;
		}
	}

#ifdef PROJECT_KALLA
	if(*p_key_action == KEY_ACTION_INVALID && app_mmi_state == APP_DISCONNECTED)
	{
		*p_key_action = KEY_AIR_PAIRING;
	}
#endif
		/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }
	return ret;
}
bool customer_key_configure_slong_press(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();

	uint8_t* abc = app_nvkey_customer_all().action_button_configuration;
	uint8_t action_press = 0;
	uint8_t action_button_event_notify[3] = { 0 };

	action_button_event_notify[0] = ami_get_audio_channel();
	action_button_event_notify[1] = 0x04;

	if (abc[0] == action_button_event_notify[0]) {
		action_press = abc[5];
	} else {
		action_press = abc[11];
	}

#if CUS_LE_BIS_TEST
	action_press = 9;
#endif

	action_button_event_notify[2] = action_press;
	gatt_zound_service_notify(action_button_event_notify, sizeof(action_button_event_notify), BLE_CHARACTERISTIC_ZOUND_ACTION_BUTTON_EVENT_T);

	if (action_press == 0)
	{
#if 0
		if (app_mmi_state == APP_CONNECTED || app_mmi_state == APP_A2DP_PLAYING)
		{
#ifdef AIR_SPOTIFY_TAP_ENABLE
			*p_key_action = KEY_SPOTIFY_TAP_TRIGGER;
#else
			*p_key_action = KEY_SPOTIFY_TAP;
#endif

		}
#endif
	}
	else
	{
		switch (action_press)
		{
			case ZOUND_ACTION_BUTTON_SPOTIFY_TAP:
				if (app_mmi_state == APP_CONNECTED || app_mmi_state == APP_A2DP_PLAYING
#if CUS_LE_BIS_TEST
					|| app_mmi_state == APP_DISCONNECTED || app_mmi_state == APP_CONNECTABLE
#endif
					)
				{
#ifdef AIR_SPOTIFY_TAP_ENABLE
#if CUS_LE_BIS_TEST
					*p_key_action = KEY_SPOTIFY_TAP;
#else
					*p_key_action = KEY_SPOTIFY_TAP_TRIGGER;
#endif
#else
					*p_key_action = KEY_SPOTIFY_TAP;
#endif

				}
				break;
			case ZOUND_ACTION_BUTTON_VOLUME_UP:
			case ZOUND_ACTION_BUTTON_VOLUME_DOWN:
				if((app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING || app_mmi_state == APP_LE_AUDIO_BIS_PLAYING)
					||app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
				{
					zound_volume_task_send_msg(ZOUND_VOLUME_RUN);
					zound_volume_task_send_msg(action_press);
				}
				break;
			default:
				break;
		}
	}

		/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }

	return true;
}
#endif
void customer_key_configure_long_press1(void)
{
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
//	apps_config_state_t app_mmi_state = apps_config_key_get_mmi_state();

	uint8_t* abc = app_nvkey_customer_all().action_button_configuration;
	uint8_t action_press = 0;
	uint8_t action_button_event_notify[3] = { 0 };

	action_button_event_notify[0] = ami_get_audio_channel();
	action_button_event_notify[1] = 0x03;

	if (abc[0] == action_button_event_notify[0]) {
		action_press = abc[4];
	} else {
		action_press = abc[10];
	}
	action_button_event_notify[2] = action_press;

#ifdef BLE_ZOUND_ENABLE
	gatt_zound_service_notify(action_button_event_notify, sizeof(action_button_event_notify), BLE_CHARACTERISTIC_ZOUND_ACTION_BUTTON_EVENT_T);
#endif

	if (action_press ==0)
	{
#if 0
		if(app_mmi_state == APP_DISCONNECTED || app_mmi_state == APP_CONNECTABLE
			|| app_mmi_state == APP_CONNECTED || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING
			|| app_mmi_state == APP_LE_AUDIO_BIS_PLAYING || app_mmi_state == APP_A2DP_PLAYING
#ifndef MUTE_LOCAL_MIC
			||app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO
			|| app_mmi_state == APP_HFP_MULTITPART_CALL || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_STATE_VA
#endif
			)
		{
			*p_key_action = KEY_SWITCH_ANC_AND_PASSTHROUGH;
		}
#ifdef MUTE_LOCAL_MIC
		else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO
			|| app_mmi_state == APP_HFP_MULTITPART_CALL || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_STATE_VA)
		{
			*p_key_action = KEY_MUTE_MIC;
		}
#endif
#endif
	}
#ifdef BLE_ZOUND_ENABLE
	else if(app_mmi_state == APP_HFP_INCOMING)
	{
		*p_key_action = KEY_REJCALL;
	}
	else if(app_mmi_state == APP_HFP_TWC_INCOMING)
	{
		*p_key_action = KEY_REJCALL_SECOND_PHONE;
	}else{
		switch (action_press)
		{
			case ZOUND_ACTION_BUTTON_VOICE_ASSISTANT:
				if(app_mmi_state == APP_CONNECTED || app_mmi_state == APP_A2DP_PLAYING
					|| app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_WAKE_UP_VOICE_ASSISTANT;
				}
				break;
			case ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_TRA:
			case ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC_TRA:
			case ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_NC_TRA:
			case ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC:{
				if(app_mmi_state == APP_DISCONNECTED || app_mmi_state == APP_CONNECTABLE
					|| app_mmi_state == APP_CONNECTED || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING
					|| app_mmi_state == APP_LE_AUDIO_BIS_PLAYING || app_mmi_state == APP_A2DP_PLAYING
#ifndef MUTE_LOCAL_MIC
					||app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO
					|| app_mmi_state == APP_HFP_MULTITPART_CALL || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_STATE_VA
#endif
					)

				{
					if(action_press == ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_TRA)
					{
						*p_key_action = KEY_PASS_THROUGH;
					}
					else if(action_press == ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC_TRA)
					{
						*p_key_action = KEY_SWITCH_ANC_AND_PASSTHROUGH;
					}
					else if(action_press == ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_NC_TRA)
					{
						*p_key_action = KEY_BETWEEN_ANC_PASSTHROUGH;
					}
					else if(action_press == ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC)
					{
						*p_key_action = KEY_ANC;
					}
					
					if(*p_key_action!=KEY_ACTION_INVALID){
        				ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
					}else{
						vPortFree(p_key_action);
					}
				}
				APPS_LOG_MSGID_I("long_press1 key action[anc]=0x%x", 1, *p_key_action);
				return;
			}
			case ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_EQ_SLOTS_1_2:
			{
#if defined (PROJECT_KALLA)
				break;
#endif
        		voice_prompt_param_t vp = {0};
				if(bt_sink_srv_cm_get_aws_connected_device() != NULL)
				{
					vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
					vp.delay_time = 200;
				}
				uint8_t* eqs = app_nvkey_customer_all().eq_settings;
				eqs[0] = eqs[0] + 1 > 1?0:1;
				if (eqs[0] == 0) {
                	vp.vp_index = VP_INDEX_SWITCH_EQ_PRESET_SLOT_1;
				} else if (eqs[0] == 1) {
					vp.vp_index = VP_INDEX_SWITCH_EQ_PRESET_SLOT_2;
				}
				if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_AGENT) {
					sync_event(EVENT_ID_PROMPT_SYNC, &vp, sizeof(voice_prompt_param_t));
				}else{
        			voice_prompt_play(&vp, NULL);
				}
        		set_peq_group_id(eqs[eqs[0]+1]+1);
				app_nvkey_eq_settings_set(eqs, 3);
				sync_event(EVENT_ID_EQ_SETTINGS_SYNC, eqs, 3);
    			uint8_t cache[7] = {0};
				memset(cache,0,7);
				cache[0] = 0xff;
				cache[1] = 0x02;
				memmove(cache+2,eqs,1);
				cache[3] = 0x01;
				cache[4] = 0x01;
				memmove(cache+5,eqs+1,2);
				gatt_zound_service_notify(cache, 7, BLE_CHARACTERISTIC_ZOUND_EQUALIZER_SETTINGS_T);
				break;
			}
			case ZOUND_ACTION_BUTTON_SPOTIFY_TAP:
				if (app_mmi_state == APP_CONNECTED || app_mmi_state == APP_A2DP_PLAYING)
				{
				#ifdef AIR_SPOTIFY_TAP_ENABLE
					*p_key_action = KEY_SPOTIFY_TAP_TRIGGER;
				#else
					*p_key_action = KEY_SPOTIFY_TAP;
				#endif
				}
				break;
			case ZOUND_ACTION_BUTTON_VOLUME_UP:
			case ZOUND_ACTION_BUTTON_VOLUME_DOWN:
				if((app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING || app_mmi_state == APP_LE_AUDIO_BIS_PLAYING)
					||app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state ==APP_CONNECTED)
				{				
					zound_volume_task_send_msg(ZOUND_VOLUME_RUN);
					zound_volume_task_send_msg(action_press);
				}
				break;
			case ZOUND_ACTION_BUTTON_PLAY_PAUSE_ANSWER_ENDCALL:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY)
				{
					*p_key_action = KEY_AVRCP_PAUSE;
				}
				else if(app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_PLAY;
				}
				else if(app_mmi_state == APP_HFP_INCOMING)
				{
					*p_key_action = KEY_ACCEPT_CALL;
				}
				else if(app_mmi_state == APP_HFP_CALLACTIVE || app_mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO || app_mmi_state == APP_HFP_MULTITPART_CALL
					|| app_mmi_state == APP_HFP_OUTGOING || app_mmi_state == APP_STATE_HELD_ACTIVE || app_mmi_state == APP_HFP_TWC_OUTGOING)
				{
					*p_key_action = KEY_END_CALL;
				}
				break;
			case ZOUND_ACTION_BUTTON_SKIP_BACK:
				if(app_mmi_state == APP_A2DP_PLAYING || app_mmi_state == APP_ULTRA_LOW_LATENCY_PLAYING || app_mmi_state == APP_WIRED_MUSIC_PLAY || app_mmi_state == APP_CONNECTED)
				{
					*p_key_action = KEY_AVRCP_BACKWARD;
				}

				break;
#if defined (SPP_AIR_ZOUND_ADIDAS_RUNNING)
			case ZOUND_ACTION_BUTTON_ADIDAS_RUNNING_START_STOP:{
				write_adidas_running(0x01);
				break;
			}
			case ZOUND_ACTION_BUTTON_ADIDAS_RUNNING_PAUSE_RESUME:{
				write_adidas_running(0x02);
				break;
			}
#endif
			default:
				break;
		}
	}

#endif
	APPS_LOG_MSGID_I("long_press1 key action=0x%x", 1, *p_key_action);
	/* Send key action */
	if (*p_key_action != KEY_ACTION_INVALID)
	{
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 50);
	}
    else
    {
        vPortFree(p_key_action);
    }

}

static bool app_customer_common_anc_and_pass_through_handle(void)
{
#if 0
    uint8_t anc_enable;
    audio_anc_control_filter_id_t anc_current_filter_id;
    audio_anc_control_type_t anc_current_type;
    int16_t anc_runtime_gain;
    uint8_t support_hybrid_enable;
    bool control_ret = FALSE;
	
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, NULL);

	if(anc_enable)
	{
		control_ret = app_anc_service_enable(anc_current_filter_id, anc_current_type, AUDIO_ANC_CONTROL_UNASSIGNED_GAIN, NULL);
	}
	else
	{
		control_ret = app_anc_service_disable();
	}

	
    APPS_LOG_MSGID_I("anc_and_pass_through Sync, enable%d anc filter %d ret: %d", 3,
                     anc_enable, anc_current_filter_id, control_ret);
#endif
	return true;
}

static bool _bt_sink_event_group_proc(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)extra_data;

    switch (event_id)
    {

#ifdef MTK_AWS_MCE_ENABLE
        case BT_SINK_SRV_EVENT_AWS_MCE_STATE_CHANGE: {
            if (BT_AWS_MCE_AGENT_STATE_ATTACHED == event->aws_state_change.aws_state)
            {
#ifdef BLE_ZOUND_ENABLE
        		notify_battery();
#endif
                /* Only agent role will enable this module, so don't need judge the role
                 * in event handle. */
                APPS_LOG_MSGID_I("[APP_CUS_COMM]AWS attached.", 0);

				uint8_t anc_bk_status = app_nvkey_anc_status_bk_read();

				if(anc_bk_status >= 0x80)
				{
					if(anc_bk_status == 0x81)
						app_anc_service_enable(AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT, AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT, AUDIO_ANC_CONTROL_UNASSIGNED_GAIN, NULL);
					else if(anc_bk_status == 0x82)
						app_anc_service_enable(AUDIO_ANC_CONTROL_FILTER_ID_ANC_2, AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT, AUDIO_ANC_CONTROL_UNASSIGNED_GAIN, NULL);
					else if(anc_bk_status == 0x80)
						app_anc_service_disable();

					app_set_anc_status_bk(anc_bk_status-0x80);
				}
				else
				{
	                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ANC_STATUS_SYNC);
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
										EVENT_ID_ANC_STATUS_SYNC, NULL, 0,
										NULL, 2000);
				}
            }
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}



static bool _proc_customer_common(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = false;
    switch (event_id)
    {
    	case EVENT_ID_RD_DEBUG:
    		{
              uint8_t * rec_data=(uint8_t *)extra_data;
              for(int i=0;i<data_len;i++)
              {
        	      APPS_LOG_MSGID_I("_proc_customer_common rec_data+%d *(rec_data+i)=%d \r\n", 2,i,*(rec_data+i));
              }
            
				ret = true;
				break;
    		}
			case EVENT_ID_EASTECH_DELAY_PLAY_ANC_VP:
    		{
				APPS_LOG_MSGID_I("app_customer_common_activity EVENT_ID_EASTECH_DELAY_PLAY_ANC_VP", 0);
				voice_prompt_play_sync_vp_anc_on();	//// harry for app ha vp 20240904	
				ret = true;
				break;
    		}
			#ifdef EASTECH_SPEC_LOW_BATTERY_VP
			case EVENT_ID_EASTECH_LOOP_LOW_BATTERY_VP:
    		{
				APPS_LOG_MSGID_I("app_customer_common_activity EVENT_ID_EASTECH_LOOP_LOW_BATTERY_VP play_lowbatt_flag=%d", 1,play_lowbatt_flag);
				voice_prompt_param_t vp = {0};
				vp.vp_index = VP_INDEX_LOW_BATTERY;
				voice_prompt_play(&vp, NULL);
				play_lowbatt_flag=1;
				ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_LOOP_LOW_BATTERY_VP, NULL,
					0, NULL, 10*60*1000);
				
				ret = true;
				break;
    		}			
			#endif



    	case EVENT_ID_CUSTOMER_COMMON_PROCESS_INPUT:
    		{

				ret = true;
				break;
    		}

    	case EVENT_ID_CASE_BT_CLEAR_BT_ON:
    		{
#ifdef AIR_SMART_CHARGER_ENABLE    		
				app_smcharger_bt_clear_enter_discoverable(APP_SMCHARGER_REQUEST_BT_ON);
#endif				
				ret = true;
				break;
    		}

		case EVENT_ID_TAKE_OVER_CLEAN:
			{
				app_bt_conn_take_over_clean();
				ret = true;
				break;
			}

		case EVENT_ID_HX300X_READ_ALL_REG:
			{
				HX300X_read_all_reg();
				ret = true;
				break;
			}		

		case EVENT_ID_DELAY_NOTIFY_BAT:
			{
//				notify_battery();
				ret = true;
				break;
			}		

#ifdef BATTERY_HEATHY_ENABLE
		case EVENT_ID_BATTERY_HEATHY_CYCLE_START:
			{
				if(battery_heathy_state != EVENT_ID_BATTERY_HEATHY_CYCLE_START)
				{
					battery_heathy_state = EVENT_ID_BATTERY_HEATHY_CYCLE_START;
					battery_percent_out = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
					APPS_LOG_MSGID_I("app_customer_common_activity battery heathy out: %d%", 1, battery_percent_out);

				}
				ret = true;
				break;
			}

		case EVENT_ID_BATTERY_HEATHY_CYCLE_END:
			{
				if(battery_heathy_state == EVENT_ID_BATTERY_HEATHY_CYCLE_START)
				{
					uint8_t percent = 0;

					battery_heathy_state = EVENT_ID_BATTERY_HEATHY_CYCLE_END;
					if(battery_percent_backup != 0)
					{
						battery_percent_in = battery_percent_backup;
						battery_percent_backup = 0;
					}
					else
						battery_percent_in = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);

					APPS_LOG_MSGID_I("app_customer_common_activity battery heathy in: %d%", 1, battery_percent_in);

					percent = (battery_percent_out>battery_percent_in)? (battery_percent_out-battery_percent_in):0;

					if(percent>100)
					{
						APPS_LOG_MSGID_E("app_customer_common_activity battery heathy error: %d%", 1, percent);
					}
					else
					{
						app_nvkey_battery_heathy_write(percent, false);
					}
				}
				ret = true;
				break;
			}
#endif
			case EVENT_ID_ANC_SWITCH:{
				uint8_t anc_enable;
				uint8_t anc_state = 0;
				audio_anc_control_filter_id_t anc_current_filter_id;
				audio_anc_control_type_t anc_current_type;
				int16_t anc_runtime_gain;
				uint8_t support_hybrid_enable;
				audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, NULL);
    			if (anc_enable &&
#if PT_USE_ANC_FILTER
        		AUDIO_ANC_CONTROL_FILTER_ID_ANC_2 == anc_current_filter_id
#else
        		AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT == anc_current_filter_id
#endif
    			){
        			anc_state = 2;
    			} else if (anc_enable && AUDIO_ANC_CONTROL_FILTER_ID_ANC_1 == anc_current_filter_id) {
        			anc_state = 1;
    			}
				uint8_t flag = *(uint8_t*)extra_data;
				if(flag == 3){
					flag = 0;
				}
				if(flag == anc_state){
					break;
				}
				switch (flag) {
				case 0:
				{
					if (anc_enable && AUDIO_ANC_CONTROL_FILTER_ID_ANC_1 == anc_current_filter_id) {
//						zound_switch_anc(ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC);
					} else if (anc_enable &&
#if PT_USE_ANC_FILTER
						AUDIO_ANC_CONTROL_FILTER_ID_ANC_2 == anc_current_filter_id
#else
						AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT == anc_current_filter_id
#endif
						) {
//						zound_switch_anc(ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_TRA);
					}
					break;
				}
				case 1:
				{
					if (!anc_enable || AUDIO_ANC_CONTROL_FILTER_ID_ANC_1 != anc_current_filter_id) {
//						zound_switch_anc(ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_NC);
					}
					break;
				}
				case 2:
				{
					if (!anc_enable ||
#if PT_USE_ANC_FILTER
						AUDIO_ANC_CONTROL_FILTER_ID_ANC_2 != anc_current_filter_id
#else
						AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT != anc_current_filter_id
#endif
						) {
//						zound_switch_anc(ZOUND_ACTION_BUTTON_TOGGLE_BETWEEN_PBO_TRA);
					}
					break;
				}
				}
				break;
			}	
/*			case EVENT_ID_DEVICE_MFI_CHECK:
			{
				#ifdef MTK_IAP2_PROFILE_ENABLE
					RACE_FACTORY_TEST_MFI_STATUS_NOTIFY(factory_test_air_check_mfi_chip());
				#endif
				break;
			}*/

			case EVENT_ID_ANC_STATUS_SYNC:
			{
				ret = app_customer_common_anc_and_pass_through_handle();
				break;
			}
			case EVENT_ID_SWITCH_HOLD:{
				uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
				*p_key_action = *(uint16_t*)extra_data;
				ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 10);
				ret = true;
				break;
			}
			case EVENT_ID_BLE_BROADCAST_INTERVAL:
			{
//				set_ble_broadcast_interval(0xF4);
//				default_ble_adv_update();
				break;
			}

#ifdef ALWAYS_PLAY_PAIRING_VP
	    case EVENT_ID_EASTECH_PRE_PAIR_VP:	
		    log_hal_msgid_info("_proc_customer_common VP_INDEX_PAIRING EVENT_ID_EASTECH_PRE_PAIR_VP",0);
           app_eastech_voice_prompt_play_pairing();
           ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,EVENT_ID_EASTECH_CALLBACK_PAIR_VP);
         	ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE,
                           EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
                           (uint32_t)EVENT_ID_EASTECH_CALLBACK_PAIR_VP,
                           NULL, 0,
                           NULL, VP_PLAY_INTERVAL);
		break;


	    case EVENT_ID_EASTECH_CALLBACK_PAIR_VP:	
		log_hal_msgid_info("_proc_customer_common EASTECH_CALLBACK_PAIR_VP",0);
		app_eastech_pair_vp_callback();
		break;
#endif

	    case EVENT_ID_EASTECH_CALLBACK_OCEAN_VP:	
		if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
		&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
		{
			log_hal_msgid_info("_proc_customer_common role=partner,bacause rho occurs,the earbud from agent change to partner,so stop vppp",0);
       			ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,EVENT_ID_EASTECH_CALLBACK_OCEAN_VP);
	 		voice_prompt_stop(VP_INDEX_Ocean,VOICE_PROMPT_ID_INVALID,false);	
		}
		else
		{
			log_hal_msgid_info("_proc_customer_common EVENT_ID_EASTECH_CALLBACK_OCEAN_VP role=agent  repeat play vp role=0x%x,ocean_cnt=%d",2,bt_device_manager_aws_local_info_get_role(),ocean_cnt);
			customer_key_triger_ocean_vp();	
		}
		break;

      
		default:
			break;
    }

	return ret;

}
#define SN_LEN  10

static bool _customer_common_app_aws_data_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
     bool *isInEar = NULL;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION)
    {
        uint32_t event_group;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

		apps_customer_common_local_context_t* p_local_context = &s_app_customer_common_context;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &event_id, &p_extra_data, &extra_data_len);

        if(event_group == EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON)
        {
            switch (event_id)
            {
				case EVENT_ID_SHIPPING_MODE_STATE_SYNC:
				{
					uint8_t shipping_mode_state[1] = {0};

					memcpy(shipping_mode_state,((uint8_t *)(aws_data_ind->param))+(2*sizeof(uint32_t)), 1);

					APPS_LOG_MSGID_I(" app customer setting sync:: shipping mode = %d", 1, shipping_mode_state[0]);

					app_set_shipping_mode_state(shipping_mode_state[0]);
				}
				ret = true;
				break;

				case EVENT_ID_DEVICE_COLOR_STATE_SYNC:
				{
					uint8_t device_color_state[1] = {0};

					memcpy(device_color_state,((uint8_t *)(aws_data_ind->param))+(2*sizeof(uint32_t)), 1);

					APPS_LOG_MSGID_I(" app customer setting sync:: device_color = %d", 1, device_color_state[0]);

					app_set_ble_device_color_state(device_color_state[0]);
				}
				ret = true;
				break;

				case EVENT_ID_ANC_BK_STATUS_SYNC:
				{
					APPS_LOG_MSGID_I(" app customer setting sync:: anc bk status = 0x%x", 1, *(uint16_t*)p_extra_data);
				
					app_set_anc_status_bk(*(uint16_t*)p_extra_data);
				}
				ret = true;
				break;

				case EVENT_ID_DEVICE_MINI_UI_STATE_SYNC:
				{
					uint8_t mini_ui_state[1] = {0};

					memcpy(mini_ui_state,((uint8_t *)(aws_data_ind->param))+(2*sizeof(uint32_t)), 1);

					APPS_LOG_MSGID_I(" app customer setting sync:: mini_ui_state = %d", 1, mini_ui_state[0]);

					app_set_ble_device_mini_ui_state(mini_ui_state[0]);
				}
				ret = true;
				break;

				case EVENT_ID_DEVICE_LEA_DISABLE_STATE_SYNC:
				{
					APPS_LOG_MSGID_I(" app customer setting sync:: LEA_DISABLE_STATE = %d", 1, *(uint16_t*)p_extra_data);

					app_nvkey_is_lea_disable_set(*(uint16_t*)p_extra_data);

					uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */
					
					*p_key_action = KEY_SYSTEM_REBOOT;
					
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
										sizeof(uint16_t), NULL, 2000);		
					
				}
				ret = true;
				break;
				

				case EVENT_ID_ECO_CHARGING_STATUS_SYNC:
				{
					uint8_t eco_charging_status[1] = {0};

					memcpy(eco_charging_status,((uint8_t *)(aws_data_ind->param))+(2*sizeof(uint32_t)), 1);

					APPS_LOG_MSGID_I(" app customer setting sync:: eco status = %d", 1, eco_charging_status[0]);

					app_set_eco_charing_profile_switch(eco_charging_status[0]);
				}
				ret = true;
				break;

				case EVENT_ID_CUSTOMER_COMMON_CUSTOMER_DATA_REQUEST:
				{
#if 0
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
										EVENT_ID_PSENSOR_AGENT_SYNC_CONTEXT_TO_PARTNER, NULL, 0,
										NULL, 0);
#endif

					app_customer_common_setting_to_partner();
				}
				ret = true;
				break;

				case EVENT_ID_CUSTOMER_COMMON_CUSTOMER_DATA_SYNC:
				{
					app_customer_setting_sync_t sync_data;

					memcpy(&sync_data,((uint8_t *)(aws_data_ind->param))+(2*sizeof(uint32_t)), sizeof(app_customer_setting_sync_t));
					race_debug_print((uint8_t*)&sync_data, sizeof(app_customer_setting_sync_t),"customer setting sync:");
					peer_battery_heathy_times = sync_data.bat_heathy_times;
					app_customer_common_sync_aws_data((void*)&sync_data);
                    ret = true;
                    break;
                }
				case EVENT_ID_PSENSOR_CALIBRATE_ACTION_SYNC:
				{
					bsp_px31bf_auto_dac();
                    ret = true;
                    break;
                }

				case EVENT_ID_HX300X_DEBUG_SYNC:
				{
				  	app_set_hx300x_debug_state(*(uint16_t*)p_extra_data);
				}
				ret = true;
				break;				

				case EVENT_ID_BATTERY_HEATHY_SYNC:
				{
					peer_battery_heathy_times = *(uint16_t*)p_extra_data;

					APPS_LOG_MSGID_I(" battery heathy times sync: times = %d", 1, peer_battery_heathy_times);

					if(peer_battery_heathy_times == 0)
					{
						app_nvkey_battery_heathy_write(0, true);
					}
					ret = true;
					break;
				}

				case EVENT_ID_DEVICE_SN_SYNC:
					race_debug_print((uint8_t*)p_extra_data, extra_data_len,"sn sync:");
					app_set_ble_write_SN((uint8_t*)p_extra_data, SN_LEN);
					ret = true;
					break;
					#ifdef FACTORY_TEST_FOR_POWEROFF
					case EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE:
					APPS_LOG_MSGID_I("app customer setting sync:[EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE] vol=%d", 1,*(uint8_t*)p_extra_data);
					app_nvkey_action_factory_autopoweroff_write(*(uint8_t*)p_extra_data);
					ret = true;
					break;
					case EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE_READ_START:
					app_nvkey_action_factory_autopoweroff_read_start_1();
					APPS_LOG_MSGID_I("app customer setting sync:[EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE_READ_START]", 0);
					ret = true;
					break;
					case EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE_READ_END:
					APPS_LOG_MSGID_I("app customer setting sync:[EVENT_ID_EASTECH_FACTORY_POWEROFF_MODE_READ_END] vol=%d", 1,*(uint8_t*)p_extra_data);
					app_nvkey_action_factory_autopoweroff_rece_peervol(*(uint8_t*)p_extra_data);
					ret = true;
					break;
					
					#else
					errrrrrrrrrrrrrrrrrrrrrrr
					#endif
				case EVENT_ID_DEVICE_SN_COMPARE:
					race_debug_print((uint8_t*)p_extra_data, extra_data_len,"sn cmp:");
					app_write_sn_compare((uint8_t*)p_extra_data, extra_data_len);
					ret = true;
					break;					

				case EVENT_ID_DEVICE_HW_VER_SYNC:
					app_nvkey_hw_version_set((uint8_t*)p_extra_data, extra_data_len);
					ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
													APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
													NULL, 500);
					
					ret = true;
					break;

#ifdef BLE_ZOUND_ENABLE
				case EVENT_ID_ZOUND_EQ_SWITCH_SYNC:
				{
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ZOUND_EQ_SWITCH_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == 1){
						set_peq_group_id(*(uint8_t*)p_extra_data);
					}
                    ret = true;
					break;
				}
                case EVENT_ID_UI_SOUNDS_EVENT_SYNC:
                {
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_UI_SOUNDS_EVENT_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == sizeof(uint16_t)) {
                        app_nvkey_ui_sounds_write(*(uint16_t*)p_extra_data);
                    }
                    ret = true;
                    break;
                }
				case EVENT_ID_ECO_SETTINGS_EVENT_SYNC:
				{
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ECO_SETTINGS_EVENT_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == 8) {
                        app_nvkey_eco_setting_set((uint8_t*)p_extra_data);
                    }
                    ret = true;
                    break;
				}
                case EVENT_ID_ACTION_BUTTON_CONFIGURATION_EVENT_SYNC:
                {
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ACTION_BUTTON_CONFIGURATION_EVENT_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == sizeof(app_nvkey_customer_all().action_button_configuration)) {
                        app_nvkey_action_button_configuration_write((uint8_t*)p_extra_data);
                    }
                    ret = true;
                    break;
                }
                case EVENT_ID_TOUCH_LOCK_EVENT_SYNC:
                {
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_TOUCH_LOCK_EVENT_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == 1) {
                        app_nvkey_action_touch_lock_write(*(uint8_t*)p_extra_data);
                    }
                    ret = true;
                    break;
                }
                case EVENT_ID_APP_VOLUME_API_SYNC:
                {
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_APP_VOLUME_API_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == sizeof(uint16_t)) {
						uint16_t* p_key_action = (uint16_t*)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
						*p_key_action = *(uint16_t*)p_extra_data;
						if (*p_key_action != KEY_ACTION_INVALID) {
							ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 50);
						} else {
							vPortFree(p_key_action);
						}
                    }
                    ret = true;
                    break;
                }
                case EVENT_ID_APP_ANC_SYNC:
                {
                    LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_APP_ANC_SYNC] size:%d", 1, extra_data_len);
                    if (extra_data_len == 1) {
                        zound_switch_anc(*(uint8_t*)p_extra_data);
                    }
                    ret = true;
                    break;
                }
				case EVENT_ID_EQ_SETTINGS_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_EQ_SETTINGS_SYNC] size:%d", 1, extra_data_len);
					app_nvkey_eq_settings_set((uint8_t*)p_extra_data, extra_data_len);
                    ret = true;
					break;
				}
				case EVENT_ID_PROMPT_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_PROMPT_SYNC] size:%d", 1, extra_data_len);
					voice_prompt_param_t *vp = NULL;
					vp = pvPortMalloc(extra_data_len);
					memmove(vp,p_extra_data,extra_data_len);
        			voice_prompt_play(vp, NULL);
					if(vp){
            			vPortFree(vp);
					}
                    ret = true;
					break;
				}
				case EVENT_ID_ANC_LEVEL:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ANC_LEVEL] size:%d", 1, extra_data_len);
        			app_nvkey_anc_level_write(*(uint8_t*)p_extra_data);
                    ret = true;
					break;
				}
				case EVENT_ID_TRA_LEVEL:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_TRA_LEVEL] size:%d", 1, extra_data_len);
        			app_nvkey_tra_level_write(*(uint8_t*)p_extra_data);
                    ret = true;
					break;
				}
				case EVENT_ID_WEAR_SENSOR_ACTION:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_WEAR_SENSOR_ACTION] size:%d", 1, extra_data_len);
        			app_nvkey_wear_sensor_action_set((uint8_t*)p_extra_data, extra_data_len);
                    ret = true;
					break;
				}
				case EVENT_ID_APP_PLAY_PAUSE_STATE_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_APP_PLAY_PAUSE_STATE_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == 1){
						_g_set_play_status(*(uint8_t*)p_extra_data);
					}
                    ret = true;
					break;
				}
#ifdef SPP_AIR_ZOUND_ADIDAS_RUNNING
				case EVENT_ID_ADIDAS_RUNNING_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ADIDAS_RUNNING_SYNC] size:%d", 1, extra_data_len);
					write_adidas_running(*(uint8_t*)p_extra_data);
                    ret = true;
					break;
				}
#endif
				case EVENT_ID_ZOUND_NOTIFY_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ZOUND_NOTIFY_SYNC] size:%d", 1, extra_data_len);
					gatt_zound_notify_data *notify_data = NULL;
					notify_data = pvPortMalloc(extra_data_len);
					memmove(notify_data,p_extra_data,extra_data_len);
					gatt_zound_service_notify(notify_data->data,notify_data->length,notify_data->handle);
					if(notify_data){
            			vPortFree(notify_data);
					}
                    ret = true;
					break;
				}
				case EVENT_ID_ZOUND_NOTIFY_CCCD_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_ZOUND_NOTIFY_CCCD_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == sizeof(bt_gatts_config_info_t)*BT_GATTS_SRV_LE_CONNECTION_MAX){
						gatt_zound_service_notify_cccd_write(p_extra_data,extra_data_len);
					}
					ret = true;
					break;
				}
				case EVENT_ID_LEFT_BATTERY_NOTIFY_CCCD_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_LEFT_BATTERY_NOTIFY_CCCD_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == sizeof(bt_gatts_config_info_battery)*BT_GATTS_SRV_LE_CONNECTION_MAX){
						gatt_left_battery_service_cccd_write(p_extra_data,extra_data_len);
					}
					ret = true;
					break;
				}
				case EVENT_ID_RIGHT_BATTERY_NOTIFY_CCCD_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_RIGHT_BATTERY_NOTIFY_CCCD_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == sizeof(bt_gatts_config_info_battery)*BT_GATTS_SRV_LE_CONNECTION_MAX){
						gatt_right_battery_service_cccd_write(p_extra_data,extra_data_len);
					}
					ret = true;
					break;
				}
				case EVENT_ID_MAIN_BATTERY_NOTIFY_CCCD_SYNC:
				{
					LOG_MSGID_I(GATT_ZOUND_SERVICE, "[EVENT_ID_MAIN_BATTERY_NOTIFY_CCCD_SYNC] size:%d", 1, extra_data_len);
					if(extra_data_len == sizeof(bt_gatts_config_info_battery)*BT_GATTS_SRV_LE_CONNECTION_MAX){
						gatt_main_battery_service_cccd_write(p_extra_data,extra_data_len);
					}
					ret = true;
					break;
				}
#endif
				case EVENT_ID_ANC_KEY_SYNC:
				{
					anc_key_count = *(uint16_t*)p_extra_data;
					APPS_LOG_MSGID_I(" EVENT_ID_ANC_KEY_SYNC: anc_key_count = %d", 1, anc_key_count);
					ret = true;
					break;
				}
			    case EVENT_ID_EASTECH_CALLBACK_OCEAN_VP:	
			if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
			&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
			{
				ocean_cnt=*(uint16_t*)p_extra_data;
				log_hal_msgid_info("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_CALLBACK_OCEAN_VP receiver agent message; sync ocean_cnt val=%d",1,ocean_cnt);
			}
			else
			{

				if(ocean_cnt==0)		
				{
				log_hal_msgid_info("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_CALLBACK_OCEAN_VP receiver partner message,play vp. ocean_cnt=%d",1,ocean_cnt);
					ocean_cnt=1;	
					customer_key_triger_ocean_vp();	
				}
				else	
				{
				log_hal_msgid_info("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_CALLBACK_OCEAN_VP receiver partner message,stop vpppp ocean_cnt=%d",1,ocean_cnt);
					ocean_cnt=0;	
		       			ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,EVENT_ID_EASTECH_CALLBACK_OCEAN_VP);
			 		voice_prompt_stop(VP_INDEX_Ocean,VOICE_PROMPT_ID_INVALID,false);	
				}
				apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_CALLBACK_OCEAN_VP,&ocean_cnt,1);
				log_hal_msgid_info("_customer_common_app_aws_data_proc sync ocean_cnt to partner.  ocean_cnt=%d",1,ocean_cnt);
			}
				break;

			    case EVENT_ID_EASTECH_FROM_CASE_PRESS_MULTI:	{
				if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
				&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
				{
					from_case_haanckey=*(uint16_t*)p_extra_data;
					log_hal_msgid_info("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_FROM_CASE_PRESS_MULTI receiver agent message; sync from_case_haanckey=%d",1,from_case_haanckey);
				}
				else
				{
					from_case_haanckey=*(uint16_t*)p_extra_data;
					log_hal_msgid_info("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_FROM_CASE_PRESS_MULTI receiver partner message,sync from_case_haanckey=%d",1,from_case_haanckey);

				}

			    }
				break;
				
			    case EVENT_ID_EASTECH_FROM_TESTCMD_IRSENSER:	{
					
					need_disable_irsenser=*(uint8_t*)p_extra_data;
					if(need_disable_irsenser==1)
					{
				           isInEar = (bool *)pvPortMalloc(sizeof(bool));
				          *isInEar =need_disable_irsenser;
					   ui_shell_send_event(true, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
									 APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT, isInEar, sizeof(bool),
									 NULL, 10);
					}
					APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_FROM_TESTCMD_IRSENSER need_disable_irsenser= 0x%x", 1, *(uint8_t*)p_extra_data);
				}
					break;

				
			    case EVENT_ID_EASTECH_VOLUME_VP:	{
					uint8_t tmp_val;
					tmp_val=*(uint8_t*)p_extra_data;
					APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc EVENT_ID_EASTECH_VOLUME_VP from partner vol= 0x%x", 1, *(uint8_t*)p_extra_data);
					if(tmp_val==1)
					{
        					voice_prompt_play_sync_vp_volume_down();  
					}
					else if (tmp_val==2)
					{
        					voice_prompt_play_sync_vp_volume_up();  
					}
					else if(tmp_val==3)
					{
		        			voice_prompt_play_sync_vp_mute();  // mute code actually is default vol vp
					}
					else if(tmp_val==4)
					{
		        			voice_prompt_play_sync_vp_mute();  // mute code actually is default vol vp
					}
					else if(tmp_val==5)
					{
		        			voice_prompt_play_sync_vp_maxvol();  
					}
					else if(tmp_val==6)
					{
		        			voice_prompt_play_sync_vp_minvol();  
					}
				}
					break;
			    case EVENT_ID_EASTECH_OCEAN_ADJUST_VP:	{
					uint8_t tmp_val1;
					tmp_val1=*(uint8_t*)p_extra_data;
					if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
					&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
					{
						APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=partner,receiver agent EVENT_ID_EASTECH_OCEAN_ADJUST_VP event, sync local ocean_vp_vol= 0x%x,tmp_val1=%d", 2, ocean_vp_vol,tmp_val1);
						ocean_vp_vol=tmp_val1;
					}
					else
					{
						if(ocean_vp_vol<15)
						{
							ocean_vp_vol++;	
						}
						APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=agent,receiver partner EVENT_ID_EASTECH_OCEAN_ADJUST_VP event, ocean_vp_vol= 0x%x", 1, ocean_vp_vol);
					APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc KEY_VP_UP role=agent   sync sent EVENT_ID_EASTECH_OCEAN_ADJUST_VP,ocean_vp_vol to partner",0);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_VP,&ocean_vp_vol,1);
					}
				}
					break;

 			    case EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP:	{
					uint8_t tmp_val1;
					if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
					&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
					{
					tmp_val1=*(uint8_t*)p_extra_data;
						APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=partner,receiver agent EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP event, local ocean_vp_vol= 0x%x,tmp_val1=%d", 2, ocean_vp_vol,tmp_val1);
						ocean_vp_vol=tmp_val1;
					}
					else
					{
					if(ocean_vp_vol>1)
					{
					ocean_vp_vol--;	
					}
					APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=agent,receiver partner EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP event, ocean_vp_vol= 0x%x", 1, ocean_vp_vol);
					APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc KEY_VP_DM role=agent   sync sent EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP,ocean_vp_vol to partner",0);
					apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP,&ocean_vp_vol,1);
					}
				}
					break;

 			    case EVENT_ID_EASTECH_OCEAN_IS_PLAY_NOW:	{
					uint8_t tmp_val1;
					if(bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
					&& bt_sink_srv_cm_get_aws_connected_device() != NULL)
					{
						tmp_val1=*(uint8_t*)p_extra_data;
						APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=partner,receiver agent EVENT_ID_EASTECH_OCEAN_IS_PLAY_NOW event,  is_play_ocean= %D,tmp_val1=%d", 2, is_play_ocean,tmp_val1);
						is_play_ocean=tmp_val1;
					}
					else
					{
					//APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc role=agent,receiver partner EVENT_ID_EASTECH_OCEAN_IS_PLAY_NOW event, ocean_vp_vol= 0x%x", 1, is_play_ocean);
					//APPS_LOG_MSGID_I("_customer_common_app_aws_data_proc KEY_VP_DM role=agent   sync sent EVENT_ID_EASTECH_OCEAN_IS_PLAY_NOW,ocean_vp_vol to partner",0);
					//apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_EASTECH_OCEAN_ADJUST_DOWN_VP,&ocean_vp_vol,1);
					}
				}
					break;


					
               default:
                    break;
            }
        }
    }

    return ret;
}




bool apps_customer_common_activity_proc(
      struct _ui_shell_activity *self,
      uint32_t event_group,
      uint32_t event_id,
      void *extra_data,
      size_t data_len)
{
	bool ret = false;

	//APPS_LOG_MSGID_I("apps_customer_common_activity_proc event_group: 0x%x, event_id: 0x%x", 2, event_group, event_id);

	switch (event_group)
	{
		case EVENT_GROUP_UI_SHELL_SYSTEM:
		{
			ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
			break;
		}

		/* UI Shell key events. */
        case EVENT_GROUP_UI_SHELL_KEY:
		{
            ret = _proc_key_event_group(self, event_id, extra_data, data_len);
            break;
        }

		case EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON:
		{
			ret = _proc_customer_common(self, event_id, extra_data, data_len);
			break;
		}

		case EVENT_GROUP_UI_SHELL_AWS_DATA:
		{
			/* Handle AWS data sync */
			ret = _customer_common_app_aws_data_proc(self, event_id, extra_data, data_len);
		}
		case EVENT_GROUP_UI_SHELL_BT_SINK:
		{
			ret = _bt_sink_event_group_proc(self, event_id, extra_data, data_len);
			break;	
		}

		default:
			break;
	}
	return ret;

}

#ifdef ALWAYS_PLAY_PAIRING_VP
void app_eastech_pair_vp_callback(void )
{
	apps_config_state_t sta = apps_config_key_get_mmi_state();
	APPS_LOG_MSGID_I("app_eastech_pair_vp_callback is_bt_visiable=%x,aws_connected=%d,connection_state=%x", 3,app_bt_connection_service_get_current_status()->bt_visible,app_bt_connection_service_get_current_status()->aws_connected,app_bt_connection_service_get_current_status()->connection_state);
	APPS_LOG_MSGID_I("app_eastech_pair_vp_callback pmu_get_power_on_reason=%d,pmu_get_power_off_reason=%d", 2,pmu_get_power_on_reason() ,pmu_get_power_off_reason());
	APPS_LOG_MSGID_I("app_eastech_pair_vp_callback mmi_state=%d,bt_sink_srv_get_music_state=%d,vpindex=%d", 3,sta ,bt_sink_srv_get_music_state(),voice_prompt_get_current_index());
    if (app_bt_connection_service_get_current_status()->bt_visible)
	{
		//if (VP_INDEX_PAIRING != voice_prompt_get_current_index())
		{
			voice_prompt_param_t vp;
			memset((void *)&vp, 0, sizeof(voice_prompt_param_t));
			vp.vp_index = VP_INDEX_PAIRING_LOOP;
			//vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC | VOICE_PROMPT_CONTROL_MASK_LOOP | VOICE_PROMPT_CONTROL_MASK_PREEMPT;
			vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
			vp.delay_time = 200;
			voice_prompt_play(&vp, NULL);
		}
      APPS_LOG_MSGID_I("app_eastech_pair_vp_callback VP_INDEX_PAIRING 444", 0);
      ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
            EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
            (uint32_t)EVENT_ID_EASTECH_CALLBACK_PAIR_VP,
            NULL, 0,
            NULL, VP_PLAY_INTERVAL);	
	}else
	{
		/*device connected or connection time out*/
		APPS_LOG_MSGID_I("app_eastech_pair_vp_callback: STOP PAIRING Voice Prompt", 0);
 		voice_prompt_stop(VP_INDEX_PAIRING_LOOP,VOICE_PROMPT_ID_INVALID,false);
	}
}	
#endif
#define VP_PLAY_INTERVAL_OCEAN	17000
void customer_key_triger_ocean_vp(void)
{
	APPS_LOG_MSGID_I("customer_key_triger_ocean_vp: start Voice Prompt", 0);
	voice_prompt_param_t vp;
	memset((void *)&vp, 0, sizeof(voice_prompt_param_t));
	vp.vp_index = VP_INDEX_Ocean;
	vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC | VOICE_PROMPT_CONTROL_MASK_PREEMPT;
	vp.delay_time = 200;
	voice_prompt_play(&vp, NULL);
	ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
	EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON,
	(uint32_t)EVENT_ID_EASTECH_CALLBACK_OCEAN_VP,
	NULL, 0,
	NULL, VP_PLAY_INTERVAL_OCEAN);	
}


