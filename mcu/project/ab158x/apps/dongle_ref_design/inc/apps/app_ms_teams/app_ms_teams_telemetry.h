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
/* Airoha restricted information */

#ifndef __APP_MS_TEAMS_TELEMETRY_H__
#define __APP_MS_TEAMS_TELEMETRY_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Notice!!!
 *
 * Some project-specific telemetry items need to be customized according to the project. 
 * We define a tag to mark which feature need be customized by customers.  
 * You can search the TEAMS_CUSTOMIZATION for them.
 * 
 * While other telemetry items have default values and we have done the testing and 
 * can guarantee that they can pass the certification test, so we recommend not modifying 
 * these values if there is no special reason.
 * 
 *
 * Must to be customized:
 *      Current Firmware (endpoint)
 *      Current Firmware (dongle/base)
 *      Endpoint Device Model ID
 *      Device Serial Number (endpoint)
 *      Device Serial Number (dongle/base)
 * Recommend not modifying these values if there is no special reason:
 *      Don to answer setting
 *      User modified SideTone Level
 *      Audio codec used
 *      DSP effects enabled by headset
 *      Battery Level
 *      Device Ready
 *      Radio Link Quality
 *      Connected wireless device change
 *      Local conference count
 *      Voice activity while muted
 *      Button press info
 * Depends on HW design:
 *      Hardmute/Mute Lock
 *      Headset worn
 */

typedef float float32_t;

/**
* @brief      This function is used to load default telemetry setting while system boot up.
* @return     None
*/
void app_ms_teams_load_default_telemetry_setting();

/* TEAMS_CUSTOMIZATION */
/**
* @brief      Set the endpoint fw version, it's the headset/earbuds fw version. You could
*             use it set a default value in function app_ms_teams_load_default_telemetry_setting.
*             This function will be called while headset/earbuds and dongle handshake done.
*             If you want to change it, please reference Airoha_IoT_SDK_Firmware_Update_Developers_Guide.pdf
* @param[in]  ver is the fw version string, this string should only be composed of letters
*             and numbers and "_" and ".", such as "V1.0.0". The version max length is 32bytes.
* @param[in]  len is the length of version string
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_endpoint_fw_version(uint8_t *ver, uint32_t len);

/* TEAMS_CUSTOMIZATION */
/**
* @brief      Set the dongle fw version in function app_ms_teams_load_default_telemetry_setting.
*             If you want to change it, please reference Airoha_IoT_SDK_Firmware_Update_Developers_Guide.pdf
* @param[in]  ver is the fw version string, this string should only be composed of letters
*             and numbers and "_" and ".", such as "V1.0.0". The version max length is 32bytes.
* @param[in]  len is the length of version string
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_dongle_fw_version(uint8_t *ver, uint32_t len);

/**
* @brief      Set the don to answer setting, true means auto accept the incoming call.
* @param[in]  en default is false
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_don_to_answer_setting(bool en);

/* TEAMS_CUSTOMIZATION */
/**
* @brief      Set the endpoint model id, it's the headset/earbuds project name. You could
*             use it set a default value in function app_ms_teams_load_default_telemetry_setting.
*             This function will be called while headset/earbuds and dongle handshake done.
* @param[in]  model_id is the model string, this string should only be composed of letters
*             and numbers and "_" and ".". The version max length is 32bytes.
* @param[in]  len is the length of model name string
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_endpoint_device_model_id(uint8_t *model_id, uint32_t len);

/* TEAMS_CUSTOMIZATION */
/**
* @brief      Set the dongle serial number.
*             You could use it set a default value in function app_ms_teams_load_default_telemetry_setting.
*             And this fw version is storage in NVKEY NVID_TEAMS_DEVICE_SN. It could be changed by
*             Airoha config tool.
* @param[in]  sn is the serial number string, this string should only be composed of letters
*             and numbers and "_" and ".", such as "SN00001". The version max length is 32bytes.
* @param[in]  len is the length of serial number string
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_dongle_device_SN(uint8_t *sn, uint32_t len);

/* TEAMS_CUSTOMIZATION */
/**
* @brief      Set the endpoint device serial number, it's the headset/earbuds serial number.
*             This function will be called while headset/earbuds and dongle handshake done.
*             You could use it set a default value in function app_ms_teams_load_default_telemetry_setting.
*             And this fw version is storage in NVKEY NVID_TEAMS_DEVICE_SN on the headset/earbuds side.
*             It could be changed by Airoha config tool.
* @param[in]  sn is the serial number string, this string should only be composed of letters
*             and numbers and "_" and ".", such as "SN00001". The version max length is 32bytes.
* @param[in]  len is the length of serial number string
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_endpoint_device_SN(uint8_t *sn, uint32_t len);

/**
* @brief      This function should be called when the sidetone level changed.
*             Now SDK always is "0.0".
* @param[in]  db_diff is the side tone level diff value when the level changed.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_sidetone_level(float32_t db_diff);

typedef enum {
    APP_MS_TEAMS_TELEMETRY_AUDIO_CODEC_NARROWBAND = 0x01,
    APP_MS_TEAMS_TELEMETRY_AUDIO_CODEC_WIDEBAND = 0x02
} app_ms_teams_telemetry_audio_codec_type_t;
/**
* @brief      This function will be called when the audio codec changed.
*             Now SDK always keep in wide band.
* @param[in]  type see the app_ms_teams_telemetry_audio_codec_type_t definition.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_audio_codec_used(app_ms_teams_telemetry_audio_codec_type_t type);

#define APP_MS_TEAMS_TELEMETRY_MASK_ACOUSTIC_ECHO_CANCELLATION 0X00000001       /* AcousticEchoCancellation */
#define APP_MS_TEAMS_TELEMETRY_MASK_AUTOMATIC_GAIN_CONTROL     0X00000002           /* AutomaticGainControl */
#define APP_MS_TEAMS_TELEMETRY_MASK_NOISE_SUPPRESSION          0X00000004                /* NoiseSuppression */
#define APP_MS_TEAMS_TELEMETRY_MASK_SPEAKER_PROTECTION 0X00000008               /* SpeakerProtection */
#define APP_MS_TEAMS_TELEMETRY_MASK_BEAM_FORMING 0X00000010                     /* BeamForming */
#define APP_MS_TEAMS_TELEMETRY_MASK_CONSTANT_TONE_REMOVAL 0X00000020            /* ConstantToneRemoval */
#define APP_MS_TEAMS_TELEMETRY_MASK_EQUALIZER 0X00000040                        /* Equalizer */
#define APP_MS_TEAMS_TELEMETRY_MASK_LOUDNESS_EQUALIZER 0X00000080               /* LoudnessEqualizer */
#define APP_MS_TEAMS_TELEMETRY_MASK_BASS_BOOST 0X00000100                       /* BassBoost */
#define APP_MS_TEAMS_TELEMETRY_MASK_VIRTUAL_SURROUND 0X00000200                 /* VirtualSurround */
#define APP_MS_TEAMS_TELEMETRY_MASK_VIRTUAL_HEADPHONES 0X00000400               /* VirtualHeadphones */
#define APP_MS_TEAMS_TELEMETRY_MASK_SPEAKER_FILL 0X00000800                     /* SpeakerFill */
#define APP_MS_TEAMS_TELEMETRY_MASK_ROOM_CORRECTION 0X00001000                  /* RoomCorrection */
#define APP_MS_TEAMS_TELEMETRY_MASK_BASS_MANAGEMENT 0X00002000                  /* BassManagement */
#define APP_MS_TEAMS_TELEMETRY_MASK_ENVIRONMENTAL_EFFECTS 0X00004000            /* EnvironmentalEffects */
#define APP_MS_TEAMS_TELEMETRY_MASK_SPEAKER_COMPENSATION 0X00008000             /* SpeakerCompensation */
#define APP_MS_TEAMS_TELEMETRY_MASK_DYNAMIC_RANGE_COMPRESSION 0X00010000        /* DynamicRangeCompression */
#define APP_MS_TEAMS_TELEMETRY_MASK_OTHER 0X00020000                            /* Other */
/**
* @brief      This function will be called when the DSP effects changed by headset/earbuds.
*             And it will be called in function app_ms_teams_load_default_telemetry_setting to set a default value.
*             Now the SDK only handle the ANC on/off event to change this mask. You could change this value
*             according to your product design.
* @param[in]  mask see the MS_TEAMS_DEFAULT_DSP_EFFECT definition.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_dsp_effect_mask(uint32_t mask);

/**
* @brief      If your headset support hardware mic enable/disable switch, such as rotary switch.
*             Use this function to report hardware mic enable/disable event.
* @param[in]  on_off should be set to true if hardware mic enable, false while disabled.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_hardmute_lock(bool on_off);


/**
* @brief      If your device support wear detection, call this function when the wear status changed. 
* @param[in]  on_off true means the wearing on, false means wearing off.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_headset_worn(bool on_off);

typedef enum {
    APP_MS_TEAMS_TELEMETRY_BATTERY_HIGH = 4,
    APP_MS_TEAMS_TELEMETRY_BATTERY_MIDDLE = 3,
    APP_MS_TEAMS_TELEMETRY_BATTERY_CRITICALLY_LOW = 2,
    APP_MS_TEAMS_TELEMETRY_BATTERY_OFF = 1,
} app_ms_teams_telemetry_battery_level_t;

/**
* @brief      This function will be called when the battery level changed.
* @param[in]  level reference the app_ms_teams_telemetry_battery_level_t type definition
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_battery_level(app_ms_teams_telemetry_battery_level_t level);

/**
* @brief      This function will be called when the headset/earbuds connected or disconnected with dongle.
* @param[in]  yes_no true means connected and false means disconnected.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_device_ready(bool yes_no);

typedef enum {
    APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_HIGH = 3,
    APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_LOW = 2,
    APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_OFF = 1, /* Means connection disconnected */
} app_ms_teams_telemetry_link_quality_t;

/**
* @brief      This function will be called when the link quality changed.
* @param[in]  quality reference the app_ms_teams_telemetry_link_quality_t type definition.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_link_quality(app_ms_teams_telemetry_link_quality_t quality);

/**
* @brief      This function will be called when some error happens.
* @param[in]  msg is the error message string.
* @param[in]  len is the error message string length.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_error_message(uint8_t *msg, uint32_t len);

/**
* @brief      This function will be called when accept or end the call.
* @param[in]  on_off true means button on and off means button off.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_button_press_info_hook(bool on_off);

/**
* @brief      This function will be called when the button pressed during call active.
* @param[in]  on_off true means mute and false means unmute.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_button_press_info_mute(bool on_off);

/**
* @brief      This function will be called when the user press key to switch the call.
*             Such as, call A active, call B is incoming, press key to accept call B and hold call A.
* @param[in]  on_off true means button on and false means button off.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_button_press_info_flash(bool on_off);

/**
* @brief      This function will be called when the connected device changed which connected with dongle.
*             Such as, A connected firstly and then B connected with dongle, set param "yes_no" to be true.
*             Then B disconnected and reconnect again, set param "yes_no" to be false.
*             This function only be called on dongle side.
* @param[in]  yes_no true means new device connected or active device changed and false means the no changes.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_connected_wireless_device_changed(bool yes_no);

/**
* @brief      This function should be called when the connected device count changed.
*             All of dongle/earbuds/headset need call this api.
*             Such as, A first connected, set param "count" to be 1,
*             and then B connected, set param "count" to be 2.
* @param[in]  count is the numbers of audio devices.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_local_reference_count(uint32_t count);

/**
* @brief      This function will be called when the voice detected if microphone muted during call.
* @param[in]  yes_no true means voice detected and false means not detected.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_voice_detected(bool yes_no);

#endif /* __APP_MS_TEAMS_TELEMETRY_H__ */

