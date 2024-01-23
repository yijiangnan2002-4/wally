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

#ifndef __BT_SINK_SRV_MUSIC_H__
#define __BT_SINK_SRV_MUSIC_H__

#include "stdint.h"
#include "bt_type.h"
#include "bt_connection_manager_internal.h"
#include "bt_sink_srv_ami.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "hal_gpt.h"
#include "bt_sink_srv.h"
#include "bt_avm.h"
#include "bt_aws_mce_srv.h"
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#include "bt_timer_external.h"
#endif
#include "audio_src_srv.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce.h"
#include "bt_sink_srv_aws_mce.h"
#endif
#include "bt_system.h"
#include "bt_avrcp.h"
#include "bt_sink_srv_state_manager.h"
#include "bt_init.h"

#define bt_sink_srv_music_get_aws_handle_by_addr(addr) bt_aws_mce_query_handle_by_address(BT_MODULE_AWS_MCE, (const bt_bd_addr_t *)addr)

#ifdef MTK_AWS_MCE_ENABLE
#define __BT_AWS_MCE_A2DP_SUPPORT__
#endif

#ifdef __MTK_AVM_DIRECT__
#define __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
#endif

#ifdef MTK_BT_SPEAKER_ENABLE
#define bt_sink_srv_music_get_spk_mode(void) bt_aws_mce_srv_get_mode()
#else
#define bt_sink_srv_music_get_spk_mode(void) BT_AWS_MCE_SRV_MODE_NORMAL
#endif
#define BT_SINK_SRV_MUSIC_MODE_BROADCAST BT_AWS_MCE_SRV_MODE_BROADCAST
#define BT_SINK_SRV_IMPROVE_RESYNC

#define __BT_SINK_SRV_A2DP_V13_SUPPORT__

#define BT_A2DP_VENDOR_CODEC_SAMPLE_RATE_MASK       (0x3F)   /**< VENDOR codec SAMPLE_RATE_MASK. */
#define BT_A2DP_VENDOR_CODEC_CHANNEL_MODE_MASK      (0x07)   /**< VENDOR codec CHANNEL_MODE_MASK.*/
#define BT_A2DP_VENDOR_2_CODEC_SAMPLE_RATE_MASK     (0x3F)   /**< VENDOR_2 codec SAMPLE_RATE_MASK.*/
#define BT_A2DP_VENDOR_2_CODEC_RESOLUTION_MASK      (0x07)   /**< VENDOR_2 codec RESOLUTION_MASK.*/
#define BT_A2DP_VENDOR_2_CODEC_LOSSLESS_MASK        (0x80)   /**< VENDOR_2 codec LOSSLESS_MASK.*/
#define BT_A2DP_VENDOR_2_CODEC_LL_MASK              (0x40)   /**< VENDOR_2 codec LOWLATENCY_MASK.*/
#define BT_A2DP_VENDOR_2_CODEC_RAW_MASK             (0x80)   /**< VENDOR_2 codec RAW_MASK.*/

#define BT_SINK_SRV_AVRCP_CONNECTION_TIMER_DUR             (1500)
#define BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_DUR             (1000)
#define BT_SINK_SRV_A2DP_START_STREAMING_TIMER_DUR         (1000)
#define BT_SINK_SRV_REINITIAL_SYNC_TIMER_DUR               (2000)
#define BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_DUR       (2000)
#define BT_SINK_SRV_SET_AM_VOLUME_TIMER_DUR              (80)
#define BT_SINK_SRV_AVRCP_PUSH_RELEASE_TIMER_DUR            (45)
#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_DUR    (1000)
#define BT_SINK_SRV_A2DP_RETRY_SEND_EIR_TIMER_DUR    (200)
#define BT_SINK_SRV_A2DP_SEND_BASE_IND_TIMER_DUR    (1000)
#define BT_SINK_SRV_RECONNECT_AGENT_TIMER_DUR      (10)
#define BT_SINK_SRV_MUSIC_MAX_UPDATE_BASE_COUNT    (10)
#endif
#define BT_SINK_SRV_MUSIC_RATIO_MAGIC_CODE         (0xab)
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#define BT_SINK_SRV_A2DP_INT_WAIT_USER_TIMER_DUR (500)
#endif
#define BT_SINK_SRV_AVRCP_CONNECTION_AS_TG_TIMER_DUR       (1)

#define BT_SINK_SRV_IPHONE_TYPE_IN_RATIO (0X0)
#define BT_SINK_SRV_SPECIAL_DEV_MAGIC_NUMBER  (0xffff)
#define BT_SINK_SRV_MUSIC_NOTIFY_N9_NO_SLEEP    (0x02)
#ifdef AIR_A2DP_REINIT_V2_ENABLE
#define BT_SINK_SRV_MUSIC_SHORT_PCDC_OBSERVATION    (0x04)
#endif
#define BT_SINK_SRV_A2DP_STATUS_SUSPEND     0x00
#define BT_SINK_SRV_A2DP_STATUS_STREAMING   0x01

#define BT_SINK_SRV_MUSIC_AVRCP_INVALID_STATUS (0xff)
#define BT_SINK_SRV_MUSIC_AVRCP_NO_OPERATION   (0xff)
#define BT_SINK_SRV_MUSIC_AVRCP_NO_STATE       (0xff)
#define BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION (0x0)

#define BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG          (1<<0)
#define BT_SINK_SRV_CNTX_FLAG_POWER_OFF                     (1<<1)
#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
#define BT_SINK_SRV_CNTX_FLAG_SWITCH_MUSIC_OFF              (1<<3)
#define BT_SINK_SRV_CNTX_FLAG_SWITCH_MUSIC_OFF_PAUSE        (1<<4)
#endif
#define BT_SINK_SRV_CNTX_FLAG_MUST_PLAY_RING_TONE_FLAG      BT_SINK_SRV_AVRCP_MUST_PLAY_RING_TONE_FLAG //(1 << 6)
typedef uint8_t bt_sink_srv_ctx_flag_t;

/**< Deveice flag */
/* Stable flag */
#define BT_SINK_SRV_MUSIC_NORMAL_STOP_FLAG                  (1 << 1)
#define BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP             (1 << 2)
#define BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT               (1 << 3)
#define BT_SINK_SRV_MUSIC_FLAG_WAITING_START                (1 << 4)
#define BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC               (1 << 5)
#define BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC           (1 << 6)
#define BT_SINK_SRV_MUSIC_INITIAL_A2DP_BY_DEVICE            (1 << 7)
#define BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME                   (1 << 8)
#define BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START       (1 << 9)
#define BT_SINK_SRV_MUSIC_FLAG_RHO_HAPPEN_DURING_STARTING_PLAY   (1<<10)
#define BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING                (1 << 11)
#define BT_SINK_SRV_MUSIC_FLAG_AWS_A2DP_END                 (1 << 12)

/*avrcp flag*/
#define BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT    (1 << 0)
#define BT_SINK_SRV_AVRCP_GET_FOLDER_ITEM                           (1 << 1)
#define BT_SINK_SRV_INIT_AVRCP_BY_REMOTE_DEVICE                     (1 << 2)
#define BT_SINK_SRV_DURING_PARTNER_TRIGGER_PASS_THROUGH             (1 << 3)
#define BT_SINK_SRV_AVRCP_MUST_PLAY_RING_TONE_FLAG                  (1 << 5)

/* Wait flag */
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_START                   BT_SINK_SRV_MUSIC_FLAG_AWS_A2DP_END
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC          ((BT_SINK_SRV_MUSIC_FLAG_WAIT_START) << 0)
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_DRV_PLAY                ((BT_SINK_SRV_MUSIC_FLAG_WAIT_START) << 1)
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_DRV_STOP                ((BT_SINK_SRV_MUSIC_FLAG_WAIT_START) << 2)
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY           ((BT_SINK_SRV_MUSIC_FLAG_WAIT_START) << 3) /**< Wait to play in waiting list*/
#define BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT        ((BT_SINK_SRV_MUSIC_FLAG_WAIT_START) << 4)

/**< Operation flag */
#define BT_SINK_SRV_MUSIC_OP_CODEC_OPEN                     (1 << 0)    /**< Codec open */
#define BT_SINK_SRV_MUSIC_OP_DRV_PLAY                       (1 << 1)    /**< DRV play done */
#define BT_SINK_SRV_MUSIC_OP_PLAY_IND                       (1 << 2)
#define BT_SINK_SRV_MUSIC_REINIT_ON_PARTNER_LATER_JOIN_FLAG (1 << 4)   /*< Use this flag to rember need to do reinit sync when partner later join*/
#define BT_SINK_SRV_MUSIC_START_PLAY_ON_NEW_AGENT_FLAG      (1 << 5)   /*< This flag should be set if a2dp start come during RHO, so need to resume music on new agent*/
#define BT_SINK_SRV_MUSIC_NO_STREAMING_STATE_UPDATE_FLAG    (1 << 6)
#define BT_SINK_SRV_MUSIC_INIT_DISCONNNECT_AVRCP_TIMER_FLAG (1 << 7)   /*< This flag should be set if avrcp connection action is send before a2dp disconnect ind*/

#define BT_SINK_SRV_MUSIC_WAITING_REINITIAL_SYNC            (1 << 9)

#define BT_SINK_SRV_MUSIC_A2DP_CONN_BIT              (1 << 0)
#define BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT             (1 << 1)
#define BT_SINK_SRV_MUSIC_AWS_CONN_BIT               (1 << 2)
#define BT_SINK_SRV_MUSIC_AVRCP_BROWSING_CONN_BIT    (1 << 3)

#define BT_SINK_SRV_MUSIC_INVALID_HD                 (0x00)
#define BT_SINK_SRV_INVALID_AID                 (-1)

/**
 * @brief This structure defines the AWS role.
 */

#define BT_SINK_SRV_SET_FLAG(FLAG, MASK) do { \
    (FLAG) |= (MASK); \
} while(0);

#define BT_SINK_SRV_REMOVE_FLAG(FLAG, MASK) do { \
    (FLAG) &= ~(MASK); \
} while(0);


#define BT_SINK_SRV_MUISC_DEV_COUNT     (BT_A2DP_TOTAL_LINK_NUM+BT_AWS_MCE_TOTAL_CONNECTION_NUM)     /**< Max device count */
#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_A2DP_SBC_MAX_BP_IN_SPEAKER_MODE                     (35)
#endif

#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
#define BT_SINK_SRV_A2DP_N_PACKET_NOTIFY                            (3)    /**< N packet done notify (default N=3) */
#define BT_SINK_SRV_A2DP_N_PACKET_NOTIFY_VENDOR_CODEC               (0x3|0x8000)
#define BT_SINK_SRV_MCE_A2DP_N_PACKET_NOTIFY                        (1)
#define BT_SINK_SRV_MCE_A2DP_N_PACKET_NOTIFY_VENDOR_CODEC           (0x1|0x8000)
#define BT_SINK_SRV_MUSIC_CONTENT_PROTECTION_MASK                   (0x4000)
#ifdef AIR_A2DP_REINIT_V2_ENABLE
#define BT_SINK_SRV_A2DP_SHORT_PCDC_OBSERVATION                     (0x2000)
#endif
#define BT_SINK_SRV_A2DP_VENDOR_2_N_PACKET_NOTIFY                   (1)


#define BT_SINK_SRV_MAX_OPTIMIZE_MEDIA_PACKET_COUNT                 (10)
#define BT_SINK_SRV_A2DP_DEFAULT_SINK_LATENCY                               (140000)
#define BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY                         (250000)
#define BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY_170MS                   (170000)
#define BT_SINK_SRV_A2DP_SPEAKER_MODE_DEFAULT_SINK_LATENCY              (190000)
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
#define BT_SINK_SRV_A2DP_LHDC_LL_MODE_LATENCY                           (60000)
#endif
#define BT_SINK_SRV_A2DP_AVM_TIMER_MIN_DUR_TO_PLAY_BY_TICK                  (192)
#define BT_SINK_SRV_A2DP_MIN_STA_DUR_TO_PLAY                                (384)
#endif

#define BT_SINK_SRV_A2DP_BT_VOL_LEV                                 (100)
#define BT_SINK_SRV_A2DP_LOCAL_VOL_LEV                              (16)

#define BT_SINK_SRV_A2DP_ASI_MASK                       (0x03ffffff)
#ifdef __BT_SINK_SRV_A2DP_V13_SUPPORT__
#define BT_SINK_SRV_A2DP_DELAY          (1500) /**150 ms*/
#endif

typedef enum {
    BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD,                       /* 0 */
    BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD,                      /* 1 */
    BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP,                     /* 2 */
    BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP,                    /* 3 */
    BT_SINK_SRV_MUSIC_DEVICE_ADDR_AWS_MP3,                  /* 4 */
    BT_SINK_SRV_MUSIC_DEVICE_UNUSED,                        /* 5 */
    BT_SINK_SRV_MUSIC_DEVICE_USED,                          /* 6 */
    BT_SINK_SRV_MUSIC_DEVICE_LATEST,                        /* 7 */
    BT_SINK_SRV_MUSIC_DEVICE_INTERRUPT,                     /* 8 */
    BT_SINK_SRV_MUSIC_DEVICE_FOCUS,                         /* 9 */
    BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD,                   /**< 10 AWS handle is A2DP pseudo devcie */
    BT_SINK_SRV_MUSIC_DEVICE_AWS_MP3_HD,                    /**< 11 */
    BT_SINK_SRV_MUSIC_DEVICE_AWS,                           /**< 12 same check pseudo type */
    BT_SINK_SRV_MUSIC_DEVICE_AWS_BY_LINK,                   /**< 13 */
    BT_SINK_SRV_MUSIC_DEVICE_PSE_HD,                        /**< 14 Pseudo device handle */
    BT_SINK_SRV_MUSIC_DEVICE_SP,                            /**< 15 */
    BT_SINK_SRV_MUSIC_DEVICE_LOCAL,                         /**< 16 */
    BT_SINK_SRV_MUSIC_DEVICE_AVRCP_BROWSE_HD                /**< 17 */
} bt_sink_srv_music_device_type_t;

typedef audio_src_srv_state_t bt_sink_srv_music_state_t;

typedef enum {
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_NONE = 0,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_CONN,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_DISCONN,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_AWS_ROLE_SET,    /**< waiting role set */
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_AWS_START,       /**< waiting role set */
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_PREPARE_CODEC,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_PREPARE_BUFFER,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_INIT_SYNC,       /**< waiting init sync */
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC,
    BT_SINK_SRV_MUSIC_TRANSIENT_STATE_WAIT_AWS_STOP,
    /**< waiting stop cnf */
} bt_sink_srv_music_transient_state_t;

#if (BT_A2DP_TOTAL_LINK_NUM>3)
typedef enum {
    BT_SINK_SRV_MUSIC_A2DP_COUNT = 0,
    BT_SINK_SRV_MUSIC_AVRCP_COUNT,
} bt_sink_srv_music_state_count_t;
#endif

#define BT_SINK_SRV_MUSIC_EVT_BASE              (0x800000)

#define BT_SINK_SRV_MUSIC_NORMAL_MODE           (0x00)
#define BT_SINK_SRV_MUSIC_GAME_MODE             (0x01)
#define BT_SINK_SRV_MUSIC_MODE_MAGIC_NUMBER     (0xa0)
#define BT_SINK_SRV_MUSIC_DATA_NOT_FOUND        (0xff)
typedef uint8_t bt_sink_srv_music_mode_t;

#define BT_SINK_SRV_MUSIC_DATA_VOLUME           (0x00)
#define BT_SINK_SRV_MUSIC_DATA_MODE             (0x01)
#define BT_SINK_SRV_MUSIC_DATA_LOCAL_ASI_FLAG   (0x02)

typedef uint8_t bt_sink_srv_music_data_type_t;

typedef uint32_t bt_sink_srv_music_event_t;

#define BT_SINK_SRV_MUSIC_EVT_START             (BT_SINK_SRV_MUSIC_EVT_BASE)
#define BT_SINK_SRV_MUSIC_EVT_UNAVAILABLE       (BT_SINK_SRV_MUSIC_EVT_START + 1)
#define BT_SINK_SRV_MUSIC_EVT_READY             (BT_SINK_SRV_MUSIC_EVT_START + 2)
#define BT_SINK_SRV_MUSIC_EVT_PLAYING           (BT_SINK_SRV_MUSIC_EVT_START + 3)
#define BT_SINK_SRV_MUSIC_EVT_REJECT            (BT_SINK_SRV_MUSIC_EVT_START + 4)
#define BT_SINK_SRV_MUSIC_EVT_CODEC_OPEN        (BT_SINK_SRV_MUSIC_EVT_START + 5)
#define BT_SINK_SRV_MUSIC_EVT_PREPARE_FAIL      (BT_SINK_SRV_MUSIC_EVT_START + 6)
#define BT_SINK_SRV_MUSIC_EVT_CODEC_CLEAR       (BT_SINK_SRV_MUSIC_EVT_START + 7)
#define BT_SINK_SRV_MUSIC_EVT_RECOVER           (BT_SINK_SRV_MUSIC_EVT_START + 8)
#define BT_SINK_SRV_MUSIC_EVT_RESUME            (BT_SINK_SRV_MUSIC_EVT_START + 9)
#define BT_SINK_SRV_MUSIC_EVT_START_IND         (BT_SINK_SRV_MUSIC_EVT_START + 10)
#define BT_SINK_SRV_MUSIC_EVT_START_CNF         (BT_SINK_SRV_MUSIC_EVT_START + 11)
#define BT_SINK_SRV_MUSIC_EVT_SUSPEND_IND       (BT_SINK_SRV_MUSIC_EVT_START + 12)
#define BT_SINK_SRV_MUSIC_EVT_SUSPEND_CNF       (BT_SINK_SRV_MUSIC_EVT_START + 13)
#define BT_SINK_SRV_MUSIC_EVT_PREPARE_CODEC     (BT_SINK_SRV_MUSIC_EVT_START + 15)
#define BT_SINK_SRV_MUSIC_EVT_PREPARE_CLEAR     (BT_SINK_SRV_MUSIC_EVT_START + 16)

#define BT_SINK_SRV_MUSIC_EVT_AVRCP_PLAYING     (BT_SINK_SRV_MUSIC_EVT_START + 20)
#define BT_SINK_SRV_MUSIC_EVT_AVRCP_PAUSED      (BT_SINK_SRV_MUSIC_EVT_START + 21)


#define BT_SINK_SRV_MUSIC_REINIT_BUF_ABNORMAL       0x01
#define BT_SINK_SRV_MUSIC_REINIT_AFE_ABNORMAL       0x02
#define BT_SINK_SRV_MUSIC_REINIT_PARTNER_LATER_JOIN 0x03
typedef uint32_t bt_sink_srv_music_dsp_parameter_t;

#define BT_SINK_SRV_AMA_NOTIFICATION_VOICE          0x01
#define BT_SINK_SRV_BISTO_NOTIFICATION_VOICE        0x02
#define BT_SINK_SRV_XIAOAI_NOTIFICATION_VOICE       0x03
#define BT_SINK_SRV_XIAOWEI_NOTIFICATION_VOICE      0x04
#define BT_SINK_SRV_INIT_NOTIFICATION_VOICE         0xff
typedef uint8_t bt_sink_srv_notification_voice_t;

typedef struct {
    volatile uint8_t state;
    uint8_t data_size;
    uint16_t size; /**Size in byte, including rtpheader and payload*/
} bt_sink_srv_audio_pcb_type_t;

typedef struct {
    uint8_t version;
    uint8_t type;
    uint16_t seqno;
    uint32_t timestamp;
    uint32_t nclk;
} bt_sink_srv_audio_rtp_header_t;


#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
typedef struct {
    uint32_t seq_magic;                 /* do not remove for campatible */
    uint8_t state;                      /**< A2DP state */
    uint8_t vol_lev;                /**< A2DP volume level */
    uint16_t sample_count;           /**< sample count per packet*/
    uint16_t drift_val;             /**< A2DP drift value> */
    uint16_t ratio: 4;              /**< A2DP ratio value, the ratio is 1-10> */
    uint16_t nclk_intra: 12;           /**< The Bluetooth clock in microseconds, the bigest value of intra is 1250.*/
    uint32_t nclk;                  /**< The Bluetooth clock in 312.5us units.*/
    uint32_t ts;                        /**< Packet ts */
    uint32_t latency_val;           /**< latency value to partner*/
    bt_a2dp_codec_capability_t codec;   /**< A2DP codec info 16 byte */
    uint8_t special_device_flag;
    uint8_t flag;
#ifdef MTK_BT_SPEAKER_ENABLE
    uint8_t vol_change_count;
#endif
    uint16_t a2dp_mtu;
#ifdef MTK_BT_SPEAKER_ENABLE
    uint32_t speaker_compatible_flag;
#endif
} bt_sink_srv_aws_mce_a2dp_base_eir_t;

/**< music action control type one byte indicate play/pause/pre track/next track */
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL                  (0x01)
/* music volume control type indicate volume up/down */
#define BT_SINK_SRV_MUSIC_TYPE_VOL                      (0x02)
/**< action control explanation */
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_LEN          (0x01)    // control value length

#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY         (0x01)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PAUSE        (0x02)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PRE_TRC      (0x03)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_NEXT_TRC     (0x04)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_REINITIAL_SYNC (0x05)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_ALC      (0x06)
#define BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY_PAUSE         (0x07)
#define BT_SINK_SRV_MUSIC_TYPE_AVRCP_PASS_THROUGH_COMMAND       (0X08)
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
#define BT_SINK_SRV_MUSIC_TYPE_SYNC_VENDOR_CODEC_CONFIG       (0X09)
#endif
#ifdef AIR_A2DP_REINIT_V2_ENABLE
#define BT_SINK_SRV_MUSIC_TYPE_BUF_ABNORMAL_SYNC (0x0a)
#endif
/**< music volume explanation */
#define BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_LEN              (0x01)    // control value length

#define BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_CHANGE           (0x01)    // mean volume value will be confirmable value--percentage
#define BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_UP               (0x02)
#define BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_DOWN             (0x03)
#define BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_SET              (0x04)
#endif /* __BT_AWS_MCE_A2DP_SUPPORT__ */

#define BT_SINK_SRV_MUSIC_PLAYBACK_STATE_NONE                    (0x00)
#define BT_SINK_SRV_MUSIC_PLAYBACK_STATE_STOPPED                 (0x01)
#define BT_SINK_SRV_MUSIC_PLAYBACK_STATE_PLAYING                 (0x02)

#define BT_SINK_STATE_MANAGER_STATE_COUNT_INVALID 0
#define BT_SINK_STATE_MANAGER_STATE_COUNT_1ST 1

typedef uint8_t bt_sink_srv_music_playback_state_t;

typedef struct {
    uint32_t asi;
    bt_clock_t clock;
    uint32_t ratio;
    uint32_t samples;
} bt_sink_srv_music_data_info_t;

typedef struct {
    bt_sink_srv_music_state_t state;
    bt_bd_addr_t dev_addr;
    uint16_t conn_bit;
    uint32_t flag;                                          /**< Device flag */
    uint32_t op;                                            /**< Operation flag */
    uint32_t avrcp_flag;                                    /**< avrcp related flag*/
    audio_src_srv_handle_t *handle;                         /**< Pseudo device handle */

    bt_sink_srv_am_media_handle_t med_handle;
    bt_a2dp_codec_capability_t codec;
    /* A2DP members */
    uint32_t a2dp_hd;
    bt_a2dp_role_t role;
    /* AVRCP members */
    uint32_t avrcp_hd;
    bt_avrcp_operation_id_t avrcp_cmd_action_id;
    uint32_t avrcp_push_release_timer_id;
    uint32_t avrcp_cmd_timer_id;
    uint32_t avrcp_browse_hd;
    uint8_t avrcp_status;
    uint8_t a2dp_status;
    bool volume_change_status;   /* record SP whether register volume change notification */
    bt_avrcp_operation_id_t last_play_pause_action;
    bt_avrcp_operation_id_t last_wear_action;
    bt_avrcp_operation_id_t operation_action;
    bt_avrcp_operation_state_t operation_state;
    uint32_t avrcp_volume;
    uint8_t aws_state;
    bt_sink_srv_avrcp_get_folder_items_parameter_t folder_parameter;
    /* AWS members */
#if defined(__BT_AWS_MCE_A2DP_SUPPORT__)
    uint32_t aws_hd;
#endif
    bt_avrcp_role_t avrcp_role;//[TD-PLAYER]
#if (BT_A2DP_TOTAL_LINK_NUM>3)
    bt_sink_srv_state_manager_play_count_t avrcp_play_count; //to recorder AVRCP Playing order
    bt_sink_srv_state_manager_play_count_t a2dp_start_count;  //to recorder A2DP start order
#endif
} bt_sink_srv_music_device_t;

typedef struct {
    bt_sink_srv_music_state_t state;
    bt_sink_srv_music_device_t *run_dev;
#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
    bt_sink_srv_music_device_t *resume_dev;
#endif

    /* Audio manager ID */
    bt_sink_srv_am_id_t a2dp_aid;
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
    bt_sink_srv_am_id_t aws_aid;
#endif

    uint8_t vol_lev; // set by user, same as NVDM
    uint8_t am_vol_lev; // AM running volume level
    uint8_t rho_flag;
#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
    uint32_t ratio;
    /* AVM related info */
    bt_clock_t bt_clk;
    uint32_t samples;
    uint32_t ts;
    uint32_t us_asi;
    uint32_t freq;
#endif
    uint8_t packet_count;
    bt_avm_a2dp_packet_info_t packet_list[BT_SINK_SRV_MAX_OPTIMIZE_MEDIA_PACKET_COUNT];
    bt_sink_srv_music_data_info_t media_data_list[BT_SINK_SRV_MAX_OPTIMIZE_MEDIA_PACKET_COUNT];
    bt_sink_srv_ctx_flag_t context_flag;

#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
    uint32_t waiting_list[BT_SINK_SRV_MUISC_DEV_COUNT];
    uint32_t waiting_list_count;
#endif
#ifdef MTK_BT_SPEAKER_ENABLE
    uint8_t update_base_count;
    uint8_t retry_send_eir_cnt;
    uint32_t retry_send_eir_cmd;
    uint8_t volume_sync_count_flag;
#endif
    bt_sink_srv_music_device_list_t played_connected_dev;
    bt_sink_srv_music_device_t sink_dev[BT_SINK_SRV_MUISC_DEV_COUNT];
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_eir;
#endif
    audio_src_srv_handle_t *resume_handler;
} bt_sink_srv_music_context_t;

typedef struct {
    uint32_t frequence;
    uint32_t ratio;
    bt_clock_t *latency_clock;
    bt_clock_t *base_clock;
    bt_avm_a2dp_packet_info_t *base_packet;
    bt_avm_a2dp_packet_info_t *cur_packet;
} bt_sink_srv_music_optimize_pta_parameter_t;

typedef struct {
    uint32_t frequence;
    uint32_t base_asi;
    uint32_t cur_asi;
    bt_clock_t *latency_clock;
    bt_clock_t *cur_recv_clock;
    bt_clock_t *pta;
} bt_sink_srv_music_optimize_pta_param_t;

typedef struct {
    uint32_t frequence;
    uint32_t gap_handle;
    bt_clock_t *pta;
    uint32_t packet_count;
    bt_sink_srv_music_data_info_t *packet_list;
} bt_sink_srv_music_set_sta_param_t;

typedef struct {
    uint16_t    music_volume;
    uint8_t     music_mode;
    uint32_t    play_order_count;
    uint8_t     local_asi_flag;
} bt_sink_srv_music_stored_data_t;

typedef struct {
    bt_bd_addr_t remote_addr;
    bt_sink_srv_music_stored_data_t dev_db;
} bt_sink_srv_music_data_list;

typedef struct {
    uint8_t volume;
    uint8_t volume_type;
} bt_sink_srv_audio_sync_music_data_t;

#ifdef __cplusplus
extern "C" {
#endif

void bt_sink_srv_music_init(void);
void bt_sink_srv_music_init_context(void);
bt_sink_srv_music_context_t *bt_sink_srv_music_get_context(void);

bt_sink_srv_music_device_t *bt_sink_srv_music_get_device(bt_sink_srv_music_device_type_t type, const void *param);

void bt_sink_srv_music_reset_device(bt_sink_srv_music_device_t *dev);

void bt_sink_srv_music_update_run_device(bt_sink_srv_music_device_t *dev);

void bt_sink_srv_music_state_machine_handle(bt_sink_srv_music_device_t *dev, uint32_t evt_id, void *param);

void BT_A2DP_MAKE_SBC_CODEC(bt_a2dp_codec_capability_t *codec,
                            bt_a2dp_role_t role,
                            uint8_t min_bit_pool, uint8_t max_bit_pool,
                            uint8_t block_length, uint8_t subband_num,
                            uint8_t alloc_method, uint8_t sample_rate,
                            uint8_t channel_mode);

void BT_A2DP_MAKE_AAC_CODEC(bt_a2dp_codec_capability_t *codec,
                            bt_a2dp_role_t role, bool vbr, bool drc, uint8_t object_type,
                            uint8_t channels, uint16_t sample_rate,
                            uint32_t bit_rate);

void BT_A2DP_MAKE_VENDOR_CODEC(bt_a2dp_codec_capability_t *codec,
                               bt_a2dp_role_t role, uint32_t vendor_id, uint16_t
                               codec_id, uint8_t sample_frequency, uint8_t
                               channel_mode);
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
void BT_A2DP_MAKE_VENDOR_CODEC_2(bt_a2dp_codec_capability_t *codec,
                                 bt_a2dp_role_t role, uint32_t vendor_id, uint16_t
                                 codec_id, uint8_t sample_frequency, uint8_t
                                 mode);
#endif
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
void BT_A2DP_MAKE_VENDOR_CODEC_3(bt_a2dp_codec_capability_t *codec,
                                 bt_a2dp_role_t role, uint32_t vendor_id, uint16_t
                                 codec_id, uint8_t frame_duration, uint16_t sample_frequency);
#endif

void BT_A2DP_CONVERT_SBC_CODEC(bt_codec_capability_t *dst_codec,
                               bt_a2dp_codec_capability_t *src_codec);

void BT_A2DP_CONVERT_AAC_CODEC(bt_codec_capability_t *dst_codec,
                               bt_a2dp_codec_capability_t *src_codec);

void BT_A2DP_CONVERT_VENDOR_CODEC(bt_codec_capability_t *dst_codec,
                                  bt_a2dp_codec_capability_t *src_codec);

void bt_sink_srv_music_fill_audio_src_callback(audio_src_srv_handle_t *handle);

uint64_t bt_sink_srv_music_convert_btaddr_to_devid(bt_bd_addr_t *bd_addr);

uint8_t bt_sink_srv_get_vol_local2bt(uint8_t vol, uint8_t local_level, uint8_t bt_level);

uint8_t bt_sink_srv_get_vol_bt2local(uint8_t vol, uint8_t local_level, uint8_t bt_level);

void bt_sink_srv_music_drv_play(void *param);
void bt_sink_srv_music_drv_stop(bt_sink_srv_music_device_t *dev, uint8_t aid);
void bt_sink_srv_music_fill_am_aud_param(bt_sink_srv_am_audio_capability_t  *aud_cap,
                                         bt_a2dp_codec_capability_t *a2dp_cap, bt_a2dp_role_t role, uint16_t a2dp_mtu);
void bt_sink_srv_music_update_base_parameters_to_dsp(int16_t drift_value, uint32_t nclk, uint32_t asi_base, uint32_t asi_cur);
bt_sink_srv_audio_rtp_header_t *bt_sink_srv_music_get_media_info(uint32_t *end_addr, void *start_addr, uint32_t pre_sn, bool
                                                                 is_agent);
void bt_sink_srv_music_reject_a2dp_1M(void);
void bt_sink_srv_music_set_audio_play_en_timer(bt_clock_t *cur_clock, bt_clock_t *play_clock);
void bt_sink_srv_music_decide_play_pause_action(bt_sink_srv_music_device_t *dev);
void bt_sink_srv_music_avrcp_status_change_notify(bt_bd_addr_t *remote_addr, bt_avrcp_status_t avrcp_status);
void bt_sink_srv_music_update_audio_buffer(uint32_t buffer_size);
bt_status_t bt_sink_srv_music_fill_recevied_media_data(bt_sink_srv_music_data_info_t *media_data);
void bt_sink_srv_music_clear_media_packet_list();
void bt_sink_srv_music_trigger_play(uint32_t gap_handle, bt_sink_srv_music_device_t *dev, bt_clock_t *pta, uint32_t asi_base, uint32_t asi);

/**
 * @brief                           Function to set sink latency, if sink latency is set, it would be valid after next time to start playing.
 * @param[in] latency_us    Is the sink latency value,it`s unit is us, and it should be the integer multiple of 1250us, default value is 140000us.
 * @return                         The status to set sink latency, if return value is not BT_STATUS_SUCCESS, please check the parameter.
 */
bt_status_t bt_sink_srv_music_set_sink_latency(uint32_t latency_value);

/**
 * @brief                           Function to get sink latency.
 * @return                         The current sink latency value.
 */
uint32_t bt_sink_srv_music_get_sink_latency(void);

bt_sink_srv_music_mode_t bt_sink_srv_music_get_mode(bt_bd_addr_t *remote_addr);

bt_status_t bt_sink_srv_music_set_mode(bt_bd_addr_t *remote_addr, bt_sink_srv_music_mode_t mode);

bt_status_t bt_sink_srv_music_get_nvdm_data(bt_bd_addr_t *remote_addr, bt_sink_srv_music_data_type_t data_type, void *data);

bt_status_t bt_sink_srv_music_set_nvdm_data(bt_bd_addr_t *remote_addr, bt_sink_srv_music_data_type_t data_type, void *data);

uint32_t bt_sink_srv_get_latency(bt_bd_addr_t *remote_addr, bool is_vendor_codec, bool is_extend, bool is_speaker);

void bt_sink_srv_music_set_max_bit_pool(uint32_t max_bp);

void bt_sink_srv_music_set_music_enable(uint32_t handle, bt_avm_role_t role, bool enable);

void bt_sink_srv_update_last_device(bt_bd_addr_t *remote_addr, bt_sink_srv_profile_type_t module_type);

bt_avrcp_operation_id_t bt_sink_srv_music_get_play_pause_action(bt_sink_srv_music_device_t *dev);

uint32_t bt_sink_srv_music_find_free_timer(uint32_t start_timer_id, uint32_t end_timer_id);

bool bt_sink_srv_music_get_a2dp_nvdm_data(bt_bd_addr_t *bt_addr, void *data_p, uint32_t size);

bt_status_t bt_sink_srv_music_set_a2dp_nvdm_data(bt_bd_addr_t *bt_addr, void *data_p, uint32_t size);

void bt_sink_srv_music_set_ALC_enable(uint32_t is_enable);

uint32_t bt_sink_srv_music_get_ALC_enable(void);

void bt_sink_srv_music_device_waiting_list_operation(uint32_t *device_list, uint32_t device_count, bool is_add);

bt_sink_srv_music_playback_state_t bt_sink_srv_get_music_state();

bt_status_t bt_sink_srv_set_must_play_tone_flag(bt_bd_addr_t *sp_addr, bt_sink_srv_notification_voice_t type, bool is_open);
void bt_sink_srv_music_get_waiting_list_devices(uint32_t *device_list, uint32_t *device_count);
bt_sink_srv_state_t bt_sink_srv_music_get_music_state(bt_bd_addr_t *addr);
bt_status_t bt_sink_srv_music_audio_switch(bool switch_on, bt_sink_srv_action_t action);
void bt_sink_srv_music_stop(bt_sink_srv_music_device_t *dev, uint8_t aid);
void bt_sink_srv_music_set_interrupt_flag(uint32_t flagMask, bool val);
bt_status_t bt_sink_srv_state_manager_music_callback(bt_sink_srv_state_manager_event_t event, bt_bd_addr_t *address, void *parameter);
void bt_sink_srv_music_stop_vp_detection(audio_src_srv_handle_t *handler);
void bt_sink_srv_music_start_vp_detection(audio_src_srv_handle_t *handler, uint32_t duration_ms);
bt_status_t bt_sink_srv_music_get_codec_type(uint32_t *codec_type);
bt_status_t bt_sink_srv_music_get_lossless_enable(uint32_t *enable);
bool bt_sink_srv_music_state_change_handler(bt_sink_srv_music_device_t *change_dev,
    uint32_t old_a2dp_status, uint32_t old_avrcp_status);
bt_status_t bt_sink_srv_music_pause_remote_music(bt_sink_srv_music_device_t *dev);
void bt_sink_srv_music_set_am_volume(uint32_t vol_levl);
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
bool bt_sink_srv_music_is_lhdc_ll_mode(bt_a2dp_codec_capability_t* codec);
#endif

#ifdef __cplusplus
}
#endif

#endif

