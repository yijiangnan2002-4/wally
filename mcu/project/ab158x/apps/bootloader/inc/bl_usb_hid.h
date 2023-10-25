/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifndef _BL_USB_HID_H_
#define _BL_USB_HID_H_

#include <stdint.h>
#include <stdbool.h>

#define BL_USB_HID_DFU_VID 0x0E8D
#define BL_USB_HID_DFU_PID 0x0004
#define BL_USB_HID_DFU_VER 0x0100

/**
 * Device Name for USB HID DFU Device
 * Length must less than 31 bytes.
 */
#define BL_USB_HID_DFU_NAME "Airoha USB HID DFU Device"

/**
 * The timeout value of usb enumerating in bl_usb_hid_init.
 * The unit is ms.
 */
#define BL_USB_HID_INIT_TIMEOUT 10000

/**
 * The timeout value of usb rx.
 * The unit is ms.
 */
#define BL_USB_HID_RX_TIMEOUT 50

/**
 * The timeout value of usb tx.
 * The unit is ms.
 */
#define BL_USB_HID_TX_TIMEOUT 1000

#define BL_USB_HID_PACKET_PAYLOAD_MAXLEN   0x03FC

typedef enum {
    BL_USB_HID_STATUS_OK = 0,
    BL_USB_HID_STATUS_TIMEOUT = 1,
    BL_USB_HID_STATUS_IS_ALREADY_INIT = 2,
    BL_USB_HID_STATUS_IS_ALREADY_DEINIT = 3,
    BL_USB_HID_STATUS_IS_NOT_INIT = 4,
    BL_USB_HID_STATUS_GET_LENGTH_ZERO = 5,
    BL_USB_HID_VBUS_INVALID = 6,
} bl_usb_hid_status_t;

bool bl_usb_hid_is_ready(void);
uint8_t bl_usb_hid_init(void);
uint8_t bl_usb_hid_deinit(void);
uint8_t bl_usb_hid_get(uint8_t *data, uint32_t length, uint32_t *rxlen);
uint8_t bl_usb_hid_put(uint8_t *data, uint32_t length, uint32_t *txlen);

#endif  /* _BL_USB_HID_H_ */


