/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_SINK_SRV_A2DP_H__
#define __BT_SINK_SRV_A2DP_H__

#include "bt_connection_manager_internal.h"
#include "bt_sink_srv_ami.h"
#include "bt_a2dp.h"
#include "bt_sink_srv_music.h"
#include "bt_sink_srv.h"
#include "hal_feature_config.h"
#include "bt_sink_srv_common.h"


#if defined(__AFE_HS_DC_CALIBRATION__)
#define BT_SINK_SRV_A2DP_OUTPUT_DEVICE          (HAL_AUDIO_DEVICE_HANDSET)
#else
#define BT_SINK_SRV_A2DP_OUTPUT_DEVICE          (HAL_AUDIO_DEVICE_HEADSET)
#endif

#define BT_SINK_SRV_A2DP_MAX_VOL_LEV            bt_sink_srv_ami_get_a2dp_max_volume_level()
#define BT_SINK_SRV_A2DP_MIN_VOL_LEV            (AUD_VOL_OUT_LEVEL0)

#define BT_SINK_SRV_A2DP_SPECIAL_DEVICE_MTU_SIZE    (672)
#define BT_SINK_SRV_A2DP_NORMAL_MTU_SIZE            (895)

#define BT_SINK_SRV_A2DP_PSEUDO_COUNT           (BT_A2DP_TOTAL_LINK_NUM)
#define BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT       (BT_AWS_MCE_TOTAL_CONNECTION_NUM)
#define BT_SINK_SRV_A2DP_PSEUDO_FLAG_USEED      (1 << 0)

#define BT_SINK_SRV_A2DP_MAGIC_CODE             (0xa000)
#define BT_SINK_SRV_A2DP_INVALID_VOLUME         (0xff)

#define BT_SINK_SRV_A2DP_MIN_SNIFF_DUR          (1000)

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#define BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW (0x00000001)
#define BT_SINK_SRV_A2DP_INT_USER_CONF_PENDING (0x00000002)
#define BT_SINK_SRV_A2DP_INT_USER_CONF_DISALLOW (0x00000004)
#define BT_SINK_SRV_A2DP_INT_USER_CONF_BYPASS (0x00000008)
#endif

typedef enum {
    VOLUME_UP     = 0,
    VOLUME_DOWN   = 1,
    VOLUME_VALUE  = 2,

    VOLUME_TOTAL
} volume_change_type_t;

typedef struct {
    uint8_t flag;
    audio_src_srv_handle_t *hd;
} bt_sink_srv_a2dp_pseudo_handle_t;

#define BT_SINK_SRV_A2DP_MPEG_2_AND_4_LC   (0x00)
#define BT_SINK_SRV_A2DP_MPEG_2_LC         (0x01)
typedef uint8_t bt_sink_srv_a2dp_aac_object_type_t;

typedef struct {
    uint8_t status;
    uint8_t enable_3M;
    uint32_t latency;
    bt_a2dp_codec_type_t codec_type;
    uint8_t min_bit_pool;
    uint8_t max_bit_pool;
    uint32_t bit_rate;
} PACKED bt_sink_srv_a2dp_basic_config_t;

typedef struct {
    uint8_t status;
    uint8_t lossless_enable;
    uint16_t codec_id;
    uint32_t vendor_id;
    uint32_t reserved_1;
    uint32_t reserved_2;
    uint32_t reserved_3;
} PACKED bt_sink_srv_a2dp_basic_config_2_t;

typedef struct {
    uint8_t name_len;
    char name[7];
} bt_sink_srv_a2dp_codec_name_t;

#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
typedef struct {
    bool changed_flag;
    uint16_t sample_frequency;
    uint8_t frame_duration;
} bt_sink_srv_a2dp_lc3plus_change_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif
bt_status_t  bt_sink_srv_a2dp_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data);
#ifdef MTK_BT_SPEAKER_ENABLE
bt_status_t bt_sink_srv_a2dp_mode_changed_handle_callback(bt_cm_event_t event_id, void *params, uint32_t params_len);

void bt_sink_srv_a2dp_send_play_ind_timer(uint32_t timer_id, uint32_t data);
#endif
void bt_sink_srv_a2dp_ami_hdr(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *user);

bt_status_t bt_sink_srv_a2dp_action_handler(bt_sink_srv_action_t action, void *param);

bt_status_t bt_sink_srv_a2dp_get_init_params(bt_a2dp_init_params_t *param);

bt_status_t bt_sink_srv_a2dp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

int32_t bt_sink_srv_a2dp_change_volume(uint8_t type, uint8_t sync, uint32_t volume_value, bt_sink_srv_music_device_t *dev);

void bt_sink_srv_a2dp_play(audio_src_srv_handle_t *handle);

void bt_sink_srv_a2dp_stop(audio_src_srv_handle_t *handle);

void bt_sink_srv_a2dp_suspend(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);

void bt_sink_srv_a2dp_reject(audio_src_srv_handle_t *handle);

void bt_sink_srv_a2dp_create_pse_handle(void);

void bt_sink_srv_a2dp_destroy_pse_handle(void);

void bt_sink_srv_a2dp_drv_play(void *param);

void bt_sink_srv_music_play_handle(audio_src_srv_handle_t *handle);

void bt_sink_srv_music_stop_handle(audio_src_srv_handle_t *handle);

void bt_sink_srv_music_suspend_handle(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);

void bt_sink_srv_music_reject_handle(audio_src_srv_handle_t *handle);

void bt_sink_srv_music_exception_handle(audio_src_srv_handle_t *handle, int32_t event, void *param);

bt_status_t bt_sink_srv_music_a2dp_action_handler(bt_sink_srv_action_t action, void *param);

audio_src_srv_handle_t *bt_sink_srv_a2dp_alloc_pseudo_handle(void);

void bt_sink_srv_a2dp_free_pseudo_handle(audio_src_srv_handle_t *hd);
void bt_sink_srv_a2dp_enable_aac(bool enable_flag);
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
void bt_sink_srv_a2dp_enable_vendor_codec(bool enable_flag);
bool bt_sink_srv_a2dp_get_vendor_codec_config(void);
#endif
bt_status_t bt_sink_srv_a2dp_reinitial_sync();
void bt_sink_srv_a2dp_initial_avrcp_timer(uint32_t timer_id, uint32_t data);
bt_status_t bt_sink_srv_a2dp_get_volume(bt_bd_addr_t *bd_addr, uint32_t *volume);
void bt_sink_srv_a2dp_update_base_timer(uint32_t timer_id, uint32_t data);

bt_status_t bt_sink_srv_a2dp_get_codec_parameters(bt_sink_srv_a2dp_basic_config_t *config_data);
bt_status_t bt_sink_srv_a2dp_get_codec_parameters_ext(bt_sink_srv_a2dp_basic_config_2_t *config_data);
bt_status_t bt_sink_srv_a2dp_get_codec_type(bt_sink_srv_a2dp_codec_name_t *codec_data);
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
void bt_sink_srv_a2dp_change_lc3plus_param(uint16_t sample_frequency, uint8_t frame_duration);
#endif
#ifdef MTK_AUDIO_SYNC_ENABLE
void bt_sink_srv_music_volume_sync_callback(bt_sink_srv_sync_status_t status, bt_sink_srv_sync_callback_data_t *sync_data);
#endif
void bt_sink_srv_a2dp_disconnect_avrcp_timer(uint32_t timer_id, uint32_t data);
void bt_sink_srv_a2dp_add_waitinglist(audio_src_srv_handle_t *handle);
int32_t bt_sink_srv_a2dp_set_volume(uint8_t volume, bt_sink_srv_music_device_t *sp_dev);

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
bt_status_t bt_sink_srv_a2dp_int_user_conf_set(uint32_t val, bt_bd_addr_ptr_t addr);
#endif
void bt_sink_srv_a2dp_set_no_retransmission_mode(uint16_t seq_num, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif
