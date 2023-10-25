/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_LE_AUDIO_SILENCE_DETECTION_H__
#define __APP_LE_AUDIO_SILENCE_DETECTION_H__

#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_SILENCE_DETECTION_ENABLE

#include "bt_type.h"
#include "app_le_audio_transmitter.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
/* Enum of detecting status of both port and device */
typedef enum
{
    APP_LE_AUDIO_SILENCE_DETECTION_STATUS_IDLE = 0,

    APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_SILENCE,    /* Data detected */
    APP_LE_AUDIO_SILENCE_DETECTION_STATUS_DETECTING_DATA,   /* Silence detected */

    APP_LE_AUDIO_SILENCE_DETECTION_STATUS_MAX = 0xFF
} app_le_audio_silence_detection_status_enum;


typedef enum
{
    APP_LE_AUDIO_SILENCE_DETECTION_MODE_NONE = 0,

    APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL,     /* Silence detection with at least one CIS connected */
    APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL,    /* Silence detection with All CISes disconnected */

    APP_LE_AUDIO_SILENCE_DETECTION_MODE_MAX = 0xFF
} app_le_audio_silence_detection_mode_enum;


typedef enum
{
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_OTHER_MODE, /* Start non-special silence detection mode */
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_STOP_ANY_MODE,
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_START_SPECIAL_SILENCE_DETECTION,
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_SPECIAL_SILENCE_DETECTION_STOPPED,
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_REMOTE_DEVICE_BREDR_STATUS_UPDATE,
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_DISCONNECT_CIS_FOR_SILENCE_UPDATE,
    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_PORT_DISABLED,

    APP_LE_AUDIO_SILENCE_DETECTION_EVENT_MAX = 0xFF
} app_le_audio_silence_detection_event_enum;

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    /* Silence detection status of device. The data is for lazy update use. Use app_le_audio_silence_detection_get_status() to get real status. */
    app_le_audio_silence_detection_status_enum silence_detection_status;
    TimerHandle_t delay_stop_timer_handle;  /* Delay stopping non-special detection mode when silence is detected. */
    uint32_t delay_stop_timer_period;    /* The timer period of delay stop timer. */
} app_le_audio_silence_detection_struct;

/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_silence_detection_start_by_port(app_le_audio_stream_port_t port);

void app_le_audio_silence_detection_stop_by_port(app_le_audio_stream_port_t port);

bool app_le_audio_silence_detection_is_speical_silence_detection_ongoing(void);

app_le_audio_silence_detection_mode_enum app_le_audio_silence_detection_get_silence_detection_mode(void);

void app_le_audio_silence_detection_stop_delay_stop_timer(void);

void app_le_audio_silence_detection_init(void);

void app_le_audio_silence_detection_handle_bt_off(void);

void app_le_audio_silence_detection_handle_event(app_le_audio_silence_detection_event_enum event_id, void *parameter);

void app_le_audio_silence_detection_set_status(app_le_audio_silence_detection_status_enum status);
app_le_audio_silence_detection_status_enum app_le_audio_silence_detection_get_status(void);

bt_status_t app_le_audio_ucst_set_remote_device_bredr_connection_status(bool bredr_connected, bt_handle_t handle);

bt_status_t app_le_audio_ucst_get_remote_device_bredr_connection_status(bool *bredr_connected, bt_handle_t handle);

bt_status_t app_le_audio_ucst_set_disconnect_cis_for_silence(bool disconnect_cis, bt_handle_t handle);

bt_status_t app_le_audio_ucst_get_disconnect_cis_for_silence(bool *disconnect_cis, bt_handle_t handle);

bool app_le_audio_silence_detection_disconnect_cis_for_silence(void);

#endif


#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_SILENCE_DETECTION_H__ */

