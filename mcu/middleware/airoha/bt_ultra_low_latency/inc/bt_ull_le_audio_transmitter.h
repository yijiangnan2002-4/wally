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

#ifndef __BT_ULL_LE_AUDIO_TRANSMITTER_H__
#define __BT_ULL_LE_AUDIO_TRANSMITTER_H__


#include "bt_type.h"
#include "bt_ull_utility.h"
#include "bt_ull_service.h"
#if defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "audio_transmitter_control.h"
#endif
#include "bt_ull_le_utility.h"


BT_EXTERN_C_BEGIN


/**
 *  @brief Define the max index of transmitter type.
 */
#define BT_ULL_LE_DL_GAMING_TRANSMITTER            (0x00) 
#define BT_ULL_LE_DL_CHAT_TRANSMITTER              (0x01) 
#define BT_ULL_LE_DL_LINEIN_TRANSMITTER            (0x02) 
#define BT_ULL_LE_DL_I2SIN_TRANSMITTER             (0x03) 
#define BT_ULL_LE_DL_TRANSMITTER_END               (BT_ULL_LE_DL_I2SIN_TRANSMITTER) 

#define BT_ULL_LE_MAX_DL_STREAMING_NUM             (BT_ULL_LE_DL_TRANSMITTER_END + 1)//BT_ULL_LE_MAX_TRANSMITTER_NUM

/**
 *  @brief Define the audio transmitter event type of ULL dongle LE.
 */
typedef uint8_t bt_ull_le_at_event_t;
#define BT_ULL_LE_AT_EVENT_START_IND               (0x00)
#define BT_ULL_LE_AT_EVENT_STOP_IND                (0x01)


/**
 *  @brief Define the audio transmitter audio out type of ULL dongle LE.
 */
typedef uint8_t bt_ull_le_transmitter_audio_out_t;
#define BT_ULL_LE_TRANSMITTER_AUDIO_OUT_USB        (0x00)
#define BT_ULL_LE_TRANSMITTER_AUDIO_OUT_AUX        (0x01)
#define BT_ULL_LE_TRANSMITTER_AUDIO_OUT_I2S        (0x02)

/**
* @brief Defines the silence detection  mode for dongle
*/
typedef uint8_t bt_ull_le_at_silence_detection_mode_t;
#define BT_ULL_LE_AT_SILENCE_DETECTION_MODE_NORMAL     (0x00)
#define BT_ULL_LE_AT_SILENCE_DETECTION_MODE_SPECIAL    (0x01)


/**
 *  @brief Define the audio transmitter result notify strcture.
 */
typedef struct {
    bt_ull_transmitter_t transmitter_type;
    bt_status_t result;
} bt_ull_le_at_result_t;


/**
 *  @brief Define the audio transmitter callback which is used to notify uplayer.
 */
typedef void (* bt_ull_le_at_callback)(bt_ull_le_at_event_t event, void *param, uint32_t param_len);


/**
 * @brief   This function is used for init audio stream info.
 * @param[in] callback    is callback function.
 * @return                void                       
 */
void bt_ull_le_at_init_ctx(bt_ull_le_at_callback callback);

/**
 * @brief       This function is used for start transmitter.
 * @param[in]transmitter_type  
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_start(bt_ull_transmitter_t transmitter_type, bool is_app_request);

/**
 * @brief       This function is used for stop transmitter.
 * @param[in]   transmitter_type  
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_stop(bt_ull_transmitter_t transmitter_type, bool is_app_request);

/**
 * @brief       This function is used for init transmitter.
 * @param[in]   client_type is the type of client
 * @param[in]   out_type
 * @param[in]   transmitter_type
 * @param[in]   codec
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_init(bt_ull_client_t client_type, uint8_t out_type, bt_ull_transmitter_t transmitter_type, bt_ull_le_codec_t codec);

/**
 * @brief       This function is used for deinit transmitter.
 * @param[in]   transmitter_type  
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_deinit(bt_ull_transmitter_t transmitter_type, bool is_app_request);

/**
 * @brief       This function is used for set transmitter volume.
 * @param[in]   transmitter_type
 * @param[in]   action
 * @param[in]   channel type
 * @param[in]   volume
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */

bt_status_t bt_ull_le_at_set_volume(bt_ull_transmitter_t transmitter_type, bt_ull_volume_action_t action, bt_ull_audio_channel_t channel, uint8_t volume);

/**
 * @brief       This function is used for set transmitter mute or unmute.
 * @param[in]   transmitter_type
 * @param[in]   is_mute
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_set_mute(bt_ull_transmitter_t transmitter_type, bool is_mute);


/**
 * @brief       This function is used for set transmitter sample rate.
 * @param[in]   transmitter_type
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_restart(bt_ull_transmitter_t transmitter_type);

/**
 * @brief       This function is used for judge transmitter is start or not.
 * @param[in]   transmitter_type
 * @return      true, transmitter is start.
 *              false, transmitter is not start
 */
bool bt_ull_le_at_is_start(bt_ull_transmitter_t transmitter_type);

/**
 * @brief       This function is used for set transmitter mix.
 * @param[in]   ratio
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_at_set_mix_ratio(bt_ull_mix_ratio_t *ratio);

/**
 * @brief       This function is used for set transmitter latency.
 * @param[in]   latency
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */
void bt_ull_le_at_set_latency(uint16_t ul_latency, uint16_t dl_latency);
#if (defined AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
bt_status_t bt_ull_le_at_set_streaming_param(bt_ull_interface_config_t *param);
#endif

/**
 * @brief     This function is to read data when transmitter is started.
 * @param[in] transmitter_type                 Audio transmitter type.
 * @param[in] data               The destination address where the data are read from the audio transmitter.
 * @param[in] length             Length of data read from the audio transmitter.
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                 #BT_STATUS_FAIL, the operation has failed.
 */

bt_status_t bt_ull_le_at_read_pcm_data(bt_ull_transmitter_t transmitter_type, uint8_t *data, uint32_t *length);

/**
 * @brief     This function is to write data when transmitter is started.
 * @param[in] transmitter_type                 Audio transmitter type.
 * @param[in] data               The source address where the data are writen to the audio transmitter.
 * @param[in] length             Length of data that write to the audio transmitter.
 * @return
 * @return      #BT_STATUS_SUCCESS, the operation completed successfully.
 *              #BT_STATUS_FAIL, the operation has failed.
 */

bt_status_t bt_ull_le_at_write_pcm_data(bt_ull_transmitter_t transmitter_type, uint8_t *data, uint32_t *length);

bt_status_t bt_ull_le_at_set_ul_channel_locaton(uint32_t location);


#ifdef AIR_WIRELESS_MIC_RX_ENABLE
bt_status_t bt_ull_le_at_set_audio_connection_info(bt_ull_transmitter_t transmitter_type, void *audio_conenction_info, uint32_t size);

bt_status_t bt_ull_le_at_update_client_addr(bt_ull_transmitter_t transmitter_type, uint8_t link_index, bt_bd_addr_t *client_adress);
bt_status_t bt_ull_le_at_set_wireless_mic_safety_mode_volume_by_channel(bt_ull_transmitter_t transmitter_type, S32 left_vol_diff, S32 right_vol_diff);

void bt_ull_le_at_set_wireless_mic_client_link_info(uint8_t link_index, bt_bd_addr_t *client_adress);
void bt_ull_le_at_clear_wireless_mic_client_link_info_by_addr(bt_bd_addr_t *client_adress);
uint8_t bt_ull_le_at_get_volume_value(bt_ull_transmitter_t transmitter_type);
#endif

#if (defined AIR_ULL_AUDIO_V2_DONGLE_ENABLE) && (defined AIR_SILENCE_DETECTION_ENABLE)
bt_ull_le_at_silence_detection_mode_t bt_ull_le_at_get_silence_detection_mode(void);
#endif

bool bt_ull_le_at_is_prepare_start(bt_ull_transmitter_t transmitter_type); 
bool bt_ull_le_at_is_any_transmitter_start(bt_ull_role_t role);
bt_status_t bt_ull_le_at_reinit( bt_ull_transmitter_t transmitter_type);



BT_EXTERN_C_END


#endif
