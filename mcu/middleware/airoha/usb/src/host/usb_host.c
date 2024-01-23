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

#ifdef AIR_USB_HOST_ENABLE

#include "usb_dbg.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if FREERTOS_ENABLE
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_def.h"
#endif /* FREERTOS_ENABLE */

/* USB fuction includes */
#include "usb_host.h"
#include "hal_usb_host.h"
#include "usb_host_def.h"
#include "usb_host_utility.h"
#include "usb_host_device_op.h"



#include "hal_pinmux_define.h"
#include "hal_gpio.h"
#include "hal_cache.h"
#include "memory_attribute.h"
#include "hal_resource_assignment.h"
#include "hal_sleep_manager_platform.h"






static TaskHandle_t usb_host_task_handle = NULL;


usb_host_handle g_HostHandle;
usb_host_instance_t g_UsbHostInstance;

#if 0
ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t inquiry_cbw[] = {
    0x55, 0x53, 0x42, 0x43, 0xc0, 0x99, 0xfe, 0x61,
    0x24, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x12,
    0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#else

ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t cbw[31];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t inquiry_data[36];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t format_capacity_data[12];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t read_capacity_data[8];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t mode_sense_data[4];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t csw[13];

uint8_t *buff_uc ;

uint8_t csw_rx_data[13];
uint8_t bulk_in_data[512];



#endif

#if 0
uint8_t inquiry_cbw_t[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
#else
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t inquiry_cbw_t[512];

#endif

static usb_host_instance_t *USB_Host_Get_Host_Instance(void)
{
    return &g_UsbHostInstance;
}

usb_host_status_t USB_Host_Get_Device_Address(void)
{
    uint8_t addr;
    addr = hal_usb_host_get_address();
    LOG_MSGID_I(USB_MAIN, "Device Address : %d", 1, addr);

    return Status_USB_Success;
}

uint8_t USB_Host_Allocate_DeviceAddress(usb_host_handle hostHandle)
{
    uint8_t index = 0, address, found = 0 ;
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;

    for (index = 0 ; index < 16 ; index++) {
        if (host_instance->addressBitMap[index] == 0) { //Available
            break;
        }
    }

    if (index == 16) {
        LOG_MSGID_E(USB_MAIN, "Device number is full", 0);
    }

    for (address = 1 ; address < 32 ; address++) {
        for (uint8_t j = 0 ; j < 16 ; j++) {
            if (host_instance->addressBitMap[j] == address) { // try next address
                break;
            }
            if (j == 15) {
                found = 1 ;
            }
        }
        if (found == 1) {
            break;
        }
    }

    LOG_MSGID_I(USB_MAIN, "allocate device address number is %d, position is %d", 2, address, index);
    host_instance->addressBitMap[index] = address;

    return address;
}

usb_host_status_t USB_Host_Enum_ProcessCallback(usb_device_instance_t *device_instance, uint16_t dataLength)
{
    usb_host_descriptor_configuration_t *configureDesc;

    LOG_MSGID_I(USB_MAIN, "Enum_ProcessCallback data len :%d", 1, dataLength);
    switch (device_instance->enum_state) {
        case USB_HOST_DEV_Status_GetDevDes:

            memcpy((uint8_t *)(device_instance->deviceDescriptor), device_instance->Control_Pipe.buff, sizeof(usb_host_descriptor_device_t));
            LOG_MSGID_I(USB_MAIN, "Vendor ID %x %x", 2, device_instance->deviceDescriptor->idVendor[0], device_instance->deviceDescriptor->idVendor[1]);

            break;

        case USB_HOST_DEV_Status_SetAddress:

            device_instance->setAddress = device_instance->allocatedAddress;
            // write new address to IC register
            LOG_MSGID_I(USB_MAIN, "Update Device Address", 0);
            hal_usb_host_set_address(device_instance->setAddress);

            break;

        case USB_HOST_DEV_Status_GetCfg9:

            configureDesc = (usb_host_descriptor_configuration_t *)(device_instance->Control_Pipe.buff);
            //LOG_MSGID_I(USB_MAIN, "configuration bLength %d", 1, configureDesc->bLength);
            //LOG_MSGID_I(USB_MAIN, "configuration bDescriptorType %x", 1, configureDesc->bDescriptorType);
            //LOG_MSGID_I(USB_MAIN, "configuration wTotalLength %x", 1, configureDesc->wTotalLength[0]);
            device_instance->configurationLen = (configureDesc->wTotalLength[0] + (configureDesc->wTotalLength[1] << 8)) ;
            LOG_MSGID_I(USB_MAIN, "Total configuration size %d", 1, device_instance->configurationLen);

            break;

        case USB_HOST_DEV_Status_SetCfg:

            LOG_MSGID_I(USB_MAIN, "Set Configuration Done", 0);

            break;

        default:

            break;
    }

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Update_Control_Transaction_State(usb_host_ctrl_req_t *ctrl_pipe)
{
    uint8_t direction;

    direction = ctrl_pipe->setup.bmRequestType & USB_REQ_DIR_MASK;

    switch (ctrl_pipe->state) {
        case CTRL_SETUP:

            /* check if there is a data stage */
            if (ctrl_pipe->setup.wLength != 0U) {
                if (direction == USB_D2H) {
                    /* Data Direction is IN */
                    ctrl_pipe->state = CTRL_DATA_IN;
                } else {
                    /* Data Direction is OUT */
                    ctrl_pipe->state = CTRL_DATA_OUT;
                }
            }
            /* No DATA stage */
            else {
                /* If there is No Data Transfer Stage */
                if (direction == USB_D2H) {
                    /* Data Direction is IN */
                    ctrl_pipe->state = CTRL_STATUS_OUT;
                } else {
                    /* Data Direction is OUT */
                    ctrl_pipe->state = CTRL_STATUS_IN;
                }
            }

            break;

        case CTRL_DATA_IN:

            ctrl_pipe->state = CTRL_STATUS_OUT;

            break;

        case CTRL_DATA_OUT:

            ctrl_pipe->state = CTRL_STATUS_IN;

            break;

        case CTRL_STATUS_IN:

            ctrl_pipe->state = CTRL_STATUS_IN_WAIT;

            break;

        case CTRL_STATUS_OUT:

            ctrl_pipe->state = CTRL_STATUS_OUT_WAIT;

            break;

        case CTRL_STATUS_IN_WAIT:
        case CTRL_STATUS_OUT_WAIT:

            ctrl_pipe->state = CTRL_SETUP;

            break;
    }

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Fill_Setup_Packet(usb_device_instance_t *device_Instance, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
    //usb_device_instance_t *device_Instance = (usb_device_instance_t *)deviceHandle;

    LOG_MSGID_I(USB_MAIN, "Fill_Setup_Packet", 0);
    device_Instance->Control_Pipe.setup.bmRequestType = bmRequestType;
    device_Instance->Control_Pipe.setup.bRequest = bRequest;

    device_Instance->Control_Pipe.setup.wValue = wValue;
    device_Instance->Control_Pipe.setup.wIndex = wIndex;
    device_Instance->Control_Pipe.setup.wLength = wLength;

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Control_Transfer_State_Machine(usb_host_handle hostHandle)
{
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;
    usb_device_instance_t *device_instance = (usb_device_instance_t *)(host_instance->deviceList);

    uint32_t ep0_rx_len;


    switch (device_instance->Control_Pipe.state) {
        case CTRL_SETUP:

            LOG_MSGID_I(USB_MAIN, "CTRL SETUP Stage", 0);
            //LOG_MSGID_I(USB_MAIN, "Setup Packet bmRequestType %x", 1, device_instance->Control_Pipe.setup.bmRequestType);
            //LOG_MSGID_I(USB_MAIN, "Setup Packet bRequest %x", 1, device_instance->Control_Pipe.setup.bRequest);
            //LOG_MSGID_I(USB_MAIN, "Setup Packet wValue %x", 1, device_instance->Control_Pipe.setup.wValue);
            //LOG_MSGID_I(USB_MAIN, "Setup Packet wLength %x", 1, device_instance->Control_Pipe.setup.wLength);
            hal_usb_host_issue_setup_command((void *) & (device_instance->Control_Pipe.setup), 8);
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));

            break;

        case CTRL_DATA_IN:

            // Issue control-In
            LOG_MSGID_I(USB_MAIN, "CTRL Data In Stage", 0);
            hal_usb_host_set_ep0_rx_request();
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));

            break;

        case CTRL_DATA_OUT:

            break;

        case CTRL_STATUS_OUT:

            // Cpoy Received data from FIFO & send status packet
            //ret = hal_usb_host_check_ep0_rx_ready();
            ep0_rx_len = hal_usb_host_get_ep0_pkt_len();
            LOG_MSGID_I(USB_MAIN, "EP0 Rx Pkt size", 1, ep0_rx_len);
            hal_usb_host_read_endpoint_fifo(0, ep0_rx_len, device_instance->Control_Pipe.buff);
            device_instance->Control_Pipe.length = ep0_rx_len;
            LOG_MSGID_I(USB_MAIN, "EP0 Rx Pkt Content [%x][%x][%x][%x]", 4, device_instance->Control_Pipe.buff[0], device_instance->Control_Pipe.buff[1], device_instance->Control_Pipe.buff[2], device_instance->Control_Pipe.buff[3]);
            hal_usb_host_clear_ep0_rx_ready();
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));
            hal_usb_host_set_ep0_out_status();

            break;

        case CTRL_STATUS_OUT_WAIT:

            // Status phase done
            LOG_MSGID_I(USB_MAIN, "CTRL Status Out Stage", 0);
            device_instance->enum_callback(device_instance, device_instance->Control_Pipe.length);

            device_instance->enum_state++;
            device_instance->Control_Pipe.length = 0 ;
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));
            USB_Host_Enumeration_Proccess(hostHandle);

            break;

        case CTRL_STATUS_IN:

            hal_usb_host_set_ep0_in_status();
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));

            break;

        case CTRL_STATUS_IN_WAIT:

            // Status phase done
            LOG_MSGID_I(USB_MAIN, "CTRL Status In Stage", 0);
            hal_usb_host_clear_ep0_rx_status();

            device_instance->enum_callback(device_instance, device_instance->Control_Pipe.length);

            device_instance->enum_state++;
            USB_Host_Update_Control_Transaction_State(&(device_instance->Control_Pipe));
            USB_Host_Enumeration_Proccess(hostHandle);

            break;
    }

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Enumeration_Proccess(usb_host_handle hostHandle)
{
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;
    usb_device_instance_t *device_instance = (usb_device_instance_t *)(host_instance->deviceList);

    //uint16_t rx_len ;

    switch (device_instance->enum_state) {
        case USB_HOST_DEV_Status_GetDevDes:

            LOG_MSGID_I(USB_MAIN, "Enum : Get Dev Descriptor", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_GET_DESCRIPTOR, USB_DESC_DEVICE, 0, 64);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_SetAddress:

            LOG_MSGID_I(USB_MAIN, "Enum : Set Address", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_SET_ADDRESS, device_instance->allocatedAddress, 0, 0);
            }
            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_GetCfg9:

            LOG_MSGID_I(USB_MAIN, "Enum : Get Config Descriptor 9", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_GET_DESCRIPTOR, USB_DESC_CONFIGURATION, 0, 9);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_GetCfg:

            LOG_MSGID_I(USB_MAIN, "Enum : Get Config Descriptor ", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_GET_DESCRIPTOR, USB_DESC_CONFIGURATION, 0, device_instance->configurationLen);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_GetProductString:

            LOG_MSGID_I(USB_MAIN, "Enum : Get Product String ", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_GET_DESCRIPTOR, (USB_DESC_STRING | 0x02), 0x0409, 255);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_GetSerialNumber:

            LOG_MSGID_I(USB_MAIN, "Enum : Get Serial Number String ", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_D2H | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_GET_DESCRIPTOR, (USB_DESC_STRING | 0x03), 0x0409, 255);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_SetCfg:

            LOG_MSGID_I(USB_MAIN, "Enum : Set Configuration ", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, (USB_H2D | USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_STANDARD), USB_REQ_SET_CONFIGURATION, 0x01, 0, 0);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_GetMax_Lun:

            LOG_MSGID_I(USB_MAIN, "MSC : Get Max Lun ", 0);
            if (device_instance->Control_Pipe.state == CTRL_SETUP) {
                USB_Host_Fill_Setup_Packet(device_instance, 0xA1, 0xFE, 0, 0, 1);
            }

            USB_Host_Control_Transfer_State_Machine(hostHandle);

            break;

        case USB_HOST_DEV_Status_Inquiry_Cbw:

            LOG_MSGID_I(USB_MAIN, "MSC : Inquiry Cbw ", 0);
            hal_usb_host_tx_ep_init(2, 0x200, HAL_USB_HOST_EP_TRANSFER_BULK, false);

            uint8_t inquiry_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0xc0, 0x99, 0xfe, 0x61,
                0x24, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x12,
                0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            //memset(inquiry_cbw_t, 0x02, 512);
            memcpy(cbw, inquiry_cbw, 31);

            //hal_cache_flush_multiple_cache_lines((uint32_t)inquiry_cbw_t, 512);
            //hal_cache_flush_all_cache_lines();

            device_instance->enum_state++;
            //device_instance->enum_state = USB_HOST_DEV_Status_EnumDone;
            //hal_usb_host_write_ep_fifo(2, 31, inquiry_cbw);
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Inquiry_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Inquiry Data ", 0);
            hal_usb_host_rx_ep_init(1, 0x200, HAL_USB_HOST_EP_TRANSFER_BULK, false);
            device_instance->enum_state++;
            //device_instance->enum_state = USB_HOST_DEV_Status_EnumDone;
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)inquiry_data), 36);
            //hal_usb_host_set_ep_rx_request(1);

            break;

        case USB_HOST_DEV_Status_Inquiry_Csw_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Inquiry Csw ", 0);
            //uint16_t rx_len = hal_usb_host_get_ep_rx_pkt_len(1);
            //LOG_MSGID_I(USB_MAIN, "MSC : Rx Inquiry Len : %d", 1, rx_len);
            uint8_t inquiry_rx_data[36];
            memcpy(inquiry_rx_data, inquiry_data, 36);
            //hal_usb_host_read_endpoint_fifo(1, rx_len, inquiry_rx_data);
            LOG_MSGID_I(USB_MAIN, "Inquiry Rx Data %x %x %x %x %x %x %x %x ", 8, inquiry_rx_data[0], inquiry_rx_data[1], inquiry_rx_data[2], inquiry_rx_data[3], inquiry_rx_data[4], inquiry_rx_data[5], inquiry_rx_data[6], inquiry_rx_data[7]);
            LOG_MSGID_I(USB_MAIN, "Inquiry Rx Data %x %x %x %x %x %x %x %x ", 8, inquiry_rx_data[8], inquiry_rx_data[9], inquiry_rx_data[10], inquiry_rx_data[11], inquiry_rx_data[12], inquiry_rx_data[13], inquiry_rx_data[14], inquiry_rx_data[15]);
            LOG_MSGID_I(USB_MAIN, "Inquiry Rx Data %x %x %x %x %x %x %x %x ", 8, inquiry_rx_data[16], inquiry_rx_data[17], inquiry_rx_data[18], inquiry_rx_data[19], inquiry_rx_data[20], inquiry_rx_data[21], inquiry_rx_data[22], inquiry_rx_data[23]);
            LOG_MSGID_I(USB_MAIN, "Inquiry Rx Data %x %x %x %x %x %x %x %x ", 8, inquiry_rx_data[24], inquiry_rx_data[25], inquiry_rx_data[26], inquiry_rx_data[27], inquiry_rx_data[28], inquiry_rx_data[29], inquiry_rx_data[30], inquiry_rx_data[31]);
            LOG_MSGID_I(USB_MAIN, "Inquiry Rx Data %x %x %x %x ", 4, inquiry_rx_data[32], inquiry_rx_data[33], inquiry_rx_data[34], inquiry_rx_data[35]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);
            device_instance->enum_state++;
            //device_instance->enum_state = USB_HOST_DEV_Status_EnumDone;
            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)csw), 13);

            break;

        case USB_HOST_DEV_Status_Format_Capacity_Cbw:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Format Capacity Cbw ", 0);
            //uint16_t rx_csw_len = hal_usb_host_get_ep_rx_pkt_len(1);
            //LOG_MSGID_I(USB_MAIN, "MSC : Rx Inquiry Len : %d", 1, rx_csw_len);

            memcpy(csw_rx_data, csw, 13);
            //hal_usb_host_read_endpoint_fifo(1, rx_csw_len, csw_rx_data);
            LOG_MSGID_I(USB_MAIN, "Inquiry CSW Rx Data %x %x %x %x %x %x %x %x ", 8, csw_rx_data[0], csw_rx_data[1], csw_rx_data[2], csw_rx_data[3], csw_rx_data[4], csw_rx_data[5], csw_rx_data[6], csw_rx_data[7]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);

            uint8_t format_capacity_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0xc0, 0x99, 0xfe, 0x61,
                0xfc, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0A, 0x23,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            memcpy(cbw, format_capacity_cbw, 31);
            device_instance->enum_state++;
            //hal_usb_host_write_ep_fifo(2, 31, format_capacity_cbw);
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Format_Capacity_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Format Capacity Data ", 0);

            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            device_instance->enum_state++;
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)format_capacity_data), 12);

            break;

        case USB_HOST_DEV_Status_Format_Capacity_Csw_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Format Capacity Csw ", 0);
            //uint16_t rx_len = hal_usb_host_get_ep_rx_pkt_len(1);
            //LOG_MSGID_I(USB_MAIN, "MSC : Rx Inquiry Len : %d", 1, rx_len);
            uint8_t format_capacity_rx_data[12];
            memcpy(format_capacity_rx_data, format_capacity_data, 12);
            //hal_usb_host_read_endpoint_fifo(1, rx_len, inquiry_rx_data);
            LOG_MSGID_I(USB_MAIN, "Format Capacity Rx Data %x %x %x %x %x %x %x %x ", 8, format_capacity_rx_data[0], format_capacity_rx_data[1], format_capacity_rx_data[2], format_capacity_rx_data[3], format_capacity_rx_data[4], format_capacity_rx_data[5], format_capacity_rx_data[6], format_capacity_rx_data[7]);

            hal_usb_host_clear_ep_rx_pkt_ready(1);
            device_instance->enum_state++;
            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)csw), 13);

            break;

        case USB_HOST_DEV_Status_Read_Capacity_Cbw:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Capacity Cbw ", 0);
            memcpy(csw_rx_data, csw, 13);
            //hal_usb_host_read_endpoint_fifo(1, rx_csw_len, csw_rx_data);
            LOG_MSGID_I(USB_MAIN, "Format Capacity CSW Rx Data %x %x %x %x %x %x %x %x ", 8, csw_rx_data[0], csw_rx_data[1], csw_rx_data[2], csw_rx_data[3], csw_rx_data[4], csw_rx_data[5], csw_rx_data[6], csw_rx_data[7]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);

            uint8_t read_capacity_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0xa0, 0x49, 0xc3, 0x55,
                0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0A, 0x25,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            memcpy(cbw, read_capacity_cbw, 31);
            device_instance->enum_state++;
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Read_Capacity_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Capacity Data ", 0);

            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            device_instance->enum_state++;
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)read_capacity_data), 8);

            break;

        case USB_HOST_DEV_Status_Read_Capacity_Csw_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read Capacity Csw ", 0);

            uint8_t read_capacity_rx_data[8];
            memcpy(read_capacity_rx_data, read_capacity_data, 8);
            //hal_usb_host_read_endpoint_fifo(1, rx_len, inquiry_rx_data);
            LOG_MSGID_I(USB_MAIN, "Read Capacity Rx Data %x %x %x %x %x %x %x %x ", 8, read_capacity_rx_data[0], read_capacity_rx_data[1], read_capacity_rx_data[2], read_capacity_rx_data[3], read_capacity_rx_data[4], read_capacity_rx_data[5], read_capacity_rx_data[6], read_capacity_rx_data[7]);

            hal_usb_host_clear_ep_rx_pkt_ready(1);
            device_instance->enum_state++;
            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)csw), 13);

            break;

        case USB_HOST_DEV_Status_Mode_Sense_Cbw:

            LOG_MSGID_I(USB_MAIN, "MSC : Mode Sense Cbw ", 0);
            memcpy(csw_rx_data, csw, 13);
            //hal_usb_host_read_endpoint_fifo(1, rx_csw_len, csw_rx_data);
            LOG_MSGID_I(USB_MAIN, "Read Capacity CSW Rx Data %x %x %x %x %x %x %x %x ", 8, csw_rx_data[0], csw_rx_data[1], csw_rx_data[2], csw_rx_data[3], csw_rx_data[4], csw_rx_data[5], csw_rx_data[6], csw_rx_data[7]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);

            uint8_t mode_sense_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0xe0, 0x49, 0x52, 0x6b,
                0xc0, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x1a,
                0x00, 0x1c, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            memcpy(cbw, mode_sense_cbw, 31);
            device_instance->enum_state++;
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Mode_Sense_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Mode Sense Data ", 0);

            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            device_instance->enum_state++;
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)mode_sense_data), 4);

            break;

        case USB_HOST_DEV_Status_Mode_Sense_Csw_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Mode Sense Csw ", 0);

            uint8_t mode_sense_rx_data[8];
            memcpy(mode_sense_rx_data, mode_sense_data, 4);
            //hal_usb_host_read_endpoint_fifo(1, rx_len, inquiry_rx_data);
            LOG_MSGID_I(USB_MAIN, "Mode Sense Rx Data %x %x %x %x", 4, mode_sense_rx_data[0], mode_sense_rx_data[1], mode_sense_rx_data[2], mode_sense_rx_data[3]);

            hal_usb_host_clear_ep_rx_pkt_ready(1);
            device_instance->enum_state++;
            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(2);    // wait for Recieve data in FIFO finished
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)csw), 13);

            break;

        case USB_HOST_DEV_Status_Write_10:

            LOG_MSGID_I(USB_MAIN, "MSC : Write 10 Cbw ", 0);

            memcpy(csw_rx_data, csw, 13);
            LOG_MSGID_I(USB_MAIN, "Mode Sense CSW Rx Data %x %x %x %x %x %x %x %x ", 8, csw_rx_data[0], csw_rx_data[1], csw_rx_data[2], csw_rx_data[3], csw_rx_data[4], csw_rx_data[5], csw_rx_data[6], csw_rx_data[7]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);

            uint8_t write_10_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0x20, 0xda, 0x4d, 0xa7,
                0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x2A,
                0x00, 0x00, 0x00, 0x20, 0x80, 0x00, 0x00, 0x08,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            memcpy(cbw, write_10_cbw, 31);
            device_instance->enum_state++;
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Write_10_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Write 10 data ", 0);

            buff_uc = (uint8_t *)pvPortMallocNC(4096);
            memset(buff_uc, 0x08, 4096);
            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            //device_instance->enum_state++;
            device_instance->enum_state = USB_HOST_DEV_Status_EnumDone;

            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)buff_uc), 4096);

            //hal_usb_host_set_ep_tx_ready(2);

            hal_gpt_delay_ms(150);   // wait for DMA finished, but need to modify according to transfer size

            break;

        case USB_HOST_DEV_Status_Read_10:

            LOG_MSGID_I(USB_MAIN, "MSC : Read 10 Cbw ", 0);

            memcpy(csw_rx_data, csw, 13);
            LOG_MSGID_I(USB_MAIN, "Mode Sense CSW Rx Data %x %x %x %x %x %x %x %x ", 8, csw_rx_data[0], csw_rx_data[1], csw_rx_data[2], csw_rx_data[3], csw_rx_data[4], csw_rx_data[5], csw_rx_data[6], csw_rx_data[7]);
            hal_usb_host_clear_ep_rx_pkt_ready(1);


            uint8_t read_10_cbw[] = {
                0x55, 0x53, 0x42, 0x43, 0xc0, 0x79, 0xad, 0x67,
                0x00, 0x10, 0x00, 0x00, 0x80, 0x00, 0x0A, 0x28,
                0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x08,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            memcpy(cbw, read_10_cbw, 31);
            device_instance->enum_state++;
            hal_usb_host_start_dma_transfer(2, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_TX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)cbw), 31);
            hal_usb_host_set_ep_tx_ready(2);

            break;

        case USB_HOST_DEV_Status_Read_10_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read 10 data ", 0);

            buff_uc = (uint8_t *)pvPortMallocNC(4096);

            hal_usb_host_clear_rxcsr(1);    // clear previous dma request setting then ep interrupt can be generated
            device_instance->enum_state++;
            //hal_usb_host_set_ep_rx_request(1);

            //hal_gpt_delay_ms(400);    // wait for Recieve data in FIFO finished
            hal_usb_host_set_rx_pkt_count(8);   //4K data
            hal_usb_host_start_dma_transfer(1, USB_HOST_SPEED_HIGH, HAL_USB_HOST_EP_DIRECTION_RX, HAL_USB_HOST_EP_TRANSFER_BULK, hal_memview_mcu_to_infrasys((uint32_t)buff_uc), 4096);

            hal_usb_host_set_ep_rx_request(1);
            hal_gpt_delay_ms(150);   // wait for DMA finished, but need to modify according to transfer size

            LOG_MSGID_I(USB_MAIN, "Read 10 Rx Data %x %x %x %x %x %x %x %x ", 8, buff_uc[0], buff_uc[1], buff_uc[2], buff_uc[3], buff_uc[4], buff_uc[5], buff_uc[6], buff_uc[7]);
            LOG_MSGID_I(USB_MAIN, "Read 10 Rx Data %x %x %x %x %x %x %x %x ", 8, buff_uc[510], buff_uc[511], buff_uc[512], buff_uc[513], buff_uc[514], buff_uc[515], buff_uc[516], buff_uc[517]);
            LOG_MSGID_I(USB_MAIN, "Read 10 Rx Data %x %x %x %x %x %x %x %x ", 8, buff_uc[4088], buff_uc[4089], buff_uc[4090], buff_uc[4091], buff_uc[4092], buff_uc[4093], buff_uc[4094], buff_uc[4095]);

            break;

        case USB_HOST_DEV_Status_Read_10_Csw_Data:

            LOG_MSGID_I(USB_MAIN, "MSC : Read 10 CSW ", 0);
            device_instance->enum_state = USB_HOST_DEV_Status_EnumDone;
            LOG_MSGID_I(USB_MAIN, "Read 10 Rx Data %x %x %x %x %x %x %x %x ", 8, buff_uc[0], buff_uc[1], buff_uc[2], buff_uc[3], buff_uc[4], buff_uc[5], buff_uc[6], buff_uc[7]);
            LOG_MSGID_I(USB_MAIN, "Read 10 Rx Data %x %x %x %x %x %x %x %x ", 8, buff_uc[510], buff_uc[511], buff_uc[512], buff_uc[513], buff_uc[514], buff_uc[515], buff_uc[516], buff_uc[517]);

            break;

    }

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Control_Transfer_Proccess(usb_host_handle hostHandle)
{
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;

    if (((usb_device_instance_t *)(host_instance->deviceList))->enum_state != USB_HOST_DEV_Status_EnumDone) {
        USB_Host_Enumeration_Proccess(hostHandle);
    }
    //else
    //    USB_Host_Control_Transfer_State_Machine(hostHandle);

    return Status_USB_Success;
}

usb_host_status_t USB_Host_Attach_Device(usb_host_handle hostHandle, uint8_t speed, uint8_t hubNumber, uint8_t portNumber, usb_device_handle *deviceHandle)
{
    usb_device_instance_t *newInstance;
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;
    uint32_t allocate_address;

    //((usb_host_instance_t *)hostHandle)->gState = HOST_ENUMERATION ;
    host_instance->gState = HOST_ENUMERATION ;

    /* Allocate new device instance */
    newInstance = (usb_device_instance_t *)pvPortMalloc(sizeof(usb_device_instance_t));
    if (newInstance == NULL) {
        LOG_MSGID_E(USB_MAIN, "Allocate Device Instance Failed!! ", 0);
    }
    allocate_address = USB_Host_Allocate_DeviceAddress(hostHandle);

    /* new instance fields init */
    newInstance->hostHandle = hostHandle;
    newInstance->speed = speed;
    newInstance->setAddress = 0;
    newInstance->allocatedAddress = allocate_address;
    newInstance->enum_state = USB_HOST_DEV_Status_Initial;
    newInstance->enum_callback = USB_Host_Enum_ProcessCallback;

    host_instance->deviceList = newInstance;
    //newInstance->deviceAttachState = Status_device_Attached;
    // malloc device descriptor and 9 byte configuration descriptor
    newInstance->deviceDescriptor = (usb_host_descriptor_device_t *)pvPortMalloc(sizeof(usb_host_descriptor_device_t) + sizeof(usb_host_descriptor_configuration_t));


    *deviceHandle = newInstance;
    return Status_USB_Success;
}

static void USB_Host_Attach_Proccess(usb_host_handle hostHandle, USBH_SpeedTypeDef speed)
{
    usb_device_handle deviceHandle;
    //usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;

    USB_Host_Attach_Device(hostHandle, speed, 0U, 0U, &deviceHandle);
    hal_usb_host_set_address(0);

    /* start enumeration */
    ((usb_device_instance_t *)deviceHandle)->enum_state = USB_HOST_DEV_Status_GetDevDes;
    ((usb_device_instance_t *)deviceHandle)->Control_Pipe.state = CTRL_SETUP ;
    USB_Host_Enumeration_Proccess(hostHandle);
}

static void USB_Host_Detach_Proccess(usb_host_handle hostHandle)
{
    usb_host_instance_t *host_instance = (usb_host_instance_t *)hostHandle;

    LOG_MSGID_I(USB_MAIN, "Detach_Proccess", 0);

    memset(host_instance->addressBitMap, 0, sizeof(host_instance->addressBitMap));
    vPortFree(((usb_device_instance_t *)(host_instance->deviceList))->deviceDescriptor);
    vPortFree((host_instance->deviceList));
    host_instance->deviceList = NULL ;

}

static usb_host_status_t USB_Host_Event(usb_device_handle deviceHandle, usb_host_configuration_handle configurationHandle, uint32_t eventCode)
{
    usb_host_status_t status = Status_USB_Success;

    return status;
}

usb_host_status_t USB_HostInit(usb_host_handle *hostHandle, host_callback_t callback_Fn)
{
    usb_host_status_t status = Status_USB_Success;
    usb_host_instance_t *host_instance = NULL;

    host_instance = USB_Host_Get_Host_Instance();

    /* HOST instance init*/
    host_instance->deviceCallback = callback_Fn;
    host_instance->deviceList     = NULL;

    *hostHandle = host_instance;

    return status;
}

static void USB_HostTask(void *hostHandle)
{
    usb_host_msg_t      msgs;
    USBH_SpeedTypeDef   speed;

    while (1) {
        vTaskDelay(5);
        Usb_Host_Receieve_Message(&msgs);
        LOG_MSGID_I(USB_MAIN, "USB Host Rx Message ID  = %d", 1, msgs.host_msg_id);

        switch (msgs.host_msg_id) {
            case USB_HOST_ATTACH_MSG:

                LOG_MSGID_I(USB_MAIN, "Rx Attach Message", 0);
                hal_usb_host_reset();
                speed = hal_usb_host_check_attach_device_speed();
                USB_Host_Attach_Proccess(hostHandle, speed);
                break;

            case USB_HOST_DETACH_MSG:

                LOG_MSGID_I(USB_MAIN, "Rx detattach Message", 0);
                USB_Host_Detach_Proccess(hostHandle);

            case USB_CONTROL_PIPE_MSG:

                LOG_MSGID_I(USB_MAIN, "Control pipe Tx/Rx Done Message", 0);
                USB_Host_Control_Transfer_Proccess(hostHandle);

                break;

            case USB_EP_TX_MSG:

                LOG_MSGID_I(USB_MAIN, "EP Tx Done Message", 0);
                USB_Host_Control_Transfer_Proccess(hostHandle);

                break;

            case USB_EP_RX_MSG:

                LOG_MSGID_I(USB_MAIN, "EP Rx Done Message", 0);
                USB_Host_Control_Transfer_Proccess(hostHandle);

                break;

            case USB_DMA_DONE_MSG:

                LOG_MSGID_I(USB_MAIN, "DMA Done Message", 0);
                USB_Host_Control_Transfer_Proccess(hostHandle);

                break;

            default:
                break;
        }
    }
}

bool Usb_Host_task_init(void)
{

    if (xTaskCreate(USB_HostTask, "UsbHost", (USB_TASK_STACKSIZE * 2) / sizeof(portSTACK_TYPE), g_HostHandle, USB_TASK_PRIO, &usb_host_task_handle) != pdPASS) {
        LOG_MSGID_E(USB_MAIN, "Create USB_HostTask fail!! ", 0);
        return false;
    } else {
        LOG_MSGID_I(USB_MAIN, "Create USB_HostTask successfully", 0);
    }

    return true;
}

void Usb_Host_start(void)
{
    hal_pinmux_status_t ret ;

    LOG_MSGID_I(USB_MAIN, "USB Host Start", 0);
    ret = hal_pinmux_set_function(HAL_GPIO_56, HAL_GPIO_56_USB_ID);
    if (ret != 0) {
        LOG_MSGID_I(USB_MAIN, "Switch GPIO56 to ID pin failed", 0);
    }

    hal_sleep_manager_lock_sleep(SLEEP_LOCK_USB);
    USB_HostInit(&g_HostHandle, USB_Host_Event);
    Usb_Host_Message_Queue_Init();
    Usb_Host_task_init();
    hal_usb_host_init();

}

#endif /* AIR_USB_HOST_ENABLE */

