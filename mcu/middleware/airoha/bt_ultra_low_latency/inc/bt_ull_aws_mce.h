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


#ifndef __BT_ULL_AWS_MCE_H__
#define __BT_ULL_AWS_MCE_H__
#include "bt_type.h"
#include "bt_ull_utility.h"

BT_EXTERN_C_BEGIN

/* aws mce sync event, agent -> partner/client */
#define BT_ULL_AWS_MCE_EVT_CONNECTION     (0x01)
#define BT_ULL_AWS_MCE_EVT_PLAY           (0x02)
#define BT_ULL_AWS_MCE_EVT_STOP           (0x03)
#define BT_ULL_AWS_MCE_EVT_VOL_SYNC       (0x04)
/* aws mce report event, partner/client -> agent */
#define BT_ULL_AWS_MCE_REPORT_VOL_SYNC    (0x05)
#define BT_ULL_AWS_MCE_RESTART_CODEC      (0x06)

typedef uint8_t bt_ull_aws_mce_evt_t;

typedef struct {
    bool is_mute;                               /**< audio streaming is mute/unmute */
    bool is_streaming;                          /**< audio streaming is playing or not */
    bt_ull_original_duel_volume_t volume;       /**< audio volume level */
} bt_ull_aws_mce_streaming_info_t;

typedef struct {
    bt_ull_last_connect_device_t remode_device;
    /* Now we only need sync music downlink info to partner, voice uplink info sync will be sync when RHO happenned */
    /* Because there is only agent MIC can be enable during ULL connection, then partner no need to known current MIC interface state */
    bool is_ull_connected;    /** < ull spp channel is connected */
    uint16_t dl_latency;    /** <downlink latency, default: 20, unit: ms */
    uint16_t ul_latency;    /** <uplink latency, unit: ms */
    bt_ull_codec_t codec_type;
    bt_ull_aws_mce_streaming_info_t dl_streaming;
    bt_ull_aws_mce_streaming_info_t ul_streaming;
    uint8_t ul_frame_size;  /*< ull uplink mic frame size*/
} bt_ull_aws_mce_connection_t;

typedef struct {
    bt_ull_aws_mce_streaming_info_t dl_streaming;
    bt_ull_aws_mce_streaming_info_t ul_streaming;
    uint16_t dl_latency;    /** <downlink latency, default: 20, unit: ms */
    uint16_t ul_latency;    /** <uplink latency, unit: ms */
} bt_ull_aws_mce_play_t;

#if 0
typedef struct {
    bt_sink_srv_am_volume_level_out_t vol_lev;      /** < speaker audio volume level */
    bool is_streaming;                              /** < speaker music is playing or not */
    uint16_t dl_latency;                            /** < downlink latency, default: 20, unit: ms */
    bt_ull_codec_t codec_type;
} bt_ull_aws_mce_restart_codec_t;
#endif

typedef struct {
    bool dl_is_streaming;       /* speaker music is playing or not */
    bool ul_is_streaming;       /* speaker music is playing or not */
} bt_ull_aws_mce_stop_t;

typedef struct {
    bt_sink_srv_am_volume_level_out_t vol_lev;              /** < speaker audio volume level */
    bool is_mute;               /* speaker music is mute/unmute */
} bt_ull_aws_mce_vol_sync_t;

typedef struct {
    bt_ull_streaming_t type;  /**< streaming interface type. */
    bt_ull_volume_action_t vol_action;              /** < speaker audio volume level */
    uint32_t  volume_level;   /**< absolute volume level. */
} bt_ull_aws_mce_vol_report_t;

typedef struct {
    bt_ull_aws_mce_evt_t event;
    union {
        bt_ull_aws_mce_connection_t aws_connection;
        bt_ull_aws_mce_play_t aws_play;
        bt_ull_aws_mce_stop_t aws_stop;
        bt_ull_aws_mce_vol_sync_t aws_vol_sync;
        bt_ull_volume_t aws_vol_report;
        //bt_ull_aws_mce_restart_codec_t aws_restart_codec;
    } param;
} bt_ull_aws_mce_eir_info_t;

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
void bt_ull_rho_init(void);
#endif
void bt_ull_aws_mce_packet_callback(bt_aws_mce_report_info_t *param);
void bt_ull_aws_mce_send_eir(void *param, uint32_t param_len, bool urgent);
void bt_ull_aws_mce_handle_connection(bool is_ull_connected);


BT_EXTERN_C_END

#endif /* __BT_ULL_AWS_MCE_H__  */

