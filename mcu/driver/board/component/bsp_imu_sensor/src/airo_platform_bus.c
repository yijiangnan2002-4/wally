/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
#include "airo_platform_bus.h"
#include "bsp_imu_sensor_common.h"
#include "bsp_imu_sensor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*****************************************************************************
 Description:
     Below code if for porting bus driver for multi-axis sensor in airoha
    platform.

 History: 2019-02-20 first create
*****************************************************************************/
#define        MAX_PLATFORM_BUS_PORT    3

static platform_bus_private_info_t                          priv_info[BSP_IMU_SENSOR_BUS_TYPE_MAX][MAX_PLATFORM_BUS_PORT];
static ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  uint8_t    s_air_tx_buf[8]  = {0};
static ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  uint8_t    s_air_rx_buf[64] = {0};
static volatile bool                                        s_air_irq_triggered = false;
static volatile hal_i2c_callback_event_t                    s_air_irq_event = 0;
static platform_bus_init_config_t                           s_air_init_config;

static void  airo_bus_i2c_callback(uint8_t slave_address, hal_i2c_callback_event_t event,  void *user_data)
{
    platform_bus_private_info_t *priv = (platform_bus_private_info_t *)user_data;

    if (event == HAL_I2C_EVENT_SUCCESS) {
        if (priv != NULL) {
            priv->call_back(priv->user_data);
        } else {
            log_sensor_bus_error("[axis][bus] airo_bus_i2c_callback fail: callback is null\r\n", 0);
        }
    } else {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_callback fail: event error %d\r\n", 1, event);
    }
}


static void airo_bus_i2c_polling_callback(uint8_t slave_address, hal_i2c_callback_event_t event,  void *user_data)
{
    s_air_irq_triggered = true;
    s_air_irq_event     = event;
}

static  int airo_bus_i2c_wait_completed(uint32_t timeout_us)
{
    uint32_t tick_pre,tick_cur,tick_dur;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,  &tick_pre);

    while(1) {
        if(s_air_irq_triggered == true) {
            return s_air_irq_event;
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,  &tick_cur);
        hal_gpt_get_duration_count(tick_pre, tick_cur, &tick_dur);
        if(tick_dur > timeout_us){
            return -1;
        }
    }
}

//=========================================================================================================================

static int airo_bus_i2c_init(hal_i2c_port_t port, hal_i2c_config_t *config)
{
    return 0;
}

static int airo_bus_i2c_deinit(hal_i2c_port_t i2c_port)
{
    return 0;
}


static int airo_bus_i2c_write(hal_i2c_port_t port, platform_transfer_config_t *config)
{
    hal_i2c_status_t status;
    hal_i2c_send_to_receive_config_ex_no_busy_t tx_rx_cfg;

    log_sensor_bus_info("[axis][bus] airo_bus_i2c_write(), i2c%d read reg %x", 2, (unsigned int)port, (unsigned int)config->reg_addr);
    if (config == NULL || config->size > 7) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_write fail : invalid para\r\n", 0);
        return -1;
    }
    memset(&tx_rx_cfg, 0, sizeof(tx_rx_cfg));
    s_air_tx_buf[0] = config->reg_addr;
    memcpy(&s_air_tx_buf[1], config->buff, config->size);

    s_air_irq_triggered    = false;
    tx_rx_cfg.i2c_config   = s_air_init_config.i2c_config;
    tx_rx_cfg.i2c_callback = airo_bus_i2c_polling_callback;
    tx_rx_cfg.user_data    = NULL;

    tx_rx_cfg.i2c_send_to_receive_config_ex.slave_address  = config->dev_addr;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_buffer = s_air_rx_buf;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = 0;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_packet_length = 0;

    tx_rx_cfg.i2c_send_to_receive_config_ex.send_data = s_air_tx_buf;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_bytes_in_one_packet = 1 + config->size;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_packet_length = 1;

    status = hal_i2c_master_send_to_receive_dma_ex_none_blocking(port, &tx_rx_cfg);
    if (status != 0) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_write fail 1: error %d\r\n", 1, status);
        return status;
    }
    status = airo_bus_i2c_wait_completed(10000);
    if(status != 0) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_write fail 2: error %d\r\n", 1, status);
    }
    return status;
}



static int airo_bus_i2c_read(hal_i2c_port_t port, platform_transfer_config_t *config)
{
    hal_i2c_status_t status;
    hal_i2c_send_to_receive_config_ex_no_busy_t tx_rx_cfg;

    log_sensor_bus_info("[axis][bus] airo_bus_i2c_read(), i2c%d read reg %x", 2, (unsigned int)port, (unsigned int)config->reg_addr);
    if (config == NULL) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_read fail : para is null\r\n", 0);
        return -1;
    }
    memset(&tx_rx_cfg, 0, sizeof(tx_rx_cfg));

    s_air_tx_buf[0] = config->reg_addr;
    s_air_irq_triggered    = false;
    tx_rx_cfg.i2c_config   = s_air_init_config.i2c_config;
    tx_rx_cfg.i2c_callback = airo_bus_i2c_polling_callback;
    tx_rx_cfg.user_data    = NULL;

    tx_rx_cfg.i2c_send_to_receive_config_ex.slave_address  = config->dev_addr;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_buffer = s_air_rx_buf;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = config->size;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_packet_length = 1;

    tx_rx_cfg.i2c_send_to_receive_config_ex.send_data = s_air_tx_buf;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_bytes_in_one_packet = 1;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_packet_length = 1;

    status = hal_i2c_master_send_to_receive_dma_ex_none_blocking(port, &tx_rx_cfg);
    if (status != 0) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_read fail 1: error %d\r\n", 1, status);
        return status;
    }
    status = airo_bus_i2c_wait_completed(10000);
    if(status == 0) {
        memcpy(config->buff, s_air_rx_buf, config->size);
    } else {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_read fail 2: error %d\r\n", 1, status);
    }
    return status;
}



static int32_t airo_bus_i2c_read_dma(hal_i2c_port_t port, platform_transfer_dma_config_t *config)
{
    hal_i2c_status_t status;
    hal_i2c_send_to_receive_config_ex_no_busy_t tx_rx_cfg;
    platform_bus_private_info_t *pinfo;

    log_sensor_bus_info("[axis][bus] airo_bus_i2c_read_dma(), i2c%d read reg %x", 2, (unsigned int)port, (unsigned int)config->reg_addr);
    if (config->buff == NULL || config->size > 4096) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_read_dma fail : invalid para\r\n", 0);
        return -1;
    }
    memset(&tx_rx_cfg, 0, sizeof(tx_rx_cfg));
    s_air_irq_triggered    = false;
    pinfo = &priv_info[BSP_IMU_SENSOR_BUS_TYPE_I2C][port];
    s_air_tx_buf[0]        = config->reg_addr;
    pinfo->call_back       = config->call_back;
    pinfo->user_data       = config->user_data;

    tx_rx_cfg.i2c_config   = s_air_init_config.i2c_config;
    tx_rx_cfg.i2c_callback = airo_bus_i2c_callback;
    tx_rx_cfg.user_data    = pinfo;

    tx_rx_cfg.i2c_send_to_receive_config_ex.slave_address  = config->dev_addr;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_buffer = config->buff;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = config->size;
    tx_rx_cfg.i2c_send_to_receive_config_ex.receive_packet_length = 1;

    tx_rx_cfg.i2c_send_to_receive_config_ex.send_data = s_air_tx_buf;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_bytes_in_one_packet = 1;
    tx_rx_cfg.i2c_send_to_receive_config_ex.send_packet_length = 1;

    status = hal_i2c_master_send_to_receive_dma_ex_none_blocking(port, &tx_rx_cfg);
    if (status != 0) {
        log_sensor_bus_error("[axis][bus] airo_bus_i2c_write fail: send error %d\r\n", 1, status);
    }
    return status;
}


//-------------------------------------------------------------------------------------------------------------------

void airo_bus_spi_callback(hal_spi_master_callback_event_t event, void *user_data)
{
    platform_bus_private_info_t *priv = (platform_bus_private_info_t *)user_data;

    switch (event) {
        case HAL_SPI_MASTER_EVENT_SEND_FINISHED:
        case HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED: {
            if (priv != NULL) {
                priv->call_back(priv->user_data);
            } else {
                log_sensor_bus_error("[axis][bus] airo_bus_spi_callback fail: callback is null\r\n", 0);
            }
        }
        break;
        default:
            log_sensor_bus_error("[bsp][sensor] airo_bus_spi_callback fail: event error %d\r\n", 1, event);
            break;
    }
}



int airo_bus_spi_init(hal_spi_master_port_t master_port, hal_spi_master_config_t *spi_config)
{
    hal_spi_master_chip_select_timing_t spi_master_chip_select_timing;
    hal_spi_master_status_t spim_status = HAL_SPI_MASTER_STATUS_OK;

    spi_master_chip_select_timing.chip_select_setup_count = 3;
    spi_master_chip_select_timing.chip_select_hold_count = 4;
    spi_master_chip_select_timing.chip_select_idle_count = 5;

    spim_status = hal_spi_master_init(master_port, spi_config);
    if (HAL_SPI_MASTER_STATUS_OK != spim_status) {
        log_sensor_bus_error("[axis][bus] SPI initialize failed, spim_status=%d\r\n", 1, spim_status);
    }
    if (HAL_SPI_MASTER_STATUS_OK != hal_spi_master_set_chip_select_timing(master_port, spi_master_chip_select_timing)) {
        log_sensor_bus_error("[axis][bus] SPI set_chip_select_timing failed\r\n" , 0);
    }

    return (int)spim_status;
}


int airo_bus_spi_write(hal_spi_master_port_t master_port, platform_transfer_config_t *config)
{
    int32_t ierror = 0;
    uint8_t array[9];
    uint8_t stringpos = 0;

    array[0] = config->reg_addr;
    for (stringpos = 0; stringpos < config->size; stringpos++)
        array[stringpos + 1]
            = *(config->buff + stringpos);

    ierror = hal_spi_master_send_polling(master_port, array, config->size + 1);
    if (HAL_SPI_MASTER_STATUS_OK != ierror) {
        log_sensor_bus_error("[axis][bus] ierror = %d, hal_spi_master_send_polling failed\r\n", 1, (int)ierror);
        hal_gpt_delay_ms(1);
    }
    return (int)ierror;
}


int airo_bus_spi_read(hal_spi_master_port_t master_port, platform_transfer_config_t *config)
{
    hal_spi_master_status_t ierror = 0;
    hal_spi_master_send_and_receive_config_t spi_master_send_and_receive_config;
    uint8_t recv_data[8] = {0};
    uint8_t send_data[1] = {0};

    send_data[0] = config->reg_addr | 0x80;
    spi_master_send_and_receive_config.send_data = send_data;
    spi_master_send_and_receive_config.send_length = 1;
    spi_master_send_and_receive_config.receive_buffer = recv_data;
    spi_master_send_and_receive_config.receive_length = 1 + config->size;
    ierror = hal_spi_master_send_and_receive_polling(master_port,  &spi_master_send_and_receive_config);
    if (HAL_SPI_MASTER_STATUS_OK == ierror) {
        memcpy(config->buff, &recv_data[1], config->size);
    } else {
        log_sensor_bus_error("[axis][bus] ierror = %d,hal_spi_master_send_and_receive failed\r\n", 1, ierror);
        hal_gpt_delay_ms(1);
    }
    return (int)ierror;

}

int airo_bus_spi_read_dma(hal_spi_master_port_t master_port, platform_transfer_dma_config_t *config)
{
    hal_spi_master_status_t ierror = 0;
    hal_spi_master_send_and_receive_config_t spi_master_send_and_receive_config;
    static ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN uint8_t send_data[1] = {0};

    send_data[0] = config->reg_addr | 0x80;
    spi_master_send_and_receive_config.send_data = send_data;
    spi_master_send_and_receive_config.send_length = 1;
    spi_master_send_and_receive_config.receive_buffer = config->buff;
    spi_master_send_and_receive_config.receive_length = 1 + config->size;
    priv_info[BSP_IMU_SENSOR_BUS_TYPE_SPI][master_port].call_back = config->call_back;
    priv_info[BSP_IMU_SENSOR_BUS_TYPE_SPI][master_port].user_data = config->user_data;
    hal_spi_master_register_callback(master_port, airo_bus_spi_callback, &priv_info[BSP_IMU_SENSOR_BUS_TYPE_SPI][master_port]);
    ierror = hal_spi_master_send_and_receive_dma(master_port, &spi_master_send_and_receive_config);
    return (int)ierror;
}

int airo_bus_spi_deinit(hal_spi_master_port_t master_port)
{
    return (int)hal_spi_master_deinit(master_port);
}


/*****************************************************************************
 Description:
    Below function is the public api for user using, it can implement with
    different platform, now it only include the airoha platform driver.

 History: 2021-05-28 first create
*****************************************************************************/

int platform_bus_init(uint32_t handle, platform_bus_init_config_t *config)
{
    static bool    initialized = false;
    uint8_t         port = 0;
    int             status = -1;

    if (initialized == false) {
        memset(priv_info, 0, sizeof(priv_info));
        initialized = true;
    }
    s_air_init_config = *config;
    log_sensor_bus_info("[axis][bus] platform_bus_init(), handle %x", 1, (unsigned int)handle);
    port = BSP_SENSOR_GET_BUS_PORT(handle);
    switch (BSP_SENSOR_GET_BUS_TYPE(handle)) {
        case BSP_IMU_SENSOR_BUS_TYPE_I2C:
            status = airo_bus_i2c_init(port, &config->i2c_config);
            break;
        case BSP_IMU_SENSOR_BUS_TYPE_SPI:
            status = airo_bus_spi_init(port, &config->spi_config);
            break;
        default:    {
            log_sensor_bus_error("[axis][bus] platform_bus_init() fail, err bustype", 0);
        }
        break;
    }
    return status;
}


int platform_bus_read(uint32_t handle, platform_transfer_config_t *config)
{
    uint8_t port = 0;
    int     status = -1;

    port = BSP_SENSOR_GET_BUS_PORT(handle);
    switch (BSP_SENSOR_GET_BUS_TYPE(handle)) {
        case BSP_IMU_SENSOR_BUS_TYPE_I2C:
            status = airo_bus_i2c_read(port, config);
            break;
        case BSP_IMU_SENSOR_BUS_TYPE_SPI:
            status = airo_bus_spi_read(port, config);
            break;
        default: {
            log_sensor_bus_error("[axis][bus] platform_bus_read() fail, err bustype", 0);
        }
        break;
    }
    return status;
}

int platform_bus_write(uint32_t handle, platform_transfer_config_t *config)
{
    uint8_t port = 0;
    int     status = -1;

    port = BSP_SENSOR_GET_BUS_PORT(handle);
    switch (BSP_SENSOR_GET_BUS_TYPE(handle)) {
        case BSP_IMU_SENSOR_BUS_TYPE_I2C:
            status = airo_bus_i2c_write(port, config);
            break;
        case BSP_IMU_SENSOR_BUS_TYPE_SPI:
            status = airo_bus_spi_write(port, config);
            break;
        default: {
            log_sensor_bus_error("[axis][bus] platform_bus_write() fail, err bustype", 0);
        }
        break;

    }
    return status;
}


int platform_bus_read_dma(uint32_t handle, platform_transfer_dma_config_t *config)
{
    uint8_t port = 0;
    int     status = -1;

    port = BSP_SENSOR_GET_BUS_PORT(handle);
    switch (BSP_SENSOR_GET_BUS_TYPE(handle)) {
        case BSP_IMU_SENSOR_BUS_TYPE_I2C:
            status = airo_bus_i2c_read_dma(port, config);
            break;
        case BSP_IMU_SENSOR_BUS_TYPE_SPI:
            status = airo_bus_spi_read_dma(port, config);
            break;
        default: {
            log_sensor_bus_error("[axis][bus] platform_bus_read_dma() fail, err bustype", 0);
        }
        break;

    }
    return status;
}

int platform_bus_write_dma(uint32_t handle, platform_transfer_dma_config_t *config)
{
    return 0;
}


int platform_bus_deinit(uint32_t handle)
{
    uint8_t port = 0;
    int     status = -1;

    port = BSP_SENSOR_GET_BUS_PORT(handle);
    switch (BSP_SENSOR_GET_BUS_TYPE(handle)) {
        case BSP_IMU_SENSOR_BUS_TYPE_I2C:
            status = airo_bus_i2c_deinit(port);
            break;
        case BSP_IMU_SENSOR_BUS_TYPE_SPI:
            status = airo_bus_spi_deinit(port);
            break;
        default: {
            log_sensor_bus_error("[axis][bus] platform_bus_deinit() fail, err bustype", 0);
        }
        break;

    }
    return status;
}





