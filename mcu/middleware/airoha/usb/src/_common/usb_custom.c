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
#include <assert.h>
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
#include "usb_resource.h"

#ifdef AIR_USB_XBOX_ENABLE
#include "usb_xbox.h"
#endif /* AIR_USB_XBOX_ENABLE */

/* Hal includes */
#include "hal_usb_internal.h"

/* Other includes */
#include "memory_attribute.h"

/************************************************************
    External variables
*************************************************************/

/************************************************************
    Device descriptor parameters
*************************************************************/
static const char usb_string_manufacturer[] = "MediaTek Inc";

#if defined(AIR_USB_DONGLE_PROJECT_ENABLE)
static const char usb_string_product[] = "Airoha Dongle";
#else
static const char usb_string_product[] = "Airoha Headset";
#endif

#ifdef AIR_USB_HID_ENABLE
static char usb_serial_number[USB_STRING_MAX_LENGTH] = "0000000000000000";
#else
static char usb_serial_number[USB_STRING_MAX_LENGTH] = "CDC_Demo";
#endif

/************************************************************
    CDC ACM parameters
*************************************************************/
/**
 * NOTE:
 * AIR_USB_CDC_ENABLE is always open now.
 * Use __unused macro to avoid build warning.
 */
#ifdef AIR_USB_CDC_ENABLE
static const char usb_string_acm_comm_interface[] __unused = "COM(comm_if)";
static const char usb_string_acm_data_interface[] __unused = "COM(data_if)";
#endif

/************************************************************
    USB HID Naming
*************************************************************/
/**
 * NOTE:
 * HID device string use audio1 device string now.
 * Microsoft Teams on MAC need Audio device and HID device keep the same name,
 * or HID report would not work.
 * Look variable custom_string_list's USB_STRING_USAGE_HID member.
 *
 * Use __unused macro to avoid build warning.
 */
#ifdef AIR_USB_HID_ENABLE
static const char usb_string_hid[] __unused = "Airoha HID";
#endif

/************************************************************
    USB Audio Naming
*************************************************************/
/* Audio1 Naming */
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE) && defined(AIR_USB_AUDIO_ENABLE)
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
static const char usb_string_audio1[] __unused = "AIROHA Dongle Chat";
#else
static const char usb_string_audio1[] __unused = "AIROHA Chat";
#endif
#else /*only one speaker*/
static const char usb_string_audio1[] __unused = "AIROHA Speaker";
#endif

/* Audio2 Naming */
#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
static const char usb_string_audio2[] __unused = "AIROHA Dongle Media";
#else
static const char usb_string_audio2[] __unused = "AIROHA Media";
#endif
#else
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
static const char usb_string_audio2[] __unused = "AIROHA Dongle Game";
#else
static const char usb_string_audio2[] __unused = "AIROHA Game";
#endif
#endif

/************************************************************
    TEAMS - UCQ String
*************************************************************/
#ifdef AIR_MS_TEAMS_ENABLE
const char USB_TEAMS_UCQ_STRING[] = {
    'U',    // Qualified Skype for Business Devices
    'C',    // -
    'Q',    // -
    '0',    // Display Supported
    '0',    // Speakerphone
    '0',    // Handset
    '1',    // Heddset
    '1',    // AEC (Acoustic Echo Cancellation)
    '0',    // RESERVED
    '0',    // RESERVED
    '0',    // RESERVED
    '1',    // Wireless
    '0',    // Skype for Business HID Version (major)
    '1',    // -
    '0',    // Skype for Business HID Version (major)
    '0',    // -
    '0',    // SIP endpoint
    '\0'
};
#endif

/*******************************************************
*   XBOX String
*******************************************************/
#ifdef AIR_USB_XBOX_ENABLE
const char USB_XBOX_MS_OS_STRING[] = {
    'M', 'S', 'F', 'T', '1', '0', '0',    /* qwSignature: Signature field */
    0x0090,    /* bPad(MSB): Pad filed; bMS_VendorCode(LSB): Vendor specified code (default is 0x90) */
    '\0'
};

const char USB_XBOX_CONTROL_STRING[] = "AIROHA XBOX CONTROL";
const char USB_XBOX_AUDIO_STRING[] = "AIROHA XBOX AUDIO";
#endif

/*******************************************************
    KBM String
*******************************************************/
static const char usb_string_ms[] __unused = "AIROHA Mouse";
static const char usb_string_kb[] __unused = "AIROHA Keyboard";

/************************************************************
    Custom String Create list
*************************************************************/
/**
 * @brief The list for USB strings created in usb_sofrware_create.
 * If the id field is zero, id will be autio assigned from number 1.
 * reference: USB_Custom_Get_String_List
 */
static usb_custom_string_t custom_string_list[] = {
    /* {usage, id, pointer of string} */
    {USB_STRING_USAGE_VENDOR,   0, usb_string_manufacturer},
    {USB_STRING_USAGE_PRODUCT,  0, usb_string_product},
    {USB_STRING_USAGE_SERIAL,   0, usb_serial_number},

#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
#if defined(AIR_USB_AUDIO_ENABLE)
    {USB_STRING_USAGE_AUDIO1,   0, usb_string_audio1},
    {USB_STRING_USAGE_AUDIO2,   0, usb_string_audio2},
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    {USB_STRING_USAGE_HID,      0, usb_string_hid},
#elif defined(AIR_USB_HID_ENABLE)
    {USB_STRING_USAGE_HID,      0, usb_string_audio1},
#endif
#else
    /* if no any USB class assigned, use ACM. e.g. bootrom */
    {USB_STRING_USAGE_ACM_COMM, 0, usb_string_acm_comm_interface},
    {USB_STRING_USAGE_ACM_DATA, 0, usb_string_acm_data_interface},
#endif

#ifdef AIR_MS_TEAMS_ENABLE
    {USB_STRING_USAGE_MSTEAMS_UCQ, TEAMS_UCQ_STRING_WVALUE, USB_TEAMS_UCQ_STRING},
#endif

#ifdef AIR_USB_XBOX_ENABLE
    {USB_STRING_USAGE_XBOX, XBOX_MS_OS_STRIG_WVALUE, USB_XBOX_MS_OS_STRING},
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    {USB_STRING_USAGE_MOUSE,    0, usb_string_ms},
    {USB_STRING_USAGE_KEYBOARD, 0, usb_string_kb},
#endif
};

/************************************************************
    USB device infos
*************************************************************/

static usb_custom_class_info_t custom_class_info = {
    .bcdUSB    = 0x0200,
    .class     = 0x00,
    .sub_class = 0x00,
    .protocol  = 0x00
};

#define CUSTOM_VID 0x0E8D

#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE

/* Dongle */
#ifdef AIR_USB_AUDIO_MULTI_CH_MODE /* DCHS */
#define CUSTOM_PID 0x080E
#elif AIR_WIRELESS_MIC_ENABLE /* Wireless MIC */
#define CUSTOM_PID 0x080C
#elif AIR_BLE_ULTRA_LOW_LATENCY_ENABLE /* ULL2.0 */
#define CUSTOM_PID 0x080A
#else
#define CUSTOM_PID 0x0808
#endif

/* Headset */
#else
#ifdef AIR_DCHS_MODE_ENABLE /* DCHS */
#define CUSTOM_PID 0x080F
#elif AIR_WIRELESS_MIC_ENABLE /* Wireless MIC */
#define CUSTOM_PID 0x080D
#elif AIR_BLE_ULTRA_LOW_LATENCY_ENABLE /* ULL2.0 */
#define CUSTOM_PID 0x080B
#else
#define CUSTOM_PID 0x0809
#endif
#endif

#elif defined(AIR_USB_XBOX_ENABLE)
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
#define CUSTOM_PID 0x0003
#else
#define CUSTOM_PID 0x0809
#endif

#elif defined(AIR_USB_CDC_ENABLE)
#define CUSTOM_PID 0x0023

#elif defined(AIR_USB_MSC_ENABLE)
#define CUSTOM_PID 0x0002

#else
#define CUSTOM_PID 0x0003
#endif

static usb_custom_product_info_t custom_product_info = {
    .vender_id   = CUSTOM_VID,
    .product_id  = CUSTOM_PID,
    .bcd_version = 0x0100,
};

#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
#define CUSTOM_SELF_POWER false
#else
#define CUSTOM_SELF_POWER true
#endif
#else
#define CUSTOM_SELF_POWER false
#endif

#if defined(AIR_USB_AUDIO_ENABLE) || defined(AIR_USB_HID_ENABLE)
#define CUSTOM_MAX_POWER 100
#else
#define CUSTOM_MAX_POWER 500
#endif

static usb_custom_power_info_t custom_power_info = {
    .self_power    = CUSTOM_SELF_POWER,
    .remote_wakeup = false,
    .maxpower      = CUSTOM_MAX_POWER,
};

/************************************************************
    Customization functinos
*************************************************************/
#ifdef AIR_USB_HID_ENABLE
extern void uid_code_get(uint8_t *p_data);

#define TO_HEX(DIGIT) ((DIGIT) <= 9 ? '0' + (DIGIT) : 'A' - 10 + (DIGIT))

void Set_SerialNumber_From_ChipUID(char *serial_number_string)
{
    uint8_t i;
    uint8_t chip_uid[16];

    /* Get Chip UID (16 bytes) */
    uid_code_get(&chip_uid[0]);

    /**
     * Convert data format (uint8_t) to string type (uint16_t)
     * Only use 8 bytes of CPU_ID convert to 16 byte SerialNum.
     * Avoid overlong SerialNum. And SerialNum is still unique.
     */
    for (i = 0; i < 8; i++) {
        serial_number_string[i * 2 + 0] = TO_HEX(chip_uid[i] >> 4);
        serial_number_string[i * 2 + 1] = TO_HEX(chip_uid[i] & 0x0F);
    }
    serial_number_string[i * 2] = '\0';
}
#endif

/* Get device VID */
uint16_t USB_GetDeviceVID(void)
{
    return gUsbDevice.devdscr.idVendor;
}

/* Get device PID */
uint16_t USB_GetDevicePID(void)
{
    return gUsbDevice.devdscr.idProduct;
}

const usb_custom_class_info_t *USB_Custom_Get_Class_Info(void)
{
    return &custom_class_info;
}

const usb_custom_product_info_t *USB_Custom_Get_Product_Info(void)
{
    return &custom_product_info;
}

const usb_custom_power_info_t *USB_Custom_Get_Power_Info(void)
{
    return &custom_power_info;
}

const usb_custom_string_t *USB_Custom_Get_String_List(uint8_t *string_num)
{
    if (string_num == NULL) {
        assert(0);
        return NULL;
    }

    *string_num = sizeof(custom_string_list) / sizeof(usb_custom_string_t);
    /**
     * Unmark commont of below line to generate serial number by chip UID.
     */
    /* Set_SerialNumber_From_ChipUID(usb_serial_number); */

    /**
     * If any string need to run time modify, add the function or code in below
     * section.
     */

    return custom_string_list;
}

/* Get/Set USB Logging Mode*/
bool b_usb_logging_mode = false;
void USB_Set_LoggingMode(bool mode)
{
    b_usb_logging_mode = mode;
}

bool USB_Get_LoggingMode(void)
{
    return b_usb_logging_mode;
}

/************************************************************
    APP layer Customization functinos
*************************************************************/
void usb_custom_set_class_info(uint16_t bcdUSB, uint8_t class, uint8_t sub_class, uint8_t protocol)
{
    custom_class_info.bcdUSB    = bcdUSB;
    custom_class_info.class     = class;
    custom_class_info.sub_class = sub_class;
    custom_class_info.protocol  = protocol;
}

void usb_custom_set_product_info(uint16_t vender_id, uint16_t product_id, uint16_t bcd_version)
{
    custom_product_info.vender_id   = vender_id;
    custom_product_info.product_id  = product_id;
    custom_product_info.bcd_version = bcd_version;
}

void usb_custom_set_power_info(bool self_power, bool remote_wakeup, uint16_t maxpower)
{
    custom_power_info.self_power    = self_power;
    custom_power_info.remote_wakeup = remote_wakeup;
    custom_power_info.maxpower      = maxpower;
}

void usb_custom_set_string(usb_string_usage_t usage, const char *s)
{
    uint8_t length = sizeof(custom_string_list) / sizeof(usb_custom_string_t);
    usb_custom_string_t *usbstr;
    uint8_t idx;
    for (idx = 0; idx < length; idx++) {
        usbstr = &(custom_string_list[idx]);
        if (usbstr->usage == usage) {
            usbstr->str = s;
            return;
        }
    }
    return;
}

void usb_custom_set_speed(bool hs_fs)
{
    hal_usb_set_speed(hs_fs);
    return;
}

/************************************************************
    USB mass storage parameters
*************************************************************/
/**
 * Below section is dummy code.
 * It's not used currently, only for reference.
 */

/**
 * Inquire data explanation
 * The length byte(Byte 4) should be always not changed, the mass storage spec define it
 * Byte 8 to 15 is Vendor Information
 * Byte 16 to 31 is Product Identification
 * Byte 32 to 35 is Product Revision Level
 */
ATTR_RWDATA_IN_NONCACHED_RAM uint8_t INQUIRE_DATA[] = {
    0x00,
    0x80,
    0x00,
    0x01,
    0x1f, /*length*/
    0x00,
    0x00,
    0x00,
    'M',  /*Vendor Identification*/
    'E',
    'D',
    'I',
    'A',
    'T',
    'E',
    'K',
    ' ', /*Product Identification*/
    'F',
    'L',
    'A',
    'S',
    'H',
    ' ',
    'D',
    'I',
    'S',
    'K',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ',
    ' ', /*Product Revision Level*/
    ' ',
    ' ',
    ' '
};

static const uint16_t  USB_MS_INTERFACE_STRING[] = {
    0x031c,
    'M', 'a', 's', 's', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e', ' '
};

static const USB_MS_PARAM usb_ms_param = {
    0x0002,              /* Mass storage Product ID */
    INQUIRE_DATA,
    sizeof(INQUIRE_DATA) / sizeof(uint8_t),
    USB_MS_INTERFACE_STRING,
    sizeof(USB_MS_INTERFACE_STRING) / sizeof(uint16_t)
};

/* Get mass storage USB device parameter function */
const USB_MS_PARAM *USB_GetMsParam(void)
{
    return (&usb_ms_param);
}

#endif /* AIR_USB_ENABLE */
