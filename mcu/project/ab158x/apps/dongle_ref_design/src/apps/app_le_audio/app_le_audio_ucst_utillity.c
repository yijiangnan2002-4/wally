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

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
#include "ble_gmas_def.h"
#endif

#include "bt_le_audio_msglog.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_DISCONNECT_UNICAST_DELAY_TIME    (5000)  /* unit: ms */

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
#define APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_LOWEST_LATENCY      0
#define APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_LOW_LATENCY         1
#define APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_BALANCED            2
#define APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_HIGH_RELIABILITY    3
#define APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_MAX                 4
typedef uint8_t app_le_audio_ucst_gmap_ac_level_t;

#define APP_LE_UCST_GMAP_QOS_CONFIG(sampling_rate, sdu_size, sdu_interval, bitrate, lowest_rtn, lowest_latency, low_rtn, low_latency, balanced_rtn, balanced_latency, high_rtn, high_latency) \
    {sampling_rate, sdu_size, sdu_interval, bitrate, {{lowest_rtn, lowest_latency}, {low_rtn, low_latency}, {balanced_rtn, balanced_latency}, {high_rtn, high_latency}}}
#endif

/**************************************************************************************************
* Structure
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
typedef struct {
    uint8_t sampling_rate;                      /* sampling rate */
    uint16_t sdu_size;                          /* Maximum SDU size (octets) */
    app_le_audio_sdu_interval_t sdu_interval;   /* SDU interval */
    float bitrate;                              /* Bitrate (kbps) */
    uint8_t rtn;                                /* retransmission number */
    uint8_t latency;                            /* Max transport latency (ms) */
} app_le_audio_ucst_gmap_qos_params_t;

typedef struct {
    uint8_t ft;
    uint8_t nse;
    uint8_t bn;
    uint8_t rtn;
} app_le_audio_ucst_gmap_ll_params_t;
#endif

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

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
app_le_audio_qos_params_t g_lea_ucst_gmap_qos_params_spk_0 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,    /* sampling_freq */
    75,                                         /* sdu_size */
    SDU_INTERVAL_7P5_MS,                        /* sdu_interval */
    80,                                         /* bitrate */
    1,                                          /* rtn, not used, check g_lea_ucst_gmap_level */
    15,                                         /* latency, not used, check g_lea_ucst_gmap_level */
};

app_le_audio_qos_params_t g_lea_ucst_gmap_qos_params_mic_0 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ,    /* sampling_freq */
    60,                                         /* sdu_size */
    SDU_INTERVAL_7P5_MS,                        /* sdu_interval */
    64,                                         /* bitrate */
    1,                                          /* rtn, not used, check g_lea_ucst_gmap_level */
    15,                                         /* latency, not used, check g_lea_ucst_gmap_level */
};

app_le_audio_qos_params_t g_lea_ucst_gmap_qos_params_spk_1 = {
    CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ,    /* sampling_freq */
    75,                                         /* sdu_size */
    SDU_INTERVAL_7P5_MS,                        /* sdu_interval */
    80,                                         /* bitrate */
    1,                                          /* rtn, not used, check g_lea_ucst_gmap_level */
    15,                                         /* latency, not used, check g_lea_ucst_gmap_level */
};

app_le_audio_ucst_gmap_ac_level_t g_lea_ucst_gmap_level = APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_LOW_LATENCY;

#endif

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
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    {160,       SDU_INTERVAL_10_MS,    128,     13,    100,      13,       100},       /* 17 48_1_LC3plusHR_CBR */
    {310,       SDU_INTERVAL_10_MS,    248,     13,    100,      13,       100},       /* 18 48_2_LC3plusHR_CBR */
    {190,       SDU_INTERVAL_10_MS,    152,     13,    100,      13,       100},       /* 19 96_1_LC3plusHR_CBR */
    {310,       SDU_INTERVAL_10_MS,    248,     13,    100,      13,       100},       /* 20 96_2_LC3plusHR_CBR */
#endif
};

#define APP_LE_AUDIO_UCST_QOS_PARAMS_TBL_COUNT (sizeof(g_lea_ucst_qos_params_tbl)/sizeof(app_le_audio_qos_params_tbl_t))

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
static const app_le_audio_ucst_gmap_qos_params_t g_lea_ucst_gmap_qos_params_tbl[] = {
/*  Sampling_rate   sdu_size    sdu_interval            bitrate  rtn, latency} */
    /* GMAP d09r09 for mic and speaker */
    {32,             60,         SDU_INTERVAL_7P5_MS,    64,     1,     15},      /* 0  32_1 */
    {32,             80,         SDU_INTERVAL_10_MS,     64,     1,     20},      /* 1  32_2 */
    {48,             75,         SDU_INTERVAL_7P5_MS,    80,     1,     15},      /* 2  48_1 */
    {48,             100,        SDU_INTERVAL_10_MS,     80,     1,     20},      /* 3  48_2 */
    /* GMAP d09r09 for speaker */
    {48,             90,         SDU_INTERVAL_7P5_MS,    96,     1,     15},      /* 4  48_3 */
    {48,             120,        SDU_INTERVAL_10_MS,     96,     1,     20},      /* 5  48_4 */
    {16,             30,         SDU_INTERVAL_7P5_MS,    32,     1,     15},      /* 6  16_1 */
    {16,             40,         SDU_INTERVAL_10_MS,     32,     1,     20},      /* 7  16_2 */
};

#define APP_LE_AUDIO_UCST_GMAP_QOS_PARAMS_TBL_COUNT  (sizeof(g_lea_ucst_gmap_qos_params_tbl)/sizeof(app_le_audio_ucst_gmap_qos_params_t))

static const app_le_audio_ucst_gmap_ll_params_t g_lea_ucst_gmap_ll_params_tbl[APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_MAX] = {
/*   FT,    NSE,    BN,     RTN             Level   */
    {1,     2,      1,      1},     /* 0:   A       */
    {1,     3,      1,      2},     /* 1:   B       */
    {2,     2,      1,      3},     /* 2:   C       */
    {2,     3,      1,      5},     /* 3:   D       */
};
#endif

extern const uint32_t g_lea_sdu_interval_tbl[];

app_le_audio_ucst_cig_params_test_t *g_lea_ucst_cig_params_test = NULL;
bool g_lea_ucst_use_cig_params_test_cmd = false;

uint8_t g_lea_ucst_ccid_list_size = 0;

uint8_t g_lea_ucst_ccid_list[APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE] = {0};

static TimerHandle_t g_lea_ucst_delay_disconnect_timer = NULL;

uint8_t g_lea_ucst_test_mode_flag = 0;


/**************************************************************************************************
* Prototype
**************************************************************************************************/

extern bt_status_t le_audio_notify_mic_mute(bt_handle_t handle, bool mic_mute);

extern uint8_t app_le_audio_ucst_get_max_link_num(void);

extern void app_le_audio_ucst_set_cig_parameter_test_ex(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval);
/**************************************************************************************************
* Static Functions
**************************************************************************************************/

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
app_le_audio_ucst_stream_port_t app_le_audio_ucst_get_stream_port(bool mic)
{
    if (mic) {
        return APP_LE_AUDIO_UCST_STREAM_PORT_MIC_0;
    }

    if (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target) {
        return APP_LE_AUDIO_UCST_STREAM_PORT_SPK_0;
    }

    return APP_LE_AUDIO_UCST_STREAM_PORT_SPK_1;
}

app_le_audio_qos_params_t *app_le_audio_ucst_get_qos_params(app_le_audio_ucst_stream_port_t port, app_le_audio_qos_params_type_t qos_params_type)
{
    LE_AUDIO_MSGLOG_I("[APP][ASE] port:%x qos_params_type(%x):%d", 3, port, g_lea_ucst_ctrl.qos_params_type, qos_params_type);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    if (APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP == qos_params_type) {
        switch (port) {
            case APP_LE_AUDIO_UCST_STREAM_PORT_SPK_0: {
                return &g_lea_ucst_gmap_qos_params_spk_0;
            }
            case APP_LE_AUDIO_UCST_STREAM_PORT_SPK_1: {
                return &g_lea_ucst_gmap_qos_params_spk_1;
            }

            case APP_LE_AUDIO_UCST_STREAM_PORT_MIC_0: {
                return &g_lea_ucst_gmap_qos_params_mic_0;
            }
            default:
                break;
        }
    } else
#endif
    if (APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL == qos_params_type) {
        switch (port) {
            case APP_LE_AUDIO_UCST_STREAM_PORT_SPK_0: {
                return &g_lea_ucst_qos_params_spk_0;
            }

            case APP_LE_AUDIO_UCST_STREAM_PORT_SPK_1: {
                return &g_lea_ucst_qos_params_spk_1;
            }

            case APP_LE_AUDIO_UCST_STREAM_PORT_MIC_0: {
                return &g_lea_ucst_qos_params_mic_0;
            }
            default:
                break;
        }
    }

    return NULL;
}

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
void app_le_audio_ucst_get_qos_params_type(app_le_audio_qos_params_type_t *qos_params_type)
{
    bool decision_can_be_made = FALSE;
    uint8_t i = 0, link_idx = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    if (!qos_params_type) {
        return;
    }

    *qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;

    for (i = 0; i < app_le_audio_ucst_get_max_link_num(); i++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (link_idx = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[i])) {
            continue;
        }
#else
        link_idx = i;
#endif

        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[link_idx].handle) {
            LE_AUDIO_MSGLOG_I("[APP][GMAP] get_qos_param_type, handle:%x is_complete:%x role:%x qos_params_type:%x", 4,
                              g_lea_ucst_link_info[link_idx].handle,
                              g_lea_ucst_link_info[link_idx].gmap_discovery_complete,
                              g_lea_ucst_link_info[link_idx].gmap_role,
                              g_lea_ucst_link_info[link_idx].qos_params_type);

            if ((APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL == g_lea_ucst_link_info[link_idx].qos_params_type) ||
                (APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP == g_lea_ucst_link_info[link_idx].qos_params_type)) {
                decision_can_be_made = TRUE;
                if (BLE_GMAP_ROLE_MASK_UGT & g_lea_ucst_link_info[link_idx].gmap_role) {
                    break;
                }
            }
        }
    }

    if (!decision_can_be_made) {
        LE_AUDIO_MSGLOG_I("[APP][GMAP] get_qos_param_type, cannot decide due to no LE link or GMAP discover not finish", 0);
        *qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NONE;
        return;
    }

    if (i < app_le_audio_ucst_get_max_link_num()) {
#ifdef AIR_LE_AUDIO_TMAP_ENABLE /* Default use TMAP. Use AT-cmd to enable GMAP. */
        if (APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP == g_lea_ucst_ctrl.qos_params_type)
#endif
        {
            /* There's at least one LE link supporting GMAP UGT. Use GMAP Qos parameters. */
            *qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP;
            if (NULL == g_lea_ucst_cig_params_test) {
                app_le_audio_ucst_set_cig_parameter_test_ex(g_lea_ucst_gmap_ll_params_tbl[g_lea_ucst_gmap_level].bn,
                                                         g_lea_ucst_gmap_ll_params_tbl[g_lea_ucst_gmap_level].nse,
                                                         g_lea_ucst_gmap_ll_params_tbl[g_lea_ucst_gmap_level].ft,
                                                         8);
            }
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][GMAP] qos_params_type(%x): %d", 2, g_lea_ucst_ctrl.qos_params_type, *qos_params_type);
}
#endif

void app_le_audio_ucst_set_cig_parameter_test_ex(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval)
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

void app_le_audio_ucst_set_cig_parameter_test(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval)
{
    g_lea_ucst_use_cig_params_test_cmd = true;
    app_le_audio_ucst_set_cig_parameter_test_ex(bn, nse, ft, iso_interval);
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

        g_lea_ucst_ctrl.qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;

        if (!g_lea_ucst_use_cig_params_test_cmd) {
            g_lea_ucst_use_cig_params_test_cmd = false;
            app_le_audio_ucst_reset_cig_parameter_test();
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

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
bool app_le_audio_ucst_gmap_set_qos_params(uint8_t sel_setting, uint8_t audio_config_level, app_le_audio_ucst_stream_port_t port)
{
    app_le_audio_qos_params_t *qos_parameter = NULL;
    app_le_audio_ucst_gmap_qos_params_t *gmap_qos;

    LE_AUDIO_MSGLOG_I("[APP][U][GMAP] set_qos_params, sel_setting:%d audio_config_level:%d port:%d", 3, sel_setting, audio_config_level, port);

    if (APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_MAX <= audio_config_level || APP_LE_AUDIO_UCST_GMAP_QOS_PARAMS_TBL_COUNT <= sel_setting || APP_LE_AUDIO_UCST_STREAM_PORT_MAX <= port) {
        LE_AUDIO_MSGLOG_I("[APP][U][GMAP] set_qos_params invalid, max sel_setting:%d level:%d port:%d", 3,
            APP_LE_AUDIO_UCST_GMAP_QOS_PARAMS_TBL_COUNT, APP_LE_AUDIO_UCST_GMAP_AC_LEVEL_MAX, APP_LE_AUDIO_UCST_STREAM_PORT_MAX);
        return FALSE;
    }

    gmap_qos = (app_le_audio_ucst_gmap_qos_params_t *)&g_lea_ucst_gmap_qos_params_tbl[sel_setting];
    g_lea_ucst_ctrl.qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP;
    g_lea_ucst_qos_params_selected = 0;
    qos_parameter = app_le_audio_ucst_get_qos_params(port, APP_LE_AUDIO_QOS_PARAMS_TYPE_GMAP);
    if (!qos_parameter) {
        LE_AUDIO_MSGLOG_I("[APP][U][GMAP] fail to get qos_parameter.", 0);
        return FALSE;
    }

    qos_parameter->sampling_freq = app_le_audio_get_sample_freq(gmap_qos->sampling_rate);
    qos_parameter->sdu_size      = gmap_qos->sdu_size;
    qos_parameter->sdu_interval  = gmap_qos->sdu_interval;
    qos_parameter->bitrate       = gmap_qos->bitrate;
    qos_parameter->rtn           = g_lea_ucst_gmap_ll_params_tbl[audio_config_level].rtn;
    qos_parameter->latency       = gmap_qos->latency;

    g_lea_ucst_gmap_level = audio_config_level;


    LE_AUDIO_MSGLOG_I("[APP][U][GMAP] set_qos_params port:%d qos_parameter:%d %d %d %d %d %d", 7,
        port,
        qos_parameter->sampling_freq,
        qos_parameter->sdu_size,
        qos_parameter->sdu_interval,
        (int)(qos_parameter->bitrate * 10),
        qos_parameter->rtn,
        qos_parameter->latency);


    uint16_t iso_interval = 8;
    if (SDU_INTERVAL_7P5_MS == gmap_qos->sdu_interval) {
        iso_interval = 6;
    }
    g_lea_ucst_use_cig_params_test_cmd = false;
    app_le_audio_ucst_set_cig_parameter_test_ex(g_lea_ucst_gmap_ll_params_tbl[audio_config_level].bn,
                                             g_lea_ucst_gmap_ll_params_tbl[audio_config_level].nse,
                                             g_lea_ucst_gmap_ll_params_tbl[audio_config_level].ft,
                                             iso_interval);

    return TRUE;
}
#endif

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
    app_le_audio_qos_params_t *p_qos_params;
    app_le_audio_qos_params_type_t qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;
    app_le_audio_ucst_stream_port_t port = app_le_audio_ucst_get_stream_port(mic);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    app_le_audio_ucst_get_qos_params_type(&qos_params_type);
#endif

    if (NULL == (p_qos_params = app_le_audio_ucst_get_qos_params(port, qos_params_type))) {
        return 0;
    }

    switch (p_qos_params->sampling_freq) {
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
        case CODEC_CONFIGURATION_SAMPLING_FREQ_96KHZ:
            return 96000;
        default:
            break;
    }

    return 0;
}

uint16_t app_le_audio_ucst_get_sdu_size(bool mic)
{
    app_le_audio_qos_params_t *p_qos_params;
    app_le_audio_qos_params_type_t qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;
    app_le_audio_ucst_stream_port_t port = app_le_audio_ucst_get_stream_port(mic);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    app_le_audio_ucst_get_qos_params_type(&qos_params_type);
#endif

    if (NULL == (p_qos_params = app_le_audio_ucst_get_qos_params(port, qos_params_type))) {
        return 0;
    }

    return p_qos_params->sdu_size;
}

uint32_t app_le_audio_ucst_get_sdu_interval(bool mic)
{
    app_le_audio_qos_params_t *p_qos_params;
    app_le_audio_qos_params_type_t qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;
    app_le_audio_ucst_stream_port_t port = app_le_audio_ucst_get_stream_port(mic);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    app_le_audio_ucst_get_qos_params_type(&qos_params_type);
#endif

    if (NULL == (p_qos_params = app_le_audio_ucst_get_qos_params(port, qos_params_type))) {
        return 0;
    }

    return g_lea_sdu_interval_tbl[p_qos_params->sdu_interval];
}

float app_le_audio_ucst_get_bitrate(bool mic)
{
    app_le_audio_qos_params_t *p_qos_params;
    app_le_audio_qos_params_type_t qos_params_type = APP_LE_AUDIO_QOS_PARAMS_TYPE_NORMAL;
    app_le_audio_ucst_stream_port_t port = app_le_audio_ucst_get_stream_port(mic);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    app_le_audio_ucst_get_qos_params_type(&qos_params_type);
#endif

    if (NULL == (p_qos_params = app_le_audio_ucst_get_qos_params(port, qos_params_type))) {
        return 0;
    }

    return p_qos_params->bitrate;
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
                                       APP_LE_AUDIO_DISCONNECT_UNICAST_DELAY_TIME,
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

