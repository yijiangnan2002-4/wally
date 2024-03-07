/**
  * @file    app_psensor_px31bf_activity.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/24
  *
  * @brief   data process for proximity sensor px31bf
**/

#include "app_psensor_px31bf_activity.h"
#include "bsp_px31bf.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_debug.h"
#include "app_fota_idle_activity.h"
#include "race_event.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"
#include "apps_config_state_list.h"

#include "apps_config_event_list.h"
#include "apps_events_key_event.h"
#include "ui_shell_manager.h"
#include "bt_hfp.h"
#include "bt_sink_srv_hf.h"
#include "bt_aws_mce_report.h"
#include "apps_events_interaction_event.h"
#include "apps_aws_sync_event.h"
#include "app_smcharger_utils.h"
#include "apps_config_key_remapper.h"
#include "app_anc_service.h"
#include "app_power_save_utils.h"
//#include "app_hall_sensor_activity.h"
#include "battery_management.h"
#ifdef BLE_ZOUND_ENABLE
#include "gatt_zound_service_tool.h"
#include "gatt_zound_service_notify.h"
#include "app_customer_common_activity.h"
#endif
#include "app_in_ear_utils.h"

static apps_psensor_local_context_t s_app_psensor_context;

static app_sensor_fota_state_t appPsensorFotaState = FOTA_STATE_IDLE;

static volatile uint8_t  psensor_limit = 0;

static volatile uint8_t ir_status_test = 0;

static void app_set_ir_isr_status(uint8_t status)
{
	ir_status_test = status;
}

uint8_t app_get_ir_isr_status(void)
{
	return ir_status_test;
}

#if 0
void app_psensor_limit_set(uint8_t limit)
{
	psensor_limit = limit;
}

uint8_t app_psensor_limit_read(void)
{
	return (psensor_limit | pmu_get_charger_state_lp());
}

uint8_t app_limit_bud_key_in_case(void)
{
	return (app_psensor_limit_read()|get_hall_sensor_status() | app_key_limit_read());
}
#endif

static void app_psensor_status_checking(void)
{
#ifndef MTK_IN_EAR_FEATURE_ENABLE
	apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;

	if(p_local_context->pre_BothStatus)
	{
		if(!p_local_context->isLeftInEar || !p_local_context->isRightInEar)
		{
			p_local_context->cur_BothStatus = false;
		}
	}
	else
	{
		if(p_local_context->isLeftInEar && p_local_context->isRightInEar)
		{
			p_local_context->cur_BothStatus = true;
		}
	}

	if(p_local_context->cur_BothStatus != p_local_context->pre_BothStatus)
	{
		p_local_context->pre_BothStatus = p_local_context->cur_BothStatus;
	
		uint8_t *BothInEar = (uint8_t *)pvPortMalloc(sizeof(uint8_t));/* free by ui shell */
	
		*BothInEar = p_local_context->cur_BothStatus;

		ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
							APPS_EVENTS_INTERACTION_PSENSOR_STATUS_UPDATE, (void *)BothInEar, sizeof(uint8_t), NULL, 0);
	}
#endif
}

#if 0

/**
  * @brief  For external module set avrcp status
  * @param  newStatus   new status of avrcp
  * @return None
  */
void apps_psensor_set_avrcp_status(bt_avrcp_status_t newStatus)
{
    APPS_LOG_MSGID_I("apps_psensor_set_avrcp_status %d", 1, newStatus);
    s_app_psensor_context.avrcpStatus = newStatus;
}


static void _bt_sink_play(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_DO_PLAY);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_DO_PAUSE);
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
                        EVENT_ID_PSENSOR_DO_PLAY, NULL, 0, NULL, 1000);
}

static void _bt_sink_pause(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_DO_PLAY);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_DO_PAUSE);
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
                        EVENT_ID_PSENSOR_DO_PAUSE, NULL, 0, NULL, 500);
}
#endif

static void setLocalInEar(bool newStatus)
{
	bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if(ami_get_audio_channel() == AUDIO_CHANNEL_L)
    {
        s_app_psensor_context.isLeftInEar = newStatus;
    }
    else
    {
        s_app_psensor_context.isRightInEar = newStatus;
    }

	if(role == BT_AWS_MCE_ROLE_AGENT && bt_sink_srv_cm_get_aws_connected_device() != NULL)
	{
		app_psensor_status_checking();
	}
}

#if 0
bool power_saving_get_inear_status(void)
{
	bool ret = false;
	
	if(bt_sink_srv_cm_get_aws_connected_device() == NULL)
	{
		ret = getLocalInEar();
	}
	else if(getLeftInEar() || getRightInEar())
	{
		ret = true;
	}

	return ret;
		
}

bool getLocalInEar(void)
{
	bool in_ear = false;
	
    if(ami_get_audio_channel() == AUDIO_CHANNEL_L)
    {
        in_ear = s_app_psensor_context.isLeftInEar;
    }
    else
    {
        in_ear = s_app_psensor_context.isRightInEar;
    }

	if(!in_ear)
	{
		uint16_t PsensorThresholdHigh = 0;
		uint16_t PsensorThresholdLow = 0;
		uint8_t temp =0;
		uint16_t ps_data = 0;
		uint16_t level = 0;
//		int32_t charger_exist = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
		
		app_nvkey_psensor_threshold_read(&temp, &PsensorThresholdHigh, &PsensorThresholdLow);
		ps_data = bsp_px31bf_Threshold_Factory_Calibrate();

		if(PsensorThresholdHigh==0)
			level = PsDefaultThresholdHigh;
		else
			level = PsensorThresholdHigh;

		if(ps_data > level
			/*&& (!get_hall_sensor_status() && !charger_exist) && !app_psensor_limit_read()*/)
		{
			uint16_t *pSensorStatus = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

			in_ear = true;
			setLocalInEar(in_ear);
			*pSensorStatus = 0x81;
			app_common_add_tracking_log(0x22);
			
			ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
								EVENT_ID_PSENSOR_NEW_SENSOR_IND, (void *)pSensorStatus, sizeof(uint16_t),
								NULL, 0);
			
		}
	}

	return in_ear;
}
#endif

#if 1	//def BLE_ZOUND_ENABLE
bool getLeftInEar(void) {
	return s_app_psensor_context.isLeftInEar;
}
bool getRightInEar(void) {
	return s_app_psensor_context.isRightInEar;
}
/*apps_psensor_local_context_t get_apps_psensor_local_context(){
	return s_app_psensor_context;
}*/
#endif

static void updateLocalContextToRemote(void)
{
    if(bt_sink_srv_cm_get_aws_connected_device() != NULL)
    {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_AWS_DATA,
                                       (void *)&s_app_psensor_context, sizeof(apps_psensor_local_context_t));
    }
    else
    {
        APPS_LOG_MSGID_I("updateLocalContextToRemote aws not connected, ignore update to remote", 0);
    }
}

static void printLocalContext(void)
{
	apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;

    APPS_LOG_MSGID_I("printLocalContext isLeftInEar: %d, isRightInEar: %d, leftPxsValue: %d, rightPxsValue: %d, isInAutoResume: %d, avrcpStatus: %d", 4,
                     p_local_context->isLeftInEar, p_local_context->isRightInEar, p_local_context->isInAutoResume, p_local_context->avrcpStatus);
}

static bool mmi_state_checking_is_need_play_vp(void)
{
	bool ret = false;
	apps_config_state_t mmi_state = apps_config_key_get_mmi_state();

	if (APP_CONNECTED == mmi_state
		|| APP_A2DP_PLAYING == mmi_state
		|| APP_ULTRA_LOW_LATENCY_PLAYING == mmi_state
		|| APP_WIRED_MUSIC_PLAY == mmi_state
		|| APP_HFP_CALLACTIVE == mmi_state
		|| APP_HFP_CALLACTIVE_WITHOUT_SCO == mmi_state		
		|| APP_HFP_MULTITPART_CALL == mmi_state
		|| APP_STATE_HELD_ACTIVE == mmi_state
		|| APP_STATE_VA == mmi_state	
		)
	{
		ret = true;
	}

	return ret;
}
#if 0
#ifdef BLE_ZOUND_ENABLE
static void app_wear_sensor_call_control_behavior_zound(void* extra_data){
	bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_highlight_device();
	uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
	*p_key_action = KEY_ACTION_INVALID;
	app_psensor_process_input_t input_event = *(app_psensor_process_input_t *)extra_data;
	if(bt_sink_srv_hf_context_p == NULL)
	{
		vPortFree(p_key_action);
		return;
	}
	
    uint8_t wear_sensor_action[3] = { 0 };
    memset(wear_sensor_action, 0, 3);
    app_nvkey_wear_sensor_action_read(wear_sensor_action, 3);
	bt_sink_srv_state_t call_state = bt_sink_srv_hf_get_call_state();

	uint8_t lconf = wear_sensor_action[0] >>=1;
	uint8_t rconf = wear_sensor_action[1] >>=1;
	uint8_t pconf = wear_sensor_action[2] >>=1;

	bool active = BT_SINK_SRV_STATE_ACTIVE == call_state || BT_SINK_SRV_STATE_TWC_INCOMING == call_state || BT_SINK_SRV_STATE_HELD_ACTIVE == call_state;
	bool hold = BT_SINK_SRV_STATE_HELD_REMAINING == call_state || BT_SINK_SRV_STATE_TWC_OUTGOING == call_state;

	if (pconf) {
		APPS_LOG_MSGID_I("call_control_behavior_zound pconf:%d,status:%d,call_state:0x%x",3,pconf,input_event.current,call_state);
		if(input_event.current <= APP_IN_EAR_STA_BOTH_OUT){
			if((active && input_event.current == APP_IN_EAR_STA_BOTH_OUT) || (hold && input_event.current == APP_IN_EAR_STA_BOTH_IN)){
				*p_key_action = KEY_ONHOLD_CALL;
			}
		}
	}else{
		APPS_LOG_MSGID_I("call_control_behavior_zound left conf:%d,right conf:%d,status:%d,call_state:0x%x",4,lconf,rconf,input_event.current,call_state);
		if(input_event.trigger_channel == AUDIO_CHANNEL_L && lconf){
			if((input_event.inout_ear == 1 && hold) || (input_event.inout_ear == 0 && active)){
				*p_key_action = KEY_ONHOLD_CALL;
			}
		}
		if(input_event.trigger_channel == AUDIO_CHANNEL_R && rconf){
			if((input_event.inout_ear == 1 && hold) || (input_event.inout_ear == 0 && active)){
				*p_key_action = KEY_ONHOLD_CALL;
			}
		}
	}
	if((input_event.trigger_channel == AUDIO_CHANNEL_L && lconf) && (input_event.inout_ear == 1 && hold)){
		*p_key_action = KEY_ONHOLD_CALL;
	}
	if((input_event.trigger_channel == AUDIO_CHANNEL_R && rconf) && (input_event.inout_ear == 1 && hold)){
		*p_key_action = KEY_ONHOLD_CALL;
	}
	if(*p_key_action != KEY_ACTION_INVALID){
		ui_shell_remove_event(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_SWITCH_HOLD);
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_SWITCH_HOLD, p_key_action, sizeof(uint8_t), NULL, 200);
	}else{
		vPortFree(p_key_action);
	}
}
#endif
#endif
//UI Set no trigger Transfer call when one earbud out of ear in a call.
static void app_wear_sensor_call_control_behavior(void* extra_data)
{
		bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_highlight_device();
		uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); // free by ui shell
		*p_key_action = KEY_ACTION_INVALID;
		app_psensor_process_input_t input_event = *(app_psensor_process_input_t *)extra_data;
		apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;

		if(bt_sink_srv_hf_context_p == NULL)
		{
			vPortFree(p_key_action);
			return;
		}
		if(input_event.event == APP_PROXIMITY_SENSOR_EVENT_OUTEAR)
		{
			if(bt_sink_srv_cm_get_aws_connected_device() == NULL)
			{	
				/* Aws is not connected */
				if( bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED)//need to get audio status(audio in earbud)
				{
					*p_key_action = KEY_SWITCH_AUDIO_PATH;					
					APPS_LOG_MSGID_I("Left&Right _proc_proximity_sensor.c::state changed: %d", 1, (bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED));
				}
			}
			else
			{ 
				 /*Aws is connected */  //both earbuds out ear & (audio in earbud)
				if((p_local_context->isLeftInEar == false) && (p_local_context->isRightInEar == false) && (bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED))
				{
					*p_key_action = KEY_SWITCH_AUDIO_PATH;
					APPS_LOG_MSGID_I("Left&Right _proc_proximity_sensor.c::state changed: %d", 1, (bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED));

				}
			}
		}
		if(input_event.event == APP_PROXIMITY_SENSOR_EVENT_INEAR && !(bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED)) //(when to inear and audio in device need to switch)
		{					
				*p_key_action = KEY_SWITCH_AUDIO_PATH;
		}		
		//ui_shell_remove_event(EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID);
		ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
						sizeof(uint16_t), NULL, 10);		
}

#if 0
static void app_wear_sensor_playback_behavior(void* extra_data)
{
	app_psensor_process_input_t input_event = *(app_psensor_process_input_t *)extra_data;

	apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;

	/* Bt on state: play/pause action */
	if (bt_sink_srv_cm_get_aws_connected_device() == NULL)
	{/* Aws is not connected */
		if (input_event.event == APP_PROXIMITY_SENSOR_EVENT_OUTEAR)
		{
			if (p_local_context->avrcpStatus == BT_AVRCP_STATUS_PLAY_PLAYING)
			{
				p_local_context->isInAutoResume = true;
				ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
									EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME, NULL, 0, NULL, TIMEOUT_CLEAN_AUDIO_RESUME_AFTER_OUTEAR);

			}

			_bt_sink_pause();

		}
		else if (input_event.event == APP_PROXIMITY_SENSOR_EVENT_INEAR)
		{
			if (p_local_context->isInAutoResume == true)
			{
				//p_local_context->isInAutoResume = false;
				_bt_sink_play();
			}
		}
	}
	else
	{
		/* Aws is connected */
		if (input_event.event == APP_PROXIMITY_SENSOR_EVENT_OUTEAR)
		{
		/*	if (p_local_context->isLeftInEar == false && p_local_context->isRightInEar == false)
			{
				p_local_context->isInAutoResume = false;
				ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME);
			}
			else*/
			{
				if (p_local_context->avrcpStatus == BT_AVRCP_STATUS_PLAY_PLAYING)
				{
					p_local_context->isInAutoResume = true;
				}
			}
			_bt_sink_pause();
		}
		else if (input_event.event == APP_PROXIMITY_SENSOR_EVENT_INEAR)
		{
			if (p_local_context->isInAutoResume == true)
			{
				//p_local_context->isInAutoResume = false;
				_bt_sink_play();
			}
		}
	}

}
#endif

static bool _proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = true; // UI shell internal event must process by this activity, so default is true
    switch (event_id)
    {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc create current activity", 0);
            self->local_context = &s_app_psensor_context;
			apps_psensor_local_context_t *cxt = &s_app_psensor_context;

			cxt->isLeftInEar = false;
			cxt->isRightInEar = false;
			cxt->isInAutoResume = false;
			cxt->avrcpStatus = BT_AVRCP_STATUS_PLAY_STOPPED;
			cxt->cur_BothStatus = false;
			cxt->pre_BothStatus = false;

			bsp_px31bf_config_calibration();
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc destroy", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_RESUME:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc resume", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc pause", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc refresh", 0);
            break;
        }

        case EVENT_ID_SHELL_SYSTEM_ON_RESULT:
        {
            APPS_LOG_MSGID_I("apps_psensor_activity_proc result", 0);
            break;
        }

        default:
            break;
    }
    return ret;
}

static bool _proc_proximity_sensor(ui_shell_activity_t *self, uint32_t event_id, void *extra_data,size_t data_len)
{
    bool ret = false;

	apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;
	int32_t charger_exist = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);

    switch (event_id)
    {
    	case EVENT_ID_PSENSOR_NEW_SENSOR_IND:
    		{
				uint16_t temp = *(uint16_t *)extra_data;
				uint16_t psensor_status = temp & 0x01;

				app_set_ir_isr_status(psensor_status);

				/* Handle sensor new status from sensor interrupt callback */
				APPS_LOG_MSGID_I("_proc_proximity_sensor.c:: psensor_status= %d", 1, psensor_status);

				if(get_hall_sensor_status() || charger_exist /*|| app_psensor_limit_read()*/)
				{
					APPS_LOG_MSGID_I("_proc_proximity_sensor.c::earbuds in case, not need handle IR interrupt!", 0);
					ret = true;
					break;
				}

				if (psensor_status == P_SENSOR_STATUS_TO_INRANGE
					&& mmi_state_checking_is_need_play_vp()
					&& temp == psensor_status)
				{
/*					voice_prompt_param_t vp = {0};
					vp.vp_index = VP_INDEX_IN_EAR;
					voice_prompt_play(&vp, NULL);			*/
					//apps_config_set_vp(VP_INDEX_IN_EAR, false, 0 , VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
				}

	            if (psensor_status == P_SENSOR_STATUS_TO_INRANGE)
	            {
	                /* Debounce: dismiss inside outside ear event interval less than 50ms */
	                //ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_INSIDE_EAR);
	                //ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_OUTSIDE_EAR);
	                ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
	                                    APPS_EVENTS_INTERACTION_INSIDE_EAR, NULL, 0, NULL, 50);
	            }
	            else if (psensor_status == P_SENSOR_STATUS_TO_OUTRANGE)
	            {
	                //ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_INSIDE_EAR);
	                //ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_OUTSIDE_EAR);
	                ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
	                                    APPS_EVENTS_INTERACTION_OUTSIDE_EAR, NULL, 0, NULL, 50);
	            }

	            if (psensor_status == P_SENSOR_STATUS_TO_INRANGE || psensor_status == P_SENSOR_STATUS_INRANGE)
	            {
                	/* In ear event: Stop auto power off timer */
					//ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP);
					//ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_POWER_SAVING_TIME_RESTART);
					if(apps_config_key_get_mmi_state() != APP_BT_OFF)
						ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
										EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP, NULL, 0, NULL, 0);

					/* Save in ear state in activity context */
	                setLocalInEar(true);
	            }
	            else if (psensor_status == P_SENSOR_STATUS_TO_OUTRANGE || psensor_status == P_SENSOR_STATUS_OUTRANGE)
	            {

	                /* Save in ear state in activity context */
	                setLocalInEar(false);
	            
	                /* Out ear event: Re-start auto power off timer */
	                if (appPsensorFotaState == FOTA_STATE_IDLE)
	                {
                        APPS_LOG_MSGID_I("_proc_proximity_sensor.c::out ear event, restart power saving timer", 0);
#if 0	//def APPS_SLEEP_AFTER_NO_CONNECTION        
						if(apps_config_key_get_mmi_state() != APP_BT_OFF && !power_saving_get_inear_status())
							app_power_save_utils_cfg_updated_notify();
#endif
	                }
	                else
	                {
	                    APPS_LOG_MSGID_I("_proc_proximity_sensor.c::out ear event, but fota running, can not start auto power off timer", 0);
	                }
	            }

				/* Update local proximity sensor status to remote */
				updateLocalContextToRemote();

	            if(apps_config_key_get_mmi_state() == APP_BT_OFF)
	            {
#if 0
	                /* Bt off state: power on action */
	                if (psensor_status == P_SENSOR_STATUS_TO_INRANGE && !get_hall_sensor_status() && !charger_exist)
	                {
						uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */
	                
	                    *p_key_action = KEY_POWER_ON;
						APPS_LOG_MSGID_I("_proc_proximity_sensor IR trigger BT ON",0);
						ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY, INVALID_KEY_EVENT_ID, p_key_action,
											sizeof(uint16_t), NULL, 0);
						
	                }
#endif
	            }				
				else
				{
					app_psensor_process_input_t process_input;

	                if (psensor_status == P_SENSOR_STATUS_TO_INRANGE)
	                {
	                    process_input.event = APP_PROXIMITY_SENSOR_EVENT_INEAR;
	                }
	                else if (psensor_status == P_SENSOR_STATUS_TO_OUTRANGE)
	                {
	                    process_input.event = APP_PROXIMITY_SENSOR_EVENT_OUTEAR;
	                }

	                if(psensor_status == P_SENSOR_STATUS_TO_INRANGE || psensor_status == P_SENSOR_STATUS_TO_OUTRANGE)
	                {
	                    process_input.lr_source = ami_get_audio_channel();
	                    bt_aws_mce_role_t localRole = bt_connection_manager_device_local_info_get_aws_role();

	                    if (localRole == BT_AWS_MCE_ROLE_AGENT)
	                    {
	                        app_psensor_process_input_t *p_process_input = (app_psensor_process_input_t *)pvPortMalloc(sizeof(app_psensor_process_input_t)); /* free by ui shell */
	                        memcpy(p_process_input, &process_input, sizeof(app_psensor_process_input_t));
	                        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_PROCESS_INPUT,
	                                            (void *)p_process_input, sizeof(app_psensor_process_input_t), NULL, 0); // Send EVENT_ID_ANC_ACTIVITY_EXECUTOR_CMD locally
	                    }
	                    else if (localRole == BT_AWS_MCE_ROLE_PARTNER)
	                    {
	                        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_PROCESS_INPUT,
	                                                       &process_input, sizeof(app_psensor_process_input_t)); // Send EVENT_ID_ANC_ACTIVITY_EXECUTOR_CMD to remote
	                    }
	                }
				}

				ret = true;
				break;
    		}

		case EVENT_ID_PSENSOR_PROCESS_INPUT:
			{
	            app_psensor_process_input_t input_event = *(app_psensor_process_input_t *)extra_data;

	            APPS_LOG_MSGID_I("_proc_proximity_sensor.c::EVENT_ID_PSENSOR_PROCESS_INPUT,lr_source:%d event:%d ", 2, input_event.lr_source, input_event.event);

	            if(input_event.lr_source == AUDIO_CHANNEL_L)
	            {
	                p_local_context->isLeftInEar = (input_event.event == APP_PROXIMITY_SENSOR_EVENT_INEAR) ? 1 : 0;
	            }
	            else if (input_event.lr_source == AUDIO_CHANNEL_R)
	            {
	                p_local_context->isRightInEar = (input_event.event == APP_PROXIMITY_SENSOR_EVENT_INEAR) ? 1 : 0;
	            }

				//Set no trigger Transfer call when one earbud out of ear in a call.
				
#ifndef MTK_IN_EAR_FEATURE_ENABLE
				apps_config_state_t mmi_state = apps_config_key_get_mmi_state();
				if(mmi_state == APP_HFP_CALLACTIVE ||  mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO 
					|| mmi_state == APP_HFP_MULTITPART_CALL || mmi_state == APP_STATE_HELD_ACTIVE)
				{	
					app_psensor_process_input_t *p_process_input = (app_psensor_process_input_t *)pvPortMalloc(sizeof(app_psensor_process_input_t)); /* free by ui shell */
					memcpy(p_process_input, &input_event, sizeof(app_psensor_process_input_t));
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_SWITCH_AUDIO);
					ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_SWITCH_AUDIO,
	                                          	 (void*)p_process_input, sizeof(app_psensor_process_input_t), NULL, 300);
				}
#endif
				// app_wear_sensor_playback_behavior(extra_data);
			}
			ret = true;
			break;

        case EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME:
			{
                p_local_context->isInAutoResume = false;
				ret = true;
           	 	break;
        	}
#if 0
		case EVENT_ID_PSENSOR_LIMIT_CLEAN:
			{
				app_psensor_limit_set(false);
				ret = true;
			}
			break;
#endif
        case EVENT_ID_PSENSOR_DO_PLAY:
			{
                bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PLAY, NULL);

				if(p_local_context->isInAutoResume)
				{
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME);				
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
										EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME, NULL, 0, NULL, 5*1000);
				}
				ret = true;
           	 	break;
        	}

        case EVENT_ID_PSENSOR_DO_PAUSE:
			{
                bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PAUSE, NULL);

				if(p_local_context->isInAutoResume)
				{
					ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME);				
					ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
										EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME, NULL, 0, NULL, TIMEOUT_CLEAN_AUDIO_RESUME_AFTER_OUTEAR);
				}
				ret = true;
	            break;
        	}

        case EVENT_ID_PSENSOR_ACTIVITY_RESUME_ANC:
			{			
				if(!get_hall_sensor_status() && !charger_exist /*&& !app_psensor_limit_read()*/)
				{
                    APPS_LOG_MSGID_I("psensor in ear, ANC resume! \n", 0);
				
					app_anc_service_resume();
					app_common_add_tracking_log(0x21);
				#ifdef AIR_SMART_CHARGER_ENABLE
//					app_mute_audio(false);
				#endif
				}
				ret = true;
           	 	break;
        	}

        case EVENT_ID_PSENSOR_ACTIVITY_PAUSE_ANC:
			{
				APPS_LOG_MSGID_I("psensor out ear, ANC pause! \n", 0);
				app_anc_service_suspend();
				app_common_add_tracking_log(0x20);
				ret = true;
	            break;
        	}

		case EVENT_ID_PSENSOR_AGENT_SYNC_CONTEXT_TO_PARTNER:
			{
				updateLocalContextToRemote();
				ret = true;
	            break;				
			}
		
        case EVENT_ID_PSENSOR_SWITCH_AUDIO:
			{
#ifdef BLE_ZOUND_ENABLE
				app_wear_sensor_call_control_behavior_zound(extra_data);
#else
            	app_wear_sensor_call_control_behavior(extra_data);
#endif
				ret = true;
            	break;
       		}

		default:
			break;
    }
	return ret;

}

static bool _psensor_app_aws_data_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    bt_aws_mce_role_t localRole = bt_connection_manager_device_local_info_get_aws_role();

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION)
    {
        uint32_t event_group;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

		apps_psensor_local_context_t* p_local_context = &s_app_psensor_context;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &event_id, &p_extra_data, &extra_data_len);

        if(event_group == EVENT_GROUP_UI_SHELL_PSENSOR)
        {
            switch (event_id)
            {
                case EVENT_ID_PSENSOR_AWS_DATA:
				{
                    apps_psensor_local_context_t *remote_context = (apps_psensor_local_context_t *)p_extra_data;
                    APPS_LOG_MSGID_I("psensor_app_aws_data_proc:: remote_context isLeftInEar: %d, isRightInEar: %d \n", 2,
                                     remote_context->isLeftInEar, remote_context->isRightInEar);
                    if (ami_get_audio_channel() == AUDIO_CHANNEL_L)
                    {
                        /* Local is left, save right data from sync message */
                        p_local_context->isRightInEar = remote_context->isRightInEar;
                    }
                    else
                    {
                        /* Local is right, save left data from sync message */
                        p_local_context->isLeftInEar = remote_context->isLeftInEar;
                    }

                    if (localRole != BT_AWS_MCE_ROLE_AGENT)
                    {
                        p_local_context->isInAutoResume = remote_context->isInAutoResume;
                        p_local_context->avrcpStatus = remote_context->avrcpStatus;
                    }
					else
					{
#if 0	//def APPS_SLEEP_AFTER_NO_CONNECTION
						if(power_saving_get_inear_status())
						{
							if(apps_config_key_get_mmi_state() != APP_BT_OFF)
								ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
												EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP, NULL, 0, NULL, 0);
						}
						else
						{
							if(appPsensorFotaState == FOTA_STATE_IDLE && apps_config_key_get_mmi_state() != APP_BT_OFF)
								app_power_save_utils_cfg_updated_notify();
						}
#endif		
					
						app_psensor_status_checking();
					}

                    printLocalContext();
                    ret = true;
                    break;
                }

                default:
                    ret = _proc_proximity_sensor(self, event_id, p_extra_data, extra_data_len);
                    break;
            }
        }
    }

    return ret;
}

static bool _proc_apps_internal_events(
        struct _ui_shell_activity *self,
        uint32_t event_id,
        void *extra_data,
        size_t data_len)
{
    bool ret = false;

    switch (event_id)
	{
		case APPS_EVENTS_INTERACTION_INSIDE_EAR:
		{
            /* Delay 1000ms to do real resume anc action */
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_ACTIVITY_RESUME_ANC);
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_ACTIVITY_PAUSE_ANC);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
                                EVENT_ID_PSENSOR_ACTIVITY_RESUME_ANC, NULL, 0, NULL, 500);
			ret = true;
			break;
		}
		case APPS_EVENTS_INTERACTION_OUTSIDE_EAR:
		{
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_ACTIVITY_RESUME_ANC);
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_ACTIVITY_PAUSE_ANC);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
                                EVENT_ID_PSENSOR_ACTIVITY_PAUSE_ANC, NULL, 0, NULL, 0);

			ret = true;
			break;
		}
#ifdef MTK_IN_EAR_FEATURE_ENABLE
		case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: 
		{
			app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
			apps_config_state_t mmi_state = apps_config_key_get_mmi_state();

			if(mmi_state == APP_HFP_CALLACTIVE || mmi_state == APP_HFP_CALLACTIVE_WITHOUT_SCO 
				|| mmi_state == APP_HFP_MULTITPART_CALL || mmi_state == APP_STATE_HELD_ACTIVE)
			{	
				app_psensor_process_input_t *p_process_input = (app_psensor_process_input_t *)pvPortMalloc(sizeof(app_psensor_process_input_t)); /* free by ui shell */
				p_process_input->event = ((sta_info->inout_ear == 1)? APP_PROXIMITY_SENSOR_EVENT_INEAR : APP_PROXIMITY_SENSOR_EVENT_OUTEAR);
				p_process_input->lr_source = sta_info->trigger_channel;
#ifdef BLE_ZOUND_ENABLE
				p_process_input->previous = sta_info->previous;
				p_process_input->current = sta_info->current;
				p_process_input->trigger_channel = sta_info->trigger_channel;
				p_process_input->inout_ear = sta_info->inout_ear;
				p_process_input->cus_needResumePlay = sta_info->cus_needResumePlay;
#endif
				ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_SWITCH_AUDIO);
				ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_SWITCH_AUDIO,
											 (void*)p_process_input, sizeof(app_psensor_process_input_t), NULL, 300);
			}
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

    app_sensor_fota_state_t lastFotaState = appPsensorFotaState;

    switch (event_id)
    {
        case RACE_EVENT_TYPE_FOTA_START: {
            APPS_LOG_MSGID_I("app_psensor_px31bf_activity.c::_proc_fota_state_change_event RACE_EVENT_TYPE_FOTA_START", 0);
            appPsensorFotaState = FOTA_STATE_RUNNING;
            break;
        }
        case RACE_EVENT_TYPE_FOTA_CANCELLING:
		case RACE_EVENT_TYPE_FOTA_CANCEL:
		case RACE_EVENT_TYPE_FOTA_TRANSFER_COMPLETE:
		{
            APPS_LOG_MSGID_I("app_psensor_px31bf_activity.c::_proc_fota_state_change_event RACE_EVENT_TYPE_FOTA_CANCELLING", 0);
            appPsensorFotaState = FOTA_STATE_IDLE;
            break;
        }
        case RACE_EVENT_TYPE_FOTA_NEED_REBOOT:
            break;
    }

    /* On fota state change */
    if (lastFotaState == FOTA_STATE_IDLE && appPsensorFotaState == FOTA_STATE_RUNNING)
    {
        /* Fota state: FOTA_STATE_IDLE->FOTA_STATE_RUNNING */
        APPS_LOG_MSGID_I("app_psensor_px31bf_activity.c::_proc_fota_state_change_event FOTA_STATE_IDLE->FOTA_STATE_RUNNING", 0);
		//ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP);
		//ui_shell_remove_event(EVENT_GROUP_UI_SHELL_PSENSOR, EVENT_ID_PSENSOR_POWER_SAVING_TIME_RESTART);
		ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
							EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP, NULL, 0, NULL, 0);
    }
    else if (lastFotaState == FOTA_STATE_RUNNING && appPsensorFotaState == FOTA_STATE_IDLE)
    {
        /* Fota state: FOTA_STATE_RUNNING->FOTA_STATE_IDLE */
        APPS_LOG_MSGID_I("app_psensor_px31bf_activity.c::_proc_fota_state_change_event FOTA_STATE_RUNNING->FOTA_STATE_IDLE", 0);
#if 0
	if (!power_saving_get_inear_status())
        {
			APPS_LOG_MSGID_I("app_psensor_px31bf_activity.c:: FOTA completed, restart power saving timer!", 0);
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION        
        	app_power_save_utils_cfg_updated_notify();
#endif
        }
#endif
    }

    return ret;
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
                /* Only agent role will enable this module, so don't need judge the role
                 * in event handle. */
                APPS_LOG_MSGID_I("[APP_P_SENSOR]AWS attached.", 0);
				ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_PSENSOR,
									EVENT_ID_PSENSOR_AGENT_SYNC_CONTEXT_TO_PARTNER, NULL, 0,
									NULL, 0);
            }
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}

bool apps_psensor_activity_proc(
      struct _ui_shell_activity *self,
      uint32_t event_group,
      uint32_t event_id,
      void *extra_data,
      size_t data_len)
{
	bool ret = false;

	//APPS_LOG_MSGID_I("apps_psensor_activity_proc event_group: 0x%x, event_id: 0x%x", 2, event_group, event_id);

	switch (event_group)
	{
		case EVENT_GROUP_UI_SHELL_SYSTEM:
		{
			ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
			break;
		}
		case EVENT_GROUP_UI_SHELL_PSENSOR:
		{
			ret = _proc_proximity_sensor(self, event_id, extra_data, data_len);
			break;
		}
		case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
		{
			ret = _proc_apps_internal_events(self, event_id, extra_data, data_len);
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
			/* Handle another side in/out ear status */
			ret = _psensor_app_aws_data_proc(self, event_id, extra_data, data_len);
		}
		case EVENT_GROUP_UI_SHELL_BT_SINK:
			ret = _bt_sink_event_group_proc(self, event_id, extra_data, data_len);
			break;
		default:
			break;
	}
	return ret;

}



