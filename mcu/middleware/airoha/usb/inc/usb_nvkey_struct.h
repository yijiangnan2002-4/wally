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

#ifndef USB_NVKEY_STRUCT_H
#define USB_NVKEY_STRUCT_H

#ifdef AIR_USB_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

/* C library */
#include <stdint.h>

/**
 * Address: 0x3B00
 * Name:    NVID_USB_SETTING_0
 */
typedef struct __attribute__((__packed__)) {
    uint8_t hs_fs : 1;              /* bit 0 */
    uint8_t duble_audio_device : 1; /* bit 1 */
    uint8_t byte0_bit2 : 1;
    uint8_t byte0_bit3 : 1;
    uint8_t byte0_bit4 : 1;
    uint8_t byte0_bit5 : 1;
    uint8_t byte0_bit6 : 1;
    uint8_t byte0_bit7 : 1;
} usb_nvkey_device_cfg_t;

/**
 * Address: 0x3B01 ~ 0x3B04
 * Name:    NVID_USB_SETTING_n (n=1~4)
 */
typedef struct __attribute__((__packed__)) {
    uint16_t terminal_type;
    uint16_t cur;
    uint16_t min;
    uint16_t max;
    uint16_t res;
} usb_nvkey_audio_device_cfg_t;

/**
 * Address: 0x3B05
 * Name:    NVID_USB_DBG
 *
 * dbg_filter_n : debug log open or not
 * dbg_option_n : switch code flow to debug
 */
typedef struct __attribute__((__packed__)) {
    uint32_t dbg_filter_1;
    uint32_t dbg_filter_2;
    uint32_t dbg_option_1;
    uint32_t dbg_option_2;
    uint32_t reserved_1;
    uint32_t reserved_2;
} usb_nvkey_dbg_t;

/**
 * Address: 0x3B10
 * Name:    NVID_USB_BASICINFO
 */
typedef struct __attribute__((__packed__)) {
    uint8_t use_class_info;
    uint8_t use_product_info;
    uint8_t use_power_info;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t self_power;
    uint8_t remote_wakeup;
    uint16_t max_power;
} usb_nvkey_basicinfo_t;

/**
 * Address: 0x3B11
 * Name:    NVID_USB_STRING_NUM
 */
typedef struct __attribute__((__packed__)) {
    uint8_t strnum;
} usb_nvkey_string_num_t;

/**
 * Address: 0x3BE0 ~ 0x3BEF
 * Name:    NVID_USB_STRING_n (n=0~15)
 */
#define USB_NVKEY_STRING_MAX_LENGTH 31
typedef struct __attribute__((__packed__)) {
    uint16_t usage;
    uint8_t id;
    char str[USB_NVKEY_STRING_MAX_LENGTH];
} usb_nvkey_string_t;

#ifdef __cplusplus
}
#endif

#endif /* AIR_USB_ENABLE */

#endif /* USB_NVKEY_STRUCT_H */

