/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio.h"
#include "app_le_audio_bcst.h"
#include "app_le_audio_bcst_utillity.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_usb.h"

#include "app_dongle_connection_common.h"

#include "ble_bap.h"
#include "ble_bass.h"
#include "ble_tmas_def.h"
#include "ble_pbp.h"
#include "hal_trng.h"

#include "bt_le_audio_msglog.h"
#include "bt_customer_config.h"

#ifdef AIR_LE_AUDIO_TMAP_ENABLE
#include "ble_tmas_def.h"
#endif
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
#include "ble_gmas_def.h"
#endif

#ifdef AIR_LE_AUDIO_BA_ENABLE
#include "app_le_audio_ba.h"
#endif
/**************************************************************************************************
* Define
**************************************************************************************************/
#define METADATA_PROGRAM_INFO_PBP_DEMO     "PBP DEMO"

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
const uint8_t g_lea_bcst_pa_metadata[] = {
    METADATA_LTV_LEN_STREAMING_AUDIO_CONTEXTS,
    METADATA_LTV_TYPE_STREAMING_AUDIO_CONTEXTS,
    (uint8_t)(AUDIO_CONTENT_TYPE_GAME),
    (uint8_t)(AUDIO_CONTENT_TYPE_GAME >> 4),
};

uint8_t g_lea_bcst_pa_metadata_len = sizeof(g_lea_bcst_pa_metadata);
bool g_lea_bcst_pa_is_include_meatadata = false;

uint8_t g_lea_bcst_config_bis_num = 2;

extern app_le_audio_ctrl_t g_lea_ctrl;

extern app_le_audio_bcst_ctrl_t g_lea_bcst_ctrl;

extern app_le_audio_qos_params_t g_lea_bcst_qos_params;

extern const uint32_t g_lea_sdu_interval_tbl[];

extern char g_le_device_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH];
/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void bt_app_common_at_cmd_print_report(char *string);
#if defined(AIR_LE_AUDIO_BIS_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
extern void bt_app_dongle_switch_link_mode_event_callback(uint8_t event, void *param, uint32_t param_len);
#endif
/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static bt_status_t app_le_audio_bcst_config_extend_adv(void)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_bd_addr_ptr_t addr = NULL;   /* Use public address */
#ifdef AIR_LE_AUDIO_PBP_ENABLE
    uint8_t name_len, program_info_len, idx = 0;
#else
    uint8_t idx = 0;
#endif
    uint32_t broadcast_id = 0;

    /* Set extended ADV parameter */
    bt_hci_le_set_ext_advertising_parameters_t ea_param = {0};
    ea_param.advertising_event_properties = 0;
    ea_param.primary_advertising_interval_min = 0x000020;
    ea_param.primary_advertising_interval_max = 0x000030;
    ea_param.primary_advertising_channel_map = 0x07;    /* channel:37, 38, 39 */
    ea_param.own_address_type = BT_ADDR_PUBLIC;
    ea_param.advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_FROM_ALL;
    ea_param.advertising_tx_power = 10;
    ea_param.primary_advertising_phy = BT_HCI_LE_ADV_PHY_1M;
    ea_param.secondary_advertising_phy = BT_HCI_LE_ADV_PHY_2M;
    ea_param.advertisng_SID = 0x08;

    /* Set scan response */
    //bt_gap_le_set_ext_scan_response_data_t scan_rsp_data ={0};
    //scan_rsp_data.data_length = 4;
    //scan_rsp_data.data = &ea[0];
#ifdef AIR_LE_AUDIO_PBP_ENABLE
    uint8_t *bcst_code = app_le_audio_bcst_get_code();
    uint8_t temp_code[BLE_BASS_BROADCAST_CODE_SIZE] ={0};
    uint8_t pbp_features = 0;

    if (g_lea_bcst_qos_params.sampling_freq == CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ)
        pbp_features = BT_PBP_FEATURE_HQ_CONFIGRATION_PRESENT;
    else
        pbp_features = BT_PBP_FEATURE_SQ_CONFIGRATION_PRESENT;

    if (bcst_code && memcmp(bcst_code, temp_code, BLE_BASS_BROADCAST_CODE_SIZE)) pbp_features |= BT_PBP_FEATURE_BROADCAST_STREAM_ENCRYPTED;

    program_info_len = strlen(METADATA_PROGRAM_INFO_PBP_DEMO);
    name_len = strlen(g_le_device_name);
    /* Set extended ADV data */
    bt_gap_le_set_ext_advertising_data_t ea_data = {0};
    ea_data.data_length = 17 + name_len + program_info_len;
#else
    bt_gap_le_set_ext_advertising_data_t ea_data = {0};
    ea_data.data_length = 7;
#endif

#ifdef AIR_LE_AUDIO_TMAP_ENABLE
    ea_data.data_length += 6;
#endif
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    ea_data.data_length += 5;
#endif

    if (NULL == (ea_data.data = pvPortMalloc(ea_data.data_length))) {
        LE_AUDIO_MSGLOG_I("[APP][B] config_extend_adv, malloc fail", 0);
        return ret;
    }

    if (HAL_TRNG_STATUS_OK != hal_trng_get_generated_random_number(&broadcast_id)) {
        LE_AUDIO_MSGLOG_I("[APP][B] generated random number for broadcast_id fail", 0);
    }
#ifdef AIR_LE_AUDIO_PBP_ENABLE
    /* Length */
    ea_data.data[idx++] = 0x05 + program_info_len + 2;
    /* Type */
    ea_data.data[idx++] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
    /* Value */
    ea_data.data[idx++] = BT_PBP_UUID16_PUBLIC_BROADCAST_ANNOUNCEMENTS & 0xFF;
    ea_data.data[idx++] = BT_PBP_UUID16_PUBLIC_BROADCAST_ANNOUNCEMENTS >> 8;
    ea_data.data[idx++] = pbp_features;
    ea_data.data[idx++] = program_info_len + 2;
    ea_data.data[idx++] = program_info_len + 1;
    ea_data.data[idx++] = BT_PBP_METADATA_PROGRAM_INFO;
    memcpy(&ea_data.data[idx], METADATA_PROGRAM_INFO_PBP_DEMO, program_info_len);
    idx += program_info_len;
#endif
    /* Length */
    ea_data.data[idx++] = 0x06;
    /* Type */
    ea_data.data[idx++] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
    /* Value */
    ea_data.data[idx++] = (BT_BAP_UUID16_BROADCAST_AUDIO_ANNOUNCEMENTS_SERVICE & 0xFF);
    ea_data.data[idx++] = ((BT_BAP_UUID16_BROADCAST_AUDIO_ANNOUNCEMENTS_SERVICE & 0xFF00) >> 8);
    memcpy(&ea_data.data[idx], &broadcast_id, 3);
    app_le_audio_bcst_set_id(broadcast_id);
    idx += 3;
#ifdef AIR_LE_AUDIO_TMAP_ENABLE
    /* Length */
    ea_data.data[idx++] = 0x05;
    /* Type */
    ea_data.data[idx++] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
    /* Value */
    ea_data.data[idx++] = (BT_SIG_UUID16_TMAS & 0x00FF);
    ea_data.data[idx++] = ((BT_SIG_UUID16_TMAS & 0xFF00) >> 8);
    ea_data.data[idx++] = (BLE_TMAP_ROLE_MASK_BMS & 0x00FF);
    ea_data.data[idx++] = ((BLE_TMAP_ROLE_MASK_BMS & 0xFF00) >> 8);
#endif
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    /* Length */
    ea_data.data[idx++] = 0x04;
    /* Type */
    ea_data.data[idx++] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
    /* Value */
    ea_data.data[idx++] = (BT_SIG_UUID16_GMAS & 0x00FF);
    ea_data.data[idx++] = ((BT_SIG_UUID16_GMAS & 0xFF00) >> 8);
    ea_data.data[idx++] = BLE_GMAP_ROLE_MASK_BGS;
#endif
#ifdef AIR_LE_AUDIO_PBP_ENABLE
    /* Length */
    ea_data.data[idx++] = name_len + 1;
    /* Type */
    ea_data.data[idx++] = BT_PBP_BROADCAST_NAME;
    /* Value */
    if (name_len) {
        memcpy(&ea_data.data[idx], g_le_device_name, name_len);
    }
#endif
    /* Config extended ADV */
    ret = bt_gap_le_config_extended_advertising(APP_LE_AUDIO_BCST_ADV_HANDLE, addr, &ea_param, &ea_data, NULL);
    LE_AUDIO_MSGLOG_I("[APP][B] config_extend_adv, ret:%x", 1, ret);

    vPortFree(ea_data.data);
    return ret;
}

static uint8_t app_le_audio_bcst_get_periodic_adv_data_len(void)
{
    uint8_t pa_len = 42;

#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    if (g_lea_bcst_qos_params.sdu_size == 190 || g_lea_bcst_qos_params.sdu_size == 160) {
        pa_len += 3;
    }
#endif

    if (true == g_lea_bcst_pa_is_include_meatadata) {
        pa_len += g_lea_bcst_pa_metadata_len;
    }

    if (g_lea_bcst_config_bis_num == 1) {
        g_lea_bcst_ctrl.bis_num = g_lea_bcst_config_bis_num;
        pa_len -= 8;
    }

    return pa_len;
}

static uint8_t app_le_audio_bcst_get_periodic_adv_data(uint8_t *p_pa, uint8_t pa_len)
{
    uint8_t acc_len = 0;
    uint8_t pa_level_3_len = 0;

    uint8_t pa_level_1[] = {
        41,     /* Length */
        0x16,   /* Type */
        /* Value */

        /* Basic Audio Announcement Service UUID */
        (uint8_t)(BT_BAP_UUID16_BASIC_AUDIO_ANNOUNCEMENTS_SERVICE),
        (uint8_t)(BT_BAP_UUID16_BASIC_AUDIO_ANNOUNCEMENTS_SERVICE >> 8),

        /* Level 1 */
        0x40, 0x9C, 0x00,   /* Presentation_Delay */
        0x01,               /*Num_Subgroups*/
    };

    uint8_t pa_level_2[] = {
        /*Level 2*/
        0x02, /*Num_BIS*/
        0x06, 0x00, 0x00, 0x00, 0x00,   /* Codec_ID */
        0x0A,                           /* Codec_Specific_Configuration_Length */
        0x02, CODEC_CONFIGURATION_TYPE_SAMPLING_FREQUENCY, g_lea_bcst_qos_params.sampling_freq,
        0x02, CODEC_CONFIGURATION_TYPE_FRAME_DURATIONS, (uint8_t)(g_lea_bcst_qos_params.sdu_interval),
        0x03, CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME, (uint8_t)(g_lea_bcst_qos_params.sdu_size), (uint8_t)(g_lea_bcst_qos_params.sdu_size >> 8),
        0x00,                           /* Metadata_Length */
    };

    uint8_t pa_level_3[] = {
        /* Level 3 */
        0x01,           /* BIS_index */
        0x06,           /* Codec_Specific_Configuration_Length */
        0x05, CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION, (uint8_t)(AUDIO_LOCATION_FRONT_LEFT), 0, 0, 0,
        0x02,           /* BIS_index */
        0x06,           /* Codec_Specific_Configuration_Length */
        0x05, CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION, (uint8_t)(AUDIO_LOCATION_FRONT_RIGHT), 0, 0, 0,
    };

    if (pa_len != app_le_audio_bcst_get_periodic_adv_data_len()) {
        LE_AUDIO_MSGLOG_E("[APP][B] get_periodic_adv_data, length not match, len:0x%x / 0x%x", 2, pa_len, app_le_audio_bcst_get_periodic_adv_data_len());
        return 0;
    }

    memcpy(&p_pa[0], pa_level_1, sizeof(pa_level_1));
    acc_len += sizeof(pa_level_1);

    if (g_lea_bcst_ctrl.bis_num == 1) {
        pa_level_2[0] = g_lea_bcst_ctrl.bis_num;
    }

    memcpy(&p_pa[acc_len], pa_level_2, sizeof(pa_level_2));
    acc_len += sizeof(pa_level_2);

#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    if (g_lea_bcst_qos_params.sdu_size == 190 || g_lea_bcst_qos_params.sdu_size == 160) {
        uint8_t lc3plus_cbr_id[AUDIO_CODEC_ID_SIZE] = CODEC_ID_LC3PLUS_CBR;
        uint8_t tmp_len = sizeof(pa_level_1), lc3plus_codec_len = CODEC_CONFIGURATION_LEN_LC3PLUSHR_FRAME_DURATION + 1;

        p_pa[0] += lc3plus_codec_len;
        memcpy(&p_pa[tmp_len + 1], lc3plus_cbr_id, AUDIO_CODEC_ID_SIZE);
        p_pa[tmp_len + 6] += lc3plus_codec_len;
        p_pa[acc_len - 1] = CODEC_CONFIGURATION_LEN_LC3PLUSHR_FRAME_DURATION;
        p_pa[acc_len] = CODEC_CONFIGURATION_TYPE_LC3PLUSHR_FRAME_DURATION;
        p_pa[acc_len + 1] = FRAME_DURATIONS_LC3PLUS_10_MS;
        p_pa[acc_len + 2] = 0x00;
        acc_len += lc3plus_codec_len;
    }
#endif

    if (true == g_lea_bcst_pa_is_include_meatadata) {
        /* GAME Metadata */
        p_pa[0] += g_lea_bcst_pa_metadata_len;
        p_pa[acc_len - 1] = g_lea_bcst_pa_metadata_len;
        memcpy(&p_pa[acc_len], g_lea_bcst_pa_metadata, g_lea_bcst_pa_metadata_len);
        acc_len += g_lea_bcst_pa_metadata_len;
        /* GAME Presentation delay (10000) */
        p_pa[4] = 0x10;
        p_pa[5] = 0x27;
        p_pa[6] = 0x00;
    }

    pa_level_3_len = sizeof(pa_level_3);
    if (g_lea_bcst_ctrl.bis_num == 1) {
        p_pa[0] -= 8;
        pa_level_3_len -= 8;
    }

    memcpy(&p_pa[acc_len], pa_level_3, pa_level_3_len);
    acc_len += pa_level_3_len;

    LE_AUDIO_MSGLOG_I("[APP][B] get_periodic_adv_data, acc_len:0x%x, pa_len:0x%x", 2, acc_len, pa_len);
    return acc_len;
}

bt_status_t app_le_audio_bcst_config_periodic_adv(void)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_hci_le_set_periodic_advertising_parameters_t pa_param = {0};
    bt_gap_le_set_periodic_advertising_data_t pa_data;
    uint8_t pa_len = app_le_audio_bcst_get_periodic_adv_data_len();
    uint8_t *p_pa = pvPortMalloc(pa_len);

    if (NULL == p_pa) {
        LE_AUDIO_MSGLOG_E("[APP][B] config_periodic_adv, no memory, len:0x%x", 1, pa_len);
        return ret;
    }

    app_le_audio_bcst_get_periodic_adv_data(p_pa, pa_len);

    memset(&pa_data, 0, sizeof(bt_gap_le_set_periodic_advertising_data_t));

    pa_param.interval_min = 0x30;
    pa_param.interval_max = 0x30;
    pa_param.properties = 0;

    pa_data.data_length = (p_pa[0] + 1);
    pa_data.data = &p_pa[0];

    /* Config periodic ADV */
    ret = bt_gap_le_config_periodic_advertising(APP_LE_AUDIO_BCST_ADV_HANDLE, &pa_param, &pa_data);
    vPortFree(p_pa);
    LE_AUDIO_MSGLOG_I("[APP][B] config_periodic_adv, ret:0x%x", 1, ret);

    return ret;
}

static bt_status_t app_le_audio_bcst_enable_extended_adv(bt_hci_enable_t enable)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_hci_le_set_ext_advertising_enable_t ea_enable;

    ea_enable.enable = enable;
    ea_enable.duration = 0;
    ea_enable.max_ext_advertising_evts = 0;

    /* Enable extended ADV */
    ret = bt_gap_le_enable_extended_advertising(APP_LE_AUDIO_BCST_ADV_HANDLE, &ea_enable);

    if (BT_HCI_ENABLE == enable) {
        LE_AUDIO_MSGLOG_I("[APP][B] start_extended_adv, ret:%x", 1, ret);
    } else {
        LE_AUDIO_MSGLOG_I("[APP][B] stop_extended_adv, ret:%x", 1, ret);
    }

    return ret;
}

static bt_status_t app_le_audio_bcst_enable_periodic_adv(bt_hci_enable_t enable)
{
    bt_status_t ret = BT_STATUS_FAIL;

    /* Enable periodic ADV */
    ret = bt_gap_le_enable_periodic_advertising(APP_LE_AUDIO_BCST_ADV_HANDLE, enable);

    if (BT_HCI_ENABLE == enable) {
        LE_AUDIO_MSGLOG_I("[APP][B] start_periodic_adv, ret:%x", 1, ret);
    } else {
        LE_AUDIO_MSGLOG_I("[APP][B] stop_periodic_adv, ret:%x", 1, ret);
    }

    return ret;
}

static bt_status_t app_le_audio_bcst_create_big(void)
{
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t *bcst_code = app_le_audio_bcst_get_code();
    uint8_t temp_code[BLE_BASS_BROADCAST_CODE_SIZE] ={0};

    /* Create BIG */
    bt_hci_le_create_big_t big_param = {0};

    big_param.big_handle = APP_LE_AUDIO_BCST_BIG_HANDLE;
    big_param.adv_handle = APP_LE_AUDIO_BCST_ADV_HANDLE;
    big_param.num_of_bis = g_lea_bcst_ctrl.bis_num;
    big_param.sdu_interval = g_lea_sdu_interval_tbl[g_lea_bcst_qos_params.sdu_interval];
    big_param.max_sdu = g_lea_bcst_qos_params.sdu_size;  /* default:100 */
    big_param.max_transport_latency =  g_lea_bcst_qos_params.latency;
    big_param.retransmission_number = g_lea_bcst_qos_params.rtn;
    big_param.phy = 0x02;
    big_param.packing = 0x01;
    big_param.framing = 0x00;
    big_param.encryption = 0x00;

    if (NULL != bcst_code) {
        if (memcmp(bcst_code, temp_code, BLE_BASS_BROADCAST_CODE_SIZE)) {
            big_param.encryption = 0x01;
            memcpy(big_param.broadcast_code, bcst_code, BLE_BASS_BROADCAST_CODE_SIZE);
        }
    }

    ret = bt_gap_le_create_big(&big_param);
    LE_AUDIO_MSGLOG_I("[APP][B] create_big, ret:%x", 1, ret);

    return ret;
}

static bt_status_t app_le_audio_bcst_terminate_big(void)
{
    bt_status_t ret = BT_STATUS_FAIL;

    /* Terminate BIG */
    bt_hci_le_terminate_big_t big_param = {0};

    big_param.big_handle = APP_LE_AUDIO_BCST_BIG_HANDLE;
    big_param.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;

    ret = bt_gap_le_terminate_big(&big_param);
    LE_AUDIO_MSGLOG_I("[APP][B] terminate_big, ret:%x", 1, ret);

    return ret;
}

static bool app_le_audio_bcst_change_state(app_le_audio_bcst_state_t new_state)
{
    if (APP_LE_AUDIO_BCST_STATE_MAX <= new_state) {
        LE_AUDIO_MSGLOG_I("[APP][B] change_state, invlid new_state:%x", 1, new_state);
        return false;
    }

    LE_AUDIO_MSGLOG_I("[APP][B] change_state, curr:%x new:%x", 2, g_lea_bcst_ctrl.curr_state, new_state);

    switch (new_state) {
        case APP_LE_AUDIO_BCST_STATE_IDLE: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM == g_lea_bcst_ctrl.curr_state) {
                app_le_audio_bcst_reset_info();
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_IDLE;
                //g_lea_bcst_ctrl.streaming_mode = APP_LE_AUDIO_STREAMING_MODE_NONE;

                if (g_lea_bcst_ctrl.is_need_restart) {
                    LE_AUDIO_MSGLOG_I("[APP][B] Broadcast retart", 0);
                    app_le_audio_bcst_start();
                    return true;
                }

                LE_AUDIO_MSGLOG_I("[APP][B] Broadcast stopped", 0);
                bt_app_common_at_cmd_print_report("Broadcast stopped\r\n");

                g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;

                /* [Switch streaming mode] BCST -> UCST*/
                if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.next_mode) {
                    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                    app_le_audio_start_unicast();
                }else if (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode) { //for CM stop LEA source;
                    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                    app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_BIS;
                    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
                }

                return true;
            }
            if (APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM == g_lea_bcst_ctrl.curr_state) {
                app_le_audio_bcst_reset_info();
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_IDLE;

                return true;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING <= g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                return false;
            }
            if (APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM <= g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] start broadcast in progress", 0);
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_CONFIG_EXTENDED_ADV:
        case APP_LE_AUDIO_BCST_STATE_ENABLE_EXTENDED_ADV: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM;
                app_le_audio_close_audio_transmitter();
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_CONFIG_PERIODIC_ADV:
        case APP_LE_AUDIO_BCST_STATE_ENABLE_PERIODIC_ADV: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_DISABLE_EXTENDED_ADV;
                app_le_audio_bcst_enable_extended_adv(BT_HCI_DISABLE);
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_CREATE_BIG: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_DISABLE_PERIODIC_ADV;
                app_le_audio_bcst_enable_periodic_adv(BT_HCI_DISABLE);
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_SETUP_ISO_DATA_PATH: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_TERMINATED_BIG;
                app_le_audio_bcst_terminate_big();
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_STREAMING: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_REMOVE_ISO_DATA_PATH;
                g_lea_bcst_ctrl.sub_state = g_lea_bcst_ctrl.bis_num;  /* sub_state (bis_num): remove all iso data path */
                app_le_audio_remove_iso_data_path(g_lea_bcst_ctrl.bis_handle[0], BT_GAP_LE_ISO_DATA_PATH_DIRECTION_INPUT);
                return false;
            }
            break;
        }
        case APP_LE_AUDIO_BCST_STATE_STOP_STREAMING: {
            if (APP_LE_AUDIO_BCST_STATE_IDLE == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop_broadcast, not streaming now!", 0);
                if (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode) {
                    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                    g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
                    app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_BIS;
                    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
                }
                return true;
            }
            if (APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM;
                app_le_audio_close_audio_transmitter();
                return false;
            }
            if (APP_LE_AUDIO_BCST_STATE_STREAMING == g_lea_bcst_ctrl.curr_state) {
                g_lea_bcst_ctrl.curr_state = APP_LE_AUDIO_BCST_STATE_REMOVE_ISO_DATA_PATH;
                g_lea_bcst_ctrl.sub_state = g_lea_bcst_ctrl.bis_num;  /* sub_state (bis_num): remove all iso data path */
                app_le_audio_remove_iso_data_path(g_lea_bcst_ctrl.bis_handle[0], BT_GAP_LE_ISO_DATA_PATH_DIRECTION_INPUT);
                return false;
            }
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING <= g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                return false;
            }
            g_lea_bcst_ctrl.curr_state = new_state;
            return false;
        }
        case APP_LE_AUDIO_BCST_STATE_REMOVE_ISO_DATA_PATH:
        case APP_LE_AUDIO_BCST_STATE_TERMINATED_BIG:
        case APP_LE_AUDIO_BCST_STATE_DISABLE_PERIODIC_ADV:
        case APP_LE_AUDIO_BCST_STATE_DISABLE_EXTENDED_ADV:
        case APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM: {
            if (APP_LE_AUDIO_BCST_STATE_STOP_STREAMING == g_lea_bcst_ctrl.curr_state) {
                LE_AUDIO_MSGLOG_I("[APP][B] stop broadcast in progress", 0);
                return false;
            }
            break;
        }
    }

    if (new_state == (g_lea_bcst_ctrl.curr_state + 1)) {
        g_lea_bcst_ctrl.curr_state = new_state;
        return true;
    }

    LE_AUDIO_MSGLOG_I("[APP][B] change_state, fail", 0);
    return false;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_bcst_handle_setup_iso_data_path_cnf(bt_status_t ret, bt_gap_le_setup_iso_data_path_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_SETUP_ISO_DATA_PATH_CNF, bis_handle:%x ret:%x sub_state:%x", 3, cnf->handle, ret, g_lea_bcst_ctrl.sub_state);

    /* sub_state (ind->num_bis): setup iso data path for each BIS */
    if (0 < g_lea_bcst_ctrl.sub_state) {
        g_lea_bcst_ctrl.sub_state--;
    }

    if (0 == g_lea_bcst_ctrl.sub_state) {
        app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_BIS;
        LE_AUDIO_MSGLOG_I("[APP][B] Broadcasting...", 0);
        app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_STREAMING);
        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_SUCCESS, &lea_mode);
        bt_app_common_at_cmd_print_report("Broadcasting...\r\n");
#ifdef AIR_LE_AUDIO_BA_ENABLE
        app_le_audio_ba_start(APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE);
#endif
    } else {
        uint8_t idx = (g_lea_bcst_ctrl.bis_num - g_lea_bcst_ctrl.sub_state);
        app_le_audio_setup_iso_data_path(g_lea_bcst_ctrl.bis_handle[idx], BT_GAP_LE_ISO_DATA_PATH_DIRECTION_INPUT, (idx + 1));

    }
}

void app_le_audio_bcst_handle_remove_iso_data_path_cnf(bt_status_t ret, bt_gap_le_remove_iso_data_path_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_REMOVE_ISO_DATA_PATH_CNF, bis_handle:%x ret:%x sub_state:%x", 3, cnf->handle, ret, g_lea_bcst_ctrl.sub_state);
    /* sub_state (bis_num): remove all iso data path */
    if (0 < g_lea_bcst_ctrl.sub_state) {
        g_lea_bcst_ctrl.sub_state--;
    }

    if (0 == g_lea_bcst_ctrl.sub_state) {
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_TERMINATED_BIG)) {
            app_le_audio_bcst_terminate_big();
        }

    } else {
        uint8_t idx = (g_lea_bcst_ctrl.bis_num - g_lea_bcst_ctrl.sub_state);
        app_le_audio_remove_iso_data_path(g_lea_bcst_ctrl.bis_handle[idx], BT_GAP_LE_ISO_DATA_PATH_DIRECTION_INPUT);

    }
}

void app_le_audio_bcst_handle_config_extended_advertising_cnf(bt_status_t ret, bt_gap_le_config_extended_advertising_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_CONFIG_EXTENDED_ADVERTISING_CNF, adv_handle:%x ret:%x ", 2, cnf->handle, ret);

    /* Start BCST next state */
    if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_ENABLE_EXTENDED_ADV)) {
        app_le_audio_bcst_enable_extended_adv(BT_HCI_ENABLE);
    }
}

void app_le_audio_bcst_handle_enable_extended_advertising_cnf(bt_status_t ret, bt_gap_le_enable_extended_advertising_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_ENABLE_EXTENDED_ADVERTISING_CNF, adv_handle:%x ret:%x enable:%x", 3, cnf->handle, ret, cnf->enable);

    if (BT_HCI_ENABLE == cnf->enable) {
        /* Start BCST next state */
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_CONFIG_PERIODIC_ADV)) {
            app_le_audio_bcst_config_periodic_adv();
        }

    } else {
        /*BT_HCI_DISABLE */
        /* Stop BCST next state */
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM)) {
            app_le_audio_close_audio_transmitter();
        }
    }
}

void app_le_audio_bcst_handle_config_periodic_advertising_cnf_t(bt_status_t ret, bt_gap_le_config_periodic_advertising_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_CONFIG_PERIODIC_ADVERTISING_CNF, adv_handle:%x ret:%x", 2, cnf->handle, ret);

    /* Start BCST next state */
    if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_ENABLE_PERIODIC_ADV)) {
        app_le_audio_bcst_enable_periodic_adv(BT_HCI_ENABLE);
    }
}

void app_le_audio_bcst_handle_enable_periodic_advertising_cnf(bt_status_t ret, bt_gap_le_enable_periodic_advertising_cnf_t *cnf)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_ENABLE_PERIODIC_ADVERTISING_CNF, adv_handle:%x ret:%x enable:%x", 3, cnf->handle, ret, cnf->enable);

    if (BT_HCI_ENABLE == cnf->enable) {
        /* Start BCST next state */
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_CREATE_BIG)) {
            app_le_audio_bcst_create_big();
        }

    } else {
        /*BT_HCI_DISABLE */
        /* Stop BCST next state */
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_DISABLE_EXTENDED_ADV)) {
            app_le_audio_bcst_enable_extended_adv(BT_HCI_DISABLE);
        }
    }
}

void app_le_audio_bcst_handle_big_established_ind(bt_status_t ret, bt_gap_le_big_established_ind_t *ind)
{
    uint8_t i;

    LE_AUDIO_MSGLOG_I("[APP][B] LE_BIG_ESTABLISHED_IND, big_handle:%x ret:%x num_bis:%d", 3, ind->big_handle, ind->status, ind->num_bis);

    for (i = 0; i < ind->num_bis; i++) {
        LE_AUDIO_MSGLOG_I("[APP][B] LE_BIG_ESTABLISHED_IND, [%x] bis_handle:%x", 2, i, ind->connection_handle_list[i]);
        g_lea_bcst_ctrl.bis_handle[i] = ind->connection_handle_list[i];
    }
    g_lea_bcst_ctrl.bis_num = ind->num_bis;

    LE_AUDIO_MSGLOG_I("[APP][B] BIG config info, sampling_freq:0x%x, sdu_interval:0x%x, sdu_size:%d, latency:0x%x, rtn: 0x%x"
        , 5, g_lea_bcst_qos_params.sampling_freq, g_lea_bcst_qos_params.sdu_interval, g_lea_bcst_qos_params.sdu_size, g_lea_bcst_qos_params.latency, g_lea_bcst_qos_params.rtn);

    /* Start BCST next state */
    if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_SETUP_ISO_DATA_PATH)) {
        g_lea_bcst_ctrl.sub_state = ind->num_bis;    /* sub_state (ind->num_bis): setup iso data path for each BIS */
        app_le_audio_setup_iso_data_path(ind->connection_handle_list[0], BT_GAP_LE_ISO_DATA_PATH_DIRECTION_INPUT, 0x01);
    }
}

void app_le_audio_bcst_handle_big_terminated_ind(bt_status_t ret, bt_gap_le_big_terminated_ind_t *ind)
{
    LE_AUDIO_MSGLOG_I("[APP][B] LE_BIG_TERMINATED_IND, big_handle:%x reason:%x", 2, ind->big_handle, ind->reason);

    /* Stop BCST next state */
    if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_DISABLE_PERIODIC_ADV)) {
        app_le_audio_bcst_enable_periodic_adv(BT_HCI_DISABLE);
    }
}

void app_le_audio_bcst_open_audio_transmitter_cb(void)
{
    LE_AUDIO_MSGLOG_I("[APP][B] open_audio_transmitter_cb, state:%x", 1, g_lea_bcst_ctrl.curr_state);

    if (APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM == g_lea_bcst_ctrl.curr_state) {
        if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_CONFIG_EXTENDED_ADV)) {
            app_le_audio_bcst_config_extend_adv();
        }
    }
}

void app_le_audio_bcst_close_audio_transmitter_cb(void)
{
    LE_AUDIO_MSGLOG_I("[APP][B] close_audio_transmitter_cb, state:%x", 1, g_lea_bcst_ctrl.curr_state);

    if (APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM == g_lea_bcst_ctrl.curr_state) {
        app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_IDLE);
    }
}

bool app_le_audio_bcst_is_streaming(void)
{
    if (APP_LE_AUDIO_BCST_STATE_STREAMING == g_lea_bcst_ctrl.curr_state) {
        return true;
    }
    return false;
}

void app_le_audio_bcst_start(void)
{
    if (app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM)) {
        LE_AUDIO_MSGLOG_I("[APP][B] start", 0);
        g_lea_bcst_ctrl.sub_state = 0;
        app_le_audio_bcst_reset_info();
        uint8_t streaming_port = app_le_audio_get_streaming_port();

        if (BT_STATUS_SUCCESS != app_le_audio_open_audio_transmitter(false, streaming_port)) {
            app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_IDLE);
        }
    }
}

bool app_le_audio_bcst_stop(bool restart)
{
    g_lea_bcst_ctrl.is_need_restart = restart;
    LE_AUDIO_MSGLOG_I("[APP][B] stop, state:%x, is_need_restart:%d", 2, g_lea_bcst_ctrl.curr_state, g_lea_bcst_ctrl.is_need_restart);

    return app_le_audio_bcst_change_state(APP_LE_AUDIO_BCST_STATE_STOP_STREAMING);
}


void app_le_audio_bcst_init(void)
{
    app_le_audio_bcst_qos_params_db_t bcst_qos_params_db = {48, 11, 1};

    if (app_le_audio_bcst_read_qos_params_nvkey(&bcst_qos_params_db) == BT_STATUS_SUCCESS){
        app_le_audio_bcst_set_qos_params(bcst_qos_params_db.sampling_rate, bcst_qos_params_db.sel_setting, bcst_qos_params_db.high_reliability);
    }
    app_le_audio_bcst_reset_info();
}

#endif  /* AIR_LE_AUDIO_ENABLE */

