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

#ifndef __APP_LE_AUDIO_UTILITY_H__
#define __APP_LE_AUDIO_UTILITY_H__

#ifdef AIR_LE_AUDIO_ENABLE
#include "FreeRTOS.h"
#include "timers.h"
#include "bt_type.h"
#include "bt_gap_le_audio.h"

#include "app_le_audio_usb.h"
#include "app_le_audio_transmitter.h"
#include "app_le_audio_silence_detection.h"


/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_TIMER_LE_CONNECTION_TIME_PERIOD    (60000) /* 1min */

/* Should be more than APP_LE_AUDIO_UCST_LINK_MAX_NUM */
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
#define APP_LE_AUDIO_TIMER_MAX_NUM (4)
#else
#define APP_LE_AUDIO_TIMER_MAX_NUM (3)
#endif

typedef enum {
    APP_LE_AUDIO_EVENT_START_SCAN_NEW_DEVICE,
    APP_LE_AUDIO_EVENT_STOP_SCAN_NEW_DEVICE,
    APP_LE_AUDIO_EVENT_TIMER_EXPIRED,
    APP_LE_AUDIO_EVENT_STOP_TIMER,
    APP_LE_AUDIO_EVENT_VCP_RETRY,
    APP_LE_AUDIO_EVENT_MICP_RETRY,
    APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED,
} app_le_audio_event_t;

#define APP_LE_AUDIO_MODE_NONE      0
#define APP_LE_AUDIO_MODE_UCST      0x01
#define APP_LE_AUDIO_MODE_BCST      0x02
#define APP_LE_AUDIO_MODE_ASIT      0x03
#define APP_LE_AUDIO_MODE_DISABLE   0x04

typedef uint8_t app_le_audio_mode_t;

/* Config releated */
#define APP_LE_AUDIO_SAMPLING_FREQ_INVALID      0

/* SDU interval (ms) */
#define SDU_INTERVAL_7P5_MS                     0x00    /* 7.5 ms */
#define SDU_INTERVAL_10_MS                      0x01    /* 10 ms */
#define SDU_INTERVAL_INVALID                    0xFF
typedef uint8_t app_le_audio_sdu_interval_t;


/* Function works only when app_le_audio_usb_port_t and app_le_audio_stream_port_t are aligned. */
// TODO: phase-out app_le_audio_stream_port_t and use app_le_audio_usb_port_t only?
#define APP_LE_AUDIO_CONVERT_STREAMING_PORT(streaming_port) ((app_le_audio_usb_port_t)streaming_port)
#define APP_LE_AUDIO_CONVERT_USB_PORT(usb_port) ((app_le_audio_stream_port_t)usb_port)

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    uint8_t sampling_freq;                      /* Sampling frequency */
    uint16_t sdu_size;                          /* Maximum SDU size (octets) */
    app_le_audio_sdu_interval_t sdu_interval;   /* SDU interval */
    float bitrate;                              /* Bitrate (kbps) */
    uint8_t rtn;                                /* Retransmission number */
    uint8_t latency;                            /* Max transport latency (ms) */
} app_le_audio_qos_params_t;


typedef struct {
    app_le_audio_mode_t curr_mode;
    app_le_audio_mode_t next_mode;
    app_le_audio_stream_info_t stream_info[APP_LE_AUDIO_STREAM_PORT_MAX];
    uint8_t open_audio_transmitter;
    uint8_t bidirectional;
    uint8_t set_buffer;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    app_le_audio_silence_detection_status_enum silence_detection_status[APP_LE_AUDIO_STREAM_PORT_MAX]; /* Silence detection status of each port */
    app_le_audio_silence_detection_struct silence_detection;
#endif
} app_le_audio_ctrl_t;

typedef struct {
    uint16_t sdu_size;                          /* Maximum SDU size (octets) */
    app_le_audio_sdu_interval_t sdu_interval;   /* SDU interval */
    float bitrate;                              /* Bitrate (kbps) */
    uint8_t low_rtn;                            /* Low latency retransmission number */
    uint8_t low_latency;                        /* Low latency Max transport latency (ms) */
    uint8_t high_rtn;                           /* High reliability retransmission number */
    uint8_t high_latency;                       /* High reliability max transport latency (ms) */
} app_le_audio_qos_params_tbl_t;

typedef void (*app_le_audio_timer_callback_t)(TimerHandle_t timer_handle, void *user_data);

typedef struct {
    TimerHandle_t timer_handle;
    app_le_audio_timer_callback_t callback;
    void *user_data;
} app_le_audio_timer_info_struct;

/**************************************************************************************************
* Public function
**************************************************************************************************/


uint8_t app_le_audio_get_sample_freq(uint8_t in_smaple_rate);

uint32_t app_le_audio_convert_sample_freq(uint8_t sampling_freq);

app_le_audio_mode_t app_le_audio_get_current_mode(void);

bt_status_t app_le_audio_setup_iso_data_path(bt_handle_t handle, bt_gap_le_iso_data_path_direction_t direction, bt_gap_le_iso_data_path_id_t id);

bt_status_t app_le_audio_remove_iso_data_path(bt_handle_t handle, uint8_t direction);

bt_status_t app_le_audio_timer_start(TimerHandle_t *timer_handle, uint32_t timer_period, app_le_audio_timer_callback_t callback, void *user_data);

void app_le_audio_timer_stop(TimerHandle_t timer_handle);

void app_le_audio_timer_handle_timer_expired_event(TimerHandle_t timer_handle);

bool app_le_audio_handle_idle_le_audio_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len);

void app_le_audio_set_streaming_port_mask(app_le_audio_stream_port_mask_t mask);
void app_le_audio_clear_streaming_port_mask(app_le_audio_stream_port_mask_t mask);
uint8_t app_le_audio_get_streaming_port(void);
bt_status_t app_le_audio_start_streaming_port(app_le_audio_stream_port_t port);
bt_status_t app_le_audio_stop_streaming_port(app_le_audio_stream_port_t port);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_UTILITY_H__ */

