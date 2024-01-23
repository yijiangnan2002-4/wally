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
#include <assert.h>

/* Kernel includes */
#if FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif /* FREERTOS_ENABLE */

/* USB Middleware includes */
#include "usb.h"
#include "usb_custom.h"
#include "usb_main.h"
#include "usb_nvkey_struct.h"
#include "usb_resource.h"
#include "usbaudio_drv.h"

#ifdef AIR_USB_XBOX_ENABLE
#include "usb_xbox.h"
#endif /* AIR_USB_XBOX_ENABLE */

/* Other Middleware includes */
#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* AIR_NVDM_ENABLE */

/* Hal includes */
#include "hal_usb.h"
#include "hal_usb_internal.h"

/* Macros */
#define USB_LANGUAGUE_ID            0x0409

/* Static functions */
static void USB_Basic_Info_Init(void);
static void USB_Basic_Info_Init_Class_Info(const usb_custom_class_info_t *info);
static void USB_Basic_Info_Init_Product_Info(const usb_custom_product_info_t *info);
static void USB_Basic_Info_Init_Power_Info(const usb_custom_power_info_t *info);
static void USB_String_Init(void);
static void USB_String_Create_Language_Id(void);
static void USB_String_Init_From_Custom_List(void);

/* Syslog create module for usb_resource.c */
#include "usb_dbg.h"
log_create_module_variant(USB_RESOURCE, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

#define USB_DESCRIPTOR_BUFFER_SIZE 512

#ifndef FREERTOS_ENABLE

#ifdef USB_LOW_MEM_REQ
#ifndef AIR_USB_AUDIO_1_MIC_ENABLE
static uint8_t USB_TEST_Buffer[USB_DESCRIPTOR_BUFFER_SIZE];
#else
static uint8_t USB_TEST_Buffer[USB_DESCRIPTOR_BUFFER_SIZE];  // Need 259 bytes
#endif
#else
static uint8_t USB_TEST_Buffer[USB_DESCRIPTOR_BUFFER_SIZE];
#endif

#endif // FREERTOS_ENABLE

/* Standard Device Descriptor */
static const uint8_t devdscr[] = {
    USB_DEVDSC_LENGTH,                  //bLength
    USB_DEVICE,                         //bDescriptorType
    0x00,                               //bcdUSB
    0x00,                               //bcdUSB
    0x00,                               //bDeviceClass
    0x00,                               //bDeviceSubClass
    0x00,                               //bDeviceProtocol
    HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0, //bMaxPacketSize0
    0x00,                               //idVendor
    0x00,                               //idVendor
    0x00,                               //idProduct
    0x00,                               //idProduct
    0x00,                               //bcdDevice
    0x00,                               //bcdDevice
    0x00,                               //iManufacturer
    0x00,                               //iProduct
    0x00,                               //iSerialNumber
    0x01                                //bNumConfigurations
};

/* Initialize the descriptors for main configuration 1 */
static const uint8_t cfgdscr[] = {
    USB_CFGDSC_LENGTH,                  //bLength
    USB_CONFIG,                         //bDescriptorType
    0x00,                               //wTotalLength
    0x00,                               //wTotalLength
    0x00,                               //bNumInterfaces
    /* the value = (cfg index+1), select this config's number */
    0x01,                               //bConfigurationValue
    0x00,                               //iConfiguration

    /* bmAttributes */
#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
    USB_CFGDSC_ATTR_NATURE,             //bmAttributes
#else
    USB_CFGDSC_ATTR_NATURE | USB_CFGDSC_ATTR_SELFPOWER,             //bmAttributes
#endif
#else
    USB_CFGDSC_ATTR_NATURE,             //bmAttributes
#endif

    /* bMaxPower */
#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
    20 / 2                              //bMaxPower,20mA
#else
    USB_CFGDSC_MAXPOWER                 //bMaxPower
#endif
};

/* Initialize the Standard Device_Qualiifier Descriptor */
static const uint8_t dev_qual_dscr[] = {
    USB_DEV_QUA_DSC_LENGTH,             //bLength
    USB_DEVICE_QUALIFIER,               //bDescriptorType
    0x00, /* USB spec rev is 2.0 */     //bcdUSB
    0x02, /* USB spec rev is 2.0 */     //bcdUSB
    0x00,                               //bDeviceClass
    0x00,                               //bDeviceSubClass
    0x00,                               //bDeviceProtocol
    HAL_USB_MAX_PACKET_SIZE_ENDPOINT_0, //bMaxPacketSize0
    0x00,                               //bNumConfigurations
    0x00                                //bReserved
};

/* interface specific create and init functions */
static Usb_IfCreate_Info usb_ifcreate_tbl[USB_MAX_INTERFACE];
static uint8_t usb_ifcreate_number = 0;

/* device code */
static Usb_Device_Code usb_device_code;

/* static functions */
static void USB_Resource_Reset(void);

/* Register class specific create functions, class specific create function
   should create interface descriptor */
void USB_Register_CreateFunc(char *if_name,
                             usb_create_if_func_ptr if_create_func,
                             usb_void_func if_init_func,
                             usb_void_func if_enable_func,
                             usb_speed_if_func_ptr if_speed_func,
                             usb_void_func if_resume_func)
{
    if ((usb_ifcreate_number >= USB_MAX_INTERFACE) ||
        (if_create_func == NULL) ||
        (if_init_func == NULL)   ||
        (if_enable_func == NULL) ||
        (if_speed_func == NULL)) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Register_CreateFunc if_num:%d>=%d create_func:0x%X init_func:0x%X enable_func:0x%X speed_func:0x%X", 6,
                   usb_ifcreate_number, USB_MAX_INTERFACE, if_create_func, if_init_func, if_enable_func, if_speed_func);
        return;
    }

    usb_ifcreate_tbl[usb_ifcreate_number].if_name_ptr = if_name;
    usb_ifcreate_tbl[usb_ifcreate_number].if_create_func = if_create_func;
    usb_ifcreate_tbl[usb_ifcreate_number].if_init_func = if_init_func;
    usb_ifcreate_tbl[usb_ifcreate_number].if_enable_func = if_enable_func;
    usb_ifcreate_tbl[usb_ifcreate_number].if_speed_func = if_speed_func;
    usb_ifcreate_tbl[usb_ifcreate_number].if_resume_func = if_resume_func;
    usb_ifcreate_number++;
    usb_device_code.b_registerd = true;
}

/* Deregister class specific create functions, class specific device
   and product code should be called when cable plug out */
void USB_DeRegister_CreateFunc(void)
{
    usb_ifcreate_number = 0;
    usb_device_code.b_registerd = false;
}

static void USB_Check_Ep_Number_ErrorLog()
{
    LOG_MSGID_E(USB_RESOURCE, "USB_Check_Ep_Number intr_tx[%x] intr_rx[%x] iso_tx[%x] iso_rx[%x] bulk_tx[%x] bulk_rx[%x]", 6,
                gUsbDevice.resource_ep_intr_tx_number, gUsbDevice.resource_ep_intr_rx_number,
                gUsbDevice.resource_ep_iso_tx_number,  gUsbDevice.resource_ep_iso_rx_number,
                gUsbDevice.resource_ep_bulk_tx_number, gUsbDevice.resource_ep_bulk_rx_number);
}

static void USB_Check_Ep_Number()
{
    uint8_t ep_tx = gUsbDevice.resource_ep_intr_tx_number + gUsbDevice.resource_ep_iso_tx_number + gUsbDevice.resource_ep_bulk_tx_number;
    uint8_t ep_rx = gUsbDevice.resource_ep_intr_rx_number + gUsbDevice.resource_ep_iso_rx_number + gUsbDevice.resource_ep_bulk_rx_number;

    /* Check the valid endpoint number */
    if(ep_tx > HAL_USB_MAX_NUMBER_ENDPOINT_TX){
       LOG_MSGID_E(USB_RESOURCE, "USB_Check_Ep_Number ep_tx[%x] > MAX EP TX[%x]", 2, ep_tx, HAL_USB_MAX_NUMBER_ENDPOINT_TX);
       USB_Check_Ep_Number_ErrorLog();
    }

    if(ep_rx > HAL_USB_MAX_NUMBER_ENDPOINT_RX){
       LOG_MSGID_E(USB_RESOURCE, "USB_Check_Ep_Number ep_rx[%x] > MAX EP RX[%x]", 2, ep_rx, HAL_USB_MAX_NUMBER_ENDPOINT_RX);
       USB_Check_Ep_Number_ErrorLog();
    }
}

static void USB_Check_Descriptor()
{
    if ((gUsbDevice.resource_interface_number > USB_MAX_INTERFACE) ||
        (gUsbDevice.resource_string_number > USB_MAX_STRING)       ||
        (gUsbDevice.resource_iad_number > USB_MAX_IAD)) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Check_Descriptor : Fail", 0);
        assert(0);
    }
}

/* Make USB endpoint number in order in XBOX mode */
uint8_t resource_tx_ep_number = 0;
uint8_t resource_rx_ep_number = 0;

/* Get interrupt TX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Intr_Tx_Ep(uint8_t *p_num)
{
    /* Count from maxiumn of tx ep */
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX){
        resource_tx_ep_number++;
        *p_num = resource_tx_ep_number;
    } else {
        *p_num = HAL_USB_MAX_NUMBER_ENDPOINT_TX - gUsbDevice.resource_ep_intr_tx_number;
    }

    Usb_Ep_Info *p_ep = &gUsbDevice.ep_intr_tx_info[gUsbDevice.resource_ep_intr_tx_number];
    gUsbDevice.resource_ep_intr_tx_number++;

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get interrupt RX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Intr_Rx_Ep(uint8_t *p_num)
{
    /* Count from maxiumn of rx ep */
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX){
        resource_rx_ep_number++;
        *p_num = resource_rx_ep_number;
    } else {
        *p_num = HAL_USB_MAX_NUMBER_ENDPOINT_RX - gUsbDevice.resource_ep_intr_rx_number;
    }

    Usb_Ep_Info *p_ep = &gUsbDevice.ep_intr_rx_info[gUsbDevice.resource_ep_intr_rx_number];
    gUsbDevice.resource_ep_intr_rx_number++;

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get bulk TX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Bulk_Tx_Ep(uint8_t *p_num)
{
    Usb_Ep_Info *p_ep = &gUsbDevice.ep_bulk_tx_info[gUsbDevice.resource_ep_bulk_tx_number];
    gUsbDevice.resource_ep_bulk_tx_number++;

    *p_num = gUsbDevice.resource_ep_iso_tx_number + gUsbDevice.resource_ep_bulk_tx_number;

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get bulk RX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Bulk_Rx_Ep(uint8_t *p_num)
{
    Usb_Ep_Info *p_ep = &gUsbDevice.ep_bulk_rx_info[gUsbDevice.resource_ep_bulk_rx_number];
    gUsbDevice.resource_ep_bulk_rx_number++;

    *p_num = gUsbDevice.resource_ep_bulk_rx_number + gUsbDevice.resource_ep_iso_rx_number;

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get iso TX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Iso_Tx_Ep(uint8_t *p_num)
{
    Usb_Ep_Info *p_ep = &gUsbDevice.ep_iso_tx_info[gUsbDevice.resource_ep_iso_tx_number];
    gUsbDevice.resource_ep_iso_tx_number++;

    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX){
        resource_tx_ep_number++;
        *p_num = resource_tx_ep_number;
    } else {
        *p_num = gUsbDevice.resource_ep_bulk_tx_number + gUsbDevice.resource_ep_iso_tx_number;

    }

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get iso RX endpoint number and endpoint info pointer */
Usb_Ep_Info *USB_Get_Iso_Rx_Ep(uint8_t *p_num)
{
    Usb_Ep_Info *p_ep = &gUsbDevice.ep_iso_rx_info[gUsbDevice.resource_ep_iso_rx_number];
    gUsbDevice.resource_ep_iso_rx_number++;

    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX){
        resource_rx_ep_number++;
        *p_num = resource_rx_ep_number;
    } else {
        *p_num = gUsbDevice.resource_ep_bulk_rx_number + gUsbDevice.resource_ep_iso_rx_number;
    }

    USB_Check_Ep_Number();
    return p_ep;
}

/* Get interface number and interface info pointer */
Usb_Interface_Info *USB_Get_Interface_Number(uint8_t *p_num)
{
    gUsbDevice.resource_interface_number++;
    USB_Check_Descriptor();
    *p_num = gUsbDevice.resource_interface_number - 1;
    return (&gUsbDevice.if_info[gUsbDevice.resource_interface_number - 1]);
}

Usb_IAD_Dscr *USB_Get_IAD_Number(void)
{
    gUsbDevice.resource_iad_number++;
    USB_Check_Descriptor();
    return (&gUsbDevice.iad_desc[gUsbDevice.resource_iad_number - 1]);
}

/* Create all descriptors, including device and class specific */
void USB_Software_Create(void)
{
    uint32_t  index_if = 0;
    uint32_t  index_if_alt = 0;
    uint32_t  index_ep;
    uint32_t  if_len = 0;
    /* initial resource number */
    USB_Resource_Reset();

    /**
     * String Descriptor Init
     * String Resource init first, cause basic info and interface will
     * reference to string.
     * e.g.
     *   - Device descriptor reference to Product and Vendor string.
     *   - Interface reference to each interface's name string.
     */
    USB_String_Init();

    /* check class has registered create function and device code */
    if ((usb_ifcreate_number == 0) || (usb_device_code.b_registerd == false)) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Software_Create : check registered function Fail", 0);
    }

    /* call the create function for each class */
    for (index_if = 0; index_if < usb_ifcreate_number; index_if++) {
        if (usb_ifcreate_tbl[index_if].if_create_func != NULL) {
            usb_ifcreate_tbl[index_if].if_create_func(usb_ifcreate_tbl[index_if].if_name_ptr);
        }

        if ((gUsbDevice.if_info[index_if].ifdscr_size == 0) || (usb_ifcreate_tbl[index_if].if_create_func == NULL)) {
            LOG_MSGID_E(USB_RESOURCE, "USB_Software_Create : create function Fail", 0);
        }
    }

    /* Initialize the Standard Device Descriptor */
    memcpy(&(gUsbDevice.devdscr), devdscr, USB_DEVDSC_LENGTH);
    memcpy(&(gUsbDevice.cfgdscr), cfgdscr, USB_CFGDSC_LENGTH);
    USB_Basic_Info_Init();

    /* Base on different AP, overwrite current basic info*/
#ifdef AIR_USB_XBOX_ENABLE
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        memcpy(&(gUsbDevice.devdscr), xbox_dev_descr, USB_DEVDSC_LENGTH);
        memcpy(&(gUsbDevice.cfgdscr), xbox_cfg_descr, USB_CFGDSC_LENGTH);
    }
#endif

    gUsbDevice.devdscr.iManufacturer = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_VENDOR);
    gUsbDevice.devdscr.iProduct = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_PRODUCT);
    gUsbDevice.devdscr.iSerialNumber = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_SERIAL);

    /* Initialize the descriptors for main configuration 1 */
    gUsbDevice.cfgdscr.bNumInterfaces = gUsbDevice.resource_interface_number;

    /* Configuration length */
    for (index_if = 0; index_if < gUsbDevice.resource_iad_number; index_if++) {
        if_len += gUsbDevice.iad_desc[index_if].bLength;
    }

    for (index_if = 0; index_if < gUsbDevice.resource_interface_number; index_if++) {
        if_len += gUsbDevice.if_info[index_if].ifdscr_size;

        for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].ifdscr.stdif.bNumEndpoints; index_ep++) {
            if_len += gUsbDevice.if_info[index_if].ep_info[index_ep]->epdscr_size;
        }

        for (index_if_alt = 0; index_if_alt < USB_MAX_INTERFACE_ALTERNATE_NUM; index_if_alt++) {
            if (gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size > 0) {
                if_len += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size;

#if 0
                uint32_t  ep_max = 0;
                /* Correct the wMaxPacketSize of RX EP for USB-Audio */
                #if defined(AIR_USB_AUDIO_ENABLE)
                #ifdef AIR_USB_AUDIO_1_SPK_ENABLE
                if(index_if == g_UsbAudio[0].stream_interface_id) {
                    ep_max = USB_Audio_Get_RX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                }
                #endif

                #ifdef AIR_USB_AUDIO_2_SPK_ENABLE
                if(index_if == g_UsbAudio[1].stream_interface_id) {
                    ep_max = USB_Audio_Get_RX_Alt_Byte(1, index_if_alt + 1); /* +1 for array offset */
                }
                #endif

                #ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                if(index_if == g_UsbAudio[0].stream_microphone_interface_id) {
                    ep_max = USB_Audio_Get_TX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                }
                #endif
                #endif

                #ifdef AIR_USB_XBOX_ENABLE
                if(usb_get_device_type()==USB_XBOX){
                    if((gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.bEndpointAddress & USB_EP_DIR) == USB_EP_DIR_OUT){
                        ep_max = USB_XBOX_AUDIO_RX_BUFFER_LEN;
                    } else {
                        ep_max = USB_XBOX_AUDIO_TX_BUFFER_LEN;
                    }
                }
                #endif

                gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0] = ep_max;
                gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1] = ep_max >> 8;

                LOG_MSGID_I(USB_RESOURCE, "USB_Software_Create : index_if_alt:%d ep:%x ep_max:%d ", 3, index_if_alt,
                            gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.bEndpointAddress, ep_max);
#endif
                for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr.stdif.bNumEndpoints; index_ep++) {
                    if_len += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdscr_size;
                }
            }
        }
    }
    gUsbDevice.cfgdscr.wTotalLength = USB_CFGDSC_LENGTH + if_len;

    if (gUsbDevice.cfgdscr.wTotalLength > USB_DESCRIPTOR_BUFFER_SIZE) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Software_Create : cfgdscr.wTotalLength=%d bytes > %d bytes Fail", 2, gUsbDevice.cfgdscr.wTotalLength, USB_DESCRIPTOR_BUFFER_SIZE);
        assert(0 && "gUsbDevice.cfgdscr.wTotalLength > USB_DESCRIPTOR_BUFFER_SIZE.");
    }

    /* allocate configuration descriptor memory */
    if (gUsbDevice.conf == NULL) {
#ifdef FREERTOS_ENABLE
        gUsbDevice.conf = (uint8_t *)USB_Get_Memory(USB_DESCRIPTOR_BUFFER_SIZE);
#else
        gUsbDevice.conf = (uint8_t *)USB_TEST_Buffer;
#endif
    }

    /* Initialize the Standard Device_Qualiifier Descriptor */
    memcpy(&(gUsbDevice.dev_qual_dscr), dev_qual_dscr, USB_DEV_QUA_DSC_LENGTH);

    gUsbDevice.dev_qual_dscr.bDeviceClass = gUsbDevice.devdscr.bDeviceClass;
    gUsbDevice.dev_qual_dscr.bDeviceSubClass = gUsbDevice.devdscr.bDeviceSubClass;
    gUsbDevice.dev_qual_dscr.bDeviceProtocol = gUsbDevice.devdscr.bDeviceProtocol;
    gUsbDevice.dev_qual_dscr.bNumConfigurations = gUsbDevice.devdscr.bNumConfigurations;

    if( g_USB_Software_Speed_skip_enable==0 ) {
        USB_Software_Speed_Init(false);
    }
}

bool g_USB_Software_Speed_skip_enable = 1; // default on, disable by nvkey flag
bool g_USB_Software_Speed_Init = 0;

uint16_t USB_MaxPacketSize_to_value(uint16_t p1)
{
    uint16_t p1_val = p1 >> USB_RXMAP_M1_POS;
    p1_val = (p1_val+1) * (p1 & USB_RXMAP_MAX_PAYLOAD_MASK);
    return p1_val;
}

uint32_t USB_MaxPacketSize_is_bigger(uint16_t p1,uint16_t p2)
{
    uint16_t p1_val = USB_MaxPacketSize_to_value(p1);
    uint16_t p2_val = USB_MaxPacketSize_to_value(p2);
    //LOG_MSGID_I(USB_RESOURCE, "USB_MaxPacketSize_is_bigger p1_val(0x%x,%d) p2_val(0x%x,%d) ", 4, p1, p1_val, p2, p2_val);
    return (p1_val>p2_val) ? 1 : 0;
}

void USB_Software_Speed_Init(bool b_other_speed)
{
    uint32_t  index_if = 0;
    uint32_t  index_if_alt = 0;
    uint32_t  ep_max = 0;
    uint32_t  index_ep;
    uint32_t  index_iad;
    uint32_t  Conf_index;
    uint32_t  index;

    if (usb_ifcreate_number == 0) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Software_Speed_Init ifcreate_number is 0", 0);
    }

    if (gUsbDevice.conf == NULL) {
        assert(0);
    }

    if( g_USB_Software_Speed_skip_enable ) {
        if( g_USB_Software_Speed_Init ) {
            /* Re-adjust configuration descriptor */
            if (b_other_speed == true) {
                gUsbDevice.conf[1] = USB_OTHER_SPEED;
            } else {
                gUsbDevice.conf[1] = USB_CONFIG;
            }

            LOG_MSGID_I(USB_RESOURCE, "USB_Software_Speed_Init, skip", 0);
            return;
        } else {
            g_USB_Software_Speed_Init = 1;
        }
    }

    /* decide EP is high speed or full speed descriptors */
    for (index = 0; index < usb_ifcreate_number; index++) {
        if (usb_ifcreate_tbl[index].if_speed_func != NULL) {
            usb_ifcreate_tbl[index].if_speed_func(b_other_speed);
        } else {
            LOG_MSGID_E(USB_RESOURCE, "USB_Software_Speed_Init speed fuction is NULL", 0);
        }
    }

    memcpy((uint8_t *)(gUsbDevice.conf), (uint8_t *)&gUsbDevice.cfgdscr, USB_CFGDSC_LENGTH);
    Conf_index = USB_CFGDSC_LENGTH;

    /* Re-construct configuration descriptor */
    if (b_other_speed == true) {
        /**
         * HS/FS no feature changs.
         * OTHER_SPEED descriptor is same as CONFIG descriptor.
         * Only byte 2 bDescriptorType is different.
         */
        gUsbDevice.conf[1] = USB_OTHER_SPEED;
    }

    if (gUsbDevice.resource_iad_number == 0) {
        /* No IAD, not VIDEO nor composite, either */
        for (index_if = 0; index_if < gUsbDevice.resource_interface_number; index_if++) {
            memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *)&gUsbDevice.if_info[index_if].ifdscr.stdif, gUsbDevice.if_info[index_if].ifdscr_size);
            Conf_index += gUsbDevice.if_info[index_if].ifdscr_size;

            for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].ifdscr.stdif.bNumEndpoints; index_ep++) {
                memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *) & (gUsbDevice.if_info[index_if].ep_info[index_ep]->epdesc.stdep),
                       gUsbDevice.if_info[index_if].ep_info[index_ep]->epdscr_size);
                Conf_index += gUsbDevice.if_info[index_if].ep_info[index_ep]->epdscr_size;
            }

            for (index_if_alt = 0; index_if_alt < USB_MAX_INTERFACE_ALTERNATE_NUM; index_if_alt++) {
                if (gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size > 0) {
                    memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *)&gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr.stdif, gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size);
                    Conf_index += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size;

                    /* Correct the wMaxPacketSize of RX EP for USB-Audio */
                    #if defined(AIR_USB_AUDIO_ENABLE)
                    #ifdef AIR_USB_AUDIO_1_SPK_ENABLE
                    if(index_if == g_UsbAudio[0].stream_interface_id) {
                        ep_max = USB_Audio_Get_RX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                    }
                    #endif

                    #ifdef AIR_USB_AUDIO_2_SPK_ENABLE
                    if(index_if == g_UsbAudio[1].stream_interface_id) {
                        ep_max = USB_Audio_Get_RX_Alt_Byte(1, index_if_alt + 1); /* +1 for array offset */
                    }
                    #endif


                    #ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                    if(index_if == g_UsbAudio[0].stream_microphone_interface_id) {
                        ep_max = USB_Audio_Get_TX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                    }
                    #endif
                    #endif

                    #ifdef AIR_USB_XBOX_ENABLE
                    if(usb_get_device_type()==USB_XBOX){
                        if((gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.bEndpointAddress & USB_EP_DIR) == USB_EP_DIR_OUT){
                            ep_max = USB_XBOX_AUDIO_RX_BUFFER_LEN;
                        } else {
                            ep_max = USB_XBOX_AUDIO_TX_BUFFER_LEN;
                        }
                    }
                    #endif

                    uint16_t bak_MaxPacketSize = gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1];
                    bak_MaxPacketSize <<= 8;
                    bak_MaxPacketSize |= gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0];

                    gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0] = ep_max;
                    gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1] = ep_max >> 8;

                    LOG_MSGID_I(USB_RESOURCE, "USB_Software_Speed_Init index_if:%d index_if_alt:%d ep:%x ep_max:0x%x", 4, index_if, index_if_alt,
                                gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.bEndpointAddress, ep_max);

                    for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr.stdif.bNumEndpoints; index_ep++) {
                        memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *) & (gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdesc.stdep), gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdscr_size);
                        Conf_index += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdscr_size;
                    }

                    // keep real MaxPacketSize in EP info
                    if(USB_MaxPacketSize_is_bigger(bak_MaxPacketSize,ep_max)) {
                        //LOG_MSGID_I(USB_RESOURCE, "USB_Software_Speed_Init keep MAX EP: %d", 1,  bak_MaxPacketSize);

                        gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0] = bak_MaxPacketSize;
                        gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1] = bak_MaxPacketSize >> 8;
                    }
                }
            }
        }
    } else {
        for (index_iad = 0; index_iad < gUsbDevice.resource_iad_number; index_iad++) {
            memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *)&gUsbDevice.iad_desc[index_iad], gUsbDevice.iad_desc[index_iad].bLength);
            Conf_index += gUsbDevice.iad_desc[index_iad].bLength;

            for (index_if = gUsbDevice.iad_desc[index_iad].bFirstInterface; index_if < (gUsbDevice.iad_desc[index_iad].bFirstInterface + gUsbDevice.iad_desc[index_iad].bInterfaceCount); index_if++) {
                memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *)&gUsbDevice.if_info[index_if].ifdscr.stdif, gUsbDevice.if_info[index_if].ifdscr_size);
                Conf_index += gUsbDevice.if_info[index_if].ifdscr_size;

                for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].ifdscr.stdif.bNumEndpoints; index_ep++) {
                    memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *) & (gUsbDevice.if_info[index_if].ep_info[index_ep]->epdesc.stdep), gUsbDevice.if_info[index_if].ep_info[index_ep]->epdscr_size);
                    Conf_index += gUsbDevice.if_info[index_if].ep_info[index_ep]->epdscr_size;
                }

                for (index_if_alt = 0; index_if_alt < USB_MAX_INTERFACE_ALTERNATE_NUM; index_if_alt++) {
                    if (gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size > 0) {
                        memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *)&gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr.stdif, gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size);
                        Conf_index += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr_size;

                        /* Correct the wMaxPacketSize of RX EP for USB-Audio */
                        #if defined(AIR_USB_AUDIO_ENABLE)
                        #ifdef AIR_USB_AUDIO_1_SPK_ENABLE
                        if(index_if == g_UsbAudio[0].stream_interface_id) {
                            ep_max = USB_Audio_Get_RX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                        }
                        #endif

                        #ifdef AIR_USB_AUDIO_2_SPK_ENABLE
                        if(index_if == g_UsbAudio[1].stream_interface_id) {
                            ep_max = USB_Audio_Get_RX_Alt_Byte(1, index_if_alt + 1); /* +1 for array offset */
                        }
                        #endif

                        #ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                        if(index_if == g_UsbAudio[0].stream_microphone_interface_id) {
                            ep_max = USB_Audio_Get_TX_Alt_Byte(0, index_if_alt + 1); /* +1 for array offset */
                        }
                        #endif
                        #endif

                        #ifdef AIR_USB_XBOX_ENABLE
                        if(usb_get_device_type()==USB_XBOX){
                            ep_max = USB_XBOX_AUDIO_RX_BUFFER_LEN;
                        }
                        #endif

                        uint16_t bak_MaxPacketSize = gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1];
                        bak_MaxPacketSize <<= 8;
                        bak_MaxPacketSize |= gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0];

                        gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0] = ep_max;
                        gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1] = ep_max >> 8;

                        LOG_MSGID_I(USB_RESOURCE, "USB_Software_Speed_Init index_if:%d index_if_alt:%d ep:%x ep_max:0x%x", 4, index_if, index_if_alt,
                                    gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.bEndpointAddress, ep_max);

                        for (index_ep = 0; index_ep < gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ifdscr.stdif.bNumEndpoints; index_ep++) {
                            memcpy((uint8_t *)(gUsbDevice.conf + Conf_index), (uint8_t *) & (gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdesc.stdep), gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdscr_size);
                            Conf_index += gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[index_ep]->epdscr_size;
                        }

                        // keep real MaxPacketSize in EP info
                        if(USB_MaxPacketSize_is_bigger(bak_MaxPacketSize,ep_max)) {
                            //LOG_MSGID_I(USB_RESOURCE, "USB_Software_Speed_Init keep MAX EP: %d", 1,  bak_MaxPacketSize);

                            gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[0] = bak_MaxPacketSize;
                            gUsbDevice.if_info[index_if].alternate_if_info[index_if_alt].ep_info[0]->epdesc.stdep.wMaxPacketSize[1] = bak_MaxPacketSize >> 8;
                        }
                    }
                }
            }
        }

    }

    if (Conf_index != gUsbDevice.cfgdscr.wTotalLength) {
        LOG_MSGID_E(USB_RESOURCE, "USB_Software_Speed_Init wTotalLength %d is not match Conf_index:%d Fail", 2, gUsbDevice.cfgdscr.wTotalLength, Conf_index);
        assert(0);
    }
}

void USB_Software_PrevInit(void)
{
    g_USB_Software_Speed_Init = 0;
}

/* init EP of each interface (class specific) */
void USB_Software_Init(void)
{
    uint32_t index;

    for (index = 0; index < usb_ifcreate_number; index++) {
        if (usb_ifcreate_tbl[index].if_init_func != NULL) {
            usb_ifcreate_tbl[index].if_init_func();
        } else {
            LOG_MSGID_E(USB_RESOURCE, "USB_Software_Init Fail", 0);
        }
    }
}

/* Resume EP of each interface (class specific) */
void USB_Software_Resume(void)
{
    uint32_t index;

    for (index = 0; index < usb_ifcreate_number; index++) {
        if (usb_ifcreate_tbl[index].if_resume_func != NULL) {
            usb_ifcreate_tbl[index].if_resume_func();
        }
    }
}


/* Init EP of each interface (class specific) */
void USB_Software_Enable(void)
{
    uint32_t index;

    for (index = 0; index < usb_ifcreate_number; index++) {
        if (usb_ifcreate_tbl[index].if_enable_func != NULL) {
            usb_ifcreate_tbl[index].if_enable_func();
        } else {
            LOG_MSGID_E(USB_RESOURCE, "USB_Software_Enable Fail", 0);
        }
    }
}

/* Reset the resource maintain in this file */
static void USB_Resource_Reset(void)
{
    uint32_t temp_addr, temp_addr2;

    temp_addr = (uint32_t)&gUsbDevice.resource_ep_bulk_tx_number;
    temp_addr2 = (uint32_t)&gUsbDevice.resource_iad_number;

    /* Reset part of gUsbDevice */
    memset(&gUsbDevice.resource_ep_bulk_tx_number, 0, temp_addr2 - temp_addr);
}

/************************************************************
    USB Basic Info Init
*************************************************************/
static void USB_Basic_Info_Init(void)
{
    const usb_custom_class_info_t *class_info     = NULL;
    const usb_custom_product_info_t *product_info = NULL;
    const usb_custom_power_info_t *power_info     = NULL;

    bool overwrite_class   = false;
    bool overwrite_product = false;
    bool overwrite_power   = false;

#ifdef AIR_NVDM_ENABLE
    nvkey_status_t nvkey_status;
    uint32_t nvkey_size;
    usb_nvkey_basicinfo_t basicinfo;

    nvkey_size = sizeof(basicinfo);
    nvkey_status = nvkey_read_data(NVID_USB_BASICINFO, (uint8_t *)&basicinfo, &nvkey_size);

    LOG_MSGID_I(USB_RESOURCE, "USB_Basic_Info_Init nvkey read nvkey_status=%d", 1, nvkey_status);
    LOG_MSGID_I(USB_RESOURCE, "USB_Basic_Info_Init USB_CLASS:%d USB_PRODUCT:%d USB_POWER:%d", 3,
                basicinfo.use_class_info, basicinfo.use_product_info, basicinfo.use_power_info);
    overwrite_class   = (bool)basicinfo.use_class_info;
    overwrite_product = (bool)basicinfo.use_product_info;
    overwrite_power   = (bool)basicinfo.use_power_info;

    if (nvkey_status == NVKEY_STATUS_OK) {
        /* USB_NVKEY_BASICINFO_OFFSET_USE_CLASS is not open */
        /*
        if(overwrite_class) {
            class_info = (usb_custom_class_info_t*)&basicinfo.bcdUSB;
        }
        */
        if (overwrite_product) {
            product_info = (usb_custom_product_info_t *)&basicinfo.idVendor;
            // product_info.product_id  = basicinfo.idProduct;
            // product_info.vender_id   = basicinfo.idVendor;
        }
        if (overwrite_power) {
            power_info = (usb_custom_power_info_t *)&basicinfo.self_power;
        }
    }
#endif /* AIR_NVDM_ENABLE */

    /* If not use info in NVKEY, use the info of custom. */
    if (overwrite_class == false) {
        class_info = USB_Custom_Get_Class_Info();
    }
    if (overwrite_product == false) {
        product_info = USB_Custom_Get_Product_Info();
    }
    if (overwrite_power == false) {
        power_info = USB_Custom_Get_Power_Info();
    }

    USB_Basic_Info_Init_Class_Info(class_info);
    USB_Basic_Info_Init_Product_Info(product_info);
    USB_Basic_Info_Init_Power_Info(power_info);
}

static void USB_Basic_Info_Init_Class_Info(const usb_custom_class_info_t *info)
{
    if (info == NULL) {
        return;
    }
    gUsbDevice.devdscr.bcdUSB          = info->bcdUSB;
    gUsbDevice.devdscr.bDeviceClass    = info->class;
    gUsbDevice.devdscr.bDeviceSubClass = info->sub_class;
    gUsbDevice.devdscr.bDeviceProtocol = info->protocol;
}

static void USB_Basic_Info_Init_Product_Info(const usb_custom_product_info_t *info)
{
    if (info == NULL) {
        return;
    }
    gUsbDevice.devdscr.idProduct = info->product_id;
    gUsbDevice.devdscr.idVendor  = info->vender_id;
    gUsbDevice.devdscr.bcdDevice = info->bcd_version;
}

static void USB_Basic_Info_Init_Power_Info(const usb_custom_power_info_t *info)
{
    if (info == NULL) {
        return;
    }
    gUsbDevice.self_powered = info->self_power;
    gUsbDevice.remoteWk = info->remote_wakeup;

    gUsbDevice.cfgdscr.bmAttributes = USB_CFGDSC_ATTR_NATURE;
    if (info->self_power) {
        gUsbDevice.cfgdscr.bmAttributes |= USB_CFGDSC_ATTR_SELFPOWER;
    }
    if (info->remote_wakeup) {
        gUsbDevice.cfgdscr.bmAttributes |= USB_CFGDSC_ATTR_REMOTEWAKEUP;
    }

    /* Device max power = bMaxPower * 2 */
    gUsbDevice.cfgdscr.bMaxPower = info->maxpower / 2;
}

/************************************************************
    USB String Functions
*************************************************************/
// This is a temp function
void USB_String_Init(void)
{
    uint8_t nvkey_string_num = 0;

#ifdef AIR_NVDM_ENABLE
    nvkey_status_t nvkey_status __unused;
    uint32_t nvkey_size;
    usb_nvkey_string_num_t nvstrnum;

    nvstrnum.strnum = gUsbDevice.resource_string_number;
    nvkey_size      = sizeof(nvstrnum);
    nvkey_status    = nvkey_read_data(NVID_USB_STRING_NUM, (uint8_t *)&nvstrnum, &nvkey_size);
    LOG_MSGID_I(USB_RESOURCE, "USB_String_Init status:%d, string_num:%d", 2, nvkey_status, nvstrnum.strnum);
    nvkey_string_num = nvstrnum.strnum;
#endif /* AIR_NVDM_ENABLE */

    LOG_MSGID_I(USB_RESOURCE, "USB_String_Init nvkey_string_num:%d", 1, nvkey_string_num);
    if (nvkey_string_num) {
        USB_String_Nvkey_Read();
    } else {
        USB_String_Init_From_Custom_List();
    }
    USB_String_Create_Language_Id();
}

static void USB_String_Init_From_Custom_List(void)
{
    uint8_t numstr = 0;
    const usb_custom_string_t *custom_strlist;
    const usb_custom_string_t *s;
    int i;
    uint8_t auto_id = 1;

    custom_strlist = USB_Custom_Get_String_List(&numstr);
    for (i = 0; i < numstr; i++) {
        s = &custom_strlist[i];

        if (s->id == 0) {
            USB_String_Create(s->usage, auto_id, s->str);
            auto_id++;
        } else {
            USB_String_Create(s->usage, s->id, s->str);
        }
    }
}

void USB_String_Reset(void)
{
    gUsbDevice.resource_string_number = 0;
}

uint8_t USB_String_Create(usb_string_usage_t usage, uint8_t id, const char *str)
{
    gUsbDevice.resource_string_number++;
    USB_Check_Descriptor();
    usb_string_t *usbstr = &gUsbDevice.string[gUsbDevice.resource_string_number - 1];

    usbstr->usage = usage;
    usbstr->id = id;

    int i;
    for (i = 0; i < USB_STRING_MAX_LENGTH; i++) {
        usbstr->desc.word[i] = str[i];
        if (str[i] == '\0') {
            break;
        }
    }

    /**
     * length = 1 byte id + 1 byte desc_type + 2 bytes word * (i)
     *        = 2 + i * 2
     */
    usbstr->desc.length = 2 + i * 2;
    usbstr->desc.desc_type = USB_DEST_TYPE_STRING;
    return 0;
}

uint8_t USB_String_Get_Id_By_Usage(usb_string_usage_t usage)
{
    int i;
    uint8_t id = 0;
    uint8_t string_num = gUsbDevice.resource_string_number;

    for (i = 0; i < string_num; i++) {
        if (gUsbDevice.string[i].usage == usage) {
            id = gUsbDevice.string[i].id;
            break;
        }
    }
    return id;
}

usb_string_t *USB_String_Get_By_Usage(usb_string_usage_t usage)
{
    int i;
    usb_string_t *ret = NULL;
    uint8_t string_num = gUsbDevice.resource_string_number;

    for (i = 0; i < string_num; i++) {
        if (gUsbDevice.string[i].usage == usage) {
            ret = &(gUsbDevice.string[i]);
            break;
        }
    }

    if (ret == NULL) {
        LOG_MSGID_E(USB_RESOURCE, "USB_String_Get_By_Usage can't find matched USB_String", 0);
        assert(0);
    }

    return ret;
}

usb_string_t *USB_String_Get_By_Id(uint8_t id)
{
    int i;
    usb_string_t *ret = NULL;
    uint8_t string_num = gUsbDevice.resource_string_number;

    for (i = 0; i < string_num; i++) {
        if (gUsbDevice.string[i].id == id) {
            ret = &(gUsbDevice.string[i]);
            break;
        }
    }

    return ret;
}

/**
 * @brief Write one usb string to NVKEY.
 *
 * The NVKEY layout of usb string shows in below image.
 * And its total length is 2 + 1 + 31 = 34 bytes.
 * ----------------------------------
 * | uasge   | id     | char string |
 * | 2 bytes | 1 byte | 31 bytes    |
 * ----------------------------------
 */
#ifdef AIR_NVDM_ENABLE
static int USB_String_Nvkey_Write_One(uint16_t nvkey, usb_string_t *usbstr)
{
    nvkey_status_t nvkey_status;
    usb_nvkey_string_t nvstr;
    uint32_t nvkey_size = sizeof(usb_nvkey_string_t);
    int i;

    nvstr.usage = usbstr->usage;
    nvstr.id    = usbstr->id;

    for (i = 0; i < USB_STRING_MAX_LENGTH; i++) {
        nvstr.str[i] = usbstr->desc.word[i] & 0xFF;
    }

    nvkey_status = nvkey_write_data(nvkey, (uint8_t *)&nvstr, nvkey_size);
    return nvkey_status;
}

void USB_String_Nvkey_Write(void)
{
    nvkey_status_t nvkey_status __unused;
    uint32_t nvkey_size;
    usb_nvkey_string_num_t nvstrnum;
    int i;

    nvstrnum.strnum = gUsbDevice.resource_string_number;
    nvkey_size      = sizeof(nvstrnum);
    nvkey_status    = nvkey_write_data(NVID_USB_STRING_NUM, (uint8_t *)&nvstrnum, nvkey_size);
    LOG_MSGID_I(USB_RESOURCE, "USB_String_Nvkey_Write status:%d, string_num:%d", 2, nvkey_status, nvstrnum.strnum);

    for (i = 0; i < nvstrnum.strnum; i++) {
        nvkey_status = USB_String_Nvkey_Write_One(NVID_USB_STRING_0 + i, &gUsbDevice.string[i]);
    }
}

void USB_String_Nvkey_Read(void)
{
    nvkey_status_t nvkey_status __unused;
    uint32_t nvkey_size;
    usb_nvkey_string_t nvstr;
    usb_nvkey_string_num_t nvstrnum;
    int i;

    nvkey_size   = sizeof(nvstrnum);
    nvkey_status = nvkey_read_data(NVID_USB_STRING_NUM, (uint8_t *)&nvstrnum, &nvkey_size);

    LOG_MSGID_I(USB_RESOURCE, "USB_String_Nvkey_Read status:%d, string_num:%d", 2, nvkey_status, nvstrnum.strnum);

    for (i = 0; i < nvstrnum.strnum; i++) {
        nvkey_size   = sizeof(nvstr);
        nvkey_status = nvkey_read_data(NVID_USB_STRING_0 + i, (uint8_t *)&nvstr, &nvkey_size);
        USB_String_Create(nvstr.usage, nvstr.id, nvstr.str);
    }
}
#endif /* AIR_NVDM_ENABLE */

static void USB_String_Create_Language_Id(void)
{
    USB_String_Create(USB_STRING_USAGE_LANGSUPP, 0, "");
    USB_String_Get_By_Usage(USB_STRING_USAGE_LANGSUPP)->desc.word[0] = USB_LANGUAGUE_ID;
    /* length = 1 byte id + 1 byte desc_type + 2 bytes LANGUAGUE_ID = 4 bytes */
    USB_String_Get_By_Usage(USB_STRING_USAGE_LANGSUPP)->desc.length = 0x04;
}

#endif /* AIR_USB_ENABLE */

