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


#ifndef __USB_HOST_SPEC_H__
#define __USB_HOST_SPEC_H__

/* C library */
#include <stdint.h>

/* bmRequestType :D7 Data Phase Transfer Direction  */
#define  USB_REQ_DIR_MASK                                  0x80U
#define  USB_H2D                                           0x00U
#define  USB_D2H                                           0x80U

/* bmRequestType D6..5 Type */
#define  USB_REQ_TYPE_STANDARD                             0x00U
#define  USB_REQ_TYPE_CLASS                                0x20U
#define  USB_REQ_TYPE_VENDOR                               0x40U
#define  USB_REQ_TYPE_RESERVED                             0x60U

/* bmRequestType D4..0 Recipient */
#define  USB_REQ_RECIPIENT_DEVICE                          0x00U
#define  USB_REQ_RECIPIENT_INTERFACE                       0x01U
#define  USB_REQ_RECIPIENT_ENDPOINT                        0x02U
#define  USB_REQ_RECIPIENT_OTHER                           0x03U

/* bRequest , Value */
#define  USB_REQ_GET_STATUS                                0x00U
#define  USB_REQ_CLEAR_FEATURE                             0x01U
#define  USB_REQ_SET_FEATURE                               0x03U
#define  USB_REQ_SET_ADDRESS                               0x05U
#define  USB_REQ_GET_DESCRIPTOR                            0x06U
#define  USB_REQ_SET_DESCRIPTOR                            0x07U
#define  USB_REQ_GET_CONFIGURATION                         0x08U
#define  USB_REQ_SET_CONFIGURATION                         0x09U
#define  USB_REQ_GET_INTERFACE                             0x0AU
#define  USB_REQ_SET_INTERFACE                             0x0BU
#define  USB_REQ_SYNCH_FRAME                               0x0CU

/* Table 9-5. Descriptor Types of USB Specifications */
#define  USB_DESC_TYPE_DEVICE                              0x01U
#define  USB_DESC_TYPE_CONFIGURATION                       0x02U
#define  USB_DESC_TYPE_STRING                              0x03U
#define  USB_DESC_TYPE_INTERFACE                           0x04U
#define  USB_DESC_TYPE_ENDPOINT                            0x05U
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                    0x06U
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION           0x07U
#define  USB_DESC_TYPE_INTERFACE_POWER                     0x08U
#define  USB_DESC_TYPE_HID                                 0x21U
#define  USB_DESC_TYPE_HID_REPORT                          0x22U

/* Descriptor Type and Descriptor Index  */
/* Use the following values when Filling Setup packet  */
#define  USB_DESC_DEVICE                    ((USB_DESC_TYPE_DEVICE << 8) & 0xFF00U)
#define  USB_DESC_CONFIGURATION             ((USB_DESC_TYPE_CONFIGURATION << 8) & 0xFF00U)
#define  USB_DESC_STRING                    ((USB_DESC_TYPE_STRING << 8) & 0xFF00U)
#define  USB_DESC_INTERFACE                 ((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00U)
#define  USB_DESC_ENDPOINT                  ((USB_DESC_TYPE_INTERFACE << 8) & 0xFF00U)
#define  USB_DESC_DEVICE_QUALIFIER          ((USB_DESC_TYPE_DEVICE_QUALIFIER << 8) & 0xFF00U)
#define  USB_DESC_OTHER_SPEED_CONFIGURATION ((USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION << 8) & 0xFF00U)
#define  USB_DESC_INTERFACE_POWER           ((USB_DESC_TYPE_INTERFACE_POWER << 8) & 0xFF00U)
#define  USB_DESC_HID_REPORT                ((USB_DESC_TYPE_HID_REPORT << 8) & 0xFF00U)
#define  USB_DESC_HID                       ((USB_DESC_TYPE_HID << 8) & 0xFF00U)


typedef struct _usb_host_SetupPkt {
    uint8_t           bmRequestType;
    uint8_t           bRequest;
    uint16_t          wValue;
    uint16_t          wIndex;
    uint16_t          wLength;
} usb_host_SetupPkt_t;


typedef struct _usb_host_descriptor_device {
    uint8_t bLength;                    /* Size of this descriptor in bytes */
    uint8_t bDescriptorType;            /* DEVICE Descriptor Type */
    uint8_t bcdUSB[2];                  /* UUSB Specification Release Number in Binary-Coded Decimal, e.g. 0x0200U */
    uint8_t bDeviceClass;               /* Class code */
    uint8_t bDeviceSubClass;            /* Sub-Class code */
    uint8_t bDeviceProtocol;            /* Protocol code */
    uint8_t bMaxPacketSize0;            /* Maximum packet size for endpoint zero */
    uint8_t idVendor[2];                /* Vendor ID (assigned by the USB-IF) */
    uint8_t idProduct[2];               /* Product ID (assigned by the manufacturer) */
    uint8_t bcdDevice[2];               /* Device release number in binary-coded decimal */
    uint8_t iManufacturer;              /* Index of string descriptor describing manufacturer */
    uint8_t iProduct;                   /* Index of string descriptor describing product */
    uint8_t iSerialNumber;              /* Index of string descriptor describing the device serial number */
    uint8_t bNumConfigurations;         /* Number of possible configurations */
} usb_host_descriptor_device_t;

typedef struct _usb_host_descriptor_configuration {
    uint8_t bLength;                    /* Descriptor size in bytes = 9U */
    uint8_t bDescriptorType;            /* CONFIGURATION type = 2U or 7U */
    uint8_t wTotalLength[2];            /* Length of concatenated descriptors */
    uint8_t bNumInterfaces;             /* Number of interfaces, this configuration. */
    uint8_t bConfigurationValue;        /* Value to set this configuration. */
    uint8_t iConfiguration;             /* Index to configuration string */
    uint8_t bmAttributes;               /* Configuration characteristics */
    uint8_t bMaxPower;                  /* Maximum power from bus, 2 mA units */
} usb_host_descriptor_configuration_t;


#endif
