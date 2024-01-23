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

#ifndef __APP_DONGLE_COMMON_IDLE_ACTIVITY_H__
#define __APP_DONGLE_COMMON_IDLE_ACTIVITY_H__

#include "ui_shell_activity.h"

typedef enum {
    /* BTA Projects statrt from 0x00 */
    APPS_USB_MODE_BTA_MIN         = 0x00,
    APPS_USB_MODE_GAMING          = 0,
    APPS_USB_MODE_XBOX            = 1,
    APPS_USB_MODE_ENTERPRISE      = 2,
    APPS_USB_MODE_WIRELESS_MIC_RX = 3,
    APPS_USB_MODE_8CH             = 4,
    APPS_USB_MODE_DCHS_CUSTOM     = 6,
    APPS_USB_MODE_AUDIO_CDC       = 8,
    APPS_USB_MODE_BT_SOURCE       = 9,
    APPS_USB_MODE_GAMING_ULL2     = 10,
    APPS_USB_MODE_MSC             = 11,
    APPS_USB_MODE_BT_LEA          = 12,
    APPS_USB_MODE_CDC             = 13,
    APPS_USB_MODE_BT_ULL2         = 14,
    APPS_USB_MODE_BTA_MAX,
    /* BTD Projects statrt from 0x80 */
    APPS_USB_MODE_BTD_MIN         = 0x80,
    APPS_USB_MODE_3IN1            = 0x80,
    APPS_USB_MODE_GAMING_MSKB     = 0x81,
    APPS_USB_MODE_GAMING_MS       = 0x82,
    APPS_USB_MODE_GAMING_KB       = 0x83,
    APPS_USB_MODE_GAMING_NVMS     = 0x84,
    APPS_USB_MODE_OFFICE_MSKB     = 0x85,
    APPS_USB_MODE_BTD_MAX,
} app_usb_mode_t;

/**
* @brief      This function is the interface of the app_dongle_common_idle_activity, and is only called by ui_shell when events are sent.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_group, the current event group to be handled.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_dongle_common_idle_activity_proc(ui_shell_activity_t *self,
                                       uint32_t event_group,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len);


app_usb_mode_t app_dongle_common_idle_activity_get_current_mode(void);

void app_dongle_common_idle_activity_init_mode(void);

#endif /* __APP_DONGLE_COMMON_IDLE_ACTIVITY_H__ */