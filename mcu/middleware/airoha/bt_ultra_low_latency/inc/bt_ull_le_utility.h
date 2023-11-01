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


#ifndef __BT_ULL_LE_UTILITY_H__
#define __BT_ULL_LE_UTILITY_H__


#include "bt_type.h"
#include "bt_system.h"
#include "bt_platform.h"
#include "bt_ull_utility.h"
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
/* Hal includes. */
#ifdef AIR_USB_AUDIO_ENABLE
#include "hal_usb.h"
#endif
#ifdef AIR_USB_AUDIO_ENABLE
#include "usbaudio_drv.h"
#endif

#ifdef AIR_USB_ENABLE
#include "usb_main.h"
#endif



BT_EXTERN_C_BEGIN

//#define BT_ULL_LE_THROUGHPUT_DEBUG
#ifdef BT_ULL_LE_THROUGHPUT_DEBUG
#include "hal_gpt.h"
#endif


/**
 *  @brief Define the ULL Client LE MAX Link number.
 */
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#define BT_ULL_LE_CLIENT_LINK_MAX_NUM      1   /**< Headset and earbuds Not support Multi-ULL. */
#else
#define BT_ULL_LE_CLIENT_LINK_MAX_NUM      0   /**< Not support ULL LE. */
#endif

/**
 *  @brief Define the ULL LE AirCis MAX Link number.
 */
#ifdef AIR_WIRELESS_MIC_ENABLE
#define BT_ULL_LE_AIR_CIS_MAX_NUM  4
#else
#define BT_ULL_LE_AIR_CIS_MAX_NUM  2
#endif


/**
 *  @brief Define the state of ULL dongle audio stream.
 */
typedef uint8_t bt_ull_le_srv_stream_state_t;
#define BT_ULL_LE_SRV_STREAM_STATE_IDLE                            0x00
#define BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM              0x01 /**< Audio Stream is Starting.*/
#define BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER               0x02 /**< Set CIG ongoing. */
#define BT_ULL_LE_SRV_STREAM_STATE_STREAMING                       0x03 /**< Sreaming. */
#define BT_ULL_LE_SRV_STREAM_STATE_MAX                             0x04

typedef uint8_t bt_ull_le_srv_audio_out_t;

#define BT_ULL_LE_SRV_AUDIO_OUT_USB         (0x00)
#define BT_ULL_LE_SRV_AUDIO_OUT_AUX         (0x01)
#define BT_ULL_LE_SRV_AUDIO_OUT_I2S         (0x02)

#define BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_1        (1)
#define BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2        (2)
#define BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_3        (3)
#define BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_4        (4)
#define BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY  (5)

#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_100  (100)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_108  (108)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_190  (190)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_126  (126)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_63   (63)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_200  (200)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_158  (158)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_350  (350)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_588  (588)
#define BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_170  (170)

#define BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_20     (20)
#define BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_25     (25)
#define BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40     (40)
#define BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65     (65)

#define BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS   (64000)
#define BT_ULL_LE_DEFAULT_BITRATE_32_0_KBPS   (32000)
#define BT_ULL_LE_DEFAULT_BITRATE_100_8_KBPS  (100800)
#define BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS  (104000)
#define BT_ULL_LE_DEFAULT_BITRATE_160_0_KBPS  (160000)
#define BT_ULL_LE_DEFAULT_BITRATE_172_8_KBPS  (172800)
#define BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS  (201600)
#define BT_ULL_LE_DEFAULT_BITRATE_252_8_KBPS  (252800)
#define BT_ULL_LE_DEFAULT_BITRATE_256_0_KBPS  (256000)
#define BT_ULL_LE_DEFAULT_BITRATE_272_0_KBPS  (272000)
#define BT_ULL_LE_DEFAULT_BITRATE_300_0_KBPS  (300000)
#define BT_ULL_LE_DEFAULT_BITRATE_304_0_KBPS  (304000)
#define BT_ULL_LE_DEFAULT_BITRATE_320_0_KBPS  (320000)
#define BT_ULL_LE_DEFAULT_BITRATE_200_0_KBPS  (200000)
#define BT_ULL_LE_DEFAULT_BITRATE_400_0_KBPS  (400000)
#define BT_ULL_LE_DEFAULT_BITRATE_560_0_KBPS  (560000)
#define BT_ULL_LE_DEFAULT_BITRATE_940_8_KBPS  (940800)


#define BT_ULL_LE_DEFAULT_SAMPLERTE_16K       (16000)
#define BT_ULL_LE_DEFAULT_SAMPLERTE_32K       (32000)
#define BT_ULL_LE_DEFAULT_SAMPLERTE_48K       (48000)
#define BT_ULL_LE_DEFAULT_SAMPLERTE_96K       (96000)

/**
 * @brief Defines for audio locations.
 */
#define BT_ULL_LE_AUDIO_LOCATION_NONE                     0x00000000  /**< Mono/Unspecified.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT               0x00000001  /**< Front Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT              0x00000002  /**< Front Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_CENTER             0x00000004  /**< Front Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_LOW_FREQ_EFFECT1         0x00000008  /**< Low Frequency Effects 1.*/
#define BT_ULL_LE_AUDIO_LOCATION_BACK_LEFT                0x00000010  /**< Back Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_BACK_RIGHT               0x00000020  /**< Back Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT_OF_CENTER     0x00000040  /**< Front Left of Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT_OF_CENTER    0x00000080  /**< Front Right of Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_BACK_CENTER              0x00000100  /**< Back Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_LOW_FREQ_EFFECT2         0x00000200  /**< Low Frequency Effects 2.*/
#define BT_ULL_LE_AUDIO_LOCATION_SIDE_LEFT                0x00000400  /**< Side Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_SIDE_RIGHT               0x00000800  /**< Side Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_FRONT_LEFT           0x00001000  /**< Top Front Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_FRONT_RIGHT          0x00002000  /**< Top Front Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_FRONT_CENTER         0x00004000  /**< Top Front Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_CENTER               0x00008000  /**< Top Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_BACK_LEFT            0x00010000  /**< Top Back Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_BACK_RIGHT           0x00020000  /**< Top Back Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_SIDE_LEFT            0x00040000  /**< Top Side Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_SIDE_RIGHT           0x00080000  /**< Top Side Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_TOP_BACK_CENTER          0x00100000  /**< Top Back Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_BOTTOM_FRONT_CENTER      0x00200000  /**< Bottom Front Center.*/
#define BT_ULL_LE_AUDIO_LOCATION_BOTTOM_FRONT_LEFT        0x00400000  /**< Bottom Front Left.*/
#define BT_ULL_LE_AUDIO_LOCATION_BOTTOM_FRONT_RIGHT       0x00800000  /**< Bottom Front Right.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT_WIDE          0x01000000  /**< Front Left Wide.*/
#define BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT_WIDE         0x02000000  /**< Front Right Wide.*/
#define BT_ULL_LE_AUDIO_LOCATION_LEFT_SURROUND            0x04000000  /**< Left Surround.*/
#define BT_ULL_LE_AUDIO_LOCATION_RIGHT_SURROUND           0x08000000  /**< Right Surround.*/
typedef uint32_t bt_ull_le_audio_location_t;              /**< The type of audio location.*/

/**
 *  @brief Define the link state of ULL Dongle.
 */
typedef uint8_t bt_ull_le_link_state_t;
#define BT_ULL_LE_LINK_STATE_IDLE                               0x00
#define BT_ULL_LE_LINK_FINDING_CS_INFO                          0x01 /**< Finding Coordinated info. */
#define BT_ULL_LE_LINK_STATE_CONFIGURATION                      0x02 /**< Sync info and Codec. */
#define BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO                    0x03 /**< Sync info and Codec. */
#define BT_ULL_LE_LINK_STATE_READY                              0x04 /**< Sync info and Codec Configued. */
#define BT_ULL_LE_LINK_STATE_CREATING_CIS                       0x05 /**< Creating CIS. */
#define BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH                  0x06 /**< Setting ISO Data Path. */
#define BT_ULL_LE_LINK_STATE_STREAMING                          0x07 /**< CIS set done, is Streaming. */
#define BT_ULL_LE_LINK_STATE_MAX                                0x08

typedef enum {
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID = 0x00,
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,                /* Left, for media streaming, cis_info[0] */
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,               /* Right, for media streaming, cis_info[1]*/
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3,                /* for mic streaming, cis_info[0] */
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK4,                /* for mic streaming, cis_info[1]  */
    BT_ULL_LE_CONN_SRV_AIR_CIS_ID_MAX
} bt_ull_le_srv_air_cis_id_enum;

#define BT_ULL_LE_SRV_DATA_PATH_ID_HCI            (0x00)
#define BT_ULL_LE_SRV_DATA_PATH_ID_1              (0x01)
#define BT_ULL_LE_SRV_DATA_PATH_ID_2              (0x02)
// for wireless mic
#define BT_ULL_LE_SRV_DATA_PATH_ID_3              (0x03)
#define BT_ULL_LE_SRV_DATA_PATH_ID_4              (0x04)
#define BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL    (0x30)
#define BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE        (0xFF)
typedef uint8_t bt_ull_le_srv_iso_data_path_t;

#define BT_ULL_LE_SRV_ISO_INTERVAL_1P25MS         (0x01)
#define BT_ULL_LE_SRV_ISO_INTERVAL_2P5MS          (0x02)
#define BT_ULL_LE_SRV_ISO_INTERVAL_5MS            (0x04)
#define BT_ULL_LE_SRV_ISO_INTERVAL_7P5MS          (0x06)
#define BT_ULL_LE_SRV_ISO_INTERVAL_10MS           (0x08)
#define BT_ULL_LE_SRV_ISO_INTERVAL_15MS           (0x0C)
#define BT_ULL_LE_SRV_ISO_INTERVAL_20MS           (0x10)
typedef uint16_t bt_ull_le_srv_iso_interval_t; /* unit is 1.25 ms*/

#define BT_ULL_LE_SRV_PHY_LE_1M                      0x01
#define BT_ULL_LE_SRV_PHY_LE_2M                      0x02
#define BT_ULL_LE_SRV_PHY_LE_CODED                   0x04
#define BT_ULL_LE_SRV_PHY_EDR_LE_4M                  0x40
#define BT_ULL_LE_SRV_PHY_EDR_LE_6M                  0x41
#define BT_ULL_LE_SRV_PHY_EDR_LE_8M                  0x42
#define BT_ULL_LE_SRV_PHY_EDR_LE_12M                 0x43

#define BT_ULL_LE_CONN_SRV_PHY_LE_1M                      0x01
#define BT_ULL_LE_CONN_SRV_PHY_LE_2M                      0x02
#define BT_ULL_LE_CONN_SRV_PHY_LE_CODED                   0x04
#define BT_ULL_LE_CONN_SRV_PHY_EDR_LE_4M                  0x40
#define BT_ULL_LE_CONN_SRV_PHY_EDR_LE_6M                  0x41
#define BT_ULL_LE_CONN_SRV_PHY_EDR_LE_8M                  0x42
#define BT_ULL_LE_CONN_SRV_PHY_EDR_LE_12M                 0x43
#define BT_ULL_LE_CONN_SRV_PHY_LE_4M                      0x80

typedef uint8_t bt_ull_le_srv_phy_t;

typedef uint8_t bt_ull_le_restart_streaming_reason_t;
#define BT_ULL_LE_RESTART_STREAMING_REASON_LATEBCY_CHANGE       0x00
#define BT_ULL_LE_RESTART_STREAMING_REASON_ALLOW_PALY           0x01
#define BT_ULL_LE_RESTART_STREAMING_REASON_RESERVED             0x02
#define BT_ULL_LE_RESTART_STREAMING_REASON_AIRCIS_RECONNECT     0x03

/**
 * @brief This structure defines the codec info ULL Dongle.
 */
typedef struct {
    bt_ull_le_codec_t codec_type;
    bt_ull_le_codec_param_t codec_param;
} bt_ull_le_codec_info_t;

typedef struct {
    bool     enable;
} bt_ull_le_srv_activate_ul_t;

typedef struct {
    uint16_t dchs_support : 1; /*DCHS feature enable*/
    uint16_t high_res_support : 1;   /*high resolution audio enable*/
    uint16_t ull_v3_support : 1;   /*ULL v3 enable*/
    uint16_t rfu : 13;        /*reserved for future use*/
} bt_ull_le_srv_capability_t;

typedef struct {
    uint8_t                    cis_id;
    uint32_t                   audio_location;
    bt_ull_le_srv_capability_t capability_msk;
    uint8_t                    aws_connected;
} bt_ull_le_srv_configuration_t;

/**
 * @brief This structure defines the context of ULL Dongle.
 */

typedef struct {
    bt_ull_role_t                    role;
    bt_ull_client_t                  client_type;
    uint8_t                          cs_size;
    uint8_t                          cis_num;
    bt_ull_le_channel_mode_t         client_preferred_channel_mode;/**< only used for wireless mic */
    bt_ull_le_codec_t                client_preferred_codec_type;  /**<client preferred_codec_type*/
    bool                             is_cig_created;
    bool                             is_share_buff_set;
    bool                             is_disable_sniff;          /**< ull link sniff mode is disabled or not */
    bool                             is_removing_cig_for_switch_latency;
    bool                             is_removing_cig_for_change_aud_quality;
    bool                             is_removing_cig_for_change_aud_codec;
    bool                             is_streaming_locked;
    uint8_t                          restart_streaming_mask;
    bt_ull_le_srv_stream_state_t     curr_stream_state;         /**< Current audio stream state */
    bt_ull_callback                  callback;
    bt_key_t                         sirk;                      /**<  little endian */
    uint8_t                          critical_data_max_len;
    uint8_t                          critical_data_tx_seq;      /**< valid seq: 1 ~ 255 */
    uint8_t                          critical_data_rx_seq;      /**< valid seq: 1 ~ 255 */
    bt_bd_addr_t                     local_random_addr;         /**< The random address of ULL LE.*/
    uint32_t                         audio_location;
    bt_ull_le_srv_client_preferred_codec_param  client_preferred_codec_param;  /**< store client preferred codec param.*/
    bt_ull_le_adaptive_bitrate_params_t     adaptive_bitrate_param;
    bt_ull_le_scenario_t                    client_preferred_scenario_type;  /**<store client preferred scenario type*/
} bt_ull_le_srv_context_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_ACTION_CLIENT_ENTER_SMART_CHARGER_NOTIFY
 *         which indicates the client enter smart charger.
 */
typedef struct {
    bool    is_need_switch_ul;
} bt_ull_le_client_switch_ul_ind_t;


typedef struct {
    uint8_t mode_mask;
    bt_ull_le_restart_streaming_reason_t reason;
} bt_ull_le_srv_notify_restart_streaming_t;


/**< Connection and Stream status information related. */
bool bt_ull_le_service_is_connected(void);
void bt_ull_le_srv_reset_stream_ctx(void);
bool bt_ull_le_srv_is_streaming(bt_ull_le_stream_mode_t stream_mode);
bt_ull_le_srv_capability_t *bt_ull_le_srv_get_peer_capability(bt_handle_t acl_handle);

/**< Memory operation related. */
void *bt_ull_le_srv_memcpy(void *dest, const void *src, uint32_t size);
void *bt_ull_le_srv_memory_alloc(uint16_t size);
void bt_ull_le_srv_memory_free(void *point);
void *bt_ull_le_srv_memset(void *buf, uint8_t ch, uint32_t size);
int32_t bt_ull_le_srv_memcmp(const void *buf1, const void *buf2, uint32_t size);

/**< Set/Get ULL device information related. */
bool bt_ull_le_srv_sirk_is_null(bt_key_t sirk);
bt_status_t bt_ull_le_srv_set_sirk(bt_key_t sirk);
bt_key_t *bt_ull_le_srv_get_sirk(void);
void bt_ull_le_srv_set_client_type(bt_ull_client_t client_type);
bt_ull_client_t bt_ull_le_srv_get_client_type(void);
bt_status_t bt_ull_le_srv_set_coordinated_set_size(uint8_t size);
uint8_t bt_ull_le_srv_get_coordinated_set_size(void);
bt_status_t bt_ull_le_srv_set_group_device_addr(bt_addr_t *addr);

/**< Share avm buffer related. */
bt_status_t bt_ull_le_srv_init_share_info(bt_ull_client_t client_type);
void bt_ull_le_srv_deinit_share_info(void);
bt_status_t bt_ull_le_srv_set_avm_share_buffer(bt_ull_role_t role, bt_ull_client_t client_type, uint8_t link_count);
uint32_t bt_ull_le_srv_get_avm_share_buffer_address(bt_ull_client_t client_type, bt_ull_le_srv_audio_out_t out_type, bt_ull_transmitter_t transmitter, uint8_t link_index);

/**< Get Codec params related. */
bt_ull_le_codec_t bt_ull_le_srv_get_codec_type(void);
void bt_ull_le_srv_switch_codec_params(bt_ull_role_t role, uint8_t index);
uint32_t bt_ull_le_srv_get_codec_sample_rate(bt_ull_transmitter_t transmitter, bool is_uplink, bt_ull_role_t role);
uint32_t bt_ull_le_srv_get_usb_sample_rate(bt_ull_transmitter_t transmitter, bt_ull_role_t role);
uint32_t bt_ull_le_srv_get_usb_sample_size(bt_ull_transmitter_t transmitter, bt_ull_role_t role);
uint32_t bt_ull_le_srv_get_usb_sample_channel(bt_ull_transmitter_t transmitter, bt_ull_role_t role);
uint32_t bt_ull_le_srv_get_sdu_interval(bool is_uplink, bt_ull_role_t role);
uint16_t bt_ull_le_srv_get_sdu_size(bool is_uplink, bt_ull_role_t role);
uint32_t bt_ull_le_srv_get_bitrate(bool is_uplink, bt_ull_role_t role);
#ifdef AIR_WIRELESS_MIC_RX_ENABLE
uint8_t bt_ull_le_srv_get_air_cis_count(void);
#endif
bt_ull_le_channel_mode_t bt_ull_le_srv_get_channel_mode(bt_ull_transmitter_t transmitter, bool is_uplink, bt_ull_role_t role);
bt_ull_le_codec_param_t *bt_ull_le_srv_get_codec_param(bt_ull_role_t role, bt_ull_transmitter_t transmitter);


bt_ull_le_srv_stream_context_t *bt_ull_le_srv_get_stream_context(void);

bt_status_t bt_ull_le_srv_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length);
uint32_t bt_ull_le_srv_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length);
bool bt_ull_le_srv_is_transmitter_start(bt_ull_streaming_t *streaming);
bt_ull_le_audio_location_t bt_ull_le_srv_get_audio_location_by_handle(bt_handle_t handle);
void bt_ull_le_srv_init_audio_location(uint32_t audio_location);
bool bt_ull_le_srv_check_ul_activate_state_by_handle(bt_handle_t acl_handle);

/**
 * @brief   This function sets the codec parameters of ULL Dongle.
 * @param[in] codec       is the codec info.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_set_codec_info(bt_ull_le_set_codec_port_t port, bt_ull_le_codec_t codec_type, bt_ull_le_codec_param_t *codec_param);

void bt_ull_le_srv_set_opus_codec_param(void);

#ifdef AIR_WIRELESS_MIC_ENABLE
void bt_ull_le_srv_set_wireless_mic_codec_param(bt_ull_role_t role);
#endif

void bt_ull_le_srv_set_client_preferred_codec_type(bt_ull_le_codec_t codec_type);
bt_ull_le_codec_t bt_ull_le_srv_get_client_preferred_codec_type(void);
bt_ull_le_codec_t bt_ull_le_srv_get_client_preffered_codec(bt_ull_le_scenario_t mode_type);

uint32_t bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(bt_ull_le_codec_t codec_type);
uint32_t bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(bt_ull_le_codec_t codec_type);

void bt_ull_le_srv_set_codec_param_by_sample_rate(bt_ull_role_t role, uint32_t dl_samplerate, uint32_t ul_samplerate);
bt_ull_le_srv_client_preferred_codec_param* bt_ull_le_srv_get_client_preferred_codec_param(void);

#ifdef AIR_DCHS_MODE_ENABLE
void bt_ull_le_srv_set_client_preffered_dl_codec_samplerate(uint32_t samplerate);
#endif


#ifdef AIR_WIRELESS_MIC_RX_ENABLE
bt_status_t bt_ull_le_srv_set_audio_connection_info(bt_ull_streaming_t *streaming, void *audio_connection_info, uint32_t size);
bt_status_t bt_ull_le_srv_set_safety_mode_volume(bt_ull_streaming_t *streaming, S32 left_vol_diff, S32 right_vol_diff);
uint8_t bt_ull_le_srv_get_streaming_volume(bt_ull_streaming_t *streaming);

#endif
bool bt_ull_le_srv_get_connected_addr_by_link_index(uint8_t index, bt_bd_addr_t *addr);

uint8_t bt_ull_le_srv_get_connected_link_index_by_addr(bt_bd_addr_t *addr);
bt_status_t bt_ull_le_srv_active_streaming(void);
bt_status_t bt_ull_le_srv_deactive_streaming(void);
void bt_ull_le_srv_notify_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason);

#ifdef AIR_SILENCE_DETECTION_ENABLE
void bt_ull_le_srv_silence_detection_notify_client_status(bool is_silence, bt_ull_transmitter_t transmitter_type);
#endif


void bt_ull_le_srv_event_callback(bt_ull_event_t event, void *param, uint32_t len);
bt_status_t bt_ull_le_srv_send_data(uint16_t handle, uint8_t *packet, uint16_t packet_size);
bt_ull_le_srv_context_t *bt_ull_le_srv_get_context(void);
bt_handle_t bt_ull_le_srv_get_connection_handle_by_index(uint8_t idx);
bt_ull_le_link_state_t bt_ull_le_srv_get_link_state_by_index(uint8_t idx);
bt_ull_le_srv_audio_quality_t bt_ull_le_srv_read_aud_quality_from_nvkey(void);
bt_status_t bt_ull_le_srv_write_aud_quality_to_nvkey(bt_ull_le_srv_audio_quality_t quality);
uint32_t bt_ull_le_srv_get_ul_sample_rate(void);
uint32_t bt_ull_le_srv_get_dl_sample_rate(void);

bt_ull_transmitter_t bt_ull_le_srv_get_transmitter_by_stream_interface(bt_ull_streaming_t *streaming);
bt_status_t bt_ull_le_srv_get_stream_by_transmitter(bt_ull_transmitter_t transmitter, bt_ull_streaming_t *streaming);
bt_ull_le_stream_port_mask_t bt_ull_le_srv_get_stream_port_by_transmitter(bt_ull_transmitter_t transmitter);
bt_ull_transmitter_t bt_ull_le_srv_get_transmitter_by_stream_port(bt_ull_le_stream_port_mask_t stream_port);
bt_ull_le_srv_audio_out_t bt_ull_le_srv_get_audio_out_by_stream_port(bt_ull_le_stream_port_mask_t stream_port);
bt_ull_le_stream_mode_t bt_ull_le_srv_get_stream_mode(bt_ull_transmitter_t transmitter);

void *bt_ull_le_srv_get_stream_info(bt_ull_role_t role, bt_ull_transmitter_t transmitter);
bool bt_ull_le_srv_is_any_streaming_started(bt_ull_role_t role);
void bt_ull_le_srv_set_transmitter_is_start(bt_ull_transmitter_t transmitter_type, bool is_start);


BT_EXTERN_C_END



#endif  /*__BT_ULL_LE_UTILITY_H__*/

