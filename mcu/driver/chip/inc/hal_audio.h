/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __HAL_AUDIO_H__
#define __HAL_AUDIO_H__
#include "hal_platform.h"
#include "hal_audio_message_struct_common.h"

#ifdef HAL_AUDIO_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup AUDIO
 * @{
 * This section introduces the Audio APIs including terms and acronyms, supported features, software architecture,
 * details on how to use this driver, Audio function groups, enums, structures and functions.
 *
 * @section Driver_Usage_Chapter How to use this driver
 *
 * - Initiate the audio play function and use an earphone to play it out. \n
 *  - step1: call #hal_audio_set_stream_out_sampling_rate() to configure the audio sampling rate.
 *  - step2: call #hal_audio_set_stream_out_channel_number() to configure the audio channel number.
 *  - step3: call #hal_audio_set_stream_out_device() to configure audio output device.
 *  - step4: call #hal_audio_start_stream_out() to start the audio playback.
 *  - sample code:
 *    @code
 *      static void user_audio_stream_out_callback(hal_audio_event_t event, void *user_data)
 *      {
 *          switch(event) {
 *              case HAL_AUDIO_EVENT_UNDERFLOW:
 *                //Provide more data once this event is received.
 *                //Audio output might be discontinuous since there is data underflow.
 *                break;
 *              case HAL_AUDIO_EVENT_DATA_REQUEST:
 *                //Fill with output data, if available.
 *                break;
 *              case HAL_AUDIO_EVENT_ERROR:
 *                //Error handler.
 *                break;
 *           }
 *      }
 *
 *      uint8_t  *memory;
 *      static void audio_play(void)
 *      {
 *          uint32_t memory_size = 0;
 *
 *          hal_audio_set_stream_out_sampling_rate(HAL_AUDIO_SAMPLING_RATE_48KHZ);
 *          hal_audio_set_stream_out_channel_number(HAL_AUDIO_STEREO);
 *          hal_audio_set_stream_out_device(HAL_AUDIO_DEVICE_HEADSET);
 *          hal_audio_register_stream_out_callback(user_audio_stream_out_callback, NULL);
 *
 *          // Get the amount of internal memory usage.
 *          hal_audio_get_memory_size(&memory_size);
 *          memory = malloc(memory_size * sizeof(uint8_t *));
 *          if(NULL == memory){
 *             //Error handler, if memory allocation failed.
 *          }
 *          result = hal_audio_set_memory(memory);
 *          if(HAL_AUDIO_STATUS_OK != result){
 *             //Error handler.
 *          }
 *
 *          hal_audio_start_stream_out(HAL_AUDIO_PLAYBACK_MUSIC);
 *      }
 *
 *      static void audio_stop(void)
 *      {
 *          hal_audio_stop_stream_out();
 *          //Free the internal memory allocation
 *          free(memory);
 *      }
 *    @endcode
 *
 * The APIs are grouped based on their functionality for more convenient usage.
 * For more information about each function, please click on the function name.
 * - \b Initialization \b and \b configuration. \n
 *   The #hal_audio_init() function intitializes the audio hardware. Call this function when the system restarts. \n
 *   The #hal_audio_set_stream_out_sampling_rate() and #hal_audio_set_stream_in_sampling_rate() are used to set the sampling rate. \n When audio application is related to voice, stream in/out
 *   sampling rate should be set to 8kHz for narrowband and 16kHz for wideband. \n Use #hal_audio_set_stream_out_channel_number() and #hal_audio_set_stream_in_channel_number() to set the channel number.\n
 *   The sampling rate and channel number configuration functions should be called before powering on the audio application, and cannot be changed during the application execution. \n
 *   Which means the user needs to reconfigure the settings to start a new audio track or application. \n
 *   Specify the #hal_audio_set_stream_out_device() and # hal_audio_set_stream_in_device() functions before starting the audio streaming. Device settings could be changed even if the audio is active.
 *   The functions are:
 *   - #hal_audio_init()
 *   - #hal_audio_deinit()
 *   - #hal_audio_set_stream_out_sampling_rate()
 *   - #hal_audio_set_stream_out_channel_number()
 *   - #hal_audio_set_stream_out_device()
 *   - #hal_audio_set_stream_in_sampling_rate()
 *   - #hal_audio_set_stream_in_channel_number()
 *   - #hal_audio_set_stream_in_device()
 *
 * - \b Callback \b registration \b functions \n
 *   Use the following functions to register callback functions to stream output, respectively.
 *  - #hal_audio_register_stream_out_callback()
 *
 * - \b Enable \b and \b disable \b functions. \n
 *   #hal_audio_start_stream_out() function enables the output audio signal streaming hardware.
 *   #hal_audio_stop_stream_out() function immediately stops the output streaming. \n
 *
 * - \b Volume \b control \b functions \n
 *   Call #hal_audio_set_stream_out_volume() and #hal_audio_set_stream_in_volume() to set the volume of stream out and in, respectively. \n The volume can be changed during the application execution. It has digital and analog gain controls. The gain range is chip dependent. Please refer to the chip specification to setup the range. \n Mute functions are also provided. Call #hal_audio_mute_stream_out()
 *   and #hal_audio_mute_stream_in() to mute the audio path. Note, that unmute should be called after mute is set when the audio needs to be played out or recorded in.
 *   The functions are:
 *   - #hal_audio_mute_stream_out()
 *   - #hal_audio_set_stream_out_volume()
 *   - #hal_audio_mute_stream_in()
 *   - #hal_audio_set_stream_in_volume()
 *
 * - \b Data \b transfer \b functions \n
 *   - Stream out transfer
 *    The #hal_audio_get_stream_out_sample_count() should be called before #hal_audio_write_stream_out() to know how many free space of output buffer can be written. The functions are:
 *    - #hal_audio_write_stream_out()
 *    - #hal_audio_get_stream_out_sample_count()
 *    - sample code:
 *    @code
 *     uint8_t user_buffer[1024] = { //with data };
 *     static void audio_write_data(void)
 *     {
 *         uint32_t sample_count = 0;
 *         uint8_t *data_buffer = (uint8_t *)user_buffer;
 *         // Get available free space of output buffer.
 *         hal_audio_get_stream_out_sample_count(&sample_count);
 *         //Write numbers of sample_count samples for audio out.
 *         hal_audio_write_stream_out(data_buffer, sample_count);
 *     }
 *    @endcode
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup hal_audio_constant Constant
  * @{
  */

#define HAL_AUDIO_GAIN_KEEP     0x7FFF /**< To keep original gain for volume control  */

/**
  * @}
  */


/** @defgroup hal_audio_enum Enum
  * @{
  */

/**
 * @brief  This macro defines the fixed afe sample rate.
 */
#define HAL_AUDIO_FIXED_AFE_SAMPLE_RATE 48000 /**<  To set the sample rate of the audio scenario. */

/** @brief Audio status. */
typedef enum {
    HAL_AUDIO_STATUS_INVALID_PARAMETER = -2,  /**<  A wrong parameter is given. */
    HAL_AUDIO_STATUS_ERROR             = -1,  /**<  An error occured during the function call. */
    HAL_AUDIO_STATUS_OK                =  0   /**<  No error occurred during the function call. */
} hal_audio_status_t;

/** @brief Audio event. */
typedef enum {
    HAL_AUDIO_EVENT_ERROR               = -1, /**<  An error occurred during the function call. */
    HAL_AUDIO_EVENT_NONE                =  0, /**<  No error occurred during the function call. */
    HAL_AUDIO_EVENT_UNDERFLOW           =  1, /**<  Transmit data underflow. */
    HAL_AUDIO_EVENT_DATA_REQUEST        =  2, /**<  Send a user request to provide data. */
    HAL_AUDIO_EVENT_DATA_NOTIFICATION   =  3, /**<  This value means notify user rx data is ready. */
    HAL_AUDIO_EVENT_END                 =  4, /**<  No more audio data from user.*/
    HAL_AUDIO_EVENT_TIME_REPORT         =  5, /**<  Report the timestamp and sample for BT audio sync. */
    HAL_AUDIO_EVENT_LTCS_REPORT         =  6, /**<  Report A2DP LTCS data. */
    HAL_AUDIO_EVENT_LTCS_TIMEOUT        =  7, /**<  [A2DP] WPC PCDC SYNC. */
    HAL_AUDIO_EVENT_DL_REINIT_REQUEST   =  8, /**<  Request to re-init downlink path for BT audio sync. */
    HAL_AUDIO_EVENT_HFP_PARA_SEND       =  9, /**<  [HFP] DSP Request to send parameter from agent to partner. */
    HAL_AUDIO_EVENT_ALC_REQUEST         =  10,/**<  [A2DP] Active Latency control. */
    HAL_AUDIO_EVENT_WWE_VERSION         =  11,/**<  Wake Word Detected. */
    HAL_AUDIO_EVENT_WWD_NOTIFICATION    =  12,/**<  Wake Word Detected. */
    HAL_AUDIO_EVENT_DATA_ABORT_NOTIFICATION   =  13,/**<  Wake Word Not Detected. */
    HAL_AUDIO_EVENT_DATA_DIRECT         =  14,/**<  Send user 32bit data by ccni message. */
    HAL_AUDIO_EVENT_AUDIO_NVDM          =  15,/**<  audio NVDM relative. */
    HAL_AUDIO_EVENT_LEQ_PARA_SEND       =  16,/**<  [HFP/LE audio call] DSP Request to send LEQ(Equivalent Continuous Sound Pressure Level) from agent to partner. */
} hal_audio_event_t;

/** @brief Mark highest bit for the valid of scenario type. */
#define HAL_AUDIO_STREAM_IN_SCENARIO_MARK 0x8000

/** @brief Define the audio stream in scenario type. */
typedef enum {
    HAL_AUDIO_STREAM_IN_SCENARIO_HFP = 0,  /**<  stream in scenario for HFP. */
    HAL_AUDIO_STREAM_IN_SCENARIO_LINE_IN,  /**<  stream in scenario for LINE IN. */
    HAL_AUDIO_STREAM_IN_SCENARIO_BLE_CALL, /**<  stream in scenario for BLE CALL. */
    HAL_AUDIO_STREAM_IN_SCENARIO_MAX,      /**<  stream in scenario max. */
    HAL_AUDIO_STREAM_IN_SCENARIO_INVALID,  /**<  stream in scenario invalid. */
} hal_audio_stream_in_scenario_t;

/** @brief Define the audio mute function. */
typedef enum {
    HAL_AUDIO_MUTE_MAIN_MIC    = 0x0001, /**<  Mute main mic. */
    HAL_AUDIO_MUTE_EAR_MIC     = 0x0002, /**<  Mute earphone mic. */
    HAL_AUDIO_MUTE_HANDSET     = 0x0004, /**<  Mute receiver. */
    HAL_AUDIO_MUTE_HANDS_FREE  = 0x0008, /**<  Mute loudspeaker. */
    HAL_AUDIO_MUTE_HEADSET     = 0x0010, /**<  Mute earphone. */
    HAL_AUDIO_MUTE_LINE_IN     = 0x0020  /**<  Mute line in path. */
} hal_audio_mute_t;

/** @brief Define the audio mute function. */
typedef enum {
    HAL_AUDIO_MUTE_FUNCTION_NONE  = 0,   /**<  No function is disabled. */
    HAL_AUDIO_MUTE_FUNCTION_PCM   = 1    /**<  PCM playback path is disabled. */
} hal_audio_mute_function_t;

/** @brief Define audio sampling rate. */
typedef enum {
    HAL_AUDIO_SAMPLING_RATE_8KHZ      = 0, /**< 8000Hz  */
    HAL_AUDIO_SAMPLING_RATE_11_025KHZ = 1, /**< 11025Hz */
    HAL_AUDIO_SAMPLING_RATE_12KHZ     = 2, /**< 12000Hz */
    HAL_AUDIO_SAMPLING_RATE_16KHZ     = 3, /**< 16000Hz */
    HAL_AUDIO_SAMPLING_RATE_22_05KHZ  = 4, /**< 22050Hz */
    HAL_AUDIO_SAMPLING_RATE_24KHZ     = 5, /**< 24000Hz */
    HAL_AUDIO_SAMPLING_RATE_32KHZ     = 6, /**< 32000Hz */
    HAL_AUDIO_SAMPLING_RATE_44_1KHZ   = 7, /**< 44100Hz */
    HAL_AUDIO_SAMPLING_RATE_48KHZ     = 8, /**< 48000Hz */
    HAL_AUDIO_SAMPLING_RATE_88_2KHZ   = 9, /**< 88200Hz */
    HAL_AUDIO_SAMPLING_RATE_96KHZ     = 10,/**< 96000Hz */
    HAL_AUDIO_SAMPLING_RATE_176_4KHZ  = 11,/**< 176400Hz */
    HAL_AUDIO_SAMPLING_RATE_192KHZ    = 12,/**< 192000Hz */
} hal_audio_sampling_rate_t;

/** @brief Define the number of bits per second (bps) to stream audio data. */
typedef enum {
    HAL_AUDIO_BITS_PER_SAMPLING_16    = 0, /**< 16 bps */
    HAL_AUDIO_BITS_PER_SAMPLING_24    = 1  /**< 24 bps */
} hal_audio_bits_per_sample_t;

/** @brief audio function define */
typedef enum {
    HAL_AUDIO_PLAYBACK_MUSIC     = 0, /**< Audio playback: music */
    HAL_AUDIO_PLAYBACK_VOICE     = 1, /**< Voice playback: voice */
    HAL_AUDIO_RECORD_VOICE       = 2, /**< Record voice */
    HAL_AUDIO_TWO_WAY_VOICE      = 3, /**< Turn on both voice stream in and out */
    HAL_AUDIO_PLAYBACK_MUSIC_RECORD_VOICE = 4  /**< Audio playback: music and Record voice */
} hal_audio_active_type_t;

/** @brief audio interface define */

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
/** @brief audio I2S Slave TDM channel setting. */
typedef enum {
    HAL_AUDIO_I2S_TDM_DISABLE = 0x0,              /**< I2S TDM channel setting*/
    HAL_AUDIO_I2S_TDM_2CH     = 0x1,              /**< I2S TDM channel setting*/
    HAL_AUDIO_I2S_TDM_4CH     = 0x2,              /**< I2S TDM channel setting*/
    HAL_AUDIO_I2S_TDM_6CH     = 0x3,              /**< I2S TDM channel setting*/
    HAL_AUDIO_I2S_TDM_8CH     = 0x4,              /**< I2S TDM channel setting*/
} hal_audio_i2s_tdm_channel_setting_t;
#endif

/**
  * @}
  */

/** @defgroup hal_audio_struct Struct
  * @{
  */

/** @brief Audio stream mode structure */
typedef struct {
    hal_audio_sampling_rate_t   stream_sampling_rate;  /**< Specifies the sampling rate of audio data.*/
    hal_audio_bits_per_sample_t stream_bit_rate;       /**< Specifies the number of bps of audio data.*/
    hal_audio_channel_number_t  stream_channel;         /**< Specifies the number of channel.*/
    hal_audio_device_t          audio_device;          /**< Specifies the device.*/
    uint32_t                    digital_gain_index;    /**< Digital gain index of the audio stream.*/
    uint32_t                    analog_gain_index;     /**< Analog gain index of the audio stream.*/
} hal_audio_stream_mode_t;

/** @brief audio structure */
typedef struct {
    hal_audio_stream_mode_t     stream_in;               /**< Audio input configuration*/
    hal_audio_stream_mode_t     stream_out;              /**< Audio output configuration*/
    hal_audio_active_type_t     audio_path_type;         /**< Audio function type*/
    hal_audio_mute_t            mute_device;             /**< Audio mute devices*/
    bool                        loopback;                /**< Audio loopback configuration*/
} hal_audio_stream_t;


/** @brief audio structure */
typedef struct {
    void*                       user_data;              /**< User data provided by user*/
    uint8_t                     *buffer;                /**< Buffer pointer to stream out data*/
    uint32_t                    buffer_size;            /**< Size of the buffer*/
    uint32_t                    sample_rate;            /**< Stream out data sampling rate*/
    uint32_t                    channel;                /**< Stream out data channel*/
} hal_audio_copy_stream_t;

/**
  * @}
  */

/** @defgroup hal_audio_typedef Typedef
  * @{
  */

/** @brief  This defines the audio stream out callback function prototype.
 *          Register a callback function while audio stream out path is turned on.
 *          For more details about the callback, please refer to #hal_audio_register_stream_out_callback().
 *  @param[in] event is the value defined in #hal_audio_event_t. This parameter is given by the driver to notify the user about the data flow processing behavior.
 *  @param[in] user_data is a user defined parameter provided by #hal_audio_register_stream_out_callback() function.
 */
typedef void (*hal_audio_stream_out_callback_t)(hal_audio_event_t event, void *user_data);

/** @brief  This defines the audio stream copy callback function prototype.
 *          Register a callback function while audio stream copy path is turned on.
 *          For more details about the callback, please refer to #hal_audio_register_copied_stream_out_callback().
 *  @param[in] event is the value defined in #hal_audio_event_t. This parameter is given by the driver to notify the user about the data flow processing behavior.
 *  @param[in] user_data is a structure #hal_audio_copy_stream_t contain stream out data and related information.
 */
typedef void (*hal_audio_stream_copy_callback_t)(hal_audio_event_t event, hal_audio_copy_stream_t *user_data);

/**
  * @}
  */

/**
 * @brief     Initializes basic settings of the audio hardware
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @note      This function should at least be called once after system restarts and before using any audio functionality.
 * @sa        #hal_audio_deinit()
 */
hal_audio_status_t hal_audio_init(void);

/**
 * @brief     Deinitialize audio settings
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @sa        #hal_audio_init()
 */
hal_audio_status_t hal_audio_deinit(void);

/**
 * @brief     Register the callback function to copy stream out data
 * @param[in] callback is the function pointer of callback for output data control
 * @param[in] user_data is a user defined extended parameter.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 */
hal_audio_status_t hal_audio_register_copied_stream_out_callback(hal_audio_stream_copy_callback_t callback, void *user_data);

/**
 * @brief     Set the sampling rate of the audio output stream.
 * @param[in] sample_rate is to set the output sampling rate. For more details, please refer to #hal_audio_sampling_rate_t.
 * @return    #HAL_AUDIO_STATUS_OK, if sampling rate is valid.
 * @note      If the audio function is for speech, such as HAL_AUDIO_PLAYBACK_VOICE/ HAL_AUDIO_RECORD_VOICE/ HAL_AUDIO_BLUETOOTH_CALL,
 *            only 8kHz or 16kHz is available.
 */
hal_audio_status_t hal_audio_set_stream_out_sampling_rate(hal_audio_sampling_rate_t sample_rate);

/**
 * @brief     Set the audio output channel number.
 * @param[in] channel_number is to set the output channel number.
 * @return    #HAL_AUDIO_STATUS_OK, if channel number is valid.
 */
hal_audio_status_t hal_audio_set_stream_out_channel_number(hal_audio_channel_number_t channel_number);

/**
 * @brief     Set the audio output channel mode.
 * @param[in] channel_mode is to set the output channel mode.
 * @return    #HAL_AUDIO_STATUS_OK, if channel number is valid.
 */
hal_audio_status_t hal_audio_set_stream_out_channel_mode(hal_audio_channel_number_t channel_mode);


/**
 * @brief     Power on the audio out.
 * @param[in] active_type is to set the audio output function type. For more details, please refer to #hal_audio_active_type_t.
 * @return    #HAL_AUDIO_STATUS_OK, if the output path type is valid
 * @sa        #hal_audio_stop_stream_out()
 */
hal_audio_status_t hal_audio_start_stream_out(hal_audio_active_type_t active_type);

/**
 * @brief     Power off the stream out path
 * @sa        #hal_audio_start_stream_out()
 */
void hal_audio_stop_stream_out(void);

/**
 * @brief     Set audio output the volume.
 * @param[in] hw_gain_index is to set HW GAIN 1/2/3, can't set HW GAIN ALL!
 * @param[in] digital_volume_index is to set digital gain in centi-db in hex (FFFFFE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the gain map.
 * @param[in] analog_volume_index is to set digital gain in centi-db in hex (FFFFFE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the hardware design and gain map.
 * @return    #HAL_AUDIO_STATUS_OK, if output volume range is valid.
 */
hal_audio_status_t hal_audio_set_stream_out_volume(hal_audio_hw_stream_out_index_t hw_gain_index, uint32_t digital_volume_index, uint32_t analog_volume_index);

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
/**
 * @brief     Mute the audio output stream
 * @param[in] mute is a flag to set the audio on/off silent.
 * @param[in] hw_gain_index is a flag to indicate specific stream out device.
 * @par       example
 * @code      hal_audio_mute_stream_out(true, HAL_AUDIO_STREAM_OUT1);
 * @endcode
 */
void hal_audio_mute_stream_out(bool mute, hal_audio_hw_stream_out_index_t hw_gain_index);

 /**
 * @brief      Get device supported frequency for the audio output device.
 * @param[in]  audio_out_device is the audio output device index you want to check. For more details, please refer to hal_audio_device_t.
 * @param[in]  freq is the sampling rate you want to use. For more details, please refer to #hal_audio_sampling_rate_t.
 * @return     Supported frequency. (It should be the same as the frequency you request. If the frequency is not supported, the supported frequency is returned.)
 */
int32_t hal_audio_get_device_out_supported_frequency(hal_audio_device_t audio_out_device,hal_audio_sampling_rate_t freq);


#else
/**
 * @brief     Mute the audio output stream
 * @param[in] mute is a flag to set the audio on/off silent.
 * @par       example
 * @code      hal_audio_mute_stream_out(true);
 * @endcode
 */
void hal_audio_mute_stream_out(bool mute);
#endif

#if defined(HAL_AUDIO_SUPPORT_DEBUG_DUMP)
/**
 * @brief     Dump audio register for debug
 * @par       Example
 * @code      hal_audio_debug_dump();
 * @endcode
 */
void hal_audio_debug_dump(void);
#endif

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
/**
 * @brief     Query shared memory address for each audio scenario and reserve query type.
 * @param[in] type is an enum of audio scenario.
 * @return    Shared memory address.
 */
uint32_t *hal_audio_query_share_info(hal_audio_message_type_t type);
#endif

/**
 * @brief     Set the audio output device
 * @param[in] device is the output component. For more details, please refer to hal_audio_device_t.
 * @return    #HAL_AUDIO_STATUS_OK, if output device is valid.
 */
hal_audio_status_t hal_audio_set_stream_out_device(hal_audio_device_t device);

/**
 * @brief     Write data to audio output
 * @param[in] buffer is the pointer to output data.
 * @param[in] size is the available free space of the output buffer (in bytes).
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @sa        #hal_audio_get_stream_out_sample_count()
 */
hal_audio_status_t hal_audio_write_stream_out(const void *buffer, uint32_t size);

/**
 * @brief      Query available free space for the output.
 * @param[out] sample_count is the available output sample count (in bytes).
 * @return     #HAL_AUDIO_STATUS_OK, if OK.
 * @note       Call this function before #hal_audio_write_stream_out() to make sure there is enough free space to write the output.
 * @sa         #hal_audio_write_stream_out()
 */
hal_audio_status_t hal_audio_get_stream_out_sample_count(uint32_t *sample_count);

/**
 * @brief     Register the callback function for output data
 * @param[in] callback is the function pointer of callback for output data control
 * @param[in] user_data is a user defined extended parameter.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 */
hal_audio_status_t hal_audio_register_stream_out_callback(hal_audio_stream_out_callback_t callback, void *user_data);

/**
 * @brief     Set audio input stream sampling rate
 * @param[in] sample_rate is to set input sampling rate. Please refers to #hal_audio_sampling_rate_t.
 * @return    #HAL_AUDIO_STATUS_OK, if sampling rate is valid
 * @note      Only 8kHz and 16kHz is supported.
 * @par       Example
 * @code      hal_audio_set_stream_in_sampling_rate(HAL_AUDIO_SAMPLING_RATE_8KHZ);
 * @endcode
 */
hal_audio_status_t hal_audio_set_stream_in_sampling_rate(hal_audio_sampling_rate_t sample_rate);

/**
 * @brief     Set audio input channel number
 * @param[in] channel_number is to set input channel number.
 * @return    #HAL_AUDIO_STATUS_OK, if channel number is valid.
 * @par       Example
 * @code      hal_audio_set_stream_in_channel_number(HAL_AUDIO_MONO);
 * @endcode
 */
hal_audio_status_t hal_audio_set_stream_in_channel_number(hal_audio_channel_number_t channel_number);

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
/**
  * @brief     Set audio input volume for multiple microphones.
  * @param[in] volume_index0 is to set a input gain in centi-db in hex (FE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the hardware design and gain map.
  * @param[in] volume_index1 is to set a input gain in centi-db in hex (FE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the hardware design and gain map.
  * @param[in] gain_select is to select which pair of gain to be setting.
  * @return    #HAL_AUDIO_STATUS_OK, if input volume range is valid.
  */
hal_audio_status_t hal_audio_set_stream_in_volume_for_multiple_microphone(uint32_t volume_index0, uint32_t volume_index1, hal_audio_input_gain_select_t gain_select);
#endif

/**
 * @brief     Set audio input volume
 * @param[in] digital_volume_index is to set a digital gain in centi-db in hex (FFFFFE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the gain map.
 * @param[in] analog_volume_index is to set digital gain in centi-db in hex (FFFFFE0C equal to -5db) and set #HAL_AUDIO_GAIN_KEEP to keep value. Gain range: depends on the hardware design and gain map.
 * @return    #HAL_AUDIO_STATUS_OK, if input volume range is valid.
 */
hal_audio_status_t hal_audio_set_stream_in_volume(uint32_t digital_volume_index, uint32_t analog_volume_index);

/**
 * @brief     Mute the audio input stream
 * @param[in] mute is a flag to set the audio on/off silent.
 * @par       Example
 * @code      hal_audio_mute_stream_in(true);
 * @endcode
 */
void hal_audio_mute_stream_in(bool mute);

/**
 * @brief     Mute the audio input stream
 * @param[in] type is a flag to set the type of stream in.
 * @param[in] mute is a flag to set the audio on/off silent.
 * @par       Example
 * @code      hal_audio_mute_stream_in(HFP, true);
 * @endcode
 */
void hal_audio_mute_stream_in_by_scenario(hal_audio_stream_in_scenario_t type, bool mute);

/**
 * @brief     Set audio input device
 * @param[in] device is the input component. For more details, please refer to hal_audio_device_t.
 * @return    #HAL_AUDIO_STATUS_OK, if input device is valid.
 * @par       Example
 * @code      hal_audio_set_stream_in_device(HAL_AUDIO_DEVICE_MAIN_MIC);
 * @endcode
 */
hal_audio_status_t hal_audio_set_stream_in_device(hal_audio_device_t device);

/**
 * @brief      Query the size of needed memory to be allocated for internal use in audio driver
 * @param[out] memory_size is the amount of memory required by the audio driver for an internal use (in bytes).
 * @return     #HAL_AUDIO_STATUS_OK, if OK.
 * @note       The function should be called at least before #hal_audio_start_stream_out() to ensure enough memory can be used internally.
 * @sa         #hal_audio_set_memory()
 */
hal_audio_status_t hal_audio_get_memory_size(uint32_t *memory_size);

/**
 * @brief      Hand over allocated memory to audio driver
 * @param[in]  memory is a pointer to an allocated memory. It should be 4 bytes aligned.
 * @return     #HAL_AUDIO_STATUS_OK, if OK.
 * @note       The function should be called after user allocates enough memory that is at least greater than the size queried from #hal_audio_get_memory_size().
 * @sa         #hal_audio_get_memory_size()
 */
hal_audio_status_t hal_audio_set_memory(void *memory);

/**
 * @brief      Get audio clock.
 * @param[out] sample_count is a pointer to the accumulated audio sample count.
 * @return     #HAL_AUDIO_STATUS_OK, if OK.
 * @note       The function is called to get the accumulated audio sample count written to audio hardware. One pair of L / R counts as one audio sample.
 */
hal_audio_status_t hal_audio_get_audio_clock(uint32_t *sample_count);

#if defined(HAL_AUDIO_SUPPORT_APLL)
/**
 * @brief     Enable/Disable APLL.
 * @param[in] enable is a flag to Enable/Disable APLL.
 * @param[in] samplerate is an index to determine which APLL to Enable/Disable.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @par       Example
 * @code      hal_audio_apll_enable(true, 48000);
 * @endcode
 */
hal_audio_status_t hal_audio_apll_enable(bool enable, uint32_t samplerate);

/**
 * @brief     Query apll status.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @par       Example
 * @code      hal_audio_query_apll_status(void);
 * @endcode
 */
hal_audio_status_t hal_audio_query_apll_status(void);

/**
 * @brief     Enable/Disable MCLK.
 * @param[in] enable is a flag to Enable/Disable MCLK.
 * @param[in] mclkoutpin is a index to determine which pin to output mclk.
 * @param[in] apll is a index to determine mclk clock source from which APLL.
 * @param[in] divider is a index to determine mclk out frequency, MCLK = clock_source/(1+divider), divider = [6:0].
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @par       Example
 * @code      hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S2, AFE_APLL2, 3);
 * @endcode
 */
hal_audio_status_t hal_audio_mclk_enable(bool enable, afe_mclk_out_pin_t mclkoutpin, afe_apll_source_t apll, uint8_t divider);

/**
 * @brief     Query mclk status.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @par       Example
 * @code      hal_audio_query_mclk_status(void);
 * @endcode
 */
hal_audio_status_t hal_audio_query_mclk_status(void);
#endif

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/
#endif /*HAL_AUDIO_MODULE_ENABLED*/
#endif /*__HAL_AUDIO_H__*/
