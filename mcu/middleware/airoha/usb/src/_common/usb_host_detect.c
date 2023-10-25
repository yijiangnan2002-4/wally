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

#ifdef AIR_USB_ENABLE
#ifdef USB_HOST_DETECT_ENABLE

/* USB Middleware includes */
#include "usb_host_detect.h"

/* Hal includes */
#include "hal_gpt.h"

/* Syslog create module for usb_detect_host.c */
#include "usb_dbg.h"
log_create_module_variant(USB_HostDetect, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

static void usb_reboot_by_host_type(void);

typedef struct {
    int32_t getten_len_langsup;
    int32_t getten_len_string;
    int32_t getten_len_config;
    uint8_t times;
} usb_detect_host_recoder_t;

static usb_detect_host_recoder_t g_recoder = {
    .getten_len_langsup = 0,
    .getten_len_string  = 0,
    .getten_len_config  = 0,
    .times = 0,
};

#define REQUEST_DETECT_TIMES            20

#define GETTEN_LENGTH_LANGSUP_WINDOWS   0x04
#define GETTEN_LENGTH_LANGSUP_ANDROID   0xFF
#define GETTEN_LENGTH_LANGSUP_MAC       0x00

#define GETTEN_LENGTH_STRING_WINDOWS    0xFF
#define GETTEN_LENGTH_STRING_ANDROID    0xFF
#define GETTEN_LENGTH_STRING_MAC        0x02

#define GETTEN_LENGTH_CONFIG_WINDOWS    0x40
#define GETTEN_LENGTH_CONFIG_ANDROID    0x09
#define GETTEN_LENGTH_CONFIG_MAC        0x09

void USB_HostDetect_ResetRecorder()
{
    g_recoder.getten_len_langsup = 0;
    g_recoder.getten_len_string = 0;
    g_recoder.getten_len_config = 0;
    g_recoder.times = 0;
}

/**
 * @brief Record the key info of USB stdcmd from HOST.
 *
 * Record key info of USB stdcmd to identify spicial HOST.
 *
 * Key info would be stored in static var 'g_recoder'.
 *
 * @param pep0state Endpoint 0 handle, use to get request response.
 * @param pcmd Command from host.
 */
void USB_HostDetect_RecordStdcmd(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    uint8_t bRequest = pcmd->bRequest;
    uint16_t wValue = pcmd->wValue;
    uint16_t wlength = pcmd->wLength;
    bool usbnvkey_host_detect = true;
    bool usbnvkey_audio_support_huawei = true;

    if (g_recoder.times == REQUEST_DETECT_TIMES) {
        return;
    }

    /**
     * GetString Request
     */
    if (bRequest == USB_GET_DESCRIPTOR && (wValue & USB_CMD_DESCMASK) == USB_CMD_STRING) {
        LOG_MSGID_I(USB_HostDetect, "CMD_STRING, id=%d, len=%d", 2, (wValue & 0xFF), wlength);
        /**
         * ID 0 : Lang Supported
         * Windows get 4 bytes
         * Android get 255 bytes, and than get 254 bytes
         * MAC     never get
         */
        if (!g_recoder.getten_len_langsup && (wValue & 0xFF) == 0x00) {
            g_recoder.getten_len_langsup = wlength;
        }

        /**
         * ID 1 : Airoha Aongle Chat
         * Host will get first N bytes, to recognize the length of string.
         * Windows get 255 bytes
         * Android get 255 bytes
         * MAC     get 2 bytes
         */
        if (!g_recoder.getten_len_string && (wValue & 0xFF) == 0x01) {
            // only record when wlength not match
            if (wlength != ((uint8_t *)pep0state->pData)[0]) {
                g_recoder.getten_len_string = wlength;
            }
        }
    }

    /**
     * GetConfiguration Request
     */
    if (bRequest == USB_GET_DESCRIPTOR && (wValue & USB_CMD_DESCMASK) == USB_CMD_CONFIG) {
        LOG_MSGID_I(USB_HostDetect, "CMD_CONFIG, len=%d", 1, wlength);
        /**
         * Host will get first N bytes, to recognize the length of string.
         * Windows get 64(x40) bytes, and than get 255 bytes.
         * AndroId get  9(x09) bytes, and than get exact length of Configuration.
         * MAC     get  9(x09) bytes, and than get exact length of Configuration.
         */
        if (!g_recoder.getten_len_config) {
            g_recoder.getten_len_config = wlength;
        }
    }

    g_recoder.times++;
    if (g_recoder.times == REQUEST_DETECT_TIMES) {
        if (usbnvkey_host_detect && Get_USB_Host_Type() == USB_HOST_TYPE_UNKNOWN) {
            USB_HostDetect_Recohnize();
        }

        if (usbnvkey_audio_support_huawei) {
            usb_reboot_by_host_type();
        }
    }

    // LOG_MSGID_I(USB_HostDetect, "bRequest %04X, wValue %04X, wlength %04X", 3, bRequest, wValue, wlength);
}

/**
 * @brief Recohnize the HOST type by prevoius records, and set HOST type.
 *
 * Match the key info from static var 'g_recoder' is equal to spicial HOST
 * or not. And set the USB HOST type.
 *
 * Current support HOST type recohnized.
 *  1. Android smart phones (HarmonyOS)
 *  2. Windows
 *  3. MAC
 */
void USB_HostDetect_Recohnize(void)
{
    LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, getten_len_langsup = %d", 1, g_recoder.getten_len_langsup);
    LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, getten_len_string  = %d", 1, g_recoder.getten_len_string);
    LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, getten_len_config  = %d", 1, g_recoder.getten_len_config);

    if (
        g_recoder.getten_len_langsup == GETTEN_LENGTH_LANGSUP_ANDROID &&
        g_recoder.getten_len_string == GETTEN_LENGTH_STRING_ANDROID &&
        g_recoder.getten_len_config == GETTEN_LENGTH_CONFIG_ANDROID
    ) {
        LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, USB_HOST_TYPE_ANDROID", 0);
        Set_USB_Host_Type(USB_HOST_TYPE_ANDROID);
    } else if (
        g_recoder.getten_len_langsup == GETTEN_LENGTH_LANGSUP_WINDOWS &&
        g_recoder.getten_len_string == GETTEN_LENGTH_STRING_WINDOWS &&
        g_recoder.getten_len_config == GETTEN_LENGTH_CONFIG_WINDOWS
    ) {
        LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, USB_HOST_TYPE_WINDOWS", 0);
        Set_USB_Host_Type(USB_HOST_TYPE_WINDOWS);
    } else if (
        g_recoder.getten_len_langsup == GETTEN_LENGTH_LANGSUP_MAC &&
        g_recoder.getten_len_string == GETTEN_LENGTH_STRING_MAC &&
        g_recoder.getten_len_config == GETTEN_LENGTH_CONFIG_MAC
    ) {
        LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, USB_HOST_TYPE_MAC", 0);
        Set_USB_Host_Type(USB_HOST_TYPE_MAC);
    } else {
        LOG_MSGID_I(USB_HostDetect, "USB_DH_Recohnize, USB_HOST_TYPE_UNKNOWN", 0);
        Set_USB_Host_Type(USB_HOST_TYPE_UNKNOWN);
    }
}

static void usb_reboot_by_host_type(void)
{
#ifdef FREERTOS_ENABLE
    USB_HOST_TYPE host_type = Get_USB_Host_Type();
    usb_dev_type_t ap_type;

    bool usbnvkey_audio_support_huawei = true;

    /**
     * If current is audio device and enable huawei support.
     * And than if the host is Android, switch to USB_AUDIO_SMARTPHONE device.
     */
    if (gUsbDevice.device_type == USB_AUDIO && usbnvkey_audio_support_huawei) {
        if (host_type == USB_HOST_TYPE_ANDROID) {
            LOG_MSGID_I(USB_MAIN, "usb_reboot_by_host_type USB_HOST_TYPE_ANDROID", 0);

            ap_type = USB_AUDIO_SMARTPHONE;
            USB_Send_Message(USB_AP_REBOOT, (void *)ap_type);
        }
    }
#endif /* FREERTOS_ENABLE */
}

#endif /* USB_HOST_DETECT_ENABLE */
#endif /* AIR_USB_ENABLE */
