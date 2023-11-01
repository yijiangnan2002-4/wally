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

#ifndef _HAL_I2C_MASTER_INTERNAL_H_
#define _HAL_I2C_MASTER_INTERNAL_H_
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "hal_platform.h"
#include "hal.h"
#include "memory_attribute.h"

#if defined(HAL_I2C_MASTER_MODULE_ENABLED)
#include "hal_log.h"
#include "hal_i2c_master.h"
#include "hal_sleep_manager_internal.h"

#ifndef HAL_I2C_MASTER_0_FIFO_DEPTH
    #define     HAL_I2C_MASTER_0_FIFO_DEPTH   5
#endif
#ifndef HAL_I2C_MASTER_1_FIFO_DEPTH
    #define     HAL_I2C_MASTER_1_FIFO_DEPTH   5
#endif
#ifndef HAL_I2C_MASTER_2_FIFO_DEPTH
    #define     HAL_I2C_MASTER_2_FIFO_DEPTH   5
#endif

#ifdef I2C_PLAIN_LOG_ENABLE
    #define log_hal_i2c_debug(_message, cnt, ...)        //printf(_message, ##__VA_ARGS__)
    #define log_hal_i2c_info(_message, cnt, ...)        //printf(_message, ##__VA_ARGS__)
    #define log_hal_i2c_warning(_message, cnt, ...)     printf(_message, ##__VA_ARGS__)
    #define log_hal_i2c_error(_message, cnt, ...)       printf(_message, ##__VA_ARGS__)
#else
    #define log_hal_i2c_level_0(_message, cnt, ...)     //log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i2c_level_1(_message, cnt, ...)     //log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i2c_debug(_message, cnt, ...)       //log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i2c_info(_message, cnt, ...)         log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i2c_warning(_message, cnt, ...)      log_hal_msgid_warning(_message,cnt, ##__VA_ARGS__)
    #define log_hal_i2c_error(_message,cnt, ...)         log_hal_msgid_error(_message,cnt,   ##__VA_ARGS__)
#endif

#ifdef HAL_NVIC_MODULE_ENABLED
    #define     I2C_NVIC_ENABLED
#endif


enum{
    I2C_TRANSFER_TYPE_TX = 0,
    I2C_TRANSFER_TYPE_RX,
    I2C_TRANSFER_TYPE_TX_RX,
};

enum{
    I2C_TRANSFER_MODE_FIFO = 0,
    I2C_TRANSFER_MODE_DMA,
};

enum {
    I2C_FIFO_OP_WR = 0,
    I2C_FIFO_OP_RD,
    I2C_FIFO_OP_CLR
};

enum {
    I2C_IOCTRL_GET_BUSY_STAT = 0,
    I2C_IOCTRL_CLR_IRQ_STAT,
    I2C_IOCTRL_START,
    I2C_IOCTRL_GET_IRQ_STAT,
    I2C_IOCTRL_LOCK_SLEEP,
    I2C_IOCTRL_ENABLE_CLOCK,
    I2C_IOCTRL_GET_SLV_ADDR,
    I2C_IOCTRL_DISABLE_IRQ,
    I2C_IOCTRL_SET_INTR_MASK,
};

typedef enum {
    I2C_IRQ_STAT_UNKNOWN_ERROR = -4,
    I2C_IRQ_STAT_ACK_ERROR = -3,           /**<  an ACK error happened during transaction. */
    I2C_IRQ_STAT_NACK_ERROR = -2,          /**<  an NACK error happened during transaction.*/
    I2C_IRQ_STAT_TIMEOUT_ERROR = -1,       /**<  an timeout error happened during transaction.*/
    I2C_IRQ_STAT_SUCCESS = 0               /**<  the transaction complete wihtout any error. */
} i2c_irq_status_t;

#define     I2C_ITEM_FLAG_IS_USED       (0x1<<31)

typedef struct {
    uint8_t  slv_addr;
    uint8_t  trans_type;
    uint8_t  trans_mode;
    uint8_t  speed;
    uint8_t *send_buff;
    uint8_t *recv_buff;
    uint32_t send_size;
    uint32_t recv_size;
    uint16_t send_pack_size;
    uint16_t recv_pack_size;
    int      priv_data;
    hal_i2c_callback_t call_back;
    void    *user_data;
}i2c_transfer_config_t;

hal_i2c_status_t    i2c_config_speed(hal_i2c_port_t  i2c_port, uint32_t i2c_speed, uint32_t i2c_hs_speed);
hal_i2c_status_t    i2c_config_io(hal_i2c_port_t  i2c_port, bool is_opendrain);
hal_i2c_status_t    i2c_config_transfer(hal_i2c_port_t  i2c_port, i2c_transfer_config_t *config);
hal_i2c_status_t    i2c_config_fifo(uint32_t i2c_port, uint8_t op_code, uint8_t *buff, uint32_t buff_sz);
i2c_irq_status_t    i2c_get_irq_status(uint32_t i2c_port);
int                 i2c_op_ioctl(uint8_t i2c_port, uint8_t op_code, uint8_t args);
uint32_t            i2c_speed_enum_to_dec(hal_i2c_frequency_t freq);
int                 i2c_wait_idle_with_timeout(uint32_t i2c_port, uint32_t timeout_us);
int                 i2c_get_port_by_nvic_id(hal_nvic_irq_t irq_num);
int                 i2c_config_irq(hal_i2c_port_t  i2c_port, hal_nvic_isr_t callback);
void                i2c_sleep_register_backup(hal_i2c_port_t  i2c_port, uint32_t *buff, uint32_t size);
void                i2c_sleep_register_restore(hal_i2c_port_t  i2c_port, uint32_t *buff, uint32_t size);


int                 i2c_pdma_init(uint32_t i2c_port, i2c_transfer_config_t *config);
int                 i2c_pdma_deinit(uint32_t i2c_port);
void                i2c_pdma_wait_rx_idle(uint32_t i2c_port, uint32_t timeout_us);

void                hal_i2c_master_dump(hal_i2c_port_t  i2c_port);

#endif /* #if defined(HAL_I2C_MASTER_MODULE_ENABLED) */


//////////////////////////////// Common API For Queue ////////////////////////////////////////////
#define    MAX_FIFO_SIZE                    (32)

#define    I2C_HW_STATE_USED_OFFSET         (0)
#define    I2C_HW_STATE_IO_CFG_OFFSET       (2)
#define    I2C_HW_STATE_DEV_BUSY_OFFSET     (3)

#define    I2C_HW_STATE_USED_MASK           (0x3)    /* 0: Idle, 1:Normal Mode, 2:Non-block Mode */
#define    I2C_HW_STATE_IO_CFG_MASK         (0x1<<2) /* 0: open-drain, 1: push-pull */
#define    I2C_HW_STATE_DEV_BUSY_MASK       (0x1<<3) /* 0: idle, 1 runing */
#define    I2C_HW_STATE_IN_DEINIT_MASK      (0x1<<4) /* 1: in deinit, 0 none-deinit */

enum {
    I2C_HW_STATE_USED_NONE  = 0,
    I2C_HW_STATE_USED_NORMAL= 1,
    I2C_HW_STATE_USED_NB    = 2
};

enum {
    I2C_HW_STATE_IO_OPENDRAIN = (0 << I2C_HW_STATE_IO_CFG_OFFSET),
    I2C_HW_STATE_IO_PUSHPULL  = (1 << I2C_HW_STATE_IO_CFG_OFFSET),
};



typedef struct {
    uint8_t   head;
    uint8_t   tail;
    int   data[MAX_FIFO_SIZE];
}airo_queue_t;


typedef enum {
    I2C_FIFO_STATUS_OK = 0,
    I2C_FIFO_STATUS_ERROR_INVALID_PARA = -1,
    I2C_FIFO_STATUS_ERROR_FULL     = -2,
    I2C_FIFO_STATUS_ERROR_EMPTY    = -3
}queue_status_t;

#if defined(HAL_I2C_MASTER_MODULE_ENABLED)
typedef struct {
    uint8_t           hw_state;
    uint8_t           idx;
    airo_queue_t       i2c_queue;
    hal_i2c_callback_t call_back;
    void              *user_data;
}i2c_private_info_t;
#endif

void    queue_init(airo_queue_t  *pQueue);
int     queue_push(airo_queue_t  *pQueue, int  data);
int     queue_pop( airo_queue_t  *pQueue,  int *data);
int     queue_top( airo_queue_t  *pQueue,  int *data);
/////////////////////////////////////////////////////////////////////////////


#endif /* #ifndef _HAL_I2C_MASTER_INTERNAL_H_ */



