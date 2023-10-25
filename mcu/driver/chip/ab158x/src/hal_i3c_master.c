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
#include "hal_platform.h"

#ifdef HAL_I3C_MASTER_MODULE_ENABLED
#include "hal_nvic.h"
#include "hal_i3c_master.h"
#include "hal_i3c_master_internal.h"
#include "hal_i2c_master_internal.h"

/******************************************************************************
 * Pre Processor Definitions
*******************************************************************************/
#define     I3C_DEFAULT_SPEED_FS_KHZ     1000
#define     I3C_DEFAULT_SPEED_HS_KHZ     12500
#define     I3C_BUS_CONFIG_WAIT_TIME_MS  50

/******************************************************************************
 * Private Function Declare
*******************************************************************************/

#define     DISABLE_ALL_IRQ(mask)       hal_nvic_save_and_set_interrupt_mask_special(&mask)
#define     ENABLE_ALL_IRQ(mask)        hal_nvic_restore_interrupt_mask_special(mask)

extern hal_nvic_status_t       hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t       hal_nvic_restore_interrupt_mask_special(uint32_t mask);
static void                    i3c_start_hardware(hal_i3c_master_port_t i3c_port,   i3c_master_transfer_config_t *pConfig);
static hal_i3c_master_status_t i3c_config_transfer(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *pConfig);
static void                    i3c_isr_handle(hal_nvic_irq_t irq_number);
static hal_i3c_master_status_t i3c_polling_config_bus(hal_i3c_master_port_t i3c_port, const hal_i3c_master_pid_da_map_t config_da[], uint32_t config_da_sz);
static hal_i3c_master_status_t i3c_master_transfer(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *pConfig);


/******************************************************************************
 * Private Variable Define
*******************************************************************************/
/* user define sw fifo*/
static i3c_transfer_config_ex_t     s_i3c0_fifo_buff[HAL_I3C_MASTER_0_FIFO_DEPTH] = {0};
static i3c_transfer_config_ex_t     s_i3c1_fifo_buff[HAL_I3C_MASTER_1_FIFO_DEPTH] = {0};
static i3c_transfer_config_ex_t    *s_i3c_fifo_buff[HAL_I3C_MASTER_MAX] = {s_i3c0_fifo_buff, s_i3c1_fifo_buff};
static const   uint8_t              s_i3c_fifo_size[HAL_I3C_MASTER_MAX] = {HAL_I3C_MASTER_0_FIFO_DEPTH, HAL_I3C_MASTER_1_FIFO_DEPTH};

/* Private information for driver */
static i3c_private_info_t           s_i3c_priv_info[HAL_I3C_MASTER_MAX] = {{0},{0}};
static bool                         s_i3c_initiaized = false;
static uint8_t                      s_i3c_default_address = 0x22;


/******************************************************************************
 * Public Function Define
*******************************************************************************/
hal_i3c_master_status_t hal_i3c_master_init(hal_i3c_master_port_t i3c_port, hal_i3c_master_config_t *config)
{
    i3c_private_info_t *priv_info = NULL;
    uint32_t mask;

    if (i3c_port >= HAL_I3C_MASTER_MAX) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_init: err port", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER;
    }
    if (config == NULL) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_init: null para", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    if(config->fast_speed_khz == 0 && config->high_speed_khz == 0){
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_init: inv speed", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }

    if (s_i3c_initiaized == false) {
        memset(s_i3c_priv_info,  0, sizeof(s_i3c_priv_info));
        memset(s_i3c0_fifo_buff, 0, sizeof(s_i3c0_fifo_buff));
        memset(s_i3c1_fifo_buff, 0, sizeof(s_i3c1_fifo_buff));
        s_i3c_initiaized = true;
    }
    priv_info   = &s_i3c_priv_info[i3c_port];
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (priv_info->op_state & I3C_OP_STATE_INIT_MASK) {
        hal_nvic_restore_interrupt_mask(mask);
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_init: busy", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_ERROR_BUSY;
    }
    priv_info->op_state      |= I3C_OP_STATE_INIT_MASK;
    hal_nvic_restore_interrupt_mask(mask);
    priv_info->work_mode      = config->mode;
    priv_info->fast_speed_khz = config->fast_speed_khz;
    priv_info->high_speed_khz = config->high_speed_khz;
    priv_info->call_back      = config->call_back;
    priv_info->user_data      = config->user_data;
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_ENABLE_CLOCK, true);
    return i3c_polling_config_bus(i3c_port, config->map_pid_da_config, config->map_pid_da_sz);

}




hal_i3c_master_status_t hal_i3c_master_control(hal_i3c_master_port_t i3c_port, hal_i3c_master_control_config_t *pConfig)
{
    i3c_master_transfer_config_t config;
    i3c_private_info_t         *priv_info = NULL;

    if (i3c_port > HAL_I3C_MASTER_MAX) {
        return HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER;
    }
    if (pConfig == NULL) {
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    priv_info = &s_i3c_priv_info[i3c_port];
    if (priv_info->work_mode == HAL_I3C_MASTER_WORKING_MODE_I2C) {
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    memset(&config, 0, sizeof(config));
    config.trans_mode     = pConfig->trans_type;
    config.slave_addr     = pConfig->slave_addr;
    config.fast_speed_khz = priv_info->fast_speed_khz;
    config.high_speed_khz = priv_info->high_speed_khz;

    if (pConfig->trans_type == HAL_I3C_MASTER_TRANSFER_TYPE_SEND) {
        config.send_size = pConfig->data_buff_size;
        config.send_buff = pConfig->data_buff;
        config.recv_size = 0;
        config.recv_buff = NULL;
    } else {
        config.send_size = 0;
        config.send_buff = NULL;
        config.recv_size = pConfig->data_buff_size;
        config.recv_buff = pConfig->data_buff;
    }
    config.call_back = NULL;
    config.user_data = NULL;
    config.o_flag    = HAL_I3C_FLAG_CCC_EN_MASK;
    config.ccc_cmd   = pConfig->ccc_cmd;
    config.ccc_append_data_sz = pConfig->def_byte_size;
    config.ccc_append_data    = pConfig->def_byte_data;

    return i3c_master_transfer(i3c_port, &config);
}



hal_i3c_master_status_t hal_i3c_master_transfer(hal_i3c_master_port_t i3c_port, hal_i3c_master_transfer_config_t *pConfig)
{
    i3c_master_transfer_config_t config;
    i3c_private_info_t         *priv_info = NULL;

    priv_info = &s_i3c_priv_info[i3c_port];

    /* parametr check */
    if (i3c_port > HAL_I3C_MASTER_MAX) {
        return HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER;
    }
    if (pConfig == NULL) {
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }

    /* setting parameter */
    memset(&config, 0, sizeof(config));
    config.trans_mode     = pConfig->trans_type;
    config.slave_addr     = pConfig->slave_addr;
    config.fast_speed_khz = priv_info->fast_speed_khz;
    config.high_speed_khz = priv_info->high_speed_khz;
    config.send_size      = pConfig->send_size;
    config.send_buff      = pConfig->send_buff;
    config.recv_size      = pConfig->recv_size;
    config.recv_buff      = pConfig->recv_buff;
    config.call_back      = NULL;
    config.user_data      = NULL;
    if (priv_info->work_mode == HAL_I3C_MASTER_WORKING_MODE_I2C) {
        config.o_flag = HAL_I3C_FLAG_I2C_MODE_MASK;
    }

    return i3c_master_transfer(i3c_port, &config);
}


hal_i3c_master_status_t hal_i3c_master_deinit(hal_i3c_master_port_t i3c_port)
{
    i3c_private_info_t *priv_info = NULL;

    priv_info = &s_i3c_priv_info[i3c_port];
    if (i3c_port >= HAL_I3C_MASTER_MAX) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_deinit: err port", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER;
    }
    if (priv_info->op_state & I3C_OP_STATE_BUSY_MASK) {
        log_hal_i3c_warning("[hal][i3c%d] hal_i3c_master_deinit: HW is working, may destory current transaction!!!", 1, i3c_port);
    }
    priv_info->op_state = 0;
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_ENABLE_CLOCK, false);

    return HAL_I3C_MASTER_STATUS_OK;
}



/*******************************************************************************************************************
*
*                                          Private Function Define
*
********************************************************************************************************************/

static void    i3c_start_hardware(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *pConfig)
{
    if (pConfig->trans_mode == HAL_I3C_TRANSFER_TYPE_DAA) {
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_START, 0xC000);
    } else {
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_START, 0);
    }
}

static hal_i3c_master_status_t i3c_config_transfer(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *pConfig)
{
    hal_i3c_master_status_t status = HAL_I3C_MASTER_STATUS_OK;
    i3c_private_info_t     *priv_info = NULL;
    uint32_t                fs_speed = 0, hs_speed = 0, mask = 0;

    priv_info = &s_i3c_priv_info[i3c_port];
    fs_speed  = (pConfig->fast_speed_khz == 0) ? priv_info->fast_speed_khz : pConfig->fast_speed_khz;
    hs_speed  = (pConfig->high_speed_khz == 0) ? priv_info->high_speed_khz : pConfig->high_speed_khz;

    if (pConfig->o_flag & HAL_I3C_FLAG_I2C_MODE_MASK) {
        status = i3c_config_i2c_speed(i3c_port, fs_speed, hs_speed);
    } else {
        status = i3c_config_i3c_speed(i3c_port, fs_speed, hs_speed);
    }
    if (status != HAL_I3C_MASTER_STATUS_OK) {
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_BUSY_STAT, 0)) {
        hal_nvic_restore_interrupt_mask(mask);
        return HAL_I3C_MASTER_STATUS_ERROR_BUSY;
    } else {
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_CLR_IRQ_STAT, 0);     /* clear intr status */
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_SET_IRQ_MASK, 0x3FF); /* unmask all irq status */
        i3c_config_fifo(i3c_port, I3C_FIFO_CLEAR, NULL, 0);     /* clear hw fifo */

        /*config io to open-drain or pushpull mode*/
        if (pConfig->o_flag & HAL_I3C_FLAG_IO_OPENDRAIN_MASK) {
            i3c_config_io(i3c_port, HAL_I3C_IO_CONFIG_OPENDRAIN);    /* config io to pushpull*/
        } else {
            i3c_config_io(i3c_port, HAL_I3C_IO_CONFIG_PUSHPULL);    /* config io to pushpull*/
        }
    }
    hal_nvic_restore_interrupt_mask(mask);

    DISABLE_ALL_IRQ(mask);
    if (i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_BUSY_STAT, 0)) {
        status = HAL_I3C_MASTER_STATUS_ERROR_BUSY;
    } else {
        status = i3c_config_hardware(i3c_port, pConfig);
    }
    ENABLE_ALL_IRQ(mask);
    return status;
}


static hal_i3c_master_status_t i3c_master_transfer(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *pConfig)
{
    i3c_transfer_config_ex_t   *pItem  = NULL;
    i3c_transfer_config_ex_t   *pArray = NULL;
    i3c_private_info_t         *priv_info = NULL;
    uint8_t                     sz_i3c_fifo= 0;
    int                         q_sta = 0, temp = 0, i=0;
    uint32_t                    mask  = 0;
    hal_i3c_master_status_t     status;

    if (i3c_port >= HAL_I3C_MASTER_MAX) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: err port", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PORT_NUMBER;
    }
    if (pConfig == NULL) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: null para", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }
    if (s_i3c_initiaized == false) {
        memset(s_i3c_priv_info,  0, sizeof(s_i3c_priv_info));
        memset(s_i3c0_fifo_buff, 0, sizeof(s_i3c0_fifo_buff));
        memset(s_i3c1_fifo_buff, 0, sizeof(s_i3c1_fifo_buff));
        s_i3c_initiaized = true;
    }
    priv_info = &s_i3c_priv_info[i3c_port];
    if((priv_info->op_state & I3C_OP_STATE_INIT_MASK) == 0){
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: null para", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_INVALID_PARAMETER;
    }

    /* step 1: find free item and push into fifo */
    pArray      = s_i3c_fifo_buff[i3c_port];
    sz_i3c_fifo = s_i3c_fifo_size[i3c_port];
    hal_nvic_save_and_set_interrupt_mask(&mask);
    i = priv_info->idx;
    if ((pArray[i].state & I3C_ITEM_FLAG_IS_USED) == 0) {
        pItem = &pArray[i];
        priv_info->idx++;
        if (priv_info->idx >= sz_i3c_fifo) {
            priv_info->idx = 0;
        }
    }
    if (pItem == NULL) {
        hal_nvic_restore_interrupt_mask(mask);
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: no avail item", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_ERROR_NO_SPACE;
    }
    memcpy(&pItem->user_config, pConfig, sizeof(i3c_master_transfer_config_t));
    pItem->state = I3C_ITEM_FLAG_IS_USED;
    q_sta = queue_push(&priv_info->i3c_queue, (int) pItem);
    if (q_sta < 0) {
        hal_nvic_restore_interrupt_mask(mask);
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: fifo full", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_ERROR_NO_SPACE;
    }
    if ( priv_info->op_state & I3C_OP_STATE_BUSY_MASK) {
        hal_nvic_restore_interrupt_mask(mask);
        log_hal_i3c_info("[hal][i3c%d] hal_i3c_master_transfer: hw busy", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_OK;
    } else {
        i3c_hardware_reset(i3c_port);
    }
    hal_nvic_restore_interrupt_mask(mask);

    /* step 2: get top and config it to hardware */
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_ENABLE_CLOCK, true);
    q_sta = queue_top(&priv_info->i3c_queue, &temp);
    if (q_sta < 0) {
        log_hal_i3c_info("[hal][i3c%d] hal_i3c_master_transfer: fifo empty", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_OK;
    }
    pItem = (i3c_transfer_config_ex_t *)temp;
    if ((status = i3c_config_transfer(i3c_port, &(pItem->user_config))) == HAL_I3C_MASTER_STATUS_ERROR_BUSY) {
        log_hal_i3c_info("[hal][i3c%d] hal_i3c_master_transfer: be preempted", 1, i3c_port);
        return HAL_I3C_MASTER_STATUS_OK;
    } else if (status != HAL_I3C_MASTER_STATUS_OK) {
        log_hal_i3c_error("[hal][i3c%d] hal_i3c_master_transfer: config err %d", 2, i3c_port, status);
        return status;
    }
    i3c_config_irq(i3c_port, i3c_isr_handle);

    /* step 3: check whether fifo is empty and start transfer */
    hal_nvic_save_and_set_interrupt_mask(&mask);
    q_sta = queue_top(&priv_info->i3c_queue, &temp);
    /* if fifo is empty or hw is busy, then direct return */
    if (q_sta < 0 || (priv_info->op_state & I3C_OP_STATE_BUSY_MASK)) {
    } else {
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_LOCK_SLEEP, true);
        i3c_start_hardware(i3c_port, &pItem->user_config);
        priv_info->op_state |= I3C_OP_STATE_BUSY_MASK;
    }
    hal_nvic_restore_interrupt_mask(mask);
    return HAL_I3C_MASTER_STATUS_OK;
}


static hal_i3c_master_status_t    i3c_polling_config_bus(hal_i3c_master_port_t i3c_port, const hal_i3c_master_pid_da_map_t config_da[], uint32_t config_da_sz)
{
    i3c_master_transfer_config_t  config;
    hal_i3c_device_id_t  i3c_id_info;
    uint32_t irq_status = 0, i = 0;
    hal_i3c_master_status_t ret = HAL_I3C_MASTER_STATUS_ERROR_BUS_CFG;
    uint8_t  slv_da = 0, slv_num = 0;
    int temp = 0;

    config.send_size = 0;
    config.recv_size = 0;
    config.trans_mode     = HAL_I3C_TRANSFER_TYPE_DAA;
    config.slave_addr     = 0x7E;
    config.fast_speed_khz = 1000;
    config.high_speed_khz = 12500;
    config.send_buff      = NULL;
    config.recv_buff      = NULL;
    config.call_back      = NULL;
    config.user_data      = NULL;
    config.ccc_cmd        = 0x07;
    config.ccc_append_data_sz = 0;
    config.o_flag          = HAL_I3C_FLAG_FIFO_MODE_MASK | HAL_I3C_FLAG_CCC_EN_MASK;
    i3c_config_transfer(i3c_port, &config);
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_CLR_IRQ_STAT, 0x0); /* Clr irq status*/
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_SET_IRQ_MASK, 0x0); /* Mask all irq */
    i3c_start_hardware(i3c_port, &config);

    if ( (temp = i3c_wait_status_with_timeout(i3c_port, I3C_BUS_CONFIG_WAIT_TIME_MS)) < 0) {
        log_hal_i3c_error("[hal][i3c%d] i3c_bus_config: wait intsta err(%d)", 2, i3c_port, temp);
        return HAL_I3C_MASTER_STATUS_ERROR_BUS_CFG;
    }

    while (1) {
        /* Wait transfer done & read back device info */
        irq_status = i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_IRQ_STAT, 0);
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_CLR_IRQ_STAT, 0x0);
        if (irq_status == 0x10) {
            i3c_config_fifo(i3c_port, I3C_FIFO_READ, (uint8_t *)&i3c_id_info, 8);
        } else {
            log_hal_i3c_warning("[hal][i3c%d] i3c_master_bus_config: completed %d slv setting(int_stat %x)", 3, i3c_port, slv_num, irq_status);
            break;
        }

        /* Get a dynamic address from config */
        for (i = 0; i < config_da_sz; i++) {
            if (memcmp(config_da[i].pid, i3c_id_info.PID, 6) == 0) {
                slv_da = config_da[i].da;
                break;
            }
        }
        if ( i == config_da_sz) {
            slv_da = s_i3c_default_address++;
        }
        /* Set dynamic address to slave */
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_SET_DEF_DA, slv_da);
        i3c_start_hardware(i3c_port, &config);
        slv_num++;
        log_hal_i3c_warning("[hal][i3c%d] i3c_master_bus_config: assign da[%x] to slave[%x-%x-%x-%x-%x-%x]", 8, i3c_port,config_da[i].da,
                              i3c_id_info.PID[0], i3c_id_info.PID[1],i3c_id_info.PID[2],
                              i3c_id_info.PID[3], i3c_id_info.PID[4],i3c_id_info.PID[5]
                           );
    }
    if (slv_num > 0) {
        ret = HAL_I3C_MASTER_STATUS_OK;
    }
    return ret;
}

static void    i3c_isr_handle(hal_nvic_irq_t irq_number)
{
    hal_i3c_master_port_t       i3c_port;
    uint32_t                    irq_status, err_status;
    int                         q_stat, temp;
    uint8_t                     slv_addr = 0;
    i3c_transfer_config_ex_t   *pconfig   = NULL;
    i3c_private_info_t         *priv_info = NULL;
    hal_i3c_master_callback_event_t    event_type;

    /* find the i3c port by nvic_number*/
    i3c_port = i3c_get_port_by_nvic_id(irq_number);
    if (i3c_port >= HAL_I3C_MASTER_MAX) {
        log_hal_i3c_error("[hal][i3c%d] irq handle err: port invalid", 1, i3c_port);
        return;
    }
    priv_info  = &s_i3c_priv_info[i3c_port];
    irq_status = i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_IRQ_STAT, 0);
    slv_addr   = i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_SLV_ADDR, 0);
    err_status = i3c_opt_ioctl(i3c_port, I3C_IOCTL_GET_ERROR_STAT, 0);
    i3c_opt_ioctl(i3c_port, I3C_IOCTL_CLR_IRQ_STAT, 0);
    event_type = i3c_get_event_type(irq_status, err_status);
    log_hal_i3c_debug("[hal][i3c%d] isr handle: intr status 0x%x, error 0x%x, event %d", 4, i3c_port, (unsigned int)irq_status, (unsigned int)err_status, event_type);

    /* process IBI event */
    if (event_type == HAL_I3C_EVENT_IBI) {
        slv_addr = err_status >> 9;
        if (priv_info->call_back != NULL) {
            priv_info->call_back(i3c_port, slv_addr, event_type, priv_info->user_data);
        }
        priv_info->op_state &= ~I3C_OP_STATE_BUSY_MASK;
        //return;
    }

    if ((q_stat = queue_pop(&priv_info->i3c_queue, (int *) &temp)) < 0 ) {
        priv_info->op_state &= ~I3C_OP_STATE_BUSY_MASK;
        log_hal_i3c_debug("[hal][i3c%d] isr handle: queue empty", 1, i3c_port);
        return;
    }
    pconfig = (i3c_transfer_config_ex_t *) temp;
    if (event_type == HAL_I3C_EVENT_IBI) {
        pconfig->state &= ~I3C_ITEM_FLAG_IS_USED;
        return;
    }

    /* delay for wait dma transfer done */
    hal_gpt_delay_us(10);

    /* read data or deinit pdma */
    if (pconfig->user_config.o_flag & HAL_I3C_FLAG_FIFO_MODE_MASK) {
        i3c_config_fifo(i3c_port, I3C_FIFO_READ, pconfig->user_config.recv_buff, pconfig->user_config.recv_size);
    } else {
        i3c_pdma_deinit(i3c_port);
    }

    /* call user callback if user callback is valid */
    if (pconfig->user_config.call_back != NULL) {
        pconfig->user_config.call_back(i3c_port, slv_addr, event_type, pconfig->user_config.user_data);
    } else if (priv_info->call_back != NULL) {
        priv_info->call_back(i3c_port, slv_addr, event_type, priv_info->user_data);
    }
    pconfig->state &= ~I3C_ITEM_FLAG_IS_USED;

    /* query wether have other transfer need handle? */
    if ((q_stat = queue_top(&priv_info->i3c_queue, (int *) &temp)) < 0 ) {
        /* unlock sleep */
        i3c_opt_ioctl(i3c_port, I3C_IOCTL_LOCK_SLEEP, false);
        priv_info->op_state &= ~I3C_OP_STATE_BUSY_MASK;
        log_hal_i3c_warning("[hal][i3c%d] isr handle: complete all transfer", 1, i3c_port);
    } else {
        hal_i3c_master_status_t     status;

        log_hal_i3c_warning("[hal][i3c%d] isr handle: next transfer", 1, i3c_port);
        pconfig = (i3c_transfer_config_ex_t *) temp;
        status = i3c_config_transfer(i3c_port, &(pconfig->user_config));
        if (status != HAL_I3C_MASTER_STATUS_OK) {
            log_hal_i3c_error("[hal][i3c%d] isr handle: config transfer err(%d)", 2, i3c_port, status);
        } else {
            i3c_start_hardware(i3c_port,  &(pconfig->user_config));
        }
    }
}


#endif

