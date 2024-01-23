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
#include "stdlib.h"
#include "apps_debug.h"
#include "FreeRTOS.h"
#include "bt_system.h"
#include "atci.h"
#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "app_dongle_session_manager.h"


#if defined(AIR_BT_SOURCE_ENABLE) || defined(AIR_LE_AUDIO_UNICAST_ENABLE)
#define LOG_TAG         "[APP][SESSION]"
#define APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX       4
#define APP_DONGLE_SESSION_MANAGER_HFP_CODEC_MASK       0x000000FF
#define APP_DONGLE_SESSION_MANAGER_A2DP_CODEC_MASK      0x0000FF00

#define APP_DONGLE_SESSION_MANAGER_SOURCE_NONE      0
#define APP_DONGLE_SESSION_MANAGER_SOURCE_LEA       1
#define APP_DONGLE_SESSION_MANAGER_SOURCE_EDR       2
typedef uint8_t app_dongle_session_manager_source_type_t;

typedef struct {
    app_dongle_session_manager_source_type_t source_type;
    app_dongle_session_manager_session_usage_t session_usage;
    uint32_t session_type;//app_dongle_session_manager_lea_session_type_t
} app_dongle_session_manager_session_type_t;

typedef struct {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bt_handle_t lea_connection_handle;
    bt_handle_t lea_connection_handle_partner;
    uint16_t sink_sampling_frequencies;         // refer to #SUPPORTED_SAMPLING_FREQ_8KHZ
    uint16_t source_sampling_frequencies;       // refer to #SUPPORTED_SAMPLING_FREQ_8KHZ
    uint32_t profile;
#endif
#ifdef AIR_BT_SOURCE_ENABLE
    bt_addr_t edr_addr;
    app_dongle_session_manager_edr_session_type_t edr_session_type;
#endif
} app_dongle_session_manager_info_t;

static app_dongle_session_manager_info_t g_app_dongle_session_manager[APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX];
static app_dongle_session_manager_source_type_t g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_NONE;
static app_dongle_session_manager_session_usage_t g_app_dongle_session_manager_current_session_usage = APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX;

static app_dongle_session_manager_session_type_t g_app_dongle_session_mgr_support_session_type_tbl[] = {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_COMM_16KHZ},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_COMM_32KHZ},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA,         APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA,         APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_MEDIA_48KHZ},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_LEA,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA,         APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_MEDIA_96KHZ},
#endif
#ifdef AIR_BT_SOURCE_ENABLE
    {APP_DONGLE_SESSION_MANAGER_SOURCE_EDR,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, BT_SOURCE_SRV_CODEC_TYPE_NONE},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_EDR,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, BT_SOURCE_SRV_CODEC_TYPE_CVSD},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_EDR,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION, BT_SOURCE_SRV_CODEC_TYPE_MSBC},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_EDR,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA,         BT_SOURCE_SRV_CODEC_TYPE_NONE},
    {APP_DONGLE_SESSION_MANAGER_SOURCE_EDR,  APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA,         BT_SOURCE_SRV_CODEC_TYPE_SBC},
#endif
};

extern void bt_app_common_at_cmd_print_report(char *string);
extern uint32_t app_le_audio_convert_sample_freq(uint8_t sampling_freq);
extern void app_le_audio_ucst_register_session_negotiation_callback(app_dongle_session_manager_lea_handle_t *callback);
extern bool app_le_audio_ucst_is_active_group_by_handle(bt_handle_t handle);


static atci_status_t app_dongle_session_manager_at_cmd_handler(atci_parse_cmd_param_t *parse_cmd);

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static void app_dongle_session_manager_get_sample_freq(
    app_dongle_session_manager_lea_session_type_t lea_session_type,
    app_dongle_session_manager_sample_freq_t *sink_sample_freq,
    app_dongle_session_manager_sample_freq_t *source_sample_freq);
static bool app_dongle_session_manager_is_lea_session_type_supported(
    bt_handle_t handle,
    app_dongle_session_manager_lea_session_type_t lea_session_type);
static app_dongle_session_manager_info_t *app_dongle_session_manager_find_lea_conncetion_info(bt_handle_t handle);
static bt_status_t app_dongle_session_manager_handle_lea_session_type(bt_handle_t handle,app_dongle_session_manager_lea_param_t* parameter);
static bt_status_t app_dongle_session_manager_handle_lea_disconnect_ind(bt_handle_t handle);
static bt_status_t app_dongle_session_manager_handle_lea_session_negotiation(bt_handle_t handle,
    app_dongle_session_manager_session_usage_t session_usage,
    app_dongle_session_manager_lea_session_info_t *session_info);

static uint8_t g_app_dongle_session_mgr_lea_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX] = {0,0};

static app_dongle_session_manager_lea_handle_t g_app_dongle_session_mgr_lea_callback = {
    .lea_session_type_noify = app_dongle_session_manager_handle_lea_session_type,
    .lea_session_disconnected = app_dongle_session_manager_handle_lea_disconnect_ind,
    .lea_session_negotiation = app_dongle_session_manager_handle_lea_session_negotiation,
};

static void app_dongle_session_manager_get_sample_freq(
    app_dongle_session_manager_lea_session_type_t lea_session_type,
    app_dongle_session_manager_sample_freq_t *sink_sample_freq,
    app_dongle_session_manager_sample_freq_t *source_sample_freq)
{
    uint16_t source_sampling_frequencies = (uint16_t)(lea_session_type & 0xFFFF);
    uint16_t sink_sampling_frequencies = (uint16_t)((lea_session_type >> 16) & 0xFFFF);
    uint8_t index = 0;
    if(NULL != sink_sample_freq) {
        while (sink_sampling_frequencies) {
            sink_sampling_frequencies = sink_sampling_frequencies >> 1;
            index++;
        }
        *sink_sample_freq = index;
    }

    if(NULL != source_sample_freq) {
        index = 0;
        while (source_sampling_frequencies) {
            source_sampling_frequencies = source_sampling_frequencies >> 1;
            index++;
        }
        *source_sample_freq = index;
    }

}

static bt_handle_t app_dongle_session_manager_get_lea_active_device_handle(void)
{
    uint8_t index = 0;
    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if (BT_HANDLE_INVALID != g_app_dongle_session_manager[index].lea_connection_handle) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (app_le_audio_ucst_is_active_group_by_handle(g_app_dongle_session_manager[index].lea_connection_handle))
#endif
            {
                return g_app_dongle_session_manager[index].lea_connection_handle;
            }
        }

    }
    return BT_HANDLE_INVALID;
}

static bool app_dongle_session_manager_is_lea_session_type_supported(
    bt_handle_t handle,
    app_dongle_session_manager_lea_session_type_t lea_session_type)
{
    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_lea_conncetion_info(handle);
    uint32_t sampling_frequencies = 0;
    if (NULL == p_session_manager) {
        return false;
    }
    sampling_frequencies = p_session_manager->sink_sampling_frequencies;
    sampling_frequencies = (sampling_frequencies << 16) | p_session_manager->source_sampling_frequencies;
    if ((sampling_frequencies & lea_session_type) || (0 == lea_session_type)) {
        return true;
    }
    return false;
}

static void app_dongle_session_manager_print_support_lea_session_type(void)
{
    uint8_t index = 0, i =0;
    char conn_string[60] = {0};
    app_dongle_session_manager_sample_freq_t sink_sample_freq;
    app_dongle_session_manager_sample_freq_t source_sample_freq;

    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if ((BT_HANDLE_INVALID != g_app_dongle_session_manager[index].lea_connection_handle)
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            && app_le_audio_ucst_is_active_group_by_handle(g_app_dongle_session_manager[index].lea_connection_handle)
#endif
            ) {
            snprintf((char *)conn_string, 60, "LEA handle:0x%04x support session type:\r\n", g_app_dongle_session_manager[index].lea_connection_handle);
            bt_app_common_at_cmd_print_report(conn_string);

            for (i = 0; i < sizeof(g_app_dongle_session_mgr_support_session_type_tbl)/sizeof(app_dongle_session_manager_session_type_t); i++) {
                if (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA != g_app_dongle_session_mgr_support_session_type_tbl[i].source_type) {
                    continue;
                }
                if (app_dongle_session_manager_is_lea_session_type_supported(g_app_dongle_session_manager[index].lea_connection_handle, g_app_dongle_session_mgr_support_session_type_tbl[i].session_type)) {
                    app_dongle_session_manager_get_sample_freq(g_app_dongle_session_mgr_support_session_type_tbl[i].session_type, &sink_sample_freq, &source_sample_freq);
                    memset(conn_string, 0, sizeof(conn_string));
                    if (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA == g_app_dongle_session_mgr_support_session_type_tbl[i].session_usage) {
                        snprintf((char *)conn_string, 60, "%d.session usage media:%dHz\r\n", i, (int)app_le_audio_convert_sample_freq(sink_sample_freq));
                    }
                    else {
                        snprintf((char *)conn_string, 60, "%d.session usage communication:DL_%dHz,UL_%dHz\r\n", i, (int)app_le_audio_convert_sample_freq(sink_sample_freq), (int)app_le_audio_convert_sample_freq(source_sample_freq));
                    }
                    bt_app_common_at_cmd_print_report(conn_string);
                }
            }
        }

    }
}


static app_dongle_session_manager_info_t *app_dongle_session_manager_find_lea_conncetion_info(bt_handle_t handle)
{
    uint8_t index = 0;
    //if support edr and LEA both connected
    //addr = get_addr_by_handle(handle)
    //check is same to edr_addr, and return
    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if (handle == g_app_dongle_session_manager[index].lea_connection_handle) {
            return &g_app_dongle_session_manager[index];
        }
    }
    return NULL;
}

static bt_status_t app_dongle_session_manager_handle_lea_session_type(bt_handle_t handle,app_dongle_session_manager_lea_param_t* parameter)
{
    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_lea_conncetion_info(BT_HANDLE_INVALID);
    if (p_session_manager) {
        if (APP_DONGLE_SESSION_MANAGER_SOURCE_NONE == g_app_dongle_session_manager_active_source) {
            g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_LEA;
        }
        p_session_manager->lea_connection_handle = handle;
        p_session_manager->lea_connection_handle_partner = parameter->partner_connection_handle;
        p_session_manager->sink_sampling_frequencies = parameter->sink_sampling_frequencies;
        p_session_manager->source_sampling_frequencies = parameter->source_sampling_frequencies;
        p_session_manager->profile = parameter->profile;
        APPS_LOG_MSGID_I(LOG_TAG"LEA handle:%x partner:%x sink_sampling_frequencies:0x%04x source_sampling_frequencies:0x%04x profile:0x%08x", 5, handle,
            p_session_manager->lea_connection_handle_partner,
            p_session_manager->sink_sampling_frequencies,
            p_session_manager->source_sampling_frequencies,
            p_session_manager->profile);
    }
    else {
        APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager connection is full handle:%x ", 1, handle);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t app_dongle_session_manager_handle_lea_disconnect_ind(bt_handle_t handle)
{
    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_lea_conncetion_info(handle);
    APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager connection is disconnected handle:%x ", 1, handle);
    if (!p_session_manager) {
        return BT_STATUS_SUCCESS;
    }

    p_session_manager->lea_connection_handle = BT_HANDLE_INVALID;
    p_session_manager->lea_connection_handle_partner = BT_HANDLE_INVALID;
    p_session_manager->sink_sampling_frequencies = APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_UNSPECIFIED;
    p_session_manager->source_sampling_frequencies = APP_DONGLE_SESSION_MGR_CODEC_SAMPLING_FREQ_UNSPECIFIED;
    p_session_manager->profile = 0;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (BT_HANDLE_INVALID == app_dongle_session_manager_get_lea_active_device_handle()) {
        if (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_manager_active_source) {
            g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_NONE;
        }
    }
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t app_dongle_session_manager_handle_lea_session_negotiation(bt_handle_t handle,
    app_dongle_session_manager_session_usage_t session_usage,
    app_dongle_session_manager_lea_session_info_t *session_info)
{
    app_dongle_session_manager_lea_session_type_t session_type;
    uint8_t session_id;

    if(BT_HANDLE_INVALID == handle) {
        return BT_STATUS_FAIL;
    }

    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_lea_conncetion_info(handle);
    if ((NULL != p_session_manager) && (NULL != session_info) && (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX > session_usage)) {
        if (APP_DONGLE_SESSION_MANAGER_SOURCE_NONE == g_app_dongle_session_manager_active_source) {
            g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_LEA;
        }
        else if (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA != g_app_dongle_session_manager_active_source) {
            return BT_STATUS_FAIL;
        }
        session_id = g_app_dongle_session_mgr_lea_current_session_id[session_usage];
        if ((session_id >= sizeof(g_app_dongle_session_mgr_support_session_type_tbl)/sizeof(app_dongle_session_manager_session_type_t)) ||
            (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA != g_app_dongle_session_mgr_support_session_type_tbl[session_id].source_type) ||
            (session_usage != g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_usage)) {
            return BT_STATUS_FAIL;
        }
        g_app_dongle_session_manager_current_session_usage = session_usage;
        session_type = g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_type;

        if (!app_dongle_session_manager_is_lea_session_type_supported(handle, session_type)) {
            session_type = APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED;
            //g_app_dongle_session_mgr_lea_current_session_id[session_usage] = 0;
        }

        app_dongle_session_manager_get_sample_freq(session_type, &session_info->sink_sample_freq, &session_info->source_sample_freq);
        session_info->mic_location = APP_DONGLE_SESSION_MGR_MIC_LOCATION_NONE;
        APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager session_negotiation handle:%x session_id:%d session_usage:%d sink_sample_freq:%d source_sample_freq:%d", 5,
            handle,
            session_id,
            session_usage,
            (int)app_le_audio_convert_sample_freq(session_info->sink_sample_freq),
            (int)app_le_audio_convert_sample_freq(session_info->source_sample_freq));
        return BT_STATUS_SUCCESS;
    }
    else {
        APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager session_negotiation fail handle:%x ", 1, handle);
        return BT_STATUS_FAIL;
    }

}
#endif

#ifdef AIR_BT_SOURCE_ENABLE
static uint8_t g_app_dongle_session_mgr_edr_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX] = {0,0};

static app_dongle_session_manager_info_t *app_dongle_session_manager_find_edr_conncetion_info(bt_addr_t *edr_addr)
{
    uint8_t index = 0;
    //if support edr and LEA both connected
    //addr = get_addr_by_handle(handle)
    //check is same to edr_addr, and return
    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if (!memcmp(edr_addr, &g_app_dongle_session_manager[index].edr_addr, sizeof(bt_addr_t))) {
            return &g_app_dongle_session_manager[index];
        }
    }
    return NULL;
}

static app_dongle_session_manager_info_t *app_dongle_session_manager_get_edr_active_devcie_info(void)
{
    uint8_t index = 0;
    bt_addr_t edr_addr_invalid = {0};

    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if (memcmp(&edr_addr_invalid, &g_app_dongle_session_manager[index].edr_addr, sizeof(bt_addr_t))) {
            return &g_app_dongle_session_manager[index];
        }
    }
    return NULL;
}


static bool app_dongle_session_manager_is_edr_session_type_supported(bt_addr_t *edr_addr, app_dongle_session_manager_edr_session_type_t session_type)
{
    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_edr_conncetion_info(edr_addr);
    if (p_session_manager) {
        if ((p_session_manager->edr_session_type & session_type) || (0 == session_type)) {
            return true;
        }
    }

    return false;
}

static void app_dongle_session_manager_print_support_edr_session_type(void)
{
    uint8_t index = 0, i =0;
    char conn_string[60] = {0};
    bt_addr_t edr_addr_invalid = {0};

    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        if (memcmp(&edr_addr_invalid, &g_app_dongle_session_manager[index].edr_addr, sizeof(bt_addr_t))) {
            snprintf((char *)conn_string, 60, "EDR addrType:%d addr:%02x:%02x:%02x:%02x:%02x:%02x support session type:\r\n", g_app_dongle_session_manager[index].edr_addr.type,
                               g_app_dongle_session_manager[index].edr_addr.addr[5],
                               g_app_dongle_session_manager[index].edr_addr.addr[4],
                               g_app_dongle_session_manager[index].edr_addr.addr[3],
                               g_app_dongle_session_manager[index].edr_addr.addr[2],
                               g_app_dongle_session_manager[index].edr_addr.addr[1],
                               g_app_dongle_session_manager[index].edr_addr.addr[0]);
            bt_app_common_at_cmd_print_report(conn_string);

            for (i = 0; i < sizeof(g_app_dongle_session_mgr_support_session_type_tbl)/sizeof(app_dongle_session_manager_session_type_t); i++) {
                if (APP_DONGLE_SESSION_MANAGER_SOURCE_EDR != g_app_dongle_session_mgr_support_session_type_tbl[i].source_type) {
                    continue;
                }
                if (app_dongle_session_manager_is_edr_session_type_supported(&g_app_dongle_session_manager[index].edr_addr, g_app_dongle_session_mgr_support_session_type_tbl[i].session_type)) {
                    memset(conn_string, 0, sizeof(conn_string));
                    if (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA == g_app_dongle_session_mgr_support_session_type_tbl[i].session_usage) {
                        if (BT_SOURCE_SRV_CODEC_TYPE_SBC == g_app_dongle_session_mgr_support_session_type_tbl[i].session_type) {
                            snprintf((char *)conn_string, 60, "%d.session usage media:SBC\r\n", i);
                        }
                        else {
                            snprintf((char *)conn_string, 60, "%d.session usage media:unspecified\r\n", i);
                        }
                    }
                    else {
                        if (BT_SOURCE_SRV_CODEC_TYPE_CVSD == g_app_dongle_session_mgr_support_session_type_tbl[i].session_type) {
                            snprintf((char *)conn_string, 60, "%d.session usage communication:CVSD\r\n", i);
                        }
                        else if (BT_SOURCE_SRV_CODEC_TYPE_MSBC == g_app_dongle_session_mgr_support_session_type_tbl[i].session_type) {
                            snprintf((char *)conn_string, 60, "%d.session usage communication:MSBC\r\n", i);
                        }
                        else {
                            snprintf((char *)conn_string, 60, "%d.session usage communication:unspecified\r\n", i);
                        }
                    }
                    bt_app_common_at_cmd_print_report(conn_string);
                }
            }

        }
    }
}

bt_status_t app_dongle_session_manager_handle_edr_session_type(bt_source_srv_event_t event,void *parameter)
{
    bt_addr_t edr_addr_invalid = {0};
    app_dongle_session_manager_info_t *p_session_manager = NULL;

    if (BT_SOURCE_SRV_EVENT_PROFILE_CONNECTED == event) {
        bt_source_srv_profile_connected_t *p_connected_info = (bt_source_srv_profile_connected_t*)parameter;
        if (NULL == p_connected_info) {
            return BT_STATUS_FAIL;
        }

        //HFP and A2DP report profile connection status separately
        p_session_manager = app_dongle_session_manager_find_edr_conncetion_info(&p_connected_info->peer_address);
        if (NULL == p_session_manager) {
            p_session_manager = app_dongle_session_manager_find_edr_conncetion_info(&edr_addr_invalid);
        }

        if (p_session_manager) {
            if (APP_DONGLE_SESSION_MANAGER_SOURCE_NONE == g_app_dongle_session_manager_active_source) {
                g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_EDR;
            }

            memcpy(&p_session_manager->edr_addr, &p_connected_info->peer_address, sizeof(bt_addr_t));
            p_session_manager->edr_session_type |= p_connected_info->support_codec;
            APPS_LOG_MSGID_I(LOG_TAG"EDR addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x session type:0x%08x", 8,
                        p_connected_info->peer_address.type,
                        p_connected_info->peer_address.addr[5],
                        p_connected_info->peer_address.addr[4],
                        p_connected_info->peer_address.addr[3],
                        p_connected_info->peer_address.addr[2],
                        p_connected_info->peer_address.addr[1],
                        p_connected_info->peer_address.addr[0],
                        p_session_manager->edr_session_type);
        }
        else {
            APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager connection is full addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x ", 7,
                        p_connected_info->peer_address.type,
                        p_connected_info->peer_address.addr[5],
                        p_connected_info->peer_address.addr[4],
                        p_connected_info->peer_address.addr[3],
                        p_connected_info->peer_address.addr[2],
                        p_connected_info->peer_address.addr[1],
                        p_connected_info->peer_address.addr[0]);
            return BT_STATUS_FAIL;
        }

    }
    else if (BT_SOURCE_SRV_EVENT_PROFILE_DISCONNECTED == event) {
        bt_source_srv_profile_disconnected_t *p_disconnected_info = (bt_source_srv_profile_disconnected_t*)parameter;
        if (NULL == p_disconnected_info) {
            return BT_STATUS_FAIL;
        }
        p_session_manager = app_dongle_session_manager_find_edr_conncetion_info(&p_disconnected_info->peer_address);
        if (p_session_manager) {
            if (BT_SOURCE_SRV_TYPE_HFP == p_disconnected_info->type) {
                p_session_manager->edr_session_type &= (~APP_DONGLE_SESSION_MANAGER_HFP_CODEC_MASK);
            }
            else if (BT_SOURCE_SRV_TYPE_A2DP == p_disconnected_info->type) {
                p_session_manager->edr_session_type &= (~APP_DONGLE_SESSION_MANAGER_A2DP_CODEC_MASK);
            }

            if (BT_SOURCE_SRV_CODEC_TYPE_NONE == p_session_manager->edr_session_type) {
                if(APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_manager_active_source) {
                    g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_NONE;
                }
                memset(&p_session_manager->edr_addr, 0, sizeof(bt_addr_t));
            }
        }
        else {
            return BT_STATUS_FAIL;
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t app_dongle_session_manager_handle_edr_session_negotiation(bt_addr_t *edr_addr,
    app_dongle_session_manager_session_usage_t session_usage,
    app_dongle_session_manager_edr_session_info_t *session_info)
{
    app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_find_edr_conncetion_info(edr_addr);
    app_dongle_session_manager_edr_session_type_t session_type = 0;
    uint8_t session_id = 0;
    if (NULL == p_session_manager) {
        return BT_STATUS_FAIL;
    }

    if (APP_DONGLE_SESSION_MANAGER_SOURCE_NONE == g_app_dongle_session_manager_active_source) {
        g_app_dongle_session_manager_active_source = APP_DONGLE_SESSION_MANAGER_SOURCE_EDR;
    }
    else if (APP_DONGLE_SESSION_MANAGER_SOURCE_EDR != g_app_dongle_session_manager_active_source) {
        return BT_STATUS_FAIL;
    }

    if (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX <= session_usage) {
        return BT_STATUS_FAIL;
    }

    if (session_info) {
        session_id = g_app_dongle_session_mgr_edr_current_session_id[session_usage];
        if ((session_id >= sizeof(g_app_dongle_session_mgr_support_session_type_tbl)/sizeof(app_dongle_session_manager_session_type_t)) ||
            (APP_DONGLE_SESSION_MANAGER_SOURCE_EDR != g_app_dongle_session_mgr_support_session_type_tbl[session_id].source_type) ||
            (session_usage != g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_usage)) {
            return BT_STATUS_FAIL;
        }
        g_app_dongle_session_manager_current_session_usage = session_usage;
        session_type = g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_type;

        if (!app_dongle_session_manager_is_edr_session_type_supported(edr_addr, session_type)) {
            session_type = BT_SOURCE_SRV_CODEC_TYPE_NONE;
            //g_app_dongle_session_mgr_edr_current_session_id[session_usage] = 0;
        }
        session_info->session_type = session_type;
        session_info->mic_location = APP_DONGLE_SESSION_MGR_MIC_LOCATION_NONE;
        APPS_LOG_MSGID_I(LOG_TAG"dongle_session_manager session_negotiation addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x session_type:0x%08x", 8,
                edr_addr->type,
                edr_addr->addr[5],
                edr_addr->addr[4],
                edr_addr->addr[3],
                edr_addr->addr[2],
                edr_addr->addr[1],
                edr_addr->addr[0],
                session_type);

    }
    else{
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

#endif

static bool app_dongle_session_manager_restart_session_type(void)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if(APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_manager_active_source) {
        ui_shell_send_event(TRUE,
                        EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_APP_SESSION_MANAGER,
                        APP_DONGLE_SESSION_MGR_RESTART,
                        NULL,
                        0,
                        NULL,
                        60);
        return true;
    }
#endif
#ifdef AIR_BT_SOURCE_ENABLE
    if(APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_manager_active_source) {
        app_dongle_session_manager_session_usage_t session_usage = g_app_dongle_session_manager_current_session_usage;
        bt_source_srv_switch_codec_t switch_codec = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
        };
        app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_get_edr_active_devcie_info();
        uint8_t session_id;
        if (p_session_manager) {
            if (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX > session_usage) {
                session_id = g_app_dongle_session_mgr_edr_current_session_id[session_usage];
                switch_codec.codec = g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_type;
                if (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA == session_usage) {
                    switch_codec.type = BT_SOURCE_SRV_TYPE_A2DP;
                }
                if ((0 == switch_codec.codec) || (switch_codec.codec & p_session_manager->edr_session_type)) {//check support
                    memcpy(&switch_codec.peer_address, &p_session_manager->edr_addr, sizeof(bt_addr_t));
                    bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_SWITCH_CODEC, &switch_codec, sizeof(bt_source_srv_switch_codec_t));
                    return true;
                }
            }

        }
    }
#endif
    return false;
}

static bool app_dongle_session_manager_set_current_session_type(uint8_t session_id)
{
    app_dongle_session_manager_session_usage_t session_usage;

    if (session_id < (sizeof(g_app_dongle_session_mgr_support_session_type_tbl) / sizeof(app_dongle_session_manager_session_type_t))) {
        session_usage = g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_usage;
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_mgr_support_session_type_tbl[session_id].source_type) {
            app_dongle_session_manager_lea_session_type_t session_type = g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_type;
            bt_handle_t handle = app_dongle_session_manager_get_lea_active_device_handle();

            if ((APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX > session_usage) &&
                (BT_HANDLE_INVALID != handle) &&
                app_dongle_session_manager_is_lea_session_type_supported(handle, session_type)) {

                if (g_app_dongle_session_mgr_lea_current_session_id[session_usage] != session_id) {
                    g_app_dongle_session_mgr_lea_current_session_id[session_usage] = session_id;
                    if ((session_usage == g_app_dongle_session_manager_current_session_usage) &&
                        (APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_manager_active_source)) {
                        app_dongle_session_manager_restart_session_type();
                    }
                }
                return true;
            }
        }
#endif
#ifdef AIR_BT_SOURCE_ENABLE
        if (APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_mgr_support_session_type_tbl[session_id].source_type) {
            app_dongle_session_manager_info_t *p_session_manager = app_dongle_session_manager_get_edr_active_devcie_info();
            if ((APP_DONGLE_SESSION_MGR_SESSION_USAGE_MAX > session_usage) &&
                (NULL != p_session_manager) &&
                (p_session_manager->edr_session_type & g_app_dongle_session_mgr_support_session_type_tbl[session_id].session_type)) {
                if (g_app_dongle_session_mgr_edr_current_session_id[session_usage] != session_id) {
                    g_app_dongle_session_mgr_edr_current_session_id[session_usage] = session_id;
                    if ((session_usage == g_app_dongle_session_manager_current_session_usage) &&
                        (APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_manager_active_source)) {
                        app_dongle_session_manager_restart_session_type();
                    }
                }
                return true;
            }
        }
#endif
    }
    return false;
}

static atci_status_t app_dongle_session_manager_at_cmd_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    char *pChar = NULL;
    uint8_t session_id = 0;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            if (0 == memcmp(pChar, "RESTART", 7)) {
                app_dongle_session_manager_restart_session_type();
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }
            else if (0 == memcmp(pChar, "QUERY", 5)) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                app_dongle_session_manager_print_support_lea_session_type();
#endif
#ifdef AIR_BT_SOURCE_ENABLE
                app_dongle_session_manager_print_support_edr_session_type();
#endif
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }
            else if (0 == memcmp(pChar, "SET", 3)) {
                pChar = strchr(pChar, ',');
                pChar++;
                session_id = (uint8_t)strtoul(pChar, NULL, 10);
                if (app_dongle_session_manager_set_current_session_type(session_id)) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }
            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

/**
 * @brief The at command table which used to do connect control opeartion.
 */
static atci_cmd_hdlr_item_t app_dongle_session_manager_at_cmd_table[] = {
    {
        .command_head = "AT+SESSION",    /**< AT command string. */
        .command_hdlr = app_dongle_session_manager_at_cmd_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

void app_dongle_session_manager_init(void)
{
    uint8_t index = 0;
    memset(g_app_dongle_session_manager, 0, APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX * sizeof(app_dongle_session_manager_info_t));
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    for (index = 0; index < APP_DONGLE_SESSION_MANAGER_CONNECTION_MAX; index++) {
        g_app_dongle_session_manager[index].lea_connection_handle = BT_HANDLE_INVALID;
    }
    app_le_audio_ucst_register_session_negotiation_callback(&g_app_dongle_session_mgr_lea_callback);
#endif

    for (index = 0; index < sizeof(g_app_dongle_session_mgr_support_session_type_tbl)/sizeof(app_dongle_session_manager_session_type_t); index++) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if ((APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_mgr_support_session_type_tbl[index].source_type) &&
            (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA == g_app_dongle_session_mgr_support_session_type_tbl[index].session_usage) &&
            (APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED == g_app_dongle_session_mgr_support_session_type_tbl[index].session_type)) {
            g_app_dongle_session_mgr_lea_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA] = index;
        }
        else if ((APP_DONGLE_SESSION_MANAGER_SOURCE_LEA == g_app_dongle_session_mgr_support_session_type_tbl[index].source_type) &&
            (APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION == g_app_dongle_session_mgr_support_session_type_tbl[index].session_usage) &&
            (APP_DONGLE_SESSION_MGR_LEA_SESSION_TYPE_UNSPECIFIED == g_app_dongle_session_mgr_support_session_type_tbl[index].session_type)) {
            g_app_dongle_session_mgr_lea_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION] = index;
        }
#endif
#ifdef AIR_BT_SOURCE_ENABLE
        if ((APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_mgr_support_session_type_tbl[index].source_type) &&
            (APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA == g_app_dongle_session_mgr_support_session_type_tbl[index].session_usage) &&
            (BT_SOURCE_SRV_CODEC_TYPE_NONE == g_app_dongle_session_mgr_support_session_type_tbl[index].session_type)) {
            g_app_dongle_session_mgr_edr_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA] = index;
        }
        else if ((APP_DONGLE_SESSION_MANAGER_SOURCE_EDR == g_app_dongle_session_mgr_support_session_type_tbl[index].source_type) &&
            (APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION == g_app_dongle_session_mgr_support_session_type_tbl[index].session_usage) &&
            (BT_SOURCE_SRV_CODEC_TYPE_NONE == g_app_dongle_session_mgr_support_session_type_tbl[index].session_type)) {
            g_app_dongle_session_mgr_edr_current_session_id[APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION] = index;
        }
#endif
    }

    atci_register_handler(app_dongle_session_manager_at_cmd_table, sizeof(app_dongle_session_manager_at_cmd_table) / sizeof(atci_cmd_hdlr_item_t));
}

#endif

