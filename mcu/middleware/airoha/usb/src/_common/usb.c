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

#ifdef AIR_USB_ENABLE

/* C library */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if FREERTOS_ENABLE
/* Kernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif /* FREERTOS_ENABLE */

/* USB Middleware includes */
#include "usb.h"
#include "usb_custom.h"
#include "usb_custom_def.h"
#include "usb_main.h"
#include "usb_resource.h"
#include "usbacm_drv.h"
#include "usbaudio_drv.h"
#include "usbhid_drv.h"
#include "usbms_state.h"
#include "usbms_drv.h"
#include "usb_nvkey_struct.h"

#ifdef AIR_USB_XBOX_ENABLE
#include "usb_xbox.h"
#endif /* AIR_USB_XBOX_ENABLE */

/* Other Middleware includes */
#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* AIR_NVDM_ENABLE */

/* Hal includes */
#include "hal_dvfs.h"
#include "hal_gpio.h"
#include "hal_gpt.h"
#include "hal_gpio.h"
#include "hal_usb.h"
#include "hal_usb_internal.h"
#include "hal_gpt.h"

/* Other includes */
#include "memory_attribute.h"

/* Syslog create module for usb.c */
#include "usb_dbg.h"
log_create_module_variant(USB, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

log_create_module_variant(USB_DBG, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

/* Temp Function Decalaration for IT */
hal_usb_status_t hal_usb_enable_endpoint0(void);
hal_usb_status_t hal_usb_disable_endpoint0(void);
hal_usb_status_t hal_usb_clear_ep0_interrupt(void);

/* Device structure, handle usb level data */
Usb_Device gUsbDevice;
static usb_void_func usb_reset_cb = NULL;
static usb_void_func usb_suspend_cb = NULL;
static usb_void_func usb_resume_cb = NULL;

/* Static fuctions */
static void USB_Init_Device_Status(void);
static void USB_Reset(void);
static void USB_Suspend(void);
static void USB_Resume(void);
static void USB_Initialize(bool double_fifo);
static bool USB_Cmd_SetAddress(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static bool USB_Cmd_GetDescriptor(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static bool USB_Cmd_SetConfiguration(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static bool USB_Cmd_GetConfiguration(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static bool USB_Cmd_SetFeature(Usb_Command *pcmd, bool bset);
static bool USB_Cmd_GetStatus(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static void USB_Stdcmd(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
static void USB_Endpoint0_Idle(void);
static void USB_Endpoint0_Rx(void);
void USB_Endpoint0_Tx(void);
static void USB_Endpoint0_Hdlr(void);

const unsigned char usb_epbulk_dscr[] = {
    USB_EPDSC_LENGTH,    // bLength;
    USB_ENDPOINT,        // bDescriptorType;
    0x00,                // bEndpointAddress;
    USB_EP_BULK,         // bmAttributes;
    0x40,                // wMaxPacketSize[2];
    0x00,                // wMaxPacketSize[2];
    0x00                 // bInterval;
};

/************************************************************
    Adaptor
*************************************************************/
unsigned int USB_Bulk_Max_Packet_Size(void)
{
    unsigned int max_bulk_pkt_size;

    if (hal_usb_is_high_speed() == true) {
        max_bulk_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED;
    } else {
        max_bulk_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED;
    }

    return max_bulk_pkt_size;
}

unsigned int USB_Intr_Max_Packet_Size(void)
{
    unsigned int max_intr_pkt_size;

    if (hal_usb_is_high_speed() == true) {
        max_intr_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_INTERRUPT_HIGH_SPEED;
    } else {
        max_intr_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_INTERRUPT_FULL_SPEED;
    }

    return max_intr_pkt_size;
}

unsigned int USB_Iso_Max_Packet_Size(void)
{
    unsigned int max_iso_pkt_size;

    if (hal_usb_is_high_speed() == true) {
        max_iso_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED;
    } else {
        max_iso_pkt_size = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED;
    }

    return max_iso_pkt_size;
}

unsigned int USB_Speed_Reset_Packet_Size(bool b_other_speed)
{
    unsigned int max_packet;

    if (hal_usb_is_high_speed() == true) {
        if (b_other_speed == false) {
            max_packet = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED;
        } else {
            max_packet = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED;
        }
    } else {
        if (b_other_speed == false) {
            max_packet = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED;
        } else {
            max_packet = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED;
        }
    }

    return max_packet;
}

/************************************************************
    gUsbDevice initialize and release functions
*************************************************************/
/* initialize the global variable gUsbDevice */
static void USB_Init_Device_Status(void)
{
    LOG_MSGID_I(USB, "USB_Init_Device_Status", 0);

    uint32_t index = 0;
    uint32_t temp_addr  = (uint32_t)&gUsbDevice.resource_ep_bulk_tx_number;
    uint32_t temp_addr2 = (uint32_t)&gUsbDevice.conf;

    /* Reset part of gUsbDevice */
    memset(&gUsbDevice.resource_ep_bulk_tx_number, 0, temp_addr2 - temp_addr);

    gUsbDevice.usb_test_type = HAL_USB_TEST_MODE_TYPE_NONE;
    gUsbDevice.remoteWk      = false;
    gUsbDevice.self_powered  = false;
    gUsbDevice.config_num    = 0;

    for (index = 0; index < USB_MAX_INTERFACE; index++) {
        gUsbDevice.interface_num[index] = 0;
    }

    gUsbDevice.ep0_rx_handler = NULL;
    gUsbDevice.ep0_class_cmd_handler.b_enable = false;
}

/* release the already get memory, and reset some parameter,
   note that hisr should not be reset to NULL since it may already be created*/
void USB_Release_Device_Status(void)
{
    USB_Init_Device_Status();
}

USB_DEVICE_STATE USB_Get_Device_State(void)
{
    return gUsbDevice.nDevState;
}

void USB_Set_Device_Status(void)
{
    gUsbDevice.nDevState = DEVSTATE_CONFIG;
}

/************************************************************
    EP0 functions
*************************************************************/
void USB_Register_EP0_Class_CmdHdlr(uint8_t cmd, usb_ep0_cmd_ptr handler)
{
    gUsbDevice.ep0_class_cmd_handler.cmd          = cmd;
    gUsbDevice.ep0_class_cmd_handler.ep0_cmd_hdlr = handler;
    gUsbDevice.ep0_class_cmd_handler.b_enable     = true;
}

/* Prepare TX data infomration for pep0state, data is actually sent out in TX state handler */
void USB_Generate_EP0Data(Usb_Ep0_Status *pep0state, Usb_Command *pcmd, void *data, int32_t len)
{
    pep0state->pData      = (void *)data;
    pep0state->nBytesLeft = len;

    /* Only transmit at most command request */
    if (pcmd->wLength < pep0state->nBytesLeft) {
        pep0state->nBytesLeft = pcmd->wLength;
    }

    /* No need ZLP when wLength == transfer size */
    if (pcmd->wLength == pep0state->nBytesLeft) {
        pep0state->no_ZLP = true;
    } else {
        pep0state->no_ZLP = false;
    }

    /* EP0 go to TX state */
    gUsbDevice.ep0_state = USB_EP0_TX;
}

/************************************************************
    Memory control APIs
*************************************************************/
#define USB_COM_FUNCTION_SIZE      ((USB_ACM_RX_BUFFER_NUM*USB_ACM_RX_BUFFER_SIZE)+USB_RX_RING_BUFFER_1_SIZE+USB_TX_RING_BUFFER_1_SIZE)
#define USB_2COM_FUNCTION_SIZE     (USB_COM_FUNCTION_SIZE+USB_RX_RING_BUFFER_2_SIZE+USB_TX_RING_BUFFER_2_SIZE)
#define USB_MS_FUNCTION_SIZE       (USBMS_MAX_BUFFERSIZE)

#if USB_MS_FUNCTION_SIZE > USB_2COM_FUNCTION_SIZE
#define USB_MAX_MEMORY_SIZE    (USB_MS_FUNCTION_SIZE + USB_DESCRIPTOR_SIZE)
#else
#define USB_MAX_MEMORY_SIZE    (USB_2COM_FUNCTION_SIZE+ USB_DESCRIPTOR_SIZE)
#endif

#if FREERTOS_ENABLE
void *USB_Get_Memory(uint32_t len)
{
    LOG_MSGID_I(USB, "USB_Get_Memory : size = %X", 1, len);
    return pvPortMallocNC(len);
}

void USB_Free_Memory(void *addr)
{
    if (addr == NULL) {
        LOG_MSGID_E(USB, "USB_Free_Memory : free null Fail", 0);
    } else {
        LOG_MSGID_I(USB, "USB_Free_Memory : free addr 0x%X", 1, addr);
        vPortFreeNC(addr);
    }
}
#endif /* FREERTOS_ENABLE */

/************************************************************
    system ctrl functions
*************************************************************/
/* Init function, called when user select usb type,
   entry function for task , B_eanble is D+ detection enable */
void USB_Init(usb_dev_type_t type)
{
    /* Initialize global variable g_UsbDrvInfo */
    hal_usb_configure_driver();

    /* Initialize global variable gUsbDevice */
    USB_Init_Device_Status();
    gUsbDevice.nDevState = DEVSTATE_DEFAULT;

    switch (type) {
        case USB_CDC_ACM:
            LOG_MSGID_I(USB, "USB_Init USB_CDC_ACM", 0);
            USB_Init_Acm_Status();
            break;

#ifdef  AIR_USB_MSC_ENABLE
        case USB_MASS_STORAGE:
            LOG_MSGID_I(USB, "USB_Init USB_MASS_STORAGE", 0);
            hal_usb_set_speed(true);
            hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
            USB_Init_Ms_Status();
            break;
#endif

#ifdef AIR_USB_HID_ENABLE
        case USB_HID:
            LOG_MSGID_I(USB, "USB_Init USB_HID", 0);
            USB_Init_Hid_Status();
            break;
#endif

#if defined(AIR_USB_AUDIO_ENABLE)
        case USB_AUDIO:
            LOG_MSGID_I(USB, "USB_Init USB_AUDIO", 0);
            USB_Init_Audio_Status();
            break;
        case USB_AUDIO_SMARTPHONE:
            LOG_MSGID_I(USB, "USB_Init USB_AUDIO_SMARTPHONE", 0);
            /* Force switch to FS mode in SMARTPHONE scenario */
            hal_usb_set_speed(false);
            USB_Init_Audio_Status();
            break;
#endif

#if defined(AIR_USB_XBOX_ENABLE)
        case USB_XBOX:
            LOG_MSGID_I(USB, "USB_Init USB_XBOX", 0);
            /* Force switch to FS mode in XBOX scenario */
            hal_usb_set_speed(false);
            USB_Init_XBOX_Status();
            break;
#endif

#if defined(AIR_USB_AUDIO_ENABLE)
        case USB_AUDIO_CDC:
            LOG_MSGID_I(USB, "USB_Init USB_AUDIO_CDC", 0);
            USB_Init_Audio_Status();
            USB_Init_Acm_Status();
            break;
#endif
        default:
            LOG_MSGID_E(USB, "USB_Init invalid device type", 0);
            break;
    }

    /* Register reset and ep0 interrupt handler to driver info */
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_RESET,   0, USB_Reset);
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP0,     0, USB_Endpoint0_Hdlr);
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_SUSPEND, 0, USB_Suspend);
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_RESUME,  0, USB_Resume);

#if 0//def __USB_RAMDISK__
    if (type == USB_MASS_STORAGE) {
        FAT_Init();
    }
#endif   /*__USB_RAMDISK__*/

    /* Create USB descriptors */
    USB_Software_Create();

    /* Reset and initialize system initial value and registers*/
    hal_usb_reset_hardware();
    USB_Initialize(false);

    /* Set D+ to high finally */
    hal_usb_pull_up_dp_line();
}

/* The enable parameter decides whether to turn off D+ at this time */
void USB_Release_Type(void)
{
    gUsbDevice.nDevState = DEVSTATE_DEFAULT;

    /* Stop and release resource */
    switch (gUsbDevice.device_type) {
        case USB_CDC_ACM:
            USB_Release_Acm_Status();
            break;

#if defined(AIR_USB_MSC_ENABLE)
        case USB_MASS_STORAGE:
            hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
            USB_Release_Ms_Status();
            break;
#endif

#ifdef AIR_USB_HID_ENABLE
        case USB_HID:
            USB_Release_Hid_Status();
            break;
#endif

#if defined(AIR_USB_AUDIO_ENABLE)
        case USB_AUDIO:
            USB_Release_Audio_Status();
            break;
#endif

#if defined(AIR_USB_XBOX_ENABLE)
        case USB_XBOX:
            USB_Release_XBOX_Status();
            break;
#endif

#if defined(AIR_USB_AUDIO_ENABLE)
        case USB_AUDIO_CDC:
            USB_Release_Audio_Status();
            USB_Release_Acm_Status();
            break;
#endif
    }

    USB_DeRegister_CreateFunc();

    /* power down USB */
    /* Disable D+ pull high */
    hal_usb_deinit();
}

/* Reset device, called when receive reset interrupt */
extern void hal_usb_reset_drv_info(void);

void USB_Register_Reset_Callback(usb_void_func cb)
{
    if (cb != NULL) {
        LOG_MSGID_I(USB, "USB_Register_reset_Callback callback:0x%X", 1, cb);
    } else {
        LOG_MSGID_E(USB, "USB_Register_reset_Callback callback is NULL", 0);
    }

    usb_reset_cb = cb;
}

static void USB_Reset(void)
{
    uint32_t ep_num;

    if(usb_reset_cb){
        LOG_MSGID_I(USB, "USB_Reset callback:0x%X", 1, usb_reset_cb);
        usb_reset_cb();
    } else {
        LOG_MSGID_E(USB, "USB_Reset usb_reset_cb callback is NULL", 0);
    }

    hal_usb_reset_drv_info();

    /* Initialize driver info and system interrupt */
    USB_Initialize(false);
    gUsbDevice.isReset = true;
    gUsbDevice.nDevState = DEVSTATE_DEFAULT;

    for (ep_num = 0; ep_num < gUsbDevice.resource_ep_bulk_tx_number; ep_num++) {
        if (gUsbDevice.ep_bulk_tx_info[ep_num].ep_reset) {
            gUsbDevice.ep_bulk_tx_info[ep_num].ep_reset();
        }
    }

    for (ep_num = 0; ep_num < gUsbDevice.resource_ep_bulk_rx_number; ep_num++) {
        if (gUsbDevice.ep_bulk_rx_info[ep_num].ep_reset) {
            gUsbDevice.ep_bulk_rx_info[ep_num].ep_reset();
        }
    }

    for (ep_num = 0; ep_num < gUsbDevice.resource_ep_intr_tx_number; ep_num++) {
        if (gUsbDevice.ep_intr_tx_info[ep_num].ep_reset) {
            gUsbDevice.ep_intr_tx_info[ep_num].ep_reset();
        }
    }

    for (ep_num = 0; ep_num < gUsbDevice.resource_ep_iso_rx_number; ep_num++) {
        if (gUsbDevice.ep_iso_rx_info[ep_num].ep_reset) {
            gUsbDevice.ep_iso_rx_info[ep_num].ep_reset();
        }
    }
}

void USB_Register_Suspend_Callback(usb_void_func cb)
{
    if (cb != NULL) {
        LOG_MSGID_I(USB, "USB_Register_Suspend_Callback callback:0x%X", 1, cb);
    } else {
        LOG_MSGID_E(USB, "USB_Register_Suspend_Callback callback is NULL", 0);
    }

    usb_suspend_cb = cb;
}

void USB_Register_Resume_Callback(usb_void_func cb)
{
    if (cb != NULL) {
        LOG_MSGID_I(USB, "USB_Register_Resume_Callback callback:0x%X", 1, cb);
    }  else {
        LOG_MSGID_E(USB, "USB_Register_Resume_Callback callback is NULL", 0);
    }

    usb_resume_cb = cb;
}

static void USB_Suspend(void)
{
    LOG_MSGID_I(USB, "USB_Suspend, usb_suspend_cb = 0x%X", 1, usb_suspend_cb);

    if (usb_suspend_cb) {
        usb_suspend_cb();
    } else {
        LOG_MSGID_I(USB, "USB_Suspend, usb_suspend_cb is NULL WARNING!!", 0);
    }
}

static void USB_Resume(void)
{
    if (usb_resume_cb) {
        LOG_MSGID_I(USB, "USB_Resume callback:0x%X", 1, usb_resume_cb);
        usb_resume_cb();
    } else {
        LOG_MSGID_E(USB, "USB_Resume callback is NULL", 0);
    }
}

usb_suspend_state_t USB_Get_Suspend_State(void)
{
    return (usb_suspend_state_t)hal_usb_get_suspend_state();
}

extern bool g_USB_dynamic_fifo;

/* initialize system */
static void USB_Initialize(bool double_fifo)
{
    bool     temp_fifo;
    uint16_t wMaxPacketSize;
    uint32_t index_ep;
    uint32_t index;

    USB_Software_PrevInit();

    /* initial class specific speed-related endpoint, and re-prepare descriptor */
    USB_Software_Speed_Init(false);

    hal_usb_reset_fifo();

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_bulk_tx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_bulk_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_bulk_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Bulk_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_bulk_tx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;

        hal_usb_init_tx_endpoint((gUsbDevice.ep_bulk_tx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_BULK, temp_fifo);

        gUsbDevice.ep_bulk_tx_info[index_ep].ep_status.epin_status.nBytesLeft = USB_EP_NODATA;
    }

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_bulk_rx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_bulk_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_bulk_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Bulk_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_bulk_rx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;

        hal_usb_init_rx_endpoint((gUsbDevice.ep_bulk_rx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_BULK, temp_fifo);

        gUsbDevice.ep_bulk_rx_info[index_ep].ep_status.epout_status.nBuffLen = 0;
        gUsbDevice.ep_bulk_rx_info[index_ep].ep_status.epout_status.nBytesRecv = 0;
    }

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_intr_tx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_intr_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_intr_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Intr_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_intr_tx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;
        hal_usb_init_tx_endpoint((gUsbDevice.ep_intr_tx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_INTR, temp_fifo);

        gUsbDevice.ep_intr_tx_info[index_ep].ep_status.epin_status.nBytesLeft = USB_EP_NODATA;
    }

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_intr_rx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_intr_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_intr_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Intr_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_intr_rx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;

        hal_usb_init_rx_endpoint((gUsbDevice.ep_intr_rx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_INTR, temp_fifo);

        gUsbDevice.ep_intr_rx_info[index_ep].ep_status.epout_status.nBuffLen = 0;
        gUsbDevice.ep_intr_rx_info[index_ep].ep_status.epout_status.nBytesRecv = 0;
    }

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_iso_rx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_iso_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_iso_rx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Iso_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_iso_rx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;
        hal_usb_init_rx_endpoint((gUsbDevice.ep_iso_rx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_ISO, temp_fifo);

        gUsbDevice.ep_iso_rx_info[index_ep].ep_status.epout_status.nBuffLen = 0;
        gUsbDevice.ep_iso_rx_info[index_ep].ep_status.epout_status.nBytesRecv = 0;
    }

    for (index_ep = 0; index_ep < gUsbDevice.resource_ep_iso_tx_number; index_ep++) {

        if(g_USB_dynamic_fifo) {
            wMaxPacketSize = gUsbDevice.ep_iso_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[1];
            wMaxPacketSize <<= 8;
            wMaxPacketSize |= gUsbDevice.ep_iso_tx_info[index_ep].epdesc.stdep.wMaxPacketSize[0];
        } else {
            wMaxPacketSize = USB_Iso_Max_Packet_Size();
        }
        LOG_MSGID_I(USB, "ep_iso_tx_info wMaxPacketSize 0x%x", 1, wMaxPacketSize);

        temp_fifo = (index_ep == 0) ? double_fifo : false;

        hal_usb_init_tx_endpoint((gUsbDevice.ep_iso_tx_info[index_ep].epdesc.stdep.bEndpointAddress & (~USB_EP_DIR)),
                                 wMaxPacketSize, HAL_USB_EP_TRANSFER_ISO, temp_fifo);

        gUsbDevice.ep_iso_tx_info[index_ep].ep_status.epin_status.nBytesLeft = USB_EP_NODATA;
    }

    /* Clear current configuration pointer */
    gUsbDevice.self_powered = false;
    gUsbDevice.remoteWk     = false;
    /* Set configuration command value  */
    gUsbDevice.config_num   = 0;

    for (index = 0; index < USB_MAX_INTERFACE; index++) {
        gUsbDevice.interface_num[index] = 0;
    }

    gUsbDevice.ep0_state = USB_EP0_IDLE;
    /* Device (function) address, no use, at HW still set 0x00 */
    gUsbDevice.ep0info.byFAddr = 0xff;

    /* Initial class specific interface functions*/
    USB_Software_Init();
}

/************************************************************
    EP0 functions
*************************************************************/
/* Parse command Set Address */
static bool USB_Cmd_SetAddress(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool bError = false;

    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_SetAddress", 0);

    /* Store device function address until status stage of request */
    if (pcmd->bmRequestType != USB_CMD_STDDEVOUT) {
        bError = true;
    } else {
        if (gUsbDevice.nDevState <= DEVSTATE_ADDRESS) {
            pep0state->byFAddr = (uint8_t)pcmd->wValue;

            if ((gUsbDevice.nDevState == DEVSTATE_DEFAULT) && (pep0state->byFAddr <= 127)) {
                gUsbDevice.nDevState = DEVSTATE_SET_ADDRESS;
                hal_usb_set_address(pep0state->byFAddr, HAL_USB_SET_ADDR_DATA);
            } else {
                gUsbDevice.nDevState = DEVSTATE_DEFAULT;
            }
        } else {
            bError = true;
        }
    }

    return bError;
}

/* Parse command Get Descriptor */
static bool USB_Cmd_GetDescriptor(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool bError = false;
    uint8_t byConfig;
    uint8_t strid;
    Usb_Cfg_Dscr *pcfg;
    usb_string_t *usbstr;

    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_GetDescriptor wValue:0x%X", 1, pcmd->wValue);

    /* Decode the required descriptor from the command */
    if ((pcmd->bmRequestType != USB_CMD_STDDEVIN) && pcmd->bmRequestType != USB_CMD_STDIFIN) {
        bError = true;
    } else {
        switch (pcmd->wValue & USB_CMD_DESCMASK) {
            case USB_CMD_DEVICE:
                /* First time host get device descriptor will only get 8 bytes (one packet), but no reset */

                /* Prepare to return Standard Device Descriptor */
                USB_Generate_EP0Data(pep0state, pcmd, &gUsbDevice.devdscr, sizeof(Usb_Dev_Dscr));
                break;

            case USB_CMD_CONFIG:
                byConfig = (uint8_t)(pcmd->wValue & 0x00FF);
                if (byConfig >= gUsbDevice.devdscr.bNumConfigurations) {
                    bError = true;
                } else {
                    USB_Software_Speed_Init(false);

                    /* Get pointer to request configuration descriptor */
                    pcfg = (Usb_Cfg_Dscr *)gUsbDevice.conf;
                    /* Prepare to return Configuration Descriptors */
                    USB_Generate_EP0Data(pep0state, pcmd, pcfg, pcfg->wTotalLength);
                }
                break;

            case USB_CMD_OTHER_SPEED:
                byConfig = (uint8_t)(pcmd->wValue & 0x00FF);
                if (byConfig >= gUsbDevice.devdscr.bNumConfigurations) {
                    bError = true;
                } else {
                    USB_Software_Speed_Init(true);

                    /* Get pointer to request configuration descriptor */
                    pcfg = (Usb_Cfg_Dscr *)gUsbDevice.conf;
                    /* Prepare to return Configuration Descriptors */
                    USB_Generate_EP0Data(pep0state, pcmd, pcfg, pcfg->wTotalLength);
                }
                break;

            case USB_CMD_STRING:
                strid = (uint8_t)(pcmd->wValue & 0x00FF);
                usbstr = USB_String_Get_By_Id(strid);

                if (usbstr) {
                    USB_Generate_EP0Data(pep0state, pcmd, (void *)&usbstr->desc, usbstr->desc.length);
                } else {
                    bError = true;
                }
                break;

            case USB_CMD_DEVICE_QUALIFIER:
#ifdef AIR_USB_XBOX_ENABLE
                /* XBOX mode*/
                if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
                    bError = true;
                    break;
                }
#endif
                /* Prepare to return Standard Device_Qualifier Descriptor */
                USB_Generate_EP0Data(pep0state, pcmd, &gUsbDevice.dev_qual_dscr, sizeof(Usb_Dev_Qual_Dscr));
                break;

#ifdef AIR_USB_HID_ENABLE
            case USB_CMD_HID_DESC:
            case USB_CMD_HID_REPORT:
            case USB_CMD_HID_PHYSICAL:
                bError = usb_hid_get_descriptor(pep0state, pcmd);
                break;
#endif

            default:
                bError = true;
                break;
        }
    }

    return bError;
}


/* Parse command Set Configuration */
static bool USB_Cmd_SetConfiguration(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool bError = false;
    uint8_t byConfig;

    byConfig = (uint8_t)(pcmd->wValue & 0x00FF);

    if (gUsbDevice.nDevState == DEVSTATE_DEFAULT) {
        bError = true;
    } else {
        /* Assumes configurations are numbered from 1 to NumConfigurations */
        if (byConfig > gUsbDevice.devdscr.bNumConfigurations) {
            bError = true;
        } else if (byConfig == 0) {
            gUsbDevice.nDevState = DEVSTATE_ADDRESS;
            gUsbDevice.config_num = 0;
        } else {
            USB_Software_Enable();
            gUsbDevice.nDevState = DEVSTATE_CONFIG;
            gUsbDevice.config_num = byConfig;
            /* Return self power */
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
            gUsbDevice.self_powered = false;
#else
            gUsbDevice.self_powered = true;
#endif
        }
    }

    /* Print current device settings */
    LOG_MSGID_I(USB, "USB_Cmd_SetConfiguration HS/FS:%d VID:0x%04X PID:0x%04X ap_type:%d", 4,
                hal_usb_is_high_speed(),
                USB_GetDeviceVID(),
                USB_GetDevicePID(),
                gUsbDevice.device_type
               );

    return bError;
}

/* Parse command Get Configuration */
static bool USB_Cmd_GetConfiguration(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool bError = false;

    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_GetConfiguration", 0);

    if ((gUsbDevice.nDevState == DEVSTATE_ADDRESS) && (!pcmd->wValue)) {
        /* Prepare to return zero */
        USB_Generate_EP0Data(pep0state, pcmd, &pcmd->wValue, 1);
    } else if (gUsbDevice.nDevState == DEVSTATE_CONFIG) {
        /* Prepare to return configuration value */
        USB_Generate_EP0Data(pep0state, pcmd, &gUsbDevice.config_num, 1);
    } else {
        bError = true;
    }

    return bError;
}

/* Parse command Set Interface */
static bool USB_Cmd_SetInterface(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_SetInterface", 0);
    gUsbDevice.interface_num[pcmd->wIndex] = pcmd->wValue & 0x00FF;
    return false;
}

/* Parse command Get Interface */
static bool USB_Cmd_GetInterface(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_GetInterface", 0);
    USB_Generate_EP0Data(pep0state, pcmd, &gUsbDevice.interface_num[pcmd->wIndex], 1);
    return false;
}

/* Parse command Set/Clear Feature */
/* bset true means command SET_FETURE, false means command CLEAR_FEATURE */
static bool USB_Cmd_SetFeature(Usb_Command *pcmd, bool bset)
{
    bool bError = false;

    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_SetFeature", 0);

    switch (pcmd->bmRequestType) {
        /* Device */
        case USB_CMD_STDDEVOUT:

            if (pcmd->wValue == USB_FTR_DEVREMWAKE) {
                gUsbDevice.remoteWk = bset;
            } else if (pcmd->wValue == USB_FTR_TEST_MODE) {
                if (((pcmd->wIndex) >> 8) == USB_TEST_J) {
                    gUsbDevice.usb_test_type = HAL_USB_TEST_MODE_TYPE_J;
                } else if (((pcmd->wIndex) >> 8) == USB_TEST_K) {
                    gUsbDevice.usb_test_type = HAL_USB_TEST_MODE_TYPE_K;
                } else if (((pcmd->wIndex) >> 8) == USB_TEST_SE0_NAK) {
                    gUsbDevice.usb_test_type = HAL_USB_TEST_MODE_TYPE_SE0_NAK;
                } else if (((pcmd->wIndex) >> 8) == USB_TEST_PACKET) {
                    gUsbDevice.usb_test_type = HAL_USB_TEST_MODE_TYPE_PACKET;
                } else {
                    bError = true;
                }
            } else {
                bError = true;
            }
            break;

        /* Endpoint */
        case USB_CMD_STDEPOUT:
            switch (pcmd->wValue) {
                case USB_FTR_EPHALT:
                    if (pcmd->wIndex != 0) {
                        if (((pcmd->wIndex & 0x80) && (((pcmd->wIndex & 0x0f) > HAL_USB_MAX_NUMBER_ENDPOINT_TX) || ((pcmd->wIndex & 0x0f) == 0x00)))
                            || (((pcmd->wIndex & 0x80) == 0) && (((pcmd->wIndex & 0x0f) > HAL_USB_MAX_NUMBER_ENDPOINT_RX) || ((pcmd->wIndex & 0x0f) == 0x00)))) {
                            return true;
                        }

#ifdef  AIR_USB_MSC_ENABLE
                        if ((gUsbDevice.device_type == USB_MASS_STORAGE) && (bset == false)) {
                            /* When stall by invalid cmd, PC must use MS reset cmd to clear stall */
                            /* Ignore this cmd, and still return stall */
                            if ((g_UsbMS.cbw_is_vaild == false) || (g_UsbMS.ms_is_reset == true)) {
                                return false;
                            }

                            if (((pcmd->wIndex & 0x80) && (hal_usb_get_endpoint_stall_status/*USB_Get_EP_Stall_Status*/(pcmd->wIndex & 0x0f, HAL_USB_EP_DIRECTION_TX) == true))
                                || (((pcmd->wIndex & 0x80) == 0) && (hal_usb_get_endpoint_stall_status/*USB_Get_EP_Stall_Status*/(pcmd->wIndex & 0x0f, HAL_USB_EP_DIRECTION_RX) == true)))
                                //USB_Send_Msg_Ext_Queue(MOD_USB, MSG_ID_USB_MSDRV_CLEAR_STALL_REQ, NULL);

                                /* After USB_reset HISR and before stop DMA, a DMA callbcak happen,
                                                       this callback message may come after reset message, and make MS state machine error */
                                if ((USB_Get_Device_State() == DEVSTATE_CONFIG) && (g_UsbMS.ms_is_reset == false)) {
                                    USB_Send_Message(USB_MSC_CLR_STALL_MSG, NULL);
                                }
                        }
#endif

                        /* Command EP direction zero indicate OUT EP */
                        if (pcmd->wIndex & 0x80) {
                            /* TX EP */
                            if ((bset == false) && (hal_usb_get_endpoint_stall_status(pcmd->wIndex & 0x0f, HAL_USB_EP_DIRECTION_TX) == false)) {
                                hal_usb_clear_tx_endpoint_data_toggle(pcmd->wIndex & 0x0f);
                                break;
                            }
                            hal_usb_set_endpoint_stall((pcmd->wIndex & 0x0f), HAL_USB_EP_DIRECTION_TX, bset);
                        } else {
                            /* RX EP */
                            hal_usb_set_endpoint_stall((pcmd->wIndex & 0x0f), HAL_USB_EP_DIRECTION_RX, bset);
                        }
                    }
                    break;
                default:
                    bError = true;
                    break;
            }
            break;

        case USB_CMD_STDIFOUT:
        default:
            bError = true;
            break;
    }

    return bError;
}

/* Parse command Get Status */
static bool USB_Cmd_GetStatus(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    static uint16_t s_status; // keep data lifetime for USB_Generate_EP0Data

    USB_DBG_I( USB_DBG_EP0_CMD, "USB_Cmd_GetStatus", 0);

    switch (pcmd->bmRequestType) {
        case USB_CMD_STDDEVIN:
            s_status = (uint16_t)(((uint8_t)gUsbDevice.remoteWk << 1) | ((uint8_t)gUsbDevice.self_powered));
            break;
        case USB_CMD_STDIFIN:
            s_status = 0;
            break;
        case USB_CMD_STDEPIN:
            if (pcmd->wIndex & 0x80) {
                /* TX EP*/
                s_status = (uint16_t)hal_usb_get_endpoint_stall_status((pcmd->wIndex & 0x000f), HAL_USB_EP_DIRECTION_TX);
            } else {
                /* RX EP*/
                if((pcmd->wIndex & 0x000f) == 0)  //EP0
                    s_status = 0;
                else
                    s_status = (uint16_t)hal_usb_get_endpoint_stall_status((pcmd->wIndex & 0x000f), HAL_USB_EP_DIRECTION_RX);
            }
            break;
        default:
            return true; // error will return true
    }

    USB_Generate_EP0Data(pep0state, pcmd, &s_status, 2);

    return false; // ok will return false
}

void USB_EP0_Command_Hdlr(bool bError)
{
    if (gUsbDevice.ep0_state == USB_EP0_IDLE) {
        gUsbDevice.ep0_state = USB_EP0_RX_STATUS;
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, bError, true);
    } else {
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, bError, false);
    }

    // if(bError){
    //     LOG_MSGID_E(USB, "USB_EP0_Command_Hdlr bError:%d", 1, bError);
    // }
}

/* Parse usb standard command */
static void USB_Stdcmd(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool   bError = false;

    gUsbDevice.ep0_send_one_pkt = false;

    switch (pcmd->bRequest) {
        case USB_SET_ADDRESS:
            bError = USB_Cmd_SetAddress(pep0state, pcmd);
            break;
        case USB_GET_DESCRIPTOR:
            bError = USB_Cmd_GetDescriptor(pep0state, pcmd);
            break;
        case USB_SET_CONFIGURATION:
            bError = USB_Cmd_SetConfiguration(pep0state, pcmd);
#ifdef FREERTOS_ENABLE
            USB_Send_Message(USB_ATCI_NOTIFY_PLUG, NULL);
            USB_Send_Message(USB_RACE_NOTIFY_PLUG, NULL);
            USB_Send_Message(USB_CONFIG_DONE, NULL);
#endif /* FREERTOS_ENABLE */
#ifndef FREERTOS_ENABLE
            /* without rtos, do callback in ep0 interrupt */
            usb_evt_exec_cb(USB_EVT_CONFIG_DONE, NULL);
#endif
            break;
        case USB_SET_INTERFACE:
            bError = USB_Cmd_SetInterface(pep0state, pcmd);
#if defined(AIR_USB_AUDIO_ENABLE)
            if((USB_Audio[USB_AUDIO_1_PORT].initialized == true) || (USB_Audio[USB_AUDIO_2_PORT].initialized == true) ){
                USB_Audio_Set_Interface((uint32_t)((pcmd->wIndex << 16) + pcmd->wValue));
                USB_Send_Message(USB_AUDIO_SET_INTERFACE, (void *)((pcmd->wIndex << 16) + pcmd->wValue));
            }
#endif
            break;
        case USB_GET_CONFIGURATION:
            bError = USB_Cmd_GetConfiguration(pep0state, pcmd);
            break;
        case USB_GET_INTERFACE:
            bError = USB_Cmd_GetInterface(pep0state, pcmd);
            break;
        case USB_SET_FEATURE:
            bError = USB_Cmd_SetFeature(pcmd, true);
            break;
        case USB_CLEAR_FEATURE:
            bError = USB_Cmd_SetFeature(pcmd, false);
            break;
        case USB_GET_STATUS:
            bError = USB_Cmd_GetStatus(pep0state, pcmd);
            break;
        /* Stall the command if an unrecognized request is received */
        case USB_SYNCH_FRAME:   /*Only support for Isoc traffic*/
        case USB_SET_DESCRIPTOR:
        default:
            bError = true;
            LOG_MSGID_E(USB, "USB_Stdcmd bRequest:0x%X is out of case", 1, pcmd->bRequest);
            break;
    }

    USB_EP0_Command_Hdlr(bError);
}

/* EP0 interrupt handler called by USB_HISR */
extern uint32_t hal_usb_ep0_pkt_len(void);

/* Check the USB HW fifo data length for EP0*/
bool USB_Check_EP0_DataLen(uint32_t ep_num, uint32_t expected_byte)
{
    uint32_t byte;
    uint32_t dma_timeout_start, dma_timeout_end, dma_timeout_duration;

    /* Get EP0 data length */
    byte = hal_usb_ep0_pkt_len();

    /* If EP0 data length is not enough, then wait 1ms */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_start);

    while (byte < expected_byte) {
        /* Get EP0 data length */
        byte = hal_usb_ep0_pkt_len();

        /* Timeout 1ms*/
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_end);
        dma_timeout_duration = dma_timeout_end - dma_timeout_start;

        if (dma_timeout_duration > USB_GPT_TIMEOUT_1ms) {
            LOG_MSGID_E(USB, "USB_Check_EP0_DataLen byte:%d < expect_byte:%d wait to timeout: %d us", 3,
                        byte, expected_byte, USB_GPT_TIMEOUT_1ms);
            return false;
        } else {
            //USB_DBG_I(USB_DBG_EP0_DATALEN_WAIT, ...
            LOG_MSGID_E(USB, "USB_Check_EP0_DataLen wait %d us to get enough data", 1, dma_timeout_duration);
        }
    }

    return true;
}


/* Parse EP0 requested command */
static void USB_Endpoint0_Idle(void)
{
    /* Check EP0 data is enough */
    USB_Check_EP0_DataLen(0, 8);

    /* Read ep0 data */
    hal_usb_read_endpoint_fifo(0, 8, &gUsbDevice.cmd);

    USB_DBG_I(USB_DBG_EP0_STATE_IDLE, "USB_Endpoint0_Idle bmRequestType:0x%x, bRequest:0x%x, wValue:0x%x, wIndex:0x%x, wLength0x%x", 5,
                gUsbDevice.cmd.bmRequestType, gUsbDevice.cmd.bRequest, gUsbDevice.cmd.wValue, gUsbDevice.cmd.wIndex, gUsbDevice.cmd.wLength);

    /* Check request type */
    switch (gUsbDevice.cmd.bmRequestType & USB_CMD_TYPEMASK) {
        case USB_CMD_STDREQ:
            /* standard request */
            USB_Stdcmd(&gUsbDevice.ep0info, &gUsbDevice.cmd);

            LOG_MSGID_I(USB, "USB_Endpoint0_Idle - USB_CMD_STDREQ bmRequestType:0x%x, bRequest:0x%x, wValue:0x%x, wIndex:0x%x, wLength0x%x", 5,
                        gUsbDevice.cmd.bmRequestType, gUsbDevice.cmd.bRequest, gUsbDevice.cmd.wValue, gUsbDevice.cmd.wIndex, gUsbDevice.cmd.wLength);
            break;

        case USB_CMD_CLASSREQ:
            /* Class specific request */
            if ((gUsbDevice.cmd.bmRequestType == USB_CMD_CLASSIFIN) || (gUsbDevice.cmd.bmRequestType == USB_CMD_CLASSIFOUT)) {
                if ((gUsbDevice.cmd.wIndex & 0xFF) > USB_MAX_INTERFACE) {
                    /* Error occur, stall endpoint */
                    LOG_MSGID_E(USB, "USB_Endpoint0_Idle if_index:%d > if max:%d", 2, gUsbDevice.cmd.wIndex & 0xFF, USB_MAX_INTERFACE);
                    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                    return;
                }

                /* Depend on interface number to dispatch usb cmd */
                if (gUsbDevice.if_info[(gUsbDevice.cmd.wIndex & 0xff)].if_class_specific_hdlr != NULL) {
                    // USB_DBG_I(USB_DBG_EP0_STATE_IDLE, "USB_Endpoint0_Idle if_class_specific_hdlr if_info:0x%X", 1, (gUsbDevice.cmd.wIndex & 0xff));
                    gUsbDevice.if_info[(gUsbDevice.cmd.wIndex & 0xff)].if_class_specific_hdlr(&gUsbDevice.ep0info, &gUsbDevice.cmd);
                } else {
                    /* Error occur, stall endpoint*/
                    LOG_MSGID_E(USB, "USB_Endpoint0_Idle can't find if_class_specific_hdlr", 0);
                    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                }
            }

#if defined(AIR_USB_AUDIO_ENABLE)
            else if ((gUsbDevice.cmd.bmRequestType == USB_CMD_CLASSEPOUT) || (gUsbDevice.cmd.bmRequestType == USB_CMD_CLASSEPIN)) {
                if(((gUsbDevice.cmd.wIndex & USB_EP_ADR_MASK) == (USB_EP_DIR_OUT | g_UsbAudio[USB_AUDIO_1_PORT].stream_ep_out_id)) &&
                     g_UsbAudio[USB_AUDIO_1_PORT].stream_if_info->if_class_specific_hdlr) {
                    /*wIdex: ep num*/
                    g_UsbAudio[USB_AUDIO_1_PORT].stream_if_info->if_class_specific_hdlr(&gUsbDevice.ep0info, &gUsbDevice.cmd);
                }
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
                else if (((gUsbDevice.cmd.wIndex & USB_EP_ADR_MASK) == (USB_EP_DIR_OUT | g_UsbAudio[USB_AUDIO_2_PORT].stream_ep_out_id)) &&
                           g_UsbAudio[USB_AUDIO_2_PORT].stream_if_info->if_class_specific_hdlr) {
                    g_UsbAudio[USB_AUDIO_2_PORT].stream_if_info->if_class_specific_hdlr(&gUsbDevice.ep0info, &gUsbDevice.cmd);
                }
#endif
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                else if((gUsbDevice.cmd.wIndex & USB_EP_ADR_MASK) == (USB_EP_DIR_IN | g_UsbAudio[USB_AUDIO_1_PORT].stream_ep_in_id) &&
                          g_UsbAudio[USB_AUDIO_1_PORT].stream_microphone_if_info->if_class_specific_hdlr) {
                    g_UsbAudio[USB_AUDIO_1_PORT].stream_microphone_if_info->if_class_specific_hdlr(&gUsbDevice.ep0info, &gUsbDevice.cmd);
                }
#endif
                else {
                    /* Error occur, stall endpoint*/
                    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                    LOG_MSGID_E(USB, "USB_Endpoint0_Idle index:%X speaker1:%X mic1:%X speaker2:%X is out of USB Audio case", 4,
                                gUsbDevice.cmd.wIndex & USB_EP_ADR_MASK,
                                USB_EP_DIR_OUT | g_UsbAudio[USB_AUDIO_1_PORT].stream_ep_out_id,
                                USB_EP_DIR_IN  | g_UsbAudio[USB_AUDIO_1_PORT].stream_ep_in_id,
                                USB_EP_DIR_OUT | g_UsbAudio[USB_AUDIO_2_PORT].stream_ep_out_id);
                }
            }
#endif
            else {
                /* Error occur, stall endpoint*/
                hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                LOG_MSGID_E(USB, "USB_Endpoint0_Idle out of class case", 0);
            }
            break;

        case USB_CMD_VENDREQ:
#ifdef AIR_USB_XBOX_ENABLE
            USB_DBG_I(USB_DBG_EP0_STATE_IDLE, "USB_Endpoint0_Idle device_type:%x", 1, gUsbDevice.device_type);

            if (gUsbDevice.cmd.bRequest == bMS_Vendor_Code) {
                if (gUsbDevice.device_type != USB_XBOX) {
                    /* Stall in PC mode (Standard USB Audio device) */
                    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                    break;
                }
                if (gUsbDevice.cmd.wIndex == EXTENDED_COMPATIBLE_ID) {
                    USB_DBG_I(USB_DBG_EP0_STATE_IDLE, "USB_Endpoint0_Idle EXTENDED_COMPATIBLE_ID", 0);
                    USB_Generate_EP0Data(&gUsbDevice.ep0info, &gUsbDevice.cmd, (void *)xbox_ext_com_ID_dscr, 0x28);  // Check sizeof(xbox_ext_com_ID_dscr)
                }
                else if (gUsbDevice.cmd.wIndex == EXTENDED_COMPATIBLE) {
                    USB_DBG_I(USB_DBG_EP0_STATE_IDLE, "USB_Endpoint0_Idle EXTENDED_COMPATIBLE", 0);
                    USB_Generate_EP0Data(&gUsbDevice.ep0info, &gUsbDevice.cmd, (void *)xbox_ext_com_dscr, 0x0A); // Check sizeof(xbox_ext_com_dscr)
                }
                else {
                    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
                }
            } else {
                hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
            }
            break;
#endif
        default:
            /* Stall the command if a reserved request is received */
            hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
            break;
    }
}

/* EP0 RX handler, called when EP0 interrupt happened and in RX state */
static void USB_Endpoint0_Rx(void)
{
    USB_DBG_I(USB_DBG_EP0_STATE_RX, "USB_Endpoint0_Rx", 0);

    if (gUsbDevice.ep0_rx_handler != NULL) {
        /* called rx handler to get data */
        gUsbDevice.ep0_rx_handler(&gUsbDevice.ep0info);
    } else {
        /* This should not happened, user should register rx handler when set EP0 into
        RX state error occur, stall endpoint */
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, true, false);
    }
}

/* EP0 TX handler, called when EP0 interrupt happened and in TX state,
   or EP0 just translate from IDLE to TX state */
void USB_Endpoint0_Tx(void)
{
    int32_t nBytes;

    USB_DBG_I(USB_DBG_EP0_STATE_TX, "USB_Endpoint0_Tx", 0);

    /* Determine number of bytes to send */
    if (gUsbDevice.ep0info.nBytesLeft <= HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0) {
        nBytes = gUsbDevice.ep0info.nBytesLeft;
        gUsbDevice.ep0info.nBytesLeft = 0;
    } else {
        nBytes = HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0;
        gUsbDevice.ep0info.nBytesLeft -= HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0;
    }

    /* Send out data */
    hal_usb_write_endpoint_fifo(0, nBytes, gUsbDevice.ep0info.pData);

    /* Update data pointer and  prepare for next transaction */
    gUsbDevice.ep0info.pData = (uint8_t *)gUsbDevice.ep0info.pData + nBytes;

    if ((nBytes < HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0) || (gUsbDevice.ep0_send_one_pkt == true) || ((gUsbDevice.ep0info.no_ZLP == true) && (gUsbDevice.ep0info.nBytesLeft == 0))) {
        gUsbDevice.ep0_state = USB_EP0_IDLE;
        /* Last data, set end as true */
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_WRITE_RDY, false, true);
    } else {
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_WRITE_RDY, false, false);
    }
}

extern volatile USB_REGISTER_T *musb;

static void USB_Endpoint0_Hdlr(void)
{
    bool b_transaction_end;
    bool b_sent_stall;
    uint16_t IntrTX = 0;
    uint32_t nCount;

    USB_DBG_I(USB_DBG_EP0_STATE_HDRL, "USB_Endpoint0_Hdlr", 0);

    hal_usb_get_endpoint_0_status(&b_transaction_end, &b_sent_stall);

    /* Check for SentStall */
    /* SentStall && SetupEnd are impossible to occur together*/
    if (b_sent_stall == true) {
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_CLEAR_SENT_STALL, false, false);
        gUsbDevice.ep0_state = USB_EP0_IDLE;
        LOG_MSGID_I(USB, "USB_Endpoint0_Hdlr b_sent_stall", 0);
    }

    /* Check for SetupEnd */
    if (b_transaction_end == true) {
        hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_TRANSACTION_END, false, false);
        gUsbDevice.ep0_state = USB_EP0_IDLE;
        LOG_MSGID_I(USB, "USB_Endpoint0_Hdlr b_transaction_end", 0);
    }

    /* Call relevant routines for endpoint 0 state */
    if (gUsbDevice.ep0_state == USB_EP0_IDLE) {
        /* receive command request */
        nCount = hal_usb_ep0_pkt_len();

        IntrTX = musb->intrtx;
         if (nCount > 0) {
             /* Clear EP0 interrupt when TX status and next set up cmd comes in very short time
                It's ok to do more one time for set up cmd in Idle status */
             if(IntrTX & USB_INTRTX_EP0){
                 hal_usb_clear_ep0_interrupt();
             }
             /* idle state handler */
             USB_Endpoint0_Idle();
         }
    } else if (gUsbDevice.ep0_state == USB_EP0_RX) {
        nCount = hal_usb_ep0_pkt_len();
        if (nCount > 0) {
            /* Rx state handler */
            USB_Endpoint0_Rx();
        }
    } else if (gUsbDevice.ep0_state == USB_EP0_RX_STATUS) {
        /* Data stage is RX, status stage is TX*/
        if (gUsbDevice.nDevState == DEVSTATE_SET_ADDRESS) {
            hal_usb_set_address(gUsbDevice.ep0info.byFAddr, HAL_USB_SET_ADDR_STATUS);
            gUsbDevice.nDevState = DEVSTATE_ADDRESS;
        }

        if (gUsbDevice.usb_test_type != HAL_USB_TEST_MODE_TYPE_NONE) {
            /* After status complete, target must transmit to test mode in 3ms */
            hal_usb_enter_test_mode(gUsbDevice.usb_test_type);
            return;
        }

        gUsbDevice.ep0_state = USB_EP0_IDLE;

        /* In case next setup followed the previous status very fast and interrupt only happens once*/
        /* Wait 25us for USB FIFO ready*/
        hal_gpt_delay_us(25);
        nCount = hal_usb_ep0_pkt_len();

        /* Check EP0 data length >0 and =8 which make sure 8 for set up cmd */
        if(nCount > 0){
            nCount = hal_usb_ep0_pkt_len();
            if(nCount == 8){
                hal_usb_clear_ep0_interrupt();
                USB_Endpoint0_Idle();
            } else {
                LOG_MSGID_E(USB, "USB_Endpoint0_Hdlr RX status polling next set up cmd fail !! IntrTX:0x%X nCount:%d", 2,  IntrTX, nCount);
            }
        }
    }

    /* Must use if, not else if, EP0 may enter TX state in previous IDLE state handler */
    if (gUsbDevice.ep0_state == USB_EP0_TX) {
        /* Tx state handler */
        USB_Endpoint0_Tx();
    }
}

#endif /* AIR_USB_ENABLE */

