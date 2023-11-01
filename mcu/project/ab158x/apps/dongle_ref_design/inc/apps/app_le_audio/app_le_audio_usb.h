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

#ifndef __APP_LE_AUDIO_USB_H__
#define __APP_LE_AUDIO_USB_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "bt_type.h"
#include "ui_shell_manager.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_USB_PORT_SPK_0 0   /* Chat SPK */
#define APP_LE_AUDIO_USB_PORT_SPK_1 1   /* Gaming */
#define APP_LE_AUDIO_USB_PORT_MIC_0 2   /* Chat MIC */
#define APP_LE_AUDIO_USB_PORT_MAX   3
typedef uint32_t app_le_audio_usb_port_t;

#define APP_LE_AUDIO_USB_PORT_MASK_SPK_0    0x01
#define APP_LE_AUDIO_USB_PORT_MASK_SPK_1    0x02
#define APP_LE_AUDIO_USB_PORT_MASK_MIC_0    0x04
typedef uint8_t app_le_audio_usb_port_mask_t;

typedef struct {
    uint32_t usb_sample_rate;               /**< USB sample rate in use. E.g. 16000, 32000, 44100, 48000, ... */
    uint8_t usb_sample_size;                /**< USB sample size in use. E.g. 1, 2, 3, 4 */
    uint8_t usb_channel;                    /**< USB channel in use. E.g. 1, 2 */
} app_le_audio_usb_config_info_t;

/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_usb_init(void);
bool app_le_audio_idle_usb_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len);
bool app_le_audio_usb_is_port_ready(app_le_audio_usb_port_t port, app_le_audio_usb_config_info_t *p_usb_config_info);
uint32_t app_le_audio_usb_convert_sample_size(uint8_t sample_size);
app_le_audio_usb_port_mask_t app_le_audio_usb_get_streaming_port(void);    /* bit_0:SPK_0, bit_1:SPK_1, bit_2:MIC_0 */
void app_le_audio_usb_refresh_volume(void);
bt_status_t app_le_audio_usb_get_volume(app_le_audio_usb_port_t usb_port, uint8_t *volume, uint8_t *mute);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_USB_H__ */

