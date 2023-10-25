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

#include "FreeRTOS.h"

#include "nvkey.h"
#include "nvkey_id_list.h"

#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "timers.h"

#include "bt_device_manager_le.h"

#include "app_le_audio.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#include "app_le_audio_ccp_call_control_server.h"
#include "app_le_audio_nvkey_struct.h"
#include "bt_le_audio_util_nvkey_struct.h"

#include "ble_csip.h"
#include "ble_bap_client.h"
#include "bt_le_audio_msglog.h"
/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_DISCONNET_UNICAST_DELAY_TIME    (5000)  /* unit: ms */

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/

app_le_audio_ucst_ctrl_t g_lea_ucst_ctrl;


app_le_audio_ucst_link_info_t g_lea_ucst_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];

uint8_t g_lea_ucst_qos_params_selected;

app_le_audio_qos_params_t g_lea_ucst_qos_params_spk_0 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ,    /* sampling_freq */
    60,                                         /* sdu_size */
    SDU_INTERVAL_7P5_MS,                        /* sdu_interval */
    64,                                         /* bitrate */
    2,                                          /* rtn */
    8,                                          /* latency */
};

app_le_audio_qos_params_t g_lea_ucst_qos_params_mic_0 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ,    /* sampling_freq */
    60,                                         /* sdu_size */
    SDU_INTERVAL_7P5_MS,                        /* sdu_interval */
    64,                                         /* bitrate */
    2,                                          /* rtn */
    8,                                          /* latency */
};

app_le_audio_qos_params_t g_lea_ucst_qos_params_spk_1 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,    /* sampling_freq */
    120,                                        /* sdu_size */
    SDU_INTERVAL_10_MS,                         /* sdu_interval */
    96,                                         /* bitrate */
    13,                                         /* rtn */
    100,                                        /* latency */
};

static const app_le_audio_qos_params_tbl_t g_lea_ucst_qos_params_tbl[] = {
/*  sdu_size    sdu_interval        bitrate low_rtn low_latency high_rtn   high_latency*/
    {30,        SDU_INTERVAL_7P5_MS,   32,      2,      8,       13,        75},       /* 0 16_1 */
    {45,        SDU_INTERVAL_7P5_MS,   48,      2,      8,       13,        75},       /* 1 24_1 */
    {60,        SDU_INTERVAL_7P5_MS,   64,      2,      8,       13,        75},       /* 2 32_1 */
    {75,        SDU_INTERVAL_7P5_MS,   80,      5,      15,      13,        75},       /* 3 48_1 */
    {90,        SDU_INTERVAL_7P5_MS,   96,      5,      15,      13,        75},       /* 4 48_3 */
    {117,       SDU_INTERVAL_7P5_MS,   124.8,   5,      15,      13,        75},       /* 5 48_5 */
    {30,        SDU_INTERVAL_10_MS,    24,      2,      10,      13,        95},       /* 6 8_2 */
    {40,        SDU_INTERVAL_10_MS,    32,      2,      10,      13,        95},       /* 7 16_2 */
    {60,        SDU_INTERVAL_10_MS,    48,      2,      10,      13,        95},       /* 8 24_2 */
    {80,        SDU_INTERVAL_10_MS,    64,      2,      10,      13,        95},       /* 9 32_2 */
    {100,       SDU_INTERVAL_10_MS,    80,      5,      20,      13,        95},       /* 10 48_2 */
    {120,       SDU_INTERVAL_10_MS,    96,      5,      20,      13,       100},       /* 11 48_4 */
    {155,       SDU_INTERVAL_10_MS,    124,     5,      20,      13,       100},       /* 12 48_6 */
    {75,        SDU_INTERVAL_7P5_MS,   80,      2,       8,      2,          8},       /* 13, HQ gaming audio/ HQ gaming audio and voice(DL) */
    {126,       SDU_INTERVAL_7P5_MS,   134.4,   2,       8,      2,          8},       /* 14, LL gaming audio */
    {60,        SDU_INTERVAL_7P5_MS,   64,      2,       8,      2,          8},       /* 15, HQ gaming audio and voice(DL) */
    {26,        SDU_INTERVAL_7P5_MS,   27.734,  2,       8,      13,        75},       /* 16 8_1 */
};

#define APP_LE_AUDIO_UCST_QOS_PARAMS_TBL_COUNT (sizeof(g_lea_ucst_qos_params_tbl)/sizeof(app_le_audio_qos_params_tbl_t))

extern const uint32_t g_lea_sdu_interval_tbl[];

app_le_audio_ucst_cig_params_test_t *g_lea_ucst_cig_params_test = NULL;

uint8_t g_lea_ucst_ccid_list_size = 0;

uint8_t g_lea_ucst_ccid_list[APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE] = {0};

static TimerHandle_t g_lea_ucst_delay_disconnect_timer = NULL;

uint8_t g_lea_ucst_test_mode_flag = 0;


/**************************************************************************************************
* Prototype
**************************************************************************************************/

extern bt_status_t le_audio_notify_mic_mute(bt_handle_t handle, bool mic_mute);
/**************************************************************************************************
* Static Functions
**************************************************************************************************/

/**************************************************************************************************
* Public Functions
**************************************************************************************************/



void app_le_audio_ucst_set_cig_parameter_test(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval)
{
    if (NULL == g_lea_ucst_cig_params_test) {
        if (NULL == (g_lea_ucst_cig_params_test = (app_le_audio_ucst_cig_params_test_t *)pvPortMalloc(sizeof(app_le_audio_ucst_cig_params_test_t)))) {
            LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameter_test, malloc fail", 0);
            return;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] set_cig_parameter_test, bn:%d nse:%d ft:%d iso_interval:%d", 4, bn, nse, ft, iso_interval);
    g_lea_ucst_cig_params_test->bn = bn;
    g_lea_ucst_cig_params_test->nse = nse;
    g_lea_ucst_cig_params_test->ft = ft;
    g_lea_ucst_cig_params_test->iso_interval = iso_interval;
}

void app_le_audio_ucst_set_qos_params(uint8_t sampling_rate, uint8_t sel_setting, uint8_t target_latency, app_le_audio_ucst_stream_port_t port)
{
    uint8_t sampling_freq;
    app_le_audio_qos_params_t *p_qos_params = NULL;

    LE_AUDIO_MSGLOG_I("[APP][U] set_qos_params, sampling_rate:%d sel_setting:%d target_latency:%x port:%x", 4, sampling_rate, sel_setting, target_latency, port);

    if (APP_LE_AUDIO_UCST_QOS_PARAMS_TBL_COUNT <= sel_setting) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_qos_params, invalid sel_setting:%d max:%d", 2, sel_setting, APP_LE_AUDIO_UCST_QOS_PARAMS_TBL_COUNT);
        return;
    }

    g_lea_ucst_qos_params_selected = target_latency;

    if (0 == g_lea_ucst_qos_params_selected) {
        g_lea_ucst_qos_params_selected = 0x01;//Target low latency
    }
    //0x02: Target banlanced latency and reliablity
    if (0x03 < g_lea_ucst_qos_params_selected) {
        g_lea_ucst_qos_params_selected = 0x03;//Target high reliablity
    }

    if(APP_LE_AUDIO_UCST_STREAM_PORT_SPK_0 == port) {
        p_qos_params = &g_lea_ucst_qos_params_spk_0;
    }
    else if(APP_LE_AUDIO_UCST_STREAM_PORT_SPK_1 == port) {
        p_qos_params = &g_lea_ucst_qos_params_spk_1;
    }
    else if(APP_LE_AUDIO_UCST_STREAM_PORT_MIC_0 == port) {
        p_qos_params = &g_lea_ucst_qos_params_mic_0;
    }

    if(NULL != p_qos_params) {
        if (APP_LE_AUDIO_SAMPLING_FREQ_INVALID == (sampling_freq = app_le_audio_get_sample_freq(sampling_rate))) {
            LE_AUDIO_MSGLOG_I("[APP][U] set_qos_params port: %d, invalid sampling_rate:%x", 2, port, sampling_rate);
            return;
        }

        p_qos_params->sampling_freq = sampling_freq;

        p_qos_params->sdu_size = g_lea_ucst_qos_params_tbl[sel_setting].sdu_size;
        p_qos_params->sdu_interval = g_lea_ucst_qos_params_tbl[sel_setting].sdu_interval;
        p_qos_params->bitrate = g_lea_ucst_qos_params_tbl[sel_setting].bitrate;

        if (0x03 == target_latency) {
            p_qos_params->rtn = g_lea_ucst_qos_params_tbl[sel_setting].high_rtn;
            p_qos_params->latency = g_lea_ucst_qos_params_tbl[sel_setting].high_latency;
        } else {
            p_qos_params->rtn = g_lea_ucst_qos_params_tbl[sel_setting].low_rtn;
            p_qos_params->latency = g_lea_ucst_qos_params_tbl[sel_setting].low_latency;
        }

        LE_AUDIO_MSGLOG_I("[APP][U] set_qos_params port:%d, %d %d %d %d %d %d %d", 8,
                          port,
                          sampling_rate,
                          p_qos_params->sampling_freq,
                          p_qos_params->sdu_size,
                          p_qos_params->sdu_interval,
                          (int)(p_qos_params->bitrate * 10),
                          p_qos_params->rtn,
                          p_qos_params->latency);
    }
}


void app_le_audio_ucst_set_create_cis_mode(app_le_audio_ucst_create_cis_mode_t create_cis_mode)
{
    LE_AUDIO_MSGLOG_I("[APP][U] set_create_cis_mode:%x", 1, create_cis_mode);

    if (APP_LE_AUDIO_UCST_CREATE_CIS_MODE_MAX <= create_cis_mode) {
        return;
    }

    g_lea_ucst_ctrl.create_cis_mode = create_cis_mode;
}


void app_le_audio_ucst_set_ccid_list(uint8_t ccid_list_size, uint8_t *ccid_list)
{
    if (APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE < ccid_list_size){
        LE_AUDIO_MSGLOG_I("[APP] set_ccid_list, err ccid_list_size:%x", 1, ccid_list_size);
        return;
    }

    if((NULL != ccid_list) && (ccid_list_size > 0)) {
        memcpy(g_lea_ucst_ccid_list, ccid_list, ccid_list_size);
        g_lea_ucst_ccid_list_size = ccid_list_size;
    }
    else{
        memset(g_lea_ucst_ccid_list, 0, APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE);
        g_lea_ucst_ccid_list_size = 0;
    }
    LE_AUDIO_MSGLOG_I("[APP] set_ccid_list, ccid_list_size:%x %x %x", 3, g_lea_ucst_ccid_list_size, g_lea_ucst_ccid_list[0], g_lea_ucst_ccid_list[1]);
}


void app_le_audio_ucst_set_test_mode(uint8_t test_mode)
{
    /* 0: Diable test mode */
    /* 1: Enable test mode and stop before config codec */
    /* 2: Enable test mode */
    g_lea_ucst_test_mode_flag = test_mode;
}

app_le_audio_ucst_target_t app_le_audio_ucst_get_curr_target(void)
{
    return g_lea_ucst_ctrl.curr_target;
}

app_le_audio_ucst_pause_stream_t app_le_audio_ucst_get_pause_stream_flag(void)
{
    return g_lea_ucst_ctrl.pause_stream;
}

app_le_audio_ucst_create_cis_mode_t app_le_audio_ucst_get_create_cis_mode(void)
{
    return g_lea_ucst_ctrl.create_cis_mode;
}

uint32_t app_le_audio_ucst_get_sampling_rate(bool mic)
{
    uint8_t sampling_freq;

    if (mic) {
        sampling_freq = g_lea_ucst_qos_params_mic_0.sampling_freq;

    } else {
        if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
            sampling_freq = g_lea_ucst_qos_params_spk_0.sampling_freq;
        } else {
            sampling_freq = g_lea_ucst_qos_params_spk_1.sampling_freq;
        }
    }

    switch (sampling_freq) {
        case CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ:
            return 8000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ:
            return 16000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ:
            return 24000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ:
            return 32000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ:
            return 44100;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ:
            return 48000;
        default:
            break;
    }

    return 0;
}

uint16_t app_le_audio_ucst_get_sdu_size(bool mic)
{
    if (mic) {
        return g_lea_ucst_qos_params_mic_0.sdu_size;
    }

    if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
        return g_lea_ucst_qos_params_spk_0.sdu_size;
    }

    return g_lea_ucst_qos_params_spk_1.sdu_size;
}

uint32_t app_le_audio_ucst_get_sdu_interval(bool mic)
{
    uint8_t sdu_interval;

    if (mic) {
        sdu_interval = g_lea_ucst_qos_params_mic_0.sdu_interval;

    } else {
        if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
            sdu_interval = g_lea_ucst_qos_params_spk_0.sdu_interval;
        } else {
            sdu_interval = g_lea_ucst_qos_params_spk_1.sdu_interval;
        }
    }

    return g_lea_sdu_interval_tbl[sdu_interval];
}

float app_le_audio_ucst_get_bitrate(bool mic)
{
    if (mic) {
        return g_lea_ucst_qos_params_mic_0.bitrate;
    }
    if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
        return g_lea_ucst_qos_params_spk_0.bitrate;
    }
    return g_lea_ucst_qos_params_spk_1.bitrate;
}

uint32_t app_le_audio_ucst_get_location(uint8_t idx, uint32_t location)
{
    uint8_t i;

    if (32 <= idx) {
        return 0;
    }

    location = (location >> idx);
    i = idx;

    for(; i < 32; i++) {
        LE_AUDIO_MSGLOG_I("[APP][U] get_location, i:%x idx:%x location:%x", 3, i, idx, location);
        if (location & 0x00000001) {
            break;
        }
        location = (location >> 1);
        idx++;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] get_location, idx:%x", 1, idx);

    if (32 == idx) {
        return 0;
    }

    return (1 << idx);
}

uint8_t app_le_audio_ucst_get_location_count(uint32_t location)
{
    uint8_t i, count = 0;

    for(i = 0; i < 32; i++) {
        if (location & 0x00000001) {
            count++;
        }
        location = (location >> 1);
    }

    LE_AUDIO_MSGLOG_I("[APP][U] get_location_count, count:%x", 1, count);

    if (APP_LE_AUDIO_UCST_LOCATION_NUM_2 < count) {
        count = APP_LE_AUDIO_UCST_LOCATION_NUM_2;
    }

    return count;
}

uint32_t app_le_audio_ucst_get_available_channel(void)
{
    uint8_t i;

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            return g_lea_ucst_link_info[i].sink_location;
        }
    }

    return AUDIO_LOCATION_NONE;
}



void app_le_audio_ucst_reset_cig_parameter_test(void)
{
    if (NULL != g_lea_ucst_cig_params_test) {
        vPortFree(g_lea_ucst_cig_params_test);
        g_lea_ucst_cig_params_test = NULL;
    }
}


void app_le_audio_ucst_notify_mic_mute(bool mic_mute)
{
    uint8_t i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        le_audio_notify_mic_mute(g_lea_ucst_link_info[i].handle, mic_mute);
    }
}


app_le_audio_ucst_ctrl_t *app_le_audio_ucst_get_ctrl(void)
{
    return &g_lea_ucst_ctrl;
}


#ifdef APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE
void app_le_audio_ucst_set_uplink_mix_status(bool enable)
{
    g_lea_ucst_ctrl.uplink_mix_enable = enable;
}


bool app_le_audio_ucst_get_uplink_mix_status(void)
{
    return g_lea_ucst_ctrl.uplink_mix_enable;
}
#endif

static void app_le_audio_ucst_handle_stop_delay_disconnect_timeout(TimerHandle_t timer_handle, void *user_data)
{
    uint8_t streaming_port = app_le_audio_get_streaming_port();

    g_lea_ucst_delay_disconnect_timer = NULL;
    LE_AUDIO_MSGLOG_I("[APP][USB] handle_timeout, port:%x", 1, streaming_port);

#ifdef AIR_LE_AUDIO_DO_NOT_STOP_CALL_MODE_WHEN_CALL_EXIST
    if (app_le_audio_usb_hid_call_existing()) {
    //if (BLE_TBS_STATE_IDLE > (bt_le_audio_source_call_get_state(ble_tbs_get_gtbs_service_idx(), 1))) {
        //LE_AUDIO_MSGLOG_I("[APP][USB] handle_timeout, call", 0);
        return;
    }
#endif

    if (APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL != app_le_audio_ucst_get_create_cis_mode() &&
        APP_LE_AUDIO_MODE_UCST == app_le_audio_get_current_mode()) {
        app_le_audio_ucst_stop(true);
    }
}

bool app_le_audio_ucst_is_delay_disconnect_timer_exist(void)
{
    if (g_lea_ucst_delay_disconnect_timer) {
        return true;
    }
    return false;
}

void app_le_audio_ucst_start_delay_disconnect_timer(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (!g_lea_ucst_delay_disconnect_timer) {
        ret = app_le_audio_timer_start(&g_lea_ucst_delay_disconnect_timer,
                                       APP_LE_AUDIO_DISCONNET_UNICAST_DELAY_TIME,
                                       app_le_audio_ucst_handle_stop_delay_disconnect_timeout,
                                       NULL);

        if (BT_STATUS_SUCCESS != ret) {
            app_le_audio_timer_stop(g_lea_ucst_delay_disconnect_timer);
            g_lea_ucst_delay_disconnect_timer = NULL;
        }
    }

    if(BT_STATUS_SUCCESS != ret){
        LE_AUDIO_MSGLOG_I("[APP] start timer fail ret=%x", 1, ret);
    }
}
void app_le_audio_ucst_stop_delay_disconnect_timer(void)
{
    if (g_lea_ucst_delay_disconnect_timer) {
        app_le_audio_timer_stop(g_lea_ucst_delay_disconnect_timer);
        g_lea_ucst_delay_disconnect_timer = NULL;
    }
}


#endif  /* AIR_LE_AUDIO_ENABLE */

