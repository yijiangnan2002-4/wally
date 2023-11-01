/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

/**
 * File: app_usb_audio_utils.h
 *
 * Description: This file defines the common structure and functions of usb audio app.
 */


#ifndef __APP_EVENTS_USB_EVENT_H__
#define __APP_EVENTS_USB_EVENT_H__

#ifdef AIR_USB_ENABLE
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t app_usb_audio_port_t;
#define APP_USB_AUDIO_UNKNOWN_PORT                      0x00     /**< unknown port. */
#define APP_USB_AUDIO_SPK_PORT                          0x01     /**< usb speaker. */
#define APP_USB_AUDIO_MIC_PORT                          0x02     /**< streaming microphone. */
#define APP_USB_AUDIO_PORT_MAX                          0x03     /**< The size of the port. */

typedef uint8_t app_usb_audio_sample_rate_t;
#define APP_USB_AUDIO_SAMPLE_RATE_UNKNOWN                0x00     /**< Unknown. */
#define APP_USB_AUDIO_SAMPLE_RATE_16K                    0x01     /**< 16KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_32K                    0x02     /**< 32KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_44_1K                  0x03     /**< 44.1KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_48K                    0x04     /**< 48KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_96K                    0x05     /**< 96KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_192K                   0x06     /**< 192KHz. */
#define APP_USB_AUDIO_SAMPLE_RATE_24K                    0x07     /**< 24KHz. */

/**
 *  @brief This enumeration defines the action of USB audio.
 */
typedef enum {
    APPS_EVENTS_USB_AUDIO_UNPLUG,        /**<  Notify app usb device unplug. Parameter is NULL */
    APPS_EVENTS_USB_AUDIO_PLAY,          /**<  Notify app to play USB audio. Parameter please ref#app_events_usb_port_t */
    APPS_EVENTS_USB_AUDIO_STOP,          /**<  Notify app to stop USB audio. Parameter please ref#app_events_usb_port_t */
    APPS_EVENTS_USB_AUDIO_VOLUME,        /**<  Notify app to set the volume of USB audio. Parameter please ref#app_events_usb_volume_t */
    APPS_EVENTS_USB_AUDIO_MUTE,          /**<  Notify app to mute the USB audio. Parameter please ref#app_events_usb_mute_t */
    APPS_EVENTS_USB_AUDIO_SAMPLE_RATE,   /**<  Notify app the sample rate of the USB audio. Parameter please ref#app_events_usb_sample_rate_t */
    APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE,   /**<  Notify app the sample size of the USB audio. */
    APPS_EVENTS_USB_AUDIO_CHANNEL,             /**<  Notify app the channel of the USB audio. */
    APPS_EVENTS_USB_AUDIO_SUSPEND,       /**<  Notify app the USB is suspend. */
    APPS_EVENTS_USB_AUDIO_RESUME,        /**<  Notify app the USB is resume. */
    APPS_EVENTS_USB_AUDIO_RESET,         /**<  Notify app the USB dirver reset. Parameter is NULL */
    APPS_EVENTS_USB_CONFIG_CHECKER,      /**<  Notify app to check the USB configured or not */
    APPS_EVENTS_USB_CONFIG_DONE,         /**<  Notify app the USB configuration done */
    APPS_EVENTS_USB_ENABLE_CONTROL_DETECT_TIMEOUT,  /**<  Control the enable/disable flow, send the event with delay, before the timeout if the usb enable/disable too many times.  */
    APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT,  /**<  Control the enable/disable flow, send the event with delay, before the timeout if the usb enable/disable callback, should not exist the force stop status.  */
} app_usb_audio_event_t;


/**
 *  @brief This enumeration defines the action of USB HID DEVICE CONTROL.
 */
typedef enum {
    APPS_EVENT_USB_HID_LED_CONTROL,
}app_usb_hid_device_control_event_t;
/**
 *  @brief This enumeration defines the interface ID in APP layer.
 */
enum {
#if defined(AIR_USB_AUDIO_ENABLE)
    APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT,
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
    APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING,
#endif
#endif
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
    APPS_USB_EVENTS_INTERFACE_MIC,
#endif
    APPS_USB_EVENTS_INTERFACE_MAX
};

typedef struct {
    app_usb_audio_port_t port_type;         /* spk/mic */
    uint8_t port_num;                       /* range:0 ~ 1; 0:chat,1:gaming */
} __attribute__((packed)) app_events_usb_port_t;

typedef struct {
    app_usb_audio_port_t port_type;         /* spk/mic */
    uint8_t port_num;                       /* range:0 ~ 1; 0:chat,1:gaming */
    uint8_t left_volume;                    /* valid range: 0 ~ 100, 0xFF means invalid */
    uint8_t right_volume;                   /* valid range: 0 ~ 100, 0xFF means invalid */
    int32_t left_db;                        /* the left channel volume with db */
    int32_t right_db;                       /* the right channel volume with db */
} app_events_usb_volume_t;

typedef struct {
    app_usb_audio_port_t port_type;         /* spk/mic */
    uint8_t port_num;                       /* range:0 ~ 1; 0:chat,1:gaming */
    uint8_t is_mute;                        /* 0:unmute; 1:mute */
} app_events_usb_mute_t;

typedef struct {
    app_events_usb_port_t port;             /* usb port info */
    app_usb_audio_sample_rate_t rate;
} __attribute__((packed)) app_events_usb_sample_rate_t;

typedef struct {
    app_events_usb_port_t port;             /* usb port info */
    uint8_t size;
} __attribute__((packed)) app_events_usb_sample_size_t;

typedef struct {
    app_events_usb_port_t port;             /* usb port info */
    uint8_t channel;
} __attribute__((packed)) app_events_usb_channel_t;

typedef struct {
	uint8_t led_control;
} app_events_usb_hid_led_control_t;

/**
 * @brief      Initialize the usb event, register callback.
 */
void apps_event_usb_event_init(void);

uint32_t apps_event_usb_event_sample_rate_convert(app_usb_audio_sample_rate_t sample_rate);

uint8_t apps_event_usb_get_interface_id_from_port_info(const app_events_usb_port_t *p_port);

uint8_t apps_event_usb_get_speaker_enable_info(uint8_t interface_id, uint8_t *sample_size, uint8_t *channel);

void apps_event_usb_enable_control_event(app_usb_audio_event_t event, uint8_t interface_id);

uint32_t apps_event_usb_get_speaker_sameple_rate(uint8_t interface_id, bool set_msg_pending);

#endif /* AIR_USB_ENABLE */

#endif /* __APP_USB_AUDIO_UTILS_H__ */
