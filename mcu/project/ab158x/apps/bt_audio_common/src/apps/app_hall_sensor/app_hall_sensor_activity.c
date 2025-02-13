
/**
  * @file    app_hall_sensor_activity.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/4/7
  * 
  * @brief   data process for hall sensor
**/

#include "app_hall_sensor_activity.h"
#include "bsp_hall_sensor.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_debug.h"
#include "app_fota_idle_activity.h"
#include "race_event.h"
#include "battery_management.h"
#include "apps_config_event_list.h"
#include "apps_events_key_event.h"
#include "apps_events_interaction_event.h"
#include "ui_shell_manager.h"
#include "apps_config_state_list.h"
#include "apps_config_key_remapper.h"
#include "app_smcharger_utils.h"
#include "app_psensor_px31bf_activity.h"


static apps_hall_sensor_local_context_t s_app_hall_sensor_context;

static app_sensor_fota_state_t appHallFotaState = FOTA_STATE_IDLE;

static uint8_t test_flag = 0;
static uint8_t case_nobattery=0;
#if 0
static bool key_limit = false;

void app_key_limit_set(uint8_t limit)
{
	key_limit = limit;
}

uint8_t app_key_limit_read(void)
{
	return key_limit;
}
#endif

bool get_hall_sensor_status(void)
{
	return bsp_Hall_status_checking();
}

uint8_t get_test_flag(void)
{
	return test_flag;
}

void set_test_flag(uint8_t bit)
{
	APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc set_test_flag = 0x%x", 1, bit);
	test_flag = (1<<bit);
}

void set_test_reset(uint8_t data)
{
	test_flag = data;
}



static bool _proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = true; // UI shell internal event must process by this activity, so default is true
    switch (event_id)
    {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc create current activity", 0);
            self->local_context = &s_app_hall_sensor_context;

            break;
        }
        
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc destroy", 0);
            break;
        }
        
        case EVENT_ID_SHELL_SYSTEM_ON_RESUME:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc resume", 0);
            break;
        }
        
        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc pause", 0);
            break;
        }
        
        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc refresh", 0);
            break;
        }
        
        case EVENT_ID_SHELL_SYSTEM_ON_RESULT:
        {
            APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc result", 0);
            break;
        }
        
        default:
            break;
    }
    return ret;
}
uint8_t case_handle_command=0;
extern void app_hear_through_handle_charger_in();
extern void app_hear_through_handle_charger_out();
static bool _proc_hall_sensor(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = false;
	int32_t charger_exist = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
	
    switch (event_id)
    {
    	case EVENT_ID_HALL_SENSOR_NEW_STATUS_IND:
    		{
				uint16_t hall_sensor_status = *(uint16_t *)extra_data;
				
				/* Handle sensor new status from sensor interrupt callback */
				APPS_LOG_MSGID_I("_proc_hall_sensor.c:: hall_sensor_status= %d,case_nobattery=%d", 2, hall_sensor_status,case_nobattery);

				app_common_add_tracking_log(0x12);

				if(hall_sensor_status == HALL_STATUS_IN_CASE)
				{
#if 0				
					if (appHallFotaState == FOTA_STATE_IDLE && !charger_exist)
					{
						app_common_add_tracking_log(0x13);
						ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_OUT_CASE);
						ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_IN_CASE);
				        	ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
				                            EVENT_ID_HALL_SENSOR_IN_CASE, NULL, 0,
				                            NULL, TIMEOUT_TO_IDLE_BY_HALL_SENSOR);  	
					}
#endif					
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_RESUME_ANC);
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_PAUSE_ANC);
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
										EVENT_ID_HALL_SENSOR_PAUSE_ANC, NULL, 0, NULL, 1200);

//					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_KEY_LIMIT);
//					app_key_limit_set(true);
				}
				else
				{
					case_handle_command=0;  // reset this flag harry 
					if (case_nobattery)
					{
		        		  ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT);
		                    	  ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT, (void *)true, 0,
                                        NULL, 0);

					}
					case_nobattery=0;
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_RESUME_ANC);
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_PAUSE_ANC);
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
										EVENT_ID_HALL_SENSOR_RESUME_ANC, NULL, 0, NULL, 500);
#if 0
			        	ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_IN_CASE);
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_OUT_CASE);
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
										EVENT_ID_HALL_SENSOR_OUT_CASE, NULL, 0,
										NULL, 500);		
					app_key_limit_set(true);
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_KEY_LIMIT);
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
										EVENT_ID_HALL_SENSOR_KEY_LIMIT, NULL, 0,
										NULL, 1500);
#endif
				}
				APPS_LOG_MSGID_I("_proc_hall_sensor.c:: hall_sensor_status= %d,case_nobattery=%d,case_handle_command=%d", 3, hall_sensor_status,case_nobattery,case_handle_command);
				
				ret = true;
				break;
    		}

		case EVENT_ID_HALL_SENSOR_IN_CASE:
			{
				ret = true;		// richard for UI requirement
				break;

				if(get_hall_sensor_status() && !charger_exist )
				{
					app_common_add_tracking_log(0x14);
#ifdef AIR_SMART_CHARGER_ENABLE
					if(app_get_smcharger_context() != NULL)
					{
						if(STATE_SMCHARGER_LID_OPEN == app_get_smcharger_context()->peer_smcharger_state)
						{	
							ret = true;
							break;
						}
					}
#endif
				
#if (APPS_IDLE_MODE == APPS_IDLE_MODE_SYSTEM_OFF)

					ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, (void *)false, 0,
										NULL, 0);

#elif (APPS_IDLE_MODE == APPS_IDLE_MODE_DISABLE_BT)
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
										EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP, NULL, 0, NULL, 5000);
					app_common_add_tracking_log(0x15);
					if(apps_config_key_get_mmi_state() != APP_BT_OFF)
					{
						app_common_tracking_log_reset();	
						app_common_add_tracking_log(0x16);
						ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT, (void *)false, 0,
											NULL, 0);
					}
										
#endif
				}				
				ret = true;
				break;
			}

		case EVENT_ID_HALL_SENSOR_OUT_CASE:
			{
				ret = true;	// richard for UI spec.
				break;

		            if(apps_config_key_get_mmi_state() == APP_BT_OFF)
		            {					
					if( !get_hall_sensor_status() && !charger_exist)
					{
						uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */
		            
		                /* Bt off state: power on action */
	                    *p_key_action = KEY_POWER_ON;
						
						ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
											sizeof(uint16_t), NULL, 0);											
					}
					
		            }				
				ret = true;
				break;
			}	

        case EVENT_ID_HALL_SENSOR_RESUME_ANC:
			{
//				int32_t charger_exist = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
			
	//			if(!charger_exist)
				{
					APPS_LOG_MSGID_I("Hall sensor out case, ANC resume! \n", 0);

//					app_anc_service_resume();
//					app_psensor_limit_set(false);
					app_common_add_tracking_log(0x11);
				#ifdef AIR_SMART_CHARGER_ENABLE
					app_mute_audio(FALSE);
				#endif
					app_hear_through_handle_charger_out();
				}
				case_nobattery=0;
				
				ret = true;
           	 	break;
        	}

        case EVENT_ID_HALL_SENSOR_PAUSE_ANC:
			{
				
				APPS_LOG_MSGID_I("Hall sensor in case, ANC pause!charger_exist=%d,case_handle_command=%d \n", 2,charger_exist,case_handle_command);
			#ifdef AIR_SMART_CHARGER_ENABLE
				app_mute_audio(TRUE);
			#endif
				app_hear_through_handle_charger_in();
//				app_anc_service_suspend();
//				app_psensor_limit_set(true);
				app_common_add_tracking_log(0x10);


				if(!case_handle_command)  // in case but not in charge mode
				{
				  case_nobattery=1;
                       	  ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT);
                           	  ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
                                EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                (void *)false, 0,
                                NULL, 1000);
				}
				ret = true;
	            break;
        	}
#if 0
        case EVENT_ID_HALL_SENSOR_KEY_LIMIT:
			{
				APPS_LOG_MSGID_I("limit release! \n", 0);
				app_key_limit_set(false);
				ret = true;
	            break;
        	}		
#endif
		default:
			break;
    }

	return ret;

}


static bool _proc_fota_state_change_event(struct _ui_shell_activity *self,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;

    app_sensor_fota_state_t lastFotaState = appHallFotaState;

    switch (event_id)
    {
        case RACE_EVENT_TYPE_FOTA_START: {
            APPS_LOG_MSGID_I("app_hall_sensor_activity.c::_proc_fota_state_change_event RACE_EVENT_TYPE_FOTA_START", 0);
            appHallFotaState = FOTA_STATE_RUNNING;
            break;
        }
        case RACE_EVENT_TYPE_FOTA_CANCELLING:
		case RACE_EVENT_TYPE_FOTA_CANCEL:
		case RACE_EVENT_TYPE_FOTA_TRANSFER_COMPLETE:
		{
            APPS_LOG_MSGID_I("app_hall_sensor_activity.c::_proc_fota_state_change_event RACE_EVENT_TYPE_FOTA_CANCELLING", 0);
            appHallFotaState = FOTA_STATE_IDLE;
            break;
        }
        case RACE_EVENT_TYPE_FOTA_NEED_REBOOT:
            break;

#if 0	// richard for UI spec.
	 	/* On fota state change */
	    if (lastFotaState == FOTA_STATE_IDLE && appHallFotaState == FOTA_STATE_RUNNING)
	    {
	        /* Fota state: FOTA_STATE_IDLE->FOTA_STATE_RUNNING */
	        APPS_LOG_MSGID_I("app_hall_sensor_activity.c::_proc_fota_state_change_event FOTA_STATE_IDLE->FOTA_STATE_RUNNING", 0);
			ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_IN_CASE);
	    }
	    else if (lastFotaState == FOTA_STATE_RUNNING && appHallFotaState == FOTA_STATE_IDLE)
	    {
	        /* Fota state: FOTA_STATE_RUNNING->FOTA_STATE_IDLE */
	        APPS_LOG_MSGID_I("app_hall_sensor_activity.c::_proc_fota_state_change_event FOTA_STATE_RUNNING->FOTA_STATE_IDLE", 0);

			int32_t charger_exitst = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
			
			if(get_hall_sensor_status() && !charger_exitst)
		        {
	        	ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HALL_SENSOR, EVENT_ID_HALL_SENSOR_OUT_CASE);
				ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
									EVENT_ID_HALL_SENSOR_IN_CASE, NULL, 0,
									NULL, TIMEOUT_TO_IDLE_BY_HALL_SENSOR);						
		        }
	    }	
#endif		
    }

    return ret;
}



bool apps_hall_sensor_activity_proc(
      struct _ui_shell_activity *self,
      uint32_t event_group,
      uint32_t event_id,
      void *extra_data,
      size_t data_len)
{
	bool ret = false;
	
	//APPS_LOG_MSGID_I("apps_hall_sensor_activity_proc event_group: 0x%x, event_id: 0x%x", 2, event_group, event_id);
	
	switch (event_group)
	{
		case EVENT_GROUP_UI_SHELL_SYSTEM:
		{
			ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
			break;
		}
		case EVENT_GROUP_UI_SHELL_HALL_SENSOR:
		{
			ret = _proc_hall_sensor(self, event_id, extra_data, data_len);
			break;
		}
		case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
		{
			//ret = _proc_apps_internal_events(self, event_id, extra_data, data_len);
			break;
		}
		case EVENT_GROUP_UI_SHELL_FOTA:
		{
			/* Handle fota state to limit automatic power off when out of ear */
			ret = _proc_fota_state_change_event(self, event_id, extra_data, data_len);
			break;
		}

		case EVENT_GROUP_UI_SHELL_AWS_DATA:
		{
			/* Handle another side in/out case status */
		}

		case EVENT_GROUP_UI_SHELL_BT_SINK:
			//ret = _bt_sink_event_group_proc(self, event_id, extra_data, data_len);
			break;

		default:
			break;
	}
	return ret;
	
}



