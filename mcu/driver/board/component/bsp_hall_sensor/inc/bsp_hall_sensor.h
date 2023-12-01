
/**
  * @file    bsp_hall_sensor.h
  *
  * @author  wayne.xiong
  *
  * @date    2022/4/7
  * 
  * @brief   Driver of hall sensor
**/

#ifndef _BSP_HALL_SENSOR_H_
#define _BSP_HALL_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"

#define PIO_HALL_SENSOR HAL_GPIO_26
#define EINT_HALL_SENSOR HAL_EINT_NUMBER_26


void bsp_component_HALL_init(void);
bool bsp_Hall_status_checking(void);



#endif //_BSP_HALL_SENSOR_H_


