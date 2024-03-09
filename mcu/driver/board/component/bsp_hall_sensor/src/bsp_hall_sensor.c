
/**
  * @file    bsp_hall_sensor.c
  *
  * @author  wayne.xiong
  *
  * @date    2022/4/7
  * 
  * @brief   Driver of hall sensor
**/

#include "bsp_hall_sensor.h"
#include "apps_debug.h"
#include "app_hall_sensor_activity.h"
#include "ui_shell_activity.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"


bool bsp_Hall_status_checking(void)
{
	hal_gpio_data_t pin_status = 0;
	hal_gpio_status_t status = hal_gpio_get_input(PIO_HALL_SENSOR, &pin_status);

	if ( status == HAL_GPIO_STATUS_OK)
	{
		if(pin_status & 0x0001)
			return 0; // out case
		else
			return 1; // in case
	}

	return 0;
}


static void bsp_HALL_callback(void *data)
{
    uint16_t *pHallStatus = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); /* free by ui shell */

	hal_gpio_data_t pin_status = 0;

	hal_gpio_status_t status = hal_gpio_get_input(PIO_HALL_SENSOR, &pin_status);

	APPS_LOG_MSGID_I("bsp_HALL_callback HALL: read status = %d, hall status = %d\n",2, status, pin_status);

	if ( status == HAL_GPIO_STATUS_OK)
	{
		if(pin_status & 0x0001)
      {  
			*pHallStatus = 0; // out case
      }
		else
      {  
			*pHallStatus = 1; // in case
      }

	    /* Send new value indication to psensor activity */
	    ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_HALL_SENSOR,
	                        EVENT_ID_HALL_SENSOR_NEW_STATUS_IND, (void *)pHallStatus, sizeof(uint16_t),
	                        NULL, 0);
	}

    /* Clear interrupt flag to receive new interrupt */
    hal_eint_unmask(EINT_HALL_SENSOR);
}


void bsp_component_HALL_init(void)
{
	hal_eint_config_t eint_config;
	hal_eint_status_t ret; 

	ret = hal_eint_mask(EINT_HALL_SENSOR);
	APPS_LOG_MSGID_I("HALL:hal_eint_mask ret = %d", 1, ret);

	eint_config.debounce_time = 200;
	eint_config.trigger_mode  = HAL_EINT_EDGE_FALLING_AND_RISING;

	ret = hal_eint_init(EINT_HALL_SENSOR, &eint_config);
	APPS_LOG_MSGID_I("HALL:hal_eint_init ret = %d", 1, ret);

	ret = hal_eint_register_callback((hal_eint_number_t)EINT_HALL_SENSOR, (hal_eint_callback_t)bsp_HALL_callback, NULL);
	APPS_LOG_MSGID_I("HALL:hal_eint_register_callback ret = %d", 1, ret);

	hal_eint_unmask(EINT_HALL_SENSOR);
}

