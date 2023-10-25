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

#ifndef __APP_LE_AUDIO_TRANSMITTER_H__
#define __APP_LE_AUDIO_TRANSMITTER_H__

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_type.h"
#include "ui_shell_manager.h"
#include "audio_transmitter_control.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
/* Audio transmitter volume information */
#define APP_LE_AUDIO_VOL_LEVEL_MIN      0
#define APP_LE_AUDIO_VOL_LEVEL_MAX      15
#define APP_LE_AUDIO_VOL_LEVEL_DEFAULT  12

#define APP_LE_AUDIO_TRANSMITTER_STATE_IDLE         0x00
#define APP_LE_AUDIO_TRANSMITTER_STATE_INIT         0x01
#define APP_LE_AUDIO_TRANSMITTER_STATE_STOP         APP_LE_AUDIO_TRANSMITTER_STATE_INIT
#define APP_LE_AUDIO_TRANSMITTER_STATE_STARTING     0x02
#define APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL      0x03
#define APP_LE_AUDIO_TRANSMITTER_STATE_STARTED      APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL
#define APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT       0x04
typedef uint8_t app_le_audio_transmitter_state_t;

/* Audio transmitter channel */
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_L          0x01
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_R          0x02
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_DUAL       0x03    /* (APP_LE_AUDIO_TRANSMITTER_CHANNEL_L | APP_LE_AUDIO_TRANSMITTER_CHANNEL_R) */

typedef enum {
    APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START = 0x01,
    APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG = 0x02,
    APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP = 0x04,
    APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_MAX = 0xFF
} app_le_audio_transmitter_wait_cnf_enum;

enum {
    APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_SUCCESS_CNF,
    APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_FAIL_CNF,
    APP_LE_AUDIO_EVENT_AUDIO_STREAM_STOP_CNF,
    APP_LE_AUDIO_EVENT_AUDIO_STREAM_CONFIG_CNF,
};

/* Audio stream releated */
enum{
    APP_LE_AUDIO_STREAM_PORT_SPK_0 = 0,
    APP_LE_AUDIO_STREAM_PORT_SPK_1,
    APP_LE_AUDIO_STREAM_PORT_MIC_0,
    APP_LE_AUDIO_STREAM_PORT_LINE_IN,
    APP_LE_AUDIO_STREAM_PORT_I2S_IN,
    APP_LE_AUDIO_STREAM_PORT_MAX,
};
typedef uint32_t app_le_audio_stream_port_t;

#define APP_LE_AUDIO_STREAM_PORT_MASK_SPK_0     (0x01<<APP_LE_AUDIO_STREAM_PORT_SPK_0)
#define APP_LE_AUDIO_STREAM_PORT_MASK_SPK_1     (0x01<<APP_LE_AUDIO_STREAM_PORT_SPK_1)
#define APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0     (0x01<<APP_LE_AUDIO_STREAM_PORT_MIC_0)
#define APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN   (0x01<<APP_LE_AUDIO_STREAM_PORT_LINE_IN)
#define APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN    (0x01<<APP_LE_AUDIO_STREAM_PORT_I2S_IN)
typedef uint8_t app_le_audio_stream_port_mask_t;

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef struct {
    audio_transmitter_id_t transmitter_id;                      /**< Audio transmitter ID */
    app_le_audio_transmitter_state_t curr_transmitter_state;    /**< Current audio transmitter state */
    app_le_audio_transmitter_state_t next_transmitter_state;    /**< Next audio transmitter state */
    uint8_t is_mixed;                                           /**< Audio stream is mixed or not, for SPK_0 and SPK_1 only */
    uint32_t config_op_queue;
    uint8_t mute;                           /**< mute state: MUTE or UNMUTE */
    uint8_t vol_level_left;                 /**< volume level L */
    uint8_t vol_level_right;                /**< volume level R */
    uint8_t mic_channel;                    /**< mic channel mode */
    app_le_audio_transmitter_wait_cnf_enum wait_cnf;
    uint32_t usb_sample_rate;               /**< USB sample rate in use. E.g. 16000, 32000, 44100, 48000, ... */
    uint8_t usb_sample_size;                /**< USB sample size in use. E.g. 1, 2, 3, 4 */
    uint8_t usb_channel;                    /**< USB channel in use. E.g. 1, 2 */
} app_le_audio_stream_info_t;

/**************************************************************************************************
* Public function
**************************************************************************************************/
bt_status_t app_le_audio_mute_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_unmute_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_set_audio_transmitter_volume_level(app_le_audio_stream_port_t port, uint8_t vol_level_left, uint8_t vol_level_right);

bt_status_t app_le_audio_set_audio_transmitter_mic_channel(uint32_t channel);

bt_status_t app_le_audio_init_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_start_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_stop_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_stop_and_deinit_audio_transmitter(app_le_audio_stream_port_t port);

bt_status_t app_le_audio_open_audio_transmitter(bool bidirectional, app_le_audio_stream_port_mask_t streaming_port);

bt_status_t app_le_audio_close_audio_transmitter(void);

bool app_le_audio_handle_idle_transmitter_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len);

uint32_t app_le_audio_get_usb_sample_rate_in_use(app_le_audio_stream_port_t port);

uint8_t app_le_audio_get_usb_sample_size_in_use(app_le_audio_stream_port_t port);

uint8_t app_le_audio_get_usb_channel_in_use(app_le_audio_stream_port_t port);


#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_TRANSMITTER_H__ */

