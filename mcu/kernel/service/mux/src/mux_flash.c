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

#include "mux.h"
#include "mux_port.h"
#include "mux_port_device.h"
#include "offline_dump.h"

#ifdef MTK_SINGLE_CPU_ENV
static volatile virtual_read_write_point_t mux_flash_r_w_point;
static volatile virtual_read_write_point_t *g_mux_flash_r_w_point = &mux_flash_r_w_point;
static uint32_t g_flash_log_start;
static volatile uint32_t *flash_log_start = &g_flash_log_start;
#else
static volatile virtual_read_write_point_t *g_mux_flash_r_w_point = (volatile virtual_read_write_point_t *)(HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START);
static volatile uint32_t *flash_log_start = (volatile uint32_t *)(HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START + sizeof(virtual_read_write_point_t));
#endif /* MTK_SINGLE_CPU_ENV */

#define LOG_TO_FLASH_IDLE        0x00
#define LOG_TO_FLASH_BUSY        0xFF
volatile uint32_t g_mux_flash_write_status = LOG_TO_FLASH_IDLE;

void internal_mux_flash_start_record(void)
{
    *flash_log_start = 0x1;
    log_config_print_switch(HAL_FLASH_MASK, DEBUG_LOG_OFF);
}

void internal_mux_flash_stop_record(void)
{
    *flash_log_start = 0x0;
    log_config_print_switch(HAL_FLASH_MASK, DEBUG_LOG_ON);
}

bool internal_mux_flash_is_record(void)
{
    if (*flash_log_start != 0x0) {
        return true;
    }

    return false;
}

#ifdef MTK_CPU_NUMBER_0

extern void offline_dump_callback_handle(offline_dump_region_type_t region_type);

void internal_mux_flash_write_flash(uint8_t port_index)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    uint32_t next_available_block_len, per_cpu_irq_mask;
    uint32_t cell_valid_size, flash_write_addr;
    uint32_t first_dump_size, wrap_dump_size;
    offline_dump_status_t status;

    /* check record is start */
    port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
    if (internal_mux_flash_is_record() == false) {
        port_mux_cross_local_exit_critical(per_cpu_irq_mask);
        return ;
    }
    g_mux_flash_write_status = LOG_TO_FLASH_BUSY;
    port_mux_cross_local_exit_critical(per_cpu_irq_mask);

    /* record mux port parameter */
    offline_dump_region_query_cell_valid_size(OFFLINE_REGION_OFFLINE_LOG, &cell_valid_size);

    /* record mux port parameter */
    port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
    next_available_block_len = mux_common_device_get_buf_next_available_block_len(p->tx_buff_start,
                                                                                    p->tx_buff_read_point,
                                                                                    p->tx_buff_write_point,
                                                                                    p->tx_buff_end,
                                                                                    p->tx_buff_available_len);

    /* limiting dump size */
    if (next_available_block_len >= cell_valid_size) {
        first_dump_size = cell_valid_size;
        wrap_dump_size = 0;
    } else {
        first_dump_size = next_available_block_len;
        wrap_dump_size = cell_valid_size - first_dump_size;
    }

    port_mux_cross_local_exit_critical(per_cpu_irq_mask);

    /* flash region alloc */
    status = offline_dump_region_alloc(OFFLINE_REGION_OFFLINE_LOG, &flash_write_addr);
    if ((status != OFFLINE_STATUS_OK) || (flash_write_addr == 0x0)) {
        g_mux_flash_write_status = LOG_TO_FLASH_IDLE;
        return ;
    }

    /* flash region write */
    status = offline_dump_region_write(OFFLINE_REGION_OFFLINE_LOG, flash_write_addr, (uint8_t *)p->tx_buff_read_point, first_dump_size);
    if (status != OFFLINE_STATUS_OK) {
        g_mux_flash_write_status = LOG_TO_FLASH_IDLE;
        return ;
    }

    /* flash region write wrap block */
    if (wrap_dump_size != 0) {
        flash_write_addr += first_dump_size;
        status = offline_dump_region_write(OFFLINE_REGION_OFFLINE_LOG, flash_write_addr, (uint8_t *)p->tx_buff_start, wrap_dump_size);
        if (status != OFFLINE_STATUS_OK) {
            g_mux_flash_write_status = LOG_TO_FLASH_IDLE;
            return ;
        }
    }

    /* flash region write end, write timestamp */
    status = offline_dump_region_write_end(OFFLINE_REGION_OFFLINE_LOG, cell_valid_size);
    if (status != OFFLINE_STATUS_OK) {
        g_mux_flash_write_status = LOG_TO_FLASH_IDLE;
        return ;
    }

    offline_dump_callback_handle(OFFLINE_REGION_OFFLINE_LOG);

    /* update tx buffer */
    port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
    mux_common_device_set_tx_hw_rptr_internal_use(p, cell_valid_size);
    port_mux_cross_local_exit_critical(per_cpu_irq_mask);

    g_mux_flash_write_status = LOG_TO_FLASH_IDLE;
}

mux_status_t port_mux_flash_normal_init(uint8_t port_index, mux_port_config_t *p_setting, mux_irq_handler_t irq_handler)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    PORT_MUX_UNUSED(irq_handler);

    if (port_index > (MUX_FLASH_END - MUX_FLASH_BEGIN)) {
        return MUX_STATUS_ERROR_PARAMETER;
    }

    if (p_setting->tx_buf_size == 0) {
        return MUX_STATUS_ERROR_PARAMETER;
    }

    mux_common_device_r_w_point_init(p, p_setting);

    /* Stop Record */
    internal_mux_flash_stop_record();

    return MUX_STATUS_OK;
}

mux_status_t port_mux_flash_deinit(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

void port_mux_flash_exception_init(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
}

void port_mux_flash_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    PORT_MUX_UNUSED(port_index);
    PORT_MUX_UNUSED(buffer);
    PORT_MUX_UNUSED(size);
}

bool port_mux_flash_get_buf_is_full(uint8_t port_index, bool is_rx)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    return mux_common_device_buf_is_full(p, is_rx);
}

#endif


ATTR_TEXT_IN_TCM uint32_t port_mux_flash_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    return mux_common_device_get_hw_rptr(p, is_rx);
}

ATTR_TEXT_IN_TCM uint32_t port_mux_flash_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    return mux_common_device_get_hw_wptr(p, is_rx);
}

ATTR_TEXT_IN_TCM void port_mux_flash_set_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    mux_common_device_set_rx_hw_rptr(p, move_bytes);
}

mux_status_t port_mux_flash_phase1_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

mux_status_t port_mux_flash_phase2_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

ATTR_TEXT_IN_TCM void port_mux_flash_set_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];

    /* update tx write pointer */
    mux_common_device_set_tx_hw_wptr(p, move_bytes);
    if (internal_mux_flash_is_record() == false) {
        mux_common_device_set_tx_hw_rptr_internal_use(p, move_bytes);
    }
}

mux_status_t port_mux_flash_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    uint32_t per_cpu_irq_mask;
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];

    if (port_index > (MUX_FLASH_END - MUX_FLASH_BEGIN)) {
        return MUX_STATUS_ERROR;
    }

    switch (command) {
        case MUX_CMD_GET_TX_SEND_STATUS: {
            if (g_mux_flash_write_status != LOG_TO_FLASH_IDLE) {
                return MUX_STATUS_ERROR_BUSY;
            }
        }
        break;

        case MUX_CMD_CONNECT: {
            internal_mux_flash_start_record();
        }
        break;

        case MUX_CMD_DISCONNECT: {
            internal_mux_flash_stop_record();
        }
        break;

        case MUX_CMD_GET_CONNECTION_PARAM: {
            uint32_t *p_mux_tx_avail = (uint32_t *)para;
            if (internal_mux_flash_is_record() == true) {
                *p_mux_tx_avail = 0x1;
            } else {
                *p_mux_tx_avail = 0x0;
            }
        }
        break;

        case MUX_CMD_GET_VIRTUAL_TX_AVAIL_LEN: {
            port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
            para->mux_get_tx_avail.ret_size = p->tx_buff_available_len;
            port_mux_cross_local_exit_critical(per_cpu_irq_mask);
        }
        break;

        case MUX_CMD_GET_VIRTUAL_RX_AVAIL_LEN: {
            port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
            para->mux_get_rx_avail.ret_size = p->rx_buff_available_len;
            port_mux_cross_local_exit_critical(per_cpu_irq_mask);
        }
        break;

        case MUX_CMD_TX_BUFFER_SEND: {
            internal_mux_flash_write_flash(port_index);
        }
        break;

        default:
            break;
    }

    return MUX_STATUS_OK;
}

port_mux_device_ops_t g_port_mux_flash_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_mux_flash_normal_init,
    port_mux_flash_deinit,
    port_mux_flash_exception_init,
    port_mux_flash_exception_send,
    port_mux_flash_get_buf_is_full,
#endif
    port_mux_flash_get_hw_rptr,
    port_mux_flash_set_hw_rptr,
    port_mux_flash_get_hw_wptr,
    port_mux_flash_set_hw_wptr,
    port_mux_flash_phase1_send,
    port_mux_flash_phase2_send,
    port_mux_flash_control,
    NULL,
    NULL,
};

