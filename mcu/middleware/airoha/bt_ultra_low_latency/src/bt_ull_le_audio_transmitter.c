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


#include "bt_type.h"
#include "bt_sink_srv_ami.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_audio_transmitter.h"
#include "bt_ull_service.h"
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "audio_transmitter_control.h"
#include "audio_transmitter_control_port.h"
#endif

/**************************************************************************************************
* Define
**************************************************************************************************/
#define BT_ULL_LE_AT_SELECT_1_CHANNEL           (0x01)
#define BT_ULL_LE_AT_SELECT_2_CHANNEL           (0x02)
#define BT_ULL_LE_AT_SELECT_3_CHANNEL           (0x03)
#define BT_ULL_LE_AT_SELECT_4_CHANNEL           (0x04)
#define BT_ULL_LE_AT_SELECT_5_CHANNEL           (0x05)
#define BT_ULL_LE_AT_SELECT_6_CHANNEL           (0x06)
#define BT_ULL_LE_AT_SELECT_7_CHANNEL           (0x07)
#define BT_ULL_LE_AT_SELECT_8_CHANNEL           (0x08)


/**
* @brief Defines the flag of Audio Transmitter for dongle
*/
typedef uint16_t bt_ull_le_transmitter_flag_t;
#define BT_ULL_LE_STREAMING_FLAG               (0x01)
#define BT_ULL_LE_STREAMING_MIXED              ((BT_ULL_LE_STREAMING_FLAG) << 0)//0x01
#define BT_ULL_LE_STREAMING_STOPPING           ((BT_ULL_LE_STREAMING_FLAG) << 1)//0x02
#define BT_ULL_LE_STREAMING_STARTING           ((BT_ULL_LE_STREAMING_FLAG) << 2)//0x04
#define BT_ULL_LE_STREAMING_RECONFIG           ((BT_ULL_LE_STREAMING_FLAG) << 3)//0x08    /* transmitter shuold re-config due to sample rate changed */
#define BT_ULL_LE_STREAMING_RESTART            ((BT_ULL_LE_STREAMING_FLAG) << 4)//0x10    /* transmitter shuold restart due to sample rate changed */
#define BT_ULL_LE_STREAMING_NEED_DEINIT        ((BT_ULL_LE_STREAMING_FLAG) << 5)//0x20   /* transmitter should deinit*/
#define BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE ((BT_ULL_LE_STREAMING_FLAG) << 6)//0x40    /* transmitter need start special mode*/
#define BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE  ((BT_ULL_LE_STREAMING_FLAG) << 7)//0x80
#define BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE  ((BT_ULL_LE_STREAMING_FLAG) << 8)//0x100    /* transmitter need start special mode*/
#define BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE   ((BT_ULL_LE_STREAMING_FLAG) << 9)//0x200
#define BT_ULL_LE_STREAMING_START_BLOCKED           ((BT_ULL_LE_STREAMING_FLAG) << 10)//0x400
#define BT_ULL_LE_STREAMING_REINIT_IN_SPECIAL_MODE  ((BT_ULL_LE_STREAMING_FLAG) << 11)//0x800

/**
* @brief Defines the silence detection flag for dongle
*/
typedef uint8_t bt_ull_le_at_silence_detection_flag_t;
#define BT_ULL_LE_AT_SILENCE_FLAG              (0x01)
#define BT_ULL_LE_AT_GAMING_IS_NOT_SILENCE     ((BT_ULL_LE_AT_SILENCE_FLAG) << 0)
#define BT_ULL_LE_AT_CHAT_IS_NOT_SILENCE       ((BT_ULL_LE_AT_SILENCE_FLAG) << 1)
#define BT_ULL_LE_AT_LINEIN_IS_NOT_SILENCE     ((BT_ULL_LE_AT_SILENCE_FLAG) << 2)
#define BT_ULL_LE_AT_I2SIN_IS_NOT_SILENCE      ((BT_ULL_LE_AT_SILENCE_FLAG) << 3)


/**
*@brief Defines the wireless mic out  type, only for internal use
*/
typedef uint32_t bt_ull_le_at_uplink_type;
#define BT_ULL_LE_AT_UPLINK_LINEOUT     (0x01)
#define BT_ULL_LE_AT_UPLINK_I2SOUT      (0x02)
#define BT_ULL_LE_AT_UPLINK_USBOUT      (0x03)


/**************************************************************************************************
* Structure
**************************************************************************************************/
/**
* @brief Defines the gain volume
*/
typedef struct {
    uint8_t                           vol;
    bt_sink_srv_am_volume_level_out_t level;
    bt_ull_audio_channel_t            audio_channel;
} bt_ull_le_at_volume_t;

typedef struct {
    bt_ull_le_at_volume_t             volume[3]; //0: dual channel  1: left channel   2: right channel
    bool                              is_dual_channel_set;
    bt_ull_audio_channel_t            latest_channel_set;
} bt_ull_le_at_volume_info_t;

typedef struct {
    uint8_t                  num_streaming;                         /**< The number of streaming path. */
    bt_ull_ratio_t           streamings[BT_ULL_LE_MAX_DL_STREAMING_NUM];  /**< The array of streaming path. 0:gaming 1:chat 2:linein 3: i2sin*/
} bt_ull_le_at_mix_ratio_t;

/**
* @brief Defines the wireless mic  link info
*/

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
typedef struct {
    bt_bd_addr_t client_addr;
    uint8_t      link_index;
} bt_ull_le_at_wireless_mic_client_link_info;
#endif

/**
 * @brief Defines the parameter of stream
 */
typedef struct {
    uint8_t                         out_type;
    bt_ull_le_codec_t               codec;
    bool                            is_mute;
    bool                            is_transmitter_start;
    bool                            is_request_transmitter_start;
    uint16_t                        streaming_flag;
    int8_t                          transmitter;
    uint32_t                        sample_rate;
    //bt_ull_le_at_volume_t           volume;
    bt_ull_le_at_volume_info_t      volume_info;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bool                            is_silence_detection_on;
    bool                            is_silence_detection_special_mode;
#endif
} bt_ull_le_at_info_t;


/**
 * @brief Defines the information of stream info
 */
typedef struct {
    bt_ull_le_at_mix_ratio_t       dl_mix_ratio;       /**< 2 downlink mixing ratio */
    /* streaming interface */
    // TOTO: bt_ull_le_at_info_t[BT_ULL_TRANSMITTER_MAX_NUM]
    bt_ull_le_at_info_t            dl_spk;             /**< downlink gaming streaming */
    bt_ull_le_at_info_t            dl_chat;            /**< downlink chat streaming */
    bt_ull_le_at_info_t            ul_mic;             /**< uplink streaming */
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
    bt_ull_le_at_info_t            dl_linein;          /**< line in */
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
    bt_ull_le_at_info_t            dl_i2sin;             /**< i2s in */
    i2s_in_dongle_config_t         i2sin_param;          /**< i2sin param */
#endif
    bt_ull_le_at_info_t            ul_lineout;       /**< line out */
    bt_ull_le_at_info_t            ul_i2sout;        /**<i2sout*/
#if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE))
    i2s_in_dongle_config_t         i2sout_param;     /**<i2sout param */
#endif
    bt_ull_le_at_callback          callback;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bt_ull_le_at_silence_detection_flag_t ull_le_at_silence_detection_flag;
    bt_ull_le_at_silence_detection_mode_t ull_le_at_silence_detection_mode;
#endif
#ifdef AIR_WIRELESS_MIC_RX_ENABLE
    bt_ull_le_at_wireless_mic_client_link_info wireleaa_mic_client_link_info[BT_ULL_LE_MAX_LINK_NUM];
#endif

} bt_ull_le_at_context_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static bt_ull_le_at_context_t g_ull_le_at_ctx;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
static void bt_ull_le_at_gaming_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_chat_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_mic_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_handle_spk_start_success_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_handle_mic_start_success_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_handle_mic_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_config_spk_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config);
static void bt_ull_le_at_config_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config);
static bt_status_t bt_ull_le_at_config_volume(bt_ull_transmitter_t transmitter_type, bt_ull_audio_channel_t channel);
static bt_status_t bt_ull_le_at_sync_volume_info(bt_ull_transmitter_t transmitter_type);
static uint32_t bt_ull_le_at_switch_channel_mode(bt_ull_le_channel_mode_t channel_mode);
static uint32_t bt_ull_le_at_switch_sample_size(uint32_t sample_size);
static void bt_ull_le_at_event_callback(bt_ull_le_at_event_t event, void *param, uint32_t param_len);
static void bt_ull_le_at_remove_flag(bt_ull_transmitter_t transmitter_type, bt_ull_le_transmitter_flag_t mask);
static void bt_ull_le_at_print_bt_out_param_log(ull_audio_v2_dongle_bt_out_param_t *bt_out_param, uint8_t link_num);
static void bt_ull_le_at_set_bt_out_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, ull_audio_v2_dongle_bt_out_param_t *bt_out_param);
static void bt_ull_le_at_print_bt_in_param_log(ull_audio_v2_dongle_bt_in_param_t *bt_in_param, uint8_t link_num);
static void bt_ull_le_at_set_bt_in_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, ull_audio_v2_dongle_bt_in_param_t *bt_in_param);
static bool bt_ull_le_at_type_is_uplink(bt_ull_transmitter_t transmitter_type);


#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
static void bt_ull_le_at_linein_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_handle_linein_start_success_cnf(void);
static void bt_ull_le_at_handle_linein_stop_success_or_start_fail_cnf(void);
static void bt_ull_le_at_config_linein_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config);
#endif

#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
static void bt_ull_le_at_i2sin_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_handle_i2sin_start_success_cnf(void);
static void bt_ull_le_at_handle_i2sin_stop_success_or_start_fail_cnf(void);
static void bt_ull_le_at_config_i2sin_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config);
#endif
#ifdef AIR_WIRELESS_MIC_ENABLE
static void bt_ull_le_at_wireless_mic_callback(audio_transmitter_event_t event, void *data, void *user_data);
static void bt_ull_le_at_handle_wireless_mic_start_success_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_handle_wireless_mic_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_config_wireless_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config);
static void bt_ull_le_at_wirelss_mic_print_bt_in_param_log(wireless_mic_rx_bt_in_param_t *bt_out_param, uint8_t link_num);
static void bt_ull_le_at_wireless_mic_set_bt_out_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, wireless_mic_rx_bt_in_param_t *bt_in_param, uint8_t link_num);
#endif

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
static bt_status_t bt_ull_le_at_start_success_refresh_all_bt_link_adress(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_reset_wireless_mic_client_link_info(void);
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
static void bt_ull_le_at_silence_detection_start_by_transmitter_type(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_silence_detection_stop_by_transmitter_type(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_silence_detection_callback(audio_scenario_type_t scenario_type, bool silence_flag);
//static void bt_ull_le_at_silence_detection_handler(bt_ull_transmitter_t transmitter_type, bool is_silence_detected);
static void bt_ull_le_at_silence_detection_start_normal_mode(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_silence_detection_stop_normal_mode(void);
static void bt_ull_le_at_silence_detection_start_special_mode(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_silence_detection_stop_special_mode(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_silence_detection_check_stop_all_special_mode(void);
//static bool bt_ull_le_at_silence_detection_is_spk_in_silence(void);
//static bool bt_ull_le_at_silence_detection_check_need_start_special_mode(void);
static void bt_ull_le_at_set_silence_flag(bt_ull_transmitter_t transmitter_type);
static void bt_ull_le_at_remove_silence_flag(bt_ull_transmitter_t transmitter_type);
static bt_ull_transmitter_t bt_ull_le_at_get_type_by_audio_scenario_type(audio_scenario_type_t scenario_type);
static audio_scenario_type_t bt_ull_le_at_get_audio_scenario_by_at_type(bt_ull_transmitter_t transmitter_type);
//static bt_ull_le_at_silence_detection_mode_t bt_ull_le_at_get_silence_detection_mode(void);
bt_ull_le_at_silence_detection_mode_t bt_ull_le_at_get_silence_detection_mode(void);
static void bt_ull_le_at_set_silence_detection_mode(bt_ull_le_at_silence_detection_mode_t mode);
static bool bt_ull_le_at_type_is_downlink(bt_ull_transmitter_t transmitter_type);
static bool bt_ull_le_at_silence_detection_is_uplink_streaming(void);
#endif


#endif

static void bt_ull_le_at_set_flag(bt_ull_transmitter_t transmitter_type, bt_ull_le_transmitter_flag_t mask);
static bt_ull_le_at_context_t* bt_ull_le_at_get_context(void);
static bt_ull_le_at_info_t *bt_ull_le_at_get_stream_info_by_transmitter_type(bt_ull_transmitter_t transmitter_type);

/**************************************************************************************************
* Functions
**************************************************************************************************/

static bt_ull_le_at_info_t *bt_ull_le_at_get_stream_info_by_transmitter_type(bt_ull_transmitter_t transmitter_type)
{
    //TODO: return g_ull_le_at_ctx.at_info[type];

    if (BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.dl_spk);
    } else if (BT_ULL_CHAT_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.dl_chat);
    } else if (BT_ULL_MIC_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.ul_mic);
    }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
    else if (BT_ULL_LINE_IN_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.dl_linein);
    }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
    else if (BT_ULL_I2S_IN_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.dl_i2sin);
    }
#endif
    else if (BT_ULL_LINE_OUT_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.ul_lineout);
    } else if (BT_ULL_I2S_OUT_TRANSMITTER == transmitter_type) {
        return &(g_ull_le_at_ctx.ul_i2sout);
    }
    else {
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter is invalid!! transmitter_type: 0x%x", 1, transmitter_type);
        return NULL;
    }
}

// TODO: streaming_interface interface[] = {BT_ULL_STREAMING_INTERFACE_SPEAKER, BT_ULL_STREAMING_INTERFACE_SPEAKER};
// interface[type]

void bt_ull_le_at_init_ctx(bt_ull_le_at_callback callback)
{
//TOTO: bt_ull_le_at_info_t *at_info[] = {&g_ull_le_at_ctx.dl_spk, &g_ull_le_at_ctx.dl_chat};
    memset(&g_ull_le_at_ctx, 0, sizeof(g_ull_le_at_ctx));
    ull_report("[ULL][LE] bt_ull_le_at_init_ctx",0);
    /* init dl_spk */
    g_ull_le_at_ctx.dl_spk.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.dl_spk.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.dl_spk.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_spk.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.dl_spk.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_spk.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[0].streaming.port = 0;/*gaming streaming port*/
    g_ull_le_at_ctx.dl_mix_ratio.streamings[0].ratio = BT_ULL_MIX_RATIO_MAX;

    /* init dl_chat */
    g_ull_le_at_ctx.dl_chat.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.dl_chat.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.dl_chat.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_chat.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.dl_chat.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_chat.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;

    g_ull_le_at_ctx.dl_mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[1].streaming.port = 1;/*chat streaming port*/
    g_ull_le_at_ctx.dl_mix_ratio.streamings[1].ratio = BT_ULL_MIX_RATIO_MAX;

    /* init ul_mic */
    g_ull_le_at_ctx.ul_mic.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.ul_mic.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.ul_mic.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_mic.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.ul_mic.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_mic.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;


#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
    /* init dl_linein */
    g_ull_le_at_ctx.dl_linein.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.dl_linein.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.dl_linein.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_linein.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.dl_linein.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_linein.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[2].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[2].streaming.port = 0;/*gaming streaming port*/
    g_ull_le_at_ctx.dl_mix_ratio.streamings[2].ratio = BT_ULL_MIX_RATIO_MAX;

#endif
    /* init ul_lineout */
    g_ull_le_at_ctx.ul_lineout.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.ul_lineout.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.ul_lineout.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_lineout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.ul_lineout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_lineout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;

#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
    /* init dl_i2sin */
    g_ull_le_at_ctx.dl_i2sin.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.dl_i2sin.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.dl_i2sin.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_i2sin.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.dl_i2sin.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.dl_i2sin.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;

    g_ull_le_at_ctx.dl_mix_ratio.streamings[3].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
    g_ull_le_at_ctx.dl_mix_ratio.streamings[3].streaming.port = 0;/*gaming streaming port*/
    g_ull_le_at_ctx.dl_mix_ratio.streamings[3].ratio = BT_ULL_MIX_RATIO_MAX;

#endif
    /* init ul_i2sout */
    g_ull_le_at_ctx.ul_i2sout.transmitter = AUD_ID_INVALID;
    g_ull_le_at_ctx.ul_i2sout.volume_info.is_dual_channel_set = true;
    g_ull_le_at_ctx.ul_i2sout.volume_info.latest_channel_set = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_i2sout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].level = AUD_VOL_OUT_MAX;
    g_ull_le_at_ctx.ul_i2sout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].audio_channel = BT_ULL_AUDIO_CHANNEL_DUAL;
    g_ull_le_at_ctx.ul_i2sout.volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol = 90;

    g_ull_le_at_ctx.dl_mix_ratio.num_streaming = BT_ULL_LE_MAX_DL_STREAMING_NUM;
    /* init callback*/
    g_ull_le_at_ctx.callback = callback;

#ifdef AIR_SILENCE_DETECTION_ENABLE
    g_ull_le_at_ctx.ull_le_at_silence_detection_flag = 0x00;
    g_ull_le_at_ctx.ull_le_at_silence_detection_mode = BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL;
#endif

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
    bt_ull_le_at_reset_wireless_mic_client_link_info();
#endif
}

bt_status_t bt_ull_le_at_start(bt_ull_transmitter_t transmitter_type, bool is_app_request)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_start, transmitter_id: 0x%x, transmitter_type: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x, is_app_request: %d", 5,
        p_stream_info->transmitter, transmitter_type, p_stream_info->streaming_flag, p_stream_info->is_transmitter_start, is_app_request);
    if (true == is_app_request) {
        p_stream_info->is_request_transmitter_start = true; //set flag, request transmitter start
        //BTA-37931
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
        if (stream_ctx->is_silence) {
            stream_ctx->is_silence = false;
        }
    #endif
        if ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
            || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
            return BT_STATUS_SUCCESS;
        }
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
        ull_report("[ULL][LE][SD_DEBUG][1] bt_ull_le_at_start, silence_detection_mode: %d", 1, silence_detection_mode);
        if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
            bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL);
            bt_ull_le_at_silence_detection_check_stop_all_special_mode();
            ull_report("[ULL][LE][SD_DEBUG] Before other spk exit special mode, not start normal mode!!!", 0);
            bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_START_BLOCKED);//BTA-38653
            return BT_STATUS_SUCCESS;
        }
    #endif
    }
    if (false == p_stream_info->is_transmitter_start) {
        if ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
            || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
            return BT_STATUS_SUCCESS;
        } else {
            bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);
            if ((AUD_ID_INVALID != p_stream_info->transmitter)
                && (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_start(p_stream_info->transmitter))){
                bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);
                return BT_STATUS_FAIL;
            }
            ull_report("[ULL][LE][AUDIO_TRANS] call audio transmitter start success!!", 0);
        }
    }
    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

bt_status_t bt_ull_le_at_stop(bt_ull_transmitter_t transmitter_type, bool is_app_request)
{
#if (defined AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || (defined AIR_WIRELESS_MIC_RX_ENABLE)
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_stop, transmitter_id: 0x%x, transmitter_type: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x, is_app_request: %d", 5,
        p_stream_info->transmitter, transmitter_type, p_stream_info->streaming_flag, p_stream_info->is_transmitter_start, is_app_request);
#ifdef AIR_SILENCE_DETECTION_ENABLE
    ull_report("[ULL][LE][SD_DEBUG][2] bt_ull_le_at_stop, is_silence_detection_on: %d, is_silence_detection_special_mode: %d", 2, p_stream_info->is_silence_detection_on, p_stream_info->is_silence_detection_special_mode);
    if (p_stream_info->is_silence_detection_on) {
        if (BT_ULL_GAMING_TRANSMITTER == transmitter_type || BT_ULL_CHAT_TRANSMITTER == transmitter_type) {
            bt_ull_le_at_silence_detection_stop_by_transmitter_type(transmitter_type);
        }
    }
#endif
        if (true == is_app_request) {
        p_stream_info->is_request_transmitter_start = false;
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
        if (stream_ctx->is_silence) {
            stream_ctx->is_silence = false;
        }
        bt_ull_le_at_remove_silence_flag(transmitter_type);
        if (true == p_stream_info->is_silence_detection_special_mode) {
            p_stream_info->is_silence_detection_special_mode = false;
            bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_NEED_DEINIT);
        }
    #endif
    }
    if (true == p_stream_info->is_transmitter_start) {
        if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING
            || p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING) {
            /* transmitter is starting or stop, just need wait */
            return BT_STATUS_FAIL;
        } else {
            /* gaming & chat transmitter should unmix before stop */
            if (!bt_ull_le_at_type_is_uplink(transmitter_type)) {
                audio_transmitter_runtime_config_type_t config_type = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_UNMIX;

                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_type, NULL)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] unmix fail, transmitter_id:0x%x", 1, p_stream_info->transmitter);
                }
            }
            bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);
            if ((AUD_ID_INVALID != p_stream_info->transmitter)
                && (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_stop(p_stream_info->transmitter))) {
                bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);
                return BT_STATUS_FAIL;
            }
            ull_report("[ULL][LE][AUDIO_TRANS] call audio transmitter stop success!!", 0);
        }
    }
    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

bt_status_t bt_ull_le_at_init(bt_ull_client_t client_type, uint8_t out_type, bt_ull_transmitter_t transmitter_type, bt_ull_le_codec_t codec)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    audio_transmitter_config_t config;
    bt_ull_le_at_info_t *p_stream_info = NULL;

    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    if (AUD_ID_INVALID != p_stream_info->transmitter){
        return BT_STATUS_SUCCESS;
    }
    memset(&config, 0x00, sizeof(config));
    p_stream_info->out_type = out_type;
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_init, transmitter_type: %d, transmitter_id: 0x%x, codec_type: %d, out_type: %d, client_type: %d", 5, transmitter_type, p_stream_info->transmitter, codec, out_type, client_type);

    /* config codec info and init transmitter. */
    if ((BT_ULL_LE_CODEC_VENDOR != codec)) {
        //p_stream_info->codec = codec;
        switch (transmitter_type) {
            //TODO : use table
            case BT_ULL_GAMING_TRANSMITTER:
            case BT_ULL_CHAT_TRANSMITTER: {
                bt_ull_le_at_config_spk_transmitter_param(transmitter_type, client_type, out_type, &config);
                break;
            }

            case BT_ULL_MIC_TRANSMITTER: {
                if (BT_ULL_MIC_CLIENT == client_type) {
                    #ifdef AIR_WIRELESS_MIC_ENABLE
                    bt_ull_le_at_config_wireless_mic_transmitter_param(transmitter_type, client_type, out_type, &config);
                    #endif
                } else {
                    bt_ull_le_at_config_mic_transmitter_param(transmitter_type, client_type, out_type, &config);
                }
                break;
            }

            case BT_ULL_LINE_IN_TRANSMITTER: {
            #ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                bt_ull_le_at_config_linein_transmitter_param(transmitter_type, client_type, out_type, &config);
            #endif
                break;
            }

            case BT_ULL_I2S_IN_TRANSMITTER: {
            #if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
                bt_ull_le_at_config_i2sin_transmitter_param(transmitter_type, client_type, out_type, &config);
            #endif
                break;
            }

            case BT_ULL_LINE_OUT_TRANSMITTER:
            case BT_ULL_I2S_OUT_TRANSMITTER: {
            if (BT_ULL_MIC_CLIENT == client_type) {
                #ifdef AIR_WIRELESS_MIC_ENABLE
                bt_ull_le_at_config_wireless_mic_transmitter_param(transmitter_type, client_type, out_type, &config);
                #endif
            } else {
                bt_ull_le_at_config_mic_transmitter_param(transmitter_type, client_type, out_type, &config);
            }
                break;
            }

            default:
                break;
        }

        //get transmitter id
        if (0 > (p_stream_info->transmitter = audio_transmitter_init(&config))) {
            ull_report_error("[ULL][LE][AUDIO_TRANS][Error] init transmitter failed!!, trans_type is %d",1, transmitter_type);
            return BT_STATUS_FAIL;
        }
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_init, init success!! transmitter_id = 0x%x", 1, p_stream_info->transmitter);
    }
    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

bt_status_t bt_ull_le_at_deinit(bt_ull_transmitter_t transmitter_type, bool is_app_request)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_deinit, transmitter_type: 0x%x, transmitter_id: 0x%x, is_transmitter_start: 0x%x, is_app_request: %d", 4,
        transmitter_type, p_stream_info->transmitter, p_stream_info->is_transmitter_start, is_app_request);
    /*if audio transmitter is started, need stop firstly and set flag.BTA-28920*/
    if ((p_stream_info->is_transmitter_start) || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
        bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_NEED_DEINIT);
        if (true == is_app_request) {
            p_stream_info->is_request_transmitter_start = false;
        }
        bt_ull_le_at_stop(transmitter_type, false);
        return BT_STATUS_SUCCESS;
    } else {
        if (AUD_ID_INVALID != p_stream_info->transmitter) {
            ret = audio_transmitter_deinit(p_stream_info->transmitter);
        }
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_deinit, transmitter_type: 0x%x, transmitter_id: 0x%x, ret: 0x%x", 3, transmitter_type, p_stream_info->transmitter, ret);
        p_stream_info->is_request_transmitter_start = false;
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == ret) {
            p_stream_info->is_transmitter_start = false;
            p_stream_info->streaming_flag = 0x00;
            p_stream_info->transmitter = -1;
        }
    }
    return (AUDIO_TRANSMITTER_STATUS_SUCCESS == ret) ? (BT_STATUS_SUCCESS) : (BT_STATUS_FAIL);
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
static void bt_ull_le_at_print_bt_out_param_log(ull_audio_v2_dongle_bt_out_param_t *bt_out_param, uint8_t link_num)
{//TODO
    uint8_t i;
    for (i = 0; i < link_num; i++) {
        ull_report("[ULL][LE][AUDIO_TRANS] init bt_out_param link_%d done.link_num: %d, sample_rate: %d, sample_format: %d, bit_rate: %d, channel_mode: %d, frame_interval: %d, frame_size: %d.", 8, i,
        bt_out_param->link_num,
        bt_out_param->link_param[i].codec_param.lc3plus.sample_rate,
        bt_out_param->link_param[i].codec_param.lc3plus.sample_format,
        bt_out_param->link_param[i].codec_param.lc3plus.bit_rate,
        bt_out_param->link_param[i].codec_param.lc3plus.channel_mode,
        bt_out_param->link_param[i].codec_param.lc3plus.frame_interval,
        bt_out_param->link_param[i].codec_param.lc3plus.frame_size);
    }
}
static void bt_ull_le_at_print_bt_in_param_log(ull_audio_v2_dongle_bt_in_param_t *bt_in_param, uint8_t link_num)
{//TODO
    uint8_t i;
    for (i = 0; i < link_num; i++) {
        ull_report("[ULL][LE][AUDIO_TRANS] init mic transmitter bt in link_%d done.sample_rate: %d, sample_format: %d, bit_rate: %d, channel_mode: %d, frame_interval: %d, frame_size: %d.", 7, i,
        bt_in_param->link_param[i].codec_param.lc3plus.sample_rate,
        bt_in_param->link_param[i].codec_param.lc3plus.sample_format,
        bt_in_param->link_param[i].codec_param.lc3plus.bit_rate,
        bt_in_param->link_param[i].codec_param.lc3plus.channel_mode,
        bt_in_param->link_param[i].codec_param.lc3plus.frame_interval,
        bt_in_param->link_param[i].codec_param.lc3plus.frame_size);
    }
}

static void bt_ull_le_at_set_bt_out_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, ull_audio_v2_dongle_bt_out_param_t *bt_out_param)
{//TODO
    /*config BT sink parameter*/
    if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {
        bt_out_param->link_num = 2;//for headset or earbuds or speaker, two cis link.
    }
    uint8_t i;
    bt_ull_le_codec_t codec_type = bt_ull_le_srv_get_codec_type();
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_init, bt out current support codec type: %d", 1, codec_type);
    for(i = 0; i < bt_out_param->link_num; i++) {
            /*The setting of the BT link 1*/
            bt_out_param->link_param[i].enable = true;
            bt_out_param->link_param[i].share_info = (void *)bt_ull_le_srv_get_avm_share_buffer_address(client_type, out_type, transmitter_type, i);

        if (BT_ULL_LE_CODEC_LC3PLUS == codec_type) {
            audio_codec_lc3plus_t *p_codec_param = (audio_codec_lc3plus_t*)&bt_out_param->link_param[i].codec_param.lc3plus;
            uint32_t bit_rate = bt_ull_le_srv_get_bitrate(false, BT_ULL_ROLE_SERVER);
            uint32_t frame_size = bt_ull_le_srv_get_sdu_size(false, BT_ULL_ROLE_SERVER);//108

            bt_out_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
            p_codec_param->sample_rate = bt_ull_le_srv_get_codec_sample_rate(transmitter_type, false, BT_ULL_ROLE_SERVER);//96000
            p_codec_param->sample_format = HAL_AUDIO_PCM_FORMAT_S24_LE;// lc3plus codec
            p_codec_param->bit_rate = bit_rate/2;
            /*switch channel mode*/
            bt_ull_le_channel_mode_t channel_mode = bt_ull_le_srv_get_channel_mode(transmitter_type, false, BT_ULL_ROLE_SERVER);
            p_codec_param->channel_mode = bt_ull_le_at_switch_channel_mode(channel_mode);
            p_codec_param->frame_interval = bt_ull_le_srv_get_sdu_interval(false, BT_ULL_ROLE_SERVER);//5000
            p_codec_param->frame_size = frame_size/2;//54
        } else if (BT_ULL_LE_CODEC_OPUS == codec_type) {
            audio_codec_opus_t *p_codec_param = (audio_codec_opus_t*)&bt_out_param->link_param[i].codec_param.opus;
            uint32_t bit_rate = bt_ull_le_srv_get_bitrate(false, BT_ULL_ROLE_SERVER);
            uint32_t frame_size = bt_ull_le_srv_get_sdu_size(false, BT_ULL_ROLE_SERVER);//108

            bt_out_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
            p_codec_param->sample_rate= bt_ull_le_srv_get_codec_sample_rate(transmitter_type, false, BT_ULL_ROLE_SERVER);//96000
            p_codec_param->sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;//opus codec

            p_codec_param->bit_rate = bit_rate/2;
            /*switch channel mode*/
            bt_ull_le_channel_mode_t channel_mode = bt_ull_le_srv_get_channel_mode(transmitter_type, false, BT_ULL_ROLE_SERVER);
            p_codec_param->channel_mode = bt_ull_le_at_switch_channel_mode(channel_mode);
            p_codec_param->frame_interval = bt_ull_le_srv_get_sdu_interval(false, BT_ULL_ROLE_SERVER);//5000
            p_codec_param->frame_size = frame_size/2;//54
        } else if ((BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == codec_type)
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
                 ||(BT_ULL_LE_CODEC_DL_ULD_UL_OPUS == codec_type)
#endif
        ) {
            audio_codec_uld_t *p_codec_param = (audio_codec_uld_t*)&bt_out_param->link_param[i].codec_param.uld;
            bt_out_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_ULD;
            p_codec_param->sample_rate= bt_ull_le_srv_get_codec_sample_rate(transmitter_type, false, BT_ULL_ROLE_SERVER);//48000
            p_codec_param->sample_format = HAL_AUDIO_PCM_FORMAT_S24_LE;//uld codec
            uint32_t bit_rate = bt_ull_le_srv_get_bitrate(false, BT_ULL_ROLE_SERVER);
            p_codec_param->bit_rate = bit_rate/2;
            /*switch channel mode*/
            bt_ull_le_channel_mode_t channel_mode = bt_ull_le_srv_get_channel_mode(transmitter_type, false, BT_ULL_ROLE_SERVER);
            p_codec_param->channel_mode = bt_ull_le_at_switch_channel_mode(channel_mode);
            p_codec_param->frame_interval = bt_ull_le_srv_get_sdu_interval(false, BT_ULL_ROLE_SERVER);//2000
            uint32_t frame_size = bt_ull_le_srv_get_sdu_size(false, BT_ULL_ROLE_SERVER);//100
            p_codec_param->frame_size = frame_size/2;//50
        } else {
          ull_assert(0 && "unsupport codec type!!");
        }
    }
}

static void bt_ull_le_at_set_bt_in_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, ull_audio_v2_dongle_bt_in_param_t *bt_in_param)
{//TODO
    bt_in_param->link_num = 2;
    bt_ull_le_codec_t codec_type = bt_ull_le_srv_get_codec_type();
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_init, bt in current support codec type: %d", 1, codec_type);
    uint8_t i;
    for (i = 0; i < bt_in_param->link_num; i++) {
        bt_in_param->link_param[i].enable = true;
        bt_in_param->link_param[i].share_info = (void *)bt_ull_le_srv_get_avm_share_buffer_address(client_type, out_type, transmitter_type, i);
        if(BT_ULL_LE_CODEC_LC3PLUS == codec_type || BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == codec_type) {
            bt_in_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_LC3PLUS;   //default lc3plus codec
        }
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
        else if (BT_ULL_LE_CODEC_DL_ULD_UL_OPUS == codec_type) {
            bt_in_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
        }
#endif
        else {
            bt_in_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
        }
        bt_in_param->link_param[i].codec_param.lc3plus.sample_rate = bt_ull_le_srv_get_codec_sample_rate(transmitter_type, true, BT_ULL_ROLE_SERVER);//32000;
        if (BT_ULL_MIC_TRANSMITTER == transmitter_type) {
            bt_in_param->link_param[i].codec_param.lc3plus.sample_format = bt_ull_le_at_switch_sample_size(bt_ull_le_srv_get_usb_sample_size(transmitter_type, BT_ULL_ROLE_SERVER));
        } else {
            bt_in_param->link_param[i].codec_param.lc3plus.sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;//need change to AFE_PCM_FORMAT_S16_LE
        }
        bt_in_param->link_param[i].codec_param.lc3plus.bit_rate = bt_ull_le_srv_get_bitrate(true, BT_ULL_ROLE_SERVER);//32*1000;
        bt_in_param->link_param[i].codec_param.lc3plus.channel_mode = bt_ull_le_srv_get_channel_mode(transmitter_type, true, BT_ULL_ROLE_SERVER);//0x8;
        bt_in_param->link_param[i].codec_param.lc3plus.frame_interval = bt_ull_le_srv_get_sdu_interval(true, BT_ULL_ROLE_SERVER);//5000;
        bt_in_param->link_param[i].codec_param.lc3plus.frame_size = bt_ull_le_srv_get_sdu_size(true, BT_ULL_ROLE_SERVER);///40;
    }
}


static void bt_ull_le_at_gaming_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_spk);
    //bt_ull_streaming_if_info_t *p_stream_info = &g_ull_le_at_ctx->dl_spk;
    bt_ull_le_at_result_t result_notify;
    result_notify.transmitter_type = BT_ULL_GAMING_TRANSMITTER;
    if (AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS != event) {
        ull_report("[ULL][LE][AUDIO_TRANS] audio_transmitter_gaming_callback event = 0x%x, is_request_transmitter_start = 0x%x, flag:0x%x", 3,
            event, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][3] gaming callback, silence_detection_mode: %d", 1, bt_ull_le_at_get_silence_detection_mode());
            if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == bt_ull_le_at_get_silence_detection_mode()) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            }
        #else
        if (bt_ull_le_service_is_connected()) {
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
        }

        #endif
            bt_ull_le_at_handle_spk_start_success_cnf(BT_ULL_GAMING_TRANSMITTER);
            //bt_ull_le_audio_transmitter_handle_spk_start_success_cnf();
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(BT_ULL_GAMING_TRANSMITTER);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
             bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(BT_ULL_GAMING_TRANSMITTER);
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            if (0 == (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            }
        #else
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
        #endif

            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_at_mic_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_transmitter_t trans_type = BT_ULL_TRANSMITTER_MAX_NUM;
    bt_ull_le_at_uplink_type at_uplink_type = (bt_ull_le_at_uplink_type)user_data;
    //bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    //bt_ull_streaming_if_info_t *p_stream_info = &g_ull_le_at_ctx->ul_mic;
    bt_ull_le_at_result_t result_notify;
    trans_type = (BT_ULL_LE_AT_UPLINK_LINEOUT == at_uplink_type)?(BT_ULL_LINE_OUT_TRANSMITTER):((BT_ULL_LE_AT_UPLINK_I2SOUT == at_uplink_type)?(BT_ULL_I2S_OUT_TRANSMITTER):(BT_ULL_MIC_TRANSMITTER));
    result_notify.transmitter_type = trans_type;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(trans_type);
    if (AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS != event) {
        ull_report("[ULL][LE][AUDIO_TRANS] audio_transmitter_mic_callback event = 0x%x, trans_type = %d, is_request_transmitter_start = 0x%x, flag:0x%x", 4,
            event, trans_type, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_mic_start_success_cnf(trans_type);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_mic_stop_success_or_start_fail_cnf(trans_type);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS:{
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_handle_mic_stop_success_or_start_fail_cnf(trans_type);
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_at_chat_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_chat);
    //bt_ull_streaming_if_info_t *p_stream_info = &g_ull_le_at_ctx->dl_chat;
    bt_ull_le_at_result_t result_notify;
    result_notify.transmitter_type = BT_ULL_CHAT_TRANSMITTER;
    if (AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS != event) {
        ull_report("[ULL][LE][AUDIO_TRANS] audio_transmitter_chat_callback event = 0x%x, is_request_transmitter_start = 0x%x, flag:0x%x", 3,
            event, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][3] chat callback, silence_detection_mode: %d", 1, bt_ull_le_at_get_silence_detection_mode());
            if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == bt_ull_le_at_get_silence_detection_mode()) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            }
        #else
            if (bt_ull_le_service_is_connected()) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            }
        #endif
            bt_ull_le_at_handle_spk_start_success_cnf(BT_ULL_CHAT_TRANSMITTER);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(BT_ULL_CHAT_TRANSMITTER);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
               bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(BT_ULL_CHAT_TRANSMITTER);
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            if (0 == (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            }
        #else
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
        #endif
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_at_handle_mic_start_success_cnf(bt_ull_transmitter_t transmitter_type)
{
    //bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if (p_stream_info) {
        //ull_report("[ULL][LE][AUDIO_TRANS] handle_mic_audio_transmitter_start_success_cnf, transmitter_type: %d, flag: 0x%x.", 2, transmitter_type, p_stream_info_mic->streaming_flag);
        p_stream_info->is_transmitter_start = true;//set flag,transmitter is start
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag
        //need notify uplayer?

        /* usb sample rate, should reinit transmitter */
        if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
            /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
            bt_ull_le_at_stop(transmitter_type, false);
        } else {
            if (!p_stream_info->is_request_transmitter_start) {
                bt_ull_le_at_stop(transmitter_type, false);
            } else {
                /*step1: set volume*/
                //bt_ull_le_at_config_volume(transmitter_type);
                bt_ull_le_at_sync_volume_info(transmitter_type);
                /*step2: config mic channel*/
            }
        }
    }
}

static void bt_ull_le_at_handle_spk_start_success_cnf(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    bt_ull_le_at_info_t *p_stream_info_other = NULL;
    bt_ull_transmitter_t trans_type;
    bt_ull_transmitter_t trans_type_other;
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    if (BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
        p_stream_info = &(trans_ctx->dl_spk);
        p_stream_info_other = &(trans_ctx->dl_chat);
        trans_type = BT_ULL_GAMING_TRANSMITTER;
        trans_type_other = BT_ULL_CHAT_TRANSMITTER;
    } else {
        p_stream_info = &(trans_ctx->dl_chat);
        p_stream_info_other = &(trans_ctx->dl_spk);
        trans_type = BT_ULL_CHAT_TRANSMITTER;
        trans_type_other = BT_ULL_GAMING_TRANSMITTER;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] handle_spk_audio_transmitter_start_success_cnf, transmitter_type: %d, is_request_transmitter_start: %d, flag: 0x%x.", 3,
         transmitter_type,p_stream_info->is_request_transmitter_start,p_stream_info->streaming_flag);
    ull_report("[ULL][LE][AUDIO_TRANS] handle_spk_audio_transmitter_start_success_cnf, check the other port status, transmitter_type: %d, is_request_transmitter_start: %d, flag: 0x%x.", 3,
         trans_type_other,p_stream_info_other->is_request_transmitter_start,p_stream_info_other->streaming_flag);


    p_stream_info->is_transmitter_start = true; //set flag, transmitter is started

    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag

    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        /*change sample rate case: should stop transmitter*/
        bt_ull_le_at_stop(trans_type, false);
    } else if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        /*need restart case, eg: change latency*/
        bt_ull_le_at_stop(trans_type, false);
    } else {
        if (!p_stream_info->is_request_transmitter_start) {
        //BTA-36811
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            if ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE) && (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == bt_ull_le_at_get_silence_detection_mode())) {
                bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_NEED_DEINIT);
            }
        #endif
            bt_ull_le_at_stop(trans_type, false);
        } else {
            /*step1: check if need mixed or not*/
            if (p_stream_info_other->is_request_transmitter_start
            && (AUD_ID_INVALID != p_stream_info_other->transmitter)
            && !(p_stream_info_other->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                /*gaming & chat is active, should mix both*/
                ull_report("[ULL][LE][AUDIO_TRANS] gaming & chat is active, should mix both!!", 0);
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = p_stream_info_other->transmitter;
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, p_stream_info_other->transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(trans_type, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(trans_type_other, BT_ULL_LE_STREAMING_MIXED);
                    //bt_ull_le_at_config_volume(trans_type_other);//chat
                    bt_ull_le_at_sync_volume_info(trans_type_other);
                }
            }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
            /*check linein status and need mix linein*/
            if (trans_ctx->dl_linein.is_request_transmitter_start
            && trans_ctx->dl_linein.is_transmitter_start
            && (AUD_ID_INVALID != trans_ctx->dl_linein.transmitter)
            && !(trans_ctx->dl_linein.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_linein.transmitter;
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, p_stream_info_other->transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(trans_type, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    //bt_ull_le_at_config_volume(BT_ULL_LINE_IN_TRANSMITTER);//line in
                    bt_ull_le_at_sync_volume_info(BT_ULL_LINE_IN_TRANSMITTER);
                }
            }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
            /*check linein status and need mix linein*/
            if (trans_ctx->dl_i2sin.is_request_transmitter_start
            && trans_ctx->dl_i2sin.is_transmitter_start
            && (AUD_ID_INVALID != trans_ctx->dl_i2sin.transmitter)
            && !(trans_ctx->dl_i2sin.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_i2sin.transmitter;
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, p_stream_info_other->transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(trans_type, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    //bt_ull_le_at_config_volume(BT_ULL_I2S_IN_TRANSMITTER);//line in
                    bt_ull_le_at_sync_volume_info(BT_ULL_I2S_IN_TRANSMITTER);
                }
            }
#endif
            /*step2: mix self*/
            audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
            audio_transmitter_runtime_config_t runtime_config;
            runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = p_stream_info->transmitter;
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                ull_report_error("[ULL][LE][AUDIO_TRANS][Error] music mixing self fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            } else {
                ull_report("[ULL][LE][AUDIO_TRANS] music mixing self success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            }

            /*step3: set self volume*/
            bt_ull_le_at_sync_volume_info(trans_type);

            /*step4: start silence detection*/
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][4] spk start success cnf, streaming_flag: 0x%x, is_silence_detetcion_on: %d", 2, p_stream_info->streaming_flag, p_stream_info->is_silence_detection_on);
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE) {
                bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
                p_stream_info->is_silence_detection_special_mode = true;
            }
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE) {
                bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
                p_stream_info->is_silence_detection_special_mode = false;
            }
            if (false == p_stream_info->is_silence_detection_on) {
                bt_ull_le_at_silence_detection_start_by_transmitter_type(transmitter_type);
                if (false == p_stream_info->is_silence_detection_special_mode) {
                    bt_ull_le_at_set_silence_flag(transmitter_type);//silence event is not occur, set bit mask
               }
            }
        #endif
        }
    }
}

static void bt_ull_le_at_handle_spk_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    bt_ull_le_at_info_t *p_stream_info_other = NULL;
    bt_ull_transmitter_t trans_type;
    bt_ull_transmitter_t trans_type_other;
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();

    if (BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
        p_stream_info = &(trans_ctx->dl_spk);
        p_stream_info_other = &(trans_ctx->dl_chat);
        trans_type = BT_ULL_GAMING_TRANSMITTER;
        trans_type_other = BT_ULL_CHAT_TRANSMITTER;
    } else {
        p_stream_info = &(trans_ctx->dl_chat);
        p_stream_info_other = &(trans_ctx->dl_spk);
        trans_type = BT_ULL_CHAT_TRANSMITTER;
        trans_type_other = BT_ULL_GAMING_TRANSMITTER;
    }
    p_stream_info->is_transmitter_start = false;//set flag, transmitter is stop

    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_MIXED);//remove flag

#ifdef AIR_SILENCE_DETECTION_ENABLE
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
#endif
    /*check if need deinit BTA-28920*/
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_NEED_DEINIT) {
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_NEED_DEINIT);
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        p_stream_info->transmitter = -1;
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x, streaming_flag: 0x%x", 3, transmitter_type, status, p_stream_info->streaming_flag);
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        ull_report("[ULL][LE][SD_DEBUG][5] spk stop cnf, streaming_flag: 0x%x, silence_detection_mode: %d, is_request_transmitter_start: %d", 3,
            p_stream_info->streaming_flag, silence_detection_mode, p_stream_info->is_request_transmitter_start);
        if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE)) {
            bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE);
            if (p_stream_info->is_request_transmitter_start && bt_ull_le_service_is_connected()) {
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
                bt_ull_le_at_silence_detection_start_special_mode(transmitter_type);
                return; //just reture and waiting for start special mode finish. BTA-41837
            }
            return; //special case: when spk stop normal mode and waiting for start special mode, but the spk port is disable, just return.  BTA-42917
        } else if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
            bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE);
            p_stream_info->is_silence_detection_special_mode = false;
            //BTA-38653
            if (p_stream_info_other->streaming_flag & BT_ULL_LE_STREAMING_START_BLOCKED) {
                bt_ull_le_at_remove_flag(trans_type_other, BT_ULL_LE_STREAMING_START_BLOCKED);
                if (p_stream_info_other->is_request_transmitter_start && bt_ull_le_service_is_connected()) {
                    bt_ull_le_at_start(trans_type_other, false);
                }
            }
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_START_BLOCKED) {
                bt_ull_le_at_remove_flag(trans_type, BT_ULL_LE_STREAMING_START_BLOCKED);
            }			
            if (p_stream_info->is_request_transmitter_start && bt_ull_le_service_is_connected()) {
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
                bt_ull_le_at_silence_detection_start_normal_mode(transmitter_type);
                bt_ull_le_srv_silence_detection_notify_client_status(false, transmitter_type);//notify client to open am
            }
        } else if ((!bt_ull_le_service_is_connected() || ((false == p_stream_info->is_request_transmitter_start) && (false == p_stream_info_other->is_silence_detection_special_mode)))
                && (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode)) {
            if (!p_stream_info_other->is_silence_detection_special_mode) {
                bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL);
            }
            if(p_stream_info->is_silence_detection_special_mode) {
                p_stream_info->is_silence_detection_special_mode = false;
            }
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            stream_ctx->is_silence = false;
            return;
        }
    #endif
    }

    /*if usb change sample rate, should reinit transmitter, when handle change sample rate, must set flag: BT_ULL_STREAMING_RECONFIG */
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);//remove flag  //
        audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
        if (AUD_ID_INVALID != p_stream_info->transmitter) {
            status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        }
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x", 2, transmitter_type, status);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
            p_stream_info->transmitter = AUD_ID_INVALID;
            //bt_ull_le_at_init(trans_type, p_stream_info->codec_type, NULL))
            bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
        }
    }
    if(p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_RESTART);//remove restart flag
    }
    if (p_stream_info->is_request_transmitter_start) {
        /* 2-rx, we should wait for all transmitter stop, then start again */
        ull_report("[ULL][LE][AUDIO_TRANS] check other port, streaming_flag: 0x%x, is_start: 0x%x", 2, p_stream_info_other->streaming_flag, p_stream_info_other->is_transmitter_start);
        if ((AUD_ID_INVALID != p_stream_info_other->transmitter)
           && p_stream_info_other->is_transmitter_start
           && (p_stream_info_other->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
           //ull_report("[ULL][LE][AUDIO_TRANS] the other speaker streaming is stopping, we need wait it stop.", 0);
           return;
        }
        if (p_stream_info_other->is_request_transmitter_start) {
            //ull_report("[ULL][LE][AUDIO_TRANS] the other speaker is request start transmitter, need start directly.", 0);
            bt_ull_le_at_start(trans_type_other, false);
        }
        bt_ull_le_at_start(trans_type, false);
    }
#if 0
    if (bt_ull_le_at_silence_detection_check_need_start_special_mode()) {
        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
        //all scenario is silence, should stop normal mode and start special mode
        ull_report("[ULL][LE][SD_DEBUG][6] need start special mode", 0);
        bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL);//set special mode
        bt_ull_le_at_silence_detection_stop_normal_mode();//all scenarios is silence, should stop audio transmitter
        //bt_ull_le_srv_silence_detection_notify_client_status(true, transmitter_type);
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
            bt_ull_le_srv_silence_detection_notify_client_status(true, BT_ULL_GAMING_TRANSMITTER);//notify client stop am
        }
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
            bt_ull_le_srv_silence_detection_notify_client_status(true, BT_ULL_CHAT_TRANSMITTER);//notify client stop am
        }
    }
#endif
}

static void bt_ull_le_at_handle_mic_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type)
{
    //print log
    //bt_ull_le_at_info_t *p_stream_info_mic = &(g_ull_le_at_ctx.ul_mic);
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag
    //need notify uplayer?
    if (p_stream_info) {
        p_stream_info->is_transmitter_start = false;
        /*check if need deinit BTA-28920*/
        if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_NEED_DEINIT) {
            bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_NEED_DEINIT);
            audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
            p_stream_info->transmitter = -1;
            ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x, streaming_flag: 0x%x", 3, transmitter_type, status, p_stream_info->streaming_flag);
        }
        /* usb sample rate, should reinit transmitter */
        if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
            bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);//remove flag
            audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
            if (AUD_ID_INVALID != p_stream_info->transmitter) {
                status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
            }
            ull_report("[ULL][LE][AUDIO_TRANS] mic transmitter deinit :0x%x", 1, status);
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                p_stream_info->transmitter = AUD_ID_INVALID;
                //bt_ull_le_at_init(transmitter_type, p_stream_info_mic->codec_type, void * codec_info));
                bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
                p_stream_info->is_request_transmitter_start = true;//set flag, request transmitter start
            }
        }
        if(p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
            bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_RESTART);//remove restart flag
        }

        /* check last user request restart transmitter or not */
        if (p_stream_info->is_request_transmitter_start) {
            bt_ull_le_at_start(transmitter_type, false);
        }
    #if 0
        if (bt_ull_le_at_silence_detection_check_need_start_special_mode()) {
            //all scenario is silence, should stop normal mode and start special mode
            ull_report("[ULL][LE][SD_DEBUG][15] need start special mode", 0);
            bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL);//set special mode
            bt_ull_le_at_silence_detection_stop_normal_mode();//all scenarios is silence, should stop audio transmitter
            bt_ull_le_srv_silence_detection_notify_client_status(true, transmitter_type);
        }
    #endif
    }
}

static void bt_ull_le_at_config_spk_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    ull_audio_v2_dongle_usb_in_param_t *usb_in_param;
    ull_audio_v2_dongle_bt_out_param_t *bt_out_param;
    config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
    config->scenario_sub_id = (BT_ULL_GAMING_TRANSMITTER == transmitter_type) ? AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1 : AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
    bool is_without_bt_link = false;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if (!p_stream_info) {
        return;
    }
    ull_report("[ULL][LE][SD_DEBUG][6] bt_ull_le_at_config_spk_transmitter_param, streaming_flag: 0x%x, silence_detection_mode: %d", 2, p_stream_info->streaming_flag, silence_detection_mode);
    if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE) 
        || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_REINIT_IN_SPECIAL_MODE))) {
        //config->scenario_config.ull_audio_v2_dongle_config.without_bt_link_mode_enable = true;
        config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = true;
        is_without_bt_link = true;
    } else
#endif
    {
        config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = false;
    }
    /*config USB source parameters*/
    usb_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.usb_in_param);
    bt_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param);

    usb_in_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    usb_in_param->codec_param.pcm.sample_rate = bt_ull_le_srv_get_usb_sample_rate(transmitter_type, BT_ULL_ROLE_SERVER);
    usb_in_param->codec_param.pcm.format = bt_ull_le_at_switch_sample_size(bt_ull_le_srv_get_usb_sample_size(transmitter_type, BT_ULL_ROLE_SERVER));
    usb_in_param->codec_param.pcm.channel_mode = bt_ull_le_srv_get_usb_sample_channel(transmitter_type, BT_ULL_ROLE_SERVER);
    usb_in_param->codec_param.pcm.frame_interval = 1000;//
    ull_report("[ULL][LE][AUDIO_TRANS] init usb_in_param done. transmitter_type: %d, sample_rate: %d, format: %d, channel_mode: %d, frame_interval: %d.", 5,
        transmitter_type,
        usb_in_param->codec_param.pcm.sample_rate,
        usb_in_param->codec_param.pcm.format,
        usb_in_param->codec_param.pcm.channel_mode,
        usb_in_param->codec_param.pcm.frame_interval);
    ull_report("[ULL][LE][AUDIO_TRANS][SD_DEBUG] bt_ull_le_at_config_spk_transmitter_param, is_without_bt_link: %d", 1, is_without_bt_link);
    /*config bt out param*/
    bt_ull_le_at_set_bt_out_param(transmitter_type, client_type, out_type, bt_out_param);

    /*Init callback handler*/
    config->msg_handler = (BT_ULL_GAMING_TRANSMITTER == transmitter_type) ? bt_ull_le_at_gaming_callback : bt_ull_le_at_chat_callback;
    config->user_data = NULL;
    bt_ull_le_at_print_bt_out_param_log(bt_out_param, 2);
}
#if 0
static void bt_ull_le_at_config_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_USB == out_type) {
        if (BT_ULL_HEADSET_CLIENT == client_type || BT_ULL_EARBUDS_CLIENT == client_type) {
            /*ull v2 dongle uplink param init*/
            ull_audio_v2_dongle_usb_out_param_t *usb_out_param;
            ull_audio_v2_dongle_bt_in_param_t *bt_in_param;

            bt_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.source_param.bt_in_param);
            usb_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.sink_param.usb_out_param);
            config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
            config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0;
            /*config BT sink parameter*/
            bt_in_param->link_num = 2;//current support one up link, may support multi up link such as wireless microphone
#ifdef AIR_ULL_ECNR_POST_PART_ENABLE
            /*for AB156x ULL 2.0  uplink AINR Feature.*/
            bt_in_param->nr_offload_flag = true;
#endif

            /*config USB sink parameter*/
            usb_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
            usb_out_param->codec_param.pcm.sample_rate = bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
            usb_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
            usb_out_param->codec_param.pcm.channel_mode = 0x01;
            //usb_out_param->codec_param.pcm.format = bt_ull_le_at_switch_sample_size(bt_ull_le_srv_get_usb_sample_size(transmitter_type));
            //usb_out_param->codec_param.pcm.channel_mode = bt_ull_le_srv_get_usb_sample_channel(transmitter_type);
            usb_out_param->codec_param.pcm.frame_interval = 1000;

            bt_ull_le_at_set_bt_in_param(transmitter_type, client_type, out_type, bt_in_param);
            /*Init callback handler*/
            config->msg_handler = bt_ull_le_at_mic_callback;
            config->user_data = NULL;
            bt_ull_le_at_print_bt_in_param_log(bt_in_param, 2);
        }

    } else {
        ull_assert(0 && "the out_type is not BT_ULL_LE_TRANSMITTER_AUDIO_OUT_USB");
    }
}
#endif
static void bt_ull_le_at_config_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    if (BT_ULL_HEADSET_CLIENT == client_type || \
        BT_ULL_EARBUDS_CLIENT == client_type || \
        BT_ULL_SPEAKER_CLIENT == client_type) {
        config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
        ull_audio_v2_dongle_bt_in_param_t *bt_in_param;
        bt_ull_le_at_uplink_type at_uplink_type = 0;
        bt_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.source_param.bt_in_param);
#ifdef AIR_ULL_ECNR_POST_PART_ENABLE
        /*for AB156x ULL 2.0  uplink AINR Feature.*/
        bt_in_param->nr_offload_flag = true;
#endif
        switch (transmitter_type) {
            case BT_ULL_MIC_TRANSMITTER: {
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_USB == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_USBOUT;
                    config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0;
                    ull_audio_v2_dongle_usb_out_param_t *usb_out_param;
                    usb_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.sink_param.usb_out_param);
                    bt_in_param->link_num = 2;
                    /*config USB sink parameter*/
                    usb_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    usb_out_param->codec_param.pcm.sample_rate = bt_ull_le_srv_get_usb_sample_rate(transmitter_type, BT_ULL_ROLE_SERVER);
                    usb_out_param->codec_param.pcm.format = bt_ull_le_at_switch_sample_size(bt_ull_le_srv_get_usb_sample_size(transmitter_type, BT_ULL_ROLE_SERVER));
                    usb_out_param->codec_param.pcm.channel_mode = 0x01;
                    usb_out_param->codec_param.pcm.frame_interval = 1000;
                    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_config_mic_transmitter_param.sample size: %d",1, usb_out_param->codec_param.pcm.format);
                }
                break;
            }

            case BT_ULL_LINE_OUT_TRANSMITTER: {
            #ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_AUX == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_LINEOUT;
                    config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT;
                    ull_audio_v2_dongle_line_out_param_t *line_out_param;
                    line_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.sink_param.line_out_param);
                    bt_in_param->link_num = 2;
                    /*config line out parameter*/
                    line_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    line_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    line_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                    line_out_param->codec_param.pcm.channel_mode = 0x01;
                    line_out_param->codec_param.pcm.frame_interval = 1000;
                }
            #endif
                break;
            }

            case BT_ULL_I2S_OUT_TRANSMITTER: {
            #if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE) || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE)
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_I2S == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_I2SOUT;
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
                    config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0;
                    ull_audio_v2_dongle_i2s_slv_out_param_t *i2s_out_param;
                    i2s_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.sink_param.i2s_slv_out_param);
                    bt_in_param->link_num = 2;
                    /*config line out parameter*/
                    i2s_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    i2s_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    i2s_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                    i2s_out_param->codec_param.pcm.channel_mode = 0x01;
                    i2s_out_param->codec_param.pcm.frame_interval = 1000;
#elif defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
                    config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0;
                    ull_audio_v2_dongle_i2s_mst_out_param_t *i2s_out_param;
                    i2s_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.ul_config.sink_param.i2s_mst_out_param);
                    bt_in_param->link_num = 2;
                    /*config line out parameter*/
                    i2s_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    i2s_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    i2s_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                    i2s_out_param->codec_param.pcm.channel_mode = 0x01;
                    i2s_out_param->codec_param.pcm.frame_interval = 1000;
#endif
                }
            #endif
                break;
            }

            default:
                break;
        }

        bt_ull_le_at_set_bt_in_param(transmitter_type, client_type, out_type, bt_in_param);
        config->msg_handler = bt_ull_le_at_mic_callback;
        config->user_data = (void *)at_uplink_type;
        bt_ull_le_at_print_bt_in_param_log(bt_in_param, 2);
    }  else {
        ull_assert(0 && "the client type is not right!!");
    }
}


static bt_status_t bt_ull_le_at_config_volume(bt_ull_transmitter_t transmitter_type, bt_ull_audio_channel_t channel)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_ull_le_at_info_t *p_stream_info = NULL;
    audio_transmitter_status_t ret;
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_client_t client_type = bt_ull_le_srv_get_client_type();
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_config_volume, transmitter_id: 0x%x, is_transmitter_start: %d, channel: %d", 3, p_stream_info->transmitter, p_stream_info->is_transmitter_start, channel);
    audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_MAX;
    audio_transmitter_runtime_config_t runtime_config = {0};

    if (bt_ull_le_at_type_is_uplink(transmitter_type)) {
        if (BT_ULL_MIC_CLIENT == client_type) {
        #ifdef AIR_WIRELESS_MIC_ENABLE
            config_op = WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_LEVEL;
        #else
            ull_report_error("[ULL][LE][AUDIO_TRANS] the option - AIR_WIRELESS_MIC_ENABLE is not open.", 0);
        #endif
        } else if (BT_ULL_HEADSET_CLIENT == client_type \
            || BT_ULL_EARBUDS_CLIENT == client_type \
            || BT_ULL_SPEAKER_CLIENT == client_type) {
        config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_VOL_LEVEL;
        }
    }
    else {
        config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_VOL_LEVEL;
    }

    if ((AUD_ID_INVALID != p_stream_info->transmitter) && p_stream_info->is_transmitter_start) {
        if (BT_ULL_MIC_CLIENT == client_type) {
            if (bt_ull_le_at_type_is_uplink(transmitter_type)) {
            #ifdef AIR_WIRELESS_MIC_ENABLE
                wireless_mic_rx_vol_info_t *vol_info = &(runtime_config.wireless_mic_rx_runtime_config.vol_info);
                if (p_stream_info->is_mute) {
                    vol_info->vol_level = AUD_VOL_OUT_LEVEL0;
                } else {
                    vol_info->vol_level = p_stream_info->volume_info.volume[channel].level;
                }
                vol_info->vol_ch = p_stream_info->volume_info.volume[channel].audio_channel;
            #else
                ull_report_error("[ULL][LE][AUDIO_TRANS] the option - AIR_WIRELESS_MIC_ENABLE is not open.", 0);
            #endif
            }
        } else if (BT_ULL_HEADSET_CLIENT == client_type \
            || BT_ULL_EARBUDS_CLIENT == client_type \
            || BT_ULL_SPEAKER_CLIENT == client_type) {
            ull_audio_v2_vol_info_t *vol_info = &(runtime_config.ull_audio_v2_dongle_runtime_config.vol_info);
            if (p_stream_info->is_mute) {
                vol_info->vol_level = AUD_VOL_OUT_LEVEL0;
            } else {
                vol_info->vol_level = p_stream_info->volume_info.volume[channel].level;
            }
            vol_info->vol_ch = p_stream_info->volume_info.volume[channel].audio_channel;
            ull_report("[ULL][LE][AUDIO_TRANS]bt_ull_le_at_config_volume, vol_level: %d, vol_ch: %d.", 2, vol_info->vol_level, vol_info->vol_ch);
        }
        /*for speaker, need config ratio*/
        if (BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[0].ratio;
        } else if(BT_ULL_CHAT_TRANSMITTER == transmitter_type) {
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[1].ratio;
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        else if(BT_ULL_LINE_IN_TRANSMITTER == transmitter_type) {
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[2].ratio;
        }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        else if(BT_ULL_I2S_IN_TRANSMITTER == transmitter_type) {
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[3].ratio;
        }
#endif

        ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            ull_report_error("[ULL][LE][AUDIO_TRANS][Error] audio_transmitter_set_runtime_config fail, trans_id:0x%x, volume:0x%x", 2, p_stream_info->transmitter,
                p_stream_info->volume_info.volume[channel].level);
            return BT_STATUS_FAIL;
        }
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_config_volume success!", 0);
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] skip set volume due to streaming is not start!", 0);
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_config_volume, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

static bt_status_t bt_ull_le_at_sync_volume_info(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!!");
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_sync_volume_info, is_dual_channel_set: %d", 1, p_stream_info->volume_info.is_dual_channel_set);
    if (p_stream_info->volume_info.is_dual_channel_set) {
        bt_ull_le_at_config_volume(transmitter_type, BT_ULL_AUDIO_CHANNEL_DUAL);//if dual channel set, only config dual channel
    } else {
        uint8_t channel;
        for (channel = BT_ULL_AUDIO_CHANNEL_LEFT; channel <= BT_ULL_AUDIO_CHANNEL_RIGHT; channel++) {
            if (bt_ull_le_at_config_volume(transmitter_type, channel)) {
                continue;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static uint32_t bt_ull_le_at_switch_channel_mode(bt_ull_le_channel_mode_t channel_mode)
{
    uint32_t ret = 0;
    //ull_report("[ULL][LE][AUDIO_TRANS] switch channel mode, channel_mode: 0x%x", 1, channel_mode);
    if(BT_ULL_LE_CHANNEL_MODE_STEREO == channel_mode) {
        ret = 0x02;
    } else if(BT_ULL_LE_CHANNEL_MODE_DUAL_MONO == channel_mode) {
        ret = 0x04;
    } else if(BT_ULL_LE_CHANNEL_MODE_MONO == channel_mode) {
        ret = 0x08;
    } else {
        ull_report_error("[ULL][LE][AUDIO_TRANS] channel_mode is invalid!!", 0);
        ull_assert(0 && "invalid channel mode");
    }
    ull_report("[ULL][LE][AUDIO_TRANS] switch channel mode, origin channel_mode: 0x%x, switched channel_mode: 0x%x", 2, channel_mode, ret);
    return ret;
}

static uint32_t bt_ull_le_at_switch_sample_size(uint32_t sample_size)
{
    uint32_t ret = 0;
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_switch_sample_size, sample size: 0x%x", 1, sample_size);
    if (0x01 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S8;
    } else if (0x02 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S16_LE;
    } else if (0x03 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S24_LE;
    } else if (0x04 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S32_LE;
    } else {
        ull_assert(0 && "invalid sample size");
    }
    return ret;
}

static void bt_ull_le_at_event_callback(bt_ull_le_at_event_t event, void *param, uint32_t param_len)
{
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    ull_report("[ULL][LE][AUDIO_TRANS][CALLBACK] bt_ull_le_at_event_callback. event:0x%x, param len:0x%x", 2, event, param_len);
    if (trans_ctx && trans_ctx->callback) {
        trans_ctx->callback(event, param, param_len);
    }
}

static void bt_ull_le_at_remove_flag(bt_ull_transmitter_t transmitter_type, bt_ull_le_transmitter_flag_t mask)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!!");
    }
    p_stream_info->streaming_flag &= ~mask;
    ull_report("[ULL][LE][AUDIO_TRANS] remove flag: 0x%x, trans_type: %d, current streaming flag: 0x%x", 3, mask, transmitter_type, p_stream_info->streaming_flag);
}


#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE

static void bt_ull_le_at_config_linein_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    ull_audio_v2_dongle_line_in_param_t *line_in_param;
    ull_audio_v2_dongle_bt_out_param_t *bt_out_param;
    config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
    config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    ull_report("[ULL][LE][SD_DEBUG][13] bt_ull_le_at_config_spk_transmitter_param, streaming_flag: 0x%x, silence_detection_mode: %d", 2, p_stream_info->streaming_flag, silence_detection_mode);
    if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE)) {
        config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = true;
    } else
#endif
    {
        //config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = false;
    }

    /*config USB source parameters*/
    line_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.line_in_param);
    bt_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param);

    line_in_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    line_in_param->codec_param.pcm.sample_rate = bt_ull_le_srv_get_usb_sample_rate(transmitter_type, BT_ULL_ROLE_SERVER);
    line_in_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
    line_in_param->codec_param.pcm.channel_mode = 0x2;
    line_in_param->codec_param.pcm.frame_interval = 1000;//
    ull_report("[ULL][LE][AUDIO_TRANS] init line_in_param done. sample_rate: %d, format: %d, channel_mode: %d, frame_interval: %d.", 4,
        line_in_param->codec_param.pcm.sample_rate,
        line_in_param->codec_param.pcm.format,
        line_in_param->codec_param.pcm.channel_mode,
        line_in_param->codec_param.pcm.frame_interval);

    /*config bt out param*/
    bt_ull_le_at_set_bt_out_param(transmitter_type, client_type, out_type, bt_out_param);

    /*Init callback handler*/
    config->msg_handler = bt_ull_le_at_linein_callback;
    config->user_data = NULL;
    bt_ull_le_at_print_bt_out_param_log(bt_out_param, 2);
}

static void bt_ull_le_at_linein_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_linein);
    bt_ull_le_at_result_t result_notify;
    result_notify.transmitter_type = BT_ULL_LINE_IN_TRANSMITTER;
    if (AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS != event) {
        ull_report("[ULL][LE][AUDIO_TRANS] audio_transmitter_linein_callback event = 0x%x, is_request_transmitter_start = 0x%x, flag:0x%x", 3,
            event, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][7] linein callback, silence_detection_mode: %d", 1, bt_ull_le_at_get_silence_detection_mode());
            if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == bt_ull_le_at_get_silence_detection_mode()) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            }
        #else
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
        #endif

            bt_ull_le_at_handle_linein_start_success_cnf();
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_linein_stop_success_or_start_fail_cnf();
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            if (0 == (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            }
        #else
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
        #endif
            bt_ull_le_at_handle_linein_stop_success_or_start_fail_cnf();
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();

}

static void bt_ull_le_at_handle_linein_start_success_cnf(void)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_linein);

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_handle_linein_start_success_cnf, transmitter_type: %d, is_request_transmitter_start: %d, flag: 0x%x.", 3,
            BT_ULL_LINE_IN_TRANSMITTER,p_stream_info->is_request_transmitter_start,p_stream_info->streaming_flag);

    p_stream_info->is_transmitter_start = true; //set flag, transmitter is started

    bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STARTING);//remove flag

    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        /*change sample rate case: should stop transmitter*/
        bt_ull_le_at_stop(BT_ULL_LINE_IN_TRANSMITTER, false);
    } else if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        /*need restart case, eg: change latency*/
        bt_ull_le_at_stop(BT_ULL_LINE_IN_TRANSMITTER, false);
    } else {
        if (!p_stream_info->is_request_transmitter_start) {
            bt_ull_le_at_stop(BT_ULL_LINE_IN_TRANSMITTER, false);
        } else {
            /*step1: check if need mixed or not*/

            /*check if need mix usb_in(gaming)*/
            if(trans_ctx->dl_spk.is_request_transmitter_start
              && trans_ctx->dl_spk.is_transmitter_start
              && (AUD_ID_INVALID != trans_ctx->dl_spk.transmitter)
              && !(trans_ctx->dl_spk.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                /*usb_in(gaming) is active, should mix usb_in(gaming)*/
                ull_report("[ULL][LE][AUDIO_TRANS] usb_in(gaming) is active, should mix both!!", 0);
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_spk.transmitter;
                if(AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, trans_ctx->dl_spk.transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_GAMING_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_sync_volume_info(BT_ULL_GAMING_TRANSMITTER);//gaming
                }
            }

            /*check if need mix usb_in(chat)*/
            if(trans_ctx->dl_chat.is_request_transmitter_start
              && trans_ctx->dl_chat.is_transmitter_start
              && (AUD_ID_INVALID != trans_ctx->dl_chat.transmitter)
              && !(trans_ctx->dl_chat.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                /*usb_in(chat) is active, should mix usb_in(chat)*/
                ull_report("[ULL][LE][AUDIO_TRANS] usb_in(chat) is active, should mix both!!", 0);
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_chat.transmitter;
                if(AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, trans_ctx->dl_chat.transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_CHAT_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_sync_volume_info(BT_ULL_CHAT_TRANSMITTER);//chat
                }
            }

            /*step2: mix self*/
            audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
            audio_transmitter_runtime_config_t runtime_config;
            runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = p_stream_info->transmitter;
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                ull_report_error("[ULL][LE][AUDIO_TRANS][Error] linein mixing self fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            } else {
                ull_report("[ULL][LE][AUDIO_TRANS] linein mixing self success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            }

            /*step3: set self volume*/
            bt_ull_le_at_sync_volume_info(BT_ULL_LINE_IN_TRANSMITTER);

        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][8] linein start success cnf, streaming_flag: 0x%x, is_silence_detetcion_on: %d", 2, p_stream_info->streaming_flag, p_stream_info->is_silence_detection_on);
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE) {
                bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
                p_stream_info->is_silence_detection_special_mode = true;
            }
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE) {
                bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
                p_stream_info->is_silence_detection_special_mode = false;
            }
            if (false == p_stream_info->is_silence_detection_on) {
                bt_ull_le_at_silence_detection_start_by_transmitter_type(BT_ULL_LINE_IN_TRANSMITTER);
                if (false == p_stream_info->is_silence_detection_special_mode) {
                    bt_ull_le_at_set_silence_flag(BT_ULL_LINE_IN_TRANSMITTER);//silence event is not occur, set bit mask
               }
            }
        #endif

        }
    }
}

static void bt_ull_le_at_handle_linein_stop_success_or_start_fail_cnf(void)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_linein);

    p_stream_info->is_transmitter_start = false;//set flag, transmitter is stop

    bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STARTING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);//remove flag
#ifdef AIR_SILENCE_DETECTION_ENABLE

    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
#endif
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_NEED_DEINIT) {
        bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_NEED_DEINIT);
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        p_stream_info->transmitter = -1;
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x, streaming_flag: 0x%x", 3, BT_ULL_LINE_IN_TRANSMITTER, status, p_stream_info->streaming_flag);
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        ull_report("[ULL][LE][SD_DEBUG][9] linein stop cnf, streaming_flag: 0x%x, silence_detection_mode: %d", 2, p_stream_info->streaming_flag, silence_detection_mode);
        if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE)) {
            bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE);
            bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
            bt_ull_le_at_silence_detection_start_special_mode(BT_ULL_LINE_IN_TRANSMITTER);
        } else if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
            bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE);
            p_stream_info->is_silence_detection_special_mode = false;
            bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
            bt_ull_le_at_silence_detection_start_normal_mode(BT_ULL_LINE_IN_TRANSMITTER);
        }
    #endif
    }

    /*if usb change sample rate, should reinit transmitter, when handle change sample rate, must set flag: BT_ULL_STREAMING_RECONFIG */
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RECONFIG);//remove flag  //
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x", 2, BT_ULL_LINE_IN_TRANSMITTER, status);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
            p_stream_info->transmitter = AUD_ID_INVALID;
            //bt_ull_le_at_init(trans_type, p_stream_info->codec_type, NULL))
            bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, BT_ULL_LINE_IN_TRANSMITTER, bt_ull_le_srv_get_codec_type());
        }
    }
    if(p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        bt_ull_le_at_remove_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);//remove restart flag
    }
    if (p_stream_info->is_request_transmitter_start) {
        /* 2-rx, we should wait for all transmitter stop, then start again */
        /* check gaming status*/
        ull_report("[ULL][LE][AUDIO_TRANS] check gaming port, streaming_flag: 0x%x, is_start: 0x%x", 2, trans_ctx->dl_spk.streaming_flag, trans_ctx->dl_spk.is_transmitter_start);
        if ((AUD_ID_INVALID != trans_ctx->dl_spk.transmitter)
            && trans_ctx->dl_spk.is_transmitter_start
            && (trans_ctx->dl_spk.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
            return;
        }

        /* check chat status*/
        ull_report("[ULL][LE][AUDIO_TRANS] check chat port, streaming_flag: 0x%x, is_start: 0x%x", 2, trans_ctx->dl_chat.streaming_flag, trans_ctx->dl_chat.is_transmitter_start);
        if ((AUD_ID_INVALID != trans_ctx->dl_chat.transmitter)
            && trans_ctx->dl_chat.is_transmitter_start
            && (trans_ctx->dl_chat.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
            return;
        }

        if (trans_ctx->dl_spk.is_request_transmitter_start) {
            bt_ull_le_at_start(BT_ULL_GAMING_TRANSMITTER, false);
        }

        if (trans_ctx->dl_chat.is_request_transmitter_start) {
            bt_ull_le_at_start(BT_ULL_CHAT_TRANSMITTER, false);
        }

        bt_ull_le_at_start(BT_ULL_LINE_IN_TRANSMITTER, false);
    }
#if 0
    if (bt_ull_le_at_silence_detection_check_need_start_special_mode()) {
        //all scenario is silence, should stop normal mode and start special mode
        ull_report("[ULL][LE][SD_DEBUG][16] need start special mode", 0);
        bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL);//set special mode
        bt_ull_le_at_silence_detection_stop_normal_mode();//all scenarios is silence, should stop audio transmitter
    }
#endif


}
#endif

#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)

static void bt_ull_le_at_config_i2sin_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        ull_audio_v2_dongle_i2s_mst_in_param_t *i2s_in_param;
        ull_audio_v2_dongle_bt_out_param_t *bt_out_param;
        config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
        config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0;
        /*config USB source parameters*/
        i2s_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.i2s_mst_in_param);
        bt_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param);
#elif (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        ull_audio_v2_dongle_i2s_slv_in_param_t *i2s_in_param;
        ull_audio_v2_dongle_bt_out_param_t *bt_out_param;
        config->scenario_type = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE;
        config->scenario_sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0;
        /*config USB source parameters*/
        i2s_in_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.i2s_slv_in_param);
        bt_out_param = &(config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param);
#endif
#ifdef AIR_SILENCE_DETECTION_ENABLE
        bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
        bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
        ull_report("[ULL][LE][SD_DEBUG][14] bt_ull_le_at_config_spk_transmitter_param, streaming_flag: 0x%x, silence_detection_mode: %d", 2, p_stream_info->streaming_flag, silence_detection_mode);
        if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE)) {
            config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = true;
        } else
#endif
        {
            //config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.without_bt_link_mode_enable = false;
        }

        i2s_in_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
        //i2s_in_param->codec_param.pcm.sample_rate = bt_ull_le_srv_get_usb_sample_rate(BT_ULL_CHAT_TRANSMITTER);
        i2s_in_param->codec_param.pcm.sample_rate = trans_ctx->i2sin_param.codec_param.pcm.sample_rate;
        i2s_in_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
        i2s_in_param->codec_param.pcm.channel_mode = 0x2;
        i2s_in_param->codec_param.pcm.frame_interval = 1000;//
        i2s_in_param->audio_device = trans_ctx->i2sin_param.audio_device;
        i2s_in_param->audio_interface = trans_ctx->i2sin_param.audio_interface;
        i2s_in_param->i2s_fromat = trans_ctx->i2sin_param.i2s_fromat;
        i2s_in_param->i2s_word_length = trans_ctx->i2sin_param.i2s_word_length;
        ull_report("[ULL][LE][AUDIO_TRANS] init i2s_in_param done. sample_rate: %d, format: %d, channel_mode: %d, frame_interval: %d, audio_device: %d, audio_interface: %d, i2s_fromat: %d, i2s_word_length: %d", 8,
            i2s_in_param->codec_param.pcm.sample_rate,
            i2s_in_param->codec_param.pcm.format,
            i2s_in_param->codec_param.pcm.channel_mode,
            i2s_in_param->codec_param.pcm.frame_interval,
            i2s_in_param->audio_device,
            i2s_in_param->audio_interface,
            i2s_in_param->i2s_fromat,
            i2s_in_param->i2s_word_length);

        /*config bt out param*/
        bt_ull_le_at_set_bt_out_param(transmitter_type, client_type, out_type, bt_out_param);

        /*Init callback handler*/
        config->msg_handler = bt_ull_le_at_i2sin_callback;
        config->user_data = NULL;
        bt_ull_le_at_print_bt_out_param_log(bt_out_param, 2);
}

static void bt_ull_le_at_i2sin_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_i2sin);
    bt_ull_le_at_result_t result_notify;
    result_notify.transmitter_type = BT_ULL_I2S_IN_TRANSMITTER;
    if (AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS != event) {
        ull_report("[ULL][LE][AUDIO_TRANS] audio_transmitter_gaming_callback event = 0x%x, is_request_transmitter_start = 0x%x, flag:0x%x", 3,
            event, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][10] i2s callback, silence_detection_mode: %d", 1, bt_ull_le_at_get_silence_detection_mode());
            if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == bt_ull_le_at_get_silence_detection_mode()) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            }
        #else
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
        #endif

            bt_ull_le_at_handle_i2sin_start_success_cnf();
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            bt_ull_le_at_handle_i2sin_stop_success_or_start_fail_cnf();
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
        #ifdef AIR_SILENCE_DETECTION_ENABLE
            if (0 == (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
                result_notify.result = BT_STATUS_SUCCESS;
                bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            }
        #else
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
        #endif

            bt_ull_le_at_handle_i2sin_stop_success_or_start_fail_cnf();
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();

}

static void bt_ull_le_at_handle_i2sin_start_success_cnf(void)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_i2sin);

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_handle_i2sin_start_success_cnf, transmitter_type: %d, is_request_transmitter_start: %d, flag: 0x%x.", 3,
        BT_ULL_I2S_IN_TRANSMITTER,p_stream_info->is_request_transmitter_start,p_stream_info->streaming_flag);

    p_stream_info->is_transmitter_start = true; //set flag, transmitter is started

    bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STARTING);//remove flag

    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        /*change sample rate case: should stop transmitter*/
        bt_ull_le_at_stop(BT_ULL_I2S_IN_TRANSMITTER, false);
    } else if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        /*need restart case, eg: change latency*/
        bt_ull_le_at_stop(BT_ULL_I2S_IN_TRANSMITTER, false);
    } else {
        if (!p_stream_info->is_request_transmitter_start) {
            bt_ull_le_at_stop(BT_ULL_I2S_IN_TRANSMITTER, false);
        } else {
            /*step1: check if need mixed or not*/

            /*check if need mix usb_in(gaming)*/
            if(trans_ctx->dl_spk.is_request_transmitter_start
              && trans_ctx->dl_spk.is_transmitter_start
              && (AUD_ID_INVALID != trans_ctx->dl_spk.transmitter)
              && !(trans_ctx->dl_spk.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                /*usb_in(gaming) is active, should mix usb_in(gaming)*/
                ull_report("[ULL][LE][AUDIO_TRANS] usb_in(gaming) is active, should mix both!!", 0);
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_spk.transmitter;
                if(AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, trans_ctx->dl_spk.transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_GAMING_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_sync_volume_info(BT_ULL_GAMING_TRANSMITTER);//gaming
                }
            }

            /*check if need mix usb_in(chat)*/
            if(trans_ctx->dl_chat.is_request_transmitter_start
              && trans_ctx->dl_chat.is_transmitter_start
              && (AUD_ID_INVALID != trans_ctx->dl_chat.transmitter)
              && !(trans_ctx->dl_chat.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
                /*usb_in(chat) is active, should mix usb_in(chat)*/
                ull_report("[ULL][LE][AUDIO_TRANS] usb_in(chat) is active, should mix both!!", 0);
                audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
                audio_transmitter_runtime_config_t runtime_config;
                runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = trans_ctx->dl_chat.transmitter;
                if(AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                    ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mix config fail, trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, trans_ctx->dl_chat.transmitter);
                } else {
                    ull_report("[ULL][LE][AUDIO_TRANS] mix config success!! trans_id:0x%x, mix_trans_id:0x%x", 2, p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
                    bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_set_flag(BT_ULL_CHAT_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);
                    bt_ull_le_at_sync_volume_info(BT_ULL_CHAT_TRANSMITTER);//chat
                }
            }

            /*step2: mix self*/
            audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX;
            audio_transmitter_runtime_config_t runtime_config;
            runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id = p_stream_info->transmitter;
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config)) {
                ull_report_error("[ULL][LE][AUDIO_TRANS][Error] i2sin mixing self fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            } else {
                ull_report("[ULL][LE][AUDIO_TRANS] i2sin mixing self success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                    p_stream_info->transmitter, runtime_config.ull_audio_v2_dongle_runtime_config.dl_mixer_id);
            }

            /*step3: set self volume*/
            bt_ull_le_at_sync_volume_info(BT_ULL_I2S_IN_TRANSMITTER);

        #ifdef AIR_SILENCE_DETECTION_ENABLE
            ull_report("[ULL][LE][SD_DEBUG][11] i2sin start success cnf, streaming_flag: 0x%x, is_silence_detetcion_on: %d", 2, p_stream_info->streaming_flag, p_stream_info->is_silence_detection_on);
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE) {
                bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
                p_stream_info->is_silence_detection_special_mode = true;
            }
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE) {
                bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
                p_stream_info->is_silence_detection_special_mode = false;
            }
            if (false == p_stream_info->is_silence_detection_on) {
                bt_ull_le_at_silence_detection_start_by_transmitter_type(BT_ULL_I2S_IN_TRANSMITTER);
                if (false == p_stream_info->is_silence_detection_special_mode) {
                    bt_ull_le_at_set_silence_flag(BT_ULL_I2S_IN_TRANSMITTER);//silence event is not occur, set bit mask
               }
            }
        #endif

        }
    }
}

static void bt_ull_le_at_handle_i2sin_stop_success_or_start_fail_cnf(void)
{
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = &(trans_ctx->dl_i2sin);

    p_stream_info->is_transmitter_start = false;//set flag, transmitter is stop

    bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_STARTING);//remove flag
    bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_MIXED);//remove flag
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
#endif

    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_NEED_DEINIT) {
        bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_NEED_DEINIT);
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        p_stream_info->transmitter = -1;
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x, streaming_flag: 0x%x", 3, BT_ULL_I2S_IN_TRANSMITTER, status, p_stream_info->streaming_flag);
    #ifdef AIR_SILENCE_DETECTION_ENABLE
        ull_report("[ULL][LE][SD_DEBUG][12] i2sin stop cnf, streaming_flag: 0x%x, silence_detection_mode: %d, is_request_transmitter_start: %d", 3,
            p_stream_info->streaming_flag, silence_detection_mode, p_stream_info->is_request_transmitter_start);
        if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE)) {
            bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE);
            bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE);
            bt_ull_le_at_silence_detection_start_special_mode(BT_ULL_I2S_IN_TRANSMITTER);
        } else if ((BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode) && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE)) {
            bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE);
            p_stream_info->is_silence_detection_special_mode = false;
            if (p_stream_info->is_request_transmitter_start) {
                bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_WAIT_START_NORMAL_MODE);
                bt_ull_le_at_silence_detection_start_normal_mode(BT_ULL_I2S_IN_TRANSMITTER);
            }
        }
    #endif
    }

    /*if usb change sample rate, should reinit transmitter, when handle change sample rate, must set flag: BT_ULL_STREAMING_RECONFIG */
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RECONFIG);//remove flag  //
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit, transmitter_type: 0x%x, status: 0x%x", 2, BT_ULL_I2S_IN_TRANSMITTER, status);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
            p_stream_info->transmitter = AUD_ID_INVALID;
            //bt_ull_le_at_init(trans_type, p_stream_info->codec_type, NULL))
            bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, BT_ULL_I2S_IN_TRANSMITTER, bt_ull_le_srv_get_codec_type());
        }
    }
    if(p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RESTART) {
        bt_ull_le_at_remove_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);//remove restart flag
    }
    if (p_stream_info->is_request_transmitter_start) {
        /* 2-rx, we should wait for all transmitter stop, then start again */
        /* check gaming status*/
        ull_report("[ULL][LE][AUDIO_TRANS] check gaming port, streaming_flag: 0x%x, is_start: 0x%x", 2, trans_ctx->dl_spk.streaming_flag, trans_ctx->dl_spk.is_transmitter_start);
        if ((AUD_ID_INVALID != trans_ctx->dl_spk.transmitter)
           && trans_ctx->dl_spk.is_transmitter_start
           && (trans_ctx->dl_spk.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
           //ull_report("[ULL][LE][AUDIO_TRANS] gaming is stopping, we need wait it stop.", 0);
           return;
        }

        /* check chat status*/
        ull_report("[ULL][LE][AUDIO_TRANS] check chat port, streaming_flag: 0x%x, is_start: 0x%x", 2, trans_ctx->dl_chat.streaming_flag, trans_ctx->dl_chat.is_transmitter_start);
        if ((AUD_ID_INVALID != trans_ctx->dl_chat.transmitter)
          && trans_ctx->dl_chat.is_transmitter_start
          && (trans_ctx->dl_chat.streaming_flag & BT_ULL_LE_STREAMING_STOPPING)) {
          return;
        }

        if (trans_ctx->dl_spk.is_request_transmitter_start) {
            bt_ull_le_at_start(BT_ULL_GAMING_TRANSMITTER, false);
        }

        if (trans_ctx->dl_chat.is_request_transmitter_start) {
            bt_ull_le_at_start(BT_ULL_CHAT_TRANSMITTER, false);
        }

        bt_ull_le_at_start(BT_ULL_I2S_IN_TRANSMITTER, false);
    }
#if 0
    if (bt_ull_le_at_silence_detection_check_need_start_special_mode()) {
        //all scenario is silence, should stop normal mode and start special mode
        ull_report("[ULL][LE][SD_DEBUG][17] need start special mode", 0);
        bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL);//set special mode
        bt_ull_le_at_silence_detection_stop_normal_mode();//all scenarios is silence, should stop audio transmitter
    }
#endif


}
#endif
#if (defined AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
bt_status_t bt_ull_le_at_set_streaming_param(bt_ull_interface_config_t *param)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    //bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = NULL;
    bt_ull_transmitter_t transmitter_type = BT_ULL_TRANSMITTER_MAX_NUM;
    bt_ull_le_srv_audio_out_t out_type = BT_ULL_LE_SRV_AUDIO_OUT_AUX;
    if (BT_ULL_STREAMING_INTERFACE_I2S_IN == param->streaming.streaming_interface) {
    #if (defined AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        p_stream_info = &(trans_ctx->dl_i2sin);
        transmitter_type = BT_ULL_I2S_IN_TRANSMITTER;
        out_type = BT_ULL_LE_SRV_AUDIO_OUT_AUX;
        trans_ctx->i2sin_param.audio_device = param->i2s_in_dongle_config.audio_device;
        trans_ctx->i2sin_param.audio_interface = param->i2s_in_dongle_config.audio_interface;
        trans_ctx->i2sin_param.codec_param.pcm.sample_rate = param->i2s_in_dongle_config.codec_param.pcm.sample_rate;
        trans_ctx->i2sin_param.i2s_fromat = param->i2s_in_dongle_config.i2s_fromat;
        trans_ctx->i2sin_param.i2s_word_length = param->i2s_in_dongle_config.i2s_word_length;
        ull_report("[ULL][LE][AUDIO_TRANS] ull set line_i2s param audio_device:%d, audio_interface:%d, sample_rate:%d, i2s_fromat:%d, i2s_word_length:%d", 5,
          trans_ctx->i2sin_param.audio_device,trans_ctx->i2sin_param.audio_interface,trans_ctx->i2sin_param.codec_param.pcm.sample_rate,trans_ctx->i2sin_param.i2s_fromat,trans_ctx->i2sin_param.i2s_word_length);
    #endif
    } else if (BT_ULL_STREAMING_INTERFACE_I2S_OUT == param->streaming.streaming_interface) {
    #if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE))
        p_stream_info = &(trans_ctx->ul_i2sout);
        transmitter_type = BT_ULL_I2S_OUT_TRANSMITTER;
        out_type = BT_ULL_LE_SRV_AUDIO_OUT_I2S;
        trans_ctx->i2sout_param.audio_device = param->i2s_in_dongle_config.audio_device;
        trans_ctx->i2sout_param.audio_interface = param->i2s_in_dongle_config.audio_interface;
        trans_ctx->i2sout_param.codec_param.pcm.sample_rate = param->i2s_in_dongle_config.codec_param.pcm.sample_rate;
        trans_ctx->i2sout_param.i2s_fromat = param->i2s_in_dongle_config.i2s_fromat;
        trans_ctx->i2sout_param.i2s_word_length = param->i2s_in_dongle_config.i2s_word_length;
        ull_report("[ULL][LE][AUDIO_TRANS] ull set line_i2s param audio_device:%d, audio_interface:%d, sample_rate:%d, i2s_fromat:%d, i2s_word_length:%d", 5,
          trans_ctx->i2sout_param.audio_device,trans_ctx->i2sout_param.audio_interface,trans_ctx->i2sout_param.codec_param.pcm.sample_rate,trans_ctx->i2sout_param.i2s_fromat,trans_ctx->i2sout_param.i2s_word_length);
    #endif
    } else {
        ull_assert(0 && "!!bt_ull_le_at_set_streaming_param fail, type not support!");
    }
      if (bt_ull_le_service_is_connected()) {
          if (AUD_ID_INVALID != p_stream_info->transmitter) {
              if (false == p_stream_info->is_transmitter_start) {
              /* transmitter is starting/stopping */
              if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING
                  || p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING) {
                  /* reconfig after start success */
                  bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
              } else {
                  /* transmitter is stopped */
                  audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);
                   ull_report("[ULL][LE][AUDIO_TRANS] transmitter deinit :0x%x", 1, status);
                  if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                      p_stream_info->transmitter = AUD_ID_INVALID;
                      //bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_I2S_IN_TRANSMITTER);
                      bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
                  }
              }
          } else {
              if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING
                  || p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING) {
                  /* transmitter is starting/stopping */
                  bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
              } else {
                  /* transmitter is started */
                  bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
                  //bt_ull_stop_transmitter(ep);
                  bt_ull_le_at_stop(transmitter_type, false);
              }
          }
        } else {
        /* init transmitter info */
        //bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_I2S_IN_TRANSMITTER);
        bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
      }
    }
   return status;
}
#endif

#ifdef AIR_WIRELESS_MIC_ENABLE
static void bt_ull_le_at_wirelss_mic_print_bt_in_param_log(wireless_mic_rx_bt_in_param_t *bt_in_param, uint8_t link_num)
{
    //TODO audio_codec_lc3plus_t* lc3plus =  bt_in_param->link_param[i].codec_param.lc3plus
    uint8_t i;
    for (i = 0; i < link_num; i++) {
        ull_report("[ULL][LE][AUDIO_TRANS] init wireless mic transmitter bt in link_%d done.sample_rate: %d, sample_format: %d, bit_rate: %d, channel_mode: %d, frame_interval: %d, frame_size: %d.", 7, i,
        bt_in_param->link_param[i].codec_param.lc3plus.sample_rate,
        bt_in_param->link_param[i].codec_param.lc3plus.sample_format,
        bt_in_param->link_param[i].codec_param.lc3plus.bit_rate,
        bt_in_param->link_param[i].codec_param.lc3plus.channel_mode,
        bt_in_param->link_param[i].codec_param.lc3plus.frame_interval,
        bt_in_param->link_param[i].codec_param.lc3plus.frame_size);
    }
}
static void bt_ull_le_at_wireless_mic_set_bt_out_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, wireless_mic_rx_bt_in_param_t *bt_in_param, uint8_t link_num)
{
    //TODO
    bt_in_param->link_num = link_num;// case1: 1<->2  case2: 1<->4
    bt_ull_le_codec_t codec_type = bt_ull_le_srv_get_codec_type();
    ull_report("[ULL][LE][AUDIO_TRANS][DEBUG] bt_ull_le_at_wireless_mic_set_bt_out_param, codec_type: %d", 1, codec_type);
    uint8_t i;
    for (i = 0; i < bt_in_param->link_num; i++) {
        bt_bd_addr_t addr;
        bt_ull_le_srv_get_connected_addr_by_link_index(i, &addr);
        ull_report("[ULL][LE][AUDIO_TRANS][DEBUG] bt_ull_le_at_wireless_mic_set_bt_out_param, i = %d, addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, i,
            addr[5],addr[4],addr[3],addr[2],addr[1],addr[0]);
        bt_in_param->link_param[i].enable = true;
        bt_in_param->link_param[i].share_info = (void *)bt_ull_le_srv_get_avm_share_buffer_address(client_type, out_type, transmitter_type, i);
        if (BT_ULL_LE_CODEC_LC3PLUS == codec_type) {
        bt_in_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
        bt_in_param->link_param[i].codec_param.lc3plus.sample_rate = bt_ull_le_srv_get_codec_sample_rate(transmitter_type, true, BT_ULL_ROLE_SERVER);
            bt_in_param->link_param[i].codec_param.lc3plus.sample_format = HAL_AUDIO_PCM_FORMAT_S24_LE;
        bt_in_param->link_param[i].codec_param.lc3plus.bit_rate = bt_ull_le_srv_get_bitrate(true, BT_ULL_ROLE_SERVER);
        bt_in_param->link_param[i].codec_param.lc3plus.channel_mode = bt_ull_le_srv_get_channel_mode(2, true, BT_ULL_ROLE_SERVER);
        bt_in_param->link_param[i].codec_param.lc3plus.frame_interval =  bt_ull_le_srv_get_sdu_interval(true, BT_ULL_ROLE_SERVER);
        bt_in_param->link_param[i].codec_param.lc3plus.frame_size = bt_ull_le_srv_get_sdu_size(true, BT_ULL_ROLE_SERVER);
        } else if (BT_ULL_LE_CODEC_ULD == codec_type) {
            bt_in_param->link_param[i].codec_type = AUDIO_DSP_CODEC_TYPE_ULD;
            bt_in_param->link_param[i].codec_param.uld.sample_rate = bt_ull_le_srv_get_codec_sample_rate(transmitter_type, true, BT_ULL_ROLE_SERVER);
            bt_in_param->link_param[i].codec_param.uld.sample_format = HAL_AUDIO_PCM_FORMAT_S24_LE;
            bt_in_param->link_param[i].codec_param.uld.bit_rate = bt_ull_le_srv_get_bitrate(true, BT_ULL_ROLE_SERVER);
            bt_in_param->link_param[i].codec_param.uld.channel_mode = bt_ull_le_srv_get_channel_mode(2, true, BT_ULL_ROLE_SERVER);
            bt_in_param->link_param[i].codec_param.uld.frame_interval =  bt_ull_le_srv_get_sdu_interval(true, BT_ULL_ROLE_SERVER);
            bt_in_param->link_param[i].codec_param.uld.frame_size = bt_ull_le_srv_get_sdu_size(true, BT_ULL_ROLE_SERVER);
        } else {
            ull_assert(0 && "wireless mic codec type is error!!");
        }
        bt_in_param->link_param[i].bt_address_l = * ((uint32_t *)&addr[0]);
        bt_in_param->link_param[i].bt_address_h = * ((uint16_t *)&addr[4]);
    }
}

#if 0
static void bt_ull_le_at_config_wireless_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_AUX == out_type) {
        if (BT_ULL_MIC_CLIENT == client_type) {
        #ifdef AIR_WIRELESS_MIC_ENABLE
            config->scenario_type = AUDIO_TRANSMITTER_WIRELESS_MIC_RX;
            config->scenario_sub_id = AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT;
            wireless_mic_rx_bt_in_param_t *bt_in_param;
            wireless_mic_rx_line_out_param_t *line_out_param;
            bt_in_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param);
            line_out_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.line_out_param);

            /*config line out parameter*/
            line_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
            line_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
            line_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
            line_out_param->codec_param.pcm.channel_mode = 0x2;
            line_out_param->codec_param.pcm.frame_interval = 1000;
            uint8_t link_num = bt_ull_le_srv_get_air_cis_count();
            bt_ull_le_at_wireless_mic_set_bt_out_param(transmitter_type, client_type, out_type, bt_in_param, link_num);
            /*Init callback handler*/
            config->msg_handler = bt_ull_le_at_wireless_mic_callback;
            config->user_data = (void *)transmitter_type;
            bt_ull_le_at_wirelss_mic_print_bt_in_param_log(bt_in_param, link_num);
        #else
           ull_report_error("[ULL][LE][AUDIO_TRANS] init wireless mic param fail, the option is disabled", 0);
        #endif
        }
    } else {
        ull_assert(0 && "the out_type is not BT_ULL_LE_TRANSMITTER_AUDIO_OUT_AUX");
    }
}
#endif
static void bt_ull_le_at_config_wireless_mic_transmitter_param(bt_ull_transmitter_t transmitter_type, bt_ull_client_t client_type, uint8_t out_type, audio_transmitter_config_t* config)
{
    if (BT_ULL_MIC_CLIENT == client_type) {
    #ifdef AIR_WIRELESS_MIC_ENABLE
        config->scenario_type = AUDIO_TRANSMITTER_WIRELESS_MIC_RX;
        wireless_mic_rx_bt_in_param_t *bt_in_param;
        bt_ull_le_at_uplink_type at_uplink_type = 0;
        bt_in_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param);
        switch (transmitter_type) {
            case BT_ULL_LINE_OUT_TRANSMITTER: {
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_AUX == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_LINEOUT;
                    config->scenario_sub_id = AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT;
                    wireless_mic_rx_line_out_param_t *line_out_param;
                    line_out_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.line_out_param);
                    line_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    line_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    line_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
                    line_out_param->codec_param.pcm.channel_mode = 0x2;
                    line_out_param->codec_param.pcm.frame_interval = 1000;
                }
                break;
            }
            case BT_ULL_I2S_OUT_TRANSMITTER: {
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_I2S == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_I2SOUT;
                    config->scenario_sub_id = AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0;
                    wireless_mic_rx_i2s_slv_out_param_t *i2s_slv_out_param;
                    i2s_slv_out_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.i2s_slv_out_param);
                    i2s_slv_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    i2s_slv_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    i2s_slv_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
                    i2s_slv_out_param->codec_param.pcm.channel_mode = 0x2;
                    i2s_slv_out_param->codec_param.pcm.frame_interval = 1000;
                }
                break;
            }
            case BT_ULL_MIC_TRANSMITTER: {
                if (BT_ULL_LE_TRANSMITTER_AUDIO_OUT_USB == out_type) {
                    at_uplink_type = BT_ULL_LE_AT_UPLINK_USBOUT;
                    config->scenario_sub_id = AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0;
                    wireless_mic_rx_usb_out_param_t *usb_out_param;
                    usb_out_param = &(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param);
                    usb_out_param->codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
                    usb_out_param->codec_param.pcm.sample_rate = 48000;//bt_ull_le_srv_get_usb_sample_rate(transmitter_type);
                    usb_out_param->codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
                    usb_out_param->codec_param.pcm.channel_mode = 0x2;
                    usb_out_param->codec_param.pcm.frame_interval = 1000;
                }
                break;
            }
            default:
                break;
        }
        uint8_t link_num = bt_ull_le_srv_get_air_cis_count();
        bt_ull_le_at_wireless_mic_set_bt_out_param(transmitter_type, client_type, out_type, bt_in_param, link_num);
        /*Init callback handler*/
        config->msg_handler = bt_ull_le_at_wireless_mic_callback;
        config->user_data = (void *)at_uplink_type;
        bt_ull_le_at_wirelss_mic_print_bt_in_param_log(bt_in_param, link_num);
    #else
        ull_report_error("[ULL][LE][AUDIO_TRANS] init wireless mic param fail, the option is disabled", 0);
    #endif
    }
}


static void bt_ull_le_at_wireless_mic_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_transmitter_t trans_type = BT_ULL_TRANSMITTER_MAX_NUM;
    bt_ull_le_at_uplink_type at_uplink_type = (bt_ull_le_at_uplink_type)user_data;
    bt_ull_le_at_result_t result_notify;
    trans_type = (BT_ULL_LE_AT_UPLINK_LINEOUT == at_uplink_type)?(BT_ULL_LINE_OUT_TRANSMITTER):((BT_ULL_LE_AT_UPLINK_I2SOUT == at_uplink_type)?(BT_ULL_I2S_OUT_TRANSMITTER):(BT_ULL_MIC_TRANSMITTER));
    result_notify.transmitter_type = trans_type;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(trans_type);
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_wireless_mic_callback event = 0x%x, transmitter_type = %d, is_request_transmitter_start = 0x%x, flag:0x%x", 4,
        event, trans_type, p_stream_info->is_request_transmitter_start, p_stream_info->streaming_flag);
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            bt_ull_le_at_handle_wireless_mic_start_success_cnf(trans_type);
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            bt_ull_le_at_handle_wireless_mic_stop_success_or_start_fail_cnf(trans_type);
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_START_IND, &result_notify, sizeof(result_notify));
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            bt_ull_le_at_handle_wireless_mic_stop_success_or_start_fail_cnf(trans_type);
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_at_event_callback(BT_ULL_LE_AT_EVENT_STOP_IND, &result_notify, sizeof(result_notify));
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_at_handle_wireless_mic_start_success_cnf(bt_ull_transmitter_t transmitter_type)
{
    //bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    //bt_ull_le_at_info_t *p_stream_info_lineout = &(trans_ctx->ul_lineout);
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_handle_wireless_mic_start_success_cnf, transmitter_type: %d, flag: 0x%x.", 2, BT_ULL_LINE_OUT_TRANSMITTER, p_stream_info_lineout->streaming_flag);
    p_stream_info->is_transmitter_start = true;//set flag,transmitter is start
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag
    //need notify uplayer?
    //refresh all bt link address
    bt_ull_le_at_start_success_refresh_all_bt_link_adress(transmitter_type);
    /* usb sample rate, should reinit transmitter */
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
        bt_ull_le_at_stop(transmitter_type, false);
    } else {
        if (!p_stream_info->is_request_transmitter_start) {
            bt_ull_le_at_stop(transmitter_type, false);
        } else {
            /*step1: set volume*/
            bt_ull_le_at_sync_volume_info(transmitter_type);
            /*step2: config mic channel*/
        }
    }
}

static void bt_ull_le_at_handle_wireless_mic_stop_success_or_start_fail_cnf(bt_ull_transmitter_t transmitter_type)
{
    //print log
    //bt_ull_le_at_info_t *p_stream_info_lineout = &(g_ull_le_at_ctx.ul_lineout);
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    p_stream_info->is_transmitter_start = false;
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STOPPING);//remove flag
    bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_STARTING);//remove flag
    //need notify uplayer?

    /* usb sample rate, should reinit transmitter */
    if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_RECONFIG) {
        bt_ull_le_at_remove_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);//remove flag
        audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);//deinit
        ull_report("[ULL][LE][AUDIO_TRANS] lineout transmitter deinit :0x%x", 1, status);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
            p_stream_info->transmitter = AUD_ID_INVALID;
            //bt_ull_le_at_init(transmitter_type, p_stream_info_mic->codec_type, void * codec_info));
            bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
            p_stream_info->is_request_transmitter_start = true;//set flag, request transmitter start
        }
    }
    /* check last user request restart transmitter or not */
    if (p_stream_info->is_request_transmitter_start) {
        bt_ull_le_at_start(transmitter_type, false);
    }
}
#endif
static bool bt_ull_le_at_type_is_uplink(bt_ull_transmitter_t transmitter_type)
{
    return ((BT_ULL_MIC_TRANSMITTER == transmitter_type) || (BT_ULL_LINE_OUT_TRANSMITTER == transmitter_type) || (BT_ULL_I2S_OUT_TRANSMITTER == transmitter_type));
}

#endif

bt_status_t bt_ull_le_at_set_mute(bt_ull_transmitter_t transmitter_type, bool is_mute)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_ull_le_at_info_t *p_stream_info = NULL;
    audio_transmitter_status_t ret;
    bt_ull_client_t client_type = bt_ull_le_srv_get_client_type();
    bt_ull_le_at_context_t *trans_ctx = bt_ull_le_at_get_context();
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mute, transmitter_type: %d, is_mute: %d, is_transmitter_start: %d, client_type: %d", 4, transmitter_type, is_mute, p_stream_info->is_transmitter_start, client_type);
    p_stream_info->is_mute = is_mute;
    if ((AUD_ID_INVALID != p_stream_info->transmitter) && p_stream_info->is_transmitter_start) {
        audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_MAX;
        audio_transmitter_runtime_config_t runtime_config = {0};
        //memset(&config, 0x00, sizeof(config));
        if (bt_ull_le_at_type_is_uplink(transmitter_type)) {
            if (BT_ULL_MIC_CLIENT == client_type) {
            #ifdef AIR_WIRELESS_MIC_ENABLE
                 config_op = WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_LEVEL;
            #else
                ull_report_error("[ULL][LE][AUDIO_TRANS] the option - AIR_WIRELESS_MIC_ENABLE is not open.", 0);
            #endif
            } else if (BT_ULL_HEADSET_CLIENT == client_type \
                || BT_ULL_EARBUDS_CLIENT == client_type \
                || BT_ULL_SPEAKER_CLIENT == client_type) {
            config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_VOL_LEVEL;
            }
        } else if(BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
            config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_VOL_LEVEL;
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[0].ratio;
        } else if(BT_ULL_CHAT_TRANSMITTER == transmitter_type) {
            config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_VOL_LEVEL;
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[1].ratio;
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        else if (BT_ULL_LINE_IN_TRANSMITTER == transmitter_type) {
            config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_VOL_LEVEL;
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[2].ratio;
        }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        else if (BT_ULL_I2S_IN_TRANSMITTER == transmitter_type) {
            config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_VOL_LEVEL;
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ratio = trans_ctx->dl_mix_ratio.streamings[3].ratio;
        }
#endif
        else {
            return BT_STATUS_FAIL;
        }

        /* we set streaming both L and R channel volume to 0 when mute */
        if (is_mute) {
            if (BT_ULL_MIC_CLIENT == client_type) {
            #ifdef AIR_WIRELESS_MIC_ENABLE
                runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_level = AUD_VOL_OUT_LEVEL0;
                runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_ch = BT_ULL_AUDIO_CHANNEL_DUAL;
            #endif
            } else if (BT_ULL_HEADSET_CLIENT == client_type \
                || BT_ULL_EARBUDS_CLIENT == client_type \
                || BT_ULL_SPEAKER_CLIENT == client_type) {
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_level = AUD_VOL_OUT_LEVEL0;
            runtime_config.ull_audio_v2_dongle_runtime_config.vol_info.vol_ch = BT_ULL_AUDIO_CHANNEL_DUAL;
            }
            ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config);
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                ull_report_error("[ULL][LE][AUDIO_TRANS][Error] mute operation, audio_transmitter_set_runtime_config fail", 0);
                return BT_STATUS_FAIL;
            } else {
                ull_report("[ULL][LE][AUDIO_TRANS] mute success!!", 0);
            }
        } else {
            if (BT_ULL_MIC_CLIENT == client_type) {
            #ifdef AIR_WIRELESS_MIC_ENABLE
                //runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_level = p_stream_info->volume.level;
                //runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_ch = p_stream_info->volume.audio_channel;
                bt_ull_le_at_sync_volume_info(transmitter_type);
            #endif
            } else if (BT_ULL_HEADSET_CLIENT == client_type \
                || BT_ULL_EARBUDS_CLIENT == client_type \
                || BT_ULL_SPEAKER_CLIENT == client_type) {
                bt_ull_le_at_sync_volume_info(transmitter_type);
            }
            ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                ull_report_error("[ULL][LE][AUDIO_TRANS][Error] unmute operation, audio_transmitter_set_runtime_config fail", 0);
                return BT_STATUS_FAIL;
            } else {
                ull_report("[ULL][LE][AUDIO_TRANS] unmute success!!", 0);
            }
        }
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] skip mute or unmute due to streaming is not start!", 0);
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mute, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;
#endif
}

bt_sink_srv_am_volume_level_out_t bt_ull_le_at_get_volume_level(uint8_t volume)
{
    /** From %(0 ~ 100) to AUD_VOL_OUT_XX(0~15) */
    float local_level_f = AUD_VOL_OUT_LEVEL0;
    float orignal_f = volume;
    bt_sink_srv_am_volume_level_t max_lev = AUD_VOL_OUT_LEVEL15;

    local_level_f = (orignal_f * max_lev) / BT_ULL_USB_VOLUME_MAX + 0.5f;
    ull_report("[ULL][LE][AUDIO_TRANS] get_volume_level[s]-orignal: %d, vol_level: %d", 2, volume, (uint8_t)local_level_f);
    return (uint8_t)local_level_f;
}
bt_status_t bt_ull_le_at_set_ul_channel_locaton(uint32_t location)
{
    bt_status_t status = BT_STATUS_FAIL;
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_ull_le_at_info_t *p_stream_info = NULL;
    audio_transmitter_runtime_config_type_t config_op = ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_CH_INPUT_SOURCE;
    audio_transmitter_runtime_config_t runtime_config;
    runtime_config.ull_audio_v2_dongle_runtime_config.connection_info.ch_choose = BT_ULL_LE_AT_SELECT_1_CHANNEL;
    if (BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT < location) {
        return BT_STATUS_FAIL;
    }

    runtime_config.ull_audio_v2_dongle_runtime_config.connection_info.ch_connection[BT_ULL_LE_AT_SELECT_1_CHANNEL - 1] = (BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT == location) ? 0x01 : 0x02;
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_ul_channel_locaton, location: %d", 1, location);

    if (NULL != (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_MIC_TRANSMITTER))) {
        if ((AUD_ID_INVALID != p_stream_info->transmitter)) {
            /*set location*/
            status = audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config);
        }
    }
    if (NULL != (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_LINE_OUT_TRANSMITTER))) {
        if ((AUD_ID_INVALID != p_stream_info->transmitter)) {
            /*set location*/
            status = audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config);
        }
    }
    if (NULL != (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_I2S_OUT_TRANSMITTER))) {
        if ((AUD_ID_INVALID != p_stream_info->transmitter)) {
            /*set location*/
            status = audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config);
        }
    }

#endif
    return status;
}

bt_status_t bt_ull_le_at_set_volume(bt_ull_transmitter_t transmitter_type, bt_ull_volume_action_t action,bt_ull_audio_channel_t channel, uint8_t volume)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    //bt_status_t status = BT_STATUS_SUCCESS;
    bool is_vol_update = false;
    bt_ull_le_at_info_t *p_stream_info = NULL;

    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }

    //special handler for wireless mic
    if (BT_ULL_MIC_CLIENT == bt_ull_le_srv_get_client_type() && (BT_ULL_MIC_TRANSMITTER == transmitter_type)) {
        if (!g_ull_le_at_ctx.ul_mic.is_transmitter_start && g_ull_le_at_ctx.ul_lineout.is_transmitter_start) {
            //transmitter_type == BT_ULL_LINE_OUT_TRANSMITTER;
            p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_LINE_OUT_TRANSMITTER);
        }
    }

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_volume, transmitter_type: %d, action: %d, channel: %d, volume: %d, origin_volume: %d", 5, transmitter_type, action, channel, volume, p_stream_info->volume_info.volume[channel].level);

    if(BT_ULL_AUDIO_CHANNEL_RIGHT < channel) {
        ull_assert(0 && "unknown audio channel");
    }
    //set is_dual_channel flag
    if (BT_ULL_AUDIO_CHANNEL_DUAL == channel) {
        p_stream_info->volume_info.is_dual_channel_set = true;
    } else {
        p_stream_info->volume_info.is_dual_channel_set = false;
    }
    p_stream_info->volume_info.latest_channel_set = channel;
    uint8_t volume_temp = p_stream_info->volume_info.volume[channel].vol;
    if (BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME == action) {
        p_stream_info->volume_info.volume[channel].vol = volume;
    } else if (BT_ULL_VOLUME_ACTION_SET_UP == action) {
        volume_temp = ((volume_temp+7 < 100) ? (volume_temp+7) : 100);
        p_stream_info->volume_info.volume[channel].vol = volume_temp;
    } else if (BT_ULL_VOLUME_ACTION_SET_DOWN == action) {
        if (volume_temp < 7) {
            volume_temp = 0;
        } else {
            volume_temp = volume_temp - 7;
        }
        p_stream_info->volume_info.volume[channel].vol = volume_temp;
    } else {
        ull_assert(0 && "[ULL][LE][AUDIO_TRANS] unknown action on server!");
    }
    //store volume info by channel
    if (p_stream_info->volume_info.volume[channel].level!= bt_ull_le_at_get_volume_level(p_stream_info->volume_info.volume[channel].vol)) {
        p_stream_info->volume_info.volume[channel].audio_channel = channel;
        p_stream_info->volume_info.volume[channel].level = bt_ull_le_at_get_volume_level(p_stream_info->volume_info.volume[channel].vol);
        is_vol_update = true;
    }
    if (is_vol_update) {
        /*if volume changed, should update DSP transmitter volume*/
        bt_ull_le_at_config_volume(transmitter_type, channel);
    }

    return BT_STATUS_SUCCESS;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_volume, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;

#endif
}

bt_status_t bt_ull_le_at_restart(bt_ull_transmitter_t transmitter_type)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_status_t status = BT_STATUS_SUCCESS;
    //uint8_t sr_switch = 0x00;
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_restart, transmitter_id:0x%x, is_transmitter_start: 0x%x, streaming_flag:0x%x.", 3,
        p_stream_info->transmitter, p_stream_info->is_transmitter_start, p_stream_info->streaming_flag);

    /*if transmitter id is not invalid, should deinit*/
    if (AUD_ID_INVALID != p_stream_info->transmitter) {
        if (!p_stream_info->is_transmitter_start) {
            /*if transmitter is starting or stopping, should set reconfig flag and wait*/
            if ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
                || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
            } else {
                /*transmitter is stoped, deinit transmitter directly*/
                audio_transmitter_status_t status = audio_transmitter_deinit(p_stream_info->transmitter);
                ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_restart, transmitter deinit :0x%x", 1, status);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    p_stream_info->transmitter = AUD_ID_INVALID;
                    bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
                }
            }
        } else {
            /*transmitter is started, should set reconfig flag and stop transmitter*/
            if (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING
                || p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING) {
                /*transmitter is stopping, should set reconfig flag*/
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
            } else {
                /*transmitter is in streaming, should set reconfig flag and stop transmitter directly*/
            #ifdef AIR_SILENCE_DETECTION_ENABLE
                if (p_stream_info->is_silence_detection_special_mode) {
                    bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_REINIT_IN_SPECIAL_MODE);
                }
            #endif
                bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_RECONFIG);
                bt_ull_le_at_stop(transmitter_type, false);
            }
        }
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_restart, transmitter id is invalid!!", 0);
    }
    return status;
#else
    return BT_STATUS_FAIL;
#endif
}

bool bt_ull_le_at_is_start(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return false;
    }
    return p_stream_info->is_transmitter_start;
}

bool bt_ull_le_at_is_any_transmitter_start(bt_ull_role_t role)
{
    uint8_t transmitter_type;
    bt_ull_le_at_info_t* p_stream_info = NULL;

    if (BT_ULL_ROLE_SERVER == role) {
        for (transmitter_type = 0; transmitter_type < BT_ULL_TRANSMITTER_MAX_NUM; transmitter_type++) {
            if (NULL != (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
                if (p_stream_info->is_transmitter_start) {
                 ull_report("[ULL][LE] bt_ull_le_at_is_any_transmitter_start, Yes! Server role, trans_type is %d.", 1, transmitter_type);
                 return true;
               }
            }
        }
    }
    ull_report_error("[ULL][LE] bt_ull_le_at_is_any_transmitter_start, No!", 0);
    return false;
}



bt_status_t bt_ull_le_at_set_mix_ratio(bt_ull_mix_ratio_t *ratio)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    if (ratio) {
        uint8_t idx = 0;
        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
            ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
                idx, ratio->streamings[idx].streaming.streaming_interface, ratio->streamings[idx].streaming.port, ratio->streamings[idx].ratio);
            if (BT_ULL_STREAMING_INTERFACE_SPEAKER == ratio->streamings[idx].streaming.streaming_interface && 0 == ratio->streamings[idx].streaming.port) {
                memcpy(&(trans_ctx->dl_mix_ratio.streamings[0]), &(ratio->streamings[idx]), sizeof(bt_ull_ratio_t));
            } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == ratio->streamings[idx].streaming.streaming_interface && 1 == ratio->streamings[idx].streaming.port) {
                memcpy(&(trans_ctx->dl_mix_ratio.streamings[1]), &(ratio->streamings[idx]), sizeof(bt_ull_ratio_t));
            } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == ratio->streamings[idx].streaming.streaming_interface) {
                memcpy(&(trans_ctx->dl_mix_ratio.streamings[2]), &(ratio->streamings[idx]), sizeof(bt_ull_ratio_t));
            } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == ratio->streamings[idx].streaming.streaming_interface) {
                memcpy(&(trans_ctx->dl_mix_ratio.streamings[3]), &(ratio->streamings[idx]), sizeof(bt_ull_ratio_t));
            }
        }
        /*set gaming transmitter mix*/
        if (trans_ctx->dl_spk.is_transmitter_start) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_sync_volume_info(BT_ULL_GAMING_TRANSMITTER)) {
                status = BT_STATUS_FAIL;
            }
        }
        /*set chat transmitter mix*/
        if (trans_ctx->dl_chat.is_transmitter_start) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_sync_volume_info(BT_ULL_CHAT_TRANSMITTER)) {
                status = BT_STATUS_FAIL;
            }
        }
        #ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        /*set linein transmitter mix*/
        if (trans_ctx->dl_linein.is_transmitter_start) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_sync_volume_info(BT_ULL_LINE_IN_TRANSMITTER)) {
            }
        }
        #endif
        /*set i2sin transmitter mix*/
        #if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        if (trans_ctx->dl_i2sin.is_transmitter_start) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_sync_volume_info(BT_ULL_I2S_IN_TRANSMITTER)) {
            }
        }
        #endif
    }else{
        status = BT_STATUS_FAIL;
        ull_assert(0 && "ratio is NULL!");
    }

    return status;
#else
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_mix_ratio, the ULL V2 option is disabled", 0);
    return BT_STATUS_FAIL;

#endif
}

bt_ull_le_at_context_t* bt_ull_le_at_get_context(void)
{
    return &g_ull_le_at_ctx;
}

void bt_ull_le_at_set_latency(uint16_t ul_latency, uint16_t dl_latency)
{
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = NULL;
    ull_report("[ULL][LE][AUDIO_TRANS] dl_spk: %d, dl_chat: %d, ul_mic: %d.", 3, trans_ctx->dl_spk.is_transmitter_start, trans_ctx->dl_chat.is_transmitter_start, trans_ctx->ul_mic.is_transmitter_start);

    /*restart gaming transmitter*/
    if (trans_ctx->dl_spk.is_transmitter_start) {
        p_stream_info = &(trans_ctx->dl_spk);
        ull_report("[ULL][LE][AUDIO_TRANS] set gaming latency handler, flag: 0x%x.", 1, p_stream_info->streaming_flag);
        if ((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
            || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
            /*transmitter is stopping or starting, should set reconfig flag and wait*/
            bt_ull_le_at_set_flag(BT_ULL_GAMING_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
        } else {
            /*transmitter is in streaming, should set reconfig flag and stop transmitter */
            bt_ull_le_at_set_flag(BT_ULL_GAMING_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
            bt_ull_le_at_stop(BT_ULL_GAMING_TRANSMITTER, false);
        }
    }

    /*restart chat transmitter*/
    if (trans_ctx->dl_chat.is_transmitter_start) {
        p_stream_info = &(trans_ctx->dl_chat);
        ull_report("[ULL][LE][AUDIO_TRANS] set chat latency handler, flag: 0x%x.", 1, p_stream_info->streaming_flag);
        if((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
            || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
            /*transmitter is stopping or starting, should set reconfig flag and wait*/
            bt_ull_le_at_set_flag(BT_ULL_CHAT_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
        } else {
            /*transmitter is in streaming, should set reconfig flag and stop transmitter */
            bt_ull_le_at_set_flag(BT_ULL_CHAT_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
            bt_ull_le_at_stop(BT_ULL_CHAT_TRANSMITTER, false);
        }
    }
#ifndef AIR_WIRELESS_MIC_ENABLE
    /*restart mic transmitter*/
    if (trans_ctx->ul_mic.is_transmitter_start) {
        p_stream_info = &(trans_ctx->ul_mic);
        ull_report("[ULL][LE][AUDIO_TRANS] set mic latency handler, flag: 0x%x.", 1, p_stream_info->streaming_flag);
        if((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
            || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
            /*transmitter is stopping or starting, should set reconfig flag and wait*/
            bt_ull_le_at_set_flag(BT_ULL_MIC_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
        } else {
            /*transmitter is in streaming, should set reconfig flag and stop transmitter */
            bt_ull_le_at_set_flag(BT_ULL_MIC_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
            bt_ull_le_at_stop(BT_ULL_MIC_TRANSMITTER, false);
        }
    }
#endif

#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        /*restart linein transmitter*/
        if (trans_ctx->dl_linein.is_transmitter_start) {
            p_stream_info = &(trans_ctx->dl_linein);
            ull_report("[ULL][LE][AUDIO_TRANS] set linein latency handler, flag: 0x%x.", 1, p_stream_info->streaming_flag);
            if((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
                || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
                /*transmitter is stopping or starting, should set reconfig flag and wait*/
                bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
            } else {
                /*transmitter is in streaming, should set reconfig flag and stop transmitter */
                bt_ull_le_at_set_flag(BT_ULL_LINE_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
                bt_ull_le_at_stop(BT_ULL_LINE_IN_TRANSMITTER, false);
            }
        }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        /*restart chat transmitter*/
        if (trans_ctx->dl_i2sin.is_transmitter_start) {
            p_stream_info = &(trans_ctx->dl_i2sin);
            ull_report("[ULL][LE][AUDIO_TRANS] set i2sin latency handler, flag: 0x%x.", 1, p_stream_info->streaming_flag);
            if((p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STOPPING)
                || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
                /*transmitter is stopping or starting, should set reconfig flag and wait*/
                bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
            } else {
                /*transmitter is in streaming, should set reconfig flag and stop transmitter */
                bt_ull_le_at_set_flag(BT_ULL_I2S_IN_TRANSMITTER, BT_ULL_LE_STREAMING_RESTART);
                bt_ull_le_at_stop(BT_ULL_I2S_IN_TRANSMITTER, false);
            }
        }
#endif

    if(!(trans_ctx->dl_spk.is_transmitter_start) && !(trans_ctx->dl_chat.is_transmitter_start)) {
        ull_report("[ULL][LE][AUDIO_TRANS] set chat latency handler, no action because gaming and chat transmitter is not start!!", 0);
    }
}

static void bt_ull_le_at_set_flag(bt_ull_transmitter_t transmitter_type, bt_ull_le_transmitter_flag_t mask)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!!");
    }
    p_stream_info->streaming_flag |= mask;
    ull_report("[ULL][LE][AUDIO_TRANS] set flag: 0x%x, trans_type: %d, current streaming flag: 0x%x", 3, mask, transmitter_type, p_stream_info->streaming_flag);

}


bt_status_t bt_ull_le_at_read_pcm_data(bt_ull_transmitter_t transmitter_type, uint8_t *data, uint32_t *length)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if ((p_stream_info) && (p_stream_info->is_transmitter_start)) {
        status = audio_transmitter_read_data(p_stream_info->transmitter, data, length);
    }
    return (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
#else
    return BT_STATUS_FAIL;
#endif
}

bt_status_t bt_ull_le_at_write_pcm_data(bt_ull_transmitter_t transmitter_type, uint8_t *data, uint32_t *length)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if ((p_stream_info) && (p_stream_info->is_transmitter_start)) {
        status = audio_transmitter_write_data(p_stream_info->transmitter, data, length);
    }
    return (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
#else
    return BT_STATUS_FAIL;
#endif
}

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
bt_status_t bt_ull_le_at_set_audio_connection_info(bt_ull_transmitter_t transmitter_type, void *audio_conenction_info, uint32_t size)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_audio_connection_info, transmitter_type: %d, transmitter_id: 0x%x.", 2, transmitter_type, p_stream_info->transmitter);
    audio_transmitter_runtime_config_t runtime_config;
    runtime_config.wireless_mic_rx_runtime_config.connection_info.info = audio_conenction_info;
    runtime_config.wireless_mic_rx_runtime_config.connection_info.size = size;
    if (AUD_ID_INVALID != p_stream_info->transmitter && (AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_set_runtime_config(p_stream_info->transmitter, WIRELESS_MIC_RX_CONFIG_OP_SET_UL_CONNECTION_INFO, &runtime_config))) {
        status = BT_STATUS_SUCCESS;
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_audio_connection_info, fail", 0);
    }
    return status;
}

bt_status_t bt_ull_le_at_update_client_addr(bt_ull_transmitter_t transmitter_type, uint8_t link_index, bt_bd_addr_t *client_adress)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }
    bt_bd_addr_t addr;
    bt_ull_le_srv_memcpy(&addr, client_adress, sizeof(bt_bd_addr_t));
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_update_connect_client_addr, transmitter_type: %d, is_transmitter_start: %d, transmitter_id: 0x%x, new_connected_client_adress: %02x:%02x:%02x:%02x:%02x:%02x, link_index: %d", 10,
        transmitter_type, p_stream_info->is_transmitter_start, p_stream_info->transmitter,
        addr[5],
        addr[4],
        addr[3],
        addr[2],
        addr[1],
        addr[0],
        link_index);
    if ((AUD_ID_INVALID != p_stream_info->transmitter) && (p_stream_info->is_transmitter_start)) {
        //runtime config new connected client addr
        //bt_bd_addr_t addr;
        //bt_ull_le_srv_memcpy(&addr, client_adress, sizeof(bt_bd_addr_t));
        audio_transmitter_runtime_config_t runtime_config;
        runtime_config.wireless_mic_rx_runtime_config.bt_address_info.bt_ch = link_index;
        runtime_config.wireless_mic_rx_runtime_config.bt_address_info.bt_address_l = *((uint32_t *)&addr[0]);
        runtime_config.wireless_mic_rx_runtime_config.bt_address_info.bt_address_h = *((uint16_t *)&addr[4]);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(p_stream_info->transmitter, WIRELESS_MIC_RX_CONFIG_OP_SET_UL_BT_ADDRESS, &runtime_config) ){
            status = BT_STATUS_FAIL;
        } else {
            status = BT_STATUS_SUCCESS;
        }
    }
    return status;
}

static bt_status_t bt_ull_le_at_start_success_refresh_all_bt_link_adress(bt_ull_transmitter_t transmitter_type)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t link_num = bt_ull_le_srv_get_air_cis_count();
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_start_success_refresh_all_bt_link_adress, transmitter_type: %d, link_num: %d", 2, transmitter_type, link_num);
    uint8_t i;
    for (i = 0; i < link_num; i++) {
        bt_bd_addr_t link_addr;
        bt_ull_le_srv_memcpy(&link_addr, &(g_ull_le_at_ctx.wireleaa_mic_client_link_info[i].client_addr), sizeof(bt_bd_addr_t));
        //bt_ull_le_srv_get_connected_addr_by_link_index(i, &link_addr);
        if (BT_STATUS_SUCCESS != bt_ull_le_at_update_client_addr(transmitter_type, i, &link_addr)) {
            status = BT_STATUS_FAIL;
            break;
        }
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_start_success_refresh_all_bt_link_adress, result: %d", 1, status);
    return status;
}

bt_status_t bt_ull_le_at_set_wireless_mic_safety_mode_volume_by_channel(bt_ull_transmitter_t transmitter_type, S32 left_vol_diff, S32 right_vol_diff)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_wireless_mic_safety_mode_volume_by_channel, transmitter_type: %d, left_vol_diff: %d, right_vol_diff: %d.", 3, transmitter_type, left_vol_diff, right_vol_diff);
    audio_transmitter_runtime_config_type_t config_op = WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_OFFSET;
    audio_transmitter_runtime_config_t runtime_config;
    //ull_assert(right_vol_diff);
    runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_gain_offset = left_vol_diff;
    runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_ch = BT_ULL_AUDIO_CHANNEL_LEFT;
    if (AUD_ID_INVALID != p_stream_info->transmitter && (AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config))) {
        status = BT_STATUS_SUCCESS;
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_audio_connection_info, fail", 0);
    }
    runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_gain_offset = right_vol_diff;
    runtime_config.wireless_mic_rx_runtime_config.vol_info.vol_ch = BT_ULL_AUDIO_CHANNEL_RIGHT;
    if (AUD_ID_INVALID != p_stream_info->transmitter && (AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_set_runtime_config(p_stream_info->transmitter, config_op, &runtime_config))) {
        status = BT_STATUS_SUCCESS;
    } else {
        ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_audio_connection_info, fail", 0);
    }


    return status;
}

static void bt_ull_le_at_reset_wireless_mic_client_link_info(void)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    while (i > 0) {
        i--;
        bt_ull_le_srv_memset(&(g_ull_le_at_ctx.wireleaa_mic_client_link_info[i]), 0x00, sizeof(bt_ull_le_at_wireless_mic_client_link_info));
    }
    ull_report("[ULL][LE][AUDIO_TRANS]reset_wireless_mic_tx_link_info, YES!", 0);
}

void bt_ull_le_at_set_wireless_mic_client_link_info(uint8_t link_index, bt_bd_addr_t *client_adress)
{
    bt_bd_addr_t addr;
    bt_ull_le_srv_memcpy(&addr, client_adress, sizeof(bt_bd_addr_t));
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_wireless_mic_client_link_info, link_index: %d, new_connected_client_adress: %02x:%02x:%02x:%02x:%02x:%02x", 7,
        link_index, addr[5],addr[4],addr[3],addr[2],addr[1],addr[0]);
    g_ull_le_at_ctx.wireleaa_mic_client_link_info[link_index].link_index = link_index;
    bt_ull_le_srv_memcpy(&(g_ull_le_at_ctx.wireleaa_mic_client_link_info[link_index].client_addr), client_adress, sizeof(bt_bd_addr_t));
}

void bt_ull_le_at_clear_wireless_mic_client_link_info_by_addr(bt_bd_addr_t *client_adress)
{
    bt_bd_addr_t addr;
    uint8_t i;
    bt_ull_le_srv_memcpy(&addr, client_adress, sizeof(bt_bd_addr_t));
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_clear_wireless_mic_client_link_info, client_adress: %02x:%02x:%02x:%02x:%02x:%02x", 6,
        addr[5],addr[4],addr[3],addr[2],addr[1],addr[0]);
    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
        if (0 == (bt_ull_le_srv_memcmp(&(g_ull_le_at_ctx.wireleaa_mic_client_link_info[i]), client_adress, sizeof(bt_bd_addr_t)))) {
            //bt_ull_le_at_update_client_addr(bt_ull_transmitter_t transmitter_type,uint8_t link_index,bt_bd_addr_t * client_adress)
            bt_bd_addr_t addr;
            bt_ull_le_srv_memset(&addr, 0, sizeof(bt_bd_addr_t));
            bt_ull_le_at_set_wireless_mic_client_link_info(i, &addr);
            break;
        }
    }
}

uint8_t bt_ull_le_at_get_volume_value(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!!");
    }
    return p_stream_info->volume_info.volume[BT_ULL_AUDIO_CHANNEL_DUAL].vol;
}

#endif

#if (defined AIR_ULL_AUDIO_V2_DONGLE_ENABLE) && (defined AIR_SILENCE_DETECTION_ENABLE)
static void bt_ull_le_at_set_silence_flag(bt_ull_transmitter_t transmitter_type)
{
    switch (transmitter_type) {
        case BT_ULL_GAMING_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag |= BT_ULL_LE_AT_GAMING_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_CHAT_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag |= BT_ULL_LE_AT_CHAT_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_LINE_IN_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag |= BT_ULL_LE_AT_LINEIN_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_I2S_IN_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag |= BT_ULL_LE_AT_I2SIN_IS_NOT_SILENCE;
            break;
        }
        default: {
            //ull_assert(0 && "invalid audio transmiter type!");
            break;
        }

    }
    ull_report("[ULL][LE][AUDIO_TRANS] set silence flag, trans_type: %d, current silence flag: 0x%x", 2, transmitter_type, g_ull_le_at_ctx.ull_le_at_silence_detection_flag);
}

static void bt_ull_le_at_remove_silence_flag(bt_ull_transmitter_t transmitter_type)
{
    switch (transmitter_type) {
        case BT_ULL_GAMING_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag &= ~BT_ULL_LE_AT_GAMING_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_CHAT_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag &= ~BT_ULL_LE_AT_CHAT_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_LINE_IN_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag &= ~BT_ULL_LE_AT_LINEIN_IS_NOT_SILENCE;
            break;
        }
        case BT_ULL_I2S_IN_TRANSMITTER: {
            g_ull_le_at_ctx.ull_le_at_silence_detection_flag &= ~BT_ULL_LE_AT_I2SIN_IS_NOT_SILENCE;
            break;
        }
        default: {
            //ull_assert(0 && "invalid audio transmiter type!");
            break;
        }
    }
    ull_report("[ULL][LE][AUDIO_TRANS] remove silence flag, trans_type: %d, current silence flag: 0x%x", 2, transmitter_type, g_ull_le_at_ctx.ull_le_at_silence_detection_flag);
}

static audio_scenario_type_t bt_ull_le_at_get_audio_scenario_by_at_type(bt_ull_transmitter_t transmitter_type)
{
    audio_scenario_type_t audio_scenario = AUDIO_SCENARIO_TYPE_COMMON;
    switch (transmitter_type) {
        case BT_ULL_CHAT_TRANSMITTER: {
            audio_scenario = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
            break;
        }
        case BT_ULL_GAMING_TRANSMITTER: {
            audio_scenario = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1;
            break;
        }
        case BT_ULL_LINE_IN_TRANSMITTER: {
            audio_scenario = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN;
            break;
        }
        case BT_ULL_I2S_IN_TRANSMITTER: {
            audio_scenario = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0;
            break;
        }
        default: {
            //ull_assert(0 && "invalid audio transmiter type!");
            break;
        }
    }
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_get_audio_scenario_by_at_type. get audio_scenario: %d.", 1, audio_scenario);
    return audio_scenario;
}

static bt_ull_transmitter_t bt_ull_le_at_get_type_by_audio_scenario_type(audio_scenario_type_t scenario_type)
{
    bt_ull_transmitter_t at_type = BT_ULL_TRANSMITTER_MAX_NUM;
    switch (scenario_type) {
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0: {
            at_type = BT_ULL_CHAT_TRANSMITTER;
            break;
        }
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1: {
            at_type = BT_ULL_GAMING_TRANSMITTER;
            break;
        }
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN: {
            at_type = BT_ULL_LINE_IN_TRANSMITTER;
            break;
        }
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0: {
            at_type = BT_ULL_I2S_IN_TRANSMITTER;
            break;
        }
        default: {
            //ull_assert(0 && "invalid scenario_type!");
            break;
        }
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_get_type_by_audio_scenario_type. get at_type: %d.", 1, at_type);
    return at_type;
}

bt_ull_le_at_silence_detection_mode_t bt_ull_le_at_get_silence_detection_mode(void)
{
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_get_silence_detection_mode. mode: %d.", 1, g_ull_le_at_ctx.ull_le_at_silence_detection_mode);
    return g_ull_le_at_ctx.ull_le_at_silence_detection_mode;
}

static void bt_ull_le_at_set_silence_detection_mode(bt_ull_le_at_silence_detection_mode_t mode)
{
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_set_silence_detection_mode. mode: %d.", 1, mode);
    g_ull_le_at_ctx.ull_le_at_silence_detection_mode = mode;
}

static bool bt_ull_le_at_type_is_downlink(bt_ull_transmitter_t transmitter_type)
{
    return ((BT_ULL_GAMING_TRANSMITTER == transmitter_type) || (BT_ULL_CHAT_TRANSMITTER == transmitter_type));
}

static void bt_ull_le_at_silence_detection_start_by_transmitter_type(bt_ull_transmitter_t transmitter_type)
{
    ull_report("[ULL][LE][AUDIO_TRANS] start silence detection, transmitter_type: %d", 1, transmitter_type);
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "invalid audio transmiter type!");
    }
    audio_scenario_type_t audio_scenario = bt_ull_le_at_get_audio_scenario_by_at_type(transmitter_type);
    audio_silence_detection_scenario_start(audio_scenario, bt_ull_le_at_silence_detection_callback);
    //ull_report("[ULL][LE][SD_DEBUG][API] audio_silence_detection_scenario_start, audio_scenario: %d", 1, audio_scenario);
    p_stream_info->is_silence_detection_on = true;//set flag which means that silence detection is on.
    //bt_ull_le_at_set_silence_flag(transmitter_type);//silence event is not occur, set bit mask
}

static void bt_ull_le_at_silence_detection_stop_by_transmitter_type(bt_ull_transmitter_t transmitter_type)
{
    ull_report("[ULL][LE][AUDIO_TRANS] stop silence detection, transmitter_type: %d", 1, transmitter_type);
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "invalid audio transmiter type!");
    }
    audio_scenario_type_t audio_scenario = bt_ull_le_at_get_audio_scenario_by_at_type(transmitter_type);
    audio_silence_detection_scenario_stop(audio_scenario);
    //ull_report("[ULL][LE][SD_DEBUG][API] audio_silence_detection_scenario_stop, audio_scenario: %d", 1, audio_scenario);
    p_stream_info->is_silence_detection_on = false;//set flag which means that silence detection is on.
    //bt_ull_le_at_remove_silence_flag(transmitter_type);
}
static void bt_ull_le_at_silence_detection_stop_normal_mode(void)
{
    bt_ull_transmitter_t i;
    for (i = BT_ULL_GAMING_TRANSMITTER; i < BT_ULL_MIC_TRANSMITTER; i++) {
        if (bt_ull_le_at_type_is_downlink(i)) {
            bt_ull_le_at_info_t *p_stream_info = NULL;
            if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(i))) {
                ull_assert(0 && "p_stream_info is NULL!");
            }
            ull_report("[ULL][LE][AUDIO_TRANS] silence detection stop normal mode, transmitter_type: %d, is_request_transmitter_start: %d, is_transmitter_start: %d", 3,
                i, p_stream_info->is_request_transmitter_start, p_stream_info->is_transmitter_start);
            if (p_stream_info->is_transmitter_start) {
                bt_ull_le_at_set_flag(i, BT_ULL_LE_STREAMING_WAIT_STOP_NORMAL_MODE);//should tell each cenario need stop normal  mode.
                bt_ull_le_at_deinit(i, false);
                //bt_ull_le_at_silence_detection_stop_by_transmitter_type(i);//disable silence detetcion.
            }
        }
    }
}

static void bt_ull_le_at_silence_detection_stop_special_mode(bt_ull_transmitter_t transmitter_type)
{
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_stop_special_mode. transmitter_type: %d", 1, transmitter_type);
    bt_ull_le_at_set_flag(transmitter_type, BT_ULL_LE_STREAMING_WAIT_STOP_SPECIAL_MODE);//set stop special  mode flag
    bt_ull_le_at_deinit(transmitter_type, false);//deinit special type
}

void bt_ull_le_at_silence_detection_handler(bt_ull_transmitter_t transmitter_type, bool is_silence_detected)
{
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if (!p_stream_info) {
        return;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_handler. is_silence_detected: %d, silence_flag: 0x%x, is_silence_detection_special_mode: %d, streaming_port: 0x%x", 4,
        is_silence_detected, trans_ctx->ull_le_at_silence_detection_flag, p_stream_info->is_silence_detection_special_mode, stream_ctx->streaming_port);
    if (is_silence_detected ) {
        if ((false == p_stream_info->is_silence_detection_special_mode)) {
            //detected silence
            bt_ull_le_at_remove_silence_flag(transmitter_type);//silence event is occured, clear bit mask
            if (bt_ull_le_at_silence_detection_is_uplink_streaming()) {
                ull_report("[ULL][LE][AUDIO_TRANS][SD_DEBUG] uplink is streaming, not handle silence event!!", 0);
                return;
            }
            if (0 == trans_ctx->ull_le_at_silence_detection_flag) {
                //all scenario is silence, should stop normal mode and start special mode
                bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL);//set special mode
                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
                    bt_ull_le_srv_silence_detection_notify_client_status(true, BT_ULL_GAMING_TRANSMITTER);//notify client stop am
                }
                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
                    bt_ull_le_srv_silence_detection_notify_client_status(true, BT_ULL_CHAT_TRANSMITTER);//notify client stop am
                }
                bt_ull_le_at_silence_detection_stop_normal_mode();//all scenarios is silence, should stop audio transmitter
            }
        }
    } else {
        bt_ull_le_at_set_silence_flag(transmitter_type);
        //detected data
        if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
            bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL);//set normal mode
        }
        if (true == p_stream_info->is_silence_detection_special_mode) {
            //bt_ull_le_at_silence_detection_stop_special_mode(transmitter_type);
            bt_ull_le_at_silence_detection_check_stop_all_special_mode();
        }
    }
}

static void bt_ull_le_at_silence_detection_start_normal_mode(bt_ull_transmitter_t transmitter_type)
{
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_start_normal_mode. transmitter_type: %d", 1, transmitter_type);
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!");
    }
    if (BT_STATUS_SUCCESS == bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type())) {
        bt_ull_le_at_start(transmitter_type, false);
    }
}
static void bt_ull_le_at_silence_detection_start_special_mode(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_le_at_info_t *p_stream_info = NULL;
    if (NULL == (p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type))) {
        ull_assert(0 && "p_stream_info is NULL!");
    }
    if (BT_STATUS_SUCCESS == bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type())) {
        bt_ull_le_at_start(transmitter_type, false);
    }
}
#if 0
static bool bt_ull_le_at_silence_detection_check_need_start_special_mode(void)
{
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    bt_ull_le_at_silence_detection_mode_t silence_detection_mode = bt_ull_le_at_get_silence_detection_mode();
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_check_need_start_special_mode. silence_detection_flag: %d, silence_detection_mode: %d", 2,
        trans_ctx->ull_le_at_silence_detection_flag, silence_detection_mode);
    if (bt_ull_le_at_silence_detection_is_uplink_streaming()) {
        if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
            bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL);
        }
        return false;
    }
    if ((0 == trans_ctx->ull_le_at_silence_detection_flag) && (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode)) {
        bt_ull_transmitter_t i;
        for (i = BT_ULL_GAMING_TRANSMITTER; i < BT_ULL_MIC_TRANSMITTER; i++) {
            if (bt_ull_le_at_type_is_downlink(i)) {
                bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(i);
                if (p_stream_info && p_stream_info->is_silence_detection_on && p_stream_info->is_transmitter_start) {
                    return true;
                }
            }
        }
    }
    if (BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode && (!bt_ull_le_at_silence_detection_is_spk_in_silence())) {
        bt_ull_le_at_set_silence_detection_mode(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL);
    }
    return false;
}
#endif

static void bt_ull_le_at_silence_detection_check_stop_all_special_mode(void)
{
    bt_ull_transmitter_t i;
    for (i = BT_ULL_GAMING_TRANSMITTER; i < BT_ULL_MIC_TRANSMITTER; i++) {
        if (bt_ull_le_at_type_is_downlink(i)) {
            bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(i);
            if (p_stream_info && ((true == p_stream_info->is_silence_detection_special_mode && true == p_stream_info->is_transmitter_start)
                || (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_WAIT_START_SPECIAL_MODE))) {
                ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_check_stop_all_special_mode. should stop special mode, transmitter_type: %d", 1, i);
                bt_ull_le_at_silence_detection_stop_special_mode(i);
            }
        }
    }
}

void bt_ull_le_at_silence_detection_callback(audio_scenario_type_t scenario_type, bool silence_flag)
{
    //ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_callback. scenario_type: %d, silence_flag: %d", 2, scenario_type, silence_flag);
    bt_ull_transmitter_t transmitter_type = bt_ull_le_at_get_type_by_audio_scenario_type(scenario_type);
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_callback. scenario_type: %d, silence_flag: %d, trans_type: %d", 3, scenario_type, silence_flag, transmitter_type);

    if (silence_flag) {
        //detect silence
        bt_ull_le_at_silence_detection_handler(transmitter_type, true);
    } else {
        //detect data (not silence)
        bt_ull_le_at_silence_detection_handler(transmitter_type, false);
    }
}

static bool bt_ull_le_at_silence_detection_is_uplink_streaming(void)
{
    bool is_uplink_streaming = false;
    bt_ull_le_at_context_t* trans_ctx = bt_ull_le_at_get_context();
    if (trans_ctx->ul_mic.is_transmitter_start || trans_ctx->ul_lineout.is_transmitter_start || trans_ctx->ul_i2sout.is_transmitter_start) {
        is_uplink_streaming = true;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_silence_detection_is_uplink_streaming. is_uplink_streaming: %d", 1, is_uplink_streaming);
    return is_uplink_streaming;
}

void bt_ull_le_at_stop_silence_detection(void)
{
    uint32_t type = 0;
    for (type = BT_ULL_GAMING_TRANSMITTER; type < BT_ULL_MIC_TRANSMITTER; type++) {
       bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(type);
       if(BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL == bt_ull_le_at_get_silence_detection_mode() && p_stream_info->is_silence_detection_on) {
          bt_ull_le_at_silence_detection_stop_by_transmitter_type(type);
       }
    }
}

#if 0
static bool bt_ull_le_at_silence_detection_is_spk_in_silence(void)
{
    bt_ull_le_at_info_t *p_stream_info_gaming = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_GAMING_TRANSMITTER);
    bt_ull_le_at_info_t *p_stream_info_chat = bt_ull_le_at_get_stream_info_by_transmitter_type(BT_ULL_CHAT_TRANSMITTER);
    if (p_stream_info_gaming->is_silence_detection_special_mode
        || p_stream_info_chat->is_silence_detection_special_mode ) {
        return true;
    }
    return false;
}
#endif
#endif

bool bt_ull_le_at_is_prepare_start(bt_ull_transmitter_t transmitter_type)
{
    bool is_at_prepare_start = false;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if (p_stream_info && (p_stream_info->streaming_flag & BT_ULL_LE_STREAMING_STARTING)) {
        is_at_prepare_start = true;
    }
    ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_is_prepare_start. is_at_prepare_start: %d", 1, is_at_prepare_start);
    return is_at_prepare_start;
}

bt_status_t bt_ull_le_at_reinit( bt_ull_transmitter_t transmitter_type)
{
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_at_info_t *p_stream_info = bt_ull_le_at_get_stream_info_by_transmitter_type(transmitter_type);
    if (p_stream_info) {
        if (AUD_ID_INVALID == p_stream_info->transmitter) {
            status = bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
        } else if (!p_stream_info->is_transmitter_start && (AUD_ID_INVALID != p_stream_info->transmitter)) {
            /*transmitter is stoped, deinit transmitter directly*/
            audio_transmitter_status_t ret = audio_transmitter_deinit(p_stream_info->transmitter);
            ull_report("[ULL][LE][AUDIO_TRANS] bt_ull_le_at_reinit, transmitter deinit :0x%x", 1, ret);
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS == ret) {
                p_stream_info->transmitter = AUD_ID_INVALID;
                status = bt_ull_le_at_init(bt_ull_le_srv_get_client_type(), p_stream_info->out_type, transmitter_type, bt_ull_le_srv_get_codec_type());
            }
        }
    }
    return status;
#else
    return BT_STATUS_FAIL;
#endif
}
