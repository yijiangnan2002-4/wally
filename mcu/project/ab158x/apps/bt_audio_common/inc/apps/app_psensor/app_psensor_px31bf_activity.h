/**
  * @file    app_psensor_px31bf_activity.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/3/24
  * 
  * @brief   data process for proximity sensor px31bf
**/

#ifndef __APP_PSENSOR_PX31BF_ACTIVITY_H__
#define __APP_PSENSOR_PX31BF_ACTIVITY_H__

#include "ui_shell_activity.h"
#include "bt_avrcp.h"
#include "bt_sink_srv_ami.h"

#define TIMEOUT_CLEAN_AUDIO_RESUME_AFTER_OUTEAR	(10*60*1000)

typedef enum
{
	PSENSOR_PX31BF_USING = 0,
	PSENSOR_HX300X_USING,
};

enum
{
    EVENT_ID_PSENSOR_NEW_SENSOR_IND = 0,  /* Getting new proximity sensor value from sensor callback */
    EVENT_ID_PSENSOR_AWS_CONTEXT_SYNC,
    EVENT_ID_PSENSOR_PROCESS_INPUT,        /* Internal use, sync inside/outside ear event between L/R
                                                       earbud(@app_proximity_sensor_process_input_t) */
    EVENT_ID_PSENSOR_DO_PLAY,              /* Internal use, sync inside/outside ear event between L/R
                                                       earbud(@app_proximity_sensor_process_input_t) */
    EVENT_ID_PSENSOR_DO_PAUSE,             /* Internal use, sync inside/outside ear event between L/R
                                                       earbud(@app_proximity_sensor_process_input_t) */
    EVENT_ID_PSENSOR_AGENT_SYNC_CONTEXT_TO_PARTNER ,      /* Internal use, agent send data to partner its inear status */
    
    EVENT_ID_PSENSOR_POWER_SAVING_TIME_RESTART,
    EVENT_ID_PSENSOR_POWER_SAVING_TIME_STOP,
    
	EVENT_ID_PSENSOR_ACTIVITY_PAUSE_ANC,								/* Interrnal use, Pause anc fuction(anc/attentive/ambient) */
	EVENT_ID_PSENSOR_ACTIVITY_RESUME_ANC,								/* Interrnal use, Resume anc fuction(anc/attentive/ambient) */
	EVENT_ID_PSENSOR_AWS_DATA,   
	EVENT_ID_PSENSOR_CLEAN_AUDIO_RESUME,
	EVENT_ID_PSENSOR_SWITCH_AUDIO,
	EVENT_ID_PSENSOR_LIMIT_CLEAN,
};

typedef enum
{
    P_SENSOR_STATUS_TO_OUTRANGE = 0,
	P_SENSOR_STATUS_TO_INRANGE,
	P_SENSOR_STATUS_INRANGE,
	P_SENSOR_STATUS_OUTRANGE,
    
} app_psensor_process_status_t;

typedef enum
{
    APP_PROXIMITY_SENSOR_EVENT_NONE = 0,
    APP_PROXIMITY_SENSOR_EVENT_OUTEAR,
    APP_PROXIMITY_SENSOR_EVENT_INEAR,
} app_psensor_event_t;

typedef struct{
    audio_channel_t lr_source;
    app_psensor_event_t event;
    uint8_t previous;      /**<  The previous state of earbuds. */
    uint8_t current;       /**<  The current state of earbuds. */
	uint8_t trigger_channel;
	uint8_t inout_ear;	
	uint8_t cus_needResumePlay;
} app_psensor_process_input_t;



typedef struct {
    bool isLeftInEar;
    bool isRightInEar;
    bool isInAutoResume;
    bt_avrcp_status_t avrcpStatus;
	bool cur_BothStatus;
	bool pre_BothStatus;
} apps_psensor_local_context_t;



bool apps_psensor_activity_proc(
      struct _ui_shell_activity *self,
      uint32_t event_group,
      uint32_t event_id,
      void *extra_data,
      size_t data_len);

void apps_psensor_set_avrcp_status(bt_avrcp_status_t newStatus);
//bool getLocalInEar(void);
//bool power_saving_get_inear_status(void);

#if 1	//def BLE_ZOUND_ENABLE
bool getLeftInEar(void);
bool getRightInEar(void);
//apps_psensor_local_context_t get_apps_psensor_local_context();
#endif

#endif //__APP_PSENSOR_PX31BF_ACTIVITY_H__


