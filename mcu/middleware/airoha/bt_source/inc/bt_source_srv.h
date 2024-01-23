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

#ifndef __BT_SOURCE_SRV_H__
#define __BT_SOURCE_SRV_H__

/**
  * @addtogroup Bluetooth_Services_Group Bluetooth Services
  * @{
  * @addtogroup BluetoothServices_Source BT Source
  * @{
  * The Source is a Bluetooth service which integrates HFP, HSP.
  * It implements most functions of these Bluetooth profiles and provides the interface which is easier to use.
  * The source service works as a Bluetooth source device and contains many usual functions such as inform new call or call change.
  * This section defines the Bluetooth source service API to use all Bluetooth source functions.
  * @{
 *
 * Terms and Acronyms
 * ======
 * |Terms                         | Details                                                                |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b SRV                        | Service, a common service that is based on Bluetooth profile. |
 * |\b TWC                        | Three-way call, a state of sink status. |
 * |\b AG                         | Audio Gateway. |
 * |\b MPTY                       | MultiParty. |
 * |\b IAC                        | International access code. |
 * |\b DTMF                       | Dual Tone Multi-Frequency,i.e. an in-band telecommunication signaling system using the voice-frequency band over telephone lines. For more information, please refer to <a href="https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling">Dual tone multi-frequency signaling</a> in Wikipedia. |
 *
 * @section bt_source_srv_api_usage How to use this module
 *  - Step1: Mandatory, initialize Bluetooth source service during system initialization.
 *   - Sample code:
 *    @code
 *       bt_source_srv_init_parameter_t init_parameter = {0};
 *       init_parameter.hfp_init_parameter.battery_level = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
 *       bt_source_srv_init(&init_parameter);
 *    @endcode
 *  - Step2: Mandatory, implement #bt_source_srv_event_callback() to handle the source events, such as accept call, voice recognition activation, etc.
 *   - Sample code:
 *    @code
 *       void bt_source_srv_event_callback(bt_source_srv_event_t event_id, void *parameter, uint32_t length);
 *       {
 *            switch (event_id) {
 *                case BT_SOURCE_SRV_EVENT_CALL_INDEX_IND: {
 *                    bt_source_srv_call_index_ind_t *call_index = (bt_source_srv_call_index_ind_t *)parameter;
 *                    bt_source_app_report("call index indication audio source type = %02x, index = %02x", call_index->type, call_index->index)
 *                }
 *                break;
 *                case BT_SOURCE_SRV_EVENT_ACCEPT_CALL: {
 *                    bt_source_srv_accept_call_t *accept_call = (bt_source_srv_accept_call_t *)parameter;
 *                    bt_source_app_report("accept call audio source type = %02x, index = %02x", accept_call->type, accept_call->index)
 *                }
 *                break;
 *                default:
 *                    break;
 *            }
 *       }
 *    @endcode
 *  - Step3: Mandatory, implement #bt_source_srv_get_feature_config() to get source feature config.
 *   - Sample code:
 *    @code
 *       bt_status_t bt_source_srv_get_feature_config(bt_source_srv_t type, void *feature_config);
 *       {
 *            bt_status_t status = BT_STATUS_FAIL;
 *            if (feature_config == NULL) {
 *                LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get feature config is NULL", 0);
 *                return status;
 *            }
 *            if (type == BT_SOURCE_SRV_TYPE_HFP) {
 *                bt_source_srv_hfp_feature_config_t *hfp_feature_config = (bt_source_srv_hfp_feature_config_t *)feature_config;
 *                hfp_feature_config->feature = BT_SOURCE_SRV_HFP_FEATURE_CALL_REJECT | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS | BT_HFP_AG_FEATURE_CODEC_NEGOTIATION |
 *                                              BT_HFP_AG_FEATURE_ESCO_S4_SETTINGS;
 *                hfp_feature_config->hold_feature = 0;
 *                hfp_feature_config->codec_type = BT_SOURCE_SRV_HFP_CODEC_TYPE_CVSD | BT_SOURCE_SRV_HFP_CODEC_TYPE_MSBC;
 *                status = BT_STATUS_SUCCESS;'
 *            }
 *            return status;
 *       }
 *    @endcode
 *  - Step4: Mandatory, implement #bt_source_srv_get_phone_card_information() to get phone card information.
 *   - Sample code:
 *    @code
 *       #define BT_CUSTOMER_CONFIG_PHONE_CARD_MAX      0x01
 *       static uint8_t g_phone_card_number[] = "02811111111";
 *       static uint8_t g_operator_information[] = "airoha";
 *       uint32_t bt_source_srv_get_phone_card_information(bt_source_srv_t type, bt_source_srv_phone_card_info_t *phone_card, uint32_t phone_card_num)
 *       {
 *           LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get phone card information type = %02x", 1, type);
 *           uint32_t card_number = 0;
 *           if (phone_card == NULL) {
 *               LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get phone card information is NULL", 0);
 *               return 0;
 *           }
 *           phone_card->own_number = g_phone_card_number;
 *           phone_card->own_number_length = stelrn(g_phone_card_number);
 *           phone_card->operator_information = g_operator_information;
 *           phone_card->operator_information_length = stelrn(g_operator_information);
 *           phone_card->operator_mode = BT_SOURCE_SRV_OPERATOR_MODE_AUTOMATIC;
 *           phone_card->signal_strength = 5;
 *           phone_card->roaming_state = BT_SOURCE_SRV_HFP_ROAMING_STATE_INACTIVE;
 *           phone_card->own_number_service = BT_SOURCE_SRV_PHONE_NUMBER_SERVICE_VOICE;
 *           phone_card->own_number_type = 129;
 *           return BT_CUSTOMER_CONFIG_PHONE_CARD_MAX;
 *       }
 *    @endcode
 *  - Step5: Optional, Call the function #bt_source_srv_send_action() to execute a source operation, such as inform an new call.
 *   - Sample code:
 *    @code
 *       const uint8_t *phone_number[] = "123456789";
 *       bt_source_srv_hfp_new_call_t new_call = {0};
 *       new_call.state = BT_SOURCE_SRV_CALL_STATE_INCOMING;
 *       new_call.mode = BT_SOURCE_SRV_HFP_CALL_MODE_DATA;
 *       new_call.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
 *       new_call.number_length = strlen(phone_number);
 *       new_call.number = phone_number;
 *       new_call.iac = BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT;
 *       bt_status_t status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_NEW_CALL, &new_call, sizeof(bt_source_srv_hfp_new_call_t));
 *    @endcode
 */
#include "bt_type.h"
#include "bt_hfp.h"
#include "bt_custom_type.h"
#include "bt_avrcp.h"

/**
 * @defgroup BluetoothServices_Source_define Define
 * @{
 * Define Bluetooth Source service data types and values.
 */


/**
 * @brief Define the phone number maximum length.
 */
#define BT_SOURCE_SRV_PHONE_NUMBER_LENGTH             20

/**
 * @brief Define the operator information maximum length.
 */
#define BT_SOURCE_SRV_OPERATOR_INFORMATION_LENGTH     20

/**
 * @brief Define the source type.
 */
#define BT_SOURCE_SRV_TYPE_NONE                  0x00 /**< The invalid source type. */
#define BT_SOURCE_SRV_TYPE_HFP                   0x01 /**< The HFP source type. */
#define BT_SOURCE_SRV_TYPE_A2DP                  0x02 /**< The A2DP source type. */
typedef uint8_t bt_source_srv_t;                      /**< The source type. */

/**
 * @brief Define the source module ID for the source service.
 */
#define BT_SOURCE_MODULE_OFFSET        16                                                                 /**< Module range: 0xF8500000 ~ 0xF85F0000. The maximum number of modules: 16. */
#define BT_SOURCE_MODULE_COMMON        ((BT_MODULE_CUSTOM_SOURCE) | (0x1U << BT_SOURCE_MODULE_OFFSET))    /**< Prefix of the common module. 0xF8510000 */
#define BT_SOURCE_MODULE_CALL          ((BT_MODULE_CUSTOM_SOURCE) | (0x2U << BT_SOURCE_MODULE_OFFSET))    /**< Prefix of the call module. 0xF8520000 */
#define BT_SOURCE_MODULE_MUSIC         ((BT_MODULE_CUSTOM_SOURCE) | (0x3U << BT_SOURCE_MODULE_OFFSET))    /**< Prefix of the call module. 0xF8530000 */
/**
 * @brief Define the sub type action and event for the source module ID.
 * +----------+--------------+--------- +---------- +
 * |Source  ID  |   Module ID  | Sub Type |   Value   |
 * +----------+--------------+--------- +---------- +
 */
#define BT_SOURCE_MODULE_SUB_TYPE_OFFSET               13           /**< Sub type range: 0xF8X00000 ~ 0xF8X8000. The maximum number of type:8. */

/**
 *  @brief  Define the source action sub type.
 */
typedef uint32_t bt_source_srv_action_t;

#define BT_SOURCE_MODULE_COMMON_ACTION                      ((BT_SOURCE_MODULE_COMMON) | (0x0U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET)) /**< Prefix of the common action. 0xF8510000 */
#define BT_SOURCE_MODULE_CALL_ACTION                        ((BT_SOURCE_MODULE_CALL)   | (0x0U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET)) /**< Prefix of the call action.   0xF8520000 */
#define BT_SOURCE_MODULE_MUSIC_ACTION                       ((BT_SOURCE_MODULE_MUSIC)  | (0x0U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET)) /**< Prefix of the music action.  0xF8530000 */

/* common action. */
#define BT_SOURCE_SRV_ACTION_AUDIO_PORT                     (BT_SOURCE_MODULE_COMMON_ACTION | 0x001U)       /**< This action informs local that a audio port is opened, \
                                                                                                                with #bt_source_srv_audio_port_t as the payload 0xF8510001 */
#define BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_RATE              (BT_SOURCE_MODULE_COMMON_ACTION | 0x002U)       /**< This action informs local that audio port sample rate is changed, \
                                                                                                                with #bt_source_srv_audio_sample_rate_t as the payload 0xF8510002 */
#define BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_SIZE              (BT_SOURCE_MODULE_COMMON_ACTION | 0x003U)       /**< This action informs local that audio port sample size is changed, \
                                                                                                                with #bt_source_srv_audio_sample_size_t as the payload 0xF8510003 */
#define BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_CHANNEL           (BT_SOURCE_MODULE_COMMON_ACTION | 0x004U)       /**< This action informs local that audio port sample channel is changed, \
                                                                                                                with #bt_source_srv_audio_sample_channel_t as the payload 0xF8510004 */
#define BT_SOURCE_SRV_ACTION_MUTE                           (BT_SOURCE_MODULE_COMMON_ACTION | 0x005U)       /**< This action informs device that speaker or MIC mute, \
                                                                                                                with #bt_source_srv_audio_mute_t as the payload 0xF8510005 */
#define BT_SOURCE_SRV_ACTION_UNMUTE                         (BT_SOURCE_MODULE_COMMON_ACTION | 0x006U)       /**< This action informs device that speaker or MIC unmute, \
                                                                                                                with #bt_source_srv_audio_unmute_t as the payload 0xF8510006 */
#define BT_SOURCE_SRV_ACTION_VOLUME_UP                      (BT_SOURCE_MODULE_COMMON_ACTION | 0x007U)       /**< This action informs device that volume up, \
                                                                                                                with #bt_source_srv_volume_up_t as the payload 0xF8510007 */
#define BT_SOURCE_SRV_ACTION_VOLUME_DOWN                    (BT_SOURCE_MODULE_COMMON_ACTION | 0x008U)       /**< This action informs device that volume down, \
                                                                                                                with #bt_source_srv_volume_down_t as the payload 0xF8510008 */
#define BT_SOURCE_SRV_ACTION_VOLUME_CHANGE                  (BT_SOURCE_MODULE_COMMON_ACTION | 0x009U)       /**< This action informs device that volume change, \
                                                                                                                with #bt_source_srv_volume_change_t as the payload 0xF8510009 */
#define BT_SOURCE_SRV_ACTION_SWITCH_CODEC                   (BT_SOURCE_MODULE_COMMON_ACTION | 0x00AU)       /**< This action requests to switch codec, \
                                                                                                                with #bt_source_srv_switch_codec_t as the payload 0xF851000A */
/* call action. */
#define BT_SOURCE_SRV_ACTION_NEW_CALL                       (BT_SOURCE_MODULE_CALL_ACTION | 0x001U)         /**< This action informs remote that a new call is created, \
                                                                                                                with #bt_source_srv_new_call_t as the payload 0xF8520001 */
#define BT_SOURCE_SRV_ACTION_CALL_STATE_CHANGE              (BT_SOURCE_MODULE_CALL_ACTION | 0x002U)         /**< This action informs remote that the call status is changed, \
                                                                                                                with #bt_source_srv_call_state_change_t as the payload 0xF8520002 */
#define BT_SOURCE_SRV_ACTION_SERVICE_AVAILABILITY_CHANGE    (BT_SOURCE_MODULE_CALL_ACTION | 0x003U)         /**< This action informs remote that the service availability is changed, \
                                                                                                                with #bt_source_srv_service_availability_change_t as the payload 0xF8520003 */
#define BT_SOURCE_SRV_ACTION_SIGNAL_STRENGTH_CHANGE         (BT_SOURCE_MODULE_CALL_ACTION | 0x004U)         /**< This action informs remote that the signal strength is changed, \
                                                                                                                with #bt_source_srv_signal_strength_change_t as the payload 0xF8520004 */
#define BT_SOURCE_SRV_ACTION_ROAMING_STATUS_CHANGE          (BT_SOURCE_MODULE_CALL_ACTION | 0x005U)         /**< This action informs remote that the roaming status is changed, \
                                                                                                                with #bt_source_srv_roaming_status_change_t as the payload 0xF8520005 */
#define BT_SOURCE_SRV_ACTION_BATTERY_LEVEL_CHANGE           (BT_SOURCE_MODULE_CALL_ACTION | 0x006U)         /**< This action informs remote that the battery level is changed, \
                                                                                                                with #bt_source_srv_battery_level_change_t as the payload 0xF8520006 */
#define BT_SOURCE_SRV_ACTION_VOICE_RECOGNITION_STATE_CHANGE (BT_SOURCE_MODULE_CALL_ACTION | 0x007U)         /**< This action informs remote that the voice recognition state is changed, \
                                                                                                                with #bt_source_srv_voice_recognition_state_change_t as the payload 0xF8520007 */
#define BT_SOURCE_SRV_ACTION_SWITCH_AUDIO_PATH              (BT_SOURCE_MODULE_CALL_ACTION | 0x008U)         /**< This action requests to switch audio path,\
                                                                                                                 with #bt_source_srv_switch_audio_path_t as the the payload 0xF8520008 */
#define BT_SOURCE_SRV_ACTION_SEND_CUSTOM_RESULT_CODE        (BT_SOURCE_MODULE_CALL_ACTION | 0x009U)         /**< This action request is for sending custom result code to HF,\
                                                                                                                 with #bt_source_srv_send_custom_result_code_t as the payload 0xF8520009 */
/**
 *  @brief Define the music operation type.
 */
typedef uint32_t bt_srv_music_operation_t;

/*music*/
#define BT_SOURCE_SRV_ACTION_START_STREAM                   (BT_SOURCE_MODULE_MUSIC_ACTION | 0x001U)         /**< This action sends a request to start stream to Remote device. 0xF8530001*/
#define BT_SOURCE_SRV_ACTION_SUSPEND_STREAM                 (BT_SOURCE_MODULE_MUSIC_ACTION | 0x002U)         /**< This action sends a request to suspend stream to Remote device. 0xF8530002*/
#define BT_SOURCE_SRV_ACTION_PLAY                           (BT_SOURCE_MODULE_MUSIC_ACTION | 0x003U)         /**< This action sends a request to play Music to Remote device. 0xF8530003*/
#define BT_SOURCE_SRV_ACTION_PAUSE                          (BT_SOURCE_MODULE_MUSIC_ACTION | 0x004U)         /**< This action sends a request to pause Music to Remote device. 0xF8530004*/
#define BT_SOURCE_SRV_ACTION_NEXT                           (BT_SOURCE_MODULE_MUSIC_ACTION | 0x005U)         /**< This action sends a request to next Music to Remote device. 0xF8530005*/
#define BT_SOURCE_SRV_ACTION_PREVIOUS                       (BT_SOURCE_MODULE_MUSIC_ACTION | 0x006U)         /**< This action sends a request to previous Music to Remote device. 0xF8530006*/
#define BT_SOURCE_SRV_ACTION_SET_ABSOLUTE_VOLUME            (BT_SOURCE_MODULE_MUSIC_ACTION | 0x009U)         /**< This action sends a request to volume down to Remote device. 0xF8530009*/
#define BT_SOURCE_SRV_ACTION_REGISTER_RESPONSE              (BT_SOURCE_MODULE_MUSIC_ACTION | 0x00aU)         /**< This action sends a notify response to Register notify. 0xF853000a*/
#define BT_SOURCE_SRV_ACTION_GET_ELEMENT                    (BT_SOURCE_MODULE_MUSIC_ACTION | 0x00bU)         /**< This action sends a notify response to Register notify. 0xF853000b*/
#define BT_SOURCE_SRV_ACTION_START_STREAM_RESPONSE          (BT_SOURCE_MODULE_MUSIC_ACTION | 0x00cU)         /**< This action sends a notify response to Register notify. 0xF853000c*/
#define BT_SOURCE_SRV_ACTION_SUSPEND_STREAM_RESPONSE        (BT_SOURCE_MODULE_MUSIC_ACTION | 0x00dU)         /**< This action sends a notify response to Register notify. 0xF853000c*/

/**
 * @deprecated Use #BT_SOURCE_MODULE_MUSIC_ACTION instead.
 */
#define BT_SOURCE_MOULE_MUSIC_ACTION                        (BT_SOURCE_MODULE_MUSIC_ACTION)      /**< This event will be phased out and removed in the next SDK major version. Do not use. */

/**
 *  @brief Define the Source event sub type.
 */

typedef uint32_t bt_source_srv_event_t;
#define BT_SOURCE_MODULE_COMMON_EVENT                   ((BT_SOURCE_MODULE_COMMON) | (0x1U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET))     /**< Prefix of the common event. 0xF8511000 */
#define BT_SOURCE_MODULE_CALL_EVENT                     ((BT_SOURCE_MODULE_CALL)   | (0x1U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET))     /**< Prefix of the call event.   0xF8521000*/
#define BT_SOURCE_MODULE_MUSIC_EVENT                    ((BT_SOURCE_MODULE_MUSIC)  | (0x1U << BT_SOURCE_MODULE_SUB_TYPE_OFFSET))     /**< Prefix of the music event.  0xF8531000*/

/* common event */
#define BT_SOURCE_SRV_EVENT_VOLUME_UP                            (BT_SOURCE_MODULE_COMMON_EVENT | 0x001U)        /**< This event indicates the volume up,\
                                                                                                                    with #bt_source_srv_volume_up_ind_t as the payload. 0xF8511001 */
#define BT_SOURCE_SRV_EVENT_VOLUME_DOWN                          (BT_SOURCE_MODULE_COMMON_EVENT | 0x002U)        /**< This event indicates the volume down,\
                                                                                                                    with #bt_source_srv_volume_down_ind_t as the payload. 0xF8511002 */
#define BT_SOURCE_SRV_EVENT_VOLUME_CHANGE                        (BT_SOURCE_MODULE_COMMON_EVENT | 0x003U)        /**< This event indicates the volume change,\
                                                                                                                    with #bt_source_srv_volume_change_ind_t as the payload. 0xF8511003 */
#define BT_SOURCE_SRV_EVENT_PROFILE_CONNECTED                    (BT_SOURCE_MODULE_COMMON_EVENT | 0x004U)        /**< This event indicates the profile connected,\
                                                                                                                    with #bt_source_srv_profile_connected_t as the payload. 0xF8511004 */
#define BT_SOURCE_SRV_EVENT_PROFILE_DISCONNECTED                 (BT_SOURCE_MODULE_COMMON_EVENT | 0x005U)        /**< This event indicates the profile disconnected,\
                                                                                                                    with #bt_source_srv_profile_disconnected_t as the payload. 0xF8511005 */

/* call event */
#define BT_SOURCE_SRV_EVENT_CALL_INDEX_IND                       (BT_SOURCE_MODULE_CALL_EVENT | 0x001U)        /**< This event indicates the call index for new call,\
                                                                                                                    with #bt_source_srv_call_index_ind_t as the payload. 0xF8521001 */
#define BT_SOURCE_SRV_EVENT_ACCEPT_CALL                          (BT_SOURCE_MODULE_CALL_EVENT | 0x002U)        /**< This event indicates the call is accepted,\
                                                                                                                    with #bt_source_srv_accept_call_t as the payload. 0xF8521002 */
#define BT_SOURCE_SRV_EVENT_REJECT_CALL                          (BT_SOURCE_MODULE_CALL_EVENT | 0x003U)        /**< This event indicates the call is rejected,\
                                                                                                                    with #bt_source_srv_reject_call_t as the payload. 0xF8521003 */
#define BT_SOURCE_SRV_EVENT_TERMINATE_CALL                       (BT_SOURCE_MODULE_CALL_EVENT | 0x004U)        /**< This event indicates the call is terminated,\
                                                                                                                    with #bt_source_srv_terminate_call_t as the payload. 0xF8521004 */
#define BT_SOURCE_SRV_EVENT_DIAL_NUMBER                          (BT_SOURCE_MODULE_CALL_EVENT | 0x005U)        /**< This event indicates dial a number,\
                                                                                                                    with #bt_source_srv_dial_number_t as the payload. 0xF8521005 */
#define BT_SOURCE_SRV_EVENT_DIAL_MEMORY                          (BT_SOURCE_MODULE_CALL_EVENT | 0x006U)        /**< This event indicates dial memory,\
                                                                                                                    with payload is NULL. 0xF8521006 */
#define BT_SOURCE_SRV_EVENT_DIAL_LAST_NUMBER                     (BT_SOURCE_MODULE_CALL_EVENT | 0x007U)        /**< This event indicates dial last number,\
                                                                                                                    with payload is NULL. 0xF8521007 */
#define BT_SOURCE_SRV_EVENT_VOICE_RECOGNITION_ACTIVATION         (BT_SOURCE_MODULE_CALL_EVENT | 0x008U)        /**< This event indicates the voice recognition activation,\
                                                                                                                    with #bt_source_srv_voice_recognition_activation_t as the payload. 0xF8521008 */
#define BT_SOURCE_SRV_EVENT_DTMF                                 (BT_SOURCE_MODULE_CALL_EVENT | 0x009U)        /**< This event indicates the dual tone multi-frequency,\
                                                                                                                    with #bt_source_srv_dtmf_t as the payload. 0xF8521009 */
#define BT_SOURCE_SRV_EVENT_UNHOLD                               (BT_SOURCE_MODULE_CALL_EVENT | 0x00AU)        /**< This event indicates the call is unhold,\
                                                                                                                    with #bt_source_srv_unhold_t as the payload. 0xF852100A */
#define BT_SOURCE_SRV_EVENT_HOLD                                 (BT_SOURCE_MODULE_CALL_EVENT | 0x00BU)        /**< This event indicates the call is hold,\
                                                                                                                    with #bt_source_srv_hold_t as the payload. 0xF852100B */
#define BT_SOURCE_SRV_EVENT_ESCO_STATE_UPDATE                    (BT_SOURCE_MODULE_CALL_EVENT | 0x00CU)        /**< This event indicates the eSCO link state,\
                                                                                                                    with #bt_source_srv_esco_state_update_t as the payload. 0xF852100C */

/*Music event*/

#define BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_CNF           (BT_SOURCE_MODULE_MUSIC_EVENT | 0x001U)        /**< The event is triggered after receiving start play/suspend response from remote devices*/
#define BT_SOURCE_SRV_EVENT_MUSIC_CONTROL_OPERATION_IND          (BT_SOURCE_MODULE_MUSIC_EVENT | 0x002U)        /**< The event is triggered after receiving control cmd, can refer to  bt_srv_avrcp_operation_id_t from remote device*/
#define BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_IND           (BT_SOURCE_MODULE_MUSIC_EVENT | 0x003U)        /**< The event is triggered after receiving start/suspend streaming req from remote device*/
#define BT_SOURCE_SRV_EVENT_MUSIC_CHANGE_VOLUME_IND              (BT_SOURCE_MODULE_MUSIC_EVENT | 0x004U)        /**< The event is triggered after receiving set absolute volume response from remote device*/
#define BT_SOURCE_SRV_EVENT_MUSIC_DETECT_MEDIA_DATA_IND          (BT_SOURCE_MODULE_MUSIC_EVENT | 0x005U)        /**< The event is triggered after receiving detection media data event from audio manager*/


/**
 * @deprecated Use #BT_SOURCE_SRV_EVENT_MUSIC_CHANGE_VOLUME_IND instead.
 */
#define BT_SOURCE_SRV_EVENT_MUSIC_CHNAGE_VOLUME_IND              (BT_SOURCE_SRV_EVENT_MUSIC_CHANGE_VOLUME_IND)      /**< This event will be phased out and removed in the next SDK major version. Do not use. */

/**
 *  @brief Define the audio port.
 */
#define BT_SOURCE_SRV_PORT_NONE                         0x00      /**< The audio port is none. */
#define BT_SOURCE_SRV_PORT_GAMING_SPEAKER               0x01      /**< The audio port is gaming speaker. */
#define BT_SOURCE_SRV_PORT_CHAT_SPEAKER                 0x02      /**< The audio port is chat speaker. */
#define BT_SOURCE_SRV_PORT_MIC                          0x03      /**< The audio port is mic */
#define BT_SOURCE_SRV_PORT_LINE_IN                      0x04      /**< The audio port is line in */
#define BT_SOURCE_SRV_PORT_I2S_IN                       0x05      /**< The audio port is i2s in */
#define BT_SOURCE_SRV_PORT_I2S_IN_1                     0x06      /**< The audio port is i2s in 1 */
typedef uint8_t bt_source_srv_port_t;                             /**< The audio port type. */

/**
 *  @brief Define the HFP audio gateway features.
 */
#define BT_SOURCE_SRV_HFP_FEATURE_3_WAY                  BT_HFP_AG_FEATURE_3_WAY                          /**< 3-way calling. */
#define BT_SOURCE_SRV_HFP_FEATURE_ECHO_NOISE             BT_HFP_AG_FEATURE_ECHO_NOISE                     /**< Echo canceling and noise reduction function. */
#define BT_SOURCE_SRV_HFP_FEATURE_VOICE_RECOGNITION      BT_HFP_AG_FEATURE_VOICE_RECOGNITION              /**< Voice recognition function. */
#define BT_SOURCE_SRV_HFP_FEATURE_IN_BAND_RING           BT_HFP_AG_FEATURE_IN_BAND_RING                   /**< In-band ring tone. */
#define BT_SOURCE_SRV_HFP_FEATURE_VOICE_TAG              BT_HFP_AG_FEATURE_VOICE_TAG                      /**< Voice tag. */
#define BT_SOURCE_SRV_HFP_FEATURE_CALL_REJECT            BT_HFP_AG_FEATURE_CALL_REJECT                    /**< Reject a call. */
#define BT_SOURCE_SRV_HFP_FEATURE_ENHANCED_CALL_STATUS   BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS           /**< Enhanced call status. */
#define BT_SOURCE_SRV_HFP_FEATURE_ENHANCED_CALL_CTRL     BT_HFP_AG_FEATURE_ENHANCED_CALL_CTRL             /**< Enhanced call control. */
#define BT_SOURCE_SRV_HFP_FEATURE_EXTENDED_ERROR         BT_HFP_AG_FEATURE_EXTENDED_ERROR                 /**< Extended error. */
#define BT_SOURCE_SRV_HFP_FEATURE_CODEC_NEGOTIATION      BT_HFP_AG_FEATURE_CODEC_NEGOTIATION              /**< Codec negotiation. */
#define BT_SOURCE_SRV_HFP_FEATURE_HF_INDICATORS          BT_HFP_AG_FEATURE_HF_INDICATORS                  /**< HF indicators to notify AG. */
#define BT_SOURCE_SRV_HFP_FEATURE_ESCO_S4_SETTINGS       BT_HFP_AG_FEATURE_ESCO_S4_SETTINGS               /**< eSCO S4 settings supported. */
typedef bt_hfp_ag_feature_t bt_source_srv_hfp_feature_t;                                                  /**< The HFP audio gateway features. */

/**
 *  @brief Define the audio gateway's 3-way calling (hold) feature set.
 */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_RELEASE_HELD_CALL         BT_HFP_AG_HOLD_FEATURE_RELEASE_HELD_CALL         /**< Releases all held calls or sets User Determined User Busy for a waiting call. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_RELEASE_ACTIVE_CALL       BT_HFP_AG_HOLD_FEATURE_RELEASE_ACTIVE_CALL       /**< Releases all active calls and accepts the other held or waiting call. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_RELEASE_SPECIFIC_CALL     BT_HFP_AG_HOLD_FEATURE_RELEASE_SPECIFIC_CALL     /**< Releases a specific call. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_HOLD_ACTIVE_CALL          BT_HFP_AG_HOLD_FEATURE_HOLD_ACTIVE_CALL          /**< Places all active calls on hold and accepts the other held or waiting call. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_HOLD_SPECIFIC_CALL        BT_HFP_AG_HOLD_FEATURE_HOLD_SPECIFIC_CALL        /**< Places a specific call on hold. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_ADD_HELD_CALL             BT_HFP_AG_HOLD_FEATURE_ADD_HELD_CALL             /**< Adds a held call to the conversation. */
#define BT_SOURCE_SRV_HFP_HOLD_FEATURE_CALL_TRANSFER             BT_HFP_AG_HOLD_FEATURE_CALL_TRANSFER             /**< Connects the two calls and disconnects the AG from both calls. */
typedef bt_hfp_ag_hold_feature_t bt_source_srv_hfp_hold_feature_t;                                                /**< The audio gateway's 3-way calling (hold) feature set. */

/**
 *  @brief Define the HFP audio gateway custom feature.
*/
#define BT_SOURCE_SRV_HFP_CUSTOM_FEATURE_LOCAL_MIC               0x0001                                           /**< Enabling the local MIC. */
typedef uint16_t bt_source_srv_hfp_custom_feature_t;                                                              /**< The HFP custom features. */
/**
 *  @brief Define HFP codec type.
 */
#define BT_SOURCE_SRV_HFP_CODEC_TYPE_NONE    BT_HFP_CODEC_TYPE_NONE       /**< The codec is not configured. */
#define BT_SOURCE_SRV_HFP_CODEC_TYPE_CVSD    BT_HFP_CODEC_TYPE_CVSD       /**< The codec is CVSD. */
#define BT_SOURCE_SRV_HFP_CODEC_TYPE_MSBC    BT_HFP_CODEC_TYPE_MSBC       /**< The codec is MSBC. */
typedef bt_hfp_audio_codec_type_t bt_source_srv_hfp_codec_t;              /**< The HFP codec type. */

/**
 *  @brief Define the HFP operator mode.
 */
#define BT_SOURCE_SRV_OPERATOR_MODE_AUTOMATIC                  0x00            /**< Automatic mode. */
#define BT_SOURCE_SRV_OPERATOR_MODE_MANUAL                     0x01            /**< Manual mode. */
#define BT_SOURCE_SRV_OPERATOR_MODE_DEREGISTER_FROM_NETWORK    0x02            /**< Deregister from network. */
#define BT_SOURCE_SRV_OPERATOR_MODE_SET_ONLY_FORMAT            0x03            /**< Set only format. */
#define BT_SOURCE_SRV_OPERATOR_MODE_AUTOMATIC_AND_MANUAL       0x04            /**< Automatic and Manual mode. */
typedef uint8_t bt_source_srv_operator_mode_t;                                 /**< The HFP operator mode. */

/**
 *  @brief Define the service availability type.
 */
#define BT_SOURCE_SRV_SERVICE_AVAILABILITY_INACTIVE     0x00     /**< The service availability is inactive. */
#define BT_SOURCE_SRV_SERVICE_AVAILABILITY_ACTIVE       0x01     /**< The service availability is active. */
typedef uint8_t bt_source_srv_service_availability_t;            /**< The service availability type. */

/**
 *  @brief Define the roaming state type.
 */
#define BT_SOURCE_SRV_HFP_ROAMING_STATE_INACTIVE    0x00     /**< The roaming state is inactive. */
#define BT_SOURCE_SRV_HFP_ROAMING_STATE_ACTIVE      0x01     /**< The roaming state is active. */
typedef uint8_t bt_source_srv_roaming_state_t;               /**< The roaming state type. */

/**
 *  @brief Define the signal strength type.
 */
typedef uint16_t bt_source_srv_signal_strength_t;            /**< The signal strength type. */

/**
 *  @brief Define the battery level type, the value is 0~100.
 */
typedef uint8_t bt_source_srv_battery_level_t;              /**< The battery level type. */

/**
 *  @brief Define the call state.
 */
#define BT_SOURCE_SRV_CALL_STATE_NONE              0x00     /**< There is no call, the state is none. */
#define BT_SOURCE_SRV_CALL_STATE_INCOMING          0x01     /**< The incoming call. */
#define BT_SOURCE_SRV_CALL_STATE_DIALING           0x02     /**< The dialing outgoing call. */
#define BT_SOURCE_SRV_CALL_STATE_ALERTING          0x03     /**< The alerting outgoing call. */
#define BT_SOURCE_SRV_CALL_STATE_ACTIVE            0x04     /**< The active call. */
#define BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD        0x05     /**< The local held call. */
#define BT_SOURCE_SRV_CALL_STATE_WAITING           0x06     /**< The waiting call. */
typedef uint8_t bt_source_srv_call_state_t;                 /**< The call state. */

/**
 *  @brief Define the HFP call mode.
 */
#define BT_SOURCE_SRV_HFP_CALL_MODE_VOICE          0x00    /**< The call mode is voice. */
#define BT_SOURCE_SRV_HFP_CALL_MODE_DATA           0x01    /**< The call mode is data. */
#define BT_SOURCE_SRV_HFP_CALL_MODE_FAX            0x02    /**< The call mode is fax. */
typedef uint8_t bt_source_srv_hfp_call_mode_t;             /**< The HFP call mode. */

/**
 *  @brief Define the HFP call multiparty.
 */
#define BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE       0x00    /**< The call is not one of multiparty (conference) call parties. */
#define BT_SOURCE_SRV_HFP_CALL_MPTY_ACTIVE         0x01    /**< The call is one of multiparty (conference) call parties. */
typedef uint8_t bt_source_srv_hfp_call_mpty_t;             /**< The HFP call multiparty. */

/**
 *  @brief Define the HFP call address type.
 */
#define BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT         0x00    /**< Dialing string without international access code "+". */
#define BT_SOURCE_SRV_HFP_CALL_IAC_INCLUDE         0x01    /**< Dialing string includes international access code character "+". */
typedef uint8_t bt_source_srv_hfp_call_iac_t;              /**< The HFP call address type. */

/**
 *  @brief Define the call idnex type.
 */
#define BT_SOURCE_SRV_CALL_INVALID_INDEX           0x00    /**< The call invalid index. */
typedef uint8_t bt_source_srv_call_index_t;                /**< The call index type, the index ranges from 0x01 to 0xFF. */

/**
 *  @brief Define the voice recognition status.
 */
#define BT_SOURCE_SRV_VOICE_RECOGNITION_STATUS_DISABLE      0x00      /**< The voice recognition is disable. */
#define BT_SOURCE_SRV_VOICE_RECOGNITION_STATUS_ENABLE       0x01      /**< The voice recognition is enable. */
typedef uint8_t bt_source_srv_voice_recognition_status_t;             /**< The voice recognition status. */

/**
 *  @brief Define the phone number service.
 */
#define BT_SOURCE_SRV_PHONE_NUMBER_SERVICE_VOICE            0x04      /**< The phone number service is voice. */
#define BT_SOURCE_SRV_PHONE_NUMBER_SERVICE_FAX              0x05      /**< The phone number service is fax. */
typedef uint8_t bt_source_srv_phone_number_service_t;                 /**< The phone number service type. */

/**
 *  @brief Define the auido port state.
 */
#define BT_SOURCE_SRV_AUDIO_PORT_STATE_DISABLE      0x00              /**< The audio port is diable. */
#define BT_SOURCE_SRV_AUDIO_PORT_STATE_ENABLE       0x01              /**< The audio port is enable. */
typedef uint8_t bt_source_srv_audio_port_state_t;                     /**< The auido port state. */

/**
 *  @brief Define the volume ajustment type.
 */
#define BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP              0x00          /**< The volume is up step. */
#define BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN            0x01          /**< The volume is down step. */
typedef uint8_t bt_source_srv_volume_step_t;                         /**< The volume step type. */

/**
 *  @brief Define the mute state type.
 */
#define BT_SOURCE_SRV_MUTE_STATE_NONE                  0x00          /**< The mute state is none. */
#define BT_SOURCE_SRV_MUTE_STATE_ENABLE                0x01          /**< The mute state is enable. */
#define BT_SOURCE_SRV_MUTE_STATE_DISABLE               0x02          /**< The mute state is disable. */
typedef uint8_t bt_source_srv_mute_state_t;                          /**< The mute state type. */

/**
 *  @brief Define the aduio direction type.
 */
#define BT_SOURCE_SRV_AUDIO_TRANSFER_TO_HF             0x00           /**< The audio connection is transferred to the HF side and AG establishes eSCO. */
#define BT_SOURCE_SRV_AUDIO_TRANSFER_TO_AG             0x01           /**< The audio connection is transferred to the AG side and AG releases eSCO. */
typedef uint8_t bt_source_srv_audio_transfer_t;                       /**< The audio transfer type. */

/**
 *  @brief Define the media detect type.
 */
#define BT_SOURCE_SRV_MUSIC_DATA_SUSPEND    0x00                    /**< No streaming or silence data is detected for Music. */
#define BT_SOURCE_SRV_MUSIC_DATA_RESUME     0x01                    /**< Audio streaming is detected for Music. */
typedef uint8_t bt_source_srv_music_data_detect_type_t;             /**< The music data detection action. */

/**
 *  @brief Define the device set volume type.
 */
#define BT_SOURCE_SRV_SET_VOLUME_TO_LOCAL  0x00                 /**< Set volume to audio on local. */
#define BT_SOURCE_SRV_SET_VOLUME_TO_REMOTE 0x01                 /**< Set volume to audio on remote side. */
typedef uint32_t bt_source_srv_set_volume_t;                    /**< The set volume type. */


/**
 *  @brief Define media data detect action type.
 */
 #define BT_SOURCE_SRV_MUSIC_DATA_DETECT_ENABLE 0x01    /**< The music data is detected by source is enabled. */
 typedef uint8_t bt_source_srv_music_data_detect_t;     /**<  The music data detection state. */

 /**
 *  @brief Define for the sco connection state.
 */
#define BT_SOURCE_SRV_ESCO_CONNECTION_STATE_DISCONNECTED  0x00     /**<  The disconnected state. */
#define BT_SOURCE_SRV_ESCO_CONNECTION_STATE_CONNECTED     0x01     /**<  The connected state. */
typedef uint8_t bt_source_srv_esco_connection_state_t;             /**<  The eSCO connection state. */

/**
 *  @brief Define the codec type.
 */
#define BT_SOURCE_SRV_CODEC_TYPE_NONE    0x00000000                   /**< There is no codec. */
#define BT_SOURCE_SRV_CODEC_TYPE_CVSD    0x00000001                   /**< The codec is CVSD. */
#define BT_SOURCE_SRV_CODEC_TYPE_MSBC    0x00000002                   /**< The codec is MSBC. */
#define BT_SOURCE_SRV_CODEC_TYPE_SBC     0x00000100                   /**< The codec is SBC. */
typedef uint32_t bt_source_srv_codec_t;                               /**< The source codec type, bit0-bit7 for HFP codec type, bit8-bit15 for A2DP codec type, bit16-bit31 for reserved. */

/**
 * @}
 */

/**
 * @defgroup BluetoothServices_Source_struct Struct
 * @{
 * Define structures for the source service.
 */

/**
 *  @brief This structure defines the parameter for audio port.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
    bt_source_srv_audio_port_state_t state;            /**< The audio port state. */
} bt_source_srv_audio_port_t;

/**
 *  @brief This structure defines the parameter for audio port sample rate.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
    uint32_t                         sample_rate;      /**< The sample rate. */
} bt_source_srv_audio_sample_rate_t;

/**
 *  @brief This structure defines the parameter for audio port sample size.
 */
typedef struct {
    bt_source_srv_port_t             port;            /**< The audio port. */
    uint32_t                         sample_size;     /**< The sample size. */
} bt_source_srv_audio_sample_size_t;

/**
 *  @brief This structure defines the parameter for audio port sample channel.
 */
typedef struct {
    bt_source_srv_port_t             port;           /**< The audio port. */
    uint32_t                         channel;        /**< The sample channel. */
} bt_source_srv_audio_sample_channel_t;

/**
 *  @brief This structure defines the parameter for device mute.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
} bt_source_srv_audio_mute_t;

/**
 *  @brief This structure defines the parameter for device unmute.
 */
typedef bt_source_srv_audio_mute_t bt_source_srv_audio_unmute_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_VOLUME_UP, which informs device to increase volume.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
} bt_source_srv_volume_up_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_VOLUME_DOWN, which informs device to reduce volume.
 */
typedef bt_source_srv_volume_up_t bt_source_srv_volume_down_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_VOLUME_CHANGE, which informs device to change volume.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
    uint32_t                         volume_value;     /**< The volume value, range is 0~100. */
    bt_source_srv_set_volume_t       is_local;         /**< The notification for volume change is sent to either a local or remote device. */
} bt_source_srv_volume_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_SWITCH_CODEC, which requests to switch codec.
 */
typedef struct {
    bt_source_srv_t                  type;           /**< The source type. */
    bt_addr_t                        peer_address;   /**< The address of a remote device. */
    bt_source_srv_codec_t            codec;          /**< The source codec type. */
} bt_source_srv_switch_codec_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_PROFILE_CONNECTED, which indicates the profile connected.
 */
typedef struct {
    bt_source_srv_t                  type;             /**< The source type. */
    bt_addr_t                        peer_address;     /**< The address of a remote device. */
    bt_source_srv_codec_t            support_codec;    /**< The source codec type. */
} bt_source_srv_profile_connected_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_PROFILE_DISCONNECTED, which indicates the profile disconnected.
 */
typedef struct {
    bt_source_srv_t                  type;             /**< The source type. */
    bt_addr_t                        peer_address;     /**< The address of a remote device. */
} bt_source_srv_profile_disconnected_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_VOLUME_UP, which indicates volume is increased.
 */
typedef struct {
    bt_source_srv_port_t             port;             /**< The audio port. */
    bt_addr_t                        peer_address;     /**< The address of a remote device. */
} bt_source_srv_volume_up_ind_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_VOLUME_DOWN, which indicates volume is reduced.
 */
typedef bt_source_srv_volume_up_ind_t bt_source_srv_volume_down_ind_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_VOLUME_CHANGE, which indicates volume is reduced.
 */
typedef struct {
    bt_source_srv_port_t               port;             /**< The audio port. */
    bt_source_srv_volume_step_t        step_type;        /**< The volume step type. */
    bt_source_srv_mute_state_t         mute_state;       /**< The mute state type. */
    bt_addr_t                          peer_address;     /**< The address of a remote device. */
    uint32_t                           volume_step;      /**< The volume step, range is 0~100. */
} bt_source_srv_volume_change_ind_t;


/**
 *  @brief This structure defines the parameter for HFP new call.
 */
typedef struct {
    bt_source_srv_call_state_t       state;            /**< The call state. */
    bt_source_srv_hfp_call_mode_t    mode;             /**< The call mode. */
    bt_source_srv_hfp_call_mpty_t    mpty;             /**< The call multiparty. */
    uint8_t                          number_length;    /**< The call remote phone number length. */
    uint8_t                          *number;          /**< The call remote phone number. */
    bt_source_srv_hfp_call_iac_t     iac;              /**< The call international access code type. */
} bt_source_srv_hfp_new_call_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_NEW_CALL, inform new call is created.
 */
typedef struct {
    bt_source_srv_t                  type;           /**< The source type. */
    union {
        bt_source_srv_hfp_new_call_t hfp_new_call;   /**< The hfp call structure. */
    };                                               /**< The new call information. */
} bt_source_srv_new_call_t;

/**
 *  @brief This structure defines the parameter for HFP call state change.
 */
typedef struct {
    bt_source_srv_call_index_t      index;    /**< The call index. */
    bt_source_srv_call_state_t      state;    /**< The call state. */
    bt_source_srv_hfp_call_mpty_t   mpty;     /**< The call multiparty. */
} bt_source_srv_hfp_call_state_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_CALL_STATE_CHANGE, inform new call state change.
 */
typedef struct {
    bt_source_srv_t                           type;              /**< The source type. */
    union {
        bt_source_srv_hfp_call_state_change_t hfp_call_change;   /**< The HFP call change structure. */
    };                                                           /**< The call change information. */
} bt_source_srv_call_state_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_SERVICE_AVAILABILITY_CHANGE, inform service availability change.
 */
typedef struct {
    bt_source_srv_service_availability_t service_ability;        /**< The service availability. */
} bt_source_srv_service_availability_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_SIGNAL_STRENGTH_CHANGE, inform signal strength change.
 */
typedef struct {
    bt_source_srv_signal_strength_t signal_strength;             /**< The signal strength. */
} bt_source_srv_signal_strength_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_ROAMING_STATUS_CHANGE, inform roaming state change.
 */
typedef struct {
    bt_source_srv_roaming_state_t roaming_state;                /**< The roaming state. */
} bt_source_srv_roaming_status_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_BATTERY_LEVEL_CHANGE, inform battery level change.
 */
typedef struct {
    bt_source_srv_battery_level_t battery_level;                /**< The battery level. */
} bt_source_srv_battery_level_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_VOICE_RECOGNITION_STATE_CHANGE, inform voice recognition status change.
 */
typedef struct {
    bt_source_srv_voice_recognition_status_t status;            /**< The voice recognition status. */
} bt_source_srv_voice_recognition_state_change_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_SWITCH_AUDIO_PATH, AG establishes or releases esco according to the parameters.
 */
typedef struct {
    bt_source_srv_audio_transfer_t        audio_transfer;              /**< The audio transfer type. */
} bt_source_srv_switch_audio_path_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_ACTION_SEND_CUSTOM_RESULT_CODE, AG send custom result code to HF.
 */
typedef struct {
    uint8_t    length;                                         /**< The result code length. */
    char       *result_code;                                   /**< The result code and format shall be a string. */
} bt_source_srv_send_custom_result_code_t;

/**
 *  @brief This structure defines the parameter for HFP phone card information.
 */
typedef struct {
    bt_source_srv_signal_strength_t        signal_strength;                                                 /**< The source signal strength. */
    bt_source_srv_operator_mode_t          operator_mode;                                                   /**< The source operator mode. */
    uint8_t                                operator_information_length;                                     /**< The source operator information length. */
    uint8_t                                *operator_information;                                           /**< The source operator information. */
    bt_source_srv_service_availability_t   service_ability;                                                 /**< The service availability. */
    bt_source_srv_phone_number_service_t   own_number_service;                                              /**< The phone_number_service, indicates which service this phone number relates to. */
    uint8_t                                own_number_type;                                                 /**< The source own number type, refer 3GPP TS 24.008[8] subclause 10.5.4.7. */
    uint8_t                                own_number_length;                                               /**< The source own number length. */
    uint8_t                                *own_number;                                                     /**< The source own number. */
    bt_source_srv_roaming_state_t          roaming_state;                                                   /**< The source roaming state. */
} bt_source_srv_phone_card_info_t;

/**
 *  @brief This structure defines the parameter for HFP feature configuration.
 */
typedef struct {
    bt_source_srv_hfp_feature_t            feature;           /**< The HFP audio gateway features. */
    bt_source_srv_hfp_hold_feature_t       hold_feature;      /**< The audio gateway's 3-way calling (hold) feature. */
    bt_source_srv_hfp_codec_t              codec_type;        /**< The codec type. */
    bt_source_srv_hfp_custom_feature_t     custom_feature;    /**< The HFP custom feature. */
} bt_source_srv_hfp_feature_config_t;

/**
 *  @brief This structure defines the parameter for common feature configuration.
 */
typedef struct {
    uint16_t                               host_volume_scale;  /**< The host volume scale, host is PC or SP etc. */
} bt_source_srv_common_feature_config_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_CALL_INDEX_IND, which indicates new call index.
 */
typedef struct {
    bt_source_srv_t            type;             /**< The source type. */
    bt_source_srv_call_index_t index;            /**< The call index value. */
} bt_source_srv_call_index_ind_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_ACCEPT_CALL, which indicates call is accepted.
 */
typedef struct {
    bt_source_srv_t            type;                /**< The source type. */
    bt_addr_t                  peer_address;        /**< The address of a remote device. */
    bt_source_srv_call_index_t index;               /**< The call index value. */
} bt_source_srv_accept_call_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_REJECT_CALL, which indicates call is rejected.
 */
typedef bt_source_srv_accept_call_t bt_source_srv_reject_call_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_TERMINATE_CALL, which indicates call is terminated.
 */
typedef bt_source_srv_accept_call_t bt_source_srv_terminate_call_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_DIAL_NUMBER, which indicates dial a number on the specified source side.
 */
typedef struct {
    bt_source_srv_t       type;             /**< The source type. */
    bt_addr_t             peer_address;     /**< The address of a remote device, user will not care about the address type when type is HFP. */
    uint8_t               number_length;    /**< The specified number length. */
    uint8_t               *number;          /**< The specified number. */
} bt_source_srv_dial_number_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_VOICE_RECOGNITION_ACTIVATION, which indicates voice recognition is triggered.
 */
typedef struct {
    bt_source_srv_t                          type;             /**< The source type. */
    bt_addr_t                                peer_address;     /**< The address of a remote device. */
    bt_source_srv_voice_recognition_status_t status;           /**< The voice recognition status. */
} bt_source_srv_voice_recognition_activation_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_DTMF, which indicates DTMF is triggered.
 */
typedef struct {
    bt_source_srv_t    type;            /**< The source type. */
    bt_addr_t          peer_address;    /**< The address of a remote device. */
    uint8_t            dtmf_value;      /**< The dual tone multi-frequency value. */
} bt_source_srv_dtmf_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_UNHOLD, which indicates unhold call is triggered.
 */
typedef struct {
    bt_source_srv_t              type;            /**< The source type. */
    bt_addr_t                    peer_address;    /**< The address of a remote device. */
    bt_source_srv_call_index_t   index;           /**< The call index value. */
} bt_source_srv_unhold_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_HOLD, which indicates hold call is triggered.
 */
typedef bt_source_srv_unhold_t bt_source_srv_hold_t;

/**
 *  @brief This structure defines the parameter for #BT_SOURCE_SRV_EVENT_ESCO_STATE_UPDATE, which indicates eSCO link state.
 */
typedef struct {
    bt_addr_t                               peer_address;    /**< The address of a remote device. */
    bt_source_srv_esco_connection_state_t   state;           /**<  The eSCO connection state. */
    bt_source_srv_hfp_codec_t               codec_type;      /**<  The codec type. */
} bt_source_srv_esco_state_update_t;

/**
 *  @brief This structure defines the parameter for HFP init.
 */
typedef struct {
    bt_source_srv_battery_level_t         battery_level;        /**< The battery level type. */
} bt_source_srv_hfp_init_parameter_t;

/**
 *  @brief This structure defines the parameter for source init.
 */
typedef struct {
    bt_source_srv_hfp_init_parameter_t hfp_init_parameter;      /**< The HFP init parameter. */
} bt_source_srv_init_parameter_t;

/**
* @brief The struct for #BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_CNF.
*/
typedef struct {
    bt_addr_t                peer_address;          /**< The address of a remote device. */
    bt_status_t              status;                /**< The status. */
    bt_srv_music_operation_t operation;             /**< The music action. */
}bt_source_srv_music_stream_operation_cnf_t;

/**
* @brief The struct for #BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_IND.
*/

typedef struct {
    bt_addr_t                peer_address;           /**< The address of a remote device. */
    bt_srv_music_operation_t operation;              /**< The music start stream & suspend stream cmd req*/
}  bt_source_srv_music_stream_operation_ind_t;


/**
* @brief The struct for #BT_SOURCE_SRV_EVENT_MUSIC_CONTROL_OPERATION_IND.
*/
typedef struct {
    bt_addr_t                peer_address;          /**< The address of a remote device. */
    bt_srv_music_operation_t operation;             /**< The music action. */
    uint8_t                  length;                /**< The length of parameter of music operation from remote device. */
    uint8_t                  *data;                 /**< The data of music operation from remote device. */
}bt_source_srv_music_control_operation_ind_t;

/**
* @brief The struct defines Music action.
*/
typedef struct {
    bt_addr_t  peer_address;    /**< The address of a remote device. */
    void       *data;           /**< The data of music action from app. */
}bt_source_srv_music_action_t;

/**
* @brief The struct for BT_SOURCE_SRV_EVENT_MUSIC_DETECT_MEDIA_DATA_IND.
*/
typedef struct {
    bt_addr_t  peer_address;                               /**< The address of a remote device. */
    bt_source_srv_music_data_detect_type_t    event;       /**< The type of media data detect. */
}bt_source_srv_music_detect_media_data_ind_t;

/**
*@brief This structure defines the parameter of getting playing device codec.
*/
typedef struct {
    bt_source_srv_t            source_type;               /**< The source type. */
    union {
        bt_source_srv_hfp_codec_t  hfp_codec;             /**< The HFP codec type. */
    };
} bt_source_srv_get_playing_device_codec_t;

/**
 * @}
 */

/**
 * @brief                       This function initializes the source service.
 * @param[in] init_param        is the init parameter of the source service.
 * @return                      #BT_STATUS_SUCCESS, the function initialization is successfully.
 *                              otherwise the function initialization failed.
 */
bt_status_t bt_source_srv_init(const bt_source_srv_init_parameter_t *init_param);

/**
 * @brief                       This function sends an action to the source service,
 *                              The source service executes a source operation to the remote device.
 * @param[in] action            is the source action.
 * @param[in] parameter         is the parameter of the source action.
 * @param[in] length            is the length of the parameter.
 * @return                      #BT_STATUS_SUCCESS, the action is sent successful.
 *                              otherwise the operation failed with the source status.
 */
bt_status_t bt_source_srv_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

/**
 * @brief                       This function is a static callback for the application to listen to the event. It should be implemented by the application.
 * @param[in] event_id          is the callback event ID.
 * @param[in] parameter         is the parameter of the callback message.
 * @param[in] length            is the parameter length of the callback message.
 * @return                      #BT_STATUS_SUCCESS, the action is successfully sent.
 *                              otherwise the operation failed with the source status.
 */
void bt_source_srv_event_callback(bt_source_srv_event_t event_id, void *parameter, uint32_t length);

/**
 * @brief                       This function is used to get the feature configuration. It should be implemented by the application.
 * @param[in] type              is the source type.
 * @param[in] feature           is the parameter of the feature config, obtain different feature config according to different types.
 * @return                      #BT_STATUS_SUCCESS, the action is sent successful.
 *                              otherwise the operation failed with the source status.
 */
bt_status_t bt_source_srv_get_feature_config(bt_source_srv_t type, void *feature);

/**
 * @brief                           This function is used to get phone card information. It should be implemented by the application.
 * @param[in] type                  is the source type.
 * @param[in] phone_card            is the parameter of phone card.
 * @param[in] phone_card_num        is the number of the phone card that the list can hold.
 * @return                          phone card number.
 */
uint32_t bt_source_srv_get_phone_card_information(bt_source_srv_t type, bt_source_srv_phone_card_info_t *phone_card, uint32_t phone_card_num);

/**
 * @brief                 This function is used to get remote device and determine if set absolute volume is supported.
 * @param[in] address     is the BT address of the specified remote device.
 * @return                the support of set absolute volume or not.
 */
bool bt_source_srv_get_remote_absolute_volume_information(const bt_bd_addr_t *address);

/**
 * @brief                         This function is used to get playing device codec.
 * @param[out] playing_codec      is the playing device codec, need to be provided by application.
 * @return                        #BT_STATUS_SUCCESS, the operation is successful and user can check the codec in output parameter.
 *
 */
bt_status_t bt_source_srv_get_playing_device_codec(bt_source_srv_get_playing_device_codec_t *playing_codec);

/**
 * @brief                           This function is used to get the audio type recommended by users. The BT source will select the appropriate audio codec type based on the current situation.
 *                                                                         It should be implemented by the application.
 * @param[in] type                  is the source type.
 * @param[in] peer_address          is the BT address of the specified remote device.
 * @return                          is the recommended codec type by user.
 */
bt_source_srv_codec_t bt_source_srv_get_audio_codec_type(bt_source_srv_t type, const bt_addr_t *peer_address);

/**
 * @}
 * @}
 * @}
 */
#endif
