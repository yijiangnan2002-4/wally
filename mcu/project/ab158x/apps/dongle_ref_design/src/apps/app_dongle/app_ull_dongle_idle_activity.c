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


/**
 * File: app_ull_dongle_idle_activity.c
 *
 * Description: This file could receive ultra low latecy events and notify BT state change.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for ULL dongle APP.
 *
 */

#include "app_ull_dongle_idle_activity.h"
#include "app_preproc_activity.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#include "bt_ull_service.h"
#include "app_dongle_service.h"
#include "race_cmd_feature.h"
#endif
#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
#include "race_fota.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#include "bt_ull_le_utility.h"
#include "app_ull_dongle_le.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "bt_ull_le_hid_service.h"
#include "app_dongle_ull_le_hid.h"
#include "bt_ull_le_hid_utility.h"
#endif

#include "apps_events_event_group.h"
#include "apps_events_usb_event.h"
#include "apps_config_event_list.h"
#include "apps_events_mic_control_event.h"
#include "apps_dongle_sync_event.h"
#include "ui_shell_manager.h"
#include "app_dongle_race.h"

#include "apps_debug.h"

#include "apps_events_interaction_event.h"
#include "hal_eint.h"
#include "hal_gpio.h"

#include "nvkey.h"
#include "nvkey_id_list.h"

#ifdef AIR_MS_GIP_ENABLE
#include "app_ms_xbox_idle_activity.h"
#endif
#include "bt_connection_manager.h"
#include "usb_main.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "apps_config_led_index_list.h"
#include "apps_config_led_manager.h"
#include "bt_ull_le_call_service.h"
#include "usb_hid_srv.h"
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#include "scenario_wireless_mic_rx.h"
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#define LOG_TAG     "[app_dongle]"

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define APP_LINE_IN_MIX_RATIO_MIN_LEVEL       (0)
#define APP_LINE_IN_MIX_RATIO_BALANCED_LEVEL  (70)
#define APP_LINE_IN_MIX_RATIO_MAX_LEVEL       (100)
#define APP_LINE_IN_DEFAULT_VOLUME_VALUES     50  /**< Customer configuration*/
#define APP_LINE_OUT_DEFAULT_VOLUME_VALUES    90  /**< Customer configuration*/

typedef struct {
        bool      spk_gaming_play;                /**< Record if the speaker gaming is playing.*/
        bool      spk_chat_play;                  /**< Record if the speaker chat is playing.*/
        bool      usb_out_play;                   /**< Record if the usb out is playing.*/
#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
        bool      line_in;                        /**< Record if the line in.*/
        bool      line_out;                       /**< Record if the line out.*/
#endif
#ifdef AIR_DONGLE_I2S_SLV_OUT_ENABLE
        bool      i2s_out;                        /**< Record if the i2s out.*/
#endif
        bool      ull_connected;                  /**< Record if the ull was connected.*/
#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
        uint8_t   linein_volume_values;           /**< Record the current values of the line in streaming.*/
        uint8_t   lineout_volume_values;          /**< Record the current values of the line out streaming.*/
#endif
} app_ull_context;

static app_ull_context g_app_ull_dongle_context = {0};
#endif

uint8_t g_dongle_mode  =   APP_DONGLE_MODE_PC;

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#define APP_WIRELESS_MIC_TX_NUM_MAX (4)
#define APP_WIRELESS_MIC_MODE_MAX   (2)
#define APP_WIRELESS_MONO_MIC_MODE_MERGE        (5)
#define APP_WIRELESS_MONO_MIC_MODE_SPLIT_LEFT   (8)
#define APP_WIRELESS_MONO_MIC_MODE_SPLIT_RIGHT  (9)
#define APP_WIRELESS_STEREO_MIC_MODE_MERGE        (1)
#define APP_WIRELESS_STEREO_MIC_MODE_SPLIT_LEFT   (6)
#define APP_WIRELESS_STEREO_MIC_MODE_SPLIT_RIGHT  (7)

#define APP_WIRELESS_MIC_SAFETY_MODE_LEFT_GAIN      (600)   /* 600 means +6 DB */
#define APP_WIRELESS_MIC_SAFETY_MODE_RIGHT_GAIN     (0)

#define APP_WIRELESS_MIC_SAFETY_MODE_LC3PLUS_LEFT_GAIN      (0)
#define APP_WIRELESS_MIC_SAFETY_MODE_LC3PLUS_RIGHT_GAIN     (-600) /* -600 means -6 DB */

enum {
#if defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
    APP_WIRELESS_MIC_OUTPUT_TYPE_LINE_OUT = 0,
#endif
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
    APP_WIRELESS_MIC_OUTPUT_TYPE_I2S_OUT,
#endif
    APP_WIRELESS_MIC_OUTPUT_TYPE_USB_OUT,
    APP_WIRELESS_MIC_OUTPUT_TYPE_MAX
};

typedef struct {
    bt_bd_addr_t addr;
    uint8_t mode_id;
} app_wireless_mic_rx_merge_split_mode_record_t;

typedef struct {
    bt_bd_addr_t addr;
    uint8_t audio_info_data[0];
} app_wireless_mic_rx_merge_split_audio_data_t;

static void app_wireless_mic_rx_generate_audio_connection_info(uint8_t **audio_info,
                                                               uint32_t *audio_info_size,
                                                               app_wireless_mic_rx_merge_split_mode_record_t *record_data,
                                                               uint8_t record_data_num)
{
    uint32_t total_size = 0;
    uint32_t position = 0;

    *audio_info = NULL;
    *audio_info_size = 0;
    if (record_data != NULL && record_data_num != 0) {
        uint8_t *audio_info_list[record_data_num];
        uint32_t audio_info_size_list[record_data_num];
        uint8_t i;
        for (i = 0; i < record_data_num; i++) {
            wireless_mic_rx_audio_connection_info_get(record_data[i].mode_id, &audio_info_list[i], &audio_info_size_list[i]);
            if (audio_info_list[i] != NULL && audio_info_size_list[i] != 0) {
                total_size = total_size + sizeof(app_wireless_mic_rx_merge_split_audio_data_t) + audio_info_size_list[i];
            }
        }
        *audio_info = wireless_mic_rx_audio_connection_info_malloc(total_size);
        if (audio_info) {
            for (i = 0; i < record_data_num; i++) {
                if (audio_info_list[i] != NULL && audio_info_size_list[i] != 0) {
                    app_wireless_mic_rx_merge_split_audio_data_t *audio_data = (app_wireless_mic_rx_merge_split_audio_data_t *)(*audio_info + position);
                    memcpy(&audio_data->addr, &record_data[i].addr, sizeof(bt_bd_addr_t));
                    memcpy(&audio_data->audio_info_data, audio_info_list[i], audio_info_size_list[i]);
                    position = position + sizeof(app_wireless_mic_rx_merge_split_audio_data_t) + audio_info_size_list[i];
                }
            }
            *audio_info_size = total_size;
        }
    }
}

static nvkey_status_t app_wireless_mic_rx_read_merge_splite_nvkey(uint8_t **nvkey_record,
                                                                  uint32_t *nvkey_size,
                                                                  uint32_t append_size)
{
    nvkey_status_t ret;
    *nvkey_record = NULL;
    *nvkey_size = 0;
    ret = nvkey_data_item_length(NVID_APP_WM_MERGE_SPLIT_MODE, nvkey_size);
    if (*nvkey_size + append_size > 0) {
        *nvkey_record = (uint8_t *)malloc(*nvkey_size + append_size);
    }
    if (*nvkey_record && *nvkey_size > 0) {
        ret = nvkey_read_data(NVID_APP_WM_MERGE_SPLIT_MODE, *nvkey_record, nvkey_size);
    } else {
        *nvkey_size = 0;
        ret = NVKEY_STATUS_OK;
    }
    APPS_LOG_MSGID_I(LOG_TAG", read_merge_splite_nvkey status=0x%x", 1, NVKEY_STATUS_OK);
    return ret;
}

static bt_status_t app_wireless_mic_rx_query_audio_mode(app_wireless_mic_rx_merge_split_mode_record_t **record_mode,
                                                                   uint8_t *record_count,
                                                                   const bt_bd_addr_t *change_addr,
                                                                   uint8_t target_mode)
{
    nvkey_status_t nvkey_ret = NVKEY_STATUS_OK;
    bt_addr_t addr_list[APP_WIRELESS_MIC_TX_NUM_MAX];
    uint8_t tx_mic_amount = APP_WIRELESS_MIC_TX_NUM_MAX;
    uint8_t connected_record_id[APP_WIRELESS_MIC_TX_NUM_MAX];
    bt_status_t bt_status = app_ull_dongle_le_get_connected_device_list(addr_list, &tx_mic_amount);
    memset(connected_record_id, 0xFF, APP_WIRELESS_MIC_TX_NUM_MAX);
    *record_mode = NULL;
    *record_count = 0;
    if (BT_STATUS_SUCCESS == bt_status && tx_mic_amount > 0) {
        bt_ull_le_channel_mode_t mode = bt_ull_le_srv_get_channel_mode(BT_ULL_LINE_OUT_TRANSMITTER, true, BT_ULL_ROLE_SERVER);
        app_wireless_mic_rx_merge_split_mode_record_t *nvkey_record = NULL;
        uint32_t nvkey_size = 0;
        uint8_t nvkey_record_count = 0;
        nvkey_ret = app_wireless_mic_rx_read_merge_splite_nvkey((uint8_t **)&nvkey_record, &nvkey_size, sizeof(app_wireless_mic_rx_merge_split_mode_record_t) * tx_mic_amount);
        nvkey_record_count = nvkey_size / sizeof(app_wireless_mic_rx_merge_split_mode_record_t);
        APPS_LOG_MSGID_I(LOG_TAG", app_wireless_mic_rx_query_audio_mode bt_ull_le_srv_get_channel_mode = %d", 1, mode);
        if (NULL != nvkey_record) {
            if (NVKEY_STATUS_OK == nvkey_ret) {
                bool nvkey_changed = false;
                uint8_t i;
                uint8_t j;
                uint8_t new_record_count = nvkey_record_count;
                for (i = 0; i < tx_mic_amount; i++) {
                    for (j = 0; j < nvkey_record_count; j++) {
                        if (0 == memcmp(addr_list[i].addr, nvkey_record[j].addr, sizeof(bt_bd_addr_t))) {
                            connected_record_id[i] = j;
                            break;
                        }
                    }
                    if (connected_record_id[i] == 0xFF) {
                        memcpy(nvkey_record[new_record_count].addr, addr_list[i].addr, sizeof(bt_bd_addr_t));
                        if (nvkey_record_count == 0 || (nvkey_record[0].mode_id == APP_WIRELESS_MONO_MIC_MODE_MERGE || nvkey_record[0].mode_id == APP_WIRELESS_STEREO_MIC_MODE_MERGE)) {
                            nvkey_record[new_record_count].mode_id = APP_WIRELESS_STEREO_MIC_MODE_MERGE;
                        } else {
                            if (i % 2 == 0) {
                                nvkey_record[new_record_count].mode_id = APP_WIRELESS_STEREO_MIC_MODE_SPLIT_LEFT;
                            } else {
                                nvkey_record[new_record_count].mode_id = APP_WIRELESS_STEREO_MIC_MODE_SPLIT_RIGHT;
                            }
                        }
                        connected_record_id[i] = new_record_count;
                        new_record_count ++;
                        nvkey_changed = true;
                    }
                    if (change_addr != NULL && 0 == memcmp(addr_list[i].addr, *change_addr, sizeof(bt_bd_addr_t))) {
                        if (nvkey_record[connected_record_id[i]].mode_id != target_mode) {
                            nvkey_record[connected_record_id[i]].mode_id = target_mode;
                            nvkey_changed = true;
                        }
                    }
                }
                if (nvkey_changed) {
                    nvkey_write_data(NVID_APP_WM_MERGE_SPLIT_MODE, (uint8_t *)nvkey_record, new_record_count * sizeof(app_wireless_mic_rx_merge_split_mode_record_t));
                }
                *record_mode = (app_wireless_mic_rx_merge_split_mode_record_t *)malloc(sizeof(app_wireless_mic_rx_merge_split_mode_record_t) * tx_mic_amount);
                if (NULL != *record_mode) {
                    *record_count = tx_mic_amount;
                    for (i = 0; i < tx_mic_amount; i++) {
                        memcpy((*record_mode)[i].addr, addr_list[i].addr, sizeof(bt_bd_addr_t));
                        (*record_mode)[i].mode_id = nvkey_record[connected_record_id[i]].mode_id;
                        if (mode == BT_ULL_LE_CHANNEL_MODE_MONO) {
                            if (APP_WIRELESS_STEREO_MIC_MODE_MERGE == (*record_mode)[i].mode_id) {
                                (*record_mode)[i].mode_id = APP_WIRELESS_MONO_MIC_MODE_MERGE;
                            } else if (APP_WIRELESS_STEREO_MIC_MODE_SPLIT_LEFT == (*record_mode)[i].mode_id) {
                                (*record_mode)[i].mode_id = APP_WIRELESS_MONO_MIC_MODE_SPLIT_LEFT;
                            } else if (APP_WIRELESS_STEREO_MIC_MODE_SPLIT_RIGHT == (*record_mode)[i].mode_id) {
                                (*record_mode)[i].mode_id = APP_WIRELESS_MONO_MIC_MODE_SPLIT_RIGHT;
                            }
                        }
                        APPS_LOG_MSGID_I(LOG_TAG", app_wireless_mic_rx_query_audio_mode addr[%02X:%02X:%02X:%02X:%02X:%02X], mode: %d", 7,
                                         (*record_mode)[i].addr[0], (*record_mode)[i].addr[1], (*record_mode)[i].addr[2],
                                         (*record_mode)[i].addr[3], (*record_mode)[i].addr[4], (*record_mode)[i].addr[5],
                                         (*record_mode)[i].mode_id);
                    }
                }
            }
            free(nvkey_record);
        }
    }

    return bt_status;
}

static bt_status_t app_wireless_mic_rx_set_audio_transmit_info(bt_ull_streaming_interface_t interface,
                                                               app_wireless_mic_rx_merge_split_mode_record_t *record_data,
                                                               uint8_t record_data_num)
{
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t *info;
    uint32_t info_size;
    app_wireless_mic_rx_generate_audio_connection_info(&info, &info_size, record_data, record_data_num);
    if (info && info_size > 0) {
        bt_ull_streaming_t stream = {
            .streaming_interface = interface,
            .port = 0,
        };
        ret = bt_ull_le_srv_set_audio_connection_info(&stream, info, info_size);
        if (BT_STATUS_SUCCESS != ret) {
            /* If success, free by audio module. */
            wireless_mic_rx_audio_connection_info_free(info);
        }
    }
    //APPS_LOG_MSGID_I(LOG_TAG", set_audio_transmit_info to interface 0x%x, info = 0x%x, size = %d, ret = 0x%x", 4, interface, info, info_size, ret);
    APPS_LOG_DUMP_I(LOG_TAG", set_audio_transmit_info to interface 0x%x, size = %d, ret = 0x%x", info, info_size, interface, info_size, ret);
    return ret;
}

static void app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(bt_ull_streaming_interface_t interface)
{
    app_wireless_mic_rx_merge_split_mode_record_t *record_mode = NULL;
    uint8_t record_count = 0;

    app_wireless_mic_rx_query_audio_mode(&record_mode, &record_count, NULL, 0);
    if (record_mode && record_count > 0) {
        app_wireless_mic_rx_set_audio_transmit_info(interface, record_mode, record_count);
        free(record_mode);
    }
}

static uint8_t app_wireless_mic_rx_get_active_interface(bt_ull_streaming_interface_t interfaces[APP_WIRELESS_MIC_OUTPUT_TYPE_MAX])
{
    uint8_t active_out = 0;
    memset(interfaces, BT_ULL_STREAMING_INTERFACE_UNKNOWN, sizeof(bt_ull_streaming_interface_t) * APP_WIRELESS_MIC_OUTPUT_TYPE_MAX);
#if defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
    if (g_app_ull_dongle_context.line_out) {
        interfaces[APP_WIRELESS_MIC_OUTPUT_TYPE_LINE_OUT] = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        active_out ++;
    }
#endif
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
    if (g_app_ull_dongle_context.i2s_out) {
        interfaces[APP_WIRELESS_MIC_OUTPUT_TYPE_I2S_OUT] = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        active_out ++;
    }
#endif
    if (g_app_ull_dongle_context.usb_out_play) {
        interfaces[APP_WIRELESS_MIC_OUTPUT_TYPE_USB_OUT] = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        active_out ++;
    }

    return active_out;
}

static void app_wireless_mic_rx_set_audio_transmit_info_to_all_interface(const bt_bd_addr_t *change_addr,
                                                                         uint8_t target_mode)
{
    uint8_t j;
    uint8_t active_out = 0;
    app_wireless_mic_rx_merge_split_mode_record_t *record_mode = NULL;
    uint8_t record_count = 0;

    /* set BT address into connection info */
    bt_ull_streaming_interface_t interfaces_list[APP_WIRELESS_MIC_OUTPUT_TYPE_MAX];
    active_out = app_wireless_mic_rx_get_active_interface(interfaces_list);
    if (active_out > 0 || NULL != change_addr) {
        app_wireless_mic_rx_query_audio_mode(&record_mode, &record_count, change_addr, target_mode);
        if (record_mode && record_count > 0) {
            if (active_out > 0) {
                for (j = 0; j < APP_WIRELESS_MIC_OUTPUT_TYPE_MAX; j++) {
                    if (interfaces_list[j] != BT_ULL_STREAMING_INTERFACE_UNKNOWN) {
                        app_wireless_mic_rx_set_audio_transmit_info(interfaces_list[j], record_mode, record_count);
                    }
                }
            }
            free(record_mode);
        }
    }
}

static void app_wireless_mic_rx_send_safety_mode_to_tx(uint8_t safety_mode, bt_bd_addr_t *addr)
{
    if (addr) {
        apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, APPS_EVENTS_MIC_CONTROL_SAFETY_MODE,
                                                    &safety_mode, sizeof(safety_mode), addr);
    } else {
        uint8_t i;
        bt_addr_t addr_list[APP_WIRELESS_MIC_TX_NUM_MAX];
        uint8_t tx_mic_amount = APP_WIRELESS_MIC_TX_NUM_MAX;
        bt_status_t status = app_ull_dongle_le_get_connected_device_list(addr_list, &tx_mic_amount);
        for (i = 0; BT_STATUS_SUCCESS == status && i < tx_mic_amount; i++) {
            apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, APPS_EVENTS_MIC_CONTROL_SAFETY_MODE,
                                                        &safety_mode, sizeof(safety_mode), &(addr_list[i].addr));
        }
    }
}

static void app_wireless_mic_rx_set_safety_mode(bool switch_mode, bt_ull_streaming_interface_t interface) {

    uint8_t i;
    uint8_t safety_mode = 0;
    uint32_t safety_mode_size = sizeof(safety_mode);
    int32_t left_vol_diff;
    int32_t right_vol_diff;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_streaming_interface_t interfaces_list[APP_WIRELESS_MIC_OUTPUT_TYPE_MAX];

    nvkey_read_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, &safety_mode_size);
    if (switch_mode) {
        safety_mode = safety_mode ? 0 : 1;
        nvkey_write_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, sizeof(safety_mode));
        app_wireless_mic_rx_send_safety_mode_to_tx(safety_mode, NULL);
    }

    if (safety_mode) {
        if (bt_ull_le_srv_get_codec_type() == BT_ULL_LE_CODEC_LC3PLUS) {
            left_vol_diff = APP_WIRELESS_MIC_SAFETY_MODE_LC3PLUS_LEFT_GAIN;
            right_vol_diff = APP_WIRELESS_MIC_SAFETY_MODE_LC3PLUS_RIGHT_GAIN;
        } else {
            left_vol_diff = APP_WIRELESS_MIC_SAFETY_MODE_LEFT_GAIN;
            right_vol_diff = APP_WIRELESS_MIC_SAFETY_MODE_RIGHT_GAIN;
        }
    } else {
        left_vol_diff = 0;
        right_vol_diff = 0;
    }
    bt_ull_streaming_t streaming = {
        .port = 0,
    };
    if (BT_ULL_STREAMING_INTERFACE_UNKNOWN == interface) {
        app_wireless_mic_rx_get_active_interface(interfaces_list);
        for (i = 0; i < APP_WIRELESS_MIC_OUTPUT_TYPE_MAX; i++) {
            if (BT_ULL_STREAMING_INTERFACE_UNKNOWN != interfaces_list[i]) {
                streaming.streaming_interface = interfaces_list[i];
                status = bt_ull_le_srv_set_safety_mode_volume(&streaming, left_vol_diff, right_vol_diff);
            }
        }
    } else {
        streaming.streaming_interface = interface,
        status = bt_ull_le_srv_set_safety_mode_volume(&streaming, left_vol_diff, right_vol_diff);
    }
    APPS_LOG_MSGID_I(LOG_TAG", app_wireless_mic_rx_set_safety_mode : %d, status: 0x%x", 2, safety_mode, status);
}
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

static bool app_ull_dongle_idle_internal_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(LOG_TAG", create", 0);
            USB_HOST_TYPE usb_host_type = Get_USB_Host_Type();
            APPS_LOG_MSGID_I(LOG_TAG", create usb_host_type=%d", 1, usb_host_type);
#ifdef AIR_MS_GIP_ENABLE
            if (usb_host_type == USB_HOST_TYPE_XBOX) {
            } else {
                // Need to power up security_chip in non-xbox mode, then let it auto enter sleep mode in order to power_saving.
                // (active mode) 10mA, (sleep mode) 0.055mA
                extern void ms_gip_i2c_init(void);
                ms_gip_i2c_init();
            }
#endif
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            APPS_LOG_MSGID_I(LOG_TAG", destroy", 0);
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_RESUME: {
            APPS_LOG_MSGID_I(LOG_TAG", resume", 0);
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE: {
            APPS_LOG_MSGID_I(LOG_TAG", pause", 0);
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH: {
            APPS_LOG_MSGID_I(LOG_TAG", refresh", 0);
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_RESULT: {
            APPS_LOG_MSGID_I(LOG_TAG", result", 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_ull_dongle_idle_key_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    apps_config_key_action_t action;
    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        return ret;
    }

    switch (action) {
        case KEY_DONGLE_CONTROL_RECORD:
            apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, APPS_EVENTS_MIC_CONTROL_LOCAL_RECORDER, NULL, 0);
            ret = true;
            break;
        case KEY_DONGLE_CONTROL_MUTE_MIC:
            apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, APPS_EVENTS_MIC_CONTROL_MUTE, NULL, 0);
            ret = true;
            break;
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        case KEY_DONGLE_SWITCH_SPLIT_MERGE_MODE: {
            app_wireless_mic_rx_merge_split_mode_record_t *nvkey_record = NULL;
            uint32_t nvkey_size = 0;
            uint8_t nvkey_record_count = 0;

            app_wireless_mic_rx_read_merge_splite_nvkey((uint8_t **)&nvkey_record, &nvkey_size, 0);
            nvkey_record_count = nvkey_size / sizeof(app_wireless_mic_rx_merge_split_mode_record_t);
            if (NULL != nvkey_record) {
                if (nvkey_record_count > 0) {
                    uint8_t i;
                    if (nvkey_record[0].mode_id == APP_WIRELESS_MONO_MIC_MODE_MERGE || nvkey_record[0].mode_id == APP_WIRELESS_STEREO_MIC_MODE_MERGE) {
                        for (i = 0; i < nvkey_record_count; i++) {
                            if (i % 2 == 0) {
                                nvkey_record[i].mode_id = APP_WIRELESS_STEREO_MIC_MODE_SPLIT_LEFT;
                            } else {
                                nvkey_record[i].mode_id = APP_WIRELESS_STEREO_MIC_MODE_SPLIT_RIGHT;
                            }
                        }
                        uint8_t safety_mode = 0;
                        uint32_t safety_mode_size = sizeof(safety_mode);
                        nvkey_read_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, &safety_mode_size);
                        if (safety_mode == 1) {
                            app_wireless_mic_rx_set_safety_mode(true, BT_ULL_STREAMING_INTERFACE_UNKNOWN);
                        }
                    } else {
                        for (i = 0; i < nvkey_record_count; i++) {
                            nvkey_record[i].mode_id = APP_WIRELESS_STEREO_MIC_MODE_MERGE;
                        }
                    }

                    nvkey_write_data(NVID_APP_WM_MERGE_SPLIT_MODE, (uint8_t *)nvkey_record, nvkey_record_count * sizeof(app_wireless_mic_rx_merge_split_mode_record_t));
                }
                free(nvkey_record);
            }
            app_wireless_mic_rx_set_audio_transmit_info_to_all_interface(NULL, 0);
            ret = true;
            break;
        }
        case KEY_DONGLE_SWITCH_SAFETY_MODE: {
            /* Only allow safety mode in merge mode. */
            app_wireless_mic_rx_merge_split_mode_record_t *nvkey_record = NULL;
            uint32_t nvkey_size = 0;
            uint8_t nvkey_record_count = 0;

            app_wireless_mic_rx_read_merge_splite_nvkey((uint8_t **)&nvkey_record, &nvkey_size, 0);
            nvkey_record_count = nvkey_size / sizeof(app_wireless_mic_rx_merge_split_mode_record_t);
            APPS_LOG_MSGID_I(LOG_TAG", switch safety mode: nvkey_record_count=%d", 1, nvkey_record_count);
            if (NULL != nvkey_record && nvkey_record_count > 0) {
                if (nvkey_record[0].mode_id == APP_WIRELESS_MONO_MIC_MODE_MERGE || nvkey_record[0].mode_id == APP_WIRELESS_STEREO_MIC_MODE_MERGE) {
                    app_wireless_mic_rx_set_safety_mode(true, BT_ULL_STREAMING_INTERFACE_UNKNOWN);
                } else {
                    uint8_t safety_mode = 0;
                    uint32_t safety_mode_size = sizeof(safety_mode);
                    nvkey_read_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, &safety_mode_size);
                    if (safety_mode == 0) {
                        uint8_t i;
                        for (i = 0; i < nvkey_record_count; i++) {
                            nvkey_record[i].mode_id = APP_WIRELESS_STEREO_MIC_MODE_MERGE;
                        }
                        nvkey_write_data(NVID_APP_WM_MERGE_SPLIT_MODE, (uint8_t *)nvkey_record, nvkey_record_count * sizeof(app_wireless_mic_rx_merge_split_mode_record_t));
                        app_wireless_mic_rx_set_audio_transmit_info_to_all_interface(NULL, 0);
                        app_wireless_mic_rx_set_safety_mode(true, BT_ULL_STREAMING_INTERFACE_UNKNOWN);
                    }
                }
            }
            if (NULL != nvkey_record) {
                free(nvkey_record);
            }
            ret = true;
            break;
        }
#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case KEY_VOICE_UP:
        case KEY_VOICE_DN: {
            app_ull_dongle_change_volume_level_for_interface(action == KEY_VOICE_UP, BT_ULL_STREAMING_INTERFACE_LINE_OUT);
            ret = true;
            break;
        }
#endif
#endif
#endif
        default:
            break;
    }

    return ret;
}

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
/**
* @brief      This function receive some USB event.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_ull_dongle_idle_ull_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    APPS_LOG_MSGID_I(LOG_TAG", received ull event id: %x", 1, event_id);
    switch (event_id) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case BT_ULL_EVENT_LE_CONNECTED :
        case BT_ULL_EVENT_LE_DISCONNECTED : {
            app_ull_dongle_le_srv_event_callback(event_id, extra_data, data_len);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
            app_wireless_mic_rx_set_audio_transmit_info_to_all_interface(NULL, 0);
            if (BT_ULL_EVENT_LE_CONNECTED == event_id) {
                if (extra_data && data_len) {
                    bt_ull_le_connected_info_t *conn_info = (bt_ull_le_connected_info_t *)extra_data;
                    bt_addr_t *le_address = app_ull_dongle_le_get_bt_addr_by_conn_handle(conn_info->conn_handle);
                    if (le_address) {
                        uint8_t safety_mode = 0;
                        uint32_t safety_mode_size = sizeof(safety_mode);
                        nvkey_read_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, &safety_mode_size);
                        app_wireless_mic_rx_send_safety_mode_to_tx(safety_mode, &(le_address->addr));
                    }
                }
            }
#endif
        }
        break;    
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        case BT_ULL_EVENT_LE_HID_CONNECTED_IND:
        case BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND:
        case BT_ULL_EVENT_LE_HID_DISCONNECTED_IND:
        case BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND:
        case BT_ULL_EVENT_LE_STREAMING_START_IND:
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND:
        case BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND:
        case BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND:
        case BT_ULL_EVENT_LE_HID_RACE_DATA_IND:
        case BT_ULL_EVENT_USER_DATA_IND: {
            app_dongle_ull_le_hid_srv_event_callback(event_id, extra_data, data_len);
            break;
        }
#endif
        case BT_ULL_EVENT_USB_PLAYING_IND: {
            bt_ull_streaming_t *p_type = (bt_ull_streaming_t *)extra_data;
            if (p_type && sizeof(bt_ull_streaming_t) == data_len) {
#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
                //if (!(race_fota_is_running(TRUE) && race_fota_is_active_mode())) {
                    APPS_LOG_MSGID_I("app_ull_dongle_idle_ull_event_proc:BT_ULL_ACTION_START_STREAMING", 0);
                    bt_ull_action(BT_ULL_ACTION_START_STREAMING, p_type, sizeof(bt_ull_streaming_t));
                //}
#else
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, p_type, sizeof(bt_ull_streaming_t));
#endif
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_PLAYING, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case BT_ULL_EVENT_USB_STOP_IND: {
            bt_ull_streaming_t *p_type = (bt_ull_streaming_t *)extra_data;
            if (p_type && sizeof(bt_ull_streaming_t) == data_len) {
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, p_type, sizeof(bt_ull_streaming_t));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_STOP, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case BT_ULL_EVENT_USB_SAMPLE_RATE_IND: {
            bt_ull_sample_rate_t *p_sample = (bt_ull_sample_rate_t *)extra_data;
            if (p_sample && sizeof(bt_ull_sample_rate_t) == data_len) {
                bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, p_sample, sizeof(bt_ull_sample_rate_t));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_SAMPLE_RATE, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case BT_ULL_EVENT_USB_VOLUME_IND: {
            bt_ull_volume_t *p_vol = (bt_ull_volume_t *)extra_data;
            if (p_vol && sizeof(bt_ull_volume_t) == data_len) {
                bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, p_vol, sizeof(bt_ull_volume_t));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_VOLUME, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case BT_ULL_EVENT_USB_MUTE_IND: {
            bt_ull_streaming_t *p_type = (bt_ull_streaming_t *)extra_data;
            if (p_type && sizeof(bt_ull_streaming_t) == data_len) {
                bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, p_type, sizeof(bt_ull_streaming_t));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_MUTE, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case BT_ULL_EVENT_USB_UNMUTE_IND: {
            bt_ull_streaming_t *p_type = (bt_ull_streaming_t *)extra_data;
            if (p_type && sizeof(bt_ull_streaming_t) == data_len) {
                bt_ull_action(BT_ULL_ACTION_SET_STREAMING_UNMUTE, p_type, sizeof(bt_ull_streaming_t));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULLA_DONGLE_MUSIC_UNMUTE, but data not correct", 0);
            }
            ret = true;
            break;
        }
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        case BT_ULL_EVENT_LE_STREAMING_START_IND: {
            bt_ull_le_streaming_start_ind_t *start_stream = (bt_ull_le_streaming_start_ind_t *)extra_data;
            if (start_stream) {
                APPS_LOG_MSGID_E(LOG_TAG", received BT_ULL_EVENT_LE_STREAMING_START_IND mode:%d,", 1, start_stream->stream.streaming_interface);
                if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == start_stream->stream.streaming_interface
#ifdef AIR_DONGLE_I2S_SLV_OUT_ENABLE
                    || BT_ULL_STREAMING_INTERFACE_LINE_OUT == start_stream->stream.streaming_interface
#endif
#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
                    || BT_ULL_STREAMING_INTERFACE_I2S_OUT == start_stream->stream.streaming_interface
#endif
                    ) {
                    app_wireless_mic_rx_set_safety_mode(false, start_stream->stream.streaming_interface);
                    app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(start_stream->stream.streaming_interface);
                }
            }
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

static void app_ull_dongle_idle_set_sample_info(uint8_t interface_id, bool update_sample_rate, bool update_size_channel)
{
    const apps_usb_interface_enable_app_task_recorder_t *interface_status = app_preproc_activity_get_usb_interface_info(interface_id);
    APPS_LOG_MSGID_I(LOG_TAG", app_ull_dongle_idle_set_sample_info[%d]", 1, interface_id);
    if (interface_status == NULL) {
        return;
    }
    bt_ull_streaming_t stream = {
        .streaming_interface = BT_ULL_STREAMING_INTERFACE_UNKNOWN,
    };
#if defined(AIR_USB_AUDIO_ENABLE) && !defined(APPS_USB_AUDIO_SUPPORT)
    if (interface_id == APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT) {
        stream.port = 1;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    }
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    else if (interface_id == APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING) {
        stream.port = 0;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    }
#endif
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    if (interface_id == APPS_USB_EVENTS_INTERFACE_MIC) {
        stream.port = 0;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
    }
#endif
    if (stream.streaming_interface == BT_ULL_STREAMING_INTERFACE_UNKNOWN) {
        return;
    }

    if (update_size_channel) {
/******************************** sample_size ****************************************************/
        bt_ull_streaming_sample_size_t sample_size;
        sample_size.streaming = stream;
        sample_size.sample_size = interface_status->sample_size;
        APPS_LOG_MSGID_I(LOG_TAG", APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE[%d], sample_size:0x%x", 2, interface_id, sample_size.sample_size);
        if (sample_size.sample_size != 0) {
            bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE, &sample_size, sizeof(sample_size));
        }
/******************************** sample_size ****************************************************/
/******************************** Channel ****************************************************/
        bt_ull_streaming_sample_channel_t sample_channel;
        sample_channel.streaming = stream;
        sample_channel.sample_channel = interface_status->channel;
        APPS_LOG_MSGID_I(LOG_TAG", APPS_EVENTS_USB_AUDIO_CHANNEL[%d], sample_channel:0x%x", 2, interface_id, sample_channel.sample_channel);
        if (sample_channel.sample_channel != 0) {
            bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL, &sample_channel, sizeof(sample_channel));
        }
/******************************** Channel ****************************************************/
    }

/******************************** sample_rate ****************************************************/
    if (update_sample_rate && interface_status->sample_rate != 0) {
        bt_ull_sample_rate_t sp_rate;
        sp_rate.streaming = stream;
        sp_rate.sample_rate = interface_status->sample_rate;
        APPS_LOG_MSGID_I(LOG_TAG", APPS_EVENTS_USB_AUDIO_SAMPLE_RATE[%d], sample_rate:0x%x", 2, interface_id, sp_rate.sample_rate);
        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, &sp_rate, sizeof(sp_rate));
    }
/******************************** sample_size ****************************************************/
}

static bool app_ull_dongle_idle_usb_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    uint32_t usb_data = (uint32_t)extra_data;
    APPS_LOG_MSGID_I(LOG_TAG", received usb event id: 0x%x, extra_data:0x%x", 2, event_id, extra_data);

    switch (event_id) {
        case APPS_EVENTS_USB_AUDIO_PLAY:
        case APPS_EVENTS_USB_AUDIO_STOP:
        case APPS_EVENTS_USB_AUDIO_UNPLUG: {
            app_events_usb_port_t *p_port = (app_events_usb_port_t *) & (usb_data);
            bt_ull_streaming_t stream;
            if (!p_port) {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_PLAY, but data not correct", 0);
                break;
            }
            uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(p_port);
            const apps_usb_interface_enable_app_task_recorder_t *interface_status = app_preproc_activity_get_usb_interface_info(interface_id);
            if (interface_status && interface_status->enabled) {
                if (APP_USB_AUDIO_SPK_PORT == p_port->port_type) {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_port->port_num) {
                        stream.port = 0;    /* gaming */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_gaming_play     = true;
#endif
                    } else {
                        stream.port = 1;    /* chat */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_chat_play       = true;
#endif
                    }
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                    app_ull_dongle_change_mix_ratio();
#endif
                } else {
                    g_app_ull_dongle_context.usb_out_play = true;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;        /* mic */
                }

#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
                if (!(race_fota_is_running(TRUE) && race_fota_is_active_mode()))
#endif
                {
                    APPS_LOG_MSGID_I("app_ull_dongle_idle_usb_event_proc:BT_ULL_ACTION_START_STREAMING[%d]", 1, interface_id);
                    app_ull_dongle_idle_set_sample_info(interface_id, false, true);
                    bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
                    app_wireless_mic_rx_set_safety_mode(false, BT_ULL_STREAMING_INTERFACE_MICROPHONE);
                    app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(BT_ULL_STREAMING_INTERFACE_MICROPHONE);
#endif
                }
                APPS_LOG_MSGID_I(LOG_TAG", received APPS_EVENTS_USB_AUDIO_PLAY[%d]: stream_interface=0x%02x, stream_port=%d",
                        3, interface_id, stream.streaming_interface, stream.port);
                app_ull_dongle_idle_set_sample_info(interface_id, true, false);
            } else if (interface_status) {
                bt_ull_streaming_t stream;
                if (APP_USB_AUDIO_SPK_PORT == p_port->port_type) {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_port->port_num) {
                        stream.port = 0;    /* gaming */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_gaming_play     = false;
#endif
                    } else {
                        stream.port = 1;    /* chat */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_chat_play       = false;
#endif
                    }
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                    app_ull_dongle_change_mix_ratio();
#endif
                } else {
                    g_app_ull_dongle_context.usb_out_play = false;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;        /* mic */
                }
                APPS_LOG_MSGID_I(LOG_TAG", received APPS_EVENTS_USB_AUDIO_STOP[%d]: stream_interface=0x%02x, stream_port=%d",
                                        3, interface_id, stream.streaming_interface, stream.port);
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            }
            break;
        }
#if 0
        case APPS_EVENTS_USB_AUDIO_PLAY: {
            app_events_usb_port_t *p_port = (app_events_usb_port_t *) & (usb_data);
            if (p_port) {
                bt_ull_streaming_t stream;
                if (APP_USB_AUDIO_SPK_PORT == p_port->port_type) {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_port->port_num) {
                        stream.port = 0;    /* gaming */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_gaming_play     = true;
#endif
                    } else {
                        stream.port = 1;    /* chat */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_chat_play       = true;
#endif
                    }
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                    app_ull_dongle_change_mix_ratio();
#endif
                } else {
                    g_app_ull_dongle_context.usb_out_play = true;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;        /* mic */
                }
#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
                //if (!(race_fota_is_running(TRUE) && race_fota_is_active_mode())) {
                    APPS_LOG_MSGID_I("app_ull_dongle_idle_usb_event_proc:BT_ULL_ACTION_START_STREAMING", 0);
                    bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
                //}
#else
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
                app_wireless_mic_rx_set_safety_mode(false, BT_ULL_STREAMING_INTERFACE_MICROPHONE);
                app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(BT_ULL_STREAMING_INTERFACE_MICROPHONE);
#endif
                APPS_LOG_MSGID_I(LOG_TAG", received APPS_EVENTS_USB_AUDIO_PLAY: stream_interface=0x%02x, stream_port=%d",
                        2, stream.streaming_interface, stream.port);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_PLAY, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case APPS_EVENTS_USB_AUDIO_STOP:
        case APPS_EVENTS_USB_AUDIO_UNPLUG: {
            app_events_usb_port_t *p_port = (app_events_usb_port_t *) & (usb_data);
            if (p_port) {
                bt_ull_streaming_t stream;
                if (APP_USB_AUDIO_SPK_PORT == p_port->port_type) {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_port->port_num) {
                        stream.port = 0;    /* gaming */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_gaming_play     = false;
#endif
                    } else {
                        stream.port = 1;    /* chat */
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                        g_app_ull_dongle_context.spk_chat_play       = false;
#endif
                    }
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                    app_ull_dongle_change_mix_ratio();
#endif
                } else {
                    g_app_ull_dongle_context.usb_out_play = false;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;        /* mic */
                }
                APPS_LOG_MSGID_I(LOG_TAG", received APPS_EVENTS_USB_AUDIO_STOP: stream_interface=0x%02x, stream_port=%d",
                                        2, stream.streaming_interface, stream.port);
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_STOP, but data not correct", 0);
            }
            ret = true;
            break;
        }
#endif
        case APPS_EVENTS_USB_AUDIO_VOLUME: {
            app_events_usb_volume_t *p_vol = (app_events_usb_volume_t *)extra_data;
            if (p_vol) {
                APPS_LOG_MSGID_I(LOG_TAG", db is l=%d, r=%d", 2, p_vol->left_db, p_vol->right_db);
                bt_ull_volume_t vol;
                if (APP_USB_AUDIO_SPK_PORT == p_vol->port_type) {
                    vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_vol->port_num) {
                        vol.streaming.port = 0;    /* gaming */
                    } else {
                        vol.streaming.port = 1;    /* chat */
                    }
                } else {
                    vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    vol.streaming.port = 0;        /* mic */
                }
                vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
                /* L/R channel volue is same */
                if (p_vol->left_volume == p_vol->right_volume) {
                    vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
                    vol.volume = p_vol->left_volume;
                    //vol.gain = p_vol->left_db;
                    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
                } else {
                    if (0xFF != p_vol->left_volume) {
                        vol.channel = BT_ULL_AUDIO_CHANNEL_LEFT;
                        vol.volume = p_vol->left_volume;
                        //vol.gain = p_vol->left_db;
                        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
                    }
                    if (0xFF != p_vol->right_volume) {
                        vol.channel = BT_ULL_AUDIO_CHANNEL_RIGHT;
                        vol.volume = p_vol->right_volume;
                        //vol.gain = p_vol->right_db;
                        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
                    }
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_VOLUME, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case APPS_EVENTS_USB_AUDIO_MUTE: {
            app_events_usb_mute_t *p_mute = (app_events_usb_mute_t *) & (usb_data);
            if (p_mute) {
                bt_ull_streaming_t stream;
                if (APP_USB_AUDIO_SPK_PORT == p_mute->port_type) {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_mute->port_num) {
                        stream.port = 0;    /* gaming */
                    } else {
                        stream.port = 1;    /* chat */
                    }
                } else {
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;        /* mic */
                }
                if (p_mute->is_mute) {
                    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
                } else {
                    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_UNMUTE, &stream, sizeof(stream));
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_MUTE, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SAMPLE_RATE: {
            app_events_usb_sample_rate_t *p_rate = (app_events_usb_sample_rate_t *) & (usb_data);
            if (p_rate) {
                uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(&p_rate->port);
                app_ull_dongle_idle_set_sample_info(interface_id, true, true);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_SAMPLE_RATE, but data not correct", 0);
            }
            ret = true;
            break;
        }
#if 0
        case APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE: {
            app_events_usb_sample_size_t *p_sample_size = (app_events_usb_sample_size_t *)&(usb_data);
            if (NULL != p_sample_size) {
                bt_ull_streaming_sample_size_t sample_size;
                if (APP_USB_AUDIO_SPK_PORT == p_sample_size->port_type) {
                    sample_size.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_sample_size->port_num) {
                        sample_size.streaming.port = 0;    /* gaming */
                    } else {
                        sample_size.streaming.port = 1;    /* chat */
                    }
                } else {
                    sample_size.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    sample_size.streaming.port = 0;        /* mic */
                }
                sample_size.sample_size = p_sample_size->size;
                APPS_LOG_MSGID_I(LOG_TAG", APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE, sample_rate:0x%x", 1, sample_size.sample_size);
                if (sample_size.sample_size != 0) {
                    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE, &sample_size, sizeof(sample_size));
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE, but data not correct", 0);
            }
            ret = true;
            break;
        }
        case APPS_EVENTS_USB_AUDIO_CHANNEL: {
            app_events_usb_channel_t *p_channel = (app_events_usb_channel_t *)&(usb_data);
            if (NULL != p_channel) {
                bt_ull_streaming_sample_channel_t sample_channel;
                if (APP_USB_AUDIO_SPK_PORT == p_channel->port_type) {
                    sample_channel.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    if (1 == p_channel->port_num) {
                        sample_channel.streaming.port = 0;    /* gaming */
                    } else {
                        sample_channel.streaming.port = 1;    /* chat */
                    }
                } else {
                    sample_channel.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    sample_channel.streaming.port = 0;        /* mic */
                }
                sample_channel.sample_channel = p_channel->channel;
                APPS_LOG_MSGID_I(LOG_TAG", APPS_EVENTS_USB_AUDIO_CHANNEL, sample_rate:0x%x", 1, sample_channel.sample_channel);
                if (sample_channel.sample_channel != 0)
                    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL, &sample_channel, sizeof(sample_channel));
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_CHANNEL, but data not correct", 0);
            }
            ret = true;
            break;
        }
#endif
        case APPS_EVENTS_USB_AUDIO_RESET: {
#if defined(AIR_USB_AUDIO_ENABLE)
            bt_ull_streaming_t stream;
            /* stop gaming */
            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            stream.port = 0;    /* gaming */
            bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            /* stop chat */
            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            stream.port = 1;    /* chat */
            bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            /* stop mic */
            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
            stream.port = 0;    /* mic */
            bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            ret = true;
#endif
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SUSPEND: {
            APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_SUSPEND, but no handler", 0);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_RESUME: {
            APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_RESUME, but no handler", 0);
            break;
        }
        default:
            break;
    }

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static bool app_ull_dongle_idle_usb_hid_generic_data_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        case APPS_EVENT_USB_HID_LED_CONTROL: {
            app_events_usb_hid_led_control_t *p_led_control = (app_events_usb_hid_led_control_t *)extra_data;
            app_events_usb_hid_led_control_t usb_hid_led_control;
            usb_hid_led_control.led_control = p_led_control->led_control;
            bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONTROL_RGB, &usb_hid_led_control, sizeof(app_events_usb_hid_led_control_t));
            ret = true;
            break;
        }
    }
    return ret;
}
#endif
static bool app_ull_dongle_send_data_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    app_ull_user_data_t *ull_app_data = NULL;
    switch (event_id) {
        case ULL_EVT_DONGLE_MODE: {
            bt_bd_addr_t addr_list = {0};
            if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), &addr_list, 1)) {
                bt_ull_user_data_t tx_data;
                memcpy(&(tx_data.remote_address), addr_list, sizeof(bt_bd_addr_t));
                ull_app_data = (app_ull_user_data_t *)pvPortMalloc(sizeof(app_ull_user_data_t));
                if (ull_app_data) {
                    ull_app_data->user_evt = ULL_EVT_DONGLE_MODE;
                    ull_app_data->data_len = 1;
                    ull_app_data->data[0] = g_dongle_mode;
                    tx_data.user_data = (uint8_t *)ull_app_data;
                    tx_data.user_data_length = sizeof(app_ull_user_data_t);
                    bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
                    vPortFree(ull_app_data);
                }
            }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            else {
                bt_ull_user_data_t tx_data;
                bt_addr_t address_list[BT_ULL_LE_MAX_LINK_NUM];
                uint8_t count = BT_ULL_LE_MAX_LINK_NUM;
                uint8_t i;
                if (BT_STATUS_SUCCESS != app_ull_dongle_le_get_connected_device_list(address_list, &count)) {
                    count = 0;
                }
                ull_app_data = (app_ull_user_data_t *)pvPortMalloc(sizeof(app_ull_user_data_t));
                if(ull_app_data) {
                    ull_app_data->user_evt = ULL_EVT_DONGLE_MODE;
                    ull_app_data->data_len = 1;
                    ull_app_data->data[0] = g_dongle_mode;
                    tx_data.user_data = (uint8_t *)ull_app_data;
                    tx_data.user_data_length = sizeof(app_ull_user_data_t);
                    for (i = 0; i < count; i++) {
                        memcpy(tx_data.remote_address, address_list[i].addr, sizeof(bt_bd_addr_t));
                        bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
                    }

                    APPS_LOG_MSGID_I("Send user data by ULL Dongle LE", 0);
                    vPortFree(ull_app_data);
                }
            }
#endif
            ret = true;
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
void app_ull_dongle_set_lineout_volume_value(uint8_t value)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_ull_volume_t vol = {0};
    vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
    vol.streaming.port                = 0;
    vol.action                        = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
    vol.channel                       = BT_ULL_AUDIO_CHANNEL_DUAL;
    vol.volume                        = value;
    bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    if (bt_status == BT_STATUS_SUCCESS) {
        g_app_ull_dongle_context.lineout_volume_values = value;
    }
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_set_line_out_volume_value :value=%d, status=0x%x", 2, value, bt_status);
}
#endif

#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
void app_ull_dongle_set_linein_volume_value(uint8_t value)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_ull_volume_t vol = {0};
    vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
    vol.streaming.port                = 0;
    vol.action                        = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
    vol.channel                       = BT_ULL_AUDIO_CHANNEL_DUAL;
    vol.volume                        = value;
    bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    if (bt_status == BT_STATUS_SUCCESS) {
        g_app_ull_dongle_context.linein_volume_values = value;
    }
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_set_line_in_volume_value :value=%d, status=0x%x", 2, value, bt_status);
}

void app_ull_dongle_change_linein_volume_level(bool up)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_ull_volume_t volume_param = {0};
    volume_param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
    volume_param.streaming.port = 0;
    volume_param.channel = BT_ULL_VOLUME_CHANNEL_DUEL;
    volume_param.volume = 10; /* The delta value. */
    if (up) {
        volume_param.action = BT_ULL_VOLUME_ACTION_SET_UP;
    } else {
        volume_param.action = BT_ULL_VOLUME_ACTION_SET_DOWN;
    }
    bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_change_line_in_volume :is_up=%d, status=0x%x", 2, up, bt_status);
}

void app_ull_dongle_change_mix_ratio(void)
{
    bt_status_t bt_status         = BT_STATUS_FAIL;
    uint8_t spk_gaming_ratio      = 0;
    uint8_t spk_chat_ratio        = 0;
    uint8_t line_in_ratio         = 0;
    bt_ull_mix_ratio_t mix_ratio  = {0};
    mix_ratio.num_streaming       = BT_ULL_MAX_STREAMING_NUM;

    if (!g_app_ull_dongle_context.line_in) {
        spk_gaming_ratio = APP_LINE_IN_MIX_RATIO_MAX_LEVEL;
        spk_chat_ratio   = APP_LINE_IN_MIX_RATIO_MAX_LEVEL;
        line_in_ratio    = APP_LINE_IN_MIX_RATIO_MIN_LEVEL;
    } else {
        if (g_app_ull_dongle_context.spk_gaming_play || g_app_ull_dongle_context.spk_chat_play) {
            spk_gaming_ratio = APP_LINE_IN_MIX_RATIO_BALANCED_LEVEL;
            spk_chat_ratio   = APP_LINE_IN_MIX_RATIO_BALANCED_LEVEL;
            line_in_ratio    = APP_LINE_IN_MIX_RATIO_BALANCED_LEVEL;
        } else if (!g_app_ull_dongle_context.spk_gaming_play && !g_app_ull_dongle_context.spk_chat_play) {
            spk_gaming_ratio = APP_LINE_IN_MIX_RATIO_MIN_LEVEL;
            spk_chat_ratio   = APP_LINE_IN_MIX_RATIO_MIN_LEVEL;
            line_in_ratio    = APP_LINE_IN_MIX_RATIO_MAX_LEVEL;
        }
    }

    mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    if (g_app_ull_dongle_context.spk_gaming_play) {
        mix_ratio.streamings[0].streaming.port = 0; /* gaming streaming port */
        mix_ratio.streamings[0].ratio = spk_gaming_ratio;
    } else {
        mix_ratio.streamings[0].streaming.port = 1; /* chat streaming port */
        mix_ratio.streamings[0].ratio = spk_chat_ratio;
    }

    mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
    mix_ratio.streamings[1].streaming.port = 0; /* line in streaming port */
    mix_ratio.streamings[1].ratio = line_in_ratio;

    bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_dongle_change_mix_ratio: spk_gaming_ratio=%d, spk_chat_ratio=%d, line_in_ratio=%d, status=0x%x",
            4, spk_gaming_ratio, spk_chat_ratio, line_in_ratio, bt_status);
}

static bool app_ull_dongle_get_line_in_streaming_is_playing(void)
{
    bool ret = false;
    bt_ull_streaming_info_t info = {0};
    bt_ull_streaming_t streaming = {
        .streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN,
        .port = 0,
    };

    if ((BT_STATUS_SUCCESS == bt_ull_get_streaming_info(streaming,&info)) && info.is_playing) {
        ret = true;
    }
    APPS_LOG_MSGID_I(LOG_TAG" line_in_streaming_is_playing=%d", 1, ret);
    return ret;
}
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
void app_ull_dongle_change_volume_level_for_interface(bool up, bt_ull_streaming_interface_t interface)
{
#ifdef AIR_WIRELESS_MIC_RX_ENABLE
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_ull_volume_t volume_param = {0};
    volume_param.streaming.streaming_interface = interface;
    volume_param.streaming.port = 0;
    volume_param.channel = BT_ULL_VOLUME_CHANNEL_DUEL;
    volume_param.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
    uint8_t current_volume = bt_ull_le_srv_get_streaming_volume(&volume_param.streaming);
    if (up) {
        volume_param.volume = current_volume + 10;
        if (volume_param.volume > 100) {
            volume_param.volume = 100;
        }
    } else {
        volume_param.volume = current_volume > 10 ? current_volume - 10 : 0;
    }
    bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_change_[%x]_volume :is_up=%d, volume = %d, status=0x%x", 4, interface, up, volume_param.volume, bt_status);
#endif
}
#endif

#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
static bool app_ull_dongle_idle_line_in_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_status_t bt_status = BT_STATUS_FAIL;

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_LINE_IN_STATUS: {
            bool line_in_status = (bool)extra_data;
            bt_ull_streaming_t stream = {0};

            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
            stream.port = 0;
            if (line_in_status) {
#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
                //if (race_fota_is_running(TRUE) && race_fota_is_active_mode()) {
                //    break;
                //}
#endif
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                g_app_ull_dongle_context.line_in = true;
                if (!app_ull_dongle_get_line_in_streaming_is_playing()) {
                    bt_status = bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
                    if (bt_status == BT_STATUS_SUCCESS) {
                        app_ull_dongle_set_linein_volume_value(APP_LINE_IN_DEFAULT_VOLUME_VALUES);
                        app_ull_dongle_change_mix_ratio();
                    }
                }
#endif
#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
                /* line out. */
                g_app_ull_dongle_context.line_out = true;
                stream.streaming_interface        = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
                //app_ull_dongle_set_lineout_volume_value(APP_LINE_OUT_DEFAULT_VOLUME_VALUES);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
                app_wireless_mic_rx_set_safety_mode(false, BT_ULL_STREAMING_INTERFACE_LINE_OUT);
                app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(BT_ULL_STREAMING_INTERFACE_LINE_OUT);
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#endif
            } else {
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                g_app_ull_dongle_context.line_in  = false;
                if (app_ull_dongle_get_line_in_streaming_is_playing()) {
                    bt_status = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
                    app_ull_dongle_change_mix_ratio();
                }
#endif
#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
                /* line out. */
                g_app_ull_dongle_context.line_out = false;
                stream.streaming_interface        = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                bt_status = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
#endif
            }
            APPS_LOG_MSGID_I(LOG_TAG",[LINE_IN_DET] received APPS_EVENTS_INTERACTION_LINE_IN_STATUS: line_in_status=%d, bt_status=%d",
                             2, line_in_status, bt_status);
            break;
        }
        default:
            break;
    }

    return ret;
}
#endif
#endif //AIR_BT_ULTRA_LOW_LATENCY_ENABLE or AIR_BLE_ULTRA_LOW_LATENCY_ENABLE

#ifdef MTK_RACE_CMD_ENABLE
static bool app_ull_dongle_idle_proc_tx_status_events(struct _ui_shell_activity *self,
                                                         uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len)
{
    bool ret             = false;
#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && (defined AIR_WIRELESS_MIC_ENABLE)
    apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
    if (pkg == NULL) {
        APPS_LOG_MSGID_W(LOG_TAG" tx_status_events: pkg is NULL!", 0);
        return ret;
    }
    /* EVENT_GROUP_UI_SHELL_WIRELESS_MIC is 50 and it must be same on dongle and headset!!! */
    if (pkg->event_group == EVENT_GROUP_UI_SHELL_WIRELESS_MIC) {
        app_ull_dongle_race_remote_control_type_t control_type = APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_INVALID;
        APPS_LOG_MSGID_I(LOG_TAG" tx_status_event, type=%dm data_len=%d.", 2, pkg->event_id, pkg->extra_data_len);
        switch (pkg->event_id) {
            /* The Teams application handshake with dongle done. */
            case APPS_EVENTS_TX_MIC_STATUS: {
                bool mute_status = false;
                if (pkg->extra_data_len != 0) {
                    memcpy(&mute_status, pkg->data, sizeof(bool));
                    control_type = APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_MUTE_MIC;
                    APPS_LOG_MSGID_I(LOG_TAG" tx_status_events: mute_status=%d", 1, mute_status);
                }
                break;
            }
            /* It will not be received now. */
            case APPS_EVENTS_TX_RECORDER_STATUS: {
                bool record_status = false;
                if (pkg->extra_data_len != 0) {
                    memcpy(&record_status, pkg->data, sizeof(bool));
                    control_type = APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_RECORD;
                    APPS_LOG_MSGID_I(LOG_TAG" tx_status_events: record_status=%d", 1, record_status);
                }
                break;
            }
            case APPS_EVENTS_TX_BATTERY_STATUS: {
                uint8_t battery_percent = 0;
                if (pkg->extra_data_len != 0) {
                    memcpy(&battery_percent, pkg->data, sizeof(uint8_t));
                    control_type = APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_BATTERY;
                    APPS_LOG_MSGID_I(LOG_TAG" tx_status_events: battery_percent=%d", 1, battery_percent);
                }
                break;
            }
            case APPS_EVENTS_TX_VOLUME_STATUS: {
                int32_t volume_data = 0;
                if (pkg->extra_data_len != 0) {
                    memcpy(&volume_data, pkg->data, sizeof(int32_t));
                    control_type = APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_TX_VOLUME;
                    APPS_LOG_MSGID_I(LOG_TAG" tx_status_events: volume=%d", 1, volume_data);
                }
                break;
            }
            default:
                break;
        }


        if (control_type != APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_INVALID) {
            uint8_t channel_id = *(((uint8_t *)extra_data) + data_len);
            apps_dongle_race_cmd_on_remote_control_state_change(control_type, pkg->data[0], channel_id);
        }

    }
#endif
    return ret;
}
#endif

#include "apps_events_i2s_in_event.h"

static bool app_ull_dongle_idle_handle_i2s_in_event(ui_shell_activity_t *self,
                                        uint32_t event_id,
                                        void *extra_data,
                                        size_t data_len)
{
    bool ret = false;

#ifdef AIR_DONGLE_I2S_SLV_OUT_ENABLE
    if (APPS_EVENTS_I2S_IN_STATUS_CHANGE == event_id && NULL != extra_data) {
#ifdef RACE_FOTA_ACTIVE_MODE_ULL_SUPPORT
        //if (race_fota_is_running(TRUE) && race_fota_is_active_mode()) {
        //    return ret;
        //}
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        app_i2s_in_det_t *i2s_param = (app_i2s_in_det_t *)extra_data;
        bt_ull_streaming_t stream = {0};
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        bt_ull_action_t action;
        g_app_ull_dongle_context.i2s_out = i2s_param->i2s_state;
        if (i2s_param->i2s_state) { 
            action = BT_ULL_ACTION_START_STREAMING;
        } else {
            action = BT_ULL_ACTION_STOP_STREAMING;
        }
        bt_ull_action(action, &stream, sizeof(stream));
        if (BT_ULL_ACTION_START_STREAMING == action) {
            app_wireless_mic_rx_set_safety_mode(false, BT_ULL_STREAMING_INTERFACE_I2S_OUT);
            app_wireless_mic_rx_set_audio_transmit_info_to_single_interface(BT_ULL_STREAMING_INTERFACE_I2S_OUT);
        }
#endif
    }
#endif

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static bt_ull_le_srv_call_event_t s_last_event = USB_HID_SRV_EVENT_CALL_END;
#endif
bool app_ull_dongle_idle_activity_proc(
    ui_shell_activity_t *self,
    uint32_t event_group,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    bool ret = false;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    static bool s_muted = false;
#endif

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_ull_dongle_idle_internal_event_proc(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = app_ull_dongle_idle_key_event_proc(self, event_id, extra_data, data_len);
            break;
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            ret = app_ull_dongle_idle_ull_event_proc(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_ULL_SEND_CUSTOM_DATA:
            ret = app_ull_dongle_send_data_event_proc(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_USB_AUDIO:
            ret = app_ull_dongle_idle_usb_event_proc(self, event_id, extra_data, data_len);
            return false;
            //break;
#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
        case EVENT_GROUP_UI_SHELL_LINE_IN: {
            ret = app_ull_dongle_idle_line_in_event_proc(self, event_id, extra_data, data_len);
            return false;
            break;
        }
#endif
#endif //AIR_BT_ULTRA_LOW_LATENCY_ENABLE
#ifdef MTK_RACE_CMD_ENABLE
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            ret = app_ull_dongle_idle_proc_tx_status_events(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_I2S_IN:
            ret = app_ull_dongle_idle_handle_i2s_in_event(self, event_id, extra_data, data_len);
            break;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
            APPS_LOG_MSGID_I("EVENT_GROUP_UI_SHELL_USB_HID_CALL event=%d", 1, event_id);
            bt_ull_le_call_srv_send_event((bt_ull_le_srv_call_event_t)event_id, NULL, 0);
            if (event_id < USB_HID_SRV_EVENT_CALL_MIC_MUTE) {
                s_last_event = (bt_ull_le_srv_call_event_t)event_id;
                if (s_last_event == BT_ULL_LE_SRV_CALL_EVENT_END) {
                    s_muted = false;
                }
            } else if (event_id == BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_MUTE) {
                s_muted = true;
            } else if (event_id == BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_UNMUTE) {
                s_muted = false;
            }
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                    NULL, 0);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            if (event_id == APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN) {
                APPS_LOG_MSGID_I("APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN last sta=%d, mute=%d", 2, s_last_event, s_muted);
                if (s_last_event == BT_ULL_LE_SRV_CALL_EVENT_INCOMING) {
                    apps_config_set_background_led_pattern(LED_INDEX_INCOMING_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                }
                else if (s_last_event >= BT_ULL_LE_SRV_CALL_EVENT_ACTIVE && s_last_event <= BT_ULL_LE_SRV_CALL_EVENT_UNHOLD)
                {
                    /* USE the air pairing to indicate the call actite but muted case. */
                    apps_config_set_background_led_pattern(s_muted ? LED_INDEX_AIR_PAIRING: LED_INDEX_CALL_ACTIVE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                }
                ret = true;
            }
            break;
        }
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_HID_GENARIC_DATA:
            ret = app_ull_dongle_idle_usb_hid_generic_data_proc(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
bool app_ull_dongle_has_call_sta() {
    return (s_last_event != BT_ULL_LE_SRV_CALL_EVENT_END);
}
#else
bool app_ull_dongle_has_call_sta() {
    return false;
}
#endif

#ifdef AIR_MS_GIP_ENABLE
//dongle switch
const unsigned char DONGLE_SWITCH_EINT = HAL_GPIO_10;
const unsigned char DONGLE_SWITCH_DET_PIN = HAL_GPIO_10;

static void dongle_switch_detect_callback(void *user_data)
{
    hal_gpio_data_t current_gpio_status = 0;
    hal_eint_mask(DONGLE_SWITCH_EINT);

    hal_gpio_get_input(DONGLE_SWITCH_DET_PIN, &current_gpio_status);
    g_dongle_mode = (current_gpio_status == HAL_GPIO_DATA_HIGH ? APP_DONGLE_MODE_PC : APP_DONGLE_MODE_XBOX_PS);
    APPS_LOG_MSGID_I("dongle_switch, dongle_mode=%d", 1, g_dongle_mode);

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    ui_shell_send_event(TRUE, EVENT_PRIORITY_HIGHEST,
                        EVENT_GROUP_UI_SHELL_APP_SERVICE,
                        APP_DONGLE_SERVICE_MODE_SWITCH_EVENT,
                        NULL, 0, NULL, 0);
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */
    hal_eint_unmask(DONGLE_SWITCH_EINT);
}

static void dongle_switch_init_set(void)
{
    hal_gpio_data_t current_gpio_status = 0;
    hal_gpio_get_input(DONGLE_SWITCH_DET_PIN, &current_gpio_status);
    g_dongle_mode = (current_gpio_status == HAL_GPIO_DATA_HIGH ? APP_DONGLE_MODE_PC : APP_DONGLE_MODE_XBOX_PS);
    APPS_LOG_MSGID_I("dongle_switch init, g_dongle_mode=%d", 1, g_dongle_mode);

    if (g_dongle_mode == APP_DONGLE_MODE_XBOX_PS) {
        Set_USB_Host_Type(USB_HOST_TYPE_XBOX);
    }
}

void dongle_switch_det_init(void)
{
    hal_eint_config_t config;
    hal_eint_status_t sta;
    APPS_LOG_MSGID_I("dongle_switch_det_init", 0);
    /* For falling and rising detect. */
    config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
    config.debounce_time = 300;

    hal_gpio_init(DONGLE_SWITCH_DET_PIN);
    hal_eint_mask(DONGLE_SWITCH_EINT);

    sta = hal_eint_init(DONGLE_SWITCH_EINT, &config);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E("init dongle_switch eint failed: %d", 1, sta);
        hal_eint_unmask(DONGLE_SWITCH_EINT);
        return;
    }

    sta = hal_eint_register_callback(DONGLE_SWITCH_EINT, dongle_switch_detect_callback, NULL);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E("registe dongle_switch eint callback failed: %d", 1, sta);
        hal_eint_unmask(DONGLE_SWITCH_EINT);
        hal_eint_deinit(DONGLE_SWITCH_EINT);
        return;
    }
    hal_eint_unmask(DONGLE_SWITCH_EINT);

    dongle_switch_init_set();
}

#endif /* AIR_MS_GIP_ENABLE */
