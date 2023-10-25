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
#include "mux_port_device.h"


#ifdef MTK_SINGLE_CPU_ENV
static volatile virtual_read_write_point_t mux_flash_r_w_point;
static volatile virtual_read_write_point_t *g_mux_flash_r_w_point = &mux_flash_r_w_point;
static uint32_t g_flash_log_start;
static volatile uint32_t *flash_log_start = &g_flash_log_start;
#else
#include "hal_resource_assignment.h"
static volatile virtual_read_write_point_t *g_mux_flash_r_w_point = (volatile virtual_read_write_point_t *)(HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START);
static volatile uint32_t *flash_log_start = (volatile uint32_t *)(HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START + sizeof(virtual_read_write_point_t));
#endif /* MTK_SINGLE_CPU_ENV */

bool internal_mux_flash_is_record(void)
{
    if (*flash_log_start != 0) {
        return true;
    }

    return false;
}

#ifdef MTK_CPU_NUMBER_0

mux_status_t port_mux_flash_normal_init(uint8_t port_index, mux_port_config_t *p_setting, mux_irq_handler_t irq_handler)
{
    PORT_MUX_UNUSED(port_index);
    PORT_MUX_UNUSED(irq_handler);

    if (port_index > (MUX_FLASH_END - MUX_FLASH_BEGIN)) {
        return MUX_STATUS_ERROR_PARAMETER;
    }

    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];

    mux_common_device_r_w_point_init(p, p_setting);

    return MUX_STATUS_OK;
}

mux_status_t port_mux_flash_deinit(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

bool port_mux_flash_get_buf_is_full(uint8_t port_index, bool is_rx)
{
    PORT_MUX_UNUSED(port_index);
    PORT_MUX_UNUSED(is_rx);

    return 0;
}

#endif


uint32_t port_mux_flash_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_rptr((virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index], is_rx);
}

uint32_t port_mux_flash_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_wptr((virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index], is_rx);
}

void port_mux_flash_set_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    mux_common_device_set_rx_hw_rptr((virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index], move_bytes);
}

void port_mux_flash_phase1_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return;
}

void port_mux_flash_phase2_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return;
}

void port_mux_flash_set_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    virtual_read_write_point_t *p = (virtual_read_write_point_t *)&g_mux_flash_r_w_point[port_index];
    mux_common_device_set_tx_hw_wptr(p, move_bytes);
    if (internal_mux_flash_is_record() == false) {
        mux_common_device_set_tx_hw_rptr_internal_use(p, move_bytes);
    }
}

mux_status_t port_mux_flash_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    PORT_MUX_UNUSED(command);
    PORT_MUX_UNUSED(para);

    if (port_index > (MUX_FLASH_END - MUX_FLASH_BEGIN)) {
        return MUX_STATUS_ERROR;
    }

    return MUX_STATUS_OK;
}

port_mux_device_ops_t g_port_mux_flash_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_mux_flash_normal_init,
    port_mux_flash_deinit,
    NULL,
    NULL,
    port_mux_flash_get_buf_is_full,
#endif
    port_mux_flash_get_hw_rptr,
    port_mux_flash_set_hw_rptr,
    port_mux_flash_get_hw_wptr,
    port_mux_flash_set_hw_wptr,
    port_mux_flash_phase1_send,
    port_mux_flash_phase2_send,
    port_mux_flash_control,
};

