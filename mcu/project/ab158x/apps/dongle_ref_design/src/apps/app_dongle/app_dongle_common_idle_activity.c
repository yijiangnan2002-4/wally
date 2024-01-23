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


/**
 * File: app_dongle_common_idle_activity.c
 *
 * Description: This file could receive common dongle events and notify BT state change.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for dongle APP.
 *
 */

#include "apps_events_event_group.h"
#include "app_dongle_common_idle_activity.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_service.h"
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "app_ull_dongle_le.h"
#endif
#include "apps_debug.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst_utillity.h"
#endif
#include "bt_gap_le.h"
#endif
#if (defined MTK_RACE_CMD_ENABLE)
#include "app_dongle_race.h"
#endif
#include "apps_config_event_list.h"
#include "ui_shell_manager.h"
#include "apps_events_interaction_event.h"
#include "app_dongle_connection_common.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_customer_config.h"
#include "usbaudio_drv.h"
#include "usbhid_drv.h"
#ifdef AIR_MS_GIP_ENABLE
#include "usb_main.h"
#endif
#ifdef AIR_USB_MFI_ENABLE
#include "usb_mfi.h"
#endif
#include "app_dongle_le_race.h"

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "app_dongle_ull_le_hid.h"
#include "apps_usb_utils.h"
#include "app_key_remap.h"
#endif

#define LOG_TAG     "[app_dongle_common]"
static uint8_t s_dongle_mode;


#if 0 // dongle_connection_common
typedef enum {
    APP_DONGLE_COMMON_PROFILE_MODE_NONE = 0,
    APP_DONGLE_COMMON_PROFILE_MODE_LE_AUDIO,
    APP_DONGLE_COMMON_PROFILE_MODE_EDR_ULL,
    APP_DONGLE_COMMON_PROFILE_MODE_BLE_ULL,
} app_dongle_comomn_profile_mode_t;

#if (defined(AIR_BLE_AUDIO_DONGLE_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)

static uint8_t s_dongle_profile_mode = APP_DONGLE_COMMON_PROFILE_MODE_NONE;



static void app_dongle_common_profile_mode_changed(app_dongle_comomn_profile_mode_t new_mode)
{
    if (s_dongle_profile_mode != new_mode) {
        APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, %d->%d", 2, s_dongle_profile_mode, new_mode);
#if (defined(AIR_BLE_AUDIO_DONGLE_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE))
        if (APP_DONGLE_COMMON_PROFILE_MODE_NONE == s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_LE_AUDIO != new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, stop BLE", 0);
        } else if (APP_DONGLE_COMMON_PROFILE_MODE_LE_AUDIO != s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_NONE == new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, start BLE", 0);
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        if (APP_DONGLE_COMMON_PROFILE_MODE_NONE == s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_BLE_ULL != new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, stop ULL2", 0);
            app_ull_dongle_le_stop_scan();
        } else if (APP_DONGLE_COMMON_PROFILE_MODE_BLE_ULL != s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_NONE == new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, start ULL2", 0);
            app_ull_dongle_le_start_scan();
        }
#endif
        if (APP_DONGLE_COMMON_PROFILE_MODE_NONE == s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_EDR_ULL != new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, stop ULL", 0);
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
        } else if (APP_DONGLE_COMMON_PROFILE_MODE_EDR_ULL != s_dongle_profile_mode && APP_DONGLE_COMMON_PROFILE_MODE_NONE == new_mode) {
            APPS_LOG_MSGID_I(LOG_TAG"profile_mode_changed, start ULL", 0);
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
#endif
        }
        s_dongle_profile_mode = new_mode;
    }
}
#endif
#endif

#if defined(AIR_LE_AUDIO_ENABLE)
static const int32_t apps_usb_ent_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
static const int32_t apps_usb_ent_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#if defined(AIR_WIRELESS_MIC_ENABLE)
static const int32_t apps_usb_wmrx_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static const int32_t apps_usb_game_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_game_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
static const int32_t apps_usb_game_ull2_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
static const int32_t apps_usb_8ch_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] __unused = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_8ch_sample_rate_alt3[USB_AUDIO_DSCR_MAX_FREQ_NUM] __unused = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_8ch_mic_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] __unused = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
static const int32_t apps_usb_dchs_custom_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] __unused = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_dchs_custom_mic_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] __unused = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

#ifdef AIR_BT_SOURCE_ENABLE
static const int32_t apps_usb_bt_source_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_bt_source_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};
#endif

static void app_home_usb_pulg_in_cb(usb_evt_t event, void* usb_data, void* user_data)
{
    switch(s_dongle_mode) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_GAMING: {
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_GAME_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_GAME_VID, APPS_USB_GAME_PID, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle Gaming");
            break;
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case APPS_USB_MODE_GAMING_ULL2: {
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_GAME_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_GAME_VID, 0x0820, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle Gaming ULL1");
            break;
        }
#endif
#if (defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE)
        case APPS_USB_MODE_XBOX: {
            usb_set_device_type(APPS_USB_XBOX_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_XBOX_HS_ENABLE);
            /* Xbox don't have product info */
            break;
        }
#endif /* (defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE) */
#if defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_ENTERPRISE: {
            usb_set_device_type(APPS_USB_ENT_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_ENT_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_ENT_VID, APPS_USB_ENT_PID, APPS_USB_ENT_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle Enterprise");
            break;
        }
#endif
#if defined(AIR_WIRELESS_MIC_ENABLE)
        case APPS_USB_MODE_WIRELESS_MIC_RX: {
            usb_set_device_type(APPS_USB_WMRX_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_WMRX_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_WMRX_VID, APPS_USB_WMRX_PID, APPS_USB_WMRX_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle Wireless Mic RX");
            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_8CH: {
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_GAME_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_GAME_VID, 0x080E, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle 8CH");
            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_DCHS_CUSTOM: {
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(APPS_USB_GAME_VID, 0x0812, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle DCHS Custom");
            break;
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_AUDIO_CDC: {
            usb_set_device_type(USB_AUDIO_CDC);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0816, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle Audio CDC");
            break;
        }
#endif
#ifdef AIR_BT_SOURCE_ENABLE
        case APPS_USB_MODE_BT_SOURCE: {
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0818, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle BT Source");
            break;
        }
        case APPS_USB_MODE_BT_LEA: {
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0824, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle BT LEA");
            break;
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case APPS_USB_MODE_BT_ULL2: {
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0828, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle BT ULL2");
            break;
        }
#endif
#endif
        case APPS_USB_MODE_CDC: {
            usb_set_device_type(USB_CDC_ACM);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0826, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle CDC");
            break;
        }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case APPS_USB_MODE_3IN1:{
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0700, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Dongle 3in1");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
#endif
#if defined(AIR_PURE_GAMING_ENABLE)
        case APPS_USB_MODE_GAMING_MSKB:{
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0703, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Gaming MS/KB Dongle");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
        case APPS_USB_MODE_GAMING_MS:{
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0704, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Gaming MS Dongle");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
        case APPS_USB_MODE_GAMING_KB:{
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0705, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Gaming Nkey KB Dongle");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
#endif
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        case APPS_USB_MODE_GAMING_NVMS: {
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0706, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Gaming NV MS Dongle");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
#endif
#if defined(AIR_HID_BT_HOGP_ENABLE)
        case APPS_USB_MODE_OFFICE_MSKB:{
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0705, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Office KB/MS Dongle");
            usb_custom_set_power_info(false, true, 100);
            break;
        }
#endif
        default:
            break;
    }
}

static void app_home_usb_pulg_out_cb(usb_evt_t event, void* usb_data, void* user_data)
{

}


static void app_home_usb_clear_tx_rx()
{
    APPS_LOG_MSGID_I(LOG_TAG"app_home_usb_clear_tx_rx start clear", 0);
    USB_Aduio_Set_RX1_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_RX1_Alt3(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_RX2_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_TX1_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    APPS_LOG_MSGID_I(LOG_TAG"app_home_usb_clear_tx_rx end clear", 0);
}

static void app_home_usb_device_init_cb(usb_evt_t event, void* usb_data, void* user_data)
{
    APPS_LOG_MSGID_I(LOG_TAG"app_home_usb_device_init_cb mode:%d", 1, s_dongle_mode);

    /* clear RX/TX alt2, alt3 setting */
    app_home_usb_clear_tx_rx();

    switch(s_dongle_mode) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_GAMING: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, true, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_spk_channels(USB_AUDIO_2_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX2_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX2_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_game_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[3]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);
            break;
        }
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case APPS_USB_MODE_GAMING_ULL2: {
                    usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
                    usb_audio_set_audio_card(USB_AUDIO_2_PORT, true, false);
                    usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
                    usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_NULL);
                    usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
                    usb_audio_set_spk_channels(USB_AUDIO_2_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
                    usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

                    USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                           USB_AUDIO_SAMPLE_SIZE_16BIT,
                                           USB_AUDIO_CHANNEL_2,
                                           (uint32_t *)apps_usb_game_sample_rate);

                    USB_Aduio_Set_RX1_Alt2(true,
                                           USB_AUDIO_SAMPLE_RATE_NUM_2,
                                           USB_AUDIO_SAMPLE_SIZE_24BIT,
                                           USB_AUDIO_CHANNEL_2,
                                           (uint32_t *)apps_usb_game_sample_rate);

                    USB_Aduio_Set_RX2_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                           USB_AUDIO_SAMPLE_SIZE_16BIT,
                                           USB_AUDIO_CHANNEL_2,
                                           (uint32_t *)apps_usb_game_sample_rate);

                    USB_Aduio_Set_RX2_Alt2(true,
                                           USB_AUDIO_SAMPLE_RATE_NUM_2,
                                           USB_AUDIO_SAMPLE_SIZE_24BIT,
                                           USB_AUDIO_CHANNEL_2,
                                           (uint32_t *)apps_usb_game_sample_rate);

                    USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_3,
                                           USB_AUDIO_SAMPLE_SIZE_16BIT,
                                           USB_AUDIO_CHANNEL_1,
                                           (uint32_t *)apps_usb_game_ull2_mic_sample_rate);

                    usb_hid_set_dscr_enable(
                        (usb_hid_report_dscr_type_t[3]){
                            USB_REPORT_DSCR_TYPE_MUX,
                            USB_REPORT_DSCR_TYPE_AC,
                            USB_REPORT_DSCR_TYPE_TELEPHONY,},
                        3);
                    break;
                }
#endif
#if (defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE)
        case APPS_USB_MODE_XBOX: {
            break;
        }
#endif /* (defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE) */
#if defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_ENTERPRISE: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_HEADSET, USB_AUDIO_TERMT_HEADSET);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_ent_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_ent_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[4]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TEAMS,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                4);
            break;
        }
#endif
#if defined(AIR_WIRELESS_MIC_ENABLE)
        case APPS_USB_MODE_WIRELESS_MIC_RX: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, false, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_wmrx_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[2]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,},
                2);

            #if defined(AIR_USB_MFI_ENABLE)
            usb_mfi_set_dscr_enable(true);
            #endif

            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_8CH: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_8, USB_AUDIO_CHANNEL_CONBINE_8CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt3(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_8,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt3);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_8ch_mic_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt2(true,
                                    USB_AUDIO_SAMPLE_RATE_NUM_2,
                                    USB_AUDIO_SAMPLE_SIZE_24BIT,
                                    USB_AUDIO_CHANNEL_1,
                                    (uint32_t *)apps_usb_8ch_mic_sample_rate_alt12);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[3]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);
            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_DCHS_CUSTOM: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_dchs_custom_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_dchs_custom_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_dchs_custom_mic_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt2(true,
                                    USB_AUDIO_SAMPLE_RATE_NUM_2,
                                    USB_AUDIO_SAMPLE_SIZE_24BIT,
                                    USB_AUDIO_CHANNEL_1,
                                    (uint32_t *)apps_usb_dchs_custom_mic_sample_rate_alt12);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[3]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);
            break;
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_AUDIO_CDC: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_game_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[3]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);
            break;
        }
#endif
#ifdef AIR_BT_SOURCE_ENABLE
        case APPS_USB_MODE_BT_SOURCE:{
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_HEADSET, USB_AUDIO_TERMT_HEADSET);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_4,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_bt_source_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_4,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_bt_source_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_bt_source_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[4]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TEAMS,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                4);
            break;
        }
        case APPS_USB_MODE_BT_LEA: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_HEADSET, USB_AUDIO_TERMT_HEADSET);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_4,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_bt_source_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_4,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_bt_source_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_bt_source_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[4]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TEAMS,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                4);
            break;
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case APPS_USB_MODE_BT_ULL2: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_bt_source_mic_sample_rate);

            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[3]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);
            break;
        }
#endif

#endif
        case APPS_USB_MODE_CDC: {
            break;
        }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case APPS_USB_MODE_3IN1: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, true, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_game_mic_sample_rate);

            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[3]) {
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,
                    USB_REPORT_DSCR_TYPE_TELEPHONY,},
                3);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_GMOUSE},
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_GKEYBOARD},
                1);
            break;
        }
#endif
#if defined(AIR_PURE_GAMING_ENABLE)
        case APPS_USB_MODE_GAMING_MSKB: {
            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_out_ep_enable(USB_HID_DEV_PORT_0, true);
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[2]) {
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_EPIO,
                    },
                2);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {
                    USB_REPORT_DSCR_TYPE_GMOUSE},
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_GKEYBOARD},
                1);
            break;
        }
        case APPS_USB_MODE_GAMING_MS: {
            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_out_ep_enable(USB_HID_DEV_PORT_0, true);
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[2]) {
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_EPIO,
                    },
                2);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {
                    USB_REPORT_DSCR_TYPE_GMOUSE},
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_GKEYBOARD},
                1);
            break;
        }
        case APPS_USB_MODE_GAMING_KB: {
            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_out_ep_enable(USB_HID_DEV_PORT_0, true);
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[2]) {
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_EPIO,
                    },
                2);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {
                    USB_REPORT_DSCR_TYPE_GMOUSE},
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_GKEYBOARD},
                1);
            break;
        }
#endif
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        case APPS_USB_MODE_GAMING_NVMS: {
            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_out_ep_enable(USB_HID_DEV_PORT_0, true);
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[2]) {
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_EPIO,
                    },
                2);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {
                    USB_REPORT_DSCR_TYPE_GMOUSE_NV,
                    },
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_125US);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {
                    USB_REPORT_DSCR_TYPE_GKEYBOARD,
                    },
                1);
            break;
        }
#endif
#if defined(AIR_HID_BT_HOGP_ENABLE)
        case APPS_USB_MODE_OFFICE_MSKB: {
            usb_hid_device_enable(USB_HID_DEV_PORT_0_MASK | USB_HID_DEV_PORT_1_MASK | USB_HID_DEV_PORT_2_MASK);

            /* HID Device 0 Race/Mux/AudioControl */
            usb_hid_set_interval(USB_HID_DEV_PORT_0, USB_HID_EP_INTERVAL_1MS);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_0, USB_HID_PROTOCOL_CODE_NONE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_0,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_MUX},
                1);

            /* HID Device 1 Mouse */
            usb_hid_set_interval(USB_HID_DEV_PORT_1, USB_HID_EP_INTERVAL_1MS);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_1, USB_HID_PROTOCOL_CODE_MOUSE);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_1,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_OFFICE_MS},
                1);

            /* HID Device 2 Keyboard */
            usb_hid_set_interval(USB_HID_DEV_PORT_2, USB_HID_EP_INTERVAL_1MS);
            usb_hid_set_protocol_code(USB_HID_DEV_PORT_2, USB_HID_PROTOCOL_CODE_KEYBOARD);
            usb_hid_report_enable(
                USB_HID_DEV_PORT_2,
                (usb_hid_report_dscr_type_t[1]) {USB_REPORT_DSCR_TYPE_OFFICE_KB},
                1);
            break;
        }
#endif
        default:
            break;
    }
}

uint8_t app_dongle_common_idle_verify_mode(uint8_t mode) {
#if !(defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE))
    if (APPS_USB_MODE_GAMING == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE))
    if (APPS_USB_MODE_AUDIO_CDC == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !((defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE))
    if (APPS_USB_MODE_XBOX == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_LE_AUDIO_ENABLE))
    if (APPS_USB_MODE_ENTERPRISE == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_WIRELESS_MIC_ENABLE))
    if (APPS_USB_MODE_WIRELESS_MIC_RX == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE))
    if (APPS_USB_MODE_8CH == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE))
    if (APPS_USB_MODE_DCHS_CUSTOM == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !defined(AIR_BT_SOURCE_ENABLE)
    if (APPS_USB_MODE_BT_SOURCE == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
    if (APPS_USB_MODE_GAMING_ULL2 == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_BT_SOURCE_ENABLE) && defined(AIR_LE_AUDIO_ENABLE))
    if (APPS_USB_MODE_BT_LEA == mode) {
            mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_BT_SOURCE_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
    if (APPS_USB_MODE_BT_ULL2 == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE))
    if (APPS_USB_MODE_3IN1 == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_PURE_GAMING_ENABLE))
    if (APPS_USB_MODE_GAMING_MSKB == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_NVIDIA_REFLEX_ENABLE))
    if (APPS_USB_MODE_GAMING_NVMS == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
#if !(defined(AIR_HID_BT_HOGP_ENABLE))
    if (APPS_USB_MODE_OFFICE_MSKB == mode) {
        mode = APPS_DEFAULT_USB_MODE;
    }
#endif
    return mode;
}

/******** UI shell proc events functions ***********/
static bool app_dongle_common_idle_system_event_proc(ui_shell_activity_t *self,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    s_dongle_mode = app_dongle_common_idle_verify_mode(s_dongle_mode);
    if (EVENT_ID_SHELL_SYSTEM_ON_CREATE == event_id) {
        usb_evt_register_cb(USB_USER_APP, USB_EVT_PLUG_IN, app_home_usb_pulg_in_cb, NULL);
        usb_evt_register_cb(USB_USER_APP, USB_EVT_PLUG_OUT, app_home_usb_pulg_out_cb, NULL);
        usb_evt_register_cb(USB_USER_APP, USB_EVT_DEVICE_INIT, app_home_usb_device_init_cb, NULL);
#if defined(AIR_USB_ENABLE)
        usb_drv_enable();
#endif
#ifdef MTK_RACE_CMD_ENABLE
        app_dongle_le_race_init();
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        app_usb_utils_hid_bt_reg();
#if defined (AIR_PURE_GAMING_MS_ENABLE) || defined (AIR_PURE_GAMING_KB_ENABLE)
        app_dongle_ull_le_hid_ep_tx_reg();
#endif
#if defined(AIR_PURE_GAMING_MS_ENABLE)
        app_key_remap_init();
#endif
#if defined(AIR_PURE_GAMING_KB_ENABLE)
        app_usb_utils_hid_kb_intr_reg();
        app_usb_utils_hid_output_reg();
#endif
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        app_usb_utils_hid_preintr_reg();
        //app_usb_utils_hid_intr_reg();
#endif
#endif
    }

    return true;
}

static bool app_dongle_common_idle_interaction_event_proc(ui_shell_activity_t *self,
                                                          uint32_t event_id,
                                                          void *extra_data,
                                                          size_t data_len)
{
    bool ret = false;

    if (APPS_EVENTS_INTERACTION_SET_USB_MODE == event_id) {
        uint32_t mode = (uint32_t)extra_data;
        if ((mode < APPS_USB_MODE_BTA_MIN || mode >= APPS_USB_MODE_BTA_MAX) &&
            (mode < APPS_USB_MODE_BTD_MIN || mode >= APPS_USB_MODE_BTD_MAX)
        ) {
            mode = APPS_DEFAULT_USB_MODE;
        }
        if (mode != s_dongle_mode) {
            s_dongle_mode = app_dongle_common_idle_verify_mode(mode);
            nvkey_write_data(NVID_APP_DONGLE_USB_MODE, (uint8_t *) &s_dongle_mode, sizeof(s_dongle_mode));
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                                (void *)0, 0, NULL, 0);
        }
        ret = true;
    } else if (APPS_EVENTS_INTERACTION_LE_SCAN_END == event_id) {
#if defined(MTK_RACE_CMD_ENABLE)
        ret = app_dongle_le_race_interaction_event_proc(self, event_id, extra_data, data_len);
#endif
    }

    return ret;
}

static bool app_dongle_common_idle_bt_event_proc(ui_shell_activity_t *self,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len)
{
    bool ret = false;

    switch (event_id) {
#if 0 // dongle_connection_common
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
        case BT_GAP_LE_CONNECT_IND: {
            uint8_t ull2_link_num = app_le_audio_ucst_get_link_num();
            if (ull2_link_num) {
                APPS_LOG_MSGID_I(LOG_TAG"bt_event_proc, LE audio connected :0x%x, ull2_link_num = %d", 2, event_id, ull2_link_num);
                if (s_dongle_profile_mode == APP_DONGLE_COMMON_PROFILE_MODE_NONE) {
                    app_dongle_common_profile_mode_changed(APP_DONGLE_COMMON_PROFILE_MODE_LE_AUDIO);
                }
            }
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            uint8_t ull2_link_num = app_le_audio_ucst_get_link_num();
            APPS_LOG_MSGID_I(LOG_TAG"bt_event_proc, LE audio disconnected :0x%x, ull2_link_num = %d", 2, event_id, ull2_link_num);
            if (0 == ull2_link_num) {
                if (s_dongle_profile_mode == APP_DONGLE_COMMON_PROFILE_MODE_LE_AUDIO) {
                    app_dongle_common_profile_mode_changed(APP_DONGLE_COMMON_PROFILE_MODE_NONE);
                }
            }
            break;
        }
#endif
#endif
        default:
            break;
    }
#ifdef MTK_RACE_CMD_ENABLE
    ret = app_dongle_race_bt_event_proc(event_id, extra_data, data_len);
#endif
    return ret;
}

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
bool app_dongle_common_idle_ull_event_proc(ui_shell_activity_t *self,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    bool ret = false;

    switch (event_id) {
#if 0 // dongle_connection_common
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case BT_ULL_EVENT_LE_CONNECTED:
        case BT_ULL_EVENT_LE_DISCONNECTED: {
            uint8_t ull2_link_num = app_ull_dongle_le_get_link_num();
            APPS_LOG_MSGID_I(LOG_TAG"ull_event_proc, ULL LE connected :0x%x, ull2_link_num = %d", 2, event_id, ull2_link_num);
            if (ull2_link_num > 0 && BT_ULL_EVENT_LE_CONNECTED == event_id) {
                if (s_dongle_profile_mode == APP_DONGLE_COMMON_PROFILE_MODE_NONE) {
                    app_dongle_common_profile_mode_changed(APP_DONGLE_COMMON_PROFILE_MODE_BLE_ULL);
                }
            } else if (ull2_link_num == 0 && BT_ULL_EVENT_LE_DISCONNECTED == event_id) {
                if (s_dongle_profile_mode == APP_DONGLE_COMMON_PROFILE_MODE_BLE_ULL) {
                    app_dongle_common_profile_mode_changed(APP_DONGLE_COMMON_PROFILE_MODE_NONE);
                }
            }
            break;
        }
#endif
#endif
        default:
            break;
    }
    return ret;
}
#endif

bool app_dongle_common_idle_activity_proc(ui_shell_activity_t *self,
    uint32_t event_group,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_dongle_common_idle_system_event_proc(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            ret = app_dongle_common_idle_interaction_event_proc(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT: {
            ret = app_dongle_common_idle_bt_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            ret = app_dongle_common_idle_ull_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
#if (defined MTK_RACE_CMD_ENABLE)
        case EVENT_GROUP_UI_SHELL_APP_DONGLE_RACE:
            ret = app_dongle_race_cmd_event_proc(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_DONGLE_LE_RACE:
            ret = app_dongle_le_race_cmd_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }

    return ret;
}

app_usb_mode_t app_dongle_common_idle_activity_get_current_mode(void)
{
    return s_dongle_mode;
}

void app_dongle_common_idle_activity_init_mode(void)
{
    app_dongle_cm_link_mode_t mode = 0;
    uint32_t read_size = sizeof(s_dongle_mode);
#if defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE)
    if (USB_HOST_TYPE_XBOX == Get_USB_Host_Type()) {
        s_dongle_mode = APPS_USB_MODE_XBOX;
    } else
#endif
    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_DONGLE_USB_MODE, &s_dongle_mode, &read_size)) {
        s_dongle_mode = APPS_DEFAULT_USB_MODE;
    }
    s_dongle_mode = app_dongle_common_idle_verify_mode(s_dongle_mode);
    switch (s_dongle_mode) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case APPS_USB_MODE_3IN1:
        case APPS_USB_MODE_GAMING:
        case APPS_USB_MODE_AUDIO_CDC:
        {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
#if defined(AIR_LE_AUDIO_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_LEA;
#endif
            break;
        }
#endif
#if (defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)) && defined(AIR_MS_GIP_ENABLE) && defined(AIR_USB_XBOX_ENABLE)
        case APPS_USB_MODE_XBOX: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_LE_AUDIO_ENABLE)
        case APPS_USB_MODE_ENTERPRISE: {
            mode |= APP_DONGLE_CM_LINK_MODE_LEA;
            break;
        }
#endif
#if defined(AIR_WIRELESS_MIC_ENABLE)
        case APPS_USB_MODE_WIRELESS_MIC_RX: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_8CH: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_BT_SOURCE_ENABLE)
        case APPS_USB_MODE_BT_SOURCE: {
            mode |= APP_DONGLE_CM_LINK_MODE_BTA;
            break;
        }
#endif
#if defined(AIR_USB_AUDIO_MULTI_CH_MODE) || defined(AIR_DCHS_MODE_ENABLE)
        case APPS_USB_MODE_DCHS_CUSTOM: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case APPS_USB_MODE_GAMING_ULL2:
        {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BT_SOURCE_ENABLE)
        case APPS_USB_MODE_BT_LEA:
        {
            mode |= APP_DONGLE_CM_LINK_MODE_LEA;
            mode |= APP_DONGLE_CM_LINK_MODE_BTA;
            break;
        }
#endif
#if defined(AIR_BT_SOURCE_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case APPS_USB_MODE_BT_ULL2: {
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
            mode |= APP_DONGLE_CM_LINK_MODE_BTA;
            break;
        }
#endif
#if defined(AIR_PURE_GAMING_ENABLE)
        case APPS_USB_MODE_GAMING_MSKB:
        case APPS_USB_MODE_GAMING_MS:
        case APPS_USB_MODE_GAMING_KB: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        case APPS_USB_MODE_GAMING_NVMS: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
#if defined(AIR_HID_BT_HOGP_ENABLE)
        case APPS_USB_MODE_OFFICE_MSKB: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V1;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            mode |= APP_DONGLE_CM_LINK_MODE_ULL_V2;
#endif
            break;
        }
#endif
        default:
            break;
    }
    app_dongle_cm_set_link_mode(mode);
    APPS_LOG_MSGID_I(LOG_TAG"app_dongle_common_idle_activity_get_init_bt_link_mode s_dongle_mode = %d, BT link mode = 0x%x", 2, s_dongle_mode, mode);
}
