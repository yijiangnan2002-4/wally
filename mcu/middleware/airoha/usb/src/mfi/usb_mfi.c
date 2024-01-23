/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_USB_MFI_ENABLE

/* C library */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Hal includes */
#include "hal_pmu.h"

/* USB Middleware includes */
#include "usb.h"
#include "usb_resource.h"
#include "usb_mfi.h"

/* Syslog create module for usb_mfi.c */
#include "usb_dbg.h"
log_create_module_variant(USB_MFI, DEBUG_LOG_ON, PRINT_LEVEL_INFO);


/*******************************************************
*   MFI Gobal Variable
*******************************************************/
#define USB_MFI_TX_WAIT             0x1000

Usb_MFI_Struct_Info g_Usb_MFI;
Usb_MFI_Struct_CB   g_Usb_MFI_CB;


/*******************************************************
*   MFI Descriptor
*******************************************************/
/* Interface Descriptor */
static USB_MFI_Interface_t mfi_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00,                   /* USB_MFI_If_Create set this parameter */
    .bAlternateSetting  = 0x00,                   /* Unused */
    .bNumEndpoints      = USB_MFI_EP_NUM,
    .bInterfaceClass    = USB_MFI_IF_CLASS,
    .bInterfaceSubClass = USB_MFI_IF_SUBCLASS,
    .bInterfaceProtocol = 0x00,                   /* Unused */
    .iInterface         = 0x00,                   /* USB_MFI_If_Create set this parameter */
};

/* Bulk OUT Endpoint Descriptor */
static USB_MFI_Dscr_Endpoint_t mfi_ep_out_dscr = {
    .bLength          = USB_EPDSC_LENGTH,
    .bDescriptorType  = USB_ENDPOINT,
    .bEndpointAddress = 0x00,                     /* USB_MFI_If_Create set this parameter */
    .bmAttributes     = USB_EP_BULK,
    .wMaxPacketSize   = USB_MFI_BULK_OUT_LEN,
    .bInterval        = 0x00,                     /* Unused */
};

/* Bulk IN Endpoint Descriptor */
static USB_MFI_Dscr_Endpoint_t mfi_ep_in_dscr = {
    .bLength          = USB_EPDSC_LENGTH,
    .bDescriptorType  = USB_ENDPOINT,
    .bEndpointAddress = 0x00,                     /* USB_MFI_If_Create set this parameter */
    .bmAttributes     = USB_EP_BULK,
    .wMaxPacketSize   = USB_MFI_BULK_IN_LEN,
    .bInterval        = 0x00,                     /* Unused */
};


/*******************************************************
*   Enable Function
*******************************************************/
void usb_mfi_set_dscr_enable(bool enable)
{
    LOG_MSGID_I(USB_MFI, "usb_mfi_set_dscr_enable enable:%d", 1, enable);

    g_Usb_MFI.enable = true;
}

bool usb_mfi_get_dscr_enable()
{
    LOG_MSGID_I(USB_MFI, "usb_mfi_get_dscr_enable enable:%d", 1, g_Usb_MFI.enable);

    return g_Usb_MFI.enable;
}

/*******************************************************
*   Initialize and Rlease Function
*******************************************************/
void USB_Init_MFI_Status(void)
{
    LOG_MSGID_I(USB_MFI, "USB_Init_MFI_Status", 0);

    g_Usb_MFI.init     = true;
    g_Usb_MFI.if_id    = 0;
}

void USB_Release_MFI_Status(void)
{
    g_Usb_MFI.init     = false;
    g_Usb_MFI.if_id    = 0;
}


/*******************************************************
*   Register Callback
*******************************************************/
void USB_MFI_Register_Rx_Callback(USB_MFI_RX_FUNC rx_cb)
{
    if (rx_cb != NULL) {
        LOG_MSGID_I(USB_MFI, "USB_MFI_Register_Rx_Callback rx_cb[0x%X]", 1, rx_cb);
    } else {
        LOG_MSGID_E(USB_MFI, "USB_MFI_Register_Rx_Callback rx_cb is NULL!!", 0);
    }

    g_Usb_MFI_CB.rx_cb = rx_cb;
}


/*******************************************************
*   Endpoint Handler
*******************************************************/
uint8_t usb_mfi_rx_buf[USB_MFI_BULK_OUT_LEN] = {0};

/* Bulk Out EP handler, called by EP interrupt */
void USB_MFI_BulkOut_Hdr(void)
{
    uint8_t ep_out_id;
    uint32_t i, len;

    /* Get endpoint ID */
    ep_out_id = g_Usb_MFI.ep_out_id;

    /* Get data length from EP */
    len = hal_usb_get_rx_packet_length(ep_out_id);

    /* Check data length is valid*/
    if(len > USB_MFI_BULK_OUT_LEN){
        LOG_MSGID_E(USB_MFI, "USB_MFI_BulkOut_Hdr len:%d > Max data len:%d", 2, len, USB_MFI_BULK_OUT_LEN);
    }
    if(len == 0){
        LOG_MSGID_E(USB_MFI, "USB_MFI_BulkOut_Hdr len:%d is zero", 1, len);
    }

    LOG_MSGID_I(USB_MFI, "USB_MFI_BulkOut_Hdr ep_out_id[%d] len[%d] callback[0x%X]", 3, ep_out_id, len, g_Usb_MFI_CB.rx_cb);

    /* Get data from EP */
    hal_usb_read_endpoint_fifo(ep_out_id, len, usb_mfi_rx_buf);
    hal_usb_set_endpoint_rx_ready(ep_out_id);

    for (i = 0; i < len; i += 8) {
        LOG_MSGID_I(USB_MFI, "USB_MFI_BulkOut_Hdr [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - %d", 9,
                    usb_mfi_rx_buf[i + 0], usb_mfi_rx_buf[i + 1], usb_mfi_rx_buf[i + 2], usb_mfi_rx_buf[i + 3],
                    usb_mfi_rx_buf[i + 4], usb_mfi_rx_buf[i + 5], usb_mfi_rx_buf[i + 6], usb_mfi_rx_buf[i + 7], i);
    }

    /* Call IAP RX callback */
    if (g_Usb_MFI_CB.rx_cb != NULL) {
        g_Usb_MFI_CB.rx_cb(len, usb_mfi_rx_buf);
    } else {
        LOG_MSGID_E(USB_MFI, "USB_MFI_BulkOut_Hdr callback is NULL!!", 0);
    }

    /* Reset mfi rx buffer */
    memset(usb_mfi_rx_buf, 0, USB_MFI_BULK_OUT_LEN);
}

/* Bulk IN EP handler, called by EP interrupt */
void USB_MFI_BulkIn_Hdr(void)
{
    // LOG_MSGID_I(USB_MFI, "USB_MFI_BulkIn_Hdr", 0);
    /* no thing to do... */
}

/* Bulk OUT EP reset handler */
void USB_MFI_BulkOut_Reset(void)
{
    /* no thing to do... */
}

/* Bulk IN EP reset handler */
void USB_MFI_BulkIn_Reset(void)
{
    /* no thing to do... */
}


/*******************************************************
*   Send Data API
*******************************************************/
USB_MFI_STATUS_t USB_MFI_TX_SendData(uint32_t len, uint8_t *data)
{
    bool chrger_detect;
    uint32_t ep_in_id;
    uint32_t data_len, left_len, send_len;
    uint32_t usb_wait;

    /* Check USB MFI is enabled */
    if(usb_mfi_get_dscr_enable() == false){
        LOG_MSGID_E(USB_MFI, "USB_MFI_TX_SendData MFI is not enabled", 0);
        return USB_MFI_IS_NOT_ENABLED;
    }

    /* Check the charger (Vbus) is ready for USB which means the USB cable is connected or not */
    chrger_detect = pmu_get_chr_detect_value();
    if (chrger_detect != PMU_OK) {
        LOG_MSGID_E(USB_MFI, "USB_MFI_TX_SendData detect charger FAIL!", 0);
        return USB_MFI_CHARGER_DETECT_ERROR;
    }

    /* Check USB enumeration is finished */
    if (USB_Get_Device_State() != DEVSTATE_CONFIG) {
        LOG_MSGID_E(USB_MFI, "USB_MFI_TX_SendData not in config state", 0);
        return USB_MFI_NOT_IN_CONFIG_STATE;
    }

    /* Get endpoint ID */
    ep_in_id = g_Usb_MFI.ep_in_id;

    LOG_MSGID_I(USB_MFI, "USB_MFI_TX_SendData ep_in_id[%d] len[%d] data[0x%X]", 3, ep_in_id, len, data);

    /* Check the length is valid*/
    if (len == 0) {
        LOG_MSGID_E(USB_MFI, "USB_MFI_TX_SendData data length is 0", 0);
        return USB_MFI_LEN_IS_ZERO;
    }

    /* Send MFI data */
    left_len = len;
    send_len = 0;

    while(left_len > 0) {
        /* Send packet by max size */
        if(left_len > USB_MFI_BULK_IN_LEN){
            data_len = USB_MFI_BULK_IN_LEN;
        } else {
            data_len = left_len;
        }

        /* Send data by EP
           Don't set up toggle bit to flush FIFO, because it will clear toggle bit to cause transmission miss.
        */
        hal_usb_clear_tx_endpoint_fifo(ep_in_id, HAL_USB_EP_TRANSFER_BULK, false);
        hal_usb_write_endpoint_fifo(ep_in_id, data_len, (data + send_len));
        hal_usb_set_endpoint_tx_ready(ep_in_id);

        /* Wait for finishing send data */
        usb_wait = 0;
        while (!hal_usb_is_endpoint_tx_empty(ep_in_id)) {
            if (usb_wait > USB_MFI_TX_WAIT) {
                LOG_MSGID_E(USB_MFI, "USB_MFI_TX_SendData wait:%d ERROR!", 1, usb_wait);
                return USB_MFI_SEND_DATA_ERROR;
            }
            usb_wait++;
        }

        left_len -= data_len;
        send_len += data_len;
    }

    /* Print data */
    uint32_t i;
    for (i = 0; i < len; i += 8) {
        LOG_MSGID_I(USB_MFI, "USB_MFI_TX_SendData [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - %d", 9,
                    data[i+0], data[i+1], data[i+2], data[i+3], data[i+4], data[i+5], data[i+6], data[i+7], i);
    }

    return USB_MFI_STATUS_OK;
}


/*******************************************************
*   Interface Function
*******************************************************/
void USB_MFI_If_Create(void *if_name)
{
    uint8_t if_id, ep_in_id, ep_out_id;

    /* Get resource number and register to gUsbDevice */
    g_Usb_MFI.if_info     = USB_Get_Interface_Number(&if_id);
    g_Usb_MFI.if_id       = if_id;

    /* Get ep out id */
    g_Usb_MFI.ep_out_info = USB_Get_Bulk_Rx_Ep(&ep_out_id);
    g_Usb_MFI.ep_out_id   = ep_out_id;

    /* Get ep in id */
    g_Usb_MFI.ep_in_info  = USB_Get_Bulk_Tx_Ep(&ep_in_id);
    g_Usb_MFI.ep_in_id    = ep_in_id;

    LOG_MSGID_I(USB_MFI, "USB_MFI_If_Create if_id[%x] ep_out_id[%x] ep_in_id[%x]", 3, if_id, ep_out_id, ep_in_id);

    /* Set interface descriptor */
    mfi_if_dscr.bInterfaceNumber = if_id;
    mfi_if_dscr.iInterface       = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_MFI);

    /* Set endpoint descriptor */
    mfi_ep_out_dscr.bEndpointAddress = (USB_EP_DIR_OUT | ep_out_id);
    mfi_ep_in_dscr.bEndpointAddress  = (USB_EP_DIR_IN  | ep_in_id);

    /* Generate interface descriptor */
    g_Usb_MFI.if_info->interface_name_ptr = (char *) if_name;
    g_Usb_MFI.if_info->ifdscr_size     = sizeof(mfi_if_dscr);

    /* Generate endpoint descriptor */
    g_Usb_MFI.ep_out_info->ep_status.epout_status.byEP   = ep_out_id;
    g_Usb_MFI.ep_in_info->ep_status.epin_status.byEP     = ep_in_id;
    g_Usb_MFI.ep_out_info->epdscr_size = sizeof(mfi_ep_out_dscr);
    g_Usb_MFI.ep_in_info->epdscr_size  = sizeof(mfi_ep_in_dscr);

    /* Related endpoint info and class specific command handler */
    g_Usb_MFI.if_info->ep_info[0] = g_Usb_MFI.ep_out_info;
    g_Usb_MFI.if_info->ep_info[1] = g_Usb_MFI.ep_in_info;

    /* Descriptor */
    memcpy((uint32_t *) &(g_Usb_MFI.if_info->ifdscr.stdif),     (uint32_t *) &(mfi_if_dscr),     sizeof(Usb_If_Dscr));
    memcpy((uint32_t *) &(g_Usb_MFI.ep_out_info->epdesc.stdep), (uint32_t *) &(mfi_ep_out_dscr), sizeof(Usb_Ep_Dscr));
    memcpy((uint32_t *) &(g_Usb_MFI.ep_in_info->epdesc.stdep),  (uint32_t *) &(mfi_ep_in_dscr),  sizeof(Usb_Ep_Dscr));

    /* Reset handler */
    g_Usb_MFI.ep_out_info->ep_reset = USB_MFI_BulkOut_Reset;
    g_Usb_MFI.ep_in_info->ep_reset  = USB_MFI_BulkIn_Reset;

    /* Endpoint handler */
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP_RX, ep_out_id, USB_MFI_BulkOut_Hdr);
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP_TX, ep_in_id,  USB_MFI_BulkIn_Hdr);

}

void USB_MFI_If_Reset(void)
{
    /* no thing to do... */
}

void USB_MFI_If_Enable(void)
{
    LOG_MSGID_I(USB_MFI, "USB_MFI_If_Enable", 0);

    uint8_t ep_in_id, ep_out_id;

    ep_in_id  = g_Usb_MFI.ep_in_id;
    ep_out_id = g_Usb_MFI.ep_out_id;

    /* Enable Bulk IN/OUT EP - Non-DMA */
    hal_usb_enable_tx_endpoint(ep_in_id,  HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_NO_DMA, false);
    hal_usb_enable_rx_endpoint(ep_out_id, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_NO_DMA, false);

    /* Clear USB EP FIFO */
    hal_usb_clear_tx_endpoint_fifo(ep_in_id,  HAL_USB_EP_TRANSFER_BULK, false);
    hal_usb_clear_rx_endpoint_fifo(ep_out_id, HAL_USB_EP_TRANSFER_BULK, false);
}

void USB_MFI_If_Speed_Reset(bool b_other_speed)
{
    LOG_MSGID_I(USB_MFI, "USB_MFI_If_Speed_Reset", 0);

    uint32_t max_size;

    /* Set up the maximum byte of packet of Bulk IN/OUT endpoint */
    max_size = USB_MFI_BULK_OUT_LEN;

    g_Usb_MFI.ep_out_info->epdesc.stdep.wMaxPacketSize[0] = ((max_size)      & 0xFF);
    g_Usb_MFI.ep_out_info->epdesc.stdep.wMaxPacketSize[1] = ((max_size >> 8) & 0xFF);

    max_size = USB_MFI_BULK_IN_LEN;
    g_Usb_MFI.ep_in_info->epdesc.stdep.wMaxPacketSize[0] = ((max_size)      & 0xFF);
    g_Usb_MFI.ep_in_info->epdesc.stdep.wMaxPacketSize[1] = ((max_size >> 8) & 0xFF);
}

#endif /* AIR_USB_MFI_ENABLE */

