/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
#include "hal.h"
#include "hal_platform.h"
#include "hal_i2c_master.h"
#if defined(HAL_I2C_MASTER_MODULE_ENABLED)
#include "hal_clock.h"
#include "hal_i2c_master_internal.h"

/******************************************************************************
 * Pre Processor Definitions
*******************************************************************************/
#define     I2C_POLLING_WAIT_TIME_US        10000 //10ms

/******************************************************************************
 * Private Variable Definitions
*******************************************************************************/
static i2c_private_info_t           s_priv_info[HAL_I2C_MASTER_MAX] = {0};
static i2c_transfer_config_t        s_i2c1_q_buf[HAL_I2C_MASTER_1_FIFO_DEPTH] = {0};
ATTR_RODATA_IN_RAM_FOR_MASK_IRQ static i2c_transfer_config_t       * const s_i2c_q_buf[HAL_I2C_MASTER_MAX] = {(i2c_transfer_config_t *) NULL, s_i2c1_q_buf, (i2c_transfer_config_t *) NULL, (i2c_transfer_config_t *) NULL};
ATTR_RODATA_IN_RAM_FOR_MASK_IRQ static const uint8_t                s_i2c_q_szb[HAL_I2C_MASTER_MAX] = {0, HAL_I2C_MASTER_1_FIFO_DEPTH, 0, 0};
static bool                         s_initialized = false;


/******************************************************************************
 * Private Funtion Definitions
*******************************************************************************/
static void                 _hal_i2c_interrupt_handle(hal_nvic_irq_t irq_number);
static hal_i2c_status_t     _hal_i2c_config_transfer(hal_i2c_port_t i2c_port, i2c_transfer_config_t *config);
static hal_i2c_status_t     _hal_i2c_config_transfer_poll(hal_i2c_port_t i2c_port, i2c_transfer_config_t *config);
static hal_i2c_status_t     _hal_i2c_master_send_and_receive_dma_with_nb(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_ex_no_busy_t *config, bool is_non_block);

/******************************************************************************
 * Private Funtion Definitions
*******************************************************************************/

#ifdef HAL_SLEEP_MANAGER_ENABLED
static  void    hal_i2c_backup_all_register()
{
    i2c_private_info_t     *priv_info = NULL;
    uint32_t                i2c_port = 0;

    for (i2c_port = HAL_I2C_MASTER_1; i2c_port <= HAL_I2C_MASTER_1; i2c_port++) {
        priv_info  = &s_priv_info[i2c_port];
        if (priv_info->hw_state & I2C_HW_STATE_USED_MASK) {
            i2c_sleep_register_backup(i2c_port, (uint32_t *) &priv_info->i2c_queue.data, MAX_FIFO_SIZE);
        }
    }
}

static void  hal_i2c_restore_all_register()
{
    i2c_private_info_t     *priv_info = NULL;
    uint32_t                i2c_port = 0;

    for (i2c_port = HAL_I2C_MASTER_1; i2c_port <= HAL_I2C_MASTER_1; i2c_port++) {
        priv_info  = &s_priv_info[i2c_port];
        if (priv_info->hw_state & I2C_HW_STATE_USED_MASK) {
            i2c_sleep_register_restore(i2c_port, (uint32_t *) &priv_info->i2c_queue.data, MAX_FIFO_SIZE);
        }
        memset(&priv_info->i2c_queue, 0, sizeof(airo_queue_t));
    }
}
#endif

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ   hal_i2c_status_t    hal_i2c_master_init_internal(hal_i2c_port_t i2c_port)
{
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (s_priv_info[i2c_port].hw_state & I2C_HW_STATE_USED_MASK) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return HAL_I2C_STATUS_ERROR;
    }
    s_priv_info[i2c_port].hw_state = (I2C_HW_STATE_USED_NORMAL << I2C_HW_STATE_USED_OFFSET);
    hal_nvic_restore_interrupt_mask(saved_mask);
    return HAL_I2C_STATUS_OK;
}

hal_i2c_status_t    hal_i2c_master_init(hal_i2c_port_t i2c_port, hal_i2c_config_t *i2c_config)
{
    if (s_initialized == false) {
        memset(s_priv_info, 0, sizeof(s_priv_info));
        s_initialized = true;
    }
    /* parameter check */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        log_hal_i2c_error("[hal][i2c%d] init: port invalid", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    if (HAL_I2C_FREQUENCY_MAX <= i2c_config->frequency) {
        log_hal_i2c_error("[hal][i2c%d] init: para invalid", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PARAMETER;
    }
    if(hal_i2c_master_init_internal(i2c_port) != HAL_I2C_STATUS_OK)
    {
        log_hal_i2c_error("[hal][i2c%d] init: dev busy", 1, i2c_port);
        return HAL_I2C_STATUS_ERROR;
    }

    i2c_op_ioctl(i2c_port, I2C_IOCTRL_SET_INTR_MASK, 0);
    i2c_config_io(i2c_port, true);
    i2c_config_speed(i2c_port, i2c_speed_enum_to_dec(i2c_config->frequency), 0);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_suspend_callback(SLEEP_BACKUP_RESTORE_I2C1, (sleep_management_suspend_callback_t)hal_i2c_backup_all_register, NULL);
    sleep_management_register_resume_callback(SLEEP_BACKUP_RESTORE_I2C1, (sleep_management_resume_callback_t) hal_i2c_restore_all_register, NULL);
#endif
    queue_init(&(s_priv_info[i2c_port].i2c_queue));
    return HAL_I2C_STATUS_OK;
}


hal_i2c_status_t hal_i2c_master_set_io_config(hal_i2c_port_t i2c_port, hal_i2c_io_config_t io)
{
    i2c_private_info_t     *priv_info = NULL;

    /* parameter check */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }

    priv_info  = &s_priv_info[i2c_port];
    priv_info->hw_state &= ~(I2C_HW_STATE_IO_CFG_MASK);
    if (io == HAL_I2C_IO_OPEN_DRAIN) {
        i2c_config_io(i2c_port, true);
    } else {
        i2c_config_io(i2c_port, false);
        priv_info->hw_state |= I2C_HW_STATE_IO_CFG_MASK;
    }
    return HAL_I2C_STATUS_OK;
}


hal_i2c_status_t hal_i2c_master_register_callback(hal_i2c_port_t i2c_port, hal_i2c_callback_t i2c_callback, void *user_data)
{
    i2c_private_info_t     *priv_info = NULL;

    /* parameter check */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    priv_info  = &s_priv_info[i2c_port];
    priv_info->call_back = i2c_callback;
    priv_info->user_data = user_data;
    return HAL_I2C_STATUS_OK;
}


hal_i2c_status_t    hal_i2c_master_deinit(hal_i2c_port_t i2c_port)
{
    /* parameter check */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }

    /* clear status & fifo */
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_CLR_IRQ_STAT, 0);
    i2c_config_fifo(i2c_port, I2C_FIFO_OP_CLR, 0, 0);
    s_priv_info[i2c_port].hw_state = 0;
    return HAL_I2C_STATUS_OK;
}


/***********************************************************************************************
 * HAL I2C POLLING FUNCTION  IMPLEMENT
***********************************************************************************************/
hal_i2c_status_t    hal_i2c_master_send_to_receive_polling_internal_init(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_t *i2c_send_to_receive_config, i2c_transfer_config_t *config)
{
    /* step 0: parameter check & init para */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        log_hal_i2c_error("[hal][i2c%d][poll]:0 invalid port\r\n", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    if (NULL == i2c_send_to_receive_config) {
        log_hal_i2c_error("[hal][i2c%d][poll]:0 invalid port\r\n", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PARAMETER;
    }
    if ((s_priv_info[i2c_port].hw_state & I2C_HW_STATE_USED_MASK) != I2C_HW_STATE_USED_NORMAL) {
        log_hal_i2c_error("[hal][i2c%d][poll]: not initialized\r\n", 1, i2c_port);
        return HAL_I2C_STATUS_ERROR;
    }
    memset(config, 0, sizeof(*config));
    config->speed      = HAL_I2C_FREQUENCY_MAX;
    config->slv_addr   = i2c_send_to_receive_config->slave_address;
    config->trans_mode = I2C_TRANSFER_MODE_FIFO;
    config->send_buff  = (uint8_t *)i2c_send_to_receive_config->send_data;
    config->send_size  = i2c_send_to_receive_config->send_length;
    config->recv_buff  = i2c_send_to_receive_config->receive_buffer;
    config->recv_size  = i2c_send_to_receive_config->receive_length;
    config->recv_pack_size = 1;
    config->send_pack_size = 1;
    if (config->recv_size == 0 && config->send_size == 0) {
        log_hal_i2c_error("[hal][i2c%d][poll]:1 invalid parameter\r\n", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PARAMETER;
    }

    if (config->recv_size && config->send_size) {
        config->trans_type = I2C_TRANSFER_TYPE_TX_RX;
    } else if (config->recv_size == 0) {
        config->trans_type = I2C_TRANSFER_TYPE_TX;
    } else {
        config->trans_type = I2C_TRANSFER_TYPE_RX;
    }
    
    return HAL_I2C_STATUS_OK;
}

hal_i2c_status_t    hal_i2c_master_send_to_receive_polling_internal_end(hal_i2c_port_t i2c_port, i2c_irq_status_t status)
{
    if (status != I2C_IRQ_STAT_SUCCESS) {
        log_hal_msgid_error("[airo][i2c%d][poll]: error %d, intr stat 0x%x \r\n", 3, i2c_port, status, i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_IRQ_STAT, 0));
        hal_i2c_master_dump(i2c_port);
        return HAL_I2C_STATUS_ERROR;
    } else {
        return HAL_I2C_STATUS_OK;
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ   hal_i2c_status_t    hal_i2c_master_send_to_receive_polling(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_t *i2c_send_to_receive_config)
{
    i2c_transfer_config_t   config;
    uint32_t save_mask;
    i2c_irq_status_t status;
    uint32_t  temp;
    hal_i2c_status_t internal_status;

    internal_status = hal_i2c_master_send_to_receive_polling_internal_init(i2c_port, i2c_send_to_receive_config, &config);
    if(internal_status != HAL_I2C_STATUS_OK)
    {
        return internal_status;
    }

    /* step 1: wait i2c hw idle */
    while (1) {
        if (i2c_wait_idle_with_timeout(i2c_port, I2C_POLLING_WAIT_TIME_US) < 0) {
            log_hal_i2c_error("[hal][i2c%d][poll]:0 wait hw idle timeout\r\n", 1, i2c_port);
            hal_i2c_master_dump(i2c_port);
            return HAL_I2C_STATUS_ERROR_BUSY;
        }
        hal_nvic_save_and_set_interrupt_mask(&save_mask);
        if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
            hal_nvic_restore_interrupt_mask(save_mask);
        } else {
            break;
        }
    }

    /* step 2: help read data and status if other had use hw */
    if (queue_pop(&(s_priv_info[i2c_port].i2c_queue), (int *)&temp) >= 0) {
        i2c_transfer_config_t   *pcfg;

        pcfg = (i2c_transfer_config_t *)temp;
        if (pcfg != NULL) {
            if (pcfg->trans_type == I2C_TRANSFER_TYPE_RX || pcfg->trans_type == I2C_TRANSFER_TYPE_TX_RX) {
                i2c_config_fifo(i2c_port, I2C_FIFO_OP_RD, pcfg->recv_buff, pcfg->recv_size);
            }
            pcfg->priv_data = i2c_get_irq_status(i2c_port) - 1;
        }
    }
    queue_push(&(s_priv_info[i2c_port].i2c_queue), (int)&config);

    /* step 3: config i2c hw */
    _hal_i2c_config_transfer_poll(i2c_port, &config);
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_START, 0);
    hal_nvic_restore_interrupt_mask(save_mask);

    /* step 4: wait i2c hw idle */
    i2c_wait_idle_with_timeout(i2c_port, I2C_POLLING_WAIT_TIME_US);
    hal_nvic_save_and_set_interrupt_mask(&save_mask);
#if 0
    while (1) {
        if (i2c_wait_idle_with_timeout(i2c_port, I2C_POLLING_WAIT_TIME_US) < 0) {
            log_hal_i2c_error("[hal][i2c%d][poll]:1 wait hw idle timeout\r\n", 1, i2c_port);
            hal_i2c_master_dump(i2c_port);
            break;
        }
        hal_nvic_save_and_set_interrupt_mask(&save_mask);
        if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
            hal_nvic_restore_interrupt_mask(save_mask);
        } else {
            break;
        }
    }
#endif
    /* step 5: get status or read fifo */
    temp = i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_IRQ_STAT, 0);
    if (config.priv_data == 0) {
        status = i2c_get_irq_status(i2c_port);
        if (status == I2C_IRQ_STAT_SUCCESS && (config.trans_type == I2C_TRANSFER_TYPE_RX || config.trans_type == I2C_TRANSFER_TYPE_TX_RX)) {
            i2c_config_fifo(i2c_port, I2C_FIFO_OP_RD, config.recv_buff, config.recv_size);
        }
        queue_pop(&(s_priv_info[i2c_port].i2c_queue), NULL);
    } else {
        status = config.priv_data + 1;
    }
    hal_nvic_restore_interrupt_mask(save_mask);
    internal_status = hal_i2c_master_send_to_receive_polling_internal_end(i2c_port, status);

    return internal_status;

}




hal_i2c_status_t    hal_i2c_master_send_polling(hal_i2c_port_t i2c_port, uint8_t slave_address, const uint8_t *data, uint32_t size)
{
    hal_i2c_send_to_receive_config_t i2c_snd_recv_cfg;

    memset(&i2c_snd_recv_cfg, 0, sizeof(i2c_snd_recv_cfg));
    i2c_snd_recv_cfg.send_data     = data;
    i2c_snd_recv_cfg.send_length   = size;
    i2c_snd_recv_cfg.slave_address = slave_address;

    return hal_i2c_master_send_to_receive_polling(i2c_port, &i2c_snd_recv_cfg);
}


hal_i2c_status_t    hal_i2c_master_receive_polling(hal_i2c_port_t i2c_port, uint8_t slave_address, uint8_t *buffer, uint32_t size)
{
    hal_i2c_send_to_receive_config_t i2c_snd_recv_cfg;

    memset(&i2c_snd_recv_cfg, 0, sizeof(i2c_snd_recv_cfg));
    i2c_snd_recv_cfg.receive_buffer = buffer;
    i2c_snd_recv_cfg.receive_length = size;
    i2c_snd_recv_cfg.slave_address  = slave_address;

    return hal_i2c_master_send_to_receive_polling(i2c_port, &i2c_snd_recv_cfg);
}


/***********************************************************************************************
 * HAL I2C DMA FUNCTION  IMPLEMENT
 *
***********************************************************************************************/
hal_i2c_status_t hal_i2c_master_send_to_receive_dma(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_t *i2c_send_to_recv_config)
{
    hal_i2c_send_to_receive_config_ex_no_busy_t    config;

    if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
        return HAL_I2C_STATUS_ERROR_BUSY;
    }

    memset(&config, 0, sizeof(config));
    config.i2c_callback = NULL;
    config.i2c_config.frequency = HAL_I2C_FREQUENCY_MAX;
    config.i2c_send_to_receive_config_ex.receive_buffer             = i2c_send_to_recv_config->receive_buffer;
    config.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = i2c_send_to_recv_config->receive_length;
    config.i2c_send_to_receive_config_ex.receive_packet_length      = 1;
    config.i2c_send_to_receive_config_ex.send_data                  = i2c_send_to_recv_config->send_data;
    config.i2c_send_to_receive_config_ex.send_bytes_in_one_packet   = i2c_send_to_recv_config->send_length;
    config.i2c_send_to_receive_config_ex.send_packet_length         = 1;
    config.i2c_send_to_receive_config_ex.slave_address              = i2c_send_to_recv_config->slave_address;
    config.user_data = NULL;

    return _hal_i2c_master_send_and_receive_dma_with_nb(i2c_port, &config, false);
}

hal_i2c_status_t hal_i2c_master_send_dma(hal_i2c_port_t i2c_port, uint8_t slave_address, const uint8_t *data, uint32_t size)
{
    hal_i2c_send_to_receive_config_t    config;

    config.receive_buffer = NULL;
    config.receive_length = 0;
    config.send_data      = data;
    config.send_length    = size;
    config.slave_address  = slave_address;

    return hal_i2c_master_send_to_receive_dma(i2c_port, &config);
}

hal_i2c_status_t hal_i2c_master_receive_dma(hal_i2c_port_t i2c_port, uint8_t slave_address, uint8_t  *buffer, uint32_t size)
{
    hal_i2c_send_to_receive_config_t    config;

    config.receive_buffer = buffer;
    config.receive_length = size;
    config.send_data      = NULL;
    config.send_length    = 0;
    config.slave_address  = slave_address;

    return hal_i2c_master_send_to_receive_dma(i2c_port, &config);
}


#ifdef HAL_I2C_MASTER_FEATURE_EXTENDED_DMA
/***********************************************************************************************
 * HAL I2C DMA EXTEND FUNCTION  IMPLEMENT
 *
***********************************************************************************************/
hal_i2c_status_t hal_i2c_master_send_to_receive_dma_ex(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_ex_t *i2c_send_to_rcv_config_ex)
{
    hal_i2c_send_to_receive_config_ex_no_busy_t    config;

    if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
        return HAL_I2C_STATUS_ERROR_BUSY;
    }
    memset(&config, 0, sizeof(config));
    config.i2c_callback = NULL;
    config.i2c_config.frequency = HAL_I2C_FREQUENCY_MAX;
    config.i2c_send_to_receive_config_ex.receive_buffer             = i2c_send_to_rcv_config_ex->receive_buffer;
    config.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = i2c_send_to_rcv_config_ex->receive_bytes_in_one_packet;
    config.i2c_send_to_receive_config_ex.receive_packet_length      = i2c_send_to_rcv_config_ex->receive_packet_length;
    config.i2c_send_to_receive_config_ex.send_data                  = i2c_send_to_rcv_config_ex->send_data;
    config.i2c_send_to_receive_config_ex.send_bytes_in_one_packet   = i2c_send_to_rcv_config_ex->send_bytes_in_one_packet;
    config.i2c_send_to_receive_config_ex.send_packet_length         = i2c_send_to_rcv_config_ex->send_packet_length;
    config.i2c_send_to_receive_config_ex.slave_address              = i2c_send_to_rcv_config_ex->slave_address;
    config.user_data = NULL;

    return _hal_i2c_master_send_and_receive_dma_with_nb(i2c_port, &config, false);
}

hal_i2c_status_t hal_i2c_master_send_dma_ex(hal_i2c_port_t i2c_port, hal_i2c_send_config_t *i2c_send_config)
{
    hal_i2c_send_to_receive_config_ex_t config;

    config.slave_address = i2c_send_config->slave_address;

    config.receive_buffer = NULL;
    config.receive_bytes_in_one_packet = 0;
    config.receive_packet_length = 0;

    config.send_data = i2c_send_config->send_data;
    config.send_bytes_in_one_packet = i2c_send_config->send_bytes_in_one_packet;
    config.send_packet_length = i2c_send_config->send_packet_length;

    return hal_i2c_master_send_to_receive_dma_ex(i2c_port, &config);
}

hal_i2c_status_t hal_i2c_master_receive_dma_ex(hal_i2c_port_t i2c_port, hal_i2c_receive_config_t *i2c_receive_config)
{
    hal_i2c_send_to_receive_config_ex_t config;

    config.slave_address = i2c_receive_config->slave_address;

    config.receive_buffer = i2c_receive_config->receive_buffer;
    config.receive_bytes_in_one_packet = i2c_receive_config->receive_bytes_in_one_packet;
    config.receive_packet_length = i2c_receive_config->receive_packet_length;

    config.send_data = NULL;
    config.send_bytes_in_one_packet = 0;
    config.send_packet_length = 0;

    return hal_i2c_master_send_to_receive_dma_ex(i2c_port, &config);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ hal_i2c_status_t hal_i2c_master_send_to_receive_dma_ex_none_blocking_internal(i2c_private_info_t *priv_info)
{
    uint32_t temp;
    uint32_t save_mask;

    hal_nvic_save_and_set_interrupt_mask(&save_mask);
    temp = priv_info->hw_state & I2C_HW_STATE_USED_MASK;
    if ((temp == I2C_HW_STATE_USED_NORMAL) || (priv_info->hw_state & I2C_HW_STATE_IN_DEINIT_MASK)) {
        hal_nvic_restore_interrupt_mask(save_mask);
        return HAL_I2C_STATUS_ERROR;
    }
    hal_nvic_restore_interrupt_mask(save_mask);
    return HAL_I2C_STATUS_OK;
}

hal_i2c_status_t hal_i2c_master_send_to_receive_dma_ex_none_blocking(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_ex_no_busy_t *config)
{
    i2c_private_info_t *priv_info = NULL;

    priv_info = &s_priv_info[i2c_port];
    if (s_initialized == false) {
        memset(s_priv_info, 0, sizeof(s_priv_info));
        s_initialized = true;
    }
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        log_hal_i2c_error("[hal][i2c%d] dma nb send recv fail: port invalid", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    if (HAL_I2C_FREQUENCY_MAX <= config->i2c_config.frequency) {
        log_hal_i2c_error("[hal][i2c%d] dma nb send recv fail: para invalid", 1, i2c_port);
        return HAL_I2C_STATUS_INVALID_PARAMETER;
    }
    if(hal_i2c_master_send_to_receive_dma_ex_none_blocking_internal(priv_info) != HAL_I2C_STATUS_OK)
    {
        log_hal_i2c_error("[hal][i2c%d] dma nb send recv fail: stat err(%x)", 2, i2c_port, priv_info->hw_state);
        return HAL_I2C_STATUS_ERROR;
    }

    return _hal_i2c_master_send_and_receive_dma_with_nb(i2c_port, config, true);
}

#endif

/***********************************************************************************************
 * HAL I2C DMA NONE_BLOCKING FUNCTION  IMPLEMENT
 *
***********************************************************************************************/

static hal_i2c_status_t    _hal_i2c_config_transfer_poll(hal_i2c_port_t i2c_port, i2c_transfer_config_t *config)
{
    i2c_private_info_t     *priv_info = NULL;

    priv_info = &s_priv_info[i2c_port];

    /* clear status & fifo */
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_SET_INTR_MASK, 0);
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_CLR_IRQ_STAT, 0);
    i2c_config_fifo(i2c_port, I2C_FIFO_OP_CLR, 0, 0);

    /* config io*/
    if ((priv_info->hw_state & I2C_HW_STATE_IO_CFG_MASK) == I2C_HW_STATE_IO_OPENDRAIN) {
        i2c_config_io(i2c_port, true);
    } else {
        i2c_config_io(i2c_port, false);
    }

    /* config fifo*/
    if (config->trans_type == I2C_TRANSFER_TYPE_TX_RX || config->trans_type == I2C_TRANSFER_TYPE_TX) {
        i2c_config_fifo(i2c_port, I2C_FIFO_OP_WR, config->send_buff, config->send_size); /* write fifo if trans type is tx */
    }
    i2c_config_transfer(i2c_port, config);

    return HAL_I2C_STATUS_OK;
}


static hal_i2c_status_t    _hal_i2c_config_transfer(hal_i2c_port_t i2c_port, i2c_transfer_config_t *config)
{
    i2c_private_info_t     *priv_info = NULL;
    //hal_i2c_status_t        status    = HAL_I2C_STATUS_OK;

    priv_info = &s_priv_info[i2c_port];

    /* config transfer */
    if (config->speed != HAL_I2C_FREQUENCY_MAX) {
        i2c_config_speed(i2c_port, i2c_speed_enum_to_dec(config->speed), 0);
    }
    if ((priv_info->hw_state & I2C_HW_STATE_USED_MASK) == I2C_HW_STATE_USED_NB) {
        if ((priv_info->hw_state & I2C_HW_STATE_IO_CFG_MASK) == I2C_HW_STATE_IO_OPENDRAIN) {
            i2c_config_io(i2c_port, true);
        } else {
            i2c_config_io(i2c_port, false);
        }
    }

    /*
     * For resolve multi-thread access & time check disable 70us, Add some dumy code.
     */
    do {
        /* clear status & fifo */
        if(i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0) == 0) {
            i2c_op_ioctl(i2c_port, I2C_IOCTRL_CLR_IRQ_STAT, 0);
            i2c_config_fifo(i2c_port, I2C_FIFO_OP_CLR, 0, 0);
        }

        /* if config is fifo mode, write data to fifo*/
        if (config->trans_mode == I2C_TRANSFER_MODE_FIFO) {
            if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0) == 0) {
                if (config->trans_type == I2C_TRANSFER_TYPE_TX_RX || config->trans_type == I2C_TRANSFER_TYPE_TX) {
                    i2c_config_fifo(i2c_port, I2C_FIFO_OP_WR, config->send_buff, config->send_size); /* write fifo if trans type is tx */
                }
            }
        }

        /* if config is DMA mode, init dma*/
        if (config->trans_mode == I2C_TRANSFER_MODE_DMA && i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0) == 0) {
            if(i2c_pdma_init(i2c_port, config)) {
                break;
            }
        }

        /* config i2c hw */
        if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0) == 0) {
            i2c_config_transfer(i2c_port, config);
        }
    } while (0);
   // hal_nvic_restore_interrupt_mask(mask);

    return HAL_I2C_STATUS_OK;
}


ATTR_TEXT_IN_RAM_FOR_MASK_IRQ   static void    _hal_i2c_interrupt_handle_internal(i2c_private_info_t *priv_info, uint32_t *temp, i2c_transfer_config_t  *pconfig, hal_i2c_port_t i2c_port)
{
    uint32_t                mask = 0;
    /* if queue is not empty then start next transfer, else deinit pdma and unlock sleep */
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (queue_top(&(priv_info->i2c_queue), (int *)temp) >= 0) {
        hal_nvic_restore_interrupt_mask(mask);
        pconfig    = (i2c_transfer_config_t *)(*temp);
        _hal_i2c_config_transfer(i2c_port, pconfig);
        i2c_op_ioctl(i2c_port, I2C_IOCTRL_START, 0);
    } else {
        priv_info->hw_state &= ~I2C_HW_STATE_DEV_BUSY_MASK;  /* clear hw busy flag */
        priv_info->hw_state |= I2C_HW_STATE_IN_DEINIT_MASK;
        hal_nvic_restore_interrupt_mask(mask);
        /* if user call non-block api, need deinit hw */
        if ((priv_info->hw_state & I2C_HW_STATE_USED_MASK) == I2C_HW_STATE_USED_NB) {
            hal_i2c_master_deinit(i2c_port);
        }
        i2c_op_ioctl(i2c_port, I2C_IOCTRL_LOCK_SLEEP, false);
    }
}

static void    _hal_i2c_interrupt_handle(hal_nvic_irq_t irq_number)
{
    uint32_t  temp = 0;
    i2c_transfer_config_t  *pconfig = NULL;
    i2c_private_info_t     *priv_info = NULL;
    hal_i2c_port_t          i2c_port;
    i2c_irq_status_t        irq_status;
    int                     q_stat;
    uint8_t                 slv_addr = 0;

    i2c_port = i2c_get_port_by_nvic_id(irq_number);
    if (i2c_port >= HAL_I2C_MASTER_MAX) {
        log_hal_i2c_error("[hal][i2c%d] irq handle err: port invalid", 1, i2c_port);
        return;
    }

    /* process cureent transfer */
    priv_info  = &s_priv_info[i2c_port];
    irq_status = i2c_get_irq_status(i2c_port);
    slv_addr   = i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_SLV_ADDR, 0);
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_CLR_IRQ_STAT, 0);

    q_stat = queue_pop(&(priv_info->i2c_queue), (int *)&temp);
    if (q_stat < 0) {
        log_hal_i2c_error("[hal][i2c%d] irq handle err: pop q fail(%d), irq_sta(0x%x)", 3, i2c_port, q_stat, irq_status);
        return;
    }
    pconfig = (i2c_transfer_config_t *)temp;
    i2c_pdma_wait_rx_idle(i2c_port, 50);
    if (pconfig->trans_mode == I2C_TRANSFER_MODE_DMA) {
        i2c_pdma_deinit(i2c_port);
    }
    /* call user callback */
    if (pconfig->call_back) {
        pconfig->call_back(pconfig->slv_addr, (hal_i2c_callback_event_t)irq_status, pconfig->user_data);
    } else if (priv_info->call_back) {
        priv_info->call_back(slv_addr, (hal_i2c_callback_event_t)irq_status, priv_info->user_data);
    }
    pconfig->priv_data &= ~(I2C_ITEM_FLAG_IS_USED); ; /* clear item flag */
    _hal_i2c_interrupt_handle_internal(priv_info, &temp, pconfig, i2c_port);
}


ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static hal_i2c_status_t _hal_i2c_master_send_and_receive_dma_with_nb(hal_i2c_port_t i2c_port, hal_i2c_send_to_receive_config_ex_no_busy_t *config, bool is_non_block)
{
    uint32_t                i = 0;
    uint32_t                save_mask;
    int                     temp;
    i2c_private_info_t     *priv_info = NULL;
    i2c_transfer_config_t  *pconfig = NULL;
    i2c_transfer_config_t  *parray  = NULL;
    hal_i2c_status_t        status  = HAL_I2C_STATUS_OK;
    uint8_t                 sz_i2c_q = 0;

    /* step 0: parameter check & init para */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    if (NULL == config || (config->i2c_send_to_receive_config_ex.receive_bytes_in_one_packet == 0 &&
                           config->i2c_send_to_receive_config_ex.send_bytes_in_one_packet == 0)) {
        return HAL_I2C_STATUS_INVALID_PARAMETER;
    }
    priv_info = &s_priv_info[i2c_port];

 /*step 1: find free item */
    hal_nvic_save_and_set_interrupt_mask(&save_mask);/* disable irq */
    temp = priv_info->hw_state & I2C_HW_STATE_USED_MASK;
    if((is_non_block == true  && temp == I2C_HW_STATE_USED_NORMAL) ||
       (is_non_block == false && temp == I2C_HW_STATE_USED_NB)) {
        hal_nvic_restore_interrupt_mask(save_mask); /* enable irq */
        return HAL_I2C_STATUS_ERROR;
    }
    if (is_non_block) {
        priv_info->hw_state &= ~(I2C_HW_STATE_USED_MASK);
        priv_info->hw_state |= (I2C_HW_STATE_USED_NB << I2C_HW_STATE_USED_OFFSET);
    }
    parray   = s_i2c_q_buf[i2c_port];
    sz_i2c_q = s_i2c_q_szb[i2c_port];
    i = priv_info->idx;
    if ((parray[i].priv_data & I2C_ITEM_FLAG_IS_USED) == 0) {
        pconfig = &parray[i];
        priv_info->idx++;
        pconfig->priv_data |= I2C_ITEM_FLAG_IS_USED;
        if (priv_info->idx >= sz_i2c_q) {
            priv_info->idx = 0;
        }
    }
    if (pconfig == NULL) {
        hal_nvic_restore_interrupt_mask(save_mask); /* enable irq */
        log_hal_i2c_error("[hal][i2c%d] dma send recv err 0: none item avail", 1,i2c_port);
        return HAL_I2C_STATUS_ERROR;
    }

    /* step 2: initial i2c transfer config parameter */
    pconfig->speed      = config->i2c_config.frequency;
    pconfig->slv_addr   = config->i2c_send_to_receive_config_ex.slave_address;
    pconfig->trans_mode = I2C_TRANSFER_MODE_DMA;
    pconfig->send_buff  = (uint8_t *)config->i2c_send_to_receive_config_ex.send_data;
    pconfig->send_size  = config->i2c_send_to_receive_config_ex.send_bytes_in_one_packet;
    pconfig->recv_buff  = config->i2c_send_to_receive_config_ex.receive_buffer;
    pconfig->recv_size  = config->i2c_send_to_receive_config_ex.receive_bytes_in_one_packet;
    pconfig->send_pack_size = config->i2c_send_to_receive_config_ex.send_packet_length;
    pconfig->recv_pack_size = config->i2c_send_to_receive_config_ex.receive_packet_length;
    pconfig->call_back   = config->i2c_callback;
    pconfig->user_data  = config->user_data;
    if (pconfig->recv_size && pconfig->send_size) {
        pconfig->trans_type = I2C_TRANSFER_TYPE_TX_RX;
    } else if (pconfig->recv_size == 0) {
        pconfig->trans_type = I2C_TRANSFER_TYPE_TX;
    } else {
        pconfig->trans_type = I2C_TRANSFER_TYPE_RX;
    }

    temp = priv_info->hw_state & I2C_HW_STATE_USED_MASK;

    /* step 3: push transfer config to queue & lock sleep */
    temp = queue_push(&priv_info->i2c_queue, (uint32_t) pconfig);
    hal_nvic_restore_interrupt_mask(save_mask); /* enable irq */

    if (temp < 0) {
        log_hal_i2c_warning("[hal][i2c%d] dma send recv err:q push fail(%d)", 2, i2c_port, (int) temp);
        return HAL_I2C_STATUS_ERROR;
    }
    hal_nvic_save_and_set_interrupt_mask(&save_mask); /* disable irq */

    if (priv_info->hw_state & I2C_HW_STATE_DEV_BUSY_MASK) {
        hal_nvic_restore_interrupt_mask(save_mask); /* enable irq */
        log_hal_i2c_warning("[hal][i2c%d] dma send recv: hw is working now(%x)", 2, i2c_port, priv_info->hw_state);
        return HAL_I2C_STATUS_OK;
    }
    priv_info->hw_state |= I2C_HW_STATE_DEV_BUSY_MASK;
    hal_nvic_restore_interrupt_mask(save_mask); /* enable irq */

    /* step 4: config transfer */
    if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
        return HAL_I2C_STATUS_ERROR;
    }
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_ENABLE_CLOCK, true); /* enable i2c clock */
    status =  _hal_i2c_config_transfer(i2c_port, pconfig);
    i2c_config_irq(i2c_port, _hal_i2c_interrupt_handle);
    i2c_op_ioctl(i2c_port, I2C_IOCTRL_SET_INTR_MASK, 0x7);

    /* step 5: start */
    hal_nvic_save_and_set_interrupt_mask(&save_mask); /* disable irq */
    if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0) == 0) {
        if (queue_top(&priv_info->i2c_queue, (int *) &temp) >= 0) {
            i2c_op_ioctl(i2c_port, I2C_IOCTRL_LOCK_SLEEP, true);   /* lock sleep */
            pconfig = (i2c_transfer_config_t *)temp;
            i2c_op_ioctl(i2c_port, I2C_IOCTRL_START, 0);  /* start transfer */

        }
    }
    hal_nvic_restore_interrupt_mask(save_mask);/* enable irq */
    return status;
}


hal_i2c_status_t hal_i2c_master_get_running_status(hal_i2c_port_t i2c_port, hal_i2c_running_status_t *running_status)
{
    /* parameter check */
    if (HAL_I2C_MASTER_MAX <= i2c_port) {
        return HAL_I2C_STATUS_INVALID_PORT_NUMBER;
    }
    if (i2c_op_ioctl(i2c_port, I2C_IOCTRL_GET_BUSY_STAT, 0)) {
        running_status->running_status = HAL_I2C_STATUS_BUS_BUSY;
    } else {
        running_status->running_status = HAL_I2C_STATUS_IDLE;
    }
    return HAL_I2C_STATUS_OK;
}


#endif /* #if defined(HAL_I2C_MASTER_MODULE_ENABLED) */
