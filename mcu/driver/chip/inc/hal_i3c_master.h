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

/* Includes ------------------------------------------------------------------*/

#ifndef  __HAL_I3C_MASTER_H__
#define  __HAL_I3C_MASTER_H__
#include "hal.h"
#include "hal_platform.h"

#ifdef HAL_I3C_MASTER_MODULE_ENABLED
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup HAL
 * @{
 * @addtogroup I3C_MASTER
 * @{
 * This section describes the programming interfaces of the Improved-Inter-Integrated Circuit Master (I3C_MASTER) driver.
 *
 * @section HAL_I3C_Master_Terms_Chapter Terms and acronyms
 *
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b DMA                        | Direct Memory Access. For an introduction to DMA, please refer to <a href="https://en.wikipedia.org/wiki/Direct_memory_access"> DMA in Wikipedia</a> .|
 * |\b GPIO                       | General Purpose Inputs-Outputs. For more details, please refer to the @ref GPIO module in HAL. |
 * |\b I3C                        | Improved-Inter-Integrated Circuit. I3C is typically used to attach low-speed peripheral ICs to processors and microcontrollers. For an introduction to I3C, please refer to <a href="https://en.wikipedia.org/wiki/I3C_(bus)"> I3C in Wikipedia</a>.|
 * |\b NVIC                       | Nested Vectored Interrupt Controller. NVIC is the interrupt controller of ARM Cortex-M4. For more details, please refer to <a href="http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100166_0001_00_en/ric1417175922867.html"> NVIC introduction in ARM Cortex-M4 Processor Technical Reference Manual </a>.|
 * @section HAL_I3C_Master_Features_Chapter Supported features
 * - \b Supported \b CCC \b command \b modes \b : \n
 *   In this mode, user can transfer CCC command packets to the slave device. The functions used for CCC transfer are:
    - #hal_i3c_master_control().
 * - \b Supported \b private \b write /\b read \b modes \b : \n
 *   In this mode, user can write/read data from the slave device. The functions used for CCC transfer are:
 *  - #hal_i3c_master_transfer().
 *
 * @section HAL_I3C_Master_Driver_Usage_Chapter How to use this driver
 * - \b Using \b CCC \b command \b mode.  \n
 *  - I3C broadcast CCC format:
 *  @image html hal_i3c_broadcast_ccc_format.png
 *
 *  - I3C direct CCC format:
 *  @image html hal_i3c_direct_ccc_format.png
 *
 *  To use the I3C driver, you must configure the GPIO pins to pinmux to I3C SCL and I3C SDA, then call
 *  #hal_pinmux_set_function() to select the pinmux function. After setting the pinmux, call #hal_i3c_master_init() to initialize I3C
 *  status. Call #hal_i3c_master_deinit() to release the I3C resource.\n
 *  Steps are shown below:
 *  - Step1: Call #hal_gpio_init() to initialize the pin. For mode details about hal_gpio_init, please refer to GPIO module in HAL.
 *  - Step2: Call #hal_pinmux_set_function() to set the GPIO pinmux or use the EPT tool to apply the pinmux settings.
 *  - Step3: Call #hal_i3c_master_init() to initialize the I3C master.
 *  - Step4: Call #hal_i3c_master_control() to send the CCC command to the slave device.
 *  - Step5: Call #hal_i3c_master_deinit() to de-allocate the I3C master if it is no longer in use.
 *  - Sample code:
 *    @code
 *       ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  static uint8_t i3c_rx_buff[128];
 *       //pid: should be gotten from the sensor datasheet, da: an address which will be assigned to the 'pid' device
 *       hal_i3c_master_pid_da_map_t slave_device_map[1] = {
 *          {.pid = {0x02, 0x08, 0x00, 0x6C, 0x10, 0x0B}, .da = 0x22},
 *       };
 *       hal_i3c_master_config_t         init_config;
 *       hal_i3c_master_control_config_t config;
 *
 *       //Init GPIO, set GPIO pinmux (if EPT tool has not been used to configure the related pinmux).
 *       hal_gpio_init(HAL_GPIO_27);
 *       hal_gpio_init(HAL_GPIO_28);
 *       //Call hal_pinmux_set_function() to set GPIO pinmux, for more information, please reference hal_pinmux_define.h
 *       //It is not necessary to configure pinmux if the EPT tool is used.
 *       //function_index = HAL_GPIO_17_I3C0_SCL
 *       hal_pinmux_set_function(HAL_GPIO_17, function_index);
 *       //function_index = HAL_GPIO_18_I3C0_SDA
 *       hal_pinmux_set_function(HAL_GPIO_18, function_index);
 *
 *       //Init I3C master config parameter
 *       init_config.mode           = HAL_I3C_MASTER_WORKING_MODE_I3C;
 *       init_config.map_pid_da_config = slave_device_map;
 *       init_config.map_pid_da_sz     = sizeof(slave_device_map) / sizeof(hal_i3c_master_pid_da_map_t);
 *       init_config.fast_speed_khz = 1000;
 *       init_config.high_speed_khz = 12000;
 *       init_config.call_back = i3c_user_callback;
 *       init_config.user_data = NULL;
 *
 *       //Init CCC config as GETMWL
 *       config.trans_type     = HAL_I3C_MASTER_TRANSFER_TYPE_RECEIVE;
 *       config.slave_addr     = 0x22;
 *       config.ccc_cmd        = 0x8B; //CCC:GETMWL
 *       config.def_byte_data  = NULL; //no defining byte for GETMWL
 *       config.data_buff_size = 0;    //no defining byte for GETMWL
 *       config.data_buff      = i3c_rx_buff;
 *       config.data_buff_size = 2;
 *
 *       if(HAL_I3C_MASTER_STATUS_OK == hal_i3c_master_init(HAL_I3C_MASTER_0, &init_config)) {
 *           //Send data
 *           if (HAL_I3C_MASTER_STATUS_OK != hal_i3c_master_control(HAL_I3C_MASTER_0, &config)) {
 *               //Error handler;
 *           }
 *       }
 *    @endcode
 *    @code
 *       // Callback function sample code. This function should be passed to driver while calling the function #hal_i3c_master_control().
 *       void i3c_user_callback (hal_i3c_master_port_t i3c_port,uint8_t slave_address, hal_i3c_master_callback_event_t event, void *user_data)
 *       {
 *           if(HAL_I3C_EVENT_SUCCESS == event) {
 *               //send success;
 *           } else {
 *               // handle other error ;
 *           }
 *       }
 *       //hal_i3c_master_deinit(HAL_I3C_MASTER_0);// Call this function if the I3C is no longer in use.
 *    @endcode
 *
 * - \b Using \b Private \b Write/\b Read \b mode.
 *  - I3C private write format:
 *  @image html hal_i3c_private_write_format.png
 *
 *  - I3C private read format:
 *  @image html hal_i3c_private_read_format.png
 *
 *  - I3C private write to read format:
 *  @image html hal_i3c_private_write_to_read_format.png
 *
 *  To use the I3C driver, you must configure the GPIO pins to pinmux to I3C SCL and I3C SDA, then call
 *  #hal_pinmux_set_function() to select the pinmux function. After setting the pinmux, call #hal_i3c_master_init() to initialize I3C
 *  status. Call #hal_i3c_master_deinit() to release the I3C resource.\n
 *  Steps are shown below:
 *  - Step1: Call #hal_gpio_init() to initialize the pin. For more details about hal_gpio_init, please refer to GPIO module in HAL.
 *  - Step2: Call #hal_pinmux_set_function() to set the GPIO pinmux or use the EPT tool to apply the pinmux settings.
 *  - Step3: Call #hal_i3c_master_init() to initialize the I3C master.
 *  - Step4: Call #hal_i3c_master_transfer() to send/receive data to or from the slave device.
 *  - Step5: Call #hal_i3c_master_deinit() to de-allocate the I3C master if it is no longer in use.
 *  - Sample code:
 *    @code
 *
 *       ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  static uint8_t i3c_rx_buff[128];
 *       ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  static uint8_t i3c_tx_buff[128];
 *
 *       //pid: should be gotten from  the sensor datasheet, da: an address which is assigned to the 'pid' device
 *       hal_i3c_master_pid_da_map_t slave_device_map[1] = {
 *          {.pid = {0x02, 0x08, 0x00, 0x6C, 0x10, 0x0B}, .da = 0x22},
 *       };
 *
 *       hal_i3c_master_config_t    init_config;
 *       hal_i3c_master_transfer_config_t config;
 *
 *       //Init GPIO, set GPIO pinmux (if EPT tool has not been used to configure the related pinmux).
 *       hal_gpio_init(HAL_GPIO_27);
 *       hal_gpio_init(HAL_GPIO_28);
 *       //Call hal_pinmux_set_function() to set GPIO pinmux, for more information, please reference hal_pinmux_define.h
 *       //It is not necessary to configure pinmux if the EPT tool is used.
 *       //function_index = HAL_GPIO_17_I3C0_SCL
 *       hal_pinmux_set_function(HAL_GPIO_17, function_index);
 *       //function_index = HAL_GPIO_18_I3C0_SDA
 *       hal_pinmux_set_function(HAL_GPIO_18, function_index);
 *
 *       //Init I3C master config parameter
 *       init_config.mode           = HAL_I3C_MASTER_WORKING_MODE_I3C;
 *       init_config.map_pid_da_config = slave_device_map;
 *       init_config.map_pid_da_sz     = sizeof(slave_device_map) / sizeof(hal_i3c_master_pid_da_map_t);;
 *       init_config.fast_speed_khz = 1000;
 *       init_config.high_speed_khz = 12000;
 *       init_config.call_back = i3c_user_callback;
 *       init_config.user_data = NULL;
 *
 *       //Get data from slave device of 0xF register
 *       i3c_tx_buff = 0x0F;
 *       config.trans_type     = HAL_I3C_MASTER_TRANSFER_TYPE_SEND_TO_RECEIVE;
 *       config.slave_addr     = 0x22; //Dynamic Address
 *       config.send_buff      = i3c_tx_buff;
 *       config.send_size      = 1;
 *       config.recv_buff      = i3c_rx_buff;
 *       config.recv_size      = 1;
 *
 *       if(HAL_I3C_MASTER_STATUS_OK == hal_i3c_master_init(HAL_I3C_MASTER_0, &init_config)) {
 *           //Send data
 *           if (HAL_I3C_MASTER_STATUS_OK != hal_i3c_master_transfer(HAL_I3C_MASTER_0, &config)) {
 *               //Error handler;
 *           }
 *       }
 *    @endcode
 *    @code
 *       // Callback function sample code. This function should be passed to the driver while calling the function #hal_i3c_master_control().
 *       void i3c_user_callback (hal_i3c_master_port_t i3c_port,uint8_t slave_address, hal_i3c_master_callback_event_t event, void *user_data)
 *       {
 *           if(HAL_I3C_EVENT_SUCCESS == event) {
 *               //send success;
 *           } else {
 *               // handle other error ;
 *           }
 *       }
 *       //hal_i3c_master_deinit(HAL_I2C_MASTER_0);// Call this function if the I3C is no longer in use.
 *    @endcode
 *
 *
 */


/** @brief This enum defines the HAL interface return value. */
typedef enum {
    HAL_I3C_MASTER_STATUS_ERROR = -6,                        /**<  An error occurred and the transaction failed. */
    HAL_I3C_MASTER_STATUS_ERROR_BUS_CFG = -5,                /**<  An error occurred during I3C bus config. */
    HAL_I3C_MASTER_STATUS_ERROR_NO_SPACE = -4,               /**<  The sw FIFO full. */
    HAL_I3C_MASTER_STATUS_ERROR_BUSY = -3,                   /**<  The I2C bus is busy. */
    HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER = -2,          /**<  A wrong port number is given. */
    HAL_I3C_MASTER_STATUS_INVALID_PARAMETER = -1,            /**<  A wrong parameter is given. */
    HAL_I3C_MASTER_STATUS_OK = 0                             /**<  No error occurred during the function call. */
} hal_i3c_master_status_t;



/** @brief This enum defines the transaction result event. This result is passed through a callback function.
  */
typedef enum {
    HAL_I3C_EVENT_SUCCESS     = 0,  /**<  The transaction completed without any error. */
    HAL_I3C_EVENT_ACK_ERROR   = 1,  /**<  An ACK error occurred during transaction. */
    HAL_I3C_EVENT_NACK_ERROR  = 2,  /**<  A NACK error occurred during transaction. */
    HAL_I3C_EVENT_ARB_LOST    = 3,  /**<  The current master lost arbtration. */
    HAL_I3C_EVENT_DAA         = 4,  /**<  An DAA event occurred. */
    HAL_I3C_EVENT_TIMEOUT     = 5,  /**<  A timeout error occurred during transaction. */
    HAL_I3C_EVENT_DMA_ERROR   = 6,  /**<  A DMA error occurred during transaction. */
    HAL_I3C_EVENT_IBI         = 7,  /**<  An IBI event occurred. */
    HAL_I3C_EVENT_ERROR       = 8,  /**<  An error occurred during transaction. */
    HAL_I3C_EVENT_UNKNOW      = 9   /**<  Unknown event reason. */
} hal_i3c_master_callback_event_t;


/** @defgroup hal_i3c_master_typedef Typedef
  * @{
  */

/** @brief  This defines the callback function prototype.
 *          This function is called after the transaction finishes in the I2C ISR routine.
 *  @param [in] i3c_port is the I3C master port number. The value is defined in #hal_i3c_master_port_t.
 *  @param [in] slave_address is a user defined slave address to send or receive data.
 *  @param [in] event is the transaction event of the current transaction. It also provides the transaction result.
 *              For more details about the event type, please refer to #hal_i3c_master_callback_event_t.
 *  @param [in] append_data is the append data only for IBI or DAA event.
 *              When event is IBI, the append_data is slave address.
                When event is DAA, the append_data is the PID/BCR/DCR.
 *  @param [in] user_data is the user defined parameter obtained from #hal_i2c_master_init() function.
 *  @sa  #hal_i2c_master_register_callback()
 */
typedef void (*hal_i3c_master_callback_t)(hal_i3c_master_port_t i3c_port,uint8_t slave_address, hal_i3c_master_callback_event_t event, void *user_data);

/**
  * @}
  */


/** @brief This enum defines the I3C transfer type.  */
typedef enum {
    HAL_I3C_MASTER_TRANSFER_TYPE_SEND_TO_RECEIVE = 0,   /**<  Transfer type of send to receive. */
    HAL_I3C_MASTER_TRANSFER_TYPE_SEND,                  /**<  Transfer type of send. */
    HAL_I3C_MASTER_TRANSFER_TYPE_RECEIVE,               /**<  Transfer type of receive. */
}hal_i3c_master_transfer_type_t;

/** @brief This enum defines the I3C hardware working mode.  */
typedef enum {
    HAL_I3C_MASTER_WORKING_MODE_I3C = 0,   /**<  I3C hardware work in I3C mode. */
    HAL_I3C_MASTER_WORKING_MODE_I2C,       /**<  I3C hardware work in I2C mode. */
}hal_i3c_master_working_mode_t;

/** @brief This structure defines the map of PID and Da for I3C initialize. */
typedef struct {
    uint8_t pid[6]; /**<  The unique ID for I3C slave device. */
    uint8_t da;     /**<  The Dynamic Address that is assigned to the slave device. */
}hal_i3c_master_pid_da_map_t;


/** @brief This structure defines the configuration parameters for I3C initialize. */
typedef struct {
    hal_i3c_master_working_mode_t mode;     /**<  Indicates the hardware working mode. */
    uint32_t  fast_speed_khz;               /**<  I3C transfer speed in fast-speed mode. */
    uint32_t  high_speed_khz;               /**<  I3C transfer speed in high-speed mode(Max:12.5MHz). */
    hal_i3c_master_pid_da_map_t  *map_pid_da_config; /**<  The map of PID and Dynamic Address. */
    uint32_t                      map_pid_da_sz;     /**<  The number of map items. */
    hal_i3c_master_callback_t   call_back;  /**<  The user callback. */
    void                       *user_data;  /**<  The user data. */
}hal_i3c_master_config_t;


/** @brief This structure defines the configuration parameters for I3C CCC transfer. */
typedef struct {
    hal_i3c_master_transfer_type_t trans_type; /**<  The I3C transfer type. */
    uint16_t    slave_addr;             /**<  The slave device address. */
    uint8_t     ccc_cmd;                /**<  The CCC for I3C protocol. */
    uint32_t    def_byte_size;          /**<  The size of Defining Byte. */
    uint8_t    *def_byte_data;          /**<  The data of Defining Byte. */
    uint32_t    data_buff_size;         /**<  The size of Data. */
    uint8_t    *data_buff;              /**<  The buffer of Data. */
}hal_i3c_master_control_config_t;


/** @brief This structure defines the configuration parameters for I3C private transfer. */
typedef struct {
    hal_i3c_master_transfer_type_t trans_type;  /**<  The I3C transfer type. */
    uint16_t    slave_addr;              /**<  The slave device address. */
    uint32_t    send_size;               /**<  The send size of transfer. */
    uint8_t    *send_buff;               /**<  The send data of transfer. */
    uint32_t    recv_size;               /**<  The receive size of transfer. */
    uint8_t    *recv_buff;               /**<  The receive buffer of tansfer. */
}hal_i3c_master_transfer_config_t;


/**
 * @brief This function initializes the I3C master before starting a transaction.
 * @param[in] i3c_port is the I3C master port number. The value is defined in #hal_i3c_master_port_t.
 * @param[in] i3c_config Is the configuration parameter to initialize the I3C. Details are described at #hal_i3c_master_config_t.
 * @return #HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER, an invalid port number is given; \n
 *         #HAL_I3C_MASTER_STATUS_INVALID_PARAMETER, an invalid transfer_frequency is given; \n
 *         #HAL_I3C_MASTER_STATUS_OK, the operation completed successfully.
 *         #HAL_I3C_MASTER_STATUS_ERROR_BUSY, the I3C bus is in use.
 * @note   #hal_i3c_master_deinit() must be called when the I3C is no longer in use, release the I3C resource for the other users to use this I3C master.
 *         Please DO NOT call #hal_i3c_master_init() in interrupt handler, it may cause deadlock.
 *         In a multi-task applications, if #hal_i3c_master_init() returns error #HAL_I3C_MASTER_STATUS_ERROR_BUSY, we strongly suggest calling functions that can yield CPU and try
 *         again later.
 * @par       Example
 *    @code
 *
 * hal_i3c_master_status_t i3c_master_init(void)
 * {
 *   hal_i3c_master_port_t   i3c_port;
 *   hal_i3c_master_status_t error_status;
 *
 *   //Use the sensor PID to assign the Dynamic Address to this sensor
 *   hal_i3c_master_pid_da_map_t slave_device_da_map[2] = {
 *      {.pid = {0x02, 0x08, 0x00, 0x6C, 0x10, 0x0B}, .da = 0x22}, //config Da(0x22) to slave device0(PID:0x02, 0x08, 0x00, 0x6C, 0x10, 0x0B)
 *      {.pid = {0x00, 0x01, 0x00, 0x12, 0x02, 0x0C}, .da = 0x23}  //config Da(0x23) to slave device1(PID:0x00, 0x01, 0x00, 0x12, 0x02, 0x0C)
 *   };
 *   hal_i3c_master_init_config_t    init_config;
 *
 *   i3c_port = HAL_I3C_MASTER_0;
 *   init_config.mode           = HAL_I3C_MASTER_WORKING_MODE_I3C;
 *   init_config.fast_speed_khz = 1000; //1Mhz
 *   init_config.high_speed_khz = 12000;//12Mhz
 *   init_config.call_back = NULL;
 *   init_config.user_data = NULL;
 *   init_config.map_pid_da_config = slave_device_map;
 *   init_config.map_pid_da_sz     = sizeof(slave_device_da_map) / sizeof(hal_i3c_master_pid_da_map_t);
 *
 *   while (try_times < 10) {
 *       error_status = hal_i3c_master_init(i3c_port, &init_config);
 *       if (error_status != HAL_I3C_MASTER_STATUS_OK) {
 *           vTaskDelay((portTickType)100 / portTICK_RATE_MS);
 *           try_times ++;
 *       } else {
 *         break;
 *       }
 *    }
 *   return error_status;
 * }
 *
 *    @endcode
 *
 * @sa #hal_i3c_master_init()
 */
hal_i3c_master_status_t hal_i3c_master_init(hal_i3c_master_port_t i3c_port, hal_i3c_master_config_t *i3c_config);


/**
 * @brief This function sends the CCC command to the slave device (only used for I3C mode).
 *  This function will return a value immediately after configuring the hardware registers. It notifies user by callback.
 * @param[in] i3c_port is the I3C master port number. The value is defined in #hal_i3c_master_port_t.
 * @param[in] config is the configuration parameter of this API for send.
 * @return #HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER, an invalid port number is given; \n
 *         #HAL_I3C_MASTER_STATUS_INVALID_PARAMETER, an invalid transfer_frequency is given; \n
 *         #HAL_I3C_MASTER_STATUS_OK, the operation completed successfully.
 *         #HAL_I3C_MASTER_STATUS_ERROR_BUSY, the I3C bus is in use.
 *
 * @par       Example
 *    Sample code, please refer to @ref HAL_I3C_Master_Driver_Usage_Chapter
 * @sa #hal_i3c_master_control()
 */
hal_i3c_master_status_t hal_i3c_master_control(hal_i3c_master_port_t i3c_port, hal_i3c_master_control_config_t *config);


/**
 * @brief This function for private read/write from/to slave device.
 *  This function will return a value immediately after configuring the hardware registers. It notifies user by callback.
 * @param[in] i3c_port is the I3C master port number. The value is defined in #hal_i3c_master_port_t.
 * @param[in] config is the configuration parameter of this API for send.
 * @return #HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER, an invalid port number is given; \n
 *         #HAL_I3C_MASTER_STATUS_INVALID_PARAMETER, an invalid transfer_frequency is given; \n
 *         #HAL_I3C_MASTER_STATUS_OK, the operation completed successfully.
 *         #HAL_I3C_MASTER_STATUS_ERROR_BUSY, the I3C bus is in use.
 *
 * @par       Example
 *    Sample code, please refer to @ref HAL_I3C_Master_Driver_Usage_Chapter
 * @sa #hal_i3c_master_transfer()
 */
hal_i3c_master_status_t hal_i3c_master_transfer(hal_i3c_master_port_t i3c_port, hal_i3c_master_transfer_config_t *config);


/**
 * @brief This function releases the I3C master when the transaction process is complete. Call this function if the I3C is no longer in use.
 * @param[in] i3c_port is the I3C master port number. The value is defined in #hal_i3c_master_port_t.
 * @return  #HAL_I2C_STATUS_INVALID_PORT_NUMBER, an invalid port number is given; \n
 *          #HAL_I2C_STATUS_OK, the operation completed successfully.
 * @note   This function must be called when the I3C is no longer in use. Release the I3C resource for the other users to use this I3C master.
 * @par       Example
 *    Sample code, please refer to @ref HAL_I3C_Master_Driver_Usage_Chapter
 * @sa #hal_i3c_master_deinit()
 */
hal_i3c_master_status_t hal_i3c_master_deinit(hal_i3c_master_port_t i3c_port);

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/

#endif /* HAL_I3C_MASTER_MODULE_ENABLED */
#endif /* __HAL_I3C_MASTER_H__ */
