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


#ifndef __BT_ULL_LE_SERVICE_H__
#define __BT_ULL_LE_SERVICE_H__

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_ULL Ultra Low Latency
 * @{
 * This section describes the Ultra Low Latency (ULL) LE APIs and events.
 * Clients (Ex. Headset/Earbuds) with a low wireless audio latency utilize Ultra Low Latency when they are connected with a well-matched server (Ex. Dongle).
 * This profile defines two roles: ULL_Client (Ex. Headset/Earbuds) and ULL_Server (Ex. Dongle).
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b ULL                        | Ultra Low Latency. |
 * |\b RSI                        | Resolvable Set Identifier. |
 * |\b CS                         | Coordinated Set. |
 *
 * @section bt_ull_le_srv_api_usage How to use this module
 *  - Step1: Initialize Bluetooth Ultra Low Latency LE service during system initialization (Mandatory).
 *   - Sample code in client (Ex. Headset/Earbuds):
 *    @code
 *           bt_ull_le_srv_init(BT_ULL_ROLE_CLIENT, bt_ull_le_event_callback);
 *    @endcode
 *   - Sample code in server (Ex. Dongle):
 *    @code
 *           bt_ull_le_srv_init(BT_ULL_ROLE_SERVER, bt_ull_le_event_callback);
 *    @endcode
 *  - Step2: Mandatory, implement bt_ull_le_event_callback() to handle the ULL LE events, such as connection event, user data received, etc.
 *   - Sample code:
 *    @code
 *       void bt_ull_le_event_callback(bt_ull_event_t event, void *param, uint32_t param_len)
 *       {
 *           switch (event)
 *           {
 *              case BT_ULL_EVENT_LE_CONNECTED:
 *                  printf("ULL LE Connected");
 *                  break;
 *              case BT_ULL_EVENT_LE_DISCONNECTED:
 *                  printf("ULL LE Disconnected");
 *                  break;
 *              default:
 *                  break;
 *            }
 *        }
 *    @endcode
 *
 *  - Step4: Call the function #bt_ull_le_srv_action() to control the device (Optional).
 *   - Sample code:
 *    @code
 *          bt_ull_volume_t volume_param;
 *          volume_param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
 *          volume_param.streaming.port = 0;
 *          volume_param.action = BT_ULL_VOLUME_ACTION_SET_UP;
 *          volume_param.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
 *          volume_param.volume = 1;
 *          bt_ull_le_srv_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
 *    @endcode
 */

#include "bt_type.h"
#include "bt_system.h"
#include "bt_platform.h"
#include "bt_ull_service.h"
#include "bt_ull_le.h"


BT_EXTERN_C_BEGIN

/**
 * @defgroup BluetoothServices_ULL_LE_define Define
 * @{
 * Define Bluetooth ULL LE data types and values.
 */

/**
 *  @brief Define the ULL LE MAX Link number.
 */
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE

#ifdef AIR_WIRELESS_MIC_ENABLE
#define BT_ULL_LE_MAX_LINK_NUM      4   /**< Wireless MIC supports four LE links. */
#else
#define BT_ULL_LE_MAX_LINK_NUM      2   /**< Wireless MIC supports two LE links. */
#endif

#else
#define BT_ULL_LE_MAX_LINK_NUM      1   /**< No support for ULL LE. */
#endif

/**
 *  @brief Define the length of ULL LE UUID.
 */
#define BT_ULL_LE_MAX_UUID_LENGTH       16

/**
 *  @brief Define the length of ULL LE RSI information.
 */
#define BT_ULL_LE_MAX_RSI_LENGTH        6


/**
 *  @brief Define the SDU interval of ULL dongle LE.
 */
typedef uint8_t bt_ull_le_sdu_interval_t;
#define BT_ULL_LE_SDU_INTERVAL_INVALID                    0x00    /**< Invalid sdu interval value. */
#define BT_ULL_LE_SDU_INTERVAL_1250_US                    0x01    /**< SDU interval is 1.25 ms. */
#define BT_ULL_LE_SDU_INTERVAL_2500_US                    0x02    /**< SDU interval is 2.5 ms. */
#define BT_ULL_LE_SDU_INTERVAL_5000_US                    0x03    /**< SDU interval is 5 ms. */
#define BT_ULL_LE_SDU_INTERVAL_7500_US                    0x04    /**< SDU interval is 7.5 ms. */
#define BT_ULL_LE_SDU_INTERVAL_10000_US                   0x05    /**< SDU interval is 10 ms. */
#define BT_ULL_LE_SDU_INTERVAL_1000_US                    0x06    /**< SDU interval is 1 ms. */
#define BT_ULL_LE_SDU_INTERVAL_2000_US                    0x07    /**< SDU interval is 2 ms. */
/**
 * @brief Define the type of setting ULL Dongle LE Codec.
 */
typedef uint8_t bt_ull_le_set_codec_port_t;
#define BT_ULL_LE_SET_CODEC_PORT_ALL_SAME                              0x00     /**< Downlink and Uplink have the same codec parameters. */
#define BT_ULL_LE_SET_CODEC_GAMING_SPK_PORT                            0x01     /**< Only set the codec of Speaker interface Port 0. */
#define BT_ULL_LE_SET_CODEC_CHAT_SPK_PORT                              0x02     /**< Only set the codec of Speaker interface Port 1. */
#define BT_ULL_LE_SET_CODEC_MIC_PORT                                   0x03     /**< Only set the codec of Microphone interface Port 0. */
#define BT_ULL_LE_SET_CODEC_LINE_IN_PORT                               0x04     /**< Only set the codec of Line in interface Port 0. */
#define BT_ULL_LE_SET_CODEC_LINE_OUT_PORT                              0x05     /**< Only set the codec of Line out interface Port 0. */
#define BT_ULL_LE_SET_CODEC_I2S_IN_PORT                                0x06     /**< Only set the codec of I2s in interface Port 0. */
#define BT_ULL_LE_SET_CODEC_I2S_OUT_PORT                               0x07     /**< Only set the codec of I2s out interface Port 0. */

/**
 *  @brief Define the state of ULL dongle audio stream.
 */
typedef uint8_t bt_ull_le_srv_latency_t;
#define BT_ULL_LE_SRV_LATENCY_DEFAULT        (0x00)                              /**< Default mode, ull air latency is 5 ms. */
#define BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE    BT_ULL_LE_SRV_LATENCY_DEFAULT  /**< Single link mode, ull air latency is 5 ms. */
#define BT_ULL_LE_SRV_LATENCY_MULTI_LINK_HFP_STANDBY_MODE   BT_ULL_LE_SRV_LATENCY_DEFAULT               /**< Multi link HFP standby mode, ull air latency is 5 ms. */
#define BT_ULL_LE_SRV_LATENCY_MULTI_LINK_LEA_INTERVAL_MORE_THAN_30MS_STANDBY_MODE  (0x01) /**< Multi link LE audio standby mode with more than 30 ms connection interval, ull air latency is 10 ms. */
#define BT_ULL_LE_SRV_LATENCY_MULTI_LINK_A2DP_STANDBY_MODE  (0x02)               /**< Multi link A2DP standby mode, ull air latency is 15 ms. */
#define BT_ULL_LE_SRV_LATENCY_MULTI_LINK_CONNECTING_MODE    (0x03)               /**< Multi link connecting mode, ull air latency is 20 ms. */
#define BT_ULL_LE_SRV_LATENCY_MULTI_LINK_LEA_7_5MS_30MS_STANDBY_MODE    BT_ULL_LE_SRV_LATENCY_MULTI_LINK_A2DP_STANDBY_MODE   /**<  Multi link LE audio standby mode with 60 ms connection interval, ull air latency is 15 ms. */

/**
 * @brief Define the audio quality of ULL LE.
 */
typedef uint8_t bt_ull_le_srv_audio_quality_t;
#define BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID        0x00                     /**< Invalid audio quality */ 
#define BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT        0x01                     /**< DL: sampling rate: 96K, bit rate: 304K, UL: sampling rate: 16K, 32K or 48K, bit rate: 64K or 104K*/
#define BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_A2DP  0x02                     /**< DL: sampling rate: 96K, bit rate: 250K, UL: sampling rate: N/A, have not uplink */
#define BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP   0x03                     /**< DL: sampling rate: 96K, bit rate: 250K, UL: sampling rate: N/A, have not uplink */
#define BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY        0x04                     /**< DL: sampling rate: 96K, bit rate: 560K, UL: sampling rate: 16K, 32K or 48K, bit rate: 104K */
#define BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION     0x05                     /**< DL: sampling rate: 96K, bit rate: 940K, UL: sampling rate: 16K, 32K or 48K, bit rate: 104K */
#define BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER           0x06                     /**< DL: sampling rate: 96K, bit rate: 940K, UL: sampling rate: 16K, 32K or 48K, bit rate: 104K */

/**
 * @brief Define the codec type of ULL Dongle LE.
 */
typedef uint8_t bt_ull_le_codec_t;
#define BT_ULL_LE_CODEC_LC3PLUS                0x00      /**< LC3Plus codec type. */
#define BT_ULL_LE_CODEC_OPUS                   0x01      /**< Opus codec type. */
#define BT_ULL_LE_CODEC_VENDOR                 0x02      /**< Vendor codec type. */
#define BT_ULL_LE_CODEC_ULD                    0x04      /**< ULD codec type. */
#define BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS      0x05      /**< DL ULD codec type,UL LC3Plus codec type.*/
#define BT_ULL_LE_CODEC_DL_ULD_UL_OPUS         0x06      /**< DL ULD codec type,UL OPUS codec type.*/

/**
 * @brief Define the channel mode of ULL Dongle LE.
 */
typedef uint8_t bt_ull_le_channel_mode_t;
#define BT_ULL_LE_CHANNEL_MODE_MONO        0x01          /**< Mono mode. */
#define BT_ULL_LE_CHANNEL_MODE_STEREO      0x02          /**< Stereo mode. */
#define BT_ULL_LE_CHANNEL_MODE_DUAL_MONO   0x04          /**< Dual mono mode. */


/**
 * @brief Define the streaming mode of ULL LE.
 */
typedef uint8_t bt_ull_le_stream_mode_t;
#define BT_ULL_LE_STREAM_MODE_DOWNLINK      0x00         /**< Downlink streaming mode. */
#define BT_ULL_LE_STREAM_MODE_UPLINK        0x01         /**< Uplink streaming mode. */

/**
 * @brief Define the scenario that the client will switch.
 */
typedef uint8_t bt_ull_le_scenario_t;
#define BT_ULL_LE_SCENARIO_INVALID          0x00        /**< Invalid scenario type. */
#define BT_ULL_LE_SCENARIO_ULLV2_0          0x01        /**< Switch to ULL2.0 scenario. */
#define BT_ULL_LE_SCENARIO_ULLV3_0          0x02        /**< Switch to ULL3.0 scenario  */

/**
 *  @brief Define the type of ULL dongle LE UUID.
 */
typedef uint8_t bt_ull_le_uuid_t[BT_ULL_LE_MAX_UUID_LENGTH];

/**
 *  @brief Define the type of ULL dongle LE RSI.
 */
typedef uint8_t bt_ull_le_rsi_t[BT_ULL_LE_MAX_RSI_LENGTH];


/**
 * @}
 */

/**
 * @defgroup BluetoothServices_ULL_LE_struct Struct
 * @{
 * Defines Ultra Low Latency structures for associated APIs and events.
 */


/**
 * @brief This structure defines the parameters of ULL Dongle LE Codec.
 */
typedef struct {
    bt_ull_le_sdu_interval_t sdu_interval;      /**< SDU interval(us). */
    uint8_t rtn;                                /**< Retransmission number, Max_Share_num. */
    uint8_t latency;                            /**< Max transport latency (ms). */
    bt_ull_le_channel_mode_t  channel_mode;     /**< b0: Mono, b1: Stereo, b2: dual channel. */
    uint16_t sdu_size;                          /**< Maximum SDU size (octets). */
    uint32_t bit_rate;                          /**< Bitrate (bps). */
    uint32_t sample_rate;                       /**< Codec sample rate (unit: HZ, e.g., 16000,32000,48000, 96000...). */
} bt_ull_le_codec_param_t;

/**
 * @brief Define the parameters of Find CS Request.
 */
typedef struct {
    uint16_t             duration;              /**< The duration of the ULL pairing process. unit: second. */
    uint16_t             conn_handle;           /**< The LE connection handle. */
} bt_ull_le_find_cs_info_t;

/**
 * @brief Define the device information of ULL Client.
 */
typedef struct {
    bt_ull_client_t client_type;                                       /**< The ULL client type.*/
    uint8_t         size;                                              /**< The size of ULL LE Coordinated set. */
    bt_key_t        sirk;                                              /**< The SIRK of ULL LE Coordinated set. */
    bt_bd_addr_t    local_random_addr;                                 /**< The random address of ULL LE. */
#ifdef AIR_WIRELESS_MIC_ENABLE
    bt_ull_le_channel_mode_t  client_preferred_channel_mode;           /**< The channel mode only used for wireless mic*/
#else
    bt_ull_le_codec_t  client_preferred_codec_type;                    /**< The client preferred codec type only used for ull v2*/
#endif
    bt_addr_t       group_device_addr[BT_ULL_LE_MAX_LINK_NUM - 1];     /**< The remote device address that is in the same group.*/
} bt_ull_le_device_info_t;

/**
 *  @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_CONNECTED
 *         which indicates the result of ULL LE connect.
 */
typedef struct {
    bt_status_t status;                           /**< The ULL LE connection status.*/
    bt_ull_client_t client_type;                  /**< The ULL client type.*/
    uint8_t set_size;                             /**< The size of ULL LE Coordinated set. */
    uint16_t conn_handle;                         /**< The LE connection handle. */
    bt_key_t sirk;                                /**< The SIRK of ULL LE Coordinated set. */
    bt_addr_t group_device_addr[BT_ULL_LE_MAX_LINK_NUM - 1]; /**< The device address that is in the same group.*/
} bt_ull_le_connected_info_t;

/**
 *  @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_DISCONNECTED
 *         which indicates the result of ULL LE disconnect.
 */
typedef struct {
    uint16_t conn_handle;                         /**< The LE connection handle. */
} bt_ull_le_disconnect_info_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_STREAMING_START_IND
 *         which indicates the result of ULL LE streaming is started.
 */
typedef struct {
    union {
        bt_ull_le_stream_mode_t stream_mode;    /**< The streaming mode. */
        bt_ull_streaming_t      stream;         /**< The streaming info. */
    };
} bt_ull_le_streaming_start_ind_t;


/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_STREAMING_STOP_IND
 *         which indicates the result of ULL LE streaming is stopped.
 */
typedef bt_ull_le_streaming_start_ind_t bt_ull_le_streaming_stop_ind_t;




/**
 * @brief This structure defines the parameters of adaptive bitrate.
 */
typedef struct {
    uint8_t     enable;                                 /**< Enable or disable the adaptive bitrate mode. */
    uint16_t    report_interval;                        /**< The interval of the Quality of Service report, unit: 10ms, value range: 10 ~ 100. */
    uint16_t    crc_threshold;                          /**< The report threshold of CRC error . */
    uint16_t    rx_timeout_threshold;                   /**< The report threshold of RX timeout error . */
    uint8_t     flush_timeout_threshold;                /**< The report threshold of flush timeout error . */
    uint16_t    resume_time;                            /**< The time of automatic resume high audio quality . */
} bt_ull_le_adaptive_bitrate_params_t;

/**
 * @}
 */


/**
 * @brief   This function is used for application to initialize Ultra Low Latency LE service.
 * @param[in] role        is Ultra Low Latency server or client.
 * @param[in] callback    is callback function.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_init(bt_ull_role_t role, bt_ull_callback callback);

/**
 * @brief   This function is used to control ULL LE device.
 * @param[in] action      is the user request action type.
 * @param[in] param       is the payload of the request action.
 * @param[in] param_len   is the payload length.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len);

/**
 * @brief   This function is used to get specified streaming info.
 * @param[in] streaming   streaming type.
 * @param[out] info       The detail streaming info.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info);

/**
 * @brief   This function is used to stop or start the streaming.
 * @param[in] lock        bool type. true: stop the dongle streaming; false: start the dongle streaming.
 * @return                None.
 */
void bt_ull_le_srv_lock_streaming(bool lock);

/**
 * @brief   This function gets the UUID of ULL Dongle LE.
 * @return                An UUID pointer, the operation completed successfully.
 *                        NULL pointer, the operation has failed.
 */
const bt_ull_le_uuid_t *bt_ull_le_srv_get_uuid(void);

/**
 * @brief   This function is used to get the RSI.
 * @param[out] rsi        is the RSI value.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_get_rsi(bt_ull_le_rsi_t rsi);

/**
 * @brief   This function is used to verify the RSI.
 * @param[out] rsi        is the RSI value.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_verify_rsi(bt_ull_le_rsi_t rsi);

/**
 * @brief   This function sets the device information of ULL Client.
 * @param[in] dev_info    is the device information of ULL Client device.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_set_device_info(bt_ull_le_device_info_t *dev_info);

/**
 * @brief   This function gets the role of ULL Dongle LE.
 * @return                #bt_ull_role_t, the role of ULL Dongle LE service.
 */
bt_ull_role_t bt_ull_le_srv_get_role(void);

/**
 * @brief   This function set the access address for adv and scan.
 * @param[in] access_addr     the access address parameters.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                        #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_srv_set_access_address(bt_ull_le_set_adv_scan_access_addr_t *access_addr);

/**
 * @brief   This function enable/disable adaptive bitrate mode.
 * @param[in] adaptive_bitrate_param       the parameters of adaptive bitrate.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 */
bt_status_t bt_ull_le_srv_enable_adaptive_bitrate_mode(bt_ull_le_adaptive_bitrate_params_t *adaptive_bitrate_param);

BT_EXTERN_C_END

/**
 * @}
 * @}
 */

#endif /* __BT_ULL_LE_SERVICE_H__ */

