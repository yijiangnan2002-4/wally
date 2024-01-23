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


#ifndef __BT_ULL_SERVICE_H__
#define __BT_ULL_SERVICE_H__

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_ULL Ultra Low Latency
 * @{
 * This section describes the Ultra Low Latency (ULL) APIs and events.
 * Ultra Low Latency utilizes for client (Ex. Headset/Earbuds) with a low wireless audio latency when it is connected with well-matched server (Ex. Dongle).
 * This profile defines two roles: ULL_Client (Ex. Headset/Earbuds) and ULL_Server (Ex. Dongle).
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b ULL                        | Ultra Low Latency. |
 *
 * @section bt_ull_api_usage How to use this module
 *  - Step1: Mandatory, initialize Bluetooth Ultra Low Latency service during system initialization.
 *   - Sample code in client (Ex. Headset/Earbuds):
 *    @code
 *           bt_ull_init(BT_ULL_ROLE_CLIENT, bt_ull_event_callback);
 *    @endcode
 *   - Sample code in server (Ex. Dongle):
 *    @code
 *           bt_ull_init(BT_ULL_ROLE_SERVER, bt_ull_event_callback);
 *    @endcode
 *  - Step2: Mandatory, implement bt_ull_event_callback() to handle the ULL events, such as pairing completed, user data received, etc.
 *   - Sample code:
 *    @code
 *       void bt_ull_event_callback(bt_ull_event_t event, void *param, uint32_t param_len)
 *       {
 *           switch (event)
 *           {
 *              case BT_ULL_EVENT_PAIRING_STARTED_IND:
 *                  printf("ULL pairing starting");
 *                  break;
 *              case BT_ULL_EVENT_PAIRING_COMPLETE_IND:
 *                  bt_ull_pairing_complete_ind_t *ind = (bt_ull_pairing_complete_ind_t*)param;
 *                  printf("ULL pairing complete, result:0x%x", ind->result);
 *                  break;
 *              default:
 *                  break;
 *            }
 *        }
 *    @endcode
 *
 *  - Step4: Optional, Call the function #bt_ull_action() to control the device.
 *   - Sample code:
 *    @code
 *          bt_ull_volume_t volume_param;
 *          volume_param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
 *          volume_param.streaming.port = 0;
 *          volume_param.action = BT_ULL_VOLUME_ACTION_SET_UP;
 *          volume_param.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
 *          volume_param.volume = 1;
 *          bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
 *    @endcode
 */



#include "bt_type.h"
#include "bt_system.h"
#include "bt_platform.h"

BT_EXTERN_C_BEGIN

/**
 * @defgroup BluetoothServices_ULL_define Define
 * @{
 * Define Bluetooth ULL data types and values.
 */

/**
 *  @brief Define the ULL role.
 */
typedef uint8_t bt_ull_role_t;
#define BT_ULL_ROLE_UNKNOWN         0x00    /**< Unknown role. */
#define BT_ULL_ROLE_CLIENT          0x01    /**< Headset/Earbuds/WirelessMic. */
#define BT_ULL_ROLE_SERVER          0x02    /**< Dongle. */

/**
 *  @brief Define the ULL Client type.
 */
typedef uint8_t bt_ull_client_t;
#define BT_ULL_UNKNOWN_CLIENT               (0x00)   /**< The unknown client type. */
#define BT_ULL_HEADSET_CLIENT               (0x01)   /**< The headset client type. */
#define BT_ULL_EARBUDS_CLIENT               (0x02)   /**< The earbuds client type. */
#define BT_ULL_MIC_CLIENT                   (0x03)   /**< The mic client type. */
#define BT_ULL_ULD_MIC_CLIENT               (0x04)   /**< The ULD mic client type. */
#define BT_ULL_SPEAKER_CLIENT               (0x05)   /**< The speaker client type. */
#define BT_ULL_INVALID_CLIENT               (BT_ULL_SPEAKER_CLIENT + 1)   /**< The invalid client type. */

/**
 *  @brief Define the USB Dongle callback event.
 */
typedef uint32_t bt_ull_event_t;
#define BT_ULL_EVENT_PAIRING_STARTED_IND                      0x01     /**< Notify user that ULL pairing is started with NULL payload. */
#define BT_ULL_EVENT_PAIRING_COMPLETE_IND                     0x02     /**< Notify user that ULL pairing is completed, for parameter please refer to structure #bt_ull_pairing_complete_ind_t. */
/* only available for USB Dongle */
#define BT_ULL_EVENT_USB_PLAYING_IND                          0x03     /**< Notify user that USB streaming is started, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_EVENT_USB_STOP_IND                             0x04     /**< Notify user that USB streaming is stopped, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_EVENT_USB_SAMPLE_RATE_IND                      0x05     /**< Notify user the sample rate of USB streaming, for parameter please refer to structure #bt_ull_sample_rate_t. */
#define BT_ULL_EVENT_USB_VOLUME_IND                           0x06     /**< Notify user the volume of USB streaming, for parameter please refer to structure #bt_ull_volume_t. */
#define BT_ULL_EVENT_USB_MUTE_IND                             0x07     /**< Notify user that USB streaming is muted, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_EVENT_USB_UNMUTE_IND                           0x08     /**< Notify user that USB streaming is unmuted, for parameter please refer to structure #bt_ull_streaming_t. */

#define BT_ULL_EVENT_USER_DATA_IND                            0x0A     /**< Notify user that the user data is received, for parameter please refer to structure #bt_ull_user_data_t. */
#define BT_ULL_EVENT_TX_CRITICAL_USER_DATA_RESULT             0x0B     /**< Notify user transmission result of the critical data, for parameter please refer to #bt_ull_tx_critical_data_status_t. */
#define BT_ULL_EVENT_RX_CRITICAL_USER_DATA_IND                0x0C     /**< Notify user that the critical data is received, for parameter please refer to structure #bt_ull_rx_critical_user_data_t. */

#define BT_ULL_EVENT_UPLINK_START_SUCCESS                     0x0D     /**< Notify user that ULL uplink is started with NULL payload */
#define BT_ULL_EVENT_UPLINK_STOP_SUCCESS                      0x0E     /**< Notify user that ULL uplink is stopped with NULL payload */

#define BT_ULL_EVENT_LE_CONNECTED                             0x0F     /**< Notify user that ULL LE is connected */
#define BT_ULL_EVENT_LE_DISCONNECTED                          0x10     /**< Notify user that ULL LE is disconnected */
#define BT_ULL_EVENT_LE_STREAMING_START_IND                   0x11     /**< Notify user that ULL LE streaming is started */
#define BT_ULL_EVENT_LE_STREAMING_STOP_IND                    0x12     /**< Notify user that ULL LE streaming is stopped */

#define BT_ULL_EVENT_LE_CALL_STATE                            0x30     /**< Notify user that ULL LE call state */ 
#define BT_ULL_EVENT_LE_CALL_ACTION                           0x31     /**< Notify user that ULL LE call action */ 

/**
 *  @brief Define the user action.
 */
typedef uint32_t bt_ull_action_t;
#define BT_ULL_ACTION_START_STREAMING                         0x00     /**< Start streaming, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_ACTION_STOP_STREAMING                          0x01     /**< Stop streaming, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_ACTION_SET_STREAMING_VOLUME                    0x02     /**< Set streaming volume level, for parameter please refer to structure #bt_ull_volume_t. */
#define BT_ULL_ACTION_SET_STREAMING_MUTE                      0x03     /**< Mute streaming, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_ACTION_SET_STREAMING_UNMUTE                    0x04     /**< Unmute streaming, for parameter please refer to structure #bt_ull_streaming_t. */
#define BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE               0x05     /**< Set streaming sample rate, for parameter please refer to structure #bt_ull_sample_rate_t. */
#define BT_ULL_ACTION_START_PAIRING                           0x06     /**< Start ULL pairing, for parameter please refer to structure #bt_ull_pairing_info_t. */
#define BT_ULL_ACTION_STOP_PAIRING                            0x07     /**< Cancel ULL pairing, with NULL payload. */
#define BT_ULL_ACTION_SET_STREAMING_LATENCY                   0x08     /**< Set streaming latency, for parameter please refer to structure #bt_ull_latency_t. */
#define BT_ULL_ACTION_SET_STREAMING_MIX_RATIO                 0x09     /**< Set multi streaming mix ratio, for parameter please refer to structure #bt_ull_mix_ratio_t. */
#define BT_ULL_ACTION_TX_USER_DATA                            0x0A     /**< Send user data to the remote device, for parameter please refer to structure #bt_ull_user_data_t. */
#define BT_ULL_ACTION_INIT_CRITICAL_USER_DATA                 0x0B     /**< Initiate critical context, for parameter please refer to structure #bt_ull_init_critical_user_data_t. */
#define BT_ULL_ACTION_TX_CRITICAL_USER_DATA                   0x0C     /**< Send critical data to remote device with flush timeout, for parameter please refer to structure #bt_ull_tx_critical_user_data_t. */
#define BT_ULL_ACTION_USB_HID_CONTROL                         0x0D     /**< Send play/pause/next/previous action from client to control the media player on USB host, for parameter please refer to structure #bt_ull_usb_hid_control_t. */
#define BT_ULL_ACTION_SET_INTERFACE_PARAM                     0x0E     /**< Set streaming interface parameter. */
#define BT_ULL_ACTION_FIND_CS_INFO                            0x0F     /**< Start ULL LE pairing, Find CS. */
#define BT_ULL_ACTION_SET_VERSION                             0x10     /**< Set SDK version*/
#define BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE               0x11     /**< Set streaming sample size , for parameter please refer to structure #bt_ull_streaming_sample_size_t. */
#define BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL            0x12     /**< Set streaming sample channel, for parameter please refer to structure #bt_ull_streaming_sample_channel_t. */
#define BT_ULL_ACTION_SET_CLIENT_SIDETONE_SWITCH              0x13     /**< Set client uplink sidetone on or off, for parameter please refer to structure #bt_ull_client_sidetone_switch_t*/
#define BT_ULL_ACTION_SET_AUDIO_QUALITY                       0x14     /**< Set audio quality, for parameter please refer to structure #bt_ull_le_srv_audio_quality_t. */
#define BT_ULL_ACTION_SWITCH_UPLINK                           0x15     /**< Switch uplink */
#define BT_ULL_ACTION_SET_ULL_SCENARIO                        0x16     /**< Set ULL scenario, for parameter please refer to structure #bt_ull_le_scenario_t. */
#define BT_ULL_ACTION_SWITCH_AIRCIS_INACTIVE_MODE             0x17     /**< Switch Aircis inactive mode, ON or OFF */
/**
 *  @brief Define the streaming interface type.
 */
typedef uint8_t bt_ull_streaming_interface_t;
#define BT_ULL_STREAMING_INTERFACE_UNKNOWN                    0x00     /**< Unknown streaming. */
#define BT_ULL_STREAMING_INTERFACE_SPEAKER                    0x01     /**< Streaming speaker. */
#define BT_ULL_STREAMING_INTERFACE_MICROPHONE                 0x02     /**< Streaming microphone. */
#define BT_ULL_STREAMING_INTERFACE_LINE_IN                    0x03     /**< Streaming Aux. */
#define BT_ULL_STREAMING_INTERFACE_LINE_OUT                   0x04     /**< Streaming Aux. */
#define BT_ULL_STREAMING_INTERFACE_I2S_IN                     0x05     /**< Streaming I2S. */
#define BT_ULL_STREAMING_INTERFACE_I2S_OUT                    0x06     /**< Streaming I2S. */
#define BT_ULL_STREAMING_INTERFACE_MAX_NUM                    (BT_ULL_STREAMING_INTERFACE_I2S_OUT + 0x01) /**< The max number of streaming interface type. */

/**
 *  @brief Define the volume action.
 */
typedef uint8_t bt_ull_volume_action_t;
#define BT_ULL_VOLUME_ACTION_SET_UP                           0x01     /**< Set volume level up. */
#define BT_ULL_VOLUME_ACTION_SET_DOWN                         0x02     /**< Set volume level down. */
#define BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME              0x03     /**< Set absolute volume. */

/**
 *  @brief Define the audio channel type.
 */
typedef uint8_t bt_ull_audio_channel_t;
#define BT_ULL_AUDIO_CHANNEL_DUAL                             0x00     /**< Dual channel. */
#define BT_ULL_AUDIO_CHANNEL_LEFT                             0x01     /**< Left channel. */
#define BT_ULL_AUDIO_CHANNEL_RIGHT                            0x02     /**< Right channel. */

/**
 *  @deprecated Please use #bt_ull_audio_channel_t instead.
 */
typedef uint8_t bt_ull_volume_channel_t;
#define BT_ULL_VOLUME_CHANNEL_DUEL                            0x00     /**< Dual channel. */
#define BT_ULL_VOLUME_CHANNEL_DUAL                            0x00     /**< Dual channel. */
#define BT_ULL_VOLUME_CHANNEL_LEFT                            0x01     /**< Left channel. */
#define BT_ULL_VOLUME_CHANNEL_RIGHT                           0x02     /**< Right channel. */

/**
 *  @brief Define the transmission result of the critical data.
 */
typedef uint8_t bt_ull_tx_critical_data_status_t;
#define BT_ULL_TX_CRITICAL_DATA_SUCCESS                       0x00     /**< The critical data is transmitted successfully. */
#define BT_ULL_TX_CRITICAL_DATA_TIMEOUT                       0x01     /**< The critical data is flushed out and not transmitted out. */
#define BT_ULL_TX_CRITICAL_DATA_ABANDON                       0x02     /**< The critical data is abandoned and not transmitted out. */

/**
 *  @brief Define the USB HID control type.
 */
typedef uint8_t bt_ull_usb_hid_control_t;
#define BT_ULL_USB_HID_PLAY                                   0x00     /**< Play request. */
#define BT_ULL_USB_HID_PAUSE                                  0x01     /**< Pause request. */
#define BT_ULL_USB_HID_PLAY_PAUSE_TOGGLE                      0x02     /**< Play/Pause toggle request. */
#define BT_ULL_USB_HID_PREVIOUS_TRACK                         0x03     /**< Previous track request. */
#define BT_ULL_USB_HID_NEXT_TRACK                             0x04     /**< Next track request. */

/**
 * @brief Define the maximum length of ULL pairing information data.
 */
#define BT_ULL_MAX_PAIRING_INFO_LENGTH                        (16)

/**
 * @brief Define the maximum number of ULL streaming.
 */
#define BT_ULL_MAX_STREAMING_NUM                              (2)


/**
 * @}
 */

/**
 * @defgroup BluetoothServices_ULL_struct Struct
 * @{
 * Defines Ultra Low Latency structures for associated APIs and events.
 */

/**
 * @brief Define the ULL pairing parameters
 */
typedef struct {
    uint16_t             duration;                /**< The duration of do ULL pairing. unit: second */
    bt_ull_role_t        role;                    /**< The device role. */
    bt_key_t             key;                     /**< The key to encrypt ULL pairing information. */
    uint8_t              info[BT_ULL_MAX_PAIRING_INFO_LENGTH];    /**< The information of pairing. */
    int8_t               rssi_threshold;          /**< The RSSI threshold for the pairing device. */
} bt_ull_pairing_info_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_PAIRING_COMPLETE_IND
 *         which indicates the result of ULL pairing.
 */
typedef struct {
    bool                      result;             /**< The ULL pairing result. */
    bt_bd_addr_t              remote_address;     /**< The remote device address. */
} bt_ull_pairing_complete_ind_t;

/**
 * @brief This structure defines the streaming info
 */
typedef struct {
    bt_ull_streaming_interface_t       streaming_interface;    /**< The streaming type. */
    uint8_t                            port;                   /**< The streaming port number. Range: 0 ~ (#BT_ULL_MAX_STREAMING_NUM - 1) */
} bt_ull_streaming_t;

/**
 * @brief This structure defines the parameters of action #BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE
 *          which is used to set the sample rate for the streaming (speaker/microphone)
 */
typedef struct {
    bt_ull_streaming_t       streaming;         /**< The streaming type. */
    uint32_t                 sample_rate;       /**< Streaming data sample rate. (16000,44100,48000) */
} bt_ull_sample_rate_t;

/**
 * @brief This structure defines the parameters of usb streaming sample size information
 */
typedef struct {
     bt_ull_streaming_t streaming;      /**< The streaming type. */
     uint32_t           sample_size;    /**< The streaming sample size. */
} bt_ull_streaming_sample_size_t;

/**
 * @brief This structure defines the parameters of usb streaming sample channel information
 */
typedef struct {
     bt_ull_streaming_t streaming;      /**< The streaming type. */
     uint32_t           sample_channel; /**< The streaming sample channel. */
} bt_ull_streaming_sample_channel_t;

/**
 * @brief This structure defines the parameters of action #BT_ULL_ACTION_SET_STREAMING_VOLUME
 *          which is used to control the streaming (speaker/microphone) volume
 */
typedef struct {
    bt_ull_streaming_t       streaming;         /**< The streaming type. */
    bt_ull_volume_action_t   action;            /**< Volume action. */
    bt_ull_audio_channel_t   channel;           /**< Audio channel. */
    uint8_t                  volume;            /**< value for action #BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME(range: 0~100).
                                                      Or for action #BT_ULL_VOLUME_ACTION_SET_UP, #BT_ULL_VOLUME_ACTION_SET_DOWN steps (range: 1~255) */
} bt_ull_volume_t;

/**
 * @brief This structure defines the specified streaming mix ratio
 */
typedef struct {
    bt_ull_streaming_t       streaming;         /**< The streaming type. */
    uint8_t                  ratio;             /**< The mix ratio value of this streaming. (rang: 0~100)*/
} bt_ull_ratio_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_SET_STREAMING_MIX_RATIO,
 *          which is used to set multi streaming mix ratio
 */
typedef struct {
    uint8_t                  num_streaming;                         /**< The number of streaming path. */
    bt_ull_ratio_t           streamings[BT_ULL_MAX_STREAMING_NUM];  /**< The array of streaming path. */
} bt_ull_mix_ratio_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_SET_STREAMING_LATENCY,
 *          which is used to set the streaming audio playback latency
 */
typedef struct {
    bt_ull_streaming_t       streaming;         /**< The streaming type.(only support speaker streaming) */
    uint16_t                 latency;           /**< The audio latency of this streaming. <suggestion value: 20~30, 60>(unit: millisecond)*/
} bt_ull_latency_t;

/**
 * @brief This structure is the return of #bt_ull_get_streaming_info
 *          which is used to get the specified streaming detail info
 */
typedef struct {
    uint8_t                  volume_left;       /**< The left channel volume of this streaming (range: 0~100). */
    uint8_t                  volume_right;      /**< The right channel volume of this streaming (range: 0~100). */
    uint8_t                  latency;           /**< The audio latency of this streaming.(unit: millisecond)*/
    bool                     is_playing;        /**< The streaming is playing or not. */
    uint32_t                 sample_rate;       /**< The streaming data sample rate. (16000,44100,48000). */
} bt_ull_streaming_info_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_TX_USER_DATA,
 *          which is used for transmission user raw data between client with server
 */
typedef struct {
    bt_bd_addr_t             remote_address;    /**< The remote device address.*/
    uint16_t                 user_data_length;  /**< The length of user_data. */
    uint8_t                  *user_data;        /**< User data pointer. */
} bt_ull_user_data_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_INIT_CRITICAL_USER_DATA,
 *          which is used for user initiate critical data context
 */
typedef struct {
    uint8_t                  max_user_data_length;     /**< The maximum length of critical data (range: 1~100). */
} bt_ull_init_critical_user_data_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_TX_CRITICAL_USER_DATA,
 *          which is used for user to send flushable critical data between client and server
 */
typedef struct {
    uint16_t                 flush_timeout;     /**< Flush timeout of send critical data.
                                                  *      Unit: milliseconds
                                                  *      Range: 0 ~ 0xFFFF; 0: means there is no timeout
                                                  *      It's recommended that the value is set larger than 20 milliseconds
                                                */
    uint8_t                  user_data_length;  /**< The length of critical data. */
    uint8_t                  *user_data;        /**< Critical data pointer. */
} bt_ull_tx_critical_user_data_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_EVENT_RX_CRITICAL_USER_DATA_IND,
 *          which is used for user to receive the critical data
 */
typedef struct {
    uint8_t                  user_data_length;  /**< The length of critical data. */
    uint8_t                  *user_data;        /**< Critical data pointer. */
} bt_ull_rx_critical_user_data_t;



/**
* @brief This structure defined the parameters of #BT_ULL_ACTION_SET_CLIENT_SIDETONE_SWITCH
*                which is used for user set client uplink sidetone on/off
*/
typedef struct {
    bool           sidetone_enable;           /**< The sidetone is switched on or off. */
} bt_ull_client_sidetone_switch_t;
/**
 * @}
 */


/**
 * @brief   Defines a function pointer to function which is used to listen and handle the events reported from Ultra Low Latency service.
 * @param[in] event       is the callback event type.
 * @param[in] param       is the payload of the callback event.
 * @param[in] param_len   is the payload length.
 * @return  Void.
 */
typedef void (* bt_ull_callback)(bt_ull_event_t event, void *param, uint32_t param_len);

/**
 * @brief   This function is used for application to initialize Ultra Low Latency service.
 * @param[in] role        is Ultra Low Latency server or client.
 * @param[in] callback    is callback function.
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_init(bt_ull_role_t role, bt_ull_callback callback);

/**
 * @brief   This function is used to control ULL device.
 * @param[in] action      is the user request action type.
 * @param[in] param       is the payload of the request action.
 * @param[in] param_len   is the payload length.
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_action(bt_ull_action_t action, const void *param, uint32_t param_len);

/**
 * @brief   This function is used to get specified streaming info.
 * @param[in] streaming   streaming type.
 * @param[out] info       The detail streaming info.
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info);



/**
 * @brief             This function is used to stop or start the streaming.
 * @param[in] lock    bool type, true: stop the dongle streaming, false: start the dongle streaming.
 * @return            void
 */
void bt_ull_lock_streaming(bool lock);

BT_EXTERN_C_END

/**
 * @}
 * @}
 */

#endif /* __BT_ULL_SERVICE_H__ */

