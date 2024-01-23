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

#ifndef USB_MFI_H
#define USB_MFI_H

/* C library */
#include <stdbool.h>
#include <stdint.h>

/* USB Middleware includes */
#include "usb_resource.h"


/* Interface(IF) Descriptor */
#define USB_MFI_IF_CLASS                  0xFF    // Vendor-specific interface
#define USB_MFI_IF_SUBCLASS               0xF0    // MFi Accessory
#define USB_MFI_EP_NUM                    0x02    // 1 Bulk IN and 1 Bulk OUT endpoint

/* USB MFI Endpoint Length Definition */
#define USB_MFI_BULK_IN_LEN                 64
#define USB_MFI_BULK_OUT_LEN                64

/* USB MFI Status */
typedef enum {
    USB_MFI_STATUS_OK              =  0,
    USB_MFI_IS_NOT_ENABLED         =  1,
    USB_MFI_CHARGER_DETECT_ERROR   =  2,
    USB_MFI_NOT_IN_CONFIG_STATE    =  3,
    USB_MFI_LEN_IS_ZERO            =  4,
    USB_MFI_LEN_IS_TOO_LARGE       =  5,
    USB_MFI_SEND_DATA_ERROR        =  6,
} USB_MFI_STATUS_t;

/* USB MFI Driver Structure */
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} USB_MFI_Interface_t;

typedef struct __attribute__((packed)) {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} USB_MFI_Dscr_Endpoint_t;

/* Information Structure */
typedef struct {
    /* Flag */
    bool                 enable;
    bool                 init;
    /* ID */
    uint8_t              if_id;
    uint8_t              ep_in_id;
    uint8_t              ep_out_id;
    /* Infomation */
    Usb_Interface_Info  *if_info;
    Usb_Ep_Info         *ep_in_info;
    Usb_Ep_Info         *ep_out_info;
} Usb_MFI_Struct_Info;

/* Callback Definition */
typedef void (*USB_MFI_RX_FUNC)(uint8_t len, uint8_t *data);

/* Callback Function */
typedef struct {
    bool                init;
    USB_MFI_RX_FUNC     rx_cb;
} Usb_MFI_Struct_CB;

/* Enable Function */
void usb_mfi_set_dscr_enable(bool enable);
bool usb_mfi_get_dscr_enable();

/* Initialize and Release Function */
void USB_Init_MFI_Status(void);
void USB_Release_MFI_Status(void);

/* Register Callback */
void USB_MFI_Register_Rx_Callback(USB_MFI_RX_FUNC rx_cb);

/* Send MFI Data */
USB_MFI_STATUS_t USB_MFI_TX_SendData(uint32_t len, uint8_t *data);

/* Interface Function */
void USB_MFI_If_Create(void *if_name);
void USB_MFI_If_Enable(void);
void USB_MFI_If_Reset(void);
void USB_MFI_If_Speed_Reset(bool b_other_speed);

#endif /*  USB_MFI_H */

