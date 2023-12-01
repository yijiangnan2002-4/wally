
/**
  * @file    app_hall_sensor_activity.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/4/7
  * 
  * @brief   data process for hall sensor
**/


#ifndef __APP_HALL_SENSOR_ACTIVITY_H__
#define __APP_HALL_SENSOR_ACTIVITY_H__

#include "ui_shell_activity.h"

#define APPS_IDLE_MODE_NONE                          (0)
#define APPS_IDLE_MODE_DISABLE_BT                    (1)
#define APPS_IDLE_MODE_SYSTEM_OFF                    (2)

/* The basic power idle mode */
#define APPS_IDLE_MODE     APPS_IDLE_MODE_DISABLE_BT 


#define TIMEOUT_TO_IDLE_BY_HALL_SENSOR (10*1000)

enum
{
    EVENT_ID_HALL_SENSOR_NEW_STATUS_IND = 0,  /* Getting new HALL sensor value from sensor callback */
    EVENT_ID_HALL_SENSOR_AWS_CONTEXT_SYNC,
    EVENT_ID_HALL_SENSOR_PROCESS_INPUT,        /* Internal use, sync HALL status event between L/R*/
    EVENT_ID_HALL_SENSOR_IN_CASE,
    EVENT_ID_HALL_SENSOR_OUT_CASE,
    EVENT_ID_HALL_SENSOR_RESUME_ANC,
    EVENT_ID_HALL_SENSOR_PAUSE_ANC,
    EVENT_ID_HALL_SENSOR_KEY_LIMIT,
};


enum
{
	HALL_STATUS_OUT_CASE     = 0,
	HALL_STATUS_IN_CASE,
};




typedef struct {
	
	uint32_t hall_sensor_status;
	
} apps_hall_sensor_local_context_t;



bool apps_hall_sensor_activity_proc(
      struct _ui_shell_activity *self,
      uint32_t event_group,
      uint32_t event_id,
      void *extra_data,
      size_t data_len);

bool get_hall_sensor_status(void);


#endif //__APP_HALL_SENSOR_ACTIVITY_H__




