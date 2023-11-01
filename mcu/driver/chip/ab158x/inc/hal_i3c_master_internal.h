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

#include "hal.h"
#include "hal_platform.h"

#ifndef __HAL_I3C_MASTER_INTERNAL_H__
#define __HAL_I3C_MASTER_INTERNAL_H__
#include "hal_i2c_master_internal.h"
#include "hal_i3c_master.h"


#define HAL_I3C_MASTER_MAX_SLAVE_DEVICE_NUM    3

#ifdef HAL_I3C_MASTER_MODULE_ENABLED

/* User not define the FIFO Depth, then define it*/
#ifndef HAL_I3C_MASTER_0_FIFO_DEPTH
    #define     HAL_I3C_MASTER_0_FIFO_DEPTH   5
#endif
#ifndef HAL_I3C_MASTER_1_FIFO_DEPTH
    #define     HAL_I3C_MASTER_1_FIFO_DEPTH   5
#endif

#ifdef HAL_NVIC_MODULE_ENABLED
#define     I3C_NVIC_ENABLED
#endif


/* Define printf for FPGA DVT logging */
//#define     I3C_PLAIN_LOG_ENABLE

#ifdef I3C_PLAIN_LOG_ENABLE
    #define log_hal_i3c_debug(_message, cnt, ...)
    #define log_hal_i3c_info(_message, cnt, ...)        printf(_message, ##__VA_ARGS__)
    #define log_hal_i3c_warning(_message, cnt, ...)     printf(_message, ##__VA_ARGS__)
    #define log_hal_i3c_error(_message, cnt, ...)       printf(_message, ##__VA_ARGS__)
#else
    #define log_hal_i3c_debug(_message, cnt, ...)       //log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i3c_info(_message, cnt, ...)        log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_hal_i3c_warning(_message, cnt, ...)     log_hal_msgid_warning(_message,cnt, ##__VA_ARGS__)
    #define log_hal_i3c_error(_message,cnt, ...)        log_hal_msgid_error(_message,cnt,   ##__VA_ARGS__)
#endif

#define     I3C_SPECIAL_ADDRESS_7E      0x7E
#define     HAL_I3C_TRANSFER_TYPE_DAA   3
/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
    I3C_FIFO_WRITE = 0,
    I3C_FIFO_READ,
    I3C_FIFO_CLEAR
} i3c_fifo_op_t;


enum {
    HAL_I3C_FLAG_IO_OPENDRAIN_MASK = (1 << 0),  /**<  The flag that control io in open-drain mode. */
    HAL_I3C_FLAG_I2C_MODE_MASK     = (1 << 1),  /**<  The flag that control hardware in I2C mode. */
    HAL_I3C_FLAG_CCC_EN_MASK       = (1 << 2),  /**<  The flag that indicate transfer had CCC. */
    HAL_I3C_FLAG_FIFO_MODE_MASK    = (1 << 3),  /**<  The flag that control the hardware work in FIFO mode. */
    HAL_I3C_FLAG_PRIVATE_MASK      = (1 << 4),  /**<  The flag that i3c in private read or write mode. */
};



typedef enum{
    I3C_IOCTL_START = 0,
    I3C_IOCTL_GET_BUSY_STAT,
    I3C_IOCTL_GET_IRQ_STAT,
    I3C_IOCTL_CLR_IRQ_STAT,
    I3C_IOCTL_SET_IRQ_MASK,
    I3C_IOCTL_SET_SLV_RADDR,
    I3C_IOCTL_SET_SLV_WADDR,
    I3C_IOCTL_GET_SLV_ADDR,
    I3C_IOCTL_GET_FIFO_STAT,
    I3C_IOCTL_GET_ERROR_STAT,
    I3C_IOCTL_ENABLE_CLOCK,
    I3C_IOCTL_SET_DEF_ADDR,
    I3C_IOCTL_GET_ERR_ADDR,
    I3C_IOCTL_LOCK_SLEEP,
    I3C_IOCTL_SOFTRESET,
    I3C_IOCTL_GET_DEF_DA,
    I3C_IOCTL_SET_DEF_DA,
}i3c_ioctl_cmd_t;

enum {
    I3C_OP_STATE_INIT_MASK = (1 << 0),
    I3C_OP_STATE_BUSY_MASK = (1 << 1),
};


/** @brief This structure defines the configuration parameters for I3C transfer. */
typedef struct {
    hal_i3c_master_transfer_type_t trans_mode;  /**<  The I3C transfer type. */
    uint16_t    slave_addr;              /**<  The slave device address. */
    uint32_t    fast_speed_khz;          /**<  The speed for I3C fast speed transfer. */
    uint32_t    high_speed_khz;          /**<  The speed for I3C high speed transfer. */
    uint32_t    send_size;               /**<  The send size of transfer. */
    uint8_t    *send_buff;               /**<  The send data of transfer. */
    uint32_t    recv_size;               /**<  The receive size of transfer. */
    uint8_t    *recv_buff;               /**<  The receive buffer of tansfer. */
    hal_i3c_master_callback_t call_back;        /**<  The user callback function. */
    void        *user_data;              /**<  The user data. */
    uint32_t    o_flag;                  /**<  The option flag for transfer control. */
    uint8_t     ccc_cmd;                 /**<  The CCC for I3C protocol. */
    uint32_t    ccc_append_data_sz;      /**<  The CCC append data size. */
    uint8_t    *ccc_append_data;         /**<  The CCC append data. */
}i3c_master_transfer_config_t;



typedef enum {
    HAL_I3C_IO_CONFIG_PUSHPULL  = 0,          /**<  push-pull. */
    HAL_I3C_IO_CONFIG_OPENDRAIN   = 3,          /**<  open-drain. */
    HAL_I3C_IO_MAX                /**<  The total number of supported I2C IO config.*/
} hal_i3c_io_config_t;


#define I3C_ITEM_FLAG_IS_USED   (1<<31)
typedef struct {
    i3c_master_transfer_config_t user_config;
    uint32_t                         state;
} i3c_transfer_config_ex_t;



typedef struct {
    uint8_t                     idx;
    uint16_t                    fast_speed_khz;
    uint16_t                    high_speed_khz;
    uint32_t                    op_state;   /*record hw is busy or not*/
    hal_i3c_master_callback_t   call_back;
    void                       *user_data;
    uint32_t                    fifo_state;
    airo_queue_t                i3c_queue;
    hal_i3c_master_working_mode_t work_mode;
} i3c_private_info_t;

/** @brief This structure defines the slave device information. */
typedef struct {
    uint8_t PID[6];     /**<  The Provisionned ID of I3C slave device. */
    uint8_t BCR;        /**<  The Bus Characteristic Register of I3C slave device. */
    uint8_t DCR;        /**<  The Device Characteristic Register of I3C slave device. */
} hal_i3c_device_id_t;


typedef struct {
    int      head;
    int      tail;
    uint8_t *buff;
    uint32_t sz_buff;
}air_queue_t;

typedef enum {
    QUEUE_STATUS_OK                 =  0,
    QUEUE_STATUS_ERROR_INVALID_PARA = -1,
    QUEUE_STATUS_ERROR_FULL         = -2,
    QUEUE_STATUS_ERROR_EMPTY        = -3,
    QUEUE_STATUS_ERROR_NOT_ENOUGH         = -4,
}airo_queue_status_t;

/* For I3C API*/
//uint32_t                i3c_enum2num(hal_i2c_frequency_t freq);
hal_i3c_master_status_t i3c_config_i2c_speed(hal_i3c_master_port_t  i3c_port, uint32_t i3c_speed, uint32_t i3c_hs_speed);
hal_i3c_master_status_t i3c_config_i3c_speed(hal_i3c_master_port_t  i3c_port, uint32_t i3c_speed, uint32_t i3c_hs_speed);
void                    i3c_config_timing(hal_i3c_master_port_t  i2c_port, uint32_t t_min_timing_ns);
void                    i3c_config_io(hal_i3c_master_port_t  i3c_port,hal_i3c_io_config_t config);
int                     i3c_config_fifo(hal_i3c_master_port_t  i2c_port, i3c_fifo_op_t  op, uint8_t *buff, uint32_t size);
void                    i3c_hardware_reset(hal_i3c_master_port_t  i2c_port);
int                     i3c_config_hardware(hal_i3c_master_port_t  i3c_port, i3c_master_transfer_config_t *config);
void                    i3c_config_irq(hal_i3c_master_port_t  i3c_port, hal_nvic_isr_t callback);
int                     i3c_opt_ioctl(hal_i3c_master_port_t  i3c_port, uint32_t cmd, uint32_t option);
int                     i3c_get_port_by_nvic_id(hal_nvic_irq_t irq_number);

void                    hal_i3c_master_dump(hal_i3c_master_port_t  i2c_port);
hal_i3c_master_callback_event_t    i3c_get_event_type(uint32_t irq_status, uint32_t err_status);
int                     i3c_wait_status_with_timeout(hal_i3c_master_port_t i3c_port, uint32_t ms);
int                     i3c_pdma_init(hal_i3c_master_port_t i3c_port, i3c_master_transfer_config_t *config);
int                     i3c_pdma_deinit(hal_i3c_master_port_t i3c_port);

int                     air_queue_init(air_queue_t *pque,  uint8_t *buff, uint32_t size);
int                     air_queue_push(air_queue_t *pque,  uint8_t *buff, uint32_t size);
int                     air_queue_pop(air_queue_t *pque,    uint8_t *buff, uint32_t size);
int                     air_queue_top(air_queue_t *pque,    uint8_t *buff, uint32_t size);



#endif /* HAL_I3C_MODULE_ENBALED */
#endif /* __HAL_I3C_MASTER_INTERNAL_H__ */


