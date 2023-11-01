/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#ifndef _BSP_IMU_SENSOR_H_
#define _BSP_IMU_SENSOR_H_
#include "hal.h"
#include "hal_platform.h"
#include <stdint.h>
#include <stdio.h>

/**
 * @addtogroup BSP
 * @{
 * @addtogroup Multi_Axis_Sensor
 * @{
 * This section describes the programming interfaces of the external Multi Axis Sensor driver.
 */

/** @brief This defines the callback function prototype. */
typedef void (*bsp_imu_sensor_callback_t)(uint32_t handle, uint32_t frame_cnt, void *user_data);


/** @brief The multi axis sensor handle. */
typedef  uint32_t bsp_imu_sensor_handle_t;

/** @defgroup multi_axis_sensor_enum Enums
  * @{
  */
/** @brief This enum defines the return type of axis sensor API. */
typedef enum {
    BSP_IMU_SENSOR_NOT_SUPPORT              = -11,   /**<multi axis sensor parameter is invalid*/
    BSP_IMU_SENSOR_INVALID_PARA             = -10,   /**<multi axis sensor parameter is invalid*/
    BSP_IMU_SENSOR_INVALID_HANDLE           = -9,    /**<multi axis sensor handle is invalid*/
    BSP_IMU_SENSOR_GET_CHIP_ID_ERROR        = -8,    /**<multi axis sensor get chip ID error*/
    BSP_IMU_SENSOR_GYRO_SELF_TEST_ERROR     = -7,    /**<multi axis sensor gyro selftest error*/
    BSP_IMU_SENSOR_ACCEL_SELF_TEST_ERROR    = -6,    /**<multi axis sensor accel selftest error*/
    BSP_IMU_SENSOR_OUT_OF_RANGE_ERROR       = -5,    /**<multi axis sensor gyro selftest error*/
    BSP_IMU_SENSOR_BUSY_ERROR               = -4,    /**<multi axis sensor is busy error*/
    BSP_IMU_SENSOR_TIME_OUT_ERROR           = -3,    /**<multi axis sensor time out error*/
    BSP_IMU_SENSOR_NULL_POINT_ERROR         = -2,    /**<multi axis sensor data struct NULL pointer error*/
    BSP_IMU_SENSOR_RETURN_INIT_VALUE        = -1,    /**<multi axis sensor return init value*/
    BSP_IMU_SENSOR_OK                       = 0,     /**<multi axis sensor operatiron OK*/
} bsp_imu_sensor_status_t;

/** @brief This enum defines the axis sensor output data rate. */
typedef enum {
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_25HZ = 0x06, /**<multi axis band width config output data rate with 25hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_50HZ,        /**<multi axis band width config output data rate with 50hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_100HZ,       /**<multi axis band width config output data rate with 100hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_200HZ,       /**<multi axis band width config output data rate with 200hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_400HZ,       /**<multi axis band width config output data rate with 400hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_800HZ,       /**<multi axis band width config output data rate with 800hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_1600HZ,      /**<multi axis band width config output data rate with 1600hz*/
    BSP_IMU_SENSOR_OUTPUT_DATA_RATE_3200HZ,      /**<multi axis band width config output data rate with 3200hz*/
} bsp_imu_sensor_output_data_rate_t;

/** @brief This enum defines the accel axis sensor range. */
typedef enum {
    BSP_IMU_SENSOR_ACCEL_RANGE_2G = 0x03,        /**<multi axis accelerator range config 2G*/
    BSP_IMU_SENSOR_ACCEL_RANGE_4G = 0x05,        /**<multi axis accelerator range config 4G*/
    BSP_IMU_SENSOR_ACCEL_RANGE_8G = 0x08 ,       /**<multi axis accelerator range config 8G*/
    BSP_IMU_SENSOR_ACCEL_RANGE_16G = 0x0C,       /**<multi axis accelerator range config 16G*/
} bsp_imu_sensor_accel_range_t;

/** @brief This enum defines the gyro axis sensor range. */
typedef enum {
    BSP_IMU_SENSOR_GYRO_RANGE_2000_DEG_SEC = 0,    /**<multi axis gyro range config 2000D*/
    BSP_IMU_SENSOR_GYRO_RANGE_1000_DEG_SEC,        /**<multi axis gyro range config 1000D*/
    BSP_IMU_SENSOR_GYRO_RANGE_500_DEG_SEC,         /**<multi axis gyro range config 500D*/
    BSP_IMU_SENSOR_GYRO_RANGE_250_DEG_SEC,         /**<multi axis gyro range config 250D*/
    BSP_IMU_SENSOR_GYRO_RANGE_125_DEG_SEC,         /**<multi axis gyro range config 125D*/
} bsp_imu_sensor_gyro_range_t;

/** @brief This enum defines the accel axis sensor oversample. */
typedef enum {
    BSP_IMU_SENSOR_ACCEL_OSR4_AVG1 = 0x00,         /**<multi axis accel oversample config with OSR4_AVG1*/
    BSP_IMU_SENSOR_ACCEL_OSR2_AVG2,                /**<multi axis accel oversample config with OSR2_AVG2*/
    BSP_IMU_SENSOR_ACCEL_NORMAL_AVG4,              /**<multi axis accel oversample config with NORMAL_AVG4*/
    BSP_IMU_SENSOR_ACCEL_RESRVER,                  /**<maxx multi axis oversample config */
} bsp_imu_sensor_accel_bandwidth_t;

/** @brief This enum defines the gyro axis sensor oversample. */
typedef enum {
    BSP_IMU_SENSOR_GYRO_OSR4_MODE = 0x00,         /**<multi axis accel oversample config with OSR4*/
    BSP_IMU_SENSOR_GYRO_OSR2_MODE,                /**<multi axis accel oversample config with OSR2*/
    BSP_IMU_SENSOR_GYRO_NORMAL_MODE,              /**<multi axis accel oversample config with NORMAL */
    BSP_IMU_SENSOR_GYRO_CIC_MODE,                 /**<multi axis accel oversample config with CIC_MODE*/
} bsp_imu_sensor_gyro_bandwidth_t;

/** @brief This enum defines the axis sensor vendor. */
typedef enum {
    IMU_LSM6DSO,                                    /**<multi axis type->IMU_LSM6DSO **/
    NOT_SUPPORT_AXIS_SENSOR                     /**<not support multi axis type **/
} bsp_imu_sensor_chip_type_t;

/** @brief This enum defines the axis sensor type. */
typedef enum {
    BSP_IMU_SENSOR_ACCEL_ENABLE = 1,         /**< enable accel sensor */
    BSP_IMU_SENSOR_GYRO_ENABLE = 1 << 1,     /**< enable gyro sensor */
    BSP_IMU_SENSOR_MAG_ENABLE = 1 << 2,      /**< enable mag sensor */
    BSP_IMU_SENSOR_DISABLE_ALL = 1 << 3,     /**< disable all sensors */
} bsp_imu_sensor_enable_t;

/** @brief This enum defines the axis sensor bus type. */
typedef enum {
    BSP_IMU_SENSOR_BUS_TYPE_I2C = 0,    /**< use I2C bus to communicate with sensor*/
    BSP_IMU_SENSOR_BUS_TYPE_SPI,        /**< use SPI bus to communicate with sensor*/
    BSP_IMU_SENSOR_BUS_TYPE_MAX         /**< the maximum value of bus type.*/
} bsp_imu_sensor_bus_type_t;

/** @brief This enum defines the command type. */
typedef enum {
    BSP_IMU_SENSOR_CMD_SELFTEST = 0,    /**< cmd for imu selftest*/
    BSP_IMU_SENSOR_CMD_GET_DEVICE_ID,    /**< cmd for get imu device id*/
    BSP_IMU_SENSOR_CMD_GET_FRAME_CNT,    /**< cmd for get imu frame count*/
    BSP_IMU_SENSOR_CMD_ENTRT_SLEEP,     /**< cmd for set imu enter sleep*/
    BSP_IMU_SENSOR_CMD_EXIT_SLEEP,      /**< cmd for set imu exit sleep*/
    BSP_IMU_SENSOR_CMD_MAX              /**< the maximum value of bus type.*/
} bsp_imu_sensor_command_t;





/**
* @}
*/

/** @defgroup multi_axis_sensor_union Union
  * @{
  */
/** @brief This union defines the bus configurations*/
typedef union {
    hal_i2c_config_t             i2c_config;    /**< I2C configurations*/
    hal_spi_master_config_t      spi_config;    /**< SPI configurations*/
} bsp_imu_sensor_bus_init_config_t;

/**
* @}
*/

/** @defgroup multi_axis_sensor_struct Struct
  * @{
  */
/** @brief Structure containing accel compensated data with float output*/
typedef struct  {
    float x;            /**<accel X  compensated data*/
    float y;            /**<accel Y  compensated data*/
    float z;            /**<accel Z  compensated data*/
} bsp_imu_sensor_accel_float_t;

/** @brief Structure containing gyro compensated data with float output */
typedef struct  {
    float x;        /**<gyro X  compensated data*/
    float y;        /**<gyro Y  compensated data*/
    float z;        /**<gyro Z  compensated data*/
} bsp_imu_sensor_gyro_float_t;

/** @brief Structure  mag compensated data with float output*/
typedef struct  {
    float  x;        /**<mag X compensated data*/
    float  y;        /**<mag Y compensated data*/
    float  z;        /**<mag Z compensated data*/
} bsp_imu_sensor_mag_float_t;

/** @brief Structure containing sensor all data*/
typedef struct  {
    bsp_imu_sensor_mag_float_t accel_data; /**<accel compensated data*/
    bsp_imu_sensor_gyro_float_t gyro_data; /**<gyro compensated data*/
    bsp_imu_sensor_mag_float_t mag_data;   /**<mag compensated data*/
} bsp_imu_sensor_float_data_t;

/** @brief Structure containing head data*/
typedef struct  {
    double system_time_ms;   /**<system timestamp*/
    uint32_t sensor_time;   /**<sensor timestamp*/
    float temperature;      /**<temperature*/
} bsp_imu_sensor_data_head_t;

/** @brief Structure containing sensor all data*/
typedef struct  {
    bsp_imu_sensor_data_head_t head;                /**< sensor data header */
    bsp_imu_sensor_float_data_t axis_sensor_data_t; /**<sensor data*/
} bsp_imu_sensor_data_t;


/** @brief This structure contains the basic settings of bus and external interrupt. */
typedef struct {
    bsp_imu_sensor_bus_type_t           bus_type;   /**< bus type*/
    uint8_t                             bus_port;   /**< bus port number*/
    bsp_imu_sensor_bus_init_config_t    bus_config; /**< bus configuration*/
    uint8_t                             slv_addr;
    hal_eint_number_t                   eint_num;   /**< external interrupt number */
    hal_eint_config_t                   eint_config;/**< external interrupt configuration */
    bsp_imu_sensor_callback_t           call_back;  /**< this function is called when axis sensor data is ready */
    void                                *user_data; /**< this is the parameter of the call_back function */
} bsp_imu_sensor_init_config_t;

/** @brief This structure contains the settings of multi axis sensor.*/
typedef struct {
    bsp_imu_sensor_enable_t             enable;         /**< sensor enable bit*/
    bsp_imu_sensor_accel_range_t        accel_range;    /**< the accel data range*/
    bsp_imu_sensor_gyro_range_t         gyro_range;     /**< the gyro data range*/
    bsp_imu_sensor_output_data_rate_t   sensor_odr;     /**< the sensor data output rate*/
    uint32_t                            frame_threshold;/**< the threshold of data frame*/
} bsp_imu_sensor_config_t;


/**
* @}
*/

/**
  *  @brief  This function is used to initialize the axis sensor.
  *  @param vendor : sensor chip name .
  *  @param config : configure information.
  *  @return If the function succeeds, the return value is user handle.
  *    Otherwise, is 0
  */
bsp_imu_sensor_handle_t    bsp_imu_sensor_init(uint32_t vendor, bsp_imu_sensor_init_config_t *config);

/**
  *  @brief  This function is used to de-initialize the axis sensor.
  *  @param handle : user handle that get from bsp_imu_sensor_init() .
  *  @return If the function succeeds, the return value is handle.
  *    Otherwise, is 0
  */
bsp_imu_sensor_status_t bsp_imu_sensor_deinit(bsp_imu_sensor_handle_t handle);

/**
  *  @brief  This function is used to set the accel & gyro sensor range and bandwith.
  *  @param handle : user handle that get from bsp_imu_sensor_init().
  *  @param config : configure information.
  *  @return If the function succeeds, the return value is AXIS_SENSOR_OK.
  * Otherwise, an error code is returned
  */
bsp_imu_sensor_status_t bsp_imu_sensor_config(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_config_t *config);

/**
  *  @brief  This function is used to do sensor selftest
  *  @param[in] handle : user handle that get from bsp_imu_sensor_init().
  *  @param[in] sensor_en : sensor enable bit.
  *  @return the count of frame.
  */
int bsp_imu_sensor_control(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_command_t cmd, uint32_t option);

/**
  *  @brief  This function is used to get sensor data.
  *  @param[in]  handle : user handle that get from bsp_imu_sensor_init().
  *  @param[out] frame_buff : address to store data .
  *  @param[in]  frame_cnt : data cnt you want to get,unit(frame).
  *  @return If the function succeeds, the return value is AXIS_SENSOR_OK.
  *  Otherwise, an error code is returned
  *  @note addr format is shown by following
  *   sensor_chip|sensor_type|temperature|sensor_time|data_cnt|reserve|accel_xyz|gyro_xyz|accel_xyz|...
  */
uint32_t bsp_imu_sensor_get_data(bsp_imu_sensor_handle_t handle, bsp_imu_sensor_data_t *frame_buff, uint32_t frame_cnt);





/**
* @}
*/


/**
* @}
* @}
*/

#endif

