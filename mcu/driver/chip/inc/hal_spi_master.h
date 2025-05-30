/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __HAL_SPI_MASTER_H__
#define __HAL_SPI_MASTER_H__
#include "hal_platform.h"

#ifdef HAL_SPI_MASTER_MODULE_ENABLED


/**
 * @addtogroup HAL
 * @{
 * @addtogroup SPI_MASTER
 * @{
 * This section introduces the Serial Peripheral Interface Master (SPI_Master) APIs including terms and acronyms,
 * supported features, software architecture, details on how to use this driver, enums, structures and functions.
 *
 * @section HAL_SPI_MASTER_Terms_Chapter Terms and acronyms
 *
 * |Terms                   |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b DMA                        | Direct Memory Access. DMA is a feature of computer systems that allows certain hardware subsystems to access main system memory independent from the central processing unit (CPU).|
 * |\b FIFO                       | First In, First Out. FIFO is a method for organizing and manipulating a data buffer, where the first entry, or 'head' of the queue, is processed first.|
 * |\b GPIO                       | General Purpose Inputs-Outputs. For more details, please refer to @ref GPIO.|
 * |\b NVIC                       | Nested Vectored Interrupt Controller. NVIC is the interrupt controller of ARM Cortex-M series processors. For more details, please refer to <a href="http://infocenter.arm.com/help/topic/com.arm.doc.100166_0001_00_en/arm_cortexm4_processor_trm_100166_0001_00_en.pdf"> ARM Cortex-M4 technical reference manual</a>.|
 * |\b SPI                        | Serial Peripheral Interface. The Serial Peripheral Interface bus is a synchronous serial communication interface specification used for short distance communication. For more information, please check <a href="https://en.wikipedia.org/wiki/Serial_peripheral_interface"> Serial Peripheral Interface Bus in Wikipedia</a>.|
 * |\b TX                          | Transmit    |
 * |\b RX                          | Receive     |
 *
 * @section HAL_SPI_MASTER_Features_Chapter Supported features
 *
 * This controller supports a wide range of SPI interface devices, including full-duplex transaction ability to
 * communicate with both half-duplex and full-duplex devices. For half-duplex devices, the data flow direction is not relevant for the software.
 * Hardware provides access for various timing adjustments.
 *
 * @}
 * @}
 */
#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* - \b Support \b transaction \b format. \n
*  - Half-duplex transaction:
*    There is only one valid transaction at a time on a single direction; either send or receive. The functions using half-duplex transaction are:
*   - #hal_spi_master_send_polling().
*   - #hal_spi_master_send_dma().
*   - #hal_spi_master_send_dma_blocking().
*
*  - Full-duplex transaction:
*    There are two valid mutually inclusive transactions; send and receive. Functions using full-duplex transaction are:
*   - #hal_spi_master_send_and_receive_polling().
*   - #hal_spi_master_send_and_receive_dma().
*   - #hal_spi_master_send_and_receive_dma_blocking().
* @}
* @}
*/
#else
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* - \b Support \b transaction \b format. \n
*  - Half-duplex transaction:
*    There is only one valid transaction at a time on a single direction; either send or receive. The function using half-duplex transaction is:
*   - #hal_spi_master_send_polling().
*
*  - Full-duplex transaction:
*    There are two valid mutually inclusive transactions; send and receive. The function using full-duplex transaction is:
*   - #hal_spi_master_send_and_receive_polling().
*
* @}
* @}
*/
#endif
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* - \b Support \b transaction \b mode. \n
*  - Polling mode transaction:
*    In polling mode, #hal_spi_master_send_polling() and #hal_spi_master_send_and_receive_polling() return a value once the transaction is complete.
*    Then the user checks the output of the return value for the transaction result and error handling if any.
*
*    See @ref HAL_SPI_MASTER_Architecture_Chapter for the software architecture of polling mode.
*
* @}
* @}
*/
#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
*  - DMA mode transaction:
*    In DMA mode, #hal_spi_master_send_dma() and #hal_spi_master_send_and_receive_dma() return a value once the SPI hardware register is assigned and the DMA configured.
*    During this process the transaction is usually incomplete, once it completes, an interrupt is triggered and a related user callback
*    is called in the SPI interrupt service routine.
*
*    See @ref HAL_SPI_MASTER_Architecture_Chapter for the software architecture of DMA mode.
*
*  - DMA blocking mode transaction:
*    In DMA blocking mode, #hal_spi_master_send_dma_blocking() and #hal_spi_master_send_and_receive_dma_blocking() return a value once the SPI DMA transaction is complete after
*    hardware register is assigned and the DMA configured.
*
*    See @ref HAL_SPI_MASTER_Architecture_Chapter for the software architecture of DMA blocking mode.
* @}
* @}
*/
#endif
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Architecture_Chapter Software architecture of the SPI
* - \b Polling \b mode \b architecture. \n
*   Polling mode architecture is similar to the polling mode architecture in HAL overview. See @ref HAL_Overview_3_Chapter for polling mode architecture.
* @}
* @}
*/
#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* - \b DMA \b mode \b architecture. \n
*   DMA mode architecture is similar to the interrupt mode architecture in HAL overview. See @ref HAL_Overview_3_Chapter for interrupt mode architecture.
* - \b DMA \b blocking \b mode \b architecture. \n
*   DMA blocking mode architecture is similar to the polling mode architecture in HAL overview. See @ref HAL_Overview_3_Chapter for polling mode architecture.
* @}
* @}
*/
#endif
#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Driver_Usage_Chapter How to use this driver
*
* - \b Using \b SPI \b master \b polling \b mode. \n
*  Call #hal_pinmux_set_function() to pinmux GPIO pins to four SPI pins (CS_N, SCK, MOSI, and MISO) based on the user's hardware platform design.\n
*  Once a transaction is complete, call #hal_spi_master_deinit() function to release the SPI master resource to make it available for other users.\n
*  The steps are shown below:
*  - Step 1. Call #hal_gpio_init() to inite the pins, if EPT tool hasn't been used to configure the related pinmux.
*  - Step 2. Call #hal_pinmux_set_function() to set GPIO pinmux, if EPT tool hasn't been used to configure the related pinmux.
*           For more details about #hal_pinmux_set_function(), please refer to @ref GPIO.
*  - Step 3. Call #hal_spi_master_init() to initialize one SPI master. If the SPI master is already initialized by another user, user will get #HAL_SPI_MASTER_STATUS_ERROR_BUSY.
*  - Step 4. Call #hal_spi_master_send_polling() to send data in the polling mode.
*  - Step 5. Call #hal_spi_master_send_and_receive_polling() to send and receive data in the polling mode.
*  - Step 6. Call #hal_spi_master_deinit() to deinitialize the SPI master, if it's no longer in use.
*  - sample code:
*    @code
*       hal_spi_master_config_t spi_config;
*       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
*       uint8_t status_cmd = SPIS_STATUS_CMD;
*       uint8_t status_receive[2];
*       uint8_t data[2] = {0x7E, 0x55};
*       uint32_t size = 2;
*       spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
*       spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
*       spi_config.clock_frequency = 1000000;
*       spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
*       spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
*       status_receive[1] = 0;
*       spi_send_and_receive_config.receive_length = 2;
*       spi_send_and_receive_config.send_length = 1;
*       spi_send_and_receive_config.send_data = &status_cmd;
*       spi_send_and_receive_config.receive_buffer = status_receive;
*
*       // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
*       hal_gpio_init(HAL_GPIO_25);
*       hal_gpio_init(HAL_GPIO_26);
*       hal_gpio_init(HAL_GPIO_27);
*       hal_gpio_init(HAL_GPIO_28);
*       // Call hal_pinmux_set_function() to set GPIO pinmux, for more information, please refer to hal_pinmux_define.h
*       // Configure the pinmux, if EPT tool is used.
*       hal_pinmux_set_function(HAL_GPIO_25, HAL_GPIO_25_MA_SPI3_A_CS);// Set the pin to be used as CS signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_MA_SPI3_A_SCK);// Set the pin to be used as SCK signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_27, HAL_GPIO_27_MA_SPI3_A_MOSI);// Set the pin to be used as MOSI signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_28, HAL_GPIO_28_MA_SPI3_A_MISO);// Set the pin to be used as MISO signal of SPI.
*       // Initialize the SPI master.
*       if (HAL_SPI_MASTER_STATUS_OK == hal_spi_master_init(HAL_SPI_MASTER_3, &spi_config)) {
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(HAL_SPI_MASTER_3, data, size)) {
*               // Error handler;
*           }
*           //send and receive data at the same time
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_polling(HAL_SPI_MASTER_3, &spi_send_and_receive_config)) {
*               // Error handler;
*           }
*           hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if the SPI master is no longer in use.
*       } else {
*           // Error handler;
*       }
*    @endcode
*
* - \b Using \b SPI \b master \b DMA \b mode. \n
*  Call #hal_pinmux_set_function() to pinmux GPIO pins to four SPI pins (CS_N, SCK, MOSI, and MISO) based on the user's hardware platform design.\n
*  Then call #hal_spi_master_register_callback() to register a callback function.\n
*  Once a transaction is complete, call #hal_spi_master_deinit() function \b in \b your \b callback \b function to release the SPI master resource to make it available for other users.\n
*  The steps are shown below:
*  - Step 1. Call #hal_gpio_init() to inite the pins, if EPT tool hasn't been used to configure the related pinmux.
*  - Step 2. Call #hal_pinmux_set_function() to set GPIO pinmux, if EPT tool hasn't been used to configure the related pinmux.
*           For more details about #hal_pinmux_set_function(), please refer to @ref GPIO.
*  - Step 3. Call #hal_spi_master_init() to init one SPI master. If the SPI master is already initialized by another user, user will get #HAL_SPI_MASTER_STATUS_ERROR_BUSY.
*  - Step 4. Call #hal_spi_master_register_callback() to register a user callback.
*  - Step 5. Call #hal_spi_master_send_dma() to send data in the DMA mode.
*  - Step 6. Call #hal_spi_master_send_and_receive_dma() to send and receive data in the DMA mode.
*  - Step 7. Call #hal_spi_master_deinit() to deinit the SPI master, if it's no longer in use.
*  - sample code:
*    @code
*       hal_spi_master_config_t spi_config;
*       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_cmd = SPIS_STATUS_CMD;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_receive[2];
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t data[2] = {0x7E, 0x55};
*       uint32_t size = 2;
*
*       spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
*       spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
*       spi_config.clock_frequency = 1000000;
*       spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
*       spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
*       status_receive[1] = 0;
*       spi_send_and_receive_config.receive_length = 2;
*       spi_send_and_receive_config.send_length = 1;
*       spi_send_and_receive_config.send_data = &status_cmd;
*       spi_send_and_receive_config.receive_buffer = status_receive;
*
*       // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
*       hal_gpio_init(HAL_GPIO_25);
*       hal_gpio_init(HAL_GPIO_26);
*       hal_gpio_init(HAL_GPIO_27);
*       hal_gpio_init(HAL_GPIO_28);
*       // Call #hal_pinmux_set_function() to set the GPIO pinmux, for more information, please refer to hal_pinmux_define.h.
*       // No need to configure the pinmux, if EPT tool is used.
*       hal_pinmux_set_function(HAL_GPIO_25, HAL_GPIO_25_MA_SPI3_A_CS);// Set the pin to be used as CS signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_MA_SPI3_A_SCK);// Set the pin to be used as SCK signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_27, HAL_GPIO_27_MA_SPI3_A_MOSI);// Set the pin to be used as MOSI signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_28, HAL_GPIO_28_MA_SPI3_A_MISO);// Set the pin to be used as MISO signal of SPI.
*       // Initialize the SPI master.
*       if (HAL_SPI_MASTER_STATUS_OK == hal_spi_master_init(HAL_SPI_MASTER_3, &spi_config)) {
*           hal_spi_master_register_callback(HAL_SPI_MASTER_3, user_spi_callback, NULL); // Register a user callback.
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_dma(HAL_SPI_MASTER_3, data, size)) {
*               // Error handler;
*           }
*           // Send and receive data simultaneously.
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_dma(HAL_SPI_MASTER_3, &spi_send_and_receive_config)) {
*               // Error handler;
*           }
*       } else {
*           // Error handler;
*       }
*    @endcode
*    @code
*       // Callback function sample code. Pass this function to the driver while calling #hal_spi_master_register_callback().
*       void user_spi_callback (hal_spi_master_callback_event_t event, void *user_data)
*       {
*           if (HAL_SPI_MASTER_EVENT_SEND_FINISHED == event) {
*               // Send finish event handler;
*               // User code;
*               hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if no longer in use.
*           } else if (HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED == event) {
*               // Receive finish event handler;
*               // User code;
*               hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if no longer in use.
*           }
*       }
*    @endcode
*
* - \b Using \b SPI \b master \b DMA \b blocking \b mode. \n
*  Call #hal_pinmux_set_function() to pinmux GPIO pins to four SPI pins (CS_N, SCK, MOSI, and MISO) based on the user's hardware platform design.\n
*  Once a transaction is complete, call #hal_spi_master_deinit() function to release the SPI master resource to make it available for other users.\n
*  The steps are shown below:
*  - Step 1. Call #hal_gpio_init() to inite the pins, if EPT tool hasn't been used to configure the related pinmux.
*  - Step 2. Call #hal_pinmux_set_function() to set GPIO pinmux, if EPT tool hasn't been used to configure the related pinmux.
*           For more details about #hal_pinmux_set_function(), please refer to @ref GPIO.
*  - Step 3. Call #hal_spi_master_init() to initialize one SPI master. If the SPI master is already initialized by another user, user will get #HAL_SPI_MASTER_STATUS_ERROR_BUSY.
*  - Step 4. Call #hal_spi_master_send_dma_blocking() to send data in the DMA blocking mode.
*  - Step 5. Call #hal_spi_master_send_and_receive_dma_blocking() to send and receive data in the DMA blocking mode.
*  - Step 6. Call #hal_spi_master_deinit() to deinitialize the SPI master, if it's no longer in use.
*  - sample code:
*    @code
*       hal_spi_master_config_t spi_config;
*       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_cmd = SPIS_STATUS_CMD;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_receive[2];
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t data[2] = {0x7E, 0x55};
*       uint32_t size = 2;
*
*       spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
*       spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
*       spi_config.clock_frequency = 1000000;
*       spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
*       spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
*       status_receive[1] = 0;
*       spi_send_and_receive_config.receive_length = 2;
*       spi_send_and_receive_config.send_length = 1;
*       spi_send_and_receive_config.send_data = &status_cmd;
*       spi_send_and_receive_config.receive_buffer = status_receive;
*
*       // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
*       hal_gpio_init(HAL_GPIO_25);
*       hal_gpio_init(HAL_GPIO_26);
*       hal_gpio_init(HAL_GPIO_27);
*       hal_gpio_init(HAL_GPIO_28);
*       // Call hal_pinmux_set_function() to set GPIO pinmux, for more information, please refer to hal_pinmux_define.h
*       // Configure the pinmux, if EPT tool is used.
*       hal_pinmux_set_function(HAL_GPIO_25, HAL_GPIO_25_MA_SPI3_A_CS);// Set the pin to be used as CS signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_MA_SPI3_A_SCK);// Set the pin to be used as SCK signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_27, HAL_GPIO_27_MA_SPI3_A_MOSI);// Set the pin to be used as MOSI signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_28, HAL_GPIO_28_MA_SPI3_A_MISO);// Set the pin to be used as MISO signal of SPI.
*       // Initialize the SPI master.
*       if (HAL_SPI_MASTER_STATUS_OK == hal_spi_master_init(HAL_SPI_MASTER_3, &spi_config)) {
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_dma_blocking(HAL_SPI_MASTER_3, data, size)) {
*               // Error handler;
*           }
*           //send and receive data at the same time
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_dma_blocking(HAL_SPI_MASTER_3, &spi_send_and_receive_config)) {
*               // Error handler;
*           }
*           hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if the SPI master is no longer in use.
*       } else {
*           // Error handler;
*       }
*    @endcode
* @}
* @}
*/
#ifdef HAL_SPI_MASTER_FEATURE_NO_BUSY
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* - \b Using \b SPI \b master \b no \b busy \b API \b in \b DMA \b mode . \n
*  Call #hal_pinmux_set_function() to pinmux GPIO pins to four SPI pins (CS_N, SCK, MOSI, and MISO) based on the user's hardware platform design.\n
*  Then call #hal_spi_master_register_callback() to register a callback function.\n
*  Once a transaction is complete, call #hal_spi_master_deinit() function \b in \b your \b callback \b function to release the SPI master resource to make it available for other users.\n
*  The steps are shown below:
*  - Step 1. Call #hal_gpio_init() to inite the pins, if EPT tool hasn't been used to configure the related pinmux.
*  - Step 2. Call #hal_pinmux_set_function() to set GPIO pinmux, if EPT tool hasn't been used to configure the related pinmux.
*           For more details about #hal_pinmux_set_function(), please refer to @ref GPIO.
*  - Step 3. Call #hal_spi_master_send_and_receive_dma_no_busy() to send and receive data in the DMA mode.
*  - sample code:
*    @code
*       hal_spi_master_config_t spi_config;
*       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
*       hal_spi_send_and_receive_config_ex_no_busy_t spi_send_and_receive_config_ex_no_busy;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_cmd = SPIS_STATUS_CMD;
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t status_receive[2];
*       ATTR_ZIDATA_IN_NONCACHED_RAM_16BYTE_ALIGN uint8_t data[2] = {0x7E, 0x55};
*       uint32_t size = 2;
*
*       spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
*       spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
*       spi_config.clock_frequency = 1000000;
*       spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
*       spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
*       status_receive[1] = 0;
*       spi_send_and_receive_config.receive_length = 2;
*       spi_send_and_receive_config.send_length = 1;
*       spi_send_and_receive_config.send_data = &status_cmd;
*       spi_send_and_receive_config.receive_buffer = status_receive;
*        hal_spi_send_and_receive_config_ex_no_busy.spi_config = spi_config;
*        hal_spi_send_and_receive_config_ex_no_busy.spi_callback = user_spi_callback;
*        hal_spi_send_and_receive_config_ex_no_busy.spi_send_and_receive_config_ex = spi_send_and_receive_config;
*        hal_spi_send_and_receive_config_ex_no_busy.user_data = NULL;
*
*       // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
*       hal_gpio_init(HAL_GPIO_25);
*       hal_gpio_init(HAL_GPIO_26);
*       hal_gpio_init(HAL_GPIO_27);
*       hal_gpio_init(HAL_GPIO_28);
*       // Call #hal_pinmux_set_function() to set the GPIO pinmux, for more information, please refer to hal_pinmux_define.h.
*       // No need to configure the pinmux, if EPT tool is used.
*       hal_pinmux_set_function(HAL_GPIO_25, HAL_GPIO_25_MA_SPI3_A_CS);// Set the pin to be used as CS signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_MA_SPI3_A_SCK);// Set the pin to be used as SCK signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_27, HAL_GPIO_27_MA_SPI3_A_MOSI);// Set the pin to be used as MOSI signal of SPI.
*       hal_pinmux_set_function(HAL_GPIO_28, HAL_GPIO_28_MA_SPI3_A_MISO);// Set the pin to be used as MISO signal of SPI.
*
*       // Send and receive data simultaneously.
*       if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_dma_no_busy(HAL_SPI_MASTER_0, &hal_spi_send_and_receive_config_ex_no_busy)) {
*           // Error handler;
*       }
*    @endcode
*    @code
*       // Callback function sample code. Pass this function to the driver while calling #hal_spi_master_send_and_receive_dma_no_busy().
*       void user_spi_callback (hal_spi_master_callback_event_t event,void *user_data)
*       {
*           if (HAL_SPI_MASTER_EVENT_SEND_FINISHED == event) {
*               // Send finish event handler;
*               // User code;
*               hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if no longer in use.
*           } else if (HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED == event) {
*               // Receive finish event handler;
*               // User code;
*               hal_spi_master_deinit(HAL_SPI_MASTER_3); // Deinitialize, if no longer in use.
*           } else if (HAL_SPI_MASTER_NO_BUSY_FUNCTION_ERROR == event) {
*          // Error handler;
*          }
*       }
*    @endcode
* @}
* @}
*/
#endif
#else
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Driver_Usage_Chapter How to use this driver
*
* - \b Using \b SPI \b master \b polling \b mode. \n
*  Call #hal_pinmux_set_function() to pinmux GPIO pins to four SPI pins (CS_N, SCK, MOSI, and MISO) based on the user's hardware platform design.\n
*  Once a transaction is complete, call #hal_spi_master_deinit() function to release the SPI master resource to make it available for other users.\n
*  The steps are shown below:
*  - Step 1. Call #hal_gpio_init() to inite the pins, if EPT tool hasn't been used to configure the related pinmux.
*  - Step 2. Call #hal_pinmux_set_function() to set GPIO pinmux, if EPT tool hasn't been used to configure the related pinmux.
*           For more details about #hal_pinmux_set_function(), please refer to @ref GPIO. Note, user
*           should configure the chip select pin as GPIO mode, this driver will set the chip select signal as valid before starting the transaction,
*           then set the chip select signal as invalid after finishing the transaction.
*  - Step 3. Call #hal_spi_master_init() to initialize one SPI master.
*  - Step 4. Call #hal_spi_master_send_polling() to send data in the polling mode.
*  - Step 5. Call #hal_spi_master_send_and_receive_polling() to send and receive data in the polling mode.
*  - Step 6. Call #hal_spi_master_deinit() to deinitialize the SPI master, if it's no longer in use.
*  - sample code:
*    @code
*       hal_spi_master_config_t spi_config;
*       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
*       uint8_t status_cmd = SPIS_STATUS_CMD;
*       uint8_t status_receive[2];
*       uint8_t send_data[2] = {0x7E, 0x55};
*       uint32_t size = 2;
*       spi_config.bit_order = HAL_SPI_MASTER_MSB_FIRST;
*       spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
*       spi_config.clock_frequency = 1000000;
*       spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE1;
*       spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY1;
*       status_receive[1] = 0;
*       spi_send_and_receive_config.receive_length = 2;
*       spi_send_and_receive_config.send_length = 1;
*       spi_send_and_receive_config.send_data = &status_cmd;
*       spi_send_and_receive_config.receive_buffer = status_receive;
*
*       // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
*       hal_gpio_init(HAL_GPIO_29);
*       hal_gpio_init(HAL_GPIO_30);
*       hal_gpio_init(HAL_GPIO_31);
*       hal_gpio_init(HAL_GPIO_32);
*       // The chip select pin should be configured in GPIO mode. No need to configure the pinmux, if EPT tool is used.
*       hal_pinmux_set_function(HAL_GPIO_29, HAL_GPIO_29_SPI_MOSI_M_CM4);
*       hal_pinmux_set_function(HAL_GPIO_30, HAL_GPIO_30_SPI_MISO_M_CM4);
*       hal_pinmux_set_function(HAL_GPIO_31, HAL_GPIO_31_SPI_SCK_M_CM4);
*       hal_pinmux_set_function(HAL_GPIO_32, HAL_GPIO_32_GPIO32);
*
*       // Initialize the SPI master.
*       if (HAL_SPI_MASTER_STATUS_OK == hal_spi_master_init(HAL_SPI_MASTER_0, &spi_config)) {
*           //send data
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_polling(HAL_SPI_MASTER_0, data, size)) {
*               // Error handle.
*           }
*           //send and receive data at the same time
*           if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_send_and_receive_polling(HAL_SPI_MASTER_0, &spi_send_and_receive_config)) {
*               // Error handle.
*           }
*           hal_spi_master_deinit(HAL_SPI_MASTER_0); //Must call deinit when don't use SPI master
*       } else {
*           // Error handle.
*       }
*    @endcode
* @}
* @}
*/
#endif

#ifdef HAL_GPIO_FEATURE_SET_DRIVING
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Pin_Driving_Adjust_Guide_Chapter How to adjust the driving current of the SPI master
*
* When the SPI master is operating on a high frequency clock and the hardware environment is not well designed,
* the SPI master may malfunction because of bad signal integrity.
* Use an oscilloscope to find out which pin requires an adjustment. Normally it'll be clock pin and data pin.
* Then call #hal_gpio_set_driving_current() to adjust the driving current of a specific SPI pin.
*  - sample code:
*    @code
*    // To adjust the SCK pin (corresponds to GPIO26) of the SPI master,
*    // call #hal_gpio_init() and then #hal_pinmux_set_function() to initialize this SPI pin.
*    hal_gpio_init(HAL_GPIO_26);
*    hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_MA_SPI3_A_SCK);
*    // Call #hal_gpio_set_driving_current() to set the proper driving current for this SPI pin.
*    hal_gpio_set_driving_current(HAL_GPIO_26, HAL_GPIO_DRIVING_8MA);
*    // Call #hal_spi_master_init() to initialize the SPI master.
*    hal_spi_master_init(HAL_SPI_MASTER_0 ,&spi_config);
*    @endcode
* @}
* @}
*/
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Transfer_Length_Limitation Transfer length limitation of the SPI master
*
* There are limits to both the send length and the receive length that are related to the design of the SPI master hardware.
* Please refer to the following table for more information about these limits.
*
* | HAL API | TX(min) | TX(max) | RX(min) | RX(max) | Note |
* |---------|---------|---------|---------|---------|------|
* | #hal_spi_master_send_polling() | 1 | #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE | N/A | N/A | |
* | #hal_spi_master_send_and_receive_polling() | 0 | #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE | 1 | #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE | Single mode |
* | #hal_spi_master_send_and_receive_polling() | 0 | #HAL_SPI_MAXIMUM_NON_SINGLE_MODE_TRANSACTION_SIZE | 1 | #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE | exclude Single mode |
* | #hal_spi_master_send_dma() | 1 | no limit | N/A | N/A | |
* | #hal_spi_master_send_dma_blocking() | 1 | no limit | N/A | N/A | |
* | #hal_spi_master_send_and_receive_dma() | 0 | no limit | 1 | no limit | Single mode |
* | #hal_spi_master_send_and_receive_dma() | 0 | #HAL_SPI_MAXIMUM_NON_SINGLE_MODE_TRANSACTION_SIZE | 1 | no limit | exclude Single mode |
* | #hal_spi_master_send_and_receive_dma_blocking() | 0 | no limit | 1 | no limit | Single mode |
* | #hal_spi_master_send_and_receive_dma_blocking() | 0 | #HAL_SPI_MAXIMUM_NON_SINGLE_MODE_TRANSACTION_SIZE | 1 | no limit | exclude Single mode |
*
* Note: The length of the TX is less than or equal to the length of RX when calling APIs to send and receive data.
* @}
* @}
*/
#else
/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
* @section HAL_SPI_MASTER_Transfer_Length_Limitation Transfer length limitation of the SPI master
*
* There are limits to both the send length and the receive length that are related to the design of the SPI master hardware.
* Please refer to the following table for more information about these limits.
*
* | HAL API | TX(min) | TX(max) | RX(min) | RX(max) |
* |---------|---------|---------|---------|---------|
* | #hal_spi_master_send_polling() | 1 | no limit | N/A | N/A |
* | #hal_spi_master_send_and_receive_polling() | 0 | #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE | 1 | no limit |
*
* Note: The length of the TX is less than or equal to the length of RX when calling APIs to send and receive data.
* @}
* @}
*/
#endif

/**
* @addtogroup HAL
* @{
* @addtogroup SPI_MASTER
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup hal_spi_master_enum Enum
  * @{
  */

#ifdef HAL_SPI_MASTER_FEATURE_ADVANCED_CONFIG
/** @brief SPI master data transfer byte order definition.
 *         Before calling #hal_spi_master_send_polling() or #hal_spi_master_send_and_receive_polling(), user should make sure the \b byte_order
 *         parameter defined in #hal_spi_master_advanced_config_t structure and configured through #hal_spi_master_set_advanced_config()
 *         is #HAL_SPI_MASTER_LITTLE_ENDIAN, because setting as #HAL_SPI_MASTER_BIG_ENDIAN has no effect in send direction, but has effect in receive direction.
 */
typedef enum {
    HAL_SPI_MASTER_LITTLE_ENDIAN = 0,                       /**< Both send and receive data use Little-Endian format. */
    HAL_SPI_MASTER_BIG_ENDIAN    = 1                        /**< Both send and receive data use Big-Endian format. */
} hal_spi_master_byte_order_t;


/** @brief SPI master chip select polarity definition. */
typedef enum {
    HAL_SPI_MASTER_CHIP_SELECT_LOW  = 0,                /**< Chip select polarity active low. */
    HAL_SPI_MASTER_CHIP_SELECT_HIGH = 1                 /**< Chip select polarity active high. */
} hal_spi_master_chip_select_polarity_t;


/** @brief SPI master tolerance get_tick timing (based on SPI system clock) setting. */
typedef enum {
    HAL_SPI_MASTER_NO_GET_TICK_MODE = 0,              /**< Timing tolerance ability not used. */
    HAL_SPI_MASTER_GET_TICK_DELAY1  = 1,              /**< Delay 1 SPI system clock cycle for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY2  = 2,              /**< Delay 2 SPI system clock cycles for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY3  = 3,              /**< Delay 3 SPI system clock cycles for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY4  = 4,              /**< Delay 4 SPI system clock cycles for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY5  = 5,              /**< Delay 5 SPI system clock cycles for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY6  = 6,              /**< Delay 6 SPI system clock cycles for get_tick. */
    HAL_SPI_MASTER_GET_TICK_DELAY7  = 7               /**< Delay 7 SPI system clock cycles for get_tick. */
} hal_spi_master_get_tick_mode_t;


/** @brief SPI master sample edge of MISO definition. */
typedef enum {
    HAL_SPI_MASTER_SAMPLE_POSITIVE = 0,               /**< Sample edge of MISO is at the positive edge. */
    HAL_SPI_MASTER_SAMPLE_NEGATIVE = 1                /**< Sample edge of MISO is at the negative edge. */
} hal_spi_master_sample_select_t;
#endif


#ifdef HAL_SPI_MASTER_FEATURE_DEASSERT_CONFIG
/** @brief SPI master chip select deasert definition. */
typedef enum {
    HAL_SPI_MASTER_DEASSERT_DISABLE = 0,              /**< Chip select deassert is disabled. */
    HAL_SPI_MASTER_DEASSERT_ENABLE  = 1               /**< Chip select deassert is enabled. */
} hal_spi_master_deassert_t;
#endif


#ifdef HAL_SPI_MASTER_FEATURE_MACRO_CONFIG
/** @brief SPI master pad macro select definition. */
typedef enum {
    HAL_SPI_MASTER_MACRO_GROUP_A = 0,                   /**< SPI use PAD group A. */
    HAL_SPI_MASTER_MACRO_GROUP_B = 1,                   /**< SPI use PAD group B. */
    HAL_SPI_MASTER_MACRO_GROUP_C = 2                    /**< SPI use PAD group C. */
} hal_spi_master_macro_select_t;
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/** @brief SPI master callback event definition. */
typedef enum {
    HAL_SPI_MASTER_EVENT_SEND_FINISHED      = 0,              /**< SPI master send operation completed. */
    HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED   = 1,              /**< SPI master receive operation completed. */
    HAL_SPI_MASTER_NO_BUSY_FUNCTION_ERROR   = 2,              /**< SPI master transsimission error in no busy API. */
} hal_spi_master_callback_event_t;
#endif

/** @brief SPI master status. */
typedef enum {
    HAL_SPI_STATUS_FULL_BY_NON_BLOCKING_FUNCTION = -7,            /**< SPI master software fifo buffer is full. */
    HAL_SPI_STATUS_ERROR_USED_BY_NORMAL_USER     = -6,            /**< SPI master port is occupied by other users who call busy API. */
    HAL_SPI_STATUS_BUSY_BY_NON_BLOCKING_FUNCTION = -5,            /**< SPI master software fifo is busy. */
    HAL_SPI_MASTER_STATUS_ERROR                  = -4,            /**< SPI master function error occurred. */
    HAL_SPI_MASTER_STATUS_ERROR_BUSY             = -3,            /**< SPI master is busy. */
    HAL_SPI_MASTER_STATUS_ERROR_PORT             = -2,            /**< SPI master invalid port. */
    HAL_SPI_MASTER_STATUS_INVALID_PARAMETER      = -1,            /**< SPI master invalid input parameter. */
    HAL_SPI_MASTER_STATUS_OK                     = 0              /**< SPI master operation completed successfully. */
} hal_spi_master_status_t;


/** @brief SPI master running status. */
typedef enum {
    HAL_SPI_MASTER_BUSY              = 0,             /**< SPI master is busy. */
    HAL_SPI_MASTER_IDLE              = 1              /**< SPI master is idle. */
} hal_spi_master_running_status_t;

/**
  * @}
  */

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/** @defgroup hal_spi_master_typedef Typedef
  * @{
  */

/** @brief  This defines the callback function prototype.
 *          User should register a callback function when using SPI DMA mode, this function will be called in SPI interrupt
 *          service routine after a transaction completes. More details about the callback architecture, please refer to
 *          @ref HAL_SPI_MASTER_Architecture_Chapter.
 *  @param [in] event is the transaction event for the current transaction, user can get the transaction result from this parameter, please
 *              refer to #hal_spi_master_callback_event_t for details about the event type.
 *  @param [in] user_data is a user-defined parameter provided by #hal_spi_master_register_callback().
 *  @sa  #hal_spi_master_register_callback()
 */
typedef void (*hal_spi_master_callback_t)(hal_spi_master_callback_event_t event, void *user_data);

/**
  * @}
  */
#endif

/** @defgroup hal_spi_master_struct Struct
  * @{
  */

/** @brief SPI master configuration structure. */
typedef struct {
    uint32_t clock_frequency;                                   /**< SPI master clock frequency setting. */
    hal_spi_master_slave_port_t slave_port;                     /**< SPI slave device selection. */
    hal_spi_master_bit_order_t bit_order;                       /**< SPI master bit order setting. */
    hal_spi_master_clock_polarity_t polarity;                   /**< SPI master clock polarity setting. */
    hal_spi_master_clock_phase_t phase;                         /**< SPI master clock phase setting. */
} hal_spi_master_config_t;

#ifdef HAL_SPI_MASTER_FEATURE_ADVANCED_CONFIG
/** @brief SPI master advanced configuration structure. */
typedef struct {
    hal_spi_master_byte_order_t byte_order;                     /**< SPI master byte order setting. */
    hal_spi_master_chip_select_polarity_t chip_polarity;        /**< SPI master chip select active polarity setting. */
    hal_spi_master_get_tick_mode_t get_tick;                    /**< SPI clock timing tolerance setting. User may need to change this setting when high frequency is used. */
    hal_spi_master_sample_select_t sample_select;               /**< SPI MISO sample edge setting. */
} hal_spi_master_advanced_config_t;
#endif


#ifdef HAL_SPI_MASTER_FEATURE_CHIP_SELECT_TIMING
/** @brief SPI master chip select configuration structure. */
typedef struct {
    uint32_t chip_select_setup_count;                 /**< SPI master chip select setup count setting. The unit is in SPI base clock.
                                                           The range is from 0 to 32767. The chip select setup time is
                                                           (cs_setup_count+1)*CLK_PERIOD, where CLK_PERIOD is the cycle time
                                                           of the clock the SPI engine adopts. */
    uint32_t chip_select_hold_count;                  /**< SPI master chip select hold count setting. The unit is in SPI base clock.
                                                           The range is from 0 to 32767. The chip select hold time is
                                                           (cs_hold_count+1)*CLK_PERIOD. */
    uint32_t chip_select_idle_count;                  /**< SPI master chip select idle count setting. The unit is in SPI base clock.
                                                           The range is from 0 to 255. The chip select idle time between consecutive
                                                           transaction is (cs_idle_count+1)*CLK_PERIOD. */
} hal_spi_master_chip_select_timing_t;
#endif


/** @brief SPI master send and receive configuration structure. */
typedef struct {
    uint8_t *send_data;                               /**< Data buffer to send. */
    uint32_t send_length;                             /**< The number of bytes to send. */
    uint8_t *receive_buffer;                          /**< Received data buffer, this parameter cannot be NULL. */
    uint32_t receive_length;                          /**< The valid number of bytes received with the number of bytes to send. */
} hal_spi_master_send_and_receive_config_t;


#if defined(HAL_SPI_MASTER_FEATURE_NO_BUSY) && defined (HAL_SPI_MASTER_FEATURE_DMA_MODE)

/** @brief SPI non-single mode configuration structure for SPI master no busy API.
 */
typedef struct {
    hal_spi_master_mode_t mode;         /**< SPI master mode. */
    uint8_t dummy_bits;                 /**< SPI master dummy cycle configuration. */
    uint8_t command_bytes;              /**< SPI master command cycle configuration. */
} hal_spi_master_non_single_config_t;

/** @brief Send and receive configuration structure for SPI master no busy API.
 */
typedef struct {
    hal_spi_master_config_t spi_config;                /**< SPI master configuration. */
    hal_spi_master_callback_t spi_callback;            /**< SPI master callback function, it is called when transfer done or error occured in transmission.*/
    void *user_data;                                /**< user data for callback function. */
    hal_spi_master_send_and_receive_config_t spi_send_and_receive_config_ex;    /**< send and receive configuration. */
    hal_spi_master_advanced_config_t spi_advanced_config;       /**< SPI master advanced configuration. */
    hal_spi_master_non_single_config_t spi_non_single_config;   /**< SPI master non-single mode configuration. */
    hal_spi_master_chip_select_timing_t chip_select_timing;      /**< SPI master chip select timing configuration. */
    hal_spi_master_deassert_t deassert;                          /**< SPI master deassert configuration for chip select pin. */
} hal_spi_send_and_receive_config_ex_no_busy_t;

#endif


/**
  * @}
  */




/**
 * @brief     This function is mainly used to initialize the SPI master and set user defined common parameters like clock frequency,
 *            bit order, clock polarity, clock phase and default settings.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] spi_config is the SPI master configure parameters. Details are described at #hal_spi_master_config_t.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR means function error; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY means this port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_PORT means master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER means an invalid parameter is given by user; \n
 *            #HAL_SPI_MASTER_STATUS_OK means this function return successfully.
 * @note      #hal_spi_master_deinit() must be called when the SPI master is no longer in use, release the SPI master resource for the other users to use this SPI master.
 *            Please DO NOT call #hal_spi_master_init() in interrupt handler, it may cause deadlock.
 *            In a multi-task applications, if #hal_spi_master_init() returns error #HAL_SPI_MASTER_STATUS_ERROR_BUSY, it is suggested to call functions that release the CPU and try
 *            again later.
 * @par       Example
 *    @code
 *
 * hal_spi_master_status_t user_spi_master_init(void)
 * {
 *   hal_spi_master_port_t spi_port;
 *   hal_spi_master_config_t spi_config;
 *   hal_spi_master_status_t error_status;
 *   uint32_t try_times = 0;
 *
 *   spi_port = HAL_SPI_MASTER_0;
 *   spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
 *   spi_config.clock_frequency = 1000000;
 *   spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
 *   spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
 *
 *   while (try_times < 10) {
 *       error_status = hal_spi_master_init(spi_port, &spi_config);
 *       if (error_status == HAL_SPI_MASTER_STATUS_ERROR_BUSY) {
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
 * @sa #hal_spi_master_deinit()
 */
hal_spi_master_status_t hal_spi_master_init(hal_spi_master_port_t master_port,
        hal_spi_master_config_t *spi_config);

/**
 * @brief     This function resets the SPI master, gates its clock, disables interrupts.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT means master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_OK means this function return successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_init()
 */
hal_spi_master_status_t hal_spi_master_deinit(hal_spi_master_port_t master_port);

#ifdef HAL_SPI_MASTER_FEATURE_ADVANCED_CONFIG
/**
 * @brief     SPI master advanced configuration function.
 *            User can call this function to customize more settings for the SPI device operation.
 *            For more information about the settings, please refer to #hal_spi_master_advanced_config_t.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] advanced_config provides advanced configuration parameters for the SPI master. Details are described at #hal_spi_master_advanced_config_t.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, this port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @note      This function must be called after #hal_spi_master_init().
 * @par       Example
 * @code
 *       hal_spi_master_advanced_config_t advanced_config;
 *       advanced_config.byte_order = HAL_SPI_MASTER_BIG_ENDIAN;
 *       advanced_config.chip_polarity = HAL_SPI_MASTER_CHIP_SELECT_LOW;
 *       advanced_config.get_tick = HAL_SPI_MASTER_GET_TICK_DELAY1;
 *       advanced_config.sample_select = HAL_SPI_MASTER_SAMPLE_NEGATIVE;
 *       ret = hal_spi_master_set_advanced_config(HAL_SPI_MASTER_1, &advanced_config);
 * @endcode
 */
hal_spi_master_status_t hal_spi_master_set_advanced_config(hal_spi_master_port_t master_port,
        hal_spi_master_advanced_config_t *advanced_config);
#endif


/**
 * @brief     This function is used to send data synchronously with FIFO mode. This function doesn't return until the transfer is complete.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] data is the data buffer to send, this parameter cannot be NULL.
 * @param[in] size is the number of bytes to send. Note the user cannot send data size larger than #HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE bytes.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR, the byte_order parameter is not properly configured for the FIFO mode; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * @code
 *       ret = hal_spi_master_send_polling(HAL_SPI_MASTER_1,data,size);
 * @endcode
 * @sa #hal_spi_master_send_and_receive_polling()
 */
hal_spi_master_status_t hal_spi_master_send_polling(hal_spi_master_port_t master_port,
        uint8_t *data,
        uint32_t size);

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
 * @brief     This function is used to send data asynchronously with DMA mode. This function returns immediately, before calling this function,
 *            user should call #hal_spi_master_register_callback() to register a callback, then the callback
 *            will be called in the SPI ISR.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] data is the data buffer to send, this parameter cannot be NULL, also the address must be a non-cacheable and 4 bytes align address.
 * @param[in] size is the number of bytes to send.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_send_and_receive_dma()
 */
hal_spi_master_status_t hal_spi_master_send_dma(hal_spi_master_port_t master_port,
        uint8_t *data,
        uint32_t size);

/**
 * @brief     This function is used to send data synchronously with DMA mode. This function doesn't return until the transfer is complete.
 *            Normally this function is used in the scenario where the IRQ must be disabled.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] data is the data buffer to send, this parameter cannot be NULL, also the address must be a non-cacheable and 4 bytes align address.
 * @param[in] size is the number of bytes to send.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_send_and_receive_dma_blocking()
 */
hal_spi_master_status_t hal_spi_master_send_dma_blocking(hal_spi_master_port_t master_port,
        uint8_t *data,
        uint32_t size);
#endif

/**
 * @brief     This function simultaneously sends and receives data in the FIFO mode. This function doesn't return until the transfer is complete.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] spi_send_and_receive_config is the structure that contains data buffer and data size, please refer to #hal_spi_master_send_and_receive_config_t
 *            , for more details. Check the SPI device's datasheet to determine if the data is sent or received.
 *            For example, if <a href="http://www.analog.com/media/en/technical-documentation/data-sheets/ADIS16375.pdf"> ADIS16375 </a>
 *            is used in full duplex communication mode, the data is received, while send command is valid.
 *            If <a href="http://www.thaieasyelec.com/downloads/EFDV423/PAH8001EI-2G.pdf"> PAH8001EI-2G </a> sensor is used,
 *            the data is received, while send command is invalid.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR, the byte_order parameter is not properly configured for the FIFO mode; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter in spi_send_and_receive_config is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * @code
 *       hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;
 *       spi_send_and_receive_config.receive_buffer = receive_buffer;
 *       spi_send_and_receive_config.send_data = send_data;
 *       spi_send_and_receive_config.send_length = send_length;
 *       spi_send_and_receive_config.receive_length = receive_length;
 *       ret = hal_spi_master_send_and_receive_polling(HAL_SPI_MASTER_1, &spi_send_and_receive_config);
 * @endcode
 * @sa  #hal_spi_master_send_polling()
 */
hal_spi_master_status_t hal_spi_master_send_and_receive_polling(hal_spi_master_port_t master_port,
        hal_spi_master_send_and_receive_config_t *spi_send_and_receive_config);

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
 * @brief     This function is used to send and receive data asynchronously with DMA mode. This function returns immediately, before
 *            calling this function, call #hal_spi_master_register_callback() to register a callback, once the transaction
 *            is complete, the callback will be called in the SPI ISR.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] spi_send_and_receive_config is the structure that contains data buffer and data size, please refer to #hal_spi_master_send_and_receive_config_t
 *            , for more details. Check the SPI device's datasheet to determine if the data is sent or received.
 *            For example, if <a href="http://www.analog.com/media/en/technical-documentation/data-sheets/ADIS16375.pdf"> ADIS16375 </a>
 *            is used in full duplex communication mode, the data is received, while send command is valid.
 *            If <a href="http://www.thaieasyelec.com/downloads/EFDV423/PAH8001EI-2G.pdf"> PAH8001EI-2G </a> sensor is used,
 *            the data is received, while send command is invalid. The address parameters must be non-cacheable 4 bytes align and cannot be NULL.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter in spi_send_and_receive_config is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_send_dma()
 */
hal_spi_master_status_t hal_spi_master_send_and_receive_dma(hal_spi_master_port_t master_port,
        hal_spi_master_send_and_receive_config_t *spi_send_and_receive_config);

/**
 * @brief     This function simultaneously sends and receives data in the DMA mode. This function doesn't return until the transfer is complete.
 *            Normally this function is used in the scenario where the IRQ must be disabled.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] spi_send_and_receive_config is the structure that contains data buffer and data size, please refer to #hal_spi_master_send_and_receive_config_t
 *            , for more details. Check the SPI device's datasheet to determine if the data is sent or received.
 *            For example, if <a href="http://www.analog.com/media/en/technical-documentation/data-sheets/ADIS16375.pdf"> ADIS16375 </a>
 *            is used in full duplex communication mode, the data is received, while send command is valid.
 *            If <a href="http://www.thaieasyelec.com/downloads/EFDV423/PAH8001EI-2G.pdf"> PAH8001EI-2G </a> sensor is used,
 *            the data is received, while send command is invalid. The address parameters must be non-cacheable 4 bytes align and cannot be NULL.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter in spi_send_and_receive_config is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_send_dma_blocking()
 */
hal_spi_master_status_t hal_spi_master_send_and_receive_dma_blocking(hal_spi_master_port_t master_port,
        hal_spi_master_send_and_receive_config_t *spi_send_and_receive_config);
#endif

/**
 * @brief      This function gets current running status of the SPI master. Note this API can only be called after #hal_spi_master_init().
 * @param[in]  master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[out] running_status is the current running status.
 *             #HAL_SPI_MASTER_BUSY, the SPI master is in busy status; \n
 *             #HAL_SPI_MASTER_IDLE, the SPI master is in idle status, user can use it to transfer data now.
 * @return     #HAL_SPI_MASTER_STATUS_ERROR, this API is called before #hal_spi_master_init() is called; \n
 *             #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *             #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, running_status parameter is NULL; \n
 *             #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par        Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_get_running_status(hal_spi_master_port_t master_port,
        hal_spi_master_running_status_t *running_status);


#ifdef HAL_SPI_MASTER_FEATURE_CHIP_SELECT_TIMING
/**
 * @brief     This function is used to configure SPI master chip select timing parameter.
 *            User can call this function to customize chip select signal timing
 *            if the default SPI master chip select signal timing doesn't match SPI device's requirement.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] chip_select_timing is the parameter settings for chip select timing.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid chip select timing parameter is givenby the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @note      This function must be called after #hal_spi_master_init().
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_chip_select_timing(hal_spi_master_port_t master_port,
        hal_spi_master_chip_select_timing_t chip_select_timing);
#endif


#ifdef HAL_SPI_MASTER_FEATURE_DEASSERT_CONFIG
/**
 * @brief     SPI master chip select de-assertion mode configuration.
 *            User can call this function to enable the deassert feature if the SPI device
 *            requires switching the chip select signal from invalid to valid after each byte transfer.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] deassert is the parameter to set SPI master chip select signal behavior after sending one byte.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid deassert parameter is given; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @note      This function must be called after #hal_spi_master_init().
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_deassert(hal_spi_master_port_t master_port,
        hal_spi_master_deassert_t deassert);
#endif


#ifdef HAL_SPI_MASTER_FEATURE_MACRO_CONFIG
/**
 * @brief     SPI master macro group configuration. If the GPIO selected doesn't belong to SPI pad macro group A,
              user should call this function to config it to the right pad macro.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] macro_select is the parameter for macro group selection.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid macro selection parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @note      This function must be called after #hal_spi_master_init().
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_macro_selection(hal_spi_master_port_t master_port,
        hal_spi_master_macro_select_t macro_select);
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DMA_MODE
/**
 * @brief     This function is used to register user's callback to SPI master driver. This function should be called when user wants to use
 *            the DMA mode, the callback will be called in SPI interrupt service routine.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] callback is the callback function given by user, which will be called at SPI master interrupt service routine.
 * @param[in] user_data is a parameter given by user and will pass to user while the callback function is called. See the last parameter of #hal_spi_master_callback_t.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 * @sa #hal_spi_master_send_dma(), #hal_spi_master_send_and_receive_dma()
 */
hal_spi_master_status_t hal_spi_master_register_callback(hal_spi_master_port_t master_port,
        hal_spi_master_callback_t callback,
        void *user_data);
#endif

#ifdef HAL_SPI_MASTER_FEATURE_DUAL_QUAD_MODE
/**
 * @brief     SPI master mode configuration, can be config to single mode, dual mode or quad mode.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] mode is the parameter for mode configuration, the value is defined in #hal_spi_master_mode_t.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid macro selection parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_mode(hal_spi_master_port_t master_port,
        hal_spi_master_mode_t mode);

/**
 * @brief     SPI master dummy cycle configuration.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] dummy_bits is the parameter for dummy cycle configuration.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid macro selection parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_dummy_bits(hal_spi_master_port_t master_port,
        uint8_t dummy_bits);

/**
 * @brief     SPI master command cycle configuration, only valid when SPI master is configured in dual mode or quad mode.
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] command_bytes is the parameter for command cycle configuration.
 * @return    #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *            #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *            #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER, an invalid macro selection parameter is given by the user; \n
 *            #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par       Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_set_command_bytes(hal_spi_master_port_t master_port,
        uint8_t command_bytes);
#endif

#if defined(HAL_SPI_MASTER_FEATURE_NO_BUSY) && defined (HAL_SPI_MASTER_FEATURE_DMA_MODE)

/**
 * @brief     This API is a special API. User can call this API directly. It is not necessary to call hal_spi_master_init().
 * @param[in] master_port is the SPI master port number, the value is defined in #hal_spi_master_port_t.
 * @param[in] spi_send_to_receive_config_no_busy_ex is the structure that contains SPI master configurations, callback function, data buffer and data size,
 *             please refer to #hal_spi_send_and_receive_config_ex_no_busy_t
 * @return
 *         #HAL_SPI_MASTER_STATUS_ERROR_PORT, master_port parameter is an invalid port number; \n
 *        #HAL_SPI_MASTER_STATUS_INVALID_PARAMETER an invalid macro selection parameter is given by the user; \n
 *        #HAL_SPI_MASTER_STATUS_ERROR_BUSY, the port of SPI master is busy and not available for use; \n
 *         #HAL_SPI_STATUS_FULL_BY_NON_BLOCKING_FUNCTION, the software fifo of SPI master is full;\n
 *        #HAL_SPI_STATUS_ERROR_USED_BY_NORMAL_USER,the SPI master port is occupied by other users who call busy API;\n
 *        #HAL_SPI_MASTER_STATUS_OK, the operation completed successfully.
 * @par Example
 * Sample code please refers to @ref HAL_SPI_MASTER_Driver_Usage_Chapter.
 */
hal_spi_master_status_t hal_spi_master_send_and_receive_dma_no_busy(hal_spi_master_port_t master_port, hal_spi_send_and_receive_config_ex_no_busy_t *spi_send_to_receive_config_no_busy_ex);
#endif

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/
#endif /*HAL_SPI_MASTER_MODULE_ENABLED*/
#endif /* __HAL_SPI_MASTER_H__ */

