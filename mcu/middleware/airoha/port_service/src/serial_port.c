/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
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
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
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

#ifdef MTK_PORT_SERVICE_ENABLE

#include "hal_nvic.h"
#include "serial_port.h"
#include "serial_port_internal.h"

static uint8_t g_serial_port_open_count = 0;
/* record status of every serial port */
static bool g_serial_port_device_is_busy[SERIAL_PORT_DEV_MAX] = {false};

const serial_port_operation_t g_serial_port_handlers[SERIAL_PORT_TYPE_MAX] = {
    {
        serial_port_uart_init,
        serial_port_uart_control,
        serial_port_uart_deinit,
        serial_port_uart_reset_para,
    },
    {
        serial_port_usb_init,
        serial_port_usb_control,
        serial_port_usb_deinit,
        NULL,
    },
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    {
        serial_port_bt_init,
        serial_port_bt_control,
        serial_port_bt_deinit,
        NULL,
    },
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
#ifndef MTK_IAP2_VIA_MUX_ENABLE
    {
        serial_port_iap2_init,
        serial_port_iap2_control,
        serial_port_iap2_deinit,
        NULL,
    },
#endif
#endif
};

serial_port_type_t serial_port_get_device_type(serial_port_dev_t device)
{
    serial_port_type_t device_type;

    if (device <= SERIAL_PORT_DEV_UART_TYPE_END) {
        device_type = SERIAL_PORT_TYPE_UART;
    } else if ((device >= SERIAL_PORT_DEV_USB_TYPE_BEGIN) && (device <= SERIAL_PORT_DEV_USB_TYPE_END)) {
        device_type = SERIAL_PORT_TYPE_USB;
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    } else if ((device >= SERIAL_PORT_DEV_BT_TYPE_BEGIN) && (device <= SERIAL_PORT_DEV_BT_TYPE_END)) {
        device_type = SERIAL_PORT_TYPE_BT;
#endif
    }
#ifdef MTK_IAP2_PROFILE_ENABLE
    else if ((device >= SERIAL_PORT_DEV_IAP2_TYPE_BEGIN) && (device <= SERIAL_PORT_DEV_IAP2_TYPE_END)) {
        device_type = SERIAL_PORT_TYPE_IAP2;
    }
#endif
    else {
        device_type = SERIAL_PORT_TYPE_UART;
    }

    return device_type;
}

/*
 * bit0~bit7 - serial_port_dev_t
 * bit8~bit15 - distinguish different open event
 * bit16~bit23 - tag to distinguish whether it's valid
 * bit24~bit31 - DCL_UART_MAGIC_NUM
 */
static serial_port_handle_t port_return_handle(bool is_valid, serial_port_dev_t device)
{
    serial_port_handle_t handle;

    if (is_valid == true) {
        g_serial_port_open_count++;
        handle = (SERIAL_PORT_MAGIC_NUM << SERIAL_PORT_MAGIC_NUM_SHIFT) |
                 (SERIAL_PORT_STATUS_VALID << SERIAL_PORT_STATUS_SHIFT) |
                 (((uint32_t)g_serial_port_open_count) << SERIAL_PORT_OPEN_COUNT_SHIFT) |
                 device;
    } else {
        handle = (SERIAL_PORT_MAGIC_NUM << SERIAL_PORT_MAGIC_NUM_SHIFT) |
                 (SERIAL_PORT_STATUS_INVALID << SERIAL_PORT_STATUS_SHIFT) |
                 (((uint32_t)g_serial_port_open_count) << SERIAL_PORT_OPEN_COUNT_SHIFT) |
                 device;
    }

    return handle;
}

static bool handle_is_valid(serial_port_handle_t handle)
{
    if ((((handle >> SERIAL_PORT_MAGIC_NUM_SHIFT) & SERIAL_PORT_MAGIC_NUM_MASK) != SERIAL_PORT_MAGIC_NUM) ||
        (((handle >> SERIAL_PORT_STATUS_SHIFT) & SERIAL_PORT_STATUS_MARK) != SERIAL_PORT_STATUS_VALID)) {
        return false;
    }

    if ((handle & SERIAL_PORT_DEV_MASK) >= SERIAL_PORT_DEV_MAX) {
        return false;
    }

    return true;
}

serial_port_status_t serial_port_open(serial_port_dev_t device,
                                      serial_port_open_para_t *para,
                                      serial_port_handle_t *handle)
{
    uint32_t irq_status;
    serial_port_type_t dev_type;
    serial_port_dev_setting_t serial_port_dev_setting;
    serial_port_status_t status;

    /* parameters check */
    if (device >= SERIAL_PORT_DEV_MAX) {
        return SERIAL_PORT_STATUS_INVALID_DEVICE;
    }
    if ((para->callback == NULL) || (handle == NULL)) {
        return SERIAL_PORT_STATUS_INVALID_PARAMETER;
    }
#ifdef MTK_PORT_SERVICE_SLIM_ENABLE
    if ((para->tx_buffer_size == 0) || (para->rx_buffer_size == 0)) {
        return SERIAL_PORT_STATUS_INVALID_PARAMETER;
    }
#endif

    /* check and set busy status */
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (g_serial_port_device_is_busy[device] == true) {
        hal_nvic_restore_interrupt_mask(irq_status);
        return SERIAL_PORT_STATUS_BUSY;
    }
    g_serial_port_device_is_busy[device] = true;
    hal_nvic_restore_interrupt_mask(irq_status);

    /* check and fetch parameters needed by porting layer */
    dev_type = serial_port_get_device_type(device);
    serial_port_config_read_dev_setting(device, &serial_port_dev_setting);
    if (g_serial_port_handlers[dev_type].init == NULL) {
        status = SERIAL_PORT_STATUS_UNSUPPORTED;
        goto error;
    }
    status = g_serial_port_handlers[dev_type].init(device, para, &serial_port_dev_setting);
    if ((status != SERIAL_PORT_STATUS_OK) && (status != SERIAL_PORT_STATUS_DEV_NOT_READY)) {
        goto error;
    }

    *handle = port_return_handle(true, device);
    return status;

error:
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    *handle = port_return_handle(false, SERIAL_PORT_DEV_UNDEFINED);
    g_serial_port_device_is_busy[device] = false;
    hal_nvic_restore_interrupt_mask(irq_status);
    return status;
}

serial_port_status_t serial_port_control(serial_port_handle_t handle,
                                         serial_port_ctrl_cmd_t command,
                                         serial_port_ctrl_para_t *para)
{
    uint32_t irq_status;
    serial_port_dev_t device;
    serial_port_type_t dev_type;
    serial_port_status_t status;

    /* parameters check */
    if (handle_is_valid(handle) == false) {
        return SERIAL_PORT_STATUS_INVALID_PARAMETER;
    }
    if (command >= SERIAL_PORT_CMD_MAX) {
        return SERIAL_PORT_STATUS_INVALID_PARAMETER;
    }

    /* get device and check whether it's busy */
    device = (serial_port_dev_t)(handle & SERIAL_PORT_DEV_MASK);
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (g_serial_port_device_is_busy[device] == false) {
        hal_nvic_restore_interrupt_mask(irq_status);
        return SERIAL_PORT_STATUS_UNINITIALIZED;
    }
    hal_nvic_restore_interrupt_mask(irq_status);

    /* check and call porting layer control method */
    dev_type = serial_port_get_device_type(device);
    if (g_serial_port_handlers[dev_type].control == NULL) {
        return SERIAL_PORT_STATUS_UNSUPPORTED;
    }
    status = g_serial_port_handlers[dev_type].control(device, command, para);

    return status;
}

serial_port_status_t serial_port_close(serial_port_handle_t handle)
{
    uint32_t irq_status;
    serial_port_dev_t device;
    serial_port_type_t dev_type;
    serial_port_status_t status;

    /* parameters check */
    if (handle_is_valid(handle) == false) {
        return SERIAL_PORT_STATUS_INVALID_PARAMETER;
    }

    /* get device and check whether it's busy */
    device = (serial_port_dev_t)(handle & SERIAL_PORT_DEV_MASK);
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (g_serial_port_device_is_busy[device] == false) {
        hal_nvic_restore_interrupt_mask(irq_status);
        return SERIAL_PORT_STATUS_FAIL;
    }
    hal_nvic_restore_interrupt_mask(irq_status);

    /* check and call porting layer control method */
    dev_type = serial_port_get_device_type(device);
    if (g_serial_port_handlers[dev_type].deinit == NULL) {
        return SERIAL_PORT_STATUS_UNSUPPORTED;
    }
    status = g_serial_port_handlers[dev_type].deinit(device);
    if (status != SERIAL_PORT_STATUS_OK) {
        return status;
    }

    /* update status of the device */
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    g_serial_port_device_is_busy[device] = false;
    hal_nvic_restore_interrupt_mask(irq_status);

    return status;
}
#endif

