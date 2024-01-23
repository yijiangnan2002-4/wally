/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_DONGLE_SESSION_MANAGER_H__
#define __APP_DONGLE_SESSION_MANAGER_H__

#ifdef AIR_BT_SOURCE_ENABLE
#include "bt_source_srv.h"
#endif

#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_UNSPECIFIED          0x00            /**< Free choice. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_8KHZ                 0x01            /**< 8 kHz. refer to #SUPPORTED_SAMPLING_FREQ_8KHZ*/
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_16KHZ                0x03            /**< 16 kHz. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_24KHZ                0x05            /**< 24 kHz. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_32KHZ                0x06            /**< 32 kHz. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_44_1KHZ              0x07            /**< 44.1 kHz. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_48KHZ                0x08            /**< 48 kHz. */
#define APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_96KHZ                0x0A            /**< 96 kHz. */
typedef uint8_t app_dongle_session_manager_sample_freq_t;

#define APP_DONGLE_SESSION_MGR_MIC_LOCATION_NONE                        0x00
#define APP_DONGLE_SESSION_MGR_MIC_LOCATION_L                           0x01
#define APP_DONGLE_SESSION_MGR_MIC_LOCATION_R                           0x02
typedef uint8_t app_dongle_session_manager_mic_location_t;

//High 16 bits: Sink, Low 16 bits: Source, refer to #SUPPORTED_SAMPLING_FREQ_8KHZ
#define APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED             0x00000000
#define APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_COMM_16KHZ              0x00040004 //((SUPPORTED_SAMPLING_FREQ_16KHZ<<16)|SUPPORTED_SAMPLING_FREQ_16KHZ)
#define APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_COMM_32KHZ              0x00200020 //((SUPPORTED_SAMPLING_FREQ_32KHZ<<16)|SUPPORTED_SAMPLING_FREQ_32KHZ)
#define APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_MEDIA_48KHZ             0x00800000 //(SUPPORTED_SAMPLING_FREQ_48KHZ<<16)
#define APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_MEDIA_96KHZ             0x02000000 //(SUPPORTED_SAMPLING_FREQ_96KHZ<<16)
typedef uint32_t app_dongle_session_manager_lea_session_type_t;


#define APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA                      0x00
#define APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION              0x01
#define APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX                        0x02
typedef uint8_t app_dongle_session_manager_session_usage_t;

#define APP_DONGLE_SESSION_MGR_RESTART                                  0x00000001
typedef uint32_t app_dongle_session_manager_event_t;

typedef struct {
     app_dongle_session_manager_sample_freq_t sink_sample_freq;     //Audio streaming from dongle to headset
     app_dongle_session_manager_sample_freq_t source_sample_freq;   //Audio streaming from headset to dongle
     app_dongle_session_manager_mic_location_t mic_location;
} app_dongle_session_manager_lea_session_info_t;

typedef struct {
    bt_handle_t partner_connection_handle;
    uint16_t sink_sampling_frequencies;     // refer to #SUPPORTED_SAMPLING_FREQ_8KHZ
    uint16_t source_sampling_frequencies;   // refer to #SUPPORTED_SAMPLING_FREQ_8KHZ
    uint32_t profile;
} app_dongle_session_manager_lea_param_t;

typedef struct {
    bt_status_t (*lea_session_type_noify)(bt_handle_t handle,app_dongle_session_manager_lea_param_t* parameter);
    bt_status_t (*lea_session_disconnected)(bt_handle_t handle);
    bt_status_t (*lea_session_negotiation)(bt_handle_t handle, app_dongle_session_manager_session_usage_t session_usage, app_dongle_session_manager_lea_session_info_t *session_info);
} app_dongle_session_manager_lea_handle_t;


#ifdef AIR_BT_SOURCE_ENABLE
typedef bt_source_srv_codec_t app_dongle_session_manager_edr_session_type_t;

typedef struct {
     app_dongle_session_manager_edr_session_type_t session_type;
     app_dongle_session_manager_mic_location_t mic_location;
} app_dongle_session_manager_edr_session_info_t;

bt_status_t app_dongle_session_manager_handle_edr_session_type(bt_source_srv_event_t event,void *parameter);
bt_status_t app_dongle_session_manager_handle_edr_session_negotiation(bt_addr_t *edr_addr,
    app_dongle_session_manager_session_usage_t session_usage,
    app_dongle_session_manager_edr_session_info_t *session_info);

#endif

#endif

