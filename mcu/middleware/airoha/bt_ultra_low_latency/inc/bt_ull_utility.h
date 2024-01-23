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

#ifndef __BT_ULL_UTILITY_H__
#define __BT_ULL_UTILITY_H__
#include "bt_type.h"
#include "audio_src_srv.h"
#include "bt_a2dp.h"
#include "bt_ull_audio_manager.h"
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#ifndef WIN32
#include <syslog.h>
#else
#include "osapi.h"
#endif
#include "bt_avm.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_control_port.h"
#endif


BT_EXTERN_C_BEGIN

//#if defined(MTK_DEBUG_LEVEL_INFO)
#define __ULL_DEBUG_INFO__
#define __ULL_TRACE__
//#endif

#define ull_assert configASSERT

#ifndef WIN32
#ifdef __ULL_TRACE__
#define ull_report(_message, art_cnt,...) LOG_MSGID_I(ULL, _message, art_cnt, ##__VA_ARGS__)
#define ull_report_error(_message, art_cnt,...) LOG_MSGID_E(ULL, _message, art_cnt, ##__VA_ARGS__)
#define ull_hex_dump(_message,...)   LOG_HEXDUMP_I(ULL, (_message), ##__VA_ARGS__)
#else
#define ull_report(_message,...);
#define ull_report_error(_message,...);
#define ull_hex_dump(_message,...);
#endif
#else
#define ull_report(_message,...) OS_Report((_message), ##__VA_ARGS__)
#define ull_report_error(_message,...) OS_Report((_message), ##__VA_ARGS__)
#define ull_hex_dump(_message,...) OS_Report((_message), ##__VA_ARGS__)
#endif /* WIN32 */

extern void bt_os_take_stack_mutex(void);
extern void bt_os_give_stack_mutex(void);
#define BT_ULL_MUTEX_LOCK() bt_os_take_stack_mutex()
#define BT_ULL_MUTEX_UNLOCK() bt_os_give_stack_mutex()


#define BT_ULL_SET_FLAG(FLAG, MASK) do { \
    (FLAG) |= (MASK); \
} while(0);

#define BT_ULL_REMOVE_FLAG(FLAG, MASK) do { \
    (FLAG) &= ~(MASK); \
} while(0);

#define BT_ULL_CODEC_AIRO_CELT (0x65) /**< OPUS codec. If you want to change this value, must sync with audio team */

#define BT_ULL_ROLE_CLIENT_UUID    0x00,0x00,0xFC,0x01,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFF    /**< the ULL client UUID.*/
#define BT_ULL_ROLE_SERVER_ID                     (0x09)

#define BT_ULL_DISCONNECTED                 (0x00)
#define BT_ULL_CONNECTED                    (0x01)
typedef uint8_t bt_ull_connection_state_t;

#define BT_ULL_EVENT_UNKNOWN                (0x00)
/* dongle <-> headset */
#define BT_ULL_EVENT_CONNECTION_INFO        (0x01)
/* dongle -> headset */
#define BT_ULL_EVENT_STREAMING_START_IND    (0x02)
#define BT_ULL_EVENT_STREAMING_STOP_IND     (0x03)
#define BT_ULL_EVENT_VOLUME_IND             (0x04)
/* dongle <-> headset */
#define BT_ULL_EVENT_VOLUME_MUTE            (0x05)
#define BT_ULL_EVENT_VOLUME_UNMUTE          (0x06)
/* headset -> dongle */
#define BT_ULL_EVENT_LATENCY_SWITCH         (0x07)
#define BT_ULL_EVENT_ALLOW_PLAY_REPORT      (0x08)
/* dongle -> headset */
#define BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE    (0x09)
/* headset -> dongle */
#define BT_ULL_EVENT_VOLUME_ACTION          (0x0A)
#define BT_ULL_EVENT_MIX_RATIO_ACTION       (0x0B)
/* user data (dongle <-> headset) */
#define BT_ULL_EVENT_USER_DATA              (0x0C)
/* headset -> dongle */
#define BT_ULL_EVENT_USB_HID_CONTROL_ACTION (0x0D)
/* dongle -> headset */
#define BT_ULL_EVENT_UPLINK_INFO            (0x0E)


#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
/* dongle <-> headset */
#define BT_ULL_EVENT_COORDINATED_SET_INFO   (0x0E)
/* dongle <-> headset */
#define BT_ULL_EVENT_LE_STREAM_CODEC_INFO   (0x0F)
/* dongle -> headset */
#define BT_ULL_EVENT_SAMPLE_RATE_CHANGE     (0x10)

#define BT_ULL_EVENT_SEND_CONFIGURATION     (0x11)

#define BT_ULL_EVENT_ACTIVATE_UPLINK_IND    (0x12)

#define BT_ULL_EVENT_RESTART_STREAMING_IND  (0x13)

#define BT_ULL_EVENT_AUDIO_QUALITY_CHANGE_IND  (0x14)

#define BT_ULL_EVENT_SWITCH_UPLINK_IND      (0x15)

#define BT_ULL_EVENT_UPDATE_VOLUME_IND      (0x16)

#define BT_ULL_EVENT_SCENARIO_IND           (0x17)

/* ULL call state event, dongle -> headset*/
#define BT_ULL_EVENT_CALL_STATE             (0x30)
/* ULL call action, headset -> dongle*/
#define BT_ULL_EVENT_CALL_ACTION            (0x31)

#define BT_ULL_EVENT_CALL_STATE_INCOMING                    (0x30)
#define BT_ULL_EVENT_CALL_STATE_DIALING                     (0x31)
#define BT_ULL_EVENT_CALL_STATE_ALERTING                    (0x32)
#define BT_ULL_EVENT_CALL_STATE_ACTIVE                      (0x33)
#define BT_ULL_EVENT_CALL_STATE_LOCALLY_HELD                (0x34)
#define BT_ULL_EVENT_CALL_STATE_REMOTELY_HELD               (0x35)
#define BT_ULL_EVENT_CALL_STATE_LOCALLY_AND_REMOTELY_HELD   (0x36)
#define BT_ULL_EVENT_CALL_STATE_IDLE                        (0x37)

/* ULL call action, headset -> dongle*/
#define BT_ULL_EVENT_CALL_ANSWER            (0x40)
#define BT_ULL_EVENT_CALL_REJECT            (0x41)
#define BT_ULL_EVENT_CALL_HANG_UP           (0x42)
#define BT_ULL_EVENT_CALL_MUTE              (0x43)
#define BT_ULL_EVENT_CALL_UNMUTE            (0x44)
#endif

typedef uint8_t bt_ull_req_event_t;

#define BT_ULL_SBC_CODEC                    (0x00)
#define BT_ULL_AAC_CODEC                    (0x01)
#define BT_ULL_OPUS_CODEC                   (0x02)
typedef uint8_t bt_ull_codec_t;

#define BT_ULL_GAMING_TRANSMITTER           (0x00)
#define BT_ULL_CHAT_TRANSMITTER             (0x01)
#define BT_ULL_MIC_TRANSMITTER              (0x02)
#define BT_ULL_LINE_IN_TRANSMITTER          (0x03)
#define BT_ULL_LINE_OUT_TRANSMITTER         (0x04)
#define BT_ULL_I2S_IN_TRANSMITTER           (0x05)
#define BT_ULL_I2S_OUT_TRANSMITTER          (0x06)
#define BT_ULL_TRANSMITTER_MAX_NUM          (BT_ULL_I2S_OUT_TRANSMITTER + 0x01)
typedef uint8_t bt_ull_transmitter_t;

#define BT_ULL_PLAY_ALLOW                   (0x00)
#define BT_ULL_PLAY_DISALLOW                (0x01)
typedef uint8_t bt_ull_allow_play_t;

#define BT_ULL_ALLOW_PLAY_REASON_AUDIO            (0x00)
#define BT_ULL_ALLOW_PLAY_REASON_FOTA             (0x01)
typedef uint8_t bt_ull_allow_play_reason_t;


#define BT_ULL_STREAMING_UNMUTE             (0x00)
#define BT_ULL_STREAMING_MUTE               (0x01)
typedef uint8_t bt_ull_mute_state_t;

/**< Operation flag */
#define BT_ULL_OP_CODEC_OPEN                (1 << 0)    /**< Codec open */
#define BT_ULL_OP_DRV_PLAY                  (1 << 1)    /**< DRV play done */

#define BT_ULL_USB_VOLUME_MAX               (100)

#define BT_ULL_TX_MAX_CACHE                 (6)  /**< According to RFCOMM credit */

#define BT_ULL_DOWNLINK_LIMITATION           (25)    /* extremity 25ms latency */
#define BT_ULL_DOWNLINK_LIMITATION_MULTILINK (25)    /* dongle + sp */
#define BT_ULL_DOWNLINK_SINGLE_LIMITATION    (30)
#define BT_ULL_DOWNLINK_MULITILINK           (60)    /* multi-link temp state, 60ms latency */

/* define deafult uplink latency */
#ifdef AIR_ULL_VOICE_LOW_LATENCY_ENABLE
#define BT_ULL_UPLINK_DEFAULT               (25)
#else
#define BT_ULL_UPLINK_DEFAULT               (40)
#endif
#define BT_ULL_UPLINK_MAX                   (60)

#define BT_ULL_UPLINK_LATENCY_DEFAULT       (40)

/* define 2-rx mixing ratio */
#define BT_ULL_MIX_RATIO_MAX                (100)
#define BT_ULL_MIX_RATIO_MIN                (0)
/* streaming state flag */
#define BT_ULL_STREAMING_FLAG               (0x01)
#define BT_ULL_STREAMING_MIXED              ((BT_ULL_STREAMING_FLAG) << 0)
#define BT_ULL_STREAMING_STOPPING           ((BT_ULL_STREAMING_FLAG) << 1)
#define BT_ULL_STREAMING_STARTING           ((BT_ULL_STREAMING_FLAG) << 2)
#define BT_ULL_STREAMING_RECONFIG           ((BT_ULL_STREAMING_FLAG) << 3)    /* transmitter shuold re-config due to sample rate changed */

#define AIROHA_SDK_VERSION                  (0x020B0000)                      /*it's means SDK2.11.0*/

typedef struct {
    bt_bd_addr_t  bt_addr;   /**< Bluetooth Device Address defined in the specification. */
    bt_cm_profile_service_mask_t    profile;        /**< The last connected profile service. */
} bt_ull_last_connect_device_t;

typedef struct {
    bt_sink_srv_am_volume_level_out_t vol_left;     /**< Headset/Earbuds side only support one audio channle, we select left as default */
    bt_sink_srv_am_volume_level_out_t vol_right;
} bt_ull_duel_volume_t;

typedef struct {
    uint8_t vol_left;     /**< range: (0 ~ 100) */
    uint8_t vol_right;
} bt_ull_original_duel_volume_t;

typedef struct {
    uint8_t                ul_frame_size;  /**< /dongle sync  mic_frame_size to agent */
} bt_ull_uplink_info_t;

typedef struct {
    uint8_t                         sample_rate;                    /**< USB Streaming data sample rate. (unit: HZ, b0: 48000, b1: 44100, b2: 32000, b3: 16000.) */
    uint8_t                         sample_size;                    /**< USB Streaming data sample size. (unit: HZ, e.g. 16bit, 24 bit, 32 bit...) */
    uint8_t                         sample_channel;                 /**< USB Streaming data sample channel. (unit: HZ, e.g. 2 channel, 4 channel, 8 channel...) */
    bool                            is_mute;                        /**< audio streaming is mute/unmute */
    bool                            is_streaming;                   /**< audio streaming is playing or not */
    bt_ull_duel_volume_t            volume;                         /**< audio volume level */
    bt_ull_original_duel_volume_t   original_volume;                /**< PC original volume */
    /* transimitter relate*/
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    audio_transmitter_id_t          transmitter;                    /**< streaming audio transmitter handle */
#endif
    bool                            is_transmitter_start;           /**< transmitter is start or not */
    bool                            is_request_transmitter_start;   /**< user request transmitter start or stop */
    uint32_t                        streaming_flag;                 /**< streaming flag <record streaming temporary state> */
    audio_src_srv_resource_manager_handle_t *resource_handle;       /**< audio source Manager handle> */
    bool                            is_suspend;                     /**< Audio is suspended by other audio resource> */
    bool                            is_usb_param_update;            /**< usb param is update > */
} bt_ull_streaming_if_info_t;

typedef struct {
    bt_bd_addr_t                    bt_addr;                /**< Bluetooth Device Address defined in the specification. */
    bt_ull_role_t                   ull_role;
    bt_ull_callback                 callback;
    /* audio manager relate */
    bt_sink_srv_am_id_t             audio_id;
    bt_ull_am_state_t               am_state;               /**< audio manager state */
    bt_ull_am_sub_state_t           am_substate;            /**< audio manager sub-state */
    uint32_t                        flag;                   /**< audio manager flag */
    audio_src_srv_handle_t         *am_handle;              /**< Pseudo device handle */
    bt_sink_srv_am_media_handle_t   med_handle;             /**< codec handle */
    bt_ull_codec_t                  codec_type;
    bt_a2dp_codec_capability_t      codec_cap;              /**< The capabilities of Bluetooth codec */
    /* streaming interface */
    bt_ull_streaming_if_info_t      dl_speaker;             /**< downlink gaming streaming */
    bt_ull_streaming_if_info_t      dl_chat;                /**< downlink chat streaming */
    bt_ull_streaming_if_info_t      ul_microphone;          /**< uplink streaming */
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
    bt_ull_streaming_if_info_t      dl_linein;              /**< downlink dl_linein */
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    bt_ull_streaming_if_info_t      ul_lineout;             /**< uplink dl_lineout */
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
    bt_ull_streaming_if_info_t      dl_linei2s;             /**< downlink dl_line_i2s */
    i2s_in_dongle_config_t          linei2s_param;          /**< backup downlink line_i2s config param*/
#endif
    uint16_t                        dl_latency;             /**< downlink latency, default: 20, unit: ms */
    uint16_t                        ul_latency;             /**< voice uplink latencty, unit:ms */
    bt_ull_allow_play_t             allow_play;             /**< headset/earbuds is allow dongle play. */
    bool                            avm_enable;             /**< notify controller avm state. */
    bt_ull_mix_ratio_t              dl_mix_ratio;           /**< 2 downlink mixing ratio */
    bt_ull_last_connect_device_t    remode_device;          /**< last connected phone info */
    /* spp relate */
    uint16_t                        server_channel_id;
    uint16_t                        max_packet_size;
    uint32_t                        spp_handle;
    bool                            is_ull_connected;       /**< ull spp channel is connected */
    bool                            is_disable_sniff;       /**< ull link sniff mode is disabled or not */
    uint8_t                         critical_data_max_len;
    uint8_t                         critical_data_tx_seq;   /**< valid seq: 1 ~ 255 */
    uint8_t                         critical_data_rx_seq;   /**< valid seq: 1 ~ 255 */
    bool                            is_ull_version_ready;   /**< ull get sdk version */
    uint32_t                        sdk_version;
} bt_ull_context_t;

typedef struct {
    bt_ull_codec_t           codec_type;         /**< streaming encodec codec info. */
    uint8_t                  sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    uint8_t                  bit_rate;           /**< b0: 16kbps, b1:32kbps, b2:64kbps. */
    uint8_t                  channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
    bool                     is_mute;            /**< dongle music is mute/unmute */
    bool                     is_streaming;       /**< streaming is playing or not */
    bt_ull_original_duel_volume_t vol;           /**< streaming current volume */
    uint8_t                  RFU[2];             /**< reserved for furture */
} bt_ull_streaming_context_t;


typedef struct {
    bt_ull_streaming_context_t  server_speaker_dl;
    bt_ull_streaming_context_t  server_mic_ul;
    bt_ull_allow_play_t         allow_play;                 /**< headset/earbuds is allow dongle play. */
    bt_ull_client_t             client_type;                /**< sync connected client type to dongle */
    bt_ull_mix_ratio_t          mix_ratio;                  /**< client sync current mix ratio setting to dongle */
    uint16_t                    dl_latency;                 /**< client sync current latency to dongle */
    uint16_t                    ul_latency;                 /**< client sync current voice uplink latencty to dongle */
    bt_ull_original_duel_volume_t server_chat_volume;
    bt_ull_uplink_info_t        uplink_info;                /**< sync uplink frame info to Client */
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
    bt_ull_original_duel_volume_t server_linein_volume;     /**< streaming line in volume. */
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
    bt_ull_original_duel_volume_t server_linei2s_volume;    /**< streaming line in volume. */
#endif
} bt_ull_connection_info_t;

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)

/**
 *  @brief Define the USB Dongle Streaming Port Mask.
 */
typedef uint8_t bt_ull_le_stream_port_mask_t;
#define BT_ULL_LE_STREAM_PORT_MASK_NONE      0x00
#define BT_ULL_LE_STREAM_PORT_MASK_GAMING    0x01
#define BT_ULL_LE_STREAM_PORT_MASK_CHAT      0x02
#define BT_ULL_LE_STREAM_PORT_MASK_MIC       0x04
#define BT_ULL_LE_STREAM_PORT_MASK_LINE_IN   0x08
#define BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT  0x10
#define BT_ULL_LE_STREAM_PORT_MASK_I2S_IN    0x20
#define BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT   0x40

/**
 *  @brief Define the opcode of finding coordinated set information.
 */
typedef uint8_t bt_ull_le_srv_cs_opcode_t;
#define BT_ULL_LE_SRV_CS_REQ          0x01    /**< Request, Dongle -> Headset. */
#define BT_ULL_LE_SRV_CS_RSP          0x02    /**< Response, Headset -> Dongle. */

/**
 * @brief This structure defines the client preferred codec param
 */
typedef struct {
    bt_ull_le_codec_t codec_type;
    uint32_t          dl_samplerate;
    uint32_t          ul_samplerate;
} bt_ull_le_srv_client_preferred_codec_param;

/**
 * @brief This structure defines the parameters of ULL Dongle coordinated set.
 */

typedef struct {
    bt_ull_le_srv_cs_opcode_t op;
    bt_ull_client_t client_type;
    uint8_t set_size;
    bt_key_t sirk;
    bt_ull_le_channel_mode_t  client_preferred_channel_mode;
    bt_ull_le_codec_t client_preferred_codec_type;
    bt_ull_le_srv_client_preferred_codec_param codec_param;
    bt_addr_t group_device_addr[BT_ULL_LE_MAX_LINK_NUM - 1];
} bt_ull_le_srv_cs_info_t;

/**
 * @brief This structure defines the parameters of ULL Dongle sample rate change.
 */
typedef struct {
    bt_ull_transmitter_t transmitter;
    uint32_t sample_rate;
} bt_ull_le_srv_sample_rate_change_t;

/**
 * @brief This structure defines the stream information of ULL Dongle Server.
 */
typedef struct
{
    bool                            is_mute;                        /**< audio streaming is mute/unmute */
    bool                            is_transmitter_start;           /**< transmitter is start or not */
    bool                            is_need_restart;                /**< transmitter is need restart */
    uint32_t                        usb_sample_rate;                /**< USB Streaming data sample rate. (unit: HZ, e.g. 16000,32000,48000, 96000...) */
    uint32_t                        usb_sample_size;                /**< USB Streaming data sample size. (unit: HZ, e.g. 16bit, 24 bit, 32 bit...) */
    uint32_t                        usb_sample_channel;             /**< USB Streaming data sample channel. (unit: HZ, e.g. 2 channel, 4 channel, 8 channel...) */
    bt_ull_original_duel_volume_t   volume;                         /**< PC original volume */
    bt_ull_le_codec_param_t         codec_param;
} bt_ull_le_srv_server_stream_t;

/**
 * @brief This structure defines the stream information of ULL Dongle Client.
 */
typedef struct
{
    bool                            is_dl_dummy_mode;               /**< To match the advanced EMP rule, Downlink Dummy Mode is needed. it means that the Uplink is ongoing but downlink has not enabled,
                                                                                                                          the uplink will use the downlink's audio manager priority to control itself.*/
    bool                            is_mute;                        /**< Audio streaming is mute/unmute */
    bool                            is_am_open;                     /**< Audio manager is open or not */
    uint32_t                        usb_sample_rate;                /**< USB Streaming data sample rate. (unit: HZ, e.g. 16000,32000,48000, 96000...) */
    uint32_t                        usb_sample_size;                /**< USB Streaming data sample size. (unit: HZ, e.g. 16bit, 24 bit, 32 bit...) */
    uint32_t                        usb_sample_channel;             /**< USB Streaming data sample channel. (unit: HZ, e.g. 2 channel, 4 channel, 8 channel...) */
    bt_ull_original_duel_volume_t   volume;                         /**< PC original volume */
    bt_ull_le_codec_param_t         codec_param;
} bt_ull_le_srv_client_stream_t;


/**
 * @brief This structure defines the stream context of ULL Dongle.
 */
typedef struct
{
    bool                             is_same_codec;      /**< The SPK gaming port, SPK chat port and Mic Port have the same codec params. */
    bt_ull_allow_play_t              allow_play;         /**< Headset/earbuds/speaker is allow dongle play. */
    bt_ull_le_codec_t                codec_type;         /**< Streaming encodec codec info. */
    bt_ull_le_stream_port_mask_t     streaming_port;     /**< Which port is streaming */
    bt_ull_le_stream_port_mask_t     locking_port;       /**< Which port is lock streaming*/
    uint8_t                          dl_latency;         /**< Client sync current latency to dongle */
    uint8_t                          ul_latency;         /**< Client sync current voice uplink latencty to dongle */
    bt_ull_mix_ratio_t               dl_mix_ratio;       /**< 2 downlink mixing ratio */
    bt_ull_original_duel_volume_t    server_chat_volume;
    bt_ull_original_duel_volume_t    server_linein_volume;
    bt_ull_original_duel_volume_t    server_linei2s_volume;
    bt_ull_original_duel_volume_t    server_lineout_volume;
    union {
        struct {
            bt_ull_le_srv_server_stream_t stream[BT_ULL_TRANSMITTER_MAX_NUM]; /**< server downlink and uplink streaming */
        } server;
        struct {
            bt_ull_le_srv_client_stream_t dl;            /**< Client downlink streaming */
            bt_ull_le_srv_client_stream_t ul;            /**< Client uplink streaming */
        } client;
    };
    bool                              is_silence;
    bt_ull_le_srv_audio_quality_t     audio_quality;
} bt_ull_le_srv_stream_context_t;

#endif

typedef struct {
    bt_ull_streaming_t       streaming;     /**< streaming interface type. */
    bt_ull_original_duel_volume_t vol;
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    //bt_ull_le_srv_volume_t   vol_gain;
    bt_ull_audio_channel_t   channel;           /**< Audio channel. */
#endif
} bt_ull_volume_ind_t;

typedef struct {
    bt_ull_allow_play_t     allow_play;     /**< headset/earbuds is allow playing or not. */
    uint8_t                 reason;
} bt_ull_allow_play_report_t;

typedef struct {
    bt_ull_req_event_t event;
    union {
        bt_ull_connection_info_t    connection_info;
        bt_ull_streaming_t          streaming_port;
        bt_ull_volume_ind_t         vol_ind;
        bt_ull_latency_t            latency;
        bt_ull_allow_play_report_t  allow_play_report;
        bt_ull_volume_t             vol_action;
        bt_ull_mix_ratio_t          mix_ratio;
        bt_ull_usb_hid_control_t    hid_control;
        bt_ull_uplink_info_t        uplink_info;
    };
} bt_ull_req_t;

#if (defined AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)

typedef struct {
    bt_ull_streaming_t       streaming;         /**< The streaming type. */
    union {
      i2s_in_dongle_config_t i2s_in_dongle_config;
    };
} bt_ull_interface_config_t;

#endif


void bt_ull_atci_init(void);
bt_status_t bt_ull_sdp_register(bt_ull_role_t role);
const uint8_t *bt_ull_get_uuid(void);
bt_ull_context_t *bt_ull_get_context(void);
void bt_ull_update_audio_buffer(bt_ull_role_t role, bt_ull_client_t client_type);
void bt_ull_set_music_enable(uint32_t handle, bt_avm_role_t role, bool enable, uint16_t dl_latency);
void bt_ull_save_last_connected_device_info(uint8_t *address, bt_cm_profile_service_mask_t profiles);
void bt_ull_event_callback(bt_ull_event_t event, void *param, uint32_t len);
bt_sink_srv_am_volume_level_out_t bt_ull_get_volume_level(uint8_t volume);
void bt_ull_clear_last_connected_device_info(void);
void bt_ull_init_transimter(bt_ull_codec_t codec, bt_ull_transmitter_t transmitter);
void bt_ull_dvfs_lock(dvfs_frequency_t feq);
void bt_ull_dvfs_unlock(dvfs_frequency_t feq);
void *bt_ull_memory_alloc(uint16_t size);
void bt_ull_memory_free(void *point);

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
void bt_ull_audio_transmitter_voice_callback(audio_transmitter_event_t event, void *data, void *user_data);
void bt_ull_audio_transmitter_music_callback(audio_transmitter_event_t event, void *data, void *user_data);
void bt_ull_audio_transmitter_chat_callback(audio_transmitter_event_t event, void *data, void *user_data);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
void bt_ull_audio_transmitter_line_in_callback(audio_transmitter_event_t event, void *data, void *user_data);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
void bt_ull_audio_transmitter_line_out_callback(audio_transmitter_event_t event, void *data, void *user_data);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
void bt_ull_audio_transmitter_i2s_in_callback(audio_transmitter_event_t event, void *data, void *user_data);
#endif


void bt_ull_set_transmitter_volume(bt_ull_streaming_t *streaming);
bt_status_t bt_ull_send_data(uint32_t handle, uint8_t *packet, uint16_t packet_size);
void bt_ull_clear_spp_cache(void);
void bt_ull_cache_spp_tx(bt_ull_req_t *tx_req);
void bt_ull_send_cache_spp_data(uint32_t handle);
uint32_t bt_ull_sample_rate_exchange(uint8_t sample_bit);
uint32_t bt_ull_sample_size_exchange(uint8_t sample_size);


bt_status_t bt_ull_air_pairing_start(const bt_ull_pairing_info_t *param);
bt_status_t bt_ull_air_pairing_stop(void);

void bt_ull_set_ULL_mode(bool enable);
bool bt_ull_get_ULL_mode(void);
void bt_ull_notify_server_play_is_allow(bt_ull_allow_play_t is_allow);

bt_status_t bt_ull_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length);
uint32_t bt_ull_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length);
bool bt_ull_is_transmitter_start(bt_ull_streaming_t *streaming);

bt_status_t bt_ull_srv_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length);
uint32_t bt_ull_srv_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length);

void bt_ull_start_transmitter(bt_ull_streaming_if_info_t *ep);
void bt_ull_stop_transmitter(bt_ull_streaming_if_info_t *ep);

bool bt_ull_srv_is_transmitter_start(bt_ull_streaming_t *streaming);
void bt_ull_set_mic_frame_size(uint8_t size);
uint8_t bt_ull_get_mic_frame_size(void);

/**< ULL 1.0 Public Functions. */
bt_status_t bt_ull_srv_init(bt_ull_role_t role, bt_ull_callback callback);
bt_status_t bt_ull_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len);
bt_status_t bt_ull_srv_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info);
void bt_ull_srv_lock_streaming(bool lock);

bt_ull_streaming_if_info_t *bt_ull_get_streaming_interface(bt_ull_streaming_t *streaming);

uint32_t bt_ull_get_usb_sample_rate(bt_ull_transmitter_t transmitter_type);
uint32_t bt_ull_get_usb_sample_size(bt_ull_transmitter_t transmitter_type);
uint32_t bt_ull_get_usb_sample_channel(bt_ull_transmitter_t transmitter_type);
void bt_ull_le_srv_change_audio_quality(bt_ull_role_t role, bt_ull_le_srv_audio_quality_t quality);

BT_EXTERN_C_END

#endif /* __BT_ULL_UTILITY_H__  */

